// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Use this class to authenticate users with Gaia and access cookies sent
// by the Gaia servers.  This class lives on the SyncEngine_AuthWatcherThread.
//
// Sample usage:
// GaiaAuthenticator gaia_auth("User-Agent", SYNC_SERVICE_NAME,
//     browser_sync::kExternalGaiaUrl);
// if (gaia_auth.Authenticate("email", "passwd", SAVE_IN_MEMORY_ONLY,
//                            true)) { // Synchronous
//   // Do something with: gaia_auth.auth_token(), or gaia_auth.sid(),
//   // or gaia_auth.lsid()
// }
//
// Credentials can also be preserved for subsequent requests, though these are
// saved in plain-text in memory, and not very secure on client systems. The
// email address associated with the Gaia account can be read; the password is
// write-only.

#ifndef CHROME_BROWSER_SYNC_ENGINE_NET_GAIA_AUTHENTICATOR_H_
#define CHROME_BROWSER_SYNC_ENGINE_NET_GAIA_AUTHENTICATOR_H_

#include <string>

#include "base/basictypes.h"
#include "base/message_loop.h"
#include "chrome/browser/sync/engine/net/http_return.h"
#include "chrome/browser/sync/util/event_sys.h"
#include "chrome/browser/sync/util/signin.h"
#include "googleurl/src/gurl.h"
#include "testing/gtest/include/gtest/gtest_prod.h"  // For FRIEND_TEST

namespace browser_sync {

static const char kGaiaUrl[] =
    "https://www.google.com:443/accounts/ClientLogin";

// Use of the following enum is odd. GaiaAuthenticator only looks at
// and DONT_SAVE_CREDENTIALS and SAVE_IN_MEMORY_ONLY (PERSIST_TO_DISK is == to
// SAVE_IN_MEMORY_ONLY for GaiaAuthenticator). The sync engine never uses
// DONT_SAVE_CREDENTIALS. AuthWatcher does look in GaiaAuthenticator's results
// object to decide if it should save credentials to disk. This currently
// works so I'm leaving the odd dance alone.

enum SaveCredentials {
  DONT_SAVE_CREDENTIALS,
  SAVE_IN_MEMORY_ONLY,
  PERSIST_TO_DISK // Saved in both memory and disk
};

// Error codes from Gaia. These will be set correctly for both Gaia V1
// (/ClientAuth) and V2 (/ClientLogin)
enum AuthenticationError {
  None                      = 0,
  BadAuthentication         = 1,
  NotVerified               = 2,
  TermsNotAgreed            = 3,
  Unknown                   = 4,
  AccountDeleted            = 5,
  AccountDisabled           = 6,
  CaptchaRequired           = 7,
  ServiceUnavailable        = 8,
  // Errors generated by this class not Gaia.
  CredentialsNotSet         = 9,
  ConnectionUnavailable     = 10
};

class GaiaAuthenticator;

struct GaiaAuthEvent {
  enum {
    GAIA_AUTH_FAILED,
    GAIA_AUTH_SUCCEEDED,
    GAIA_AUTHENTICATOR_DESTROYED
  }
  what_happened;
  AuthenticationError error;
  const GaiaAuthenticator* authenticator;

  // Lets us use GaiaAuthEvent as its own traits type in hookups.
  typedef GaiaAuthEvent EventType;
  static inline bool IsChannelShutdownEvent(const GaiaAuthEvent& event) {
    return event.what_happened == GAIA_AUTHENTICATOR_DESTROYED;
  }
};

// GaiaAuthenticator can be used to pass user credentials to Gaia and obtain
// cookies set by the Gaia servers.
class GaiaAuthenticator {
  FRIEND_TEST(GaiaAuthenticatorTest, TestNewlineAtEndOfAuthTokenRemoved);
 public:

  // Since GaiaAuthenticator can be used for any service, or by any client, you
  // must include a user-agent and a service-id when creating one. The
  // user_agent is a short string used for simple log analysis. gaia_url is used
  // to choose the server to authenticate with (e.g.
  // http://www.google.com/accounts/ClientLogin).
  GaiaAuthenticator(const std::string& user_agent,
                    const std::string& service_id,
                    const std::string& gaia_url);

  virtual ~GaiaAuthenticator();

  // This object should only be invoked from the AuthWatcherThread message
  // loop, which is injected here.
  void set_message_loop(const MessageLoop* loop) {
    message_loop_ = loop;
  }

  // Pass credentials to authenticate with, or use saved credentials via an
  // overload. If authentication succeeds, you can retrieve the authentication
  // token via the respective accessors. Returns a boolean indicating whether
  // authentication succeeded or not.
  bool Authenticate(const std::string& user_name, const std::string& password,
                    SaveCredentials should_save_credentials,
                    const std::string& captcha_token,
                    const std::string& captcha_value,
                    SignIn try_first);

  bool Authenticate(const std::string& user_name, const std::string& password,
                    SaveCredentials should_save_credentials,
                    SignIn try_first);

  // Resets all stored cookies to their default values.
  void ResetCredentials();

