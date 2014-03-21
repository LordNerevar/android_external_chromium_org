// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// These tests make sure MediaGalleriesPermission values are parsed correctly.

#include "base/values.h"
#include "extensions/common/permissions/api_permission.h"
#include "extensions/common/permissions/media_galleries_permission.h"
#include "extensions/common/permissions/media_galleries_permission_data.h"
#include "extensions/common/permissions/permissions_info.h"
#include "testing/gtest/include/gtest/gtest.h"

using content::SocketPermissionRequest;
using extensions::SocketPermissionData;

namespace extensions {

namespace {

TEST(MediaGalleriesPermissionTest, GoodValues) {
  std::string error;
  const APIPermissionInfo* permission_info =
    PermissionsInfo::GetInstance()->GetByID(APIPermission::kMediaGalleries);

  scoped_ptr<APIPermission> permission(
      permission_info->CreateAPIPermission());

  // access_type + all_detected
  scoped_ptr<base::ListValue> value(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  EXPECT_TRUE(permission->FromValue(value.get(), &error));
  EXPECT_TRUE(error.empty());
  error.clear();

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  value->AppendString(MediaGalleriesPermission::kCopyToPermission);
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  EXPECT_TRUE(permission->FromValue(value.get(), &error));
  EXPECT_TRUE(error.empty());
  error.clear();

  // all_detected
  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  EXPECT_TRUE(permission->FromValue(value.get(), &error));
  EXPECT_TRUE(error.empty());
  error.clear();

  // access_type
  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  EXPECT_TRUE(permission->FromValue(value.get(), &error));
  EXPECT_TRUE(error.empty());
  error.clear();

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  EXPECT_TRUE(permission->FromValue(value.get(), &error));
  EXPECT_TRUE(error.empty());
  error.clear();

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kCopyToPermission);
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  EXPECT_TRUE(permission->FromValue(value.get(), &error));
  EXPECT_TRUE(error.empty());
  error.clear();

  // Repeats do not make a difference.
  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  EXPECT_TRUE(permission->FromValue(value.get(), &error));
  EXPECT_TRUE(error.empty());
  error.clear();

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  EXPECT_TRUE(permission->FromValue(value.get(), &error));
  EXPECT_TRUE(error.empty());
  error.clear();
}

TEST(MediaGalleriesPermissionTest, BadValues) {
  std::string error;
  const APIPermissionInfo* permission_info =
    PermissionsInfo::GetInstance()->GetByID(APIPermission::kMediaGalleries);

  scoped_ptr<APIPermission> permission(permission_info->CreateAPIPermission());

  // Empty
  scoped_ptr<base::ListValue> value(new base::ListValue());
  EXPECT_FALSE(permission->FromValue(value.get(), &error));
  EXPECT_FALSE(error.empty());
  error.clear();

  // copyTo and delete without read
  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kCopyToPermission);
  EXPECT_FALSE(permission->FromValue(value.get(), &error));
  EXPECT_FALSE(error.empty());
  error.clear();

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  EXPECT_FALSE(permission->FromValue(value.get(), &error));
  EXPECT_FALSE(error.empty());
  error.clear();

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  value->AppendString(MediaGalleriesPermission::kCopyToPermission);
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  EXPECT_FALSE(permission->FromValue(value.get(), &error));
  EXPECT_FALSE(error.empty());
  error.clear();

  // copyTo without delete
  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  value->AppendString(MediaGalleriesPermission::kCopyToPermission);
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  EXPECT_FALSE(permission->FromValue(value.get(), &error));
  EXPECT_FALSE(error.empty());
  error.clear();

  // Repeats do not make a difference.
  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kCopyToPermission);
  value->AppendString(MediaGalleriesPermission::kCopyToPermission);
  EXPECT_FALSE(permission->FromValue(value.get(), &error));
  EXPECT_FALSE(error.empty());
  error.clear();

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  value->AppendString(MediaGalleriesPermission::kCopyToPermission);
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  EXPECT_FALSE(permission->FromValue(value.get(), &error));
  EXPECT_FALSE(error.empty());
  error.clear();
}

