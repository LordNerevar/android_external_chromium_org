// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/stub_cros_settings_provider.h"

#include <string>

#include "base/bind.h"
#include "base/memory/scoped_ptr.h"
#include "base/values.h"
#include "chrome/browser/chromeos/cros_settings_names.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace chromeos {

namespace {

const Value* kTrueValue = base::Value::CreateBooleanValue(true);
const Value* kFalseValue = base::Value::CreateBooleanValue(false);

void Fail() {
  // Should never be called.
  FAIL();
}

}  // namespace

class StubCrosSettingsProviderTest : public testing::Test {
 protected:
  StubCrosSettingsProviderTest()
      : provider_(new StubCrosSettingsProvider(
          base::Bind(&StubCrosSettingsProviderTest::FireObservers,
                     base::Unretained(this)))) {
  }

  virtual ~StubCrosSettingsProviderTest() {
  }

  virtual void SetUp() OVERRIDE {
    // Reset the observer notification count.
    observer_count_.clear();
  }

  void AssertPref(const std::string& prefName, const Value* value) {
    const Value* pref = provider_->Get(prefName);
    ASSERT_TRUE(pref);
    ASSERT_TRUE(pref->Equals(value));
  }

  void ExpectObservers(const std::string& prefName, int count) {
    EXPECT_EQ(observer_count_[prefName], count);
  }

  void FireObservers(const std::string& path) {
    observer_count_[path]++;
  }

  scoped_ptr<StubCrosSettingsProvider> provider_;
  std::map<std::string, int> observer_count_;
};

TEST_F(StubCrosSettingsProviderTest, HandlesSettings) {
  // HandlesSettings should return false for unknown settings.
  ASSERT_TRUE(provider_->HandlesSetting(kDeviceOwner));
  ASSERT_FALSE(provider_->HandlesSetting("no.such.setting"));
}

TEST_F(StubCrosSettingsProviderTest, Defaults) {
  // Verify default values.
  AssertPref(kAccountsPrefAllowGuest, kTrueValue);
  AssertPref(kAccountsPrefAllowNewUser, kTrueValue);
  AssertPref(kAccountsPrefShowUserNamesOnSignIn, kTrueValue);
}

TEST_F(StubCrosSettingsProviderTest, Set) {
  // Setting value and reading it afterwards returns the same value.
  base::StringValue owner_value("me@owner");
  provider_->Set(kDeviceOwner, owner_value);
  AssertPref(kDeviceOwner, &owner_value);
  ExpectObservers(kDeviceOwner, 1);
}

TEST_F(StubCrosSettingsProviderTest, SetMissing) {
  // Setting is missing initially but is added by |Set|.
  base::StringValue pref_value("testing");
  ASSERT_FALSE(provider_->Get(kReleaseChannel));
  provider_->Set(kReleaseChannel, pref_value);
  AssertPref(kReleaseChannel, &pref_value);
  ExpectObservers(kReleaseChannel, 1);
}

TEST_F(StubCrosSettingsProviderTest, GetTrusted) {
  // Should return immediately without invoking the callback.
  bool trusted = provider_->GetTrusted(kDeviceOwner, base::Bind(&Fail));
  EXPECT_TRUE(trusted);
}

}  // namespace chromeos
