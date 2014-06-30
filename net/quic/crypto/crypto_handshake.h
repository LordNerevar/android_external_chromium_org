// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_QUIC_CRYPTO_CRYPTO_HANDSHAKE_H_
#define NET_QUIC_CRYPTO_CRYPTO_HANDSHAKE_H_

#include <string>
#include <vector>

#include "base/memory/scoped_ptr.h"
#include "net/base/net_export.h"
#include "net/quic/quic_protocol.h"

namespace net {

class CommonCertSets;
class KeyExchange;
class QuicDecrypter;
class QuicEncrypter;

// HandshakeFailureReason enum values are uploaded to UMA, they cannot be
// changed.
enum HandshakeFailureReason {
  HANDSHAKE_OK = 0,

  // Failure reasons for an invalid client nonce in CHLO.
  //
  // Client nonce had incorrect length.
  CLIENT_NONCE_INVALID_FAILURE = 1,
  // Client nonce is not unique.
  CLIENT_NONCE_NOT_UNIQUE_FAILURE = 2,
  // Client orbit is invalid or incorrect.
  CLIENT_NONCE_INVALID_ORBIT_FAILURE = 3,
  // Client nonce's timestamp is not in the strike register's valid time range.
  CLIENT_NONCE_INVALID_TIME_FAILURE = 4,
  // Client nonce verification has failed because strike register is down.
  CLIENT_NONCE_NO_STRIKE_REGISTER_FAILURE = 5,

  // Failure reasons for an invalid server nonce in CHLO.
  //
  // Unbox of server nonce failed.
  SERVER_NONCE_DECRYPTION_FAILURE = 6,
  // Decrypted server nonce had incorrect length.
  SERVER_NONCE_INVALID_FAILURE = 7,
  // Server nonce is not unique.
  SERVER_NONCE_NOT_UNIQUE_FAILURE = 8,
  // Server nonce's timestamp is not in the strike register's valid time range.
  SERVER_NONCE_INVALID_TIME_FAILURE = 9,

  // Failure reasons for an invalid server config in CHLO.
  //
  // Missing Server config id (kSCID) tag.
  SERVER_CONFIG_INCHOATE_HELLO_FAILURE = 10,
  // Couldn't find the Server config id (kSCID).
  SERVER_CONFIG_UNKNOWN_CONFIG_FAILURE = 11,

  // Failure reasons for an invalid source-address token.
  //
  // Missing Source-address token (kSourceAddressTokenTag) tag.
  SOURCE_ADDRESS_TOKEN_INVALID_FAILURE = 12,
  // Unbox of Source-address token failed.
  SOURCE_ADDRESS_TOKEN_DECRYPTION_FAILURE = 13,
  // Couldn't parse the unbox'ed Source-address token.
  SOURCE_ADDRESS_TOKEN_PARSE_FAILURE = 14,
  // Source-address token is for a different IP address.
  SOURCE_ADDRESS_TOKEN_DIFFERENT_IP_ADDRESS_FAILURE = 15,
  // The source-address token has a timestamp in the future.
  SOURCE_ADDRESS_TOKEN_CLOCK_SKEW_FAILURE = 16,
  // The source-address token has expired.
  SOURCE_ADDRESS_TOKEN_EXPIRED_FAILURE = 17,

  MAX_FAILURE_REASON,
};

// These errors will be packed into an uint32 and we don't want to set the most
// significant bit, which may be misinterpreted as the sign bit.
COMPILE_ASSERT(MAX_FAILURE_REASON <= 32, failure_reason_out_of_sync);

// A CrypterPair contains the encrypter and decrypter for an encryption level.
struct NET_EXPORT_PRIVATE CrypterPair {
  CrypterPair();
  ~CrypterPair();
  scoped_ptr<QuicEncrypter> encrypter;
  scoped_ptr<QuicDecrypter> decrypter;
};

// Parameters negotiated by the crypto handshake.
struct NET_EXPORT_PRIVATE QuicCryptoNegotiatedParameters {
  // Initializes the members to 0 or empty values.
  QuicCryptoNegotiatedParameters();
  ~QuicCryptoNegotiatedParameters();

  QuicTag key_exchange;
  QuicTag aead;
  std::string initial_premaster_secret;
  std::string forward_secure_premaster_secret;
  CrypterPair initial_crypters;
  CrypterPair forward_secure_crypters;
  // Normalized SNI: converted to lower case and trailing '.' removed.
  std::string sni;
  std::string client_nonce;
  std::string server_nonce;
  // hkdf_input_suffix contains the HKDF input following the label: the
  // ConnectionId, client hello and server config. This is only populated in the
  // client because only the client needs to derive the forward secure keys at a
  // later time from the initial keys.
  std::string hkdf_input_suffix;
  // cached_certs contains the cached certificates that a client used when
  // sending a client hello.
  std::vector<std::string> cached_certs;
  // client_key_exchange is used by clients to store the ephemeral KeyExchange
  // for the connection.
  scoped_ptr<KeyExchange> client_key_exchange;
  // channel_id is set by servers to a ChannelID key when the client correctly
  // proves possession of the corresponding private key. It consists of 32
  // bytes of x coordinate, followed by 32 bytes of y coordinate. Both values
  // are big-endian and the pair is a P-256 public key.
  std::string channel_id;
};

// QuicCryptoConfig contains common configuration between clients and servers.
class NET_EXPORT_PRIVATE QuicCryptoConfig {
 public:
  // kInitialLabel is a constant that is used when deriving the initial
  // (non-forward secure) keys for the connection in order to tie the resulting
  // key to this protocol.
  static const char kInitialLabel[];

  // kCETVLabel is a constant that is used when deriving the keys for the
  // encrypted tag/value block in the client hello.
  static const char kCETVLabel[];

  // kForwardSecureLabel is a constant that is used when deriving the forward
  // secure keys for the connection in order to tie the resulting key to this
  // protocol.
  static const char kForwardSecureLabel[];

  QuicCryptoConfig();
  ~QuicCryptoConfig();

  // Key exchange methods. The following two members' values correspond by
  // index.
  QuicTagVector kexs;
  // Authenticated encryption with associated data (AEAD) algorithms.
  QuicTagVector aead;

  const CommonCertSets* common_cert_sets;

 private:
  DISALLOW_COPY_AND_ASSIGN(QuicCryptoConfig);
};

}  // namespace net

#endif  // NET_QUIC_CRYPTO_CRYPTO_HANDSHAKE_H_
