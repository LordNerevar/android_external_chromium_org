// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/crashes_ui.h"

#include <vector>

#include "base/i18n/time_formatting.h"
#include "base/memory/ref_counted_memory.h"
#include "base/utf_string_conversions.h"
#include "base/values.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/crash_upload_list.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/chrome_url_data_manager.h"
#include "chrome/browser/ui/webui/chrome_web_ui_data_source.h"
#include "chrome/common/chrome_version_info.h"
#include "chrome/common/pref_names.h"
#include "chrome/common/url_constants.h"
#include "content/browser/tab_contents/tab_contents.h"
#include "grit/browser_resources.h"
#include "grit/chromium_strings.h"
#include "grit/generated_resources.h"
#include "grit/theme_resources.h"
#include "grit/theme_resources_standard.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"

#if defined(OS_CHROMEOS)
#include "chrome/browser/chromeos/user_cros_settings_provider.h"
#endif

namespace {

ChromeWebUIDataSource* CreateCrashesUIHTMLSource() {
  ChromeWebUIDataSource* source =
      new ChromeWebUIDataSource(chrome::kChromeUICrashesHost);

  source->AddLocalizedString("crashesTitle", IDS_CRASHES_TITLE);
  source->AddLocalizedString("crashCountFormat",
                             IDS_CRASHES_CRASH_COUNT_BANNER_FORMAT);
  source->AddLocalizedString("crashHeaderFormat",
                             IDS_CRASHES_CRASH_HEADER_FORMAT);
  source->AddLocalizedString("crashTimeFormat", IDS_CRASHES_CRASH_TIME_FORMAT);
  source->AddLocalizedString("bugLinkText", IDS_CRASHES_BUG_LINK_LABEL);
  source->AddLocalizedString("noCrashesMessage",
                             IDS_CRASHES_NO_CRASHES_MESSAGE);
  source->AddLocalizedString("disabledHeader", IDS_CRASHES_DISABLED_HEADER);
  source->AddLocalizedString("disabledMessage", IDS_CRASHES_DISABLED_MESSAGE);
  source->set_json_path("strings.js");
  source->add_resource_path("crashes.js", IDR_CRASHES_JS);
  source->set_default_resource(IDR_CRASHES_HTML);
  return source;
}

////////////////////////////////////////////////////////////////////////////////
//
// CrashesDOMHandler
//
////////////////////////////////////////////////////////////////////////////////

// The handler for Javascript messages for the chrome://crashes/ page.
class CrashesDOMHandler : public WebUIMessageHandler,
                          public CrashUploadList::Delegate {
 public:
  explicit CrashesDOMHandler();
  virtual ~CrashesDOMHandler();

  // WebUIMessageHandler implementation.
  virtual WebUIMessageHandler* Attach(WebUI* web_ui) OVERRIDE;
  virtual void RegisterMessages() OVERRIDE;

  // CrashUploadList::Delegate implemenation.
  virtual void OnCrashListAvailable() OVERRIDE;

 private:
  // Asynchronously fetches the list of crashes. Called from JS.
  void HandleRequestCrashes(const ListValue* args);

  // Sends the recent crashes list JS.
  void UpdateUI();

  scoped_refptr<CrashUploadList> upload_list_;
  bool list_available_;
  bool js_request_pending_;

  DISALLOW_COPY_AND_ASSIGN(CrashesDOMHandler);
};

CrashesDOMHandler::CrashesDOMHandler()
    : list_available_(false), js_request_pending_(false) {
  upload_list_ = CrashUploadList::Create(this);
}

CrashesDOMHandler::~CrashesDOMHandler() {
  upload_list_->ClearDelegate();
}

WebUIMessageHandler* CrashesDOMHandler::Attach(WebUI* web_ui) {
  upload_list_->LoadCrashListAsynchronously();
  return WebUIMessageHandler::Attach(web_ui);
}

void CrashesDOMHandler::RegisterMessages() {
  web_ui_->RegisterMessageCallback("requestCrashList",
      NewCallback(this, &CrashesDOMHandler::HandleRequestCrashes));
}

void CrashesDOMHandler::HandleRequestCrashes(const ListValue* args) {
  if (!CrashesUI::CrashReportingEnabled() || list_available_)
    UpdateUI();
  else
    js_request_pending_ = true;
}

void CrashesDOMHandler::OnCrashListAvailable() {
  list_available_ = true;
  if (js_request_pending_)
    UpdateUI();
}

void CrashesDOMHandler::UpdateUI() {
  bool crash_reporting_enabled = CrashesUI::CrashReportingEnabled();
  ListValue crash_list;

  if (crash_reporting_enabled) {
    std::vector<CrashUploadList::CrashInfo> crashes;
    upload_list_->GetUploadedCrashes(50, &crashes);

    for (std::vector<CrashUploadList::CrashInfo>::iterator i = crashes.begin();
         i != crashes.end(); ++i) {
      DictionaryValue* crash = new DictionaryValue();
      crash->SetString("id", i->crash_id);
      crash->SetString("time",
                       base::TimeFormatFriendlyDateAndTime(i->crash_time));
      crash_list.Append(crash);
    }
  }

  base::FundamentalValue enabled(crash_reporting_enabled);

  const chrome::VersionInfo version_info;
  base::StringValue version(version_info.Version());

  web_ui_->CallJavascriptFunction("updateCrashList", enabled, crash_list,
                                  version);
}

}  // namespace

///////////////////////////////////////////////////////////////////////////////
//
// CrashesUI
//
///////////////////////////////////////////////////////////////////////////////

CrashesUI::CrashesUI(TabContents* contents) : ChromeWebUI(contents) {
  AddMessageHandler((new CrashesDOMHandler())->Attach(this));

  // Set up the chrome://crashes/ source.
  Profile* profile = Profile::FromBrowserContext(contents->browser_context());
  profile->GetChromeURLDataManager()->AddDataSource(
      CreateCrashesUIHTMLSource());
}

// static
RefCountedMemory* CrashesUI::GetFaviconResourceBytes() {
  return ResourceBundle::GetSharedInstance().
      LoadDataResourceBytes(IDR_SAD_FAVICON);
}

// static
bool CrashesUI::CrashReportingEnabled() {
#if defined(GOOGLE_CHROME_BUILD) && !defined(OS_CHROMEOS)
  PrefService* prefs = g_browser_process->local_state();
  return prefs->GetBoolean(prefs::kMetricsReportingEnabled);
#elif defined(GOOGLE_CHROME_BUILD) && defined(OS_CHROMEOS)
  return chromeos::UserCrosSettingsProvider::cached_reporting_enabled();
#else
  return false;
#endif
}
