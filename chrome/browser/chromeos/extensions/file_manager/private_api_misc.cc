// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/extensions/file_manager/private_api_misc.h"

#include "apps/shell_window.h"
#include "apps/shell_window_registry.h"
#include "base/files/file_path.h"
#include "base/prefs/pref_service.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/chromeos/drive/file_system_util.h"
#include "chrome/browser/chromeos/drive/logging.h"
#include "chrome/browser/chromeos/extensions/file_manager/private_api_util.h"
#include "chrome/browser/chromeos/file_manager/app_installer.h"
#include "chrome/browser/chromeos/login/user_manager.h"
#include "chrome/browser/chromeos/settings/cros_settings.h"
#include "chrome/browser/lifetime/application_lifetime.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/signin/profile_oauth2_token_service.h"
#include "chrome/browser/signin/profile_oauth2_token_service_factory.h"
#include "chrome/browser/ui/ash/multi_user/multi_user_util.h"
#include "chrome/browser/ui/ash/multi_user/multi_user_window_manager.h"
#include "chrome/common/extensions/api/file_browser_private.h"
#include "chrome/common/pref_names.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/page_zoom.h"
#include "google_apis/drive/auth_service.h"
#include "url/gurl.h"

namespace extensions {

namespace {
const char kCWSScope[] = "https://www.googleapis.com/auth/chromewebstore";

std::vector<linked_ptr<api::file_browser_private::ProfileInfo> >
GetLoggedInProfileInfoList(Profile* current_profile) {
  DCHECK(chromeos::UserManager::IsInitialized());
  const std::vector<Profile*>& profiles =
      g_browser_process->profile_manager()->GetLoadedProfiles();
  std::set<Profile*> original_profiles;
  std::vector<linked_ptr<api::file_browser_private::ProfileInfo> >
      result_profiles;

  for (size_t i = 0; i < profiles.size(); ++i) {
    // Filter the profile.
    Profile* const profile = profiles[i]->GetOriginalProfile();
    if (original_profiles.count(profile))
      continue;
    original_profiles.insert(profile);
    const chromeos::User* const user =
        chromeos::UserManager::Get()->GetUserByProfile(profile);
    if (!user || !user->is_logged_in())
      continue;

    // Make a ProfileInfo.
    linked_ptr<api::file_browser_private::ProfileInfo> profile_info(
        new api::file_browser_private::ProfileInfo());
    profile_info->profile_id = multi_user_util::GetUserIDFromProfile(profile);
    profile_info->display_name = UTF16ToUTF8(user->GetDisplayName());
    profile_info->is_current_profile = true;
    result_profiles.push_back(profile_info);
  }

  return result_profiles;
}
} // namespace

bool FileBrowserPrivateLogoutUserForReauthenticationFunction::RunImpl() {
  chromeos::User* user =
      chromeos::UserManager::Get()->GetUserByProfile(GetProfile());
  if (user) {
    chromeos::UserManager::Get()->SaveUserOAuthStatus(
        user->email(),
        chromeos::User::OAUTH2_TOKEN_STATUS_INVALID);
  }

  chrome::AttemptUserExit();
  return true;
}

bool FileBrowserPrivateGetPreferencesFunction::RunImpl() {
  api::file_browser_private::Preferences result;
  const PrefService* const service = GetProfile()->GetPrefs();

  result.drive_enabled = drive::util::IsDriveEnabledForProfile(GetProfile());
  result.cellular_disabled =
      service->GetBoolean(prefs::kDisableDriveOverCellular);
  result.hosted_files_disabled =
      service->GetBoolean(prefs::kDisableDriveHostedFiles);
  result.use24hour_clock = service->GetBoolean(prefs::kUse24HourClock);
  result.allow_redeem_offers = true;
  if (!chromeos::CrosSettings::Get()->GetBoolean(
          chromeos::kAllowRedeemChromeOsRegistrationOffers,
          &result.allow_redeem_offers)) {
    result.allow_redeem_offers = true;
  }

  SetResult(result.ToValue().release());

  drive::util::Log(logging::LOG_INFO, "%s succeeded.", name().c_str());
  return true;
}

bool FileBrowserPrivateSetPreferencesFunction::RunImpl() {
  using extensions::api::file_browser_private::SetPreferences::Params;
  const scoped_ptr<Params> params(Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params);

  PrefService* const service = GetProfile()->GetPrefs();

  if (params->change_info.cellular_disabled)
    service->SetBoolean(prefs::kDisableDriveOverCellular,
                        *params->change_info.cellular_disabled);

  if (params->change_info.hosted_files_disabled)
    service->SetBoolean(prefs::kDisableDriveHostedFiles,
                        *params->change_info.hosted_files_disabled);

  drive::util::Log(logging::LOG_INFO, "%s succeeded.", name().c_str());
  return true;
}

FileBrowserPrivateZipSelectionFunction::
    FileBrowserPrivateZipSelectionFunction() {}

FileBrowserPrivateZipSelectionFunction::
    ~FileBrowserPrivateZipSelectionFunction() {}

bool FileBrowserPrivateZipSelectionFunction::RunImpl() {
  using extensions::api::file_browser_private::ZipSelection::Params;
  const scoped_ptr<Params> params(Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params);

  // First param is the source directory URL.
  if (params->dir_url.empty())
    return false;

  base::FilePath src_dir = file_manager::util::GetLocalPathFromURL(
      render_view_host(), GetProfile(), GURL(params->dir_url));
  if (src_dir.empty())
    return false;

  // Second param is the list of selected file URLs.
  if (params->selection_urls.empty())
    return false;

  std::vector<base::FilePath> files;
  for (size_t i = 0; i < params->selection_urls.size(); ++i) {
    base::FilePath path = file_manager::util::GetLocalPathFromURL(
        render_view_host(), GetProfile(), GURL(params->selection_urls[i]));
    if (path.empty())
      return false;
    files.push_back(path);
  }

  // Third param is the name of the output zip file.
  if (params->dest_name.empty())
    return false;

  // Check if the dir path is under Drive mount point.
  // TODO(hshi): support create zip file on Drive (crbug.com/158690).
  if (drive::util::IsUnderDriveMountPoint(src_dir))
    return false;

  base::FilePath dest_file = src_dir.Append(params->dest_name);
  std::vector<base::FilePath> src_relative_paths;
  for (size_t i = 0; i != files.size(); ++i) {
    const base::FilePath& file_path = files[i];

    // Obtain the relative path of |file_path| under |src_dir|.
    base::FilePath relative_path;
    if (!src_dir.AppendRelativePath(file_path, &relative_path))
      return false;
    src_relative_paths.push_back(relative_path);
  }

  zip_file_creator_ = new file_manager::ZipFileCreator(this,
                                                       src_dir,
                                                       src_relative_paths,
                                                       dest_file);

  // Keep the refcount until the zipping is complete on utility process.
  AddRef();

  zip_file_creator_->Start();
  return true;
}

void FileBrowserPrivateZipSelectionFunction::OnZipDone(bool success) {
  SetResult(new base::FundamentalValue(success));
  SendResponse(true);
  Release();
}

bool FileBrowserPrivateZoomFunction::RunImpl() {
  using extensions::api::file_browser_private::Zoom::Params;
  const scoped_ptr<Params> params(Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params);

  content::PageZoom zoom_type;
  switch (params->operation) {
    case api::file_browser_private::ZOOM_OPERATION_TYPE_IN:
      zoom_type = content::PAGE_ZOOM_IN;
      break;
    case api::file_browser_private::ZOOM_OPERATION_TYPE_OUT:
      zoom_type = content::PAGE_ZOOM_OUT;
      break;
    case api::file_browser_private::ZOOM_OPERATION_TYPE_RESET:
      zoom_type = content::PAGE_ZOOM_RESET;
      break;
    default:
      NOTREACHED();
      return false;
  }
  render_view_host()->Zoom(zoom_type);
  return true;
}

bool FileBrowserPrivateInstallWebstoreItemFunction::RunImpl() {
  using extensions::api::file_browser_private::InstallWebstoreItem::Params;
  const scoped_ptr<Params> params(Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params);

  if (params->item_id.empty())
    return false;

  const extensions::WebstoreStandaloneInstaller::Callback callback =
      base::Bind(
          &FileBrowserPrivateInstallWebstoreItemFunction::OnInstallComplete,
          this);

  scoped_refptr<file_manager::AppInstaller> installer(
      new file_manager::AppInstaller(
          GetAssociatedWebContents(),  // web_contents(),
          params->item_id,
          GetProfile(),
          callback));
  // installer will be AddRef()'d in BeginInstall().
  installer->BeginInstall();
  return true;
}

void FileBrowserPrivateInstallWebstoreItemFunction::OnInstallComplete(
    bool success,
    const std::string& error) {
  if (success) {
    drive::util::Log(logging::LOG_INFO,
                     "App install succeeded. (item id: %s)",
                     webstore_item_id_.c_str());
  } else {
    drive::util::Log(logging::LOG_ERROR,
                     "App install failed. (item id: %s, reason: %s)",
                     webstore_item_id_.c_str(),
                     error.c_str());
    error_ = error;
  }

  SendResponse(success);
}

FileBrowserPrivateRequestWebStoreAccessTokenFunction::
    FileBrowserPrivateRequestWebStoreAccessTokenFunction() {
}

FileBrowserPrivateRequestWebStoreAccessTokenFunction::
    ~FileBrowserPrivateRequestWebStoreAccessTokenFunction() {
}

bool FileBrowserPrivateRequestWebStoreAccessTokenFunction::RunImpl() {
  std::vector<std::string> scopes;
  scopes.push_back(kCWSScope);

  ProfileOAuth2TokenService* oauth_service =
      ProfileOAuth2TokenServiceFactory::GetForProfile(GetProfile());
  net::URLRequestContextGetter* url_request_context_getter =
      g_browser_process->system_request_context();

  if (!oauth_service) {
    drive::util::Log(logging::LOG_ERROR,
                     "CWS OAuth token fetch failed. OAuth2TokenService can't "
                     "be retrived.");
    SetResult(base::Value::CreateNullValue());
    return false;
  }

  auth_service_.reset(new google_apis::AuthService(
      oauth_service,
      oauth_service->GetPrimaryAccountId(),
      url_request_context_getter,
      scopes));
  auth_service_->StartAuthentication(base::Bind(
      &FileBrowserPrivateRequestWebStoreAccessTokenFunction::
          OnAccessTokenFetched,
      this));

  return true;
}

void FileBrowserPrivateRequestWebStoreAccessTokenFunction::OnAccessTokenFetched(
    google_apis::GDataErrorCode code,
    const std::string& access_token) {
  if (code == google_apis::HTTP_SUCCESS) {
    DCHECK(auth_service_->HasAccessToken());
    DCHECK(access_token == auth_service_->access_token());
    drive::util::Log(logging::LOG_INFO,
                     "CWS OAuth token fetch succeeded.");
    SetResult(new base::StringValue(access_token));
    SendResponse(true);
  } else {
    drive::util::Log(logging::LOG_ERROR,
                     "CWS OAuth token fetch failed. (GDataErrorCode: %s)",
                     google_apis::GDataErrorCodeToString(code).c_str());
    SetResult(base::Value::CreateNullValue());
    SendResponse(false);
  }
}

bool FileBrowserPrivateGetProfilesFunction::RunImpl() {
  results_ = api::file_browser_private::GetProfiles::Results::Create(
      GetLoggedInProfileInfoList(GetProfile()),
      "",  // TODO(hirono): Set the correct value.
      "");  // TODO(hriono): Set the correct value.
  return true;
}

bool FileBrowserPrivateVisitDesktopFunction::RunImpl() {
  using api::file_browser_private::VisitDesktop::Params;
  const scoped_ptr<Params> params(Params::Create(*args_));
  const std::vector<linked_ptr<api::file_browser_private::ProfileInfo> >&
      profiles = GetLoggedInProfileInfoList(GetProfile());

  // Check the multi-profile support.
  chrome::MultiUserWindowManager* const window_manager =
      chrome::MultiUserWindowManager::GetInstance();
  if (!window_manager) {
    SetError("Multi-profile support is not enabled.");
    return false;
  }

  // Check if the target user is logged-in or not.
  bool logged_in = false;
  for (size_t i = 0; i < profiles.size(); ++i) {
    if (profiles[i]->profile_id == params->profile_id) {
      logged_in = true;
      break;
    }
  }
  if (!logged_in) {
    SetError("The user is not logged-in now.");
    return false;
  }

  // Look for the current shell window.
  apps::ShellWindowRegistry* const shell_window_registry =
      apps::ShellWindowRegistry::Get(GetProfile());
  content::WebContents* const contents = GetAssociatedWebContents();
  content::RenderViewHost* const render_view_host =
      contents ? contents->GetRenderViewHost() : NULL;
  apps::ShellWindow* const shell_window = render_view_host ?
      shell_window_registry->GetShellWindowForRenderViewHost(render_view_host) :
      NULL;
  if (!shell_window) {
    SetError("Target window is not found.");
    return false;
  }

  // Move the window to the user's desktop.
  window_manager->ShowWindowForUser(
      shell_window->GetNativeWindow(),
      params->profile_id);

  // Check the result.
  if (!window_manager->IsWindowOnDesktopOfUser(
          shell_window->GetNativeWindow(),
          params->profile_id)) {
    SetError("The window cannot visit the desktop.");
    return false;
  }

  return true;
}

}  // namespace extensions
