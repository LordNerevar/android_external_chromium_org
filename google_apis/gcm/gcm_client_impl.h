// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GOOGLE_APIS_GCM_GCM_CLIENT_IMPL_H_
#define GOOGLE_APIS_GCM_GCM_CLIENT_IMPL_H_

#include <map>
#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/stl_util.h"
#include "google_apis/gcm/base/mcs_message.h"
#include "google_apis/gcm/engine/gcm_store.h"
#include "google_apis/gcm/engine/mcs_client.h"
#include "google_apis/gcm/gcm_client.h"
#include "google_apis/gcm/protocol/android_checkin.pb.h"
#include "net/base/net_log.h"
#include "net/url_request/url_request_context_getter.h"

namespace base {
class Clock;
}  // namespace base

namespace net {
class HttpNetworkSession;
}  // namespace net

namespace gcm {

class CheckinRequest;
class ConnectionFactory;
class GCMClientImplTest;
class RegistrationRequest;
class UserList;

// Implements the GCM Client. It is used to coordinate MCS Client (communication
// with MCS) and other pieces of GCM infrastructure like Registration and
// Checkins. It also allows for registering user delegates that host
// applications that send and receive messages.
class GCM_EXPORT GCMClientImpl : public GCMClient {
 public:
  GCMClientImpl();
  virtual ~GCMClientImpl();

  // Overridden from GCMClient:
  virtual void Initialize(
      const checkin_proto::ChromeBuildProto& chrome_build_proto,
      const base::FilePath& store_path,
      const scoped_refptr<base::SequencedTaskRunner>& blocking_task_runner,
      const scoped_refptr<net::URLRequestContextGetter>&
          url_request_context_getter) OVERRIDE;
  virtual void SetUserDelegate(const std::string& username,
                               Delegate* delegate) OVERRIDE;
  virtual void CheckIn(const std::string& username) OVERRIDE;
  virtual void Register(const std::string& username,
                        const std::string& app_id,
                        const std::string& cert,
                        const std::vector<std::string>& sender_ids) OVERRIDE;
  virtual void Unregister(const std::string& username,
                          const std::string& app_id) OVERRIDE;
  virtual void Send(const std::string& username,
                    const std::string& app_id,
                    const std::string& receiver_id,
                    const OutgoingMessage& message) OVERRIDE;
  virtual bool IsReady() const OVERRIDE;

 private:
  // State representation of the GCMClient.
  enum State {
    // Uninitialized.
    UNINITIALIZED,
    // GCM store loading is in progress.
    LOADING,
    // Initial device checkin is in progress.
    INITIAL_DEVICE_CHECKIN,
    // Ready to accept requests.
    READY,
  };

  // Collection of pending checkin requests. Keys are serial numbers of the
  // users as assigned by the user_list_. Values are pending checkin requests to
  // obtain android IDs and security tokens for the users.
  typedef std::map<int64, CheckinRequest*> PendingCheckins;

  // A pair of |username| and |app_id| identifying a pending
  // RegistrationRequest.
  struct PendingRegistrationKey {
    PendingRegistrationKey(const std::string& username,
                           const std::string& app_id);
    ~PendingRegistrationKey();
    bool operator<(const PendingRegistrationKey& rhs) const;

    std::string username;
    std::string app_id;
  };

  // Collection of pending registration requests. Keys are pairs of |username|
  // and |app_id|, while values are pending registration requests to obtain a
  // registration ID for requesting application.
  typedef std::map<PendingRegistrationKey, RegistrationRequest*>
      PendingRegistrations;

  friend class GCMClientImplTest;

  // Callbacks for the MCSClient.
  // Receives messages and dispatches them to relevant user delegates.
  void OnMessageReceivedFromMCS(const gcm::MCSMessage& message);
  // Receives confirmation of sent messages or information about errors.
  void OnMessageSentToMCS(int64 user_serial_number,
                          const std::string& app_id,
                          const std::string& message_id,
                          MCSClient::MessageSendStatus status);
  // Receives information about mcs_client_ errors.
  void OnMCSError();

