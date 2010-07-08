// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/env_var.h"
#include "base/scoped_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/platform_test.h"

typedef PlatformTest EnvVarTest;

TEST_F(EnvVarTest, GetEnvVar) {
  // Every setup should have non-empty PATH...
  scoped_ptr<base::EnvVarGetter> env(base::EnvVarGetter::Create());
  std::string env_value;
  EXPECT_TRUE(env->GetEnv("PATH", &env_value));
  EXPECT_NE(env_value, "");
}

TEST_F(EnvVarTest, HasEnvVar) {
  // Every setup should have PATH...
  scoped_ptr<base::EnvVarGetter> env(base::EnvVarGetter::Create());
  EXPECT_TRUE(env->HasEnv("PATH"));
}
