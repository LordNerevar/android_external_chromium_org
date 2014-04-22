// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_BROWSER_APPCACHE_APPCACHE_GROUP_H_
#define WEBKIT_BROWSER_APPCACHE_APPCACHE_GROUP_H_

#include <map>
#include <vector>

#include "base/cancelable_callback.h"
#include "base/gtest_prod_util.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "base/time/time.h"
#include "url/gurl.h"
#include "webkit/browser/webkit_storage_browser_export.h"

namespace content {
FORWARD_DECLARE_TEST(AppCacheGroupTest, StartUpdate);
FORWARD_DECLARE_TEST(AppCacheGroupTest, CancelUpdate);
FORWARD_DECLARE_TEST(AppCacheGroupTest, QueueUpdate);
FORWARD_DECLARE_TEST(AppCacheUpdateJobTest, AlreadyChecking);
FORWARD_DECLARE_TEST(AppCacheUpdateJobTest, AlreadyDownloading);
class AppCacheUpdateJobTest;
class MockAppCacheStorage;
}

namespace appcache {

class AppCache;
class AppCacheHost;
class AppCacheStorage;
class AppCacheUpdateJob;
class HostObserver;

// Collection of application caches identified by the same manifest URL.
// A group exists as long as it is in use by a host or is being updated.
class WEBKIT_STORAGE_BROWSER_EXPORT AppCacheGroup
    : public base::RefCounted<AppCacheGroup> {
 public:

  class WEBKIT_STORAGE_BROWSER_EXPORT UpdateObserver {
    public:
      // Called just after an appcache update has completed.
      virtual void OnUpdateComplete(AppCacheGroup* group) = 0;
      virtual ~UpdateObserver() {}
  };

  enum UpdateStatus {
    IDLE,
    CHECKING,
    DOWNLOADING,
  };

  AppCacheGroup(AppCacheStorage* storage, const GURL& manifest_url,
                int64 group_id);

  // Adds/removes an update observer, the AppCacheGroup does not take
  // ownership of the observer.
  void AddUpdateObserver(UpdateObserver* observer);
  void RemoveUpdateObserver(UpdateObserver* observer);

  int64 group_id() const { return group_id_; }
  const GURL& manifest_url() const { return manifest_url_; }
  const base::Time& creation_time() const { return creation_time_; }
  void set_creation_time(const base::Time& time) { creation_time_ = time; }
  bool is_obsolete() const { return is_obsolete_; }
  void set_obsolete(bool value) { is_obsolete_ = value; }

  bool is_being_deleted() const { return is_being_deleted_; }
  void set_being_deleted(bool value) { is_being_deleted_ = value; }

  AppCache* newest_complete_cache() const { return newest_complete_cache_; }

  void AddCache(AppCache* complete_cache);
  void RemoveCache(AppCache* cache);
  bool HasCache() const { return newest_complete_cache_ != NULL; }

  void AddNewlyDeletableResponseIds(std::vector<int64>* response_ids);

  UpdateStatus update_status() const { return update_status_; }

  // Starts an update via update() javascript API.
  void StartUpdate() {
    StartUpdateWithHost(NULL);
  }

  // Starts an update for a doc loaded from an application cache.
  void StartUpdateWithHost(AppCacheHost* host)  {
    StartUpdateWithNewMasterEntry(host, GURL());
  }

  // Starts an update for a doc loaded using HTTP GET or equivalent with
  // an <html> tag manifest attribute value that matches this group's
  // manifest url.
  void StartUpdateWithNewMasterEntry(AppCacheHost* host,
                                     const GURL& new_master_resource);

  // Cancels an update if one is running.
  void CancelUpdate();

 private:
  class HostObserver;

  friend class base::RefCounted<AppCacheGroup>;
  friend class content::AppCacheUpdateJobTest;
  friend class content::MockAppCacheStorage;  // for old_caches()
  friend class AppCacheUpdateJob;

  ~AppCacheGroup();

  typedef std::vector<AppCache*> Caches;
  typedef std::map<AppCacheHost*, GURL> QueuedUpdates;

  static const int kUpdateRestartDelayMs = 1000;

  AppCacheUpdateJob* update_job() { return update_job_; }
  void SetUpdateStatus(UpdateStatus status);

  void NotifyContentBlocked();

  const Caches& old_caches() const { return old_caches_; }

  // Update cannot be processed at this time. Queue it for a later run.
  void QueueUpdate(AppCacheHost* host, const GURL& new_master_resource);
  void RunQueuedUpdates();
  bool FindObserver(UpdateObserver* find_me,
                    const ObserverList<UpdateObserver>& observer_list);
  void ScheduleUpdateRestart(int delay_ms);
  void HostDestructionImminent(AppCacheHost* host);

  const int64 group_id_;
  const GURL manifest_url_;
  base::Time creation_time_;
  UpdateStatus update_status_;
  bool is_obsolete_;
  bool is_being_deleted_;
  std::vector<int64> newly_deletable_response_ids_;

  // Old complete app caches.
  Caches old_caches_;

  // Newest cache in this group to be complete, aka relevant cache.
  AppCache* newest_complete_cache_;

  // Current update job for this group, if any.
  AppCacheUpdateJob* update_job_;

  // Central storage object.
  AppCacheStorage* storage_;

  // List of objects observing this group.
  ObserverList<UpdateObserver> observers_;

  // Updates that have been queued for the next run.
  QueuedUpdates queued_updates_;
  ObserverList<UpdateObserver> queued_observers_;
  base::CancelableClosure restart_update_task_;
  scoped_ptr<HostObserver> host_observer_;

  // True if we're in our destructor.
  bool is_in_dtor_;

  FRIEND_TEST_ALL_PREFIXES(content::AppCacheGroupTest, StartUpdate);
  FRIEND_TEST_ALL_PREFIXES(content::AppCacheGroupTest, CancelUpdate);
  FRIEND_TEST_ALL_PREFIXES(content::AppCacheGroupTest, QueueUpdate);
  FRIEND_TEST_ALL_PREFIXES(content::AppCacheUpdateJobTest, AlreadyChecking);
  FRIEND_TEST_ALL_PREFIXES(content::AppCacheUpdateJobTest, AlreadyDownloading);

  DISALLOW_COPY_AND_ASSIGN(AppCacheGroup);
};

}  // namespace appcache

#endif  // WEBKIT_BROWSER_APPCACHE_APPCACHE_GROUP_H_