  void SetUsernamePassword(const std::string& username,
                           const std::string& password);

  void SetUsername(const std::string& username);

  void SetAuthToken(const std::string& auth_token, SaveCredentials);

  struct AuthResults {
    SaveCredentials credentials_saved;
    std::string email;
    std::string password;

    // Fields that store various cookies.
    std::string sid;
    std::string lsid;
    std::string auth_token;

    std::string primary_email;

    // Fields for items returned when authentication fails.
    std::string error_msg;
    enum AuthenticationError auth_error;
    std::string auth_error_url;
    std::string captcha_token;
    std::string captcha_url;
    SignIn signin;

    AuthResults () : credentials_saved(DONT_SAVE_CREDENTIALS),
                     auth_error(None) { }
  };

 protected:

  struct AuthParams {
    GaiaAuthenticator* authenticator;
    uint32 request_id;
    SaveCredentials should_save_credentials;
    std::string email;
    std::string password;
    std::string captcha_token;
    std::string captcha_value;
    SignIn try_first;
  };

  // mutex_ must be entered before calling this function.
  AuthParams MakeParams(const std::string& user_name,
                        const std::string& password,
                        SaveCredentials should_save_credentials,
                        const std::string& captcha_token,
                        const std::string& captcha_value,
                        SignIn try_first);

  // The real Authenticate implementations.
  bool AuthenticateImpl(const AuthParams& params);
  bool AuthenticateImpl(const AuthParams& params, AuthResults* results);

  // virtual for testing purposes.
  virtual bool PerformGaiaRequest(const AuthParams& params,
                                  AuthResults* results);
  virtual bool Post(const GURL& url, const std::string& post_body,
                    unsigned long* response_code, std::string* response_body) {
    return false;
  }

  // Caller should fill in results->LSID before calling. Result in
  // results->primary_email.
  virtual bool LookupEmail(AuthResults* results);

 public:
  // Retrieve email.
  inline std::string email() const {
    DCHECK_EQ(MessageLoop::current(), message_loop_);
    return auth_results_.email;
  }

  // Retrieve password.
  inline std::string password() const {
    DCHECK_EQ(MessageLoop::current(), message_loop_);
    return auth_results_.password;
  }

  // Retrieve AuthToken, if previously authenticated; otherwise returns "".
  inline std::string auth_token() const {
    DCHECK_EQ(MessageLoop::current(), message_loop_);
    return auth_results_.auth_token;
  }

  // Retrieve SID cookie. For details, see the Google Accounts documentation.
  inline std::string sid() const {
    DCHECK_EQ(MessageLoop::current(), message_loop_);
    return auth_results_.sid;
  }

  // Retrieve LSID cookie. For details, see the Google Accounts documentation.
  inline std::string lsid() const {
    DCHECK_EQ(MessageLoop::current(), message_loop_);
    return auth_results_.lsid;
  }

  // Get last authentication error.
  inline enum AuthenticationError auth_error() const {
    DCHECK_EQ(MessageLoop::current(), message_loop_);
    return auth_results_.auth_error;
  }

  inline std::string auth_error_url() const {
    DCHECK_EQ(MessageLoop::current(), message_loop_);
    return auth_results_.auth_error_url;
  }

  inline std::string captcha_token() const {
    DCHECK_EQ(MessageLoop::current(), message_loop_);
    return auth_results_.captcha_token;
  }

  inline std::string captcha_url() const {
    DCHECK_EQ(MessageLoop::current(), message_loop_);
    return auth_results_.captcha_url;
  }

  inline AuthResults results() const {
    DCHECK_EQ(MessageLoop::current(), message_loop_);
    return auth_results_;
  }

  typedef EventChannel<GaiaAuthEvent, Lock> Channel;

  inline Channel* channel() const {
    return channel_;
  }

 private:
  bool IssueAuthToken(AuthResults* results, const std::string& service_id,
                      bool long_lived_token);

  // Helper method to parse response when authentication succeeds.
  void ExtractTokensFrom(const std::string& response, AuthResults* results);
  // Helper method to parse response when authentication fails.
  void ExtractAuthErrorFrom(const std::string& response, AuthResults* results);

  // Fields for the obvious data items.
  const std::string user_agent_;
  const std::string service_id_;
  const std::string gaia_url_;

  AuthResults auth_results_;

  // When multiple async requests are running, only the one that started most
  // recently updates the values.
  //
  // Note that even though this code was written to handle multiple requests
  // simultaneously, the sync code issues auth requests one at a time.
  uint32 request_count_;

  Channel* channel_;

  // Used to compute backoff time for next allowed authentication.
  int delay_;  // In seconds.
  time_t next_allowed_auth_attempt_time_;
  int early_auth_attempt_count_;

  // The message loop all our methods are invoked on.  Generally this is the
  // SyncEngine_AuthWatcherThread's message loop.
  const MessageLoop* message_loop_;
};

}  // namespace browser_sync

#endif  // CHROME_BROWSER_SYNC_ENGINE_NET_GAIA_AUTHENTICATOR_H_