TEST(MediaGalleriesPermissionTest, Equal) {
  const APIPermissionInfo* permission_info =
    PermissionsInfo::GetInstance()->GetByID(APIPermission::kMediaGalleries);

  scoped_ptr<APIPermission> permission1(
      permission_info->CreateAPIPermission());
  scoped_ptr<APIPermission> permission2(
      permission_info->CreateAPIPermission());

  scoped_ptr<base::ListValue> value(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  ASSERT_TRUE(permission1->FromValue(value.get(), NULL));

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  ASSERT_TRUE(permission2->FromValue(value.get(), NULL));
  EXPECT_TRUE(permission1->Equal(permission2.get()));

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  ASSERT_TRUE(permission2->FromValue(value.get(), NULL));
  EXPECT_TRUE(permission1->Equal(permission2.get()));

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  ASSERT_TRUE(permission1->FromValue(value.get(), NULL));

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  ASSERT_TRUE(permission2->FromValue(value.get(), NULL));
  EXPECT_TRUE(permission1->Equal(permission2.get()));

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  value->AppendString(MediaGalleriesPermission::kCopyToPermission);
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  ASSERT_TRUE(permission1->FromValue(value.get(), NULL));

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  value->AppendString(MediaGalleriesPermission::kCopyToPermission);
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  ASSERT_TRUE(permission2->FromValue(value.get(), NULL));
  EXPECT_TRUE(permission1->Equal(permission2.get()));
}

TEST(MediaGalleriesPermissionTest, NotEqual) {
  const APIPermissionInfo* permission_info =
    PermissionsInfo::GetInstance()->GetByID(APIPermission::kMediaGalleries);

  scoped_ptr<APIPermission> permission1(
      permission_info->CreateAPIPermission());
  scoped_ptr<APIPermission> permission2(
      permission_info->CreateAPIPermission());

  scoped_ptr<base::ListValue> value(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  ASSERT_TRUE(permission1->FromValue(value.get(), NULL));

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  value->AppendString(MediaGalleriesPermission::kCopyToPermission);
  ASSERT_TRUE(permission2->FromValue(value.get(), NULL));
  EXPECT_FALSE(permission1->Equal(permission2.get()));
}

TEST(MediaGalleriesPermissionTest, ToFromValue) {
  const APIPermissionInfo* permission_info =
    PermissionsInfo::GetInstance()->GetByID(APIPermission::kMediaGalleries);

  scoped_ptr<APIPermission> permission1(
      permission_info->CreateAPIPermission());
  scoped_ptr<APIPermission> permission2(
      permission_info->CreateAPIPermission());

  scoped_ptr<base::ListValue> value(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kAllAutoDetectedPermission);
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  ASSERT_TRUE(permission1->FromValue(value.get(), NULL));

  scoped_ptr<base::Value> vtmp(permission1->ToValue());
  ASSERT_TRUE(vtmp);
  ASSERT_TRUE(permission2->FromValue(vtmp.get(), NULL));
  EXPECT_TRUE(permission1->Equal(permission2.get()));

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  value->AppendString(MediaGalleriesPermission::kCopyToPermission);
  ASSERT_TRUE(permission1->FromValue(value.get(), NULL));

  vtmp = permission1->ToValue();
  ASSERT_TRUE(vtmp);
  ASSERT_TRUE(permission2->FromValue(vtmp.get(), NULL));
  EXPECT_TRUE(permission1->Equal(permission2.get()));

  value.reset(new base::ListValue());
  value->AppendString(MediaGalleriesPermission::kReadPermission);
  value->AppendString(MediaGalleriesPermission::kDeletePermission);
  ASSERT_TRUE(permission1->FromValue(value.get(), NULL));

  vtmp = permission1->ToValue();
  ASSERT_TRUE(vtmp);
  ASSERT_TRUE(permission2->FromValue(vtmp.get(), NULL));
  EXPECT_TRUE(permission1->Equal(permission2.get()));
}

}  // namespace

}  // namespace extensions
