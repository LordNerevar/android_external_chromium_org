// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/policy/device_local_account_policy_service.h"

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop.h"
#include "chrome/browser/chromeos/settings/device_settings_service.h"
#include "chrome/browser/chromeos/settings/device_settings_test_helper.h"
#include "chrome/browser/chromeos/settings/mock_owner_key_util.h"
#include "chrome/browser/policy/mock_device_management_service.h"
#include "chrome/browser/policy/policy_builder.h"
#include "chrome/browser/policy/proto/chrome_device_policy.pb.h"
#include "content/public/test/test_browser_thread.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace em = enterprise_management;

using testing::Mock;

namespace policy {

class MockDeviceLocalAccountPolicyServiceObserver
    : public DeviceLocalAccountPolicyService::Observer {
 public:
  MOCK_METHOD1(OnPolicyChanged, void(const std::string&));
  MOCK_METHOD0(OnDeviceLocalAccountsChanged, void(void));
};

class DeviceLocalAccountPolicyServiceTest : public testing::Test {
 protected:
  DeviceLocalAccountPolicyServiceTest()
      : loop_(MessageLoop::TYPE_UI),
        ui_thread_(content::BrowserThread::UI, &loop_),
        file_thread_(content::BrowserThread::FILE, &loop_),
        owner_key_util_(new chromeos::MockOwnerKeyUtil()),
        service_(&device_settings_test_helper_, &device_settings_service_) {}

  virtual void SetUp() OVERRIDE {
    owner_key_util_->SetPublicKeyFromPrivateKey(device_policy_.signing_key());

    device_settings_service_.Initialize(&device_settings_test_helper_,
                                        owner_key_util_);
    device_settings_service_.Load();

    device_local_account_policy_.payload().mutable_disablespdy()->
        set_disablespdy(true);
    device_local_account_policy_.policy_data().set_policy_type(
        dm_protocol::kChromePublicAccountPolicyType);
    device_local_account_policy_.Build();

    device_policy_.payload().mutable_device_local_accounts()->add_account()->
        set_id(PolicyBuilder::kFakeUsername);
    device_policy_.Build();

    service_.AddObserver(&observer_);
  }

  virtual void TearDown() OVERRIDE {
    service_.RemoveObserver(&observer_);
  }

  void InstallDevicePolicy() {
    EXPECT_CALL(observer_, OnDeviceLocalAccountsChanged());
    device_settings_test_helper_.set_policy_blob(device_policy_.GetBlob());
    device_settings_test_helper_.Flush();
    Mock::VerifyAndClearExpectations(&observer_);
  }

  MessageLoop loop_;
  content::TestBrowserThread ui_thread_;
  content::TestBrowserThread file_thread_;

  DevicePolicyBuilder device_policy_;
  UserPolicyBuilder device_local_account_policy_;

  chromeos::DeviceSettingsTestHelper device_settings_test_helper_;
  scoped_refptr<chromeos::MockOwnerKeyUtil> owner_key_util_;
  chromeos::DeviceSettingsService device_settings_service_;
  MockDeviceManagementService device_management_service_;

  MockDeviceLocalAccountPolicyServiceObserver observer_;

  DeviceLocalAccountPolicyService service_;

