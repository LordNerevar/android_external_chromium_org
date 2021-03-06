// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/identity/identity_api.h"

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "base/lazy_instance.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/values.h"
#include "chrome/browser/app_mode/app_mode_utils.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/extensions/extension_function_dispatcher.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/extension_system.h"
#include "chrome/browser/policy/browser_policy_connector.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/signin/profile_oauth2_token_service.h"
#include "chrome/browser/signin/profile_oauth2_token_service_factory.h"
#include "chrome/browser/signin/signin_global_error.h"
#include "chrome/common/extensions/api/identity.h"
#include "chrome/common/extensions/api/identity/oauth2_manifest_handler.h"
#include "chrome/common/pref_names.h"
#include "chrome/common/url_constants.h"
#include "extensions/browser/event_router.h"
#include "extensions/common/extension.h"
#include "google_apis/gaia/gaia_urls.h"
#include "url/gurl.h"

#if defined(OS_CHROMEOS)
#include "chrome/browser/chromeos/login/user_manager.h"
#include "chrome/browser/chromeos/settings/device_oauth2_token_service.h"
#include "chrome/browser/chromeos/settings/device_oauth2_token_service_factory.h"
#include "google_apis/gaia/gaia_constants.h"
#endif

namespace extensions {

namespace identity_constants {
const char kInvalidClientId[] = "Invalid OAuth2 Client ID.";
const char kInvalidScopes[] = "Invalid OAuth2 scopes.";
const char kAuthFailure[] = "OAuth2 request failed: ";
const char kNoGrant[] = "OAuth2 not granted or revoked.";
const char kUserRejected[] = "The user did not approve access.";
const char kUserNotSignedIn[] = "The user is not signed in.";
const char kInteractionRequired[] = "User interaction required.";
const char kInvalidRedirect[] = "Did not redirect to the right URL.";
const char kOffTheRecord[] = "Identity API is disabled in incognito windows.";
const char kPageLoadFailure[] = "Authorization page could not be loaded.";

const int kCachedIssueAdviceTTLSeconds = 1;
}  // namespace identity_constants

namespace {

static const char kChromiumDomainRedirectUrlPattern[] =
    "https://%s.chromiumapp.org/";

}  // namespace

namespace identity = api::identity;

IdentityGetAuthTokenFunction::IdentityGetAuthTokenFunction()
    : should_prompt_for_scopes_(false),
      should_prompt_for_signin_(false) {}

IdentityGetAuthTokenFunction::~IdentityGetAuthTokenFunction() {}

bool IdentityGetAuthTokenFunction::RunImpl() {
  if (GetProfile()->IsOffTheRecord()) {
    error_ = identity_constants::kOffTheRecord;
    return false;
  }

  scoped_ptr<identity::GetAuthToken::Params> params(
      identity::GetAuthToken::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());
  bool interactive = params->details.get() &&
      params->details->interactive.get() &&
      *params->details->interactive;

  should_prompt_for_scopes_ = interactive;
  should_prompt_for_signin_ = interactive;

  const OAuth2Info& oauth2_info = OAuth2Info::GetOAuth2Info(GetExtension());

  // Check that the necessary information is present in the manifest.
  oauth2_client_id_ = GetOAuth2ClientId();
  if (oauth2_client_id_.empty()) {
    error_ = identity_constants::kInvalidClientId;
    return false;
  }

  if (oauth2_info.scopes.size() == 0) {
    error_ = identity_constants::kInvalidScopes;
    return false;
  }

  ProfileOAuth2TokenService* token_service =
      ProfileOAuth2TokenServiceFactory::GetForProfile(GetProfile());

  std::set<std::string> scopes(oauth2_info.scopes.begin(),
                               oauth2_info.scopes.end());
  token_key_.reset(new ExtensionTokenKey(
      GetExtension()->id(), token_service->GetPrimaryAccountId(), scopes));

  // Balanced in CompleteFunctionWithResult|CompleteFunctionWithError
  AddRef();

#if defined(OS_CHROMEOS)
  if (chromeos::UserManager::Get()->IsLoggedInAsKioskApp() &&
      g_browser_process->browser_policy_connector()->IsEnterpriseManaged()) {
    StartMintTokenFlow(IdentityMintRequestQueue::MINT_TYPE_NONINTERACTIVE);
    return true;
  }
#endif

  if (!HasLoginToken()) {
    if (!should_prompt_for_signin_) {
      error_ = identity_constants::kUserNotSignedIn;
      Release();
      return false;
    }
    // Display a login prompt.
    StartSigninFlow();
  } else {
    StartMintTokenFlow(IdentityMintRequestQueue::MINT_TYPE_NONINTERACTIVE);
  }

  return true;
}

void IdentityGetAuthTokenFunction::CompleteFunctionWithResult(
    const std::string& access_token) {
  SetResult(new base::StringValue(access_token));
  SendResponse(true);
  Release();  // Balanced in RunImpl.
}

void IdentityGetAuthTokenFunction::CompleteFunctionWithError(
    const std::string& error) {
  error_ = error;
  SendResponse(false);
  Release();  // Balanced in RunImpl.
}

void IdentityGetAuthTokenFunction::StartSigninFlow() {
  // All cached tokens are invalid because the user is not signed in.
  IdentityAPI* id_api =
      extensions::IdentityAPI::GetFactoryInstance()->GetForProfile(
          GetProfile());
  id_api->EraseAllCachedTokens();
  // Display a login prompt. If the subsequent mint fails, don't display the
  // login prompt again.
  should_prompt_for_signin_ = false;
  ShowLoginPopup();
}

void IdentityGetAuthTokenFunction::StartMintTokenFlow(
    IdentityMintRequestQueue::MintType type) {
  mint_token_flow_type_ = type;

  // Flows are serialized to prevent excessive traffic to GAIA, and
  // to consolidate UI pop-ups.
  IdentityAPI* id_api =
      extensions::IdentityAPI::GetFactoryInstance()->GetForProfile(
          GetProfile());

  if (!should_prompt_for_scopes_) {
    // Caller requested no interaction.

    if (type == IdentityMintRequestQueue::MINT_TYPE_INTERACTIVE) {
      // GAIA told us to do a consent UI.
      CompleteFunctionWithError(identity_constants::kNoGrant);
      return;
    }
    if (!id_api->mint_queue()->empty(
            IdentityMintRequestQueue::MINT_TYPE_INTERACTIVE, *token_key_)) {
      // Another call is going through a consent UI.
      CompleteFunctionWithError(identity_constants::kNoGrant);
      return;
    }
  }
  id_api->mint_queue()->RequestStart(type, *token_key_, this);
}

void IdentityGetAuthTokenFunction::CompleteMintTokenFlow() {
  IdentityMintRequestQueue::MintType type = mint_token_flow_type_;

  const OAuth2Info& oauth2_info = OAuth2Info::GetOAuth2Info(GetExtension());
  std::set<std::string> scopes(oauth2_info.scopes.begin(),
                               oauth2_info.scopes.end());

  extensions::IdentityAPI::GetFactoryInstance()
      ->GetForProfile(GetProfile())
      ->mint_queue()
      ->RequestComplete(type, *token_key_, this);
}

void IdentityGetAuthTokenFunction::StartMintToken(
    IdentityMintRequestQueue::MintType type) {
  const OAuth2Info& oauth2_info = OAuth2Info::GetOAuth2Info(GetExtension());
  IdentityAPI* id_api =
      IdentityAPI::GetFactoryInstance()->GetForProfile(GetProfile());
  IdentityTokenCacheValue cache_entry = id_api->GetCachedToken(*token_key_);
  IdentityTokenCacheValue::CacheValueStatus cache_status =
      cache_entry.status();

  if (type == IdentityMintRequestQueue::MINT_TYPE_NONINTERACTIVE) {
    switch (cache_status) {
      case IdentityTokenCacheValue::CACHE_STATUS_NOTFOUND:
#if defined(OS_CHROMEOS)
        // Always force minting token for ChromeOS kiosk app.
        if (chromeos::UserManager::Get()->IsLoggedInAsKioskApp()) {
          gaia_mint_token_mode_ = OAuth2MintTokenFlow::MODE_MINT_TOKEN_FORCE;
          if (g_browser_process->browser_policy_connector()->
                  IsEnterpriseManaged()) {
            StartDeviceLoginAccessTokenRequest();
          } else {
            StartLoginAccessTokenRequest();
          }
          return;
        }
#endif

        if (oauth2_info.auto_approve)
          // oauth2_info.auto_approve is protected by a whitelist in
          // _manifest_features.json hence only selected extensions take
          // advantage of forcefully minting the token.
          gaia_mint_token_mode_ = OAuth2MintTokenFlow::MODE_MINT_TOKEN_FORCE;
        else
          gaia_mint_token_mode_ = OAuth2MintTokenFlow::MODE_MINT_TOKEN_NO_FORCE;
        StartLoginAccessTokenRequest();
        break;

      case IdentityTokenCacheValue::CACHE_STATUS_TOKEN:
        CompleteMintTokenFlow();
        CompleteFunctionWithResult(cache_entry.token());
        break;

      case IdentityTokenCacheValue::CACHE_STATUS_ADVICE:
        CompleteMintTokenFlow();
        should_prompt_for_signin_ = false;
        issue_advice_ = cache_entry.issue_advice();
        StartMintTokenFlow(IdentityMintRequestQueue::MINT_TYPE_INTERACTIVE);
        break;
    }
  } else {
    DCHECK(type == IdentityMintRequestQueue::MINT_TYPE_INTERACTIVE);

    if (cache_status == IdentityTokenCacheValue::CACHE_STATUS_TOKEN) {
      CompleteMintTokenFlow();
      CompleteFunctionWithResult(cache_entry.token());
    } else {
      ShowOAuthApprovalDialog(issue_advice_);
    }
  }
}

void IdentityGetAuthTokenFunction::OnMintTokenSuccess(
    const std::string& access_token, int time_to_live) {
  IdentityTokenCacheValue token(access_token,
                                base::TimeDelta::FromSeconds(time_to_live));
  IdentityAPI::GetFactoryInstance()
      ->GetForProfile(GetProfile())
      ->SetCachedToken(*token_key_, token);

  CompleteMintTokenFlow();
  CompleteFunctionWithResult(access_token);
}

void IdentityGetAuthTokenFunction::OnMintTokenFailure(
    const GoogleServiceAuthError& error) {
  CompleteMintTokenFlow();

  switch (error.state()) {
    case GoogleServiceAuthError::INVALID_GAIA_CREDENTIALS:
    case GoogleServiceAuthError::SERVICE_ERROR:
    case GoogleServiceAuthError::ACCOUNT_DELETED:
    case GoogleServiceAuthError::ACCOUNT_DISABLED:
      extensions::IdentityAPI::GetFactoryInstance()
          ->GetForProfile(GetProfile())
          ->ReportAuthError(error);
      if (should_prompt_for_signin_) {
        // Display a login prompt and try again (once).
        StartSigninFlow();
        return;
      }
      break;
    default:
      // Return error to caller.
      break;
  }

  CompleteFunctionWithError(
      std::string(identity_constants::kAuthFailure) + error.ToString());
}

void IdentityGetAuthTokenFunction::OnIssueAdviceSuccess(
    const IssueAdviceInfo& issue_advice) {
  IdentityAPI::GetFactoryInstance()
      ->GetForProfile(GetProfile())
      ->SetCachedToken(*token_key_,
                       IdentityTokenCacheValue(issue_advice));
  CompleteMintTokenFlow();

  should_prompt_for_signin_ = false;
  // Existing grant was revoked and we used NO_FORCE, so we got info back
  // instead. Start a consent UI if we can.
  issue_advice_ = issue_advice;
  StartMintTokenFlow(IdentityMintRequestQueue::MINT_TYPE_INTERACTIVE);
}

void IdentityGetAuthTokenFunction::SigninSuccess() {
  StartMintTokenFlow(IdentityMintRequestQueue::MINT_TYPE_NONINTERACTIVE);
}

void IdentityGetAuthTokenFunction::SigninFailed() {
  CompleteFunctionWithError(identity_constants::kUserNotSignedIn);
}

void IdentityGetAuthTokenFunction::OnGaiaFlowFailure(
    GaiaWebAuthFlow::Failure failure,
    GoogleServiceAuthError service_error,
    const std::string& oauth_error) {
  CompleteMintTokenFlow();
  std::string error;

  switch (failure) {
    case GaiaWebAuthFlow::WINDOW_CLOSED:
      error = identity_constants::kUserRejected;
      break;

    case GaiaWebAuthFlow::INVALID_REDIRECT:
      error = identity_constants::kInvalidRedirect;
      break;

    case GaiaWebAuthFlow::SERVICE_AUTH_ERROR:
      error = std::string(identity_constants::kAuthFailure) +
          service_error.ToString();
      break;

    case GaiaWebAuthFlow::OAUTH_ERROR:
      error = MapOAuth2ErrorToDescription(oauth_error);
      break;

    case GaiaWebAuthFlow::LOAD_FAILED:
      error = identity_constants::kPageLoadFailure;
      break;

    default:
      NOTREACHED() << "Unexpected error from gaia web auth flow: " << failure;
      error = identity_constants::kInvalidRedirect;
      break;
  }

  CompleteFunctionWithError(error);
}

void IdentityGetAuthTokenFunction::OnGaiaFlowCompleted(
    const std::string& access_token,
    const std::string& expiration) {

  int time_to_live;
  if (!expiration.empty() && base::StringToInt(expiration, &time_to_live)) {
    IdentityTokenCacheValue token_value(
        access_token, base::TimeDelta::FromSeconds(time_to_live));
    IdentityAPI::GetFactoryInstance()
        ->GetForProfile(GetProfile())
        ->SetCachedToken(*token_key_, token_value);
  }

  CompleteMintTokenFlow();
  CompleteFunctionWithResult(access_token);
}

void IdentityGetAuthTokenFunction::OnGetTokenSuccess(
    const OAuth2TokenService::Request* request,
    const std::string& access_token,
    const base::Time& expiration_time) {
  login_token_request_.reset();
  StartGaiaRequest(access_token);
}

void IdentityGetAuthTokenFunction::OnGetTokenFailure(
    const OAuth2TokenService::Request* request,
    const GoogleServiceAuthError& error) {
  login_token_request_.reset();
  OnGaiaFlowFailure(GaiaWebAuthFlow::SERVICE_AUTH_ERROR, error, std::string());
}

#if defined(OS_CHROMEOS)
void IdentityGetAuthTokenFunction::StartDeviceLoginAccessTokenRequest() {
  chromeos::DeviceOAuth2TokenServiceFactory::Get(
      base::Bind(&IdentityGetAuthTokenFunction::DidGetTokenService,
                 this));
}

void IdentityGetAuthTokenFunction::DidGetTokenService(
    chromeos::DeviceOAuth2TokenService* service) {
  if (!service) {
    CompleteFunctionWithError(identity_constants::kUserNotSignedIn);
    return;
  }
  // Since robot account refresh tokens are scoped down to [any-api] only,
  // request access token for [any-api] instead of login.
  OAuth2TokenService::ScopeSet scopes;
  scopes.insert(GaiaConstants::kAnyApiOAuth2Scope);
  login_token_request_ =
      service->StartRequest(service->GetRobotAccountId(),
                            scopes,
                            this);
}
#endif

void IdentityGetAuthTokenFunction::StartLoginAccessTokenRequest() {
  ProfileOAuth2TokenService* service =
      ProfileOAuth2TokenServiceFactory::GetForProfile(GetProfile());
#if defined(OS_CHROMEOS)
  if (chrome::IsRunningInForcedAppMode()) {
    std::string app_client_id;
    std::string app_client_secret;
    if (chromeos::UserManager::Get()->GetAppModeChromeClientOAuthInfo(
           &app_client_id, &app_client_secret)) {
      login_token_request_ =
          service->StartRequestForClient(service->GetPrimaryAccountId(),
                                         app_client_id,
                                         app_client_secret,
                                         OAuth2TokenService::ScopeSet(),
                                         this);
      return;
    }
  }
#endif
  login_token_request_ = service->StartRequest(
      service->GetPrimaryAccountId(), OAuth2TokenService::ScopeSet(), this);
}

void IdentityGetAuthTokenFunction::StartGaiaRequest(
    const std::string& login_access_token) {
  DCHECK(!login_access_token.empty());
  mint_token_flow_.reset(CreateMintTokenFlow(login_access_token));
  mint_token_flow_->Start();
}

void IdentityGetAuthTokenFunction::ShowLoginPopup() {
  signin_flow_.reset(new IdentitySigninFlow(this, GetProfile()));
  signin_flow_->Start();
}

void IdentityGetAuthTokenFunction::ShowOAuthApprovalDialog(
    const IssueAdviceInfo& issue_advice) {
  const OAuth2Info& oauth2_info = OAuth2Info::GetOAuth2Info(GetExtension());
  const std::string locale = g_browser_process->local_state()->GetString(
      prefs::kApplicationLocale);

  gaia_web_auth_flow_.reset(new GaiaWebAuthFlow(
      this, GetProfile(), GetExtension()->id(), oauth2_info, locale));
  gaia_web_auth_flow_->Start();
}

OAuth2MintTokenFlow* IdentityGetAuthTokenFunction::CreateMintTokenFlow(
    const std::string& login_access_token) {
  const OAuth2Info& oauth2_info = OAuth2Info::GetOAuth2Info(GetExtension());

  OAuth2MintTokenFlow* mint_token_flow = new OAuth2MintTokenFlow(
      GetProfile()->GetRequestContext(),
      this,
      OAuth2MintTokenFlow::Parameters(login_access_token,
                                      GetExtension()->id(),
                                      oauth2_client_id_,
                                      oauth2_info.scopes,
                                      gaia_mint_token_mode_));
  return mint_token_flow;
}

bool IdentityGetAuthTokenFunction::HasLoginToken() const {
  ProfileOAuth2TokenService* token_service =
      ProfileOAuth2TokenServiceFactory::GetForProfile(GetProfile());
  return token_service->RefreshTokenIsAvailable(
      token_service->GetPrimaryAccountId());
}

std::string IdentityGetAuthTokenFunction::MapOAuth2ErrorToDescription(
    const std::string& error) {
  const char kOAuth2ErrorAccessDenied[] = "access_denied";
  const char kOAuth2ErrorInvalidScope[] = "invalid_scope";

  if (error == kOAuth2ErrorAccessDenied)
    return std::string(identity_constants::kUserRejected);
  else if (error == kOAuth2ErrorInvalidScope)
    return std::string(identity_constants::kInvalidScopes);
  else
    return std::string(identity_constants::kAuthFailure) + error;
}

std::string IdentityGetAuthTokenFunction::GetOAuth2ClientId() const {
  const OAuth2Info& oauth2_info = OAuth2Info::GetOAuth2Info(GetExtension());
  std::string client_id = oauth2_info.client_id;

  // Component apps using auto_approve may use Chrome's client ID by
  // omitting the field.
  if (client_id.empty() && GetExtension()->location() == Manifest::COMPONENT &&
      oauth2_info.auto_approve) {
    client_id = GaiaUrls::GetInstance()->oauth2_chrome_client_id();
  }
  return client_id;
}

IdentityRemoveCachedAuthTokenFunction::IdentityRemoveCachedAuthTokenFunction() {
}

IdentityRemoveCachedAuthTokenFunction::
    ~IdentityRemoveCachedAuthTokenFunction() {
}

bool IdentityRemoveCachedAuthTokenFunction::RunImpl() {
  if (GetProfile()->IsOffTheRecord()) {
    error_ = identity_constants::kOffTheRecord;
    return false;
  }

  scoped_ptr<identity::RemoveCachedAuthToken::Params> params(
      identity::RemoveCachedAuthToken::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());
  IdentityAPI::GetFactoryInstance()
      ->GetForProfile(GetProfile())
      ->EraseCachedToken(GetExtension()->id(), params->details.token);
  return true;
}

IdentityLaunchWebAuthFlowFunction::IdentityLaunchWebAuthFlowFunction() {}

IdentityLaunchWebAuthFlowFunction::~IdentityLaunchWebAuthFlowFunction() {
  if (auth_flow_)
    auth_flow_.release()->DetachDelegateAndDelete();
}

bool IdentityLaunchWebAuthFlowFunction::RunImpl() {
  if (GetProfile()->IsOffTheRecord()) {
    error_ = identity_constants::kOffTheRecord;
    return false;
  }

  scoped_ptr<identity::LaunchWebAuthFlow::Params> params(
      identity::LaunchWebAuthFlow::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  GURL auth_url(params->details.url);
  WebAuthFlow::Mode mode =
      params->details.interactive && *params->details.interactive ?
      WebAuthFlow::INTERACTIVE : WebAuthFlow::SILENT;

  // Set up acceptable target URLs. (Does not include chrome-extension
  // scheme for this version of the API.)
  InitFinalRedirectURLPrefix(GetExtension()->id());

  AddRef();  // Balanced in OnAuthFlowSuccess/Failure.

  auth_flow_.reset(new WebAuthFlow(this, GetProfile(), auth_url, mode));
  auth_flow_->Start();
  return true;
}

void IdentityLaunchWebAuthFlowFunction::InitFinalRedirectURLPrefixForTest(
    const std::string& extension_id) {
  InitFinalRedirectURLPrefix(extension_id);
}

void IdentityLaunchWebAuthFlowFunction::InitFinalRedirectURLPrefix(
    const std::string& extension_id) {
  if (final_url_prefix_.is_empty()) {
    final_url_prefix_ = GURL(base::StringPrintf(
        kChromiumDomainRedirectUrlPattern, extension_id.c_str()));
  }
}

void IdentityLaunchWebAuthFlowFunction::OnAuthFlowFailure(
    WebAuthFlow::Failure failure) {
  switch (failure) {
    case WebAuthFlow::WINDOW_CLOSED:
      error_ = identity_constants::kUserRejected;
      break;
    case WebAuthFlow::INTERACTION_REQUIRED:
      error_ = identity_constants::kInteractionRequired;
      break;
    case WebAuthFlow::LOAD_FAILED:
      error_ = identity_constants::kPageLoadFailure;
      break;
    default:
      NOTREACHED() << "Unexpected error from web auth flow: " << failure;
      error_ = identity_constants::kInvalidRedirect;
      break;
  }
  SendResponse(false);
  Release();  // Balanced in RunImpl.
}

void IdentityLaunchWebAuthFlowFunction::OnAuthFlowURLChange(
    const GURL& redirect_url) {
  if (redirect_url.GetWithEmptyPath() == final_url_prefix_) {
    SetResult(new base::StringValue(redirect_url.spec()));
    SendResponse(true);
    Release();  // Balanced in RunImpl.
  }
}

IdentityTokenCacheValue::IdentityTokenCacheValue()
    : status_(CACHE_STATUS_NOTFOUND) {
}

IdentityTokenCacheValue::IdentityTokenCacheValue(
    const IssueAdviceInfo& issue_advice) : status_(CACHE_STATUS_ADVICE),
                                           issue_advice_(issue_advice) {
  expiration_time_ = base::Time::Now() + base::TimeDelta::FromSeconds(
      identity_constants::kCachedIssueAdviceTTLSeconds);
}

IdentityTokenCacheValue::IdentityTokenCacheValue(
    const std::string& token, base::TimeDelta time_to_live)
    : status_(CACHE_STATUS_TOKEN),
      token_(token) {
  // Remove 20 minutes from the ttl so cached tokens will have some time
  // to live any time they are returned.
  time_to_live -= base::TimeDelta::FromMinutes(20);

  base::TimeDelta zero_delta;
  if (time_to_live < zero_delta)
    time_to_live = zero_delta;

  expiration_time_ = base::Time::Now() + time_to_live;
}

IdentityTokenCacheValue::~IdentityTokenCacheValue() {
}

IdentityTokenCacheValue::CacheValueStatus
    IdentityTokenCacheValue::status() const {
  if (is_expired())
    return IdentityTokenCacheValue::CACHE_STATUS_NOTFOUND;
  else
    return status_;
}

const IssueAdviceInfo& IdentityTokenCacheValue::issue_advice() const {
  return issue_advice_;
}

const std::string& IdentityTokenCacheValue::token() const {
  return token_;
}

bool IdentityTokenCacheValue::is_expired() const {
  return status_ == CACHE_STATUS_NOTFOUND ||
      expiration_time_ < base::Time::Now();
}

const base::Time& IdentityTokenCacheValue::expiration_time() const {
  return expiration_time_;
}

IdentityAPI::IdentityAPI(Profile* profile)
    : profile_(profile),
      account_tracker_(profile) {
  account_tracker_.AddObserver(this);
}

IdentityAPI::~IdentityAPI() {}

IdentityMintRequestQueue* IdentityAPI::mint_queue() {
    return &mint_queue_;
}

void IdentityAPI::SetCachedToken(const ExtensionTokenKey& key,
                                 const IdentityTokenCacheValue& token_data) {
  CachedTokens::iterator it = token_cache_.find(key);
  if (it != token_cache_.end() && it->second.status() <= token_data.status())
    token_cache_.erase(it);

  token_cache_.insert(std::make_pair(key, token_data));
}

void IdentityAPI::EraseCachedToken(const std::string& extension_id,
                                   const std::string& token) {
  CachedTokens::iterator it;
  for (it = token_cache_.begin(); it != token_cache_.end(); ++it) {
    if (it->first.extension_id == extension_id &&
        it->second.status() == IdentityTokenCacheValue::CACHE_STATUS_TOKEN &&
        it->second.token() == token) {
      token_cache_.erase(it);
      break;
    }
  }
}

void IdentityAPI::EraseAllCachedTokens() {
  token_cache_.clear();
}

const IdentityTokenCacheValue& IdentityAPI::GetCachedToken(
    const ExtensionTokenKey& key) {
  return token_cache_[key];
}

const IdentityAPI::CachedTokens& IdentityAPI::GetAllCachedTokens() {
  return token_cache_;
}

void IdentityAPI::ReportAuthError(const GoogleServiceAuthError& error) {
  ProfileOAuth2TokenService* token_service =
      ProfileOAuth2TokenServiceFactory::GetForProfile(profile_);
  account_tracker_.ReportAuthError(token_service->GetPrimaryAccountId(), error);
}

void IdentityAPI::Shutdown() {
  account_tracker_.RemoveObserver(this);
  account_tracker_.Shutdown();
}

static base::LazyInstance<ProfileKeyedAPIFactory<IdentityAPI> >
    g_factory = LAZY_INSTANCE_INITIALIZER;

// static
ProfileKeyedAPIFactory<IdentityAPI>* IdentityAPI::GetFactoryInstance() {
  return &g_factory.Get();
}

void IdentityAPI::OnAccountAdded(const AccountIds& ids) {}

void IdentityAPI::OnAccountRemoved(const AccountIds& ids) {}

void IdentityAPI::OnAccountSignInChanged(const AccountIds& ids,
                                         bool is_signed_in) {
  api::identity::AccountInfo account_info;
  account_info.id = ids.gaia;

  scoped_ptr<base::ListValue> args =
      api::identity::OnSignInChanged::Create(account_info, is_signed_in);
  scoped_ptr<Event> event(new Event(
      api::identity::OnSignInChanged::kEventName, args.Pass(), profile_));

  ExtensionSystem::Get(profile_)->event_router()->BroadcastEvent(event.Pass());
}

template <>
void ProfileKeyedAPIFactory<IdentityAPI>::DeclareFactoryDependencies() {
  DependsOn(ExtensionSystemFactory::GetInstance());
  DependsOn(ProfileOAuth2TokenServiceFactory::GetInstance());
}

}  // namespace extensions
