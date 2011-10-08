// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/sync_promo_ui.h"

#include "base/command_line.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/first_run/first_run.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/sync/profile_sync_service.h"
#include "chrome/browser/ui/webui/chrome_url_data_manager.h"
#include "chrome/browser/ui/webui/chrome_web_ui_data_source.h"
#include "chrome/browser/ui/webui/options/core_options_handler.h"
#include "chrome/browser/ui/webui/sync_promo_handler.h"
#include "chrome/browser/ui/webui/theme_source.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/pref_names.h"
#include "chrome/common/url_constants.h"
#include "content/browser/tab_contents/tab_contents.h"
#include "grit/browser_resources.h"
#include "grit/generated_resources.h"
#include "grit/theme_resources.h"
#include "ui/base/l10n/l10n_util.h"

namespace {

const char kStringsJsFile[] = "strings.js";
const char kSyncPromoJsFile[]  = "sync_promo.js";

// The maximum number of times we want to show the sync promo at startup.
const int kSyncPromoShowAtStartupMaxiumum = 10;

void RegisterSyncPromoPrefs(Profile* profile) {
  if (!profile->GetPrefs()->FindPreference(prefs::kSyncPromoStartupCount)) {
    profile->GetPrefs()->RegisterIntegerPref(
        prefs::kSyncPromoStartupCount, 0, PrefService::UNSYNCABLE_PREF);
  }
}

// The Web UI data source for the sync promo page.
class SyncPromoUIHTMLSource : public ChromeWebUIDataSource {
 public:
  SyncPromoUIHTMLSource();

 private:
  ~SyncPromoUIHTMLSource() {}
  DISALLOW_COPY_AND_ASSIGN(SyncPromoUIHTMLSource);
};

SyncPromoUIHTMLSource::SyncPromoUIHTMLSource()
    : ChromeWebUIDataSource(chrome::kChromeUISyncPromoHost) {
  DictionaryValue localized_strings;
  CoreOptionsHandler::GetStaticLocalizedValues(&localized_strings);
  SyncSetupHandler::GetStaticLocalizedValues(&localized_strings);
  AddLocalizedStrings(localized_strings);
}

}  // namespace

SyncPromoUI::SyncPromoUI(TabContents* contents) : ChromeWebUI(contents) {
  should_hide_url_ = true;

  SyncPromoHandler* handler = new SyncPromoHandler(
      g_browser_process->profile_manager());
  AddMessageHandler(handler);
  handler->Attach(this);

  // Set up the chrome://theme/ source.
  Profile* profile = Profile::FromBrowserContext(contents->browser_context());
  ThemeSource* theme = new ThemeSource(profile);
  profile->GetChromeURLDataManager()->AddDataSource(theme);

  // Set up the sync promo source.
  SyncPromoUIHTMLSource* html_source =
      new SyncPromoUIHTMLSource();
  html_source->set_json_path(kStringsJsFile);
  html_source->add_resource_path(kSyncPromoJsFile, IDR_SYNC_PROMO_JS);
  html_source->set_default_resource(IDR_SYNC_PROMO_HTML);
  profile->GetChromeURLDataManager()->AddDataSource(html_source);
}

bool SyncPromoUI::ShouldShowSyncPromo(Profile* profile) {
#if defined(OS_CHROMEOS)
  // There's no need to show the sync promo on cros since cros users are logged
  // into sync already.
  return false;
#endif

  // Honor the sync policies.
  if (!profile->GetOriginalProfile()->IsSyncAccessible())
    return false;

  // If the user is already signed into sync then don't show the promo.
  ProfileSyncService* service =
      profile->GetOriginalProfile()->GetProfileSyncService();
  if (!service || service->HasSyncSetupCompleted())
    return false;

  return true;
}

bool IsFirstRun(Profile* profile) {
  return FirstRun::IsChromeFirstRun();
}

bool SyncPromoUI::ShouldShowSyncPromoAtStartup(Profile* profile,
                                               bool is_new_profile) {
  if (!ShouldShowSyncPromo(profile))
    return false;

  const CommandLine& command_line  = *CommandLine::ForCurrentProcess();
  if (command_line.HasSwitch(switches::kNoFirstRun))
    is_new_profile = false;

  RegisterSyncPromoPrefs(profile);
  if (!is_new_profile) {
    if (!profile->GetPrefs()->HasPrefPath(prefs::kSyncPromoStartupCount))
      return false;
  }

  int show_count = profile->GetPrefs()->GetInteger(
      prefs::kSyncPromoStartupCount);
  return show_count < kSyncPromoShowAtStartupMaxiumum;
}

void SyncPromoUI::DidShowSyncPromoAtStartup(Profile* profile) {
  RegisterSyncPromoPrefs(profile);
  int show_count = profile->GetPrefs()->GetInteger(
      prefs::kSyncPromoStartupCount);
  show_count++;
  profile->GetPrefs()->SetInteger(prefs::kSyncPromoStartupCount, show_count);
}
