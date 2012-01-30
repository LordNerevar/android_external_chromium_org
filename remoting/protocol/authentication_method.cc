// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/protocol/authentication_method.h"

#include "base/logging.h"
#include "crypto/hmac.h"
#include "remoting/protocol/auth_util.h"
#include "remoting/protocol/v1_authenticator.h"
#include "remoting/protocol/v2_authenticator.h"

namespace remoting {
namespace protocol {

// static
AuthenticationMethod AuthenticationMethod::Invalid() {
  return AuthenticationMethod();
}

// static
AuthenticationMethod AuthenticationMethod::V1Token() {
  return AuthenticationMethod(VERSION_1, NONE);
}

// static
AuthenticationMethod AuthenticationMethod::Spake2(HashFunction hash_function) {
  return AuthenticationMethod(VERSION_2, hash_function);
}

// static
AuthenticationMethod AuthenticationMethod::FromString(
    const std::string& value) {
  if (value == "v1_token") {
    return V1Token();
  } else if (value == "spake2_plain") {
    return Spake2(NONE);
  } else if (value == "spake2_hmac") {
    return Spake2(HMAC_SHA256);
  } else {
    return AuthenticationMethod::Invalid();
  }
}

// static
std::string AuthenticationMethod::ApplyHashFunction(
    HashFunction hash_function,
    const std::string& tag,
    const std::string& shared_secret) {
  switch (hash_function) {
    case NONE:
      return shared_secret;
      break;

    case HMAC_SHA256: {
      crypto::HMAC response(crypto::HMAC::SHA256);
      if (!response.Init(tag)) {
        LOG(FATAL) << "HMAC::Init failed";
      }

      unsigned char out_bytes[kSharedSecretHashLength];
      if (!response.Sign(shared_secret, out_bytes, sizeof(out_bytes))) {
        LOG(FATAL) << "HMAC::Sign failed";
      }

      return std::string(out_bytes, out_bytes + sizeof(out_bytes));
    }
  }

  NOTREACHED();
  return shared_secret;
}

AuthenticationMethod::AuthenticationMethod()
    : invalid_(true),
      version_(VERSION_2),
      hash_function_(NONE) {
}

AuthenticationMethod::AuthenticationMethod(Version version,
                                           HashFunction hash_function)
    : invalid_(false),
      version_(version),
      hash_function_(hash_function) {
}

scoped_ptr<Authenticator> AuthenticationMethod::CreateAuthenticator(
    const std::string& local_jid,
    const std::string& tag,
    const std::string& shared_secret) const {
  DCHECK(is_valid());

  switch (version_) {
    case VERSION_1:
      DCHECK_EQ(hash_function_, NONE);
      return scoped_ptr<Authenticator>(
          new protocol::V1ClientAuthenticator(local_jid, shared_secret));

    case VERSION_2:
      return protocol::V2Authenticator::CreateForClient(
          ApplyHashFunction(hash_function_, tag, shared_secret),
          Authenticator::MESSAGE_READY);
  }

  NOTREACHED();
  return scoped_ptr<Authenticator>(NULL);
}

AuthenticationMethod::Version AuthenticationMethod::version() const {
  DCHECK(is_valid());
  return version_;
}

AuthenticationMethod::HashFunction AuthenticationMethod::hash_function() const {
  DCHECK(is_valid());
  return hash_function_;
}

const std::string AuthenticationMethod::ToString() const {
  DCHECK(is_valid());

  switch (version_) {
    case VERSION_1:
      return "v1_token";

    case VERSION_2:
      switch (hash_function_) {
        case NONE:
          return "spake2_plain";
        case HMAC_SHA256:
          return "spake2_hmac";
      }
  }

  NOTREACHED();
  return "";
}

bool AuthenticationMethod::operator ==(
    const AuthenticationMethod& other) const {
  if (!is_valid())
    return !other.is_valid();
  if (!other.is_valid())
    return false;
  return version_ == other.version_ &&
      hash_function_ == other.hash_function_;
}

}  // namespace protocol
}  // namespace remoting