  // Runs after GCM Store load is done to trigger continuation of the
  // initialization.
  void OnLoadCompleted(scoped_ptr<GCMStore::LoadResult> result);
  // Initializes mcs_client_, which handles the connection to MCS.
  void InitializeMCSClient(scoped_ptr<GCMStore::LoadResult> result);
  // Complets the first time device checkin.
  void OnFirstTimeDeviceCheckinCompleted(const CheckinInfo& checkin_info);
  // Starts a login on mcs_client_.
  void StartMCSLogin();
  // Resets state to before initialization.
  void ResetState();
  // Sets state to ready. This will initiate the MCS login and notify the
  // delegates.
  void OnReady();

  // Startes a checkin request for a user with specified |serial_number|.
  // Checkin info can be invalid, in which case it is considered a first time
  // checkin.
  void StartCheckin(int64 user_serial_number,
                    const CheckinInfo& checkin_info);
  // Completes the checkin request for the specified |serial_number|.
  // |android_id| and |security_token| are expected to be non-zero or an error
  // is triggered. Function also cleans up the pending checkin.
  void OnCheckinCompleted(int64 user_serial_number,
                          uint64 android_id,
                          uint64 security_token);
  // Completes the checkin request for a device (serial number of 0).
  void OnDeviceCheckinCompleted(const CheckinInfo& checkin_info);

  // Callback for persisting device credentials in the |gcm_store_|.
  void SetDeviceCredentialsCallback(bool success);

  // Completes the registration request.
  void OnRegisterCompleted(const PendingRegistrationKey& registration_key,
                           const std::string& registration_id);

  // Callback for setting a delegate on a |user_list_|. Informs that the
  // delegate with matching |username| was assigned a |user_serial_number|.
  void SetDelegateCompleted(const std::string& username,
                            int64 user_serial_number);

  // Handles incoming data message and dispatches it the a relevant user
  // delegate.
  void HandleIncomingMessage(const gcm::MCSMessage& message);

  // Fires OnMessageSendError event on |delegate|, with specified |app_id| and
  // message ID obtained from |incoming_message| if one is available.
  void NotifyDelegateOnMessageSendError(
      GCMClient::Delegate* delegate,
      const std::string& app_id,
      const IncomingMessage& incoming_message);

  // For testing purpose only.
  // Sets an |mcs_client_| for testing. Takes the ownership of |mcs_client|.
  // TODO(fgorski): Remove this method. Create GCMEngineFactory that will create
  // components of the engine.
  void SetMCSClientForTesting(scoped_ptr<MCSClient> mcs_client);

  // State of the GCM Client Implementation.
  State state_;

  // Device checkin info (android ID and security token used by device).
  CheckinInfo device_checkin_info_;

  // Clock used for timing of retry logic. Passed in for testing. Owned by
  // GCMClientImpl.
  scoped_ptr<base::Clock> clock_;

  // Information about the chrome build.
  // TODO(fgorski): Check if it can be passed in constructor and made const.
  checkin_proto::ChromeBuildProto chrome_build_proto_;

  // Persistent data store for keeping device credentials, messages and user to
  // serial number mappings.
  scoped_ptr<GCMStore> gcm_store_;

  // Keeps the mappings of user's serial numbers and assigns new serial numbers
  // once a user delegate is added for the first time.
  scoped_ptr<UserList> user_list_;

  scoped_refptr<net::HttpNetworkSession> network_session_;
  net::BoundNetLog net_log_;
  scoped_ptr<ConnectionFactory> connection_factory_;
  scoped_refptr<net::URLRequestContextGetter> url_request_context_getter_;

  // Controls receiving and sending of packets and reliable message queueing.
  scoped_ptr<MCSClient> mcs_client_;

  // Currently pending checkins. GCMClientImpl owns the CheckinRequests.
  PendingCheckins pending_checkins_;
  STLValueDeleter<PendingCheckins> pending_checkins_deleter_;

  // Currently pending registrations. GCMClientImpl owns the
  // RegistrationRequests.
  PendingRegistrations pending_registrations_;
  STLValueDeleter<PendingRegistrations> pending_registrations_deleter_;

  // Factory for creating references in callbacks.
  base::WeakPtrFactory<GCMClientImpl> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(GCMClientImpl);
};

}  // namespace gcm

#endif  // GOOGLE_APIS_GCM_GCM_CLIENT_IMPL_H_
