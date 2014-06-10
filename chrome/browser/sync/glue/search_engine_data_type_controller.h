// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SYNC_GLUE_SEARCH_ENGINE_DATA_TYPE_CONTROLLER_H__
#define CHROME_BROWSER_SYNC_GLUE_SEARCH_ENGINE_DATA_TYPE_CONTROLLER_H__

#include <string>

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "chrome/browser/search_engines/template_url_service.h"
#include "chrome/browser/sync/glue/ui_data_type_controller.h"
#include "components/sync_driver/generic_change_processor.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"

class Profile;

namespace browser_sync {

class SearchEngineDataTypeController : public UIDataTypeController {
 public:
  SearchEngineDataTypeController(
      SyncApiComponentFactory* profile_sync_factory,
      Profile* profile,
      ProfileSyncService* sync_service);

 private:
  virtual ~SearchEngineDataTypeController();

  // FrontendDataTypeController implementations.
  virtual bool StartModels() OVERRIDE;

  void OnTemplateURLServiceLoaded();

  scoped_ptr<TemplateURLService::Subscription> template_url_subscription_;
  Profile* const profile_;

  DISALLOW_COPY_AND_ASSIGN(SearchEngineDataTypeController);
};

}  // namespace browser_sync

#endif  // CHROME_BROWSER_SYNC_GLUE_SEARCH_ENGINE_DATA_TYPE_CONTROLLER_H__
