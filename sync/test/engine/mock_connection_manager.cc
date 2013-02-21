// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Mock ServerConnectionManager class for use in client regression tests.

#include "sync/test/engine/mock_connection_manager.h"

#include <map>

#include "base/location.h"
#include "base/stringprintf.h"
#include "sync/engine/syncer_proto_util.h"
#include "sync/test/engine/test_id_factory.h"
#include "sync/protocol/bookmark_specifics.pb.h"
#include "sync/syncable/directory.h"
#include "sync/syncable/syncable_write_transaction.h"
#include "sync/test/engine/test_id_factory.h"
#include "testing/gtest/include/gtest/gtest.h"

using std::map;
using std::string;
using sync_pb::ClientToServerMessage;
using sync_pb::CommitMessage;
using sync_pb::CommitResponse;
using sync_pb::GetUpdatesMessage;
using sync_pb::SyncEnums;

namespace syncer {

using syncable::WriteTransaction;

static char kValidAuthToken[] = "AuthToken";

MockConnectionManager::MockConnectionManager(syncable::Directory* directory)
    : ServerConnectionManager("unused", 0, false),
      server_reachable_(true),
      conflict_all_commits_(false),
      conflict_n_commits_(0),
      next_new_id_(10000),
      store_birthday_("Store BDay!"),
      store_birthday_sent_(false),
      client_stuck_(false),
      commit_time_rename_prepended_string_(""),
      countdown_to_postbuffer_fail_(0),
      directory_(directory),
      mid_commit_observer_(NULL),
      throttling_(false),
      fail_with_auth_invalid_(false),
      fail_non_periodic_get_updates_(false),
      gu_client_command_(NULL),
      commit_client_command_(NULL),
      next_position_in_parent_(2),
      use_legacy_bookmarks_protocol_(false),
      num_get_updates_requests_(0) {
  SetNewTimestamp(0);
  set_auth_token(kValidAuthToken);
}

MockConnectionManager::~MockConnectionManager() {
  EXPECT_TRUE(update_queue_.empty()) << "Unfetched updates.";
}

void MockConnectionManager::SetCommitTimeRename(string prepend) {
  commit_time_rename_prepended_string_ = prepend;
}

void MockConnectionManager::SetMidCommitCallback(
    const base::Closure& callback) {
  mid_commit_callback_ = callback;
}

void MockConnectionManager::SetMidCommitObserver(
    MockConnectionManager::MidCommitObserver* observer) {
    mid_commit_observer_ = observer;
}

bool MockConnectionManager::PostBufferToPath(PostBufferParams* params,
    const string& path,
    const string& auth_token,
    ScopedServerStatusWatcher* watcher) {
  ClientToServerMessage post;
  CHECK(post.ParseFromString(params->buffer_in));
  CHECK(post.has_protocol_version());
  CHECK(post.has_api_key());
  CHECK(post.has_bag_of_chips());
  last_request_.CopyFrom(post);
  client_stuck_ = post.sync_problem_detected();
  sync_pb::ClientToServerResponse response;
  response.Clear();

  if (directory_) {
    // If the Directory's locked when we do this, it's a problem as in normal
    // use this function could take a while to return because it accesses the
    // network. As we can't test this we do the next best thing and hang here
    // when there's an issue.
    CHECK(directory_->good());
    WriteTransaction wt(FROM_HERE, syncable::UNITTEST, directory_);
  }

  if (auth_token.empty()) {
    params->response.server_status = HttpResponse::SYNC_AUTH_ERROR;
    return false;
  }

  if (auth_token != kValidAuthToken) {
    // Simulate server-side auth failure.
    params->response.server_status = HttpResponse::SYNC_AUTH_ERROR;
    InvalidateAndClearAuthToken();
  }

  if (--countdown_to_postbuffer_fail_ == 0) {
    // Fail as countdown hits zero.
    params->response.server_status = HttpResponse::SYNC_SERVER_ERROR;
    return false;
  }

  if (!server_reachable_) {
    params->response.server_status = HttpResponse::CONNECTION_UNAVAILABLE;
    return false;
  }

  // Default to an ok connection.
  params->response.server_status = HttpResponse::SERVER_CONNECTION_OK;
  response.set_error_code(SyncEnums::SUCCESS);
  const string current_store_birthday = store_birthday();
  response.set_store_birthday(current_store_birthday);
  if (post.has_store_birthday() && post.store_birthday() !=
      current_store_birthday) {
    response.set_error_code(SyncEnums::NOT_MY_BIRTHDAY);
    response.set_error_message("Merry Unbirthday!");
    response.SerializeToString(&params->buffer_out);
    store_birthday_sent_ = true;
    return true;
  }
  bool result = true;
  EXPECT_TRUE(!store_birthday_sent_ || post.has_store_birthday() ||
              post.message_contents() == ClientToServerMessage::AUTHENTICATE);
  store_birthday_sent_ = true;

  if (post.message_contents() == ClientToServerMessage::COMMIT) {
    ProcessCommit(&post, &response);
  } else if (post.message_contents() == ClientToServerMessage::GET_UPDATES) {
    ProcessGetUpdates(&post, &response);
  } else {
    EXPECT_TRUE(false) << "Unknown/unsupported ClientToServerMessage";
    return false;
  }

  {
    base::AutoLock lock(response_code_override_lock_);
    if (throttling_) {
      response.set_error_code(SyncEnums::THROTTLED);
      throttling_ = false;
    }

    if (fail_with_auth_invalid_)
      response.set_error_code(SyncEnums::AUTH_INVALID);
  }

  response.SerializeToString(&params->buffer_out);
  if (post.message_contents() == ClientToServerMessage::COMMIT &&
      !mid_commit_callback_.is_null()) {
    mid_commit_callback_.Run();
    mid_commit_callback_.Reset();
  }
  if (mid_commit_observer_) {
    mid_commit_observer_->Observe();
  }

  return result;
}

sync_pb::GetUpdatesResponse* MockConnectionManager::GetUpdateResponse() {
  if (update_queue_.empty()) {
    NextUpdateBatch();
  }
  return &update_queue_.back();
}

void MockConnectionManager::AddDefaultBookmarkData(sync_pb::SyncEntity* entity,
                                                   bool is_folder) {
  if (use_legacy_bookmarks_protocol_) {
    sync_pb::SyncEntity_BookmarkData* data = entity->mutable_bookmarkdata();
    data->set_bookmark_folder(is_folder);

    if (!is_folder) {
      data->set_bookmark_url("http://google.com");
    }
  } else {
    entity->set_folder(is_folder);
    entity->mutable_specifics()->mutable_bookmark();
    if (!is_folder) {
      entity->mutable_specifics()->mutable_bookmark()->
          set_url("http://google.com");
    }
  }
}

sync_pb::SyncEntity* MockConnectionManager::AddUpdateDirectory(
    int id,
    int parent_id,
    string name,
    int64 version,
    int64 sync_ts,
    std::string originator_cache_guid,
    std::string originator_client_item_id) {
  return AddUpdateDirectory(TestIdFactory::FromNumber(id),
                            TestIdFactory::FromNumber(parent_id),
                            name,
                            version,
                            sync_ts,
                            originator_cache_guid,
                            originator_client_item_id);
}

void MockConnectionManager::SetGUClientCommand(
    sync_pb::ClientCommand* command) {
  gu_client_command_.reset(command);
}

void MockConnectionManager::SetCommitClientCommand(
    sync_pb::ClientCommand* command) {
  commit_client_command_.reset(command);
}

sync_pb::SyncEntity* MockConnectionManager::AddUpdateBookmark(
    int id, int parent_id,
    string name, int64 version,
    int64 sync_ts,
    string originator_client_item_id,
    string originator_cache_guid) {
  return AddUpdateBookmark(TestIdFactory::FromNumber(id),
                           TestIdFactory::FromNumber(parent_id),
                           name,
                           version,
                           sync_ts,
                           originator_client_item_id,
                           originator_cache_guid);
}

sync_pb::SyncEntity* MockConnectionManager::AddUpdateSpecifics(
    int id,
    int parent_id,
    string name,
    int64 version,
    int64 sync_ts,
    bool is_dir,
    int64 position,
    const sync_pb::EntitySpecifics& specifics) {
  sync_pb::SyncEntity* ent = AddUpdateMeta(
      TestIdFactory::FromNumber(id).GetServerId(),
      TestIdFactory::FromNumber(parent_id).GetServerId(),
      name, version, sync_ts);
  ent->set_position_in_parent(position);
  ent->mutable_specifics()->CopyFrom(specifics);
  ent->set_folder(is_dir);
  return ent;
}

sync_pb::SyncEntity* MockConnectionManager::AddUpdateSpecifics(
    int id,
    int parent_id,
    string name,
    int64 version,
    int64 sync_ts,
    bool is_dir,
    int64 position,
    const sync_pb::EntitySpecifics& specifics,
    string originator_cache_guid,
    string originator_client_item_id) {
  sync_pb::SyncEntity* ent = AddUpdateSpecifics(
      id, parent_id, name, version, sync_ts, is_dir, position, specifics);
  ent->set_originator_cache_guid(originator_cache_guid);
  ent->set_originator_client_item_id(originator_client_item_id);
  return ent;
}

sync_pb::SyncEntity* MockConnectionManager::SetNigori(
    int id,
    int64 version,
    int64 sync_ts,
    const sync_pb::EntitySpecifics& specifics) {
  sync_pb::SyncEntity* ent = GetUpdateResponse()->add_entries();
  ent->set_id_string(TestIdFactory::FromNumber(id).GetServerId());
  ent->set_parent_id_string(TestIdFactory::FromNumber(0).GetServerId());
  ent->set_server_defined_unique_tag(ModelTypeToRootTag(NIGORI));
  ent->set_name("Nigori");
  ent->set_non_unique_name("Nigori");
  ent->set_version(version);
  ent->set_sync_timestamp(sync_ts);
  ent->set_mtime(sync_ts);
  ent->set_ctime(1);
  ent->set_position_in_parent(0);
  ent->set_folder(false);
  ent->mutable_specifics()->CopyFrom(specifics);
  return ent;
}

sync_pb::SyncEntity* MockConnectionManager::AddUpdatePref(string id,
                                                          string parent_id,
                                                          string client_tag,
                                                          int64 version,
                                                          int64 sync_ts) {
  sync_pb::SyncEntity* ent =
      AddUpdateMeta(id, parent_id, " ", version, sync_ts);

  ent->set_client_defined_unique_tag(client_tag);

  sync_pb::EntitySpecifics specifics;
  AddDefaultFieldValue(PREFERENCES, &specifics);
  ent->mutable_specifics()->CopyFrom(specifics);

  return ent;
}

sync_pb::SyncEntity* MockConnectionManager::AddUpdateFull(
    string id, string parent_id,
    string name, int64 version,
    int64 sync_ts, bool is_dir) {
  sync_pb::SyncEntity* ent =
      AddUpdateMeta(id, parent_id, name, version, sync_ts);
  AddDefaultBookmarkData(ent, is_dir);
  return ent;
}

sync_pb::SyncEntity* MockConnectionManager::AddUpdateMeta(
    string id, string parent_id,
    string name, int64 version,
    int64 sync_ts) {
  sync_pb::SyncEntity* ent = GetUpdateResponse()->add_entries();
  ent->set_id_string(id);
  ent->set_parent_id_string(parent_id);
  ent->set_non_unique_name(name);
  ent->set_name(name);
  ent->set_version(version);
  ent->set_sync_timestamp(sync_ts);
  ent->set_mtime(sync_ts);
  ent->set_ctime(1);
  ent->set_position_in_parent(GeneratePositionInParent());
  return ent;
}

sync_pb::SyncEntity* MockConnectionManager::AddUpdateDirectory(
    string id,
    string parent_id,
    string name,
    int64 version,
    int64 sync_ts,
    std::string originator_cache_guid,
    std::string originator_client_item_id) {
  sync_pb::SyncEntity* ret =
      AddUpdateFull(id, parent_id, name, version, sync_ts, true);
  ret->set_originator_cache_guid(originator_cache_guid);
  ret->set_originator_client_item_id(originator_client_item_id);
  return ret;
}

sync_pb::SyncEntity* MockConnectionManager::AddUpdateBookmark(
    string id,
    string parent_id,
    string name, int64 version,
    int64 sync_ts,
    string originator_cache_guid,
    string originator_client_item_id) {
  sync_pb::SyncEntity* ret =
      AddUpdateFull(id, parent_id, name, version, sync_ts, false);
  ret->set_originator_cache_guid(originator_cache_guid);
  ret->set_originator_client_item_id(originator_client_item_id);
  return ret;
}

sync_pb::SyncEntity* MockConnectionManager::AddUpdateFromLastCommit() {
  EXPECT_EQ(1, last_sent_commit().entries_size());
  EXPECT_EQ(1, last_commit_response().entryresponse_size());
  EXPECT_EQ(CommitResponse::SUCCESS,
      last_commit_response().entryresponse(0).response_type());

  if (last_sent_commit().entries(0).deleted()) {
    AddUpdateTombstone(syncable::Id::CreateFromServerId(
        last_sent_commit().entries(0).id_string()));
  } else {
    sync_pb::SyncEntity* ent = GetUpdateResponse()->add_entries();
    ent->CopyFrom(last_sent_commit().entries(0));
    ent->clear_insert_after_item_id();
    ent->clear_old_parent_id();
    ent->set_position_in_parent(
        last_commit_response().entryresponse(0).position_in_parent());
    ent->set_version(
        last_commit_response().entryresponse(0).version());
    ent->set_id_string(
        last_commit_response().entryresponse(0).id_string());
    // Tests don't currently care about the following:
    // originator_cache_guid, originator_client_item_id, parent_id_string,
    // name, non_unique_name.
  }
  return GetMutableLastUpdate();
}

void MockConnectionManager::AddUpdateTombstone(const syncable::Id& id) {
  // Tombstones have only the ID set and dummy values for the required fields.
  sync_pb::SyncEntity* ent = GetUpdateResponse()->add_entries();
  ent->set_id_string(id.GetServerId());
  ent->set_version(0);
  ent->set_name("");
  ent->set_deleted(true);
}

void MockConnectionManager::SetLastUpdateDeleted() {
  // Tombstones have only the ID set.  Wipe anything else.
  string id_string = GetMutableLastUpdate()->id_string();
  GetUpdateResponse()->mutable_entries()->RemoveLast();
  AddUpdateTombstone(syncable::Id::CreateFromServerId(id_string));
}

void MockConnectionManager::SetLastUpdateOriginatorFields(
    const string& client_id,
    const string& entry_id) {
  GetMutableLastUpdate()->set_originator_cache_guid(client_id);
  GetMutableLastUpdate()->set_originator_client_item_id(entry_id);
}

void MockConnectionManager::SetLastUpdateServerTag(const string& tag) {
  GetMutableLastUpdate()->set_server_defined_unique_tag(tag);
}

void MockConnectionManager::SetLastUpdateClientTag(const string& tag) {
  GetMutableLastUpdate()->set_client_defined_unique_tag(tag);
}

void MockConnectionManager::SetLastUpdatePosition(int64 server_position) {
  GetMutableLastUpdate()->set_position_in_parent(server_position);
}

void MockConnectionManager::SetNewTimestamp(int ts) {
  next_token_ = base::StringPrintf("mock connection ts = %d", ts);
  ApplyToken();
}

void MockConnectionManager::ApplyToken() {
  if (!update_queue_.empty()) {
    GetUpdateResponse()->clear_new_progress_marker();
    sync_pb::DataTypeProgressMarker* new_marker =
        GetUpdateResponse()->add_new_progress_marker();
    new_marker->set_data_type_id(-1);  // Invalid -- clients shouldn't see.
    new_marker->set_token(next_token_);
  }
}

void MockConnectionManager::SetChangesRemaining(int64 timestamp) {
  GetUpdateResponse()->set_changes_remaining(timestamp);
}

void MockConnectionManager::ProcessGetUpdates(
    sync_pb::ClientToServerMessage* csm,
    sync_pb::ClientToServerResponse* response) {
  CHECK(csm->has_get_updates());
  ASSERT_EQ(csm->message_contents(), ClientToServerMessage::GET_UPDATES);
  const GetUpdatesMessage& gu = csm->get_updates();
  num_get_updates_requests_++;
  EXPECT_FALSE(gu.has_from_timestamp());
  EXPECT_FALSE(gu.has_requested_types());

  if (fail_non_periodic_get_updates_) {
    EXPECT_EQ(sync_pb::GetUpdatesCallerInfo::PERIODIC,
              gu.caller_info().source());
  }

  // Verify that the GetUpdates filter sent by the Syncer matches the test
  // expectation.
  ModelTypeSet protocol_types = ProtocolTypes();
  for (ModelTypeSet::Iterator iter = protocol_types.First(); iter.Good();
       iter.Inc()) {
    ModelType model_type = iter.Get();
    sync_pb::DataTypeProgressMarker const* progress_marker =
        GetProgressMarkerForType(gu.from_progress_marker(), model_type);
    EXPECT_EQ(expected_filter_.Has(model_type), (progress_marker != NULL))
        << "Syncer requested_types differs from test expectation.";
    if (progress_marker) {
      EXPECT_EQ((expected_states_.count(model_type) > 0 ?
                 expected_states_[model_type].payload :
                 std::string()),
                progress_marker->notification_hint());
    }
  }

  // Verify that the items we're about to send back to the client are of
  // the types requested by the client.  If this fails, it probably indicates
  // a test bug.
  EXPECT_TRUE(gu.fetch_folders());
  EXPECT_FALSE(gu.has_requested_types());
  if (update_queue_.empty()) {
    GetUpdateResponse();
  }
  sync_pb::GetUpdatesResponse* updates = &update_queue_.front();
  for (int i = 0; i < updates->entries_size(); ++i) {
    if (!updates->entries(i).deleted()) {
      ModelType entry_type = GetModelType(updates->entries(i));
      EXPECT_TRUE(
          IsModelTypePresentInSpecifics(gu.from_progress_marker(), entry_type))
          << "Syncer did not request updates being provided by the test.";
    }
  }

  response->mutable_get_updates()->CopyFrom(*updates);

  // Set appropriate progress markers, overriding the value squirreled
  // away by ApplyToken().
  std::string token = response->get_updates().new_progress_marker(0).token();
  response->mutable_get_updates()->clear_new_progress_marker();
  for (int i = 0; i < gu.from_progress_marker_size(); ++i) {
    if (gu.from_progress_marker(i).token() != token) {
      sync_pb::DataTypeProgressMarker* new_marker =
          response->mutable_get_updates()->add_new_progress_marker();
      new_marker->set_data_type_id(gu.from_progress_marker(i).data_type_id());
      new_marker->set_token(token);
    }
  }

  // Fill the keystore key if requested.
  if (gu.need_encryption_key())
    response->mutable_get_updates()->add_encryption_keys(keystore_key_);

  update_queue_.pop_front();

  if (gu_client_command_.get()) {
    response->mutable_client_command()->CopyFrom(*gu_client_command_.get());
  }
}

void MockConnectionManager::SetKeystoreKey(const std::string& key) {
  // Note: this is not a thread-safe set, ok for now.  NOT ok if tests
  // run the syncer on the background thread while this method is called.
  keystore_key_ = key;
}

bool MockConnectionManager::ShouldConflictThisCommit() {
  bool conflict = false;
  if (conflict_all_commits_) {
    conflict = true;
  } else if (conflict_n_commits_ > 0) {
    conflict = true;
    --conflict_n_commits_;
  }
  return conflict;
}

void MockConnectionManager::ProcessCommit(
    sync_pb::ClientToServerMessage* csm,
    sync_pb::ClientToServerResponse* response_buffer) {
  CHECK(csm->has_commit());
  ASSERT_EQ(csm->message_contents(), ClientToServerMessage::COMMIT);
  map <string, string> changed_ids;
  const CommitMessage& commit_message = csm->commit();
  CommitResponse* commit_response = response_buffer->mutable_commit();
  commit_messages_.push_back(new CommitMessage);
  commit_messages_.back()->CopyFrom(commit_message);
  map<string, sync_pb::CommitResponse_EntryResponse*> response_map;
  for (int i = 0; i < commit_message.entries_size() ; i++) {
    const sync_pb::SyncEntity& entry = commit_message.entries(i);
    CHECK(entry.has_id_string());
    string id = entry.id_string();
    ASSERT_LT(entry.name().length(), 256ul) << " name probably too long. True "
        "server name checking not implemented";
    if (entry.version() == 0) {
      // Relies on our new item string id format. (string representation of a
      // negative number).
      committed_ids_.push_back(syncable::Id::CreateFromClientString(id));
    } else {
      committed_ids_.push_back(syncable::Id::CreateFromServerId(id));
    }
    if (response_map.end() == response_map.find(id))
      response_map[id] = commit_response->add_entryresponse();
    sync_pb::CommitResponse_EntryResponse* er = response_map[id];
    if (ShouldConflictThisCommit()) {
      er->set_response_type(CommitResponse::CONFLICT);
      continue;
    }
    er->set_response_type(CommitResponse::SUCCESS);
    er->set_version(entry.version() + 1);
    if (!commit_time_rename_prepended_string_.empty()) {
      // Commit time rename sent down from the server.
      er->set_name(commit_time_rename_prepended_string_ + entry.name());
    }
    string parent_id = entry.parent_id_string();
    // Remap id's we've already assigned.
    if (changed_ids.end() != changed_ids.find(parent_id)) {
      parent_id = changed_ids[parent_id];
      er->set_parent_id_string(parent_id);
    }
    if (entry.has_version() && 0 != entry.version()) {
      er->set_id_string(id);  // Allows verification.
    } else {
      string new_id = base::StringPrintf("mock_server:%d", next_new_id_++);
      changed_ids[id] = new_id;
      er->set_id_string(new_id);
    }
  }
  commit_responses_.push_back(new CommitResponse(*commit_response));

  if (commit_client_command_.get()) {
    response_buffer->mutable_client_command()->CopyFrom(
        *commit_client_command_.get());
  }
}

sync_pb::SyncEntity* MockConnectionManager::AddUpdateDirectory(
    syncable::Id id,
    syncable::Id parent_id,
    string name,
    int64 version,
    int64 sync_ts,
    string originator_cache_guid,
    string originator_client_item_id) {
  return AddUpdateDirectory(id.GetServerId(), parent_id.GetServerId(),
                            name, version, sync_ts, originator_cache_guid,
                            originator_client_item_id);
}

sync_pb::SyncEntity* MockConnectionManager::AddUpdateBookmark(
    syncable::Id id,
    syncable::Id parent_id,
    string name,
    int64 version,
    int64 sync_ts,
    string originator_cache_guid,
    string originator_client_item_id) {
  return AddUpdateBookmark(id.GetServerId(), parent_id.GetServerId(),
                           name, version, sync_ts, originator_cache_guid,
                           originator_client_item_id);
}

sync_pb::SyncEntity* MockConnectionManager::GetMutableLastUpdate() {
  sync_pb::GetUpdatesResponse* updates = GetUpdateResponse();
  EXPECT_GT(updates->entries_size(), 0);
  return updates->mutable_entries()->Mutable(updates->entries_size() - 1);
}

void MockConnectionManager::NextUpdateBatch() {
  update_queue_.push_back(sync_pb::GetUpdatesResponse::default_instance());
  SetChangesRemaining(0);
  ApplyToken();
}

const CommitMessage& MockConnectionManager::last_sent_commit() const {
  EXPECT_TRUE(!commit_messages_.empty());
  return *commit_messages_.back();
}

const CommitResponse& MockConnectionManager::last_commit_response() const {
  EXPECT_TRUE(!commit_responses_.empty());
  return *commit_responses_.back();
}

bool MockConnectionManager::IsModelTypePresentInSpecifics(
    const google::protobuf::RepeatedPtrField<
        sync_pb::DataTypeProgressMarker>& filter,
    ModelType value) {
  int data_type_id = GetSpecificsFieldNumberFromModelType(value);
  for (int i = 0; i < filter.size(); ++i) {
    if (filter.Get(i).data_type_id() == data_type_id) {
      return true;
    }
  }
  return false;
}

sync_pb::DataTypeProgressMarker const*
    MockConnectionManager::GetProgressMarkerForType(
        const google::protobuf::RepeatedPtrField<
            sync_pb::DataTypeProgressMarker>& filter,
        ModelType value) {
  int data_type_id = GetSpecificsFieldNumberFromModelType(value);
  for (int i = 0; i < filter.size(); ++i) {
    if (filter.Get(i).data_type_id() == data_type_id) {
      return &(filter.Get(i));
    }
  }
  return NULL;
}

void MockConnectionManager::SetServerReachable() {
  server_reachable_ = true;
}

void MockConnectionManager::SetServerNotReachable() {
  server_reachable_ = false;
}

void MockConnectionManager::UpdateConnectionStatus() {
  if (!server_reachable_) {
    server_status_ = HttpResponse::CONNECTION_UNAVAILABLE;
  } else {
    server_status_ = HttpResponse::SERVER_CONNECTION_OK;
  }
}

}  // namespace syncer
