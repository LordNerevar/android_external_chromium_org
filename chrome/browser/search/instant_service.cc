// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/search/instant_service.h"

#include <vector>

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "build/build_config.h"
#include "chrome/browser/history/history_notifications.h"
#include "chrome/browser/history/top_sites.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/search/instant_io_context.h"
#include "chrome/browser/search/instant_service_factory.h"
#include "chrome/browser/search/local_ntp_source.h"
#include "chrome/browser/search/most_visited_iframe_source.h"
#include "chrome/browser/search/search.h"
#include "chrome/browser/search/suggestion_iframe_source.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_instant_controller.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/host_desktop.h"
#include "chrome/browser/ui/search/instant_controller.h"
#include "chrome/browser/ui/webui/favicon_source.h"
#include "chrome/browser/ui/webui/ntp/thumbnail_source.h"
#include "chrome/browser/ui/webui/theme_source.h"
#include "chrome/common/chrome_notification_types.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/url_data_source.h"
#include "googleurl/src/gurl.h"
#include "net/url_request/url_request.h"

using content::BrowserThread;

InstantService::InstantService(Profile* profile)
    : profile_(profile),
      weak_ptr_factory_(this) {
  // Stub for unit tests.
  if (!BrowserThread::CurrentlyOn(BrowserThread::UI))
    return;

  registrar_.Add(this,
                 content::NOTIFICATION_RENDERER_PROCESS_TERMINATED,
                 content::NotificationService::AllSources());

  history::TopSites* top_sites = profile_->GetTopSites();
  if (top_sites) {
    registrar_.Add(this,
                   chrome::NOTIFICATION_TOP_SITES_CHANGED,
                   content::Source<history::TopSites>(top_sites));
  }
  instant_io_context_ = new InstantIOContext();

  if (profile_ && profile_->GetResourceContext()) {
    BrowserThread::PostTask(
        BrowserThread::IO, FROM_HERE,
        base::Bind(&InstantIOContext::SetUserDataOnIO,
                   profile->GetResourceContext(), instant_io_context_));
  }

  // Set up the data sources that Instant uses on the NTP.
#if defined(ENABLE_THEMES)
  content::URLDataSource::Add(profile, new ThemeSource(profile));
#endif
  content::URLDataSource::Add(profile, new ThumbnailSource(profile));
  content::URLDataSource::Add(profile, new FaviconSource(
      profile, FaviconSource::FAVICON));
  content::URLDataSource::Add(profile, new LocalNtpSource());
  content::URLDataSource::Add(profile, new SuggestionIframeSource());
  content::URLDataSource::Add(profile, new MostVisitedIframeSource());
}

InstantService::~InstantService() {
}

void InstantService::AddInstantProcess(int process_id) {
  process_ids_.insert(process_id);

  if (instant_io_context_.get()) {
    BrowserThread::PostTask(BrowserThread::IO,
                            FROM_HERE,
                            base::Bind(&InstantIOContext::AddInstantProcessOnIO,
                                       instant_io_context_,
                                       process_id));
  }
}

bool InstantService::IsInstantProcess(int process_id) const {
  return process_ids_.find(process_id) != process_ids_.end();
}

void InstantService::DeleteMostVisitedItem(const GURL& url) {
  history::TopSites* top_sites = profile_->GetTopSites();
  if (!top_sites)
    return;

  top_sites->AddBlacklistedURL(url);
}

void InstantService::UndoMostVisitedDeletion(const GURL& url) {
  history::TopSites* top_sites = profile_->GetTopSites();
  if (!top_sites)
    return;

  top_sites->RemoveBlacklistedURL(url);
}

void InstantService::UndoAllMostVisitedDeletions() {
  history::TopSites* top_sites = profile_->GetTopSites();
  if (!top_sites)
    return;

  top_sites->ClearBlacklistedURLs();
}

void InstantService::GetCurrentMostVisitedItems(
    std::vector<InstantMostVisitedItem>* items) const {
  *items = most_visited_items_;
}

void InstantService::Shutdown() {
  process_ids_.clear();

  if (instant_io_context_.get()) {
    BrowserThread::PostTask(
        BrowserThread::IO,
        FROM_HERE,
        base::Bind(&InstantIOContext::ClearInstantProcessesOnIO,
                   instant_io_context_));
  }
  instant_io_context_ = NULL;
}

void InstantService::Observe(int type,
                             const content::NotificationSource& source,
                             const content::NotificationDetails& details) {
  switch (type) {
    case content::NOTIFICATION_RENDERER_PROCESS_TERMINATED: {
      int process_id =
          content::Source<content::RenderProcessHost>(source)->GetID();
      process_ids_.erase(process_id);

      if (instant_io_context_.get()) {
        BrowserThread::PostTask(
            BrowserThread::IO,
            FROM_HERE,
            base::Bind(&InstantIOContext::RemoveInstantProcessOnIO,
                       instant_io_context_,
                       process_id));
      }
      break;
    }
    case chrome::NOTIFICATION_TOP_SITES_CHANGED: {
      history::TopSites* top_sites = profile_->GetTopSites();
      if (top_sites) {
        top_sites->GetMostVisitedURLs(
            base::Bind(&InstantService::OnMostVisitedItemsReceived,
                       weak_ptr_factory_.GetWeakPtr()));
      }
      break;
    }
    default:
      NOTREACHED() << "Unexpected notification type in InstantService.";
  }
}

void InstantService::OnMostVisitedItemsReceived(
    const history::MostVisitedURLList& data) {
  // Android doesn't use Browser/BrowserList. Do nothing for Android platform.
#if !defined(OS_ANDROID)
  history::MostVisitedURLList reordered_data(data);
  history::TopSites::MaybeShuffle(&reordered_data);

  std::vector<InstantMostVisitedItem> new_most_visited_items;
  for (size_t i = 0; i < reordered_data.size(); i++) {
    const history::MostVisitedURL& url = reordered_data[i];
    InstantMostVisitedItem item;
    item.url = url.url;
    item.title = url.title;
    new_most_visited_items.push_back(item);
  }
  if (chrome::AreMostVisitedItemsEqual(new_most_visited_items,
                                       most_visited_items_)) {
    return;
  }

  most_visited_items_ = new_most_visited_items;

  const BrowserList* browser_list =
      BrowserList::GetInstance(chrome::GetActiveDesktop());
  for (BrowserList::const_iterator it = browser_list->begin();
       it != browser_list->end(); ++it) {
    if ((*it)->profile() != profile_ || !((*it)->instant_controller()))
      continue;

    InstantController* controller = (*it)->instant_controller()->instant();
    if (!controller)
      continue;
    // TODO(kmadhusu): It would be cleaner to have each InstantController
    // register itself as an InstantServiceObserver and push out updates that
    // way. Refer to crbug.com/246355 for more details.
    controller->UpdateMostVisitedItems();
  }
#endif
}
