// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/sync/engine/conflict_resolver.h"

#include <algorithm>
#include <map>
#include <set>

#include "base/location.h"
#include "base/metrics/histogram.h"
#include "chrome/browser/sync/engine/syncer.h"
#include "chrome/browser/sync/engine/syncer_util.h"
#include "chrome/browser/sync/protocol/nigori_specifics.pb.h"
#include "chrome/browser/sync/protocol/service_constants.h"
#include "chrome/browser/sync/sessions/status_controller.h"
#include "chrome/browser/sync/syncable/directory_manager.h"
#include "chrome/browser/sync/syncable/syncable.h"
#include "chrome/browser/sync/util/cryptographer.h"

using std::map;
using std::set;
using syncable::BaseTransaction;
using syncable::Directory;
using syncable::Entry;
using syncable::Id;
using syncable::MutableEntry;
using syncable::ScopedDirLookup;
using syncable::WriteTransaction;

namespace browser_sync {

using sessions::ConflictProgress;
using sessions::StatusController;

namespace {

const int SYNC_CYCLES_BEFORE_ADMITTING_DEFEAT = 8;

// Enumeration of different conflict resolutions. Used for histogramming.
enum SimpleConflictResolutions {
  OVERWRITE_LOCAL,           // Resolved by overwriting local changes.
  OVERWRITE_SERVER,          // Resolved by overwriting server changes.
  UNDELETE,                  // Resolved by undeleting local item.
  IGNORE_ENCRYPTION,         // Resolved by ignoring an encryption-only server
                             // change. TODO(zea): implement and use this.
  NIGORI_MERGE,              // Resolved by merging nigori nodes.
  CONFLICT_RESOLUTION_SIZE,
};

}  // namespace

ConflictResolver::ConflictResolver() {
}

ConflictResolver::~ConflictResolver() {
}

void ConflictResolver::IgnoreLocalChanges(MutableEntry* entry) {
  // An update matches local actions, merge the changes.
  // This is a little fishy because we don't actually merge them.
  // In the future we should do a 3-way merge.
  DVLOG(1) << "Resolving conflict by ignoring local changes:" << entry;
  // With IS_UNSYNCED false, changes should be merged.
  entry->Put(syncable::IS_UNSYNCED, false);
}

void ConflictResolver::OverwriteServerChanges(WriteTransaction* trans,
                                              MutableEntry * entry) {
  // This is similar to an overwrite from the old client.
  // This is equivalent to a scenario where we got the update before we'd
  // made our local client changes.
  // TODO(chron): This is really a general property clobber. We clobber
  // the server side property. Perhaps we should actually do property merging.
  DVLOG(1) << "Resolving conflict by ignoring server changes:" << entry;
  entry->Put(syncable::BASE_VERSION, entry->Get(syncable::SERVER_VERSION));
  entry->Put(syncable::IS_UNAPPLIED_UPDATE, false);
}

ConflictResolver::ProcessSimpleConflictResult
ConflictResolver::ProcessSimpleConflict(WriteTransaction* trans,
                                        const Id& id,
                                        const Cryptographer* cryptographer,
                                        StatusController* status) {
  MutableEntry entry(trans, syncable::GET_BY_ID, id);
  // Must be good as the entry won't have been cleaned up.
  CHECK(entry.good());
  // If an update fails, locally we have to be in a set or unsynced. We're not
  // in a set here, so we must be unsynced.
  if (!entry.Get(syncable::IS_UNSYNCED)) {
    return NO_SYNC_PROGRESS;
  }

  if (!entry.Get(syncable::IS_UNAPPLIED_UPDATE)) {
    if (!entry.Get(syncable::PARENT_ID).ServerKnows()) {
      DVLOG(1) << "Item conflicting because its parent not yet committed. Id: "
               << id;
    } else {
      DVLOG(1) << "No set for conflicting entry id " << id << ". There should "
               << "be an update/commit that will fix this soon. This message "
               << "should not repeat.";
    }
    return NO_SYNC_PROGRESS;
  }

  if (entry.Get(syncable::IS_DEL) && entry.Get(syncable::SERVER_IS_DEL)) {
    // we've both deleted it, so lets just drop the need to commit/update this
    // entry.
    entry.Put(syncable::IS_UNSYNCED, false);
    entry.Put(syncable::IS_UNAPPLIED_UPDATE, false);
    // we've made changes, but they won't help syncing progress.
    // METRIC simple conflict resolved by merge.
    return NO_SYNC_PROGRESS;
  }

  if (!entry.Get(syncable::SERVER_IS_DEL)) {
    // This logic determines "client wins" vs. "server wins" strategy picking.
    // TODO(nick): The current logic is arbitrary; instead, it ought to be made
    // consistent with the ModelAssociator behavior for a datatype.  It would
    // be nice if we could route this back to ModelAssociator code to pick one
    // of three options: CLIENT, SERVER, or MERGE.  Some datatypes (autofill)
    // are easily mergeable.
    // See http://crbug.com/77339.
    bool name_matches = entry.Get(syncable::NON_UNIQUE_NAME) ==
                        entry.Get(syncable::SERVER_NON_UNIQUE_NAME);
    bool parent_matches = entry.Get(syncable::PARENT_ID) ==
                                    entry.Get(syncable::SERVER_PARENT_ID);
    bool entry_deleted = entry.Get(syncable::IS_DEL);

    // We manually merge nigori data.
    if (entry.GetModelType() == syncable::NIGORI) {
      // Create a new set of specifics based on the server specifics (which
      // preserves their encryption keys).
      sync_pb::EntitySpecifics specifics =
          entry.Get(syncable::SERVER_SPECIFICS);
      sync_pb::NigoriSpecifics* nigori =
          specifics.MutableExtension(sync_pb::nigori);
      // Store the merged set of encrypted types (cryptographer->Update(..) will
      // have merged the local types already).
      cryptographer->UpdateNigoriFromEncryptedTypes(nigori);
      // The local set of keys is already merged with the server's set within
      // the cryptographer. If we don't have pending keys we can store the
      // merged set back immediately. Else we preserve the server keys and will
      // update the nigori when the user provides the pending passphrase via
      // SetPassphrase(..).
      if (cryptographer->is_ready()) {
        cryptographer->GetKeys(nigori->mutable_encrypted());
      }
      // TODO(zea): Find a better way of doing this. As it stands, we have to
      // update this code whenever we add a new non-cryptographer related field
      // to the nigori node.
      if (entry.Get(syncable::SPECIFICS).GetExtension(sync_pb::nigori)
              .sync_tabs()) {
        nigori->set_sync_tabs(true);
      }
      entry.Put(syncable::SPECIFICS, specifics);
      DVLOG(1) << "Resovling simple conflict, merging nigori nodes: " << entry;
      status->increment_num_server_overwrites();
      OverwriteServerChanges(trans, &entry);
      UMA_HISTOGRAM_ENUMERATION("Sync.ResolveSimpleConflict",
                                NIGORI_MERGE,
                                CONFLICT_RESOLUTION_SIZE);
    } else if (!entry_deleted && name_matches && parent_matches) {
      // TODO(zea): We may prefer to choose the local changes over the server
      // if we know the local changes happened before (or vice versa).
      // See http://crbug.com/76596
      DVLOG(1) << "Resolving simple conflict, ignoring local changes for:"
               << entry;
      IgnoreLocalChanges(&entry);
      status->increment_num_local_overwrites();
      UMA_HISTOGRAM_ENUMERATION("Sync.ResolveSimpleConflict",
                                OVERWRITE_LOCAL,
                                CONFLICT_RESOLUTION_SIZE);
    } else {
      DVLOG(1) << "Resolving simple conflict, overwriting server changes for:"
               << entry;
      OverwriteServerChanges(trans, &entry);
      status->increment_num_server_overwrites();
      UMA_HISTOGRAM_ENUMERATION("Sync.ResolveSimpleConflict",
                                OVERWRITE_SERVER,
                                CONFLICT_RESOLUTION_SIZE);
    }
    return SYNC_PROGRESS;
  } else {  // SERVER_IS_DEL is true
    // If a server deleted folder has local contents we should be in a set.
    if (entry.Get(syncable::IS_DIR)) {
      Directory::ChildHandles children;
      trans->directory()->GetChildHandlesById(trans,
                                              entry.Get(syncable::ID),
                                              &children);
      if (0 != children.size()) {
        DVLOG(1) << "Entry is a server deleted directory with local contents, "
                 << "should be in a set. (race condition).";
        return NO_SYNC_PROGRESS;
      }
    }

    // The entry is deleted on the server but still exists locally.
    if (!entry.Get(syncable::UNIQUE_CLIENT_TAG).empty()) {
      // If we've got a client-unique tag, we can undelete while retaining
      // our present ID.
      DCHECK_EQ(entry.Get(syncable::SERVER_VERSION), 0) << "For the server to "
          "know to re-create, client-tagged items should revert to version 0 "
          "when server-deleted.";
      OverwriteServerChanges(trans, &entry);
      status->increment_num_server_overwrites();
      UMA_HISTOGRAM_ENUMERATION("Sync.ResolveSimpleConflict",
                                OVERWRITE_SERVER,
                                CONFLICT_RESOLUTION_SIZE);
      // Clobber the versions, just in case the above DCHECK is violated.
      entry.Put(syncable::SERVER_VERSION, 0);
      entry.Put(syncable::BASE_VERSION, 0);
    } else {
      // Otherwise, we've got to undelete by creating a new locally
      // uncommitted entry.
      SyncerUtil::SplitServerInformationIntoNewEntry(trans, &entry);

      MutableEntry server_update(trans, syncable::GET_BY_ID, id);
      CHECK(server_update.good());
      CHECK(server_update.Get(syncable::META_HANDLE) !=
            entry.Get(syncable::META_HANDLE))
          << server_update << entry;
      UMA_HISTOGRAM_ENUMERATION("Sync.ResolveSimpleConflict",
                                UNDELETE,
                                CONFLICT_RESOLUTION_SIZE);
    }
    return SYNC_PROGRESS;
  }
}

bool ConflictResolver::ResolveSimpleConflicts(
    syncable::WriteTransaction* trans,
    const Cryptographer* cryptographer,
    const ConflictProgress& progress,
    sessions::StatusController* status) {
  bool forward_progress = false;
  // First iterate over simple conflict items (those that belong to no set).
  set<Id>::const_iterator conflicting_item_it;
  for (conflicting_item_it = progress.ConflictingItemsBegin();
       conflicting_item_it != progress.ConflictingItemsEnd();
       ++conflicting_item_it) {
    Id id = *conflicting_item_it;
    map<Id, ConflictSet*>::const_iterator item_set_it =
        progress.IdToConflictSetFind(id);
    if (item_set_it == progress.IdToConflictSetEnd() ||
        0 == item_set_it->second) {
      // We have a simple conflict.
      switch (ProcessSimpleConflict(trans, id, cryptographer, status)) {
        case NO_SYNC_PROGRESS:
          break;
        case SYNC_PROGRESS:
          forward_progress = true;
          break;
      }
    }
  }
  return forward_progress;
}

bool ConflictResolver::ResolveConflicts(syncable::WriteTransaction* trans,
                                        const Cryptographer* cryptographer,
                                        const ConflictProgress& progress,
                                        sessions::StatusController* status) {
  // TODO(rlarocque): A good amount of code related to the resolution of
  // conflict sets has been deleted here.  This was done because the code had
  // not been used in years.  An unrelated bug fix accidentally re-enabled the
  // code, forcing us to make a decision about what we should do with the code.
  // We decided to do the safe thing and delete it for now.  This restores the
  // behaviour we've relied on for quite some time.  We should think about what
  // that code was trying to do and consider re-enabling parts of it.

  if (progress.ConflictSetsSize() > 0) {
    DVLOG(1) << "Detected " << progress.IdToConflictSetSize()
        << " non-simple conflicting entries in " << progress.ConflictSetsSize()
        << " unprocessed conflict sets.";
  }

  return ResolveSimpleConflicts(trans, cryptographer, progress, status);
}

}  // namespace browser_sync