 private:
  DISALLOW_COPY_AND_ASSIGN(DeviceLocalAccountPolicyServiceTest);
};

TEST_F(DeviceLocalAccountPolicyServiceTest, NoAccounts) {
  device_settings_test_helper_.Flush();
  EXPECT_FALSE(service_.GetBrokerForAccount(PolicyBuilder::kFakeUsername));
}

TEST_F(DeviceLocalAccountPolicyServiceTest, GetBroker) {
  InstallDevicePolicy();

  DeviceLocalAccountPolicyBroker* broker =
      service_.GetBrokerForAccount(PolicyBuilder::kFakeUsername);
  ASSERT_TRUE(broker);
  EXPECT_EQ(PolicyBuilder::kFakeUsername, broker->account_id());
  EXPECT_EQ(CloudPolicyStore::STATUS_OK, broker->status());
  EXPECT_FALSE(broker->policy_data());
  EXPECT_FALSE(broker->policy_settings());
}

TEST_F(DeviceLocalAccountPolicyServiceTest, LoadNoPolicy) {
  InstallDevicePolicy();

  EXPECT_CALL(observer_, OnPolicyChanged(PolicyBuilder::kFakeUsername));
  DeviceLocalAccountPolicyBroker* broker =
      service_.GetBrokerForAccount(PolicyBuilder::kFakeUsername);
  ASSERT_TRUE(broker);
  broker->Load();
  device_settings_test_helper_.Flush();
  Mock::VerifyAndClearExpectations(&observer_);

  EXPECT_EQ(CloudPolicyStore::STATUS_LOAD_ERROR, broker->status());
  EXPECT_FALSE(broker->policy_data());
  EXPECT_FALSE(broker->policy_settings());
}

TEST_F(DeviceLocalAccountPolicyServiceTest, LoadValidationFailure) {
  device_local_account_policy_.policy_data().set_policy_type(
      dm_protocol::kChromeUserPolicyType);
  device_local_account_policy_.Build();
  device_settings_test_helper_.set_device_local_account_policy_blob(
      PolicyBuilder::kFakeUsername, device_local_account_policy_.GetBlob());
  InstallDevicePolicy();

  EXPECT_CALL(observer_, OnPolicyChanged(PolicyBuilder::kFakeUsername));
  DeviceLocalAccountPolicyBroker* broker =
      service_.GetBrokerForAccount(PolicyBuilder::kFakeUsername);
  ASSERT_TRUE(broker);
  broker->Load();
  device_settings_test_helper_.Flush();
  Mock::VerifyAndClearExpectations(&observer_);

  EXPECT_EQ(CloudPolicyStore::STATUS_VALIDATION_ERROR, broker->status());
  EXPECT_EQ(CloudPolicyValidatorBase::VALIDATION_WRONG_POLICY_TYPE,
            broker->validation_status());
  EXPECT_FALSE(broker->policy_data());
  EXPECT_FALSE(broker->policy_settings());
}

TEST_F(DeviceLocalAccountPolicyServiceTest, LoadPolicy) {
  device_settings_test_helper_.set_device_local_account_policy_blob(
      PolicyBuilder::kFakeUsername, device_local_account_policy_.GetBlob());
  InstallDevicePolicy();

  EXPECT_CALL(observer_, OnPolicyChanged(PolicyBuilder::kFakeUsername));
  DeviceLocalAccountPolicyBroker* broker =
      service_.GetBrokerForAccount(PolicyBuilder::kFakeUsername);
  ASSERT_TRUE(broker);
  broker->Load();
  device_settings_test_helper_.Flush();
  Mock::VerifyAndClearExpectations(&observer_);

  EXPECT_EQ(CloudPolicyStore::STATUS_OK, broker->status());
  ASSERT_TRUE(broker->policy_data());
  EXPECT_EQ(device_local_account_policy_.policy_data().SerializeAsString(),
            broker->policy_data()->SerializeAsString());
  ASSERT_TRUE(broker->policy_settings());
  EXPECT_EQ(device_local_account_policy_.payload().SerializeAsString(),
            broker->policy_settings()->SerializeAsString());
}

TEST_F(DeviceLocalAccountPolicyServiceTest, StoreValidationFailure) {
  device_local_account_policy_.policy_data().set_policy_type(
      dm_protocol::kChromeUserPolicyType);
  device_local_account_policy_.Build();
  InstallDevicePolicy();

  EXPECT_CALL(observer_, OnPolicyChanged(PolicyBuilder::kFakeUsername));
  DeviceLocalAccountPolicyBroker* broker =
      service_.GetBrokerForAccount(PolicyBuilder::kFakeUsername);
  ASSERT_TRUE(broker);
  broker->Store(device_local_account_policy_.policy());
  device_settings_test_helper_.Flush();
  Mock::VerifyAndClearExpectations(&observer_);

  EXPECT_EQ(CloudPolicyStore::STATUS_VALIDATION_ERROR, broker->status());
  EXPECT_EQ(CloudPolicyValidatorBase::VALIDATION_WRONG_POLICY_TYPE,
            broker->validation_status());
}

TEST_F(DeviceLocalAccountPolicyServiceTest, StorePolicy) {
  InstallDevicePolicy();

  EXPECT_CALL(observer_, OnPolicyChanged(PolicyBuilder::kFakeUsername));
  DeviceLocalAccountPolicyBroker* broker =
      service_.GetBrokerForAccount(PolicyBuilder::kFakeUsername);
  ASSERT_TRUE(broker);
  broker->Store(device_local_account_policy_.policy());
  device_settings_test_helper_.Flush();
  Mock::VerifyAndClearExpectations(&observer_);

  EXPECT_EQ(device_local_account_policy_.GetBlob(),
            device_settings_test_helper_.device_local_account_policy_blob(
                PolicyBuilder::kFakeUsername));
}

TEST_F(DeviceLocalAccountPolicyServiceTest, DevicePolicyChange) {
  device_settings_test_helper_.set_device_local_account_policy_blob(
      PolicyBuilder::kFakeUsername, device_local_account_policy_.GetBlob());
  InstallDevicePolicy();

  EXPECT_CALL(observer_, OnDeviceLocalAccountsChanged());
  device_policy_.payload().mutable_device_local_accounts()->clear_account();
  device_policy_.Build();
  device_settings_test_helper_.set_policy_blob(device_policy_.GetBlob());
  device_settings_service_.PropertyChangeComplete(true);
  device_settings_test_helper_.Flush();
  EXPECT_FALSE(service_.GetBrokerForAccount(PolicyBuilder::kFakeUsername));
  Mock::VerifyAndClearExpectations(&observer_);
}

}  // namespace policy
