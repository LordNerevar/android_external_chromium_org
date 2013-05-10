// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/message_loop.h"
#include "base/run_loop.h"
#include "base/time.h"
#include "content/browser/gpu/gpu_data_manager_impl_private.h"
#include "content/public/browser/gpu_data_manager_observer.h"
#include "content/public/common/gpu_feature_type.h"
#include "content/public/common/gpu_info.h"
#include "googleurl/src/gurl.h"
#include "gpu/command_buffer/service/gpu_switches.h"
#include "testing/gtest/include/gtest/gtest.h"

#define LONG_STRING_CONST(...) #__VA_ARGS__

namespace content {
namespace {

class TestObserver : public GpuDataManagerObserver {
 public:
  TestObserver()
      : gpu_info_updated_(false),
        video_memory_usage_stats_updated_(false) {
  }
  virtual ~TestObserver() { }

  bool gpu_info_updated() const { return gpu_info_updated_; }
  bool video_memory_usage_stats_updated() const {
    return video_memory_usage_stats_updated_;
  }

  virtual void OnGpuInfoUpdate() OVERRIDE {
    gpu_info_updated_ = true;
  }

  virtual void OnVideoMemoryUsageStatsUpdate(
      const GPUVideoMemoryUsageStats& stats) OVERRIDE {
    video_memory_usage_stats_updated_ = true;
  }

 private:
  bool gpu_info_updated_;
  bool video_memory_usage_stats_updated_;
};

static base::Time GetTimeForTesting() {
  return base::Time::FromDoubleT(1000);
}

static GURL GetDomain1ForTesting() {
  return GURL("http://foo.com/");
}

static GURL GetDomain2ForTesting() {
  return GURL("http://bar.com/");
}

}  // namespace anonymous

class GpuDataManagerImplPrivateTest : public testing::Test {
 public:
  GpuDataManagerImplPrivateTest() { }

  virtual ~GpuDataManagerImplPrivateTest() { }

 protected:
  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

  base::Time JustBeforeExpiration(const GpuDataManagerImplPrivate& manager);
  base::Time JustAfterExpiration(const GpuDataManagerImplPrivate& manager);
  void TestBlockingDomainFrom3DAPIs(
      GpuDataManagerImpl::DomainGuilt guilt_level);
  void TestUnblockingDomainFrom3DAPIs(
      GpuDataManagerImpl::DomainGuilt guilt_level);

  base::MessageLoop message_loop_;
};

// We use new method instead of GetInstance() method because we want
// each test to be independent of each other.

TEST_F(GpuDataManagerImplPrivateTest, GpuSideBlacklisting) {
  // If a feature is allowed in preliminary step (browser side), but
  // disabled when GPU process launches and collects full GPU info,
  // it's too late to let renderer know, so we basically block all GPU
  // access, to be on the safe side.
  GpuDataManagerImplPrivate manager;
  EXPECT_EQ(0u, manager.GetBlacklistedFeatureCount());
  std::string reason;
  EXPECT_TRUE(manager.GpuAccessAllowed(&reason));
  EXPECT_TRUE(reason.empty());

  const std::string blacklist_json = LONG_STRING_CONST(
      {
        "name": "gpu blacklist",
        "version": "0.1",
        "entries": [
          {
            "id": 1,
            "features": [
              "webgl"
            ]
          },
          {
            "id": 2,
            "gl_renderer": {
              "op": "contains",
              "value": "GeForce"
            },
            "features": [
              "accelerated_2d_canvas"
            ]
          }
        ]
      }
  );

  GPUInfo gpu_info;
  gpu_info.gpu.vendor_id = 0x10de;
  gpu_info.gpu.device_id = 0x0640;
  manager.InitializeForTesting(blacklist_json, gpu_info);

  EXPECT_TRUE(manager.GpuAccessAllowed(&reason));
  EXPECT_TRUE(reason.empty());
  EXPECT_EQ(1u, manager.GetBlacklistedFeatureCount());
  EXPECT_TRUE(manager.IsFeatureBlacklisted(GPU_FEATURE_TYPE_WEBGL));

  gpu_info.gl_vendor = "NVIDIA";
  gpu_info.gl_renderer = "NVIDIA GeForce GT 120";
  manager.UpdateGpuInfo(gpu_info);
  EXPECT_FALSE(manager.GpuAccessAllowed(&reason));
  EXPECT_FALSE(reason.empty());
  EXPECT_EQ(2u, manager.GetBlacklistedFeatureCount());
  EXPECT_TRUE(manager.IsFeatureBlacklisted(GPU_FEATURE_TYPE_WEBGL));
  EXPECT_TRUE(manager.IsFeatureBlacklisted(
      GPU_FEATURE_TYPE_ACCELERATED_2D_CANVAS));
}

TEST_F(GpuDataManagerImplPrivateTest, GpuSideExceptions) {
  GpuDataManagerImplPrivate manager;
  EXPECT_EQ(0u, manager.GetBlacklistedFeatureCount());
  EXPECT_TRUE(manager.GpuAccessAllowed(NULL));

  const std::string blacklist_json = LONG_STRING_CONST(
      {
        "name": "gpu blacklist",
        "version": "0.1",
        "entries": [
          {
            "id": 1,
            "exceptions": [
              {
                "gl_renderer": {
                  "op": "contains",
                  "value": "GeForce"
                }
              }
            ],
            "features": [
              "webgl"
            ]
          }
        ]
      }
  );
  GPUInfo gpu_info;
  gpu_info.gpu.vendor_id = 0x10de;
  gpu_info.gpu.device_id = 0x0640;
  manager.InitializeForTesting(blacklist_json, gpu_info);

  EXPECT_TRUE(manager.GpuAccessAllowed(NULL));
  EXPECT_EQ(0u, manager.GetBlacklistedFeatureCount());

  // Now assume gpu process launches and full GPU info is collected.
  gpu_info.gl_renderer = "NVIDIA GeForce GT 120";
  manager.UpdateGpuInfo(gpu_info);
  EXPECT_TRUE(manager.GpuAccessAllowed(NULL));
  EXPECT_EQ(0u, manager.GetBlacklistedFeatureCount());
}

TEST_F(GpuDataManagerImplPrivateTest, DisableHardwareAcceleration) {
  GpuDataManagerImplPrivate manager;
  EXPECT_EQ(0u, manager.GetBlacklistedFeatureCount());
  std::string reason;
  EXPECT_TRUE(manager.GpuAccessAllowed(&reason));
  EXPECT_TRUE(reason.empty());

  manager.DisableHardwareAcceleration();
  EXPECT_FALSE(manager.GpuAccessAllowed(&reason));
  EXPECT_FALSE(reason.empty());
  EXPECT_EQ(static_cast<size_t>(NUMBER_OF_GPU_FEATURE_TYPES),
            manager.GetBlacklistedFeatureCount());
}

TEST_F(GpuDataManagerImplPrivateTest, SwiftShaderRendering) {
  // Blacklist, then register SwiftShader.
  GpuDataManagerImplPrivate manager;
  EXPECT_EQ(0u, manager.GetBlacklistedFeatureCount());
  EXPECT_TRUE(manager.GpuAccessAllowed(NULL));
  EXPECT_FALSE(manager.ShouldUseSwiftShader());

  manager.DisableHardwareAcceleration();
  EXPECT_FALSE(manager.GpuAccessAllowed(NULL));
  EXPECT_FALSE(manager.ShouldUseSwiftShader());

  // If SwiftShader is enabled, even if we blacklist GPU,
  // GPU process is still allowed.
  const base::FilePath test_path(FILE_PATH_LITERAL("AnyPath"));
  manager.RegisterSwiftShaderPath(test_path);
  EXPECT_TRUE(manager.ShouldUseSwiftShader());
  EXPECT_TRUE(manager.GpuAccessAllowed(NULL));
  EXPECT_EQ(1u, manager.GetBlacklistedFeatureCount());
  EXPECT_TRUE(
      manager.IsFeatureBlacklisted(GPU_FEATURE_TYPE_ACCELERATED_2D_CANVAS));
}

TEST_F(GpuDataManagerImplPrivateTest, SwiftShaderRendering2) {
  // Register SwiftShader, then blacklist.
  GpuDataManagerImplPrivate manager;
  EXPECT_EQ(0u, manager.GetBlacklistedFeatureCount());
  EXPECT_TRUE(manager.GpuAccessAllowed(NULL));
  EXPECT_FALSE(manager.ShouldUseSwiftShader());

  const base::FilePath test_path(FILE_PATH_LITERAL("AnyPath"));
  manager.RegisterSwiftShaderPath(test_path);
  EXPECT_EQ(0u, manager.GetBlacklistedFeatureCount());
  EXPECT_TRUE(manager.GpuAccessAllowed(NULL));
  EXPECT_FALSE(manager.ShouldUseSwiftShader());

  manager.DisableHardwareAcceleration();
  EXPECT_TRUE(manager.GpuAccessAllowed(NULL));
  EXPECT_TRUE(manager.ShouldUseSwiftShader());
  EXPECT_EQ(1u, manager.GetBlacklistedFeatureCount());
  EXPECT_TRUE(
      manager.IsFeatureBlacklisted(GPU_FEATURE_TYPE_ACCELERATED_2D_CANVAS));
}

TEST_F(GpuDataManagerImplPrivateTest, GpuInfoUpdate) {
  GpuDataManagerImplPrivate manager;

  TestObserver observer;
  manager.AddObserver(&observer);

  {
    base::RunLoop run_loop;
    run_loop.RunUntilIdle();
  }
  EXPECT_FALSE(observer.gpu_info_updated());

  GPUInfo gpu_info;
  manager.UpdateGpuInfo(gpu_info);
  {
    base::RunLoop run_loop;
    run_loop.RunUntilIdle();
  }
  EXPECT_TRUE(observer.gpu_info_updated());
}

TEST_F(GpuDataManagerImplPrivateTest, NoGpuInfoUpdateWithSwiftShader) {
  GpuDataManagerImplPrivate manager;

  manager.DisableHardwareAcceleration();
  const base::FilePath test_path(FILE_PATH_LITERAL("AnyPath"));
  manager.RegisterSwiftShaderPath(test_path);
  EXPECT_TRUE(manager.ShouldUseSwiftShader());
  EXPECT_TRUE(manager.GpuAccessAllowed(NULL));

  {
    base::RunLoop run_loop;
    run_loop.RunUntilIdle();
  }

  TestObserver observer;
  manager.AddObserver(&observer);
  {
    base::RunLoop run_loop;
    run_loop.RunUntilIdle();
  }
  EXPECT_FALSE(observer.gpu_info_updated());

  GPUInfo gpu_info;
  manager.UpdateGpuInfo(gpu_info);
  {
    base::RunLoop run_loop;
    run_loop.RunUntilIdle();
  }
  EXPECT_FALSE(observer.gpu_info_updated());
}

TEST_F(GpuDataManagerImplPrivateTest, GPUVideoMemoryUsageStatsUpdate) {
  GpuDataManagerImplPrivate manager;

  TestObserver observer;
  manager.AddObserver(&observer);

  {
    base::RunLoop run_loop;
    run_loop.RunUntilIdle();
  }
  EXPECT_FALSE(observer.video_memory_usage_stats_updated());

  GPUVideoMemoryUsageStats vram_stats;
  manager.UpdateVideoMemoryUsageStats(vram_stats);
  {
    base::RunLoop run_loop;
    run_loop.RunUntilIdle();
  }
  EXPECT_TRUE(observer.video_memory_usage_stats_updated());
}

base::Time GpuDataManagerImplPrivateTest::JustBeforeExpiration(
    const GpuDataManagerImplPrivate& manager) {
  return GetTimeForTesting() + base::TimeDelta::FromMilliseconds(
      manager.GetBlockAllDomainsDurationInMs()) -
      base::TimeDelta::FromMilliseconds(3);
}

base::Time GpuDataManagerImplPrivateTest::JustAfterExpiration(
    const GpuDataManagerImplPrivate& manager) {
  return GetTimeForTesting() + base::TimeDelta::FromMilliseconds(
      manager.GetBlockAllDomainsDurationInMs()) +
      base::TimeDelta::FromMilliseconds(3);
}

void GpuDataManagerImplPrivateTest::TestBlockingDomainFrom3DAPIs(
    GpuDataManagerImpl::DomainGuilt guilt_level) {
  GpuDataManagerImplPrivate manager;

  manager.BlockDomainFrom3DAPIsAtTime(GetDomain1ForTesting(),
                                      guilt_level,
                                      GetTimeForTesting());

  // This domain should be blocked no matter what.
  EXPECT_EQ(GpuDataManagerImpl::DOMAIN_BLOCK_STATUS_BLOCKED,
            manager.Are3DAPIsBlockedAtTime(GetDomain1ForTesting(),
                                           GetTimeForTesting()));
  EXPECT_EQ(GpuDataManagerImpl::DOMAIN_BLOCK_STATUS_BLOCKED,
            manager.Are3DAPIsBlockedAtTime(
                GetDomain1ForTesting(), JustBeforeExpiration(manager)));
  EXPECT_EQ(GpuDataManagerImpl::DOMAIN_BLOCK_STATUS_BLOCKED,
            manager.Are3DAPIsBlockedAtTime(
                GetDomain1ForTesting(), JustAfterExpiration(manager)));
}

void GpuDataManagerImplPrivateTest::TestUnblockingDomainFrom3DAPIs(
    GpuDataManagerImpl::DomainGuilt guilt_level) {
  GpuDataManagerImplPrivate manager;

  manager.BlockDomainFrom3DAPIsAtTime(GetDomain1ForTesting(),
                                      guilt_level,
                                      GetTimeForTesting());

  // Unblocking the domain should work.
  manager.UnblockDomainFrom3DAPIs(GetDomain1ForTesting());
  EXPECT_EQ(GpuDataManagerImpl::DOMAIN_BLOCK_STATUS_NOT_BLOCKED,
            manager.Are3DAPIsBlockedAtTime(GetDomain1ForTesting(),
                                           GetTimeForTesting()));
  EXPECT_EQ(GpuDataManagerImpl::DOMAIN_BLOCK_STATUS_NOT_BLOCKED,
            manager.Are3DAPIsBlockedAtTime(
                GetDomain1ForTesting(), JustBeforeExpiration(manager)));
  EXPECT_EQ(GpuDataManagerImpl::DOMAIN_BLOCK_STATUS_NOT_BLOCKED,
            manager.Are3DAPIsBlockedAtTime(
                GetDomain1ForTesting(), JustAfterExpiration(manager)));
}

TEST_F(GpuDataManagerImplPrivateTest, BlockGuiltyDomainFrom3DAPIs) {
  TestBlockingDomainFrom3DAPIs(GpuDataManagerImpl::DOMAIN_GUILT_KNOWN);
}

TEST_F(GpuDataManagerImplPrivateTest, BlockDomainOfUnknownGuiltFrom3DAPIs) {
  TestBlockingDomainFrom3DAPIs(GpuDataManagerImpl::DOMAIN_GUILT_UNKNOWN);
}

TEST_F(GpuDataManagerImplPrivateTest, BlockAllDomainsFrom3DAPIs) {
  GpuDataManagerImplPrivate manager;

  manager.BlockDomainFrom3DAPIsAtTime(GetDomain1ForTesting(),
                                      GpuDataManagerImpl::DOMAIN_GUILT_UNKNOWN,
                                      GetTimeForTesting());

  // Blocking of other domains should expire.
  EXPECT_EQ(GpuDataManagerImpl::DOMAIN_BLOCK_STATUS_ALL_DOMAINS_BLOCKED,
            manager.Are3DAPIsBlockedAtTime(
                GetDomain2ForTesting(), JustBeforeExpiration(manager)));
  EXPECT_EQ(GpuDataManagerImpl::DOMAIN_BLOCK_STATUS_NOT_BLOCKED,
            manager.Are3DAPIsBlockedAtTime(
                GetDomain2ForTesting(), JustAfterExpiration(manager)));
}

TEST_F(GpuDataManagerImplPrivateTest, UnblockGuiltyDomainFrom3DAPIs) {
  TestUnblockingDomainFrom3DAPIs(GpuDataManagerImpl::DOMAIN_GUILT_KNOWN);
}

TEST_F(GpuDataManagerImplPrivateTest, UnblockDomainOfUnknownGuiltFrom3DAPIs) {
  TestUnblockingDomainFrom3DAPIs(GpuDataManagerImpl::DOMAIN_GUILT_UNKNOWN);
}

TEST_F(GpuDataManagerImplPrivateTest, UnblockOtherDomainFrom3DAPIs) {
  GpuDataManagerImplPrivate manager;

  manager.BlockDomainFrom3DAPIsAtTime(GetDomain1ForTesting(),
                                      GpuDataManagerImpl::DOMAIN_GUILT_UNKNOWN,
                                      GetTimeForTesting());

  manager.UnblockDomainFrom3DAPIs(GetDomain2ForTesting());

  EXPECT_EQ(GpuDataManagerImpl::DOMAIN_BLOCK_STATUS_NOT_BLOCKED,
            manager.Are3DAPIsBlockedAtTime(
                GetDomain2ForTesting(), JustBeforeExpiration(manager)));

  // The original domain should still be blocked.
  EXPECT_EQ(GpuDataManagerImpl::DOMAIN_BLOCK_STATUS_BLOCKED,
            manager.Are3DAPIsBlockedAtTime(
                GetDomain1ForTesting(), JustBeforeExpiration(manager)));
}

TEST_F(GpuDataManagerImplPrivateTest, UnblockThisDomainFrom3DAPIs) {
  GpuDataManagerImplPrivate manager;

  manager.BlockDomainFrom3DAPIsAtTime(GetDomain1ForTesting(),
                                      GpuDataManagerImpl::DOMAIN_GUILT_UNKNOWN,
                                      GetTimeForTesting());

  manager.UnblockDomainFrom3DAPIs(GetDomain1ForTesting());

  // This behavior is debatable. Perhaps the GPU reset caused by
  // domain 1 should still cause other domains to be blocked.
  EXPECT_EQ(GpuDataManagerImpl::DOMAIN_BLOCK_STATUS_NOT_BLOCKED,
            manager.Are3DAPIsBlockedAtTime(
                GetDomain2ForTesting(), JustBeforeExpiration(manager)));
}

#if defined(OS_LINUX)
TEST_F(GpuDataManagerImplPrivateTest, SetGLStrings) {
  const char* kGLVendorMesa = "Tungsten Graphics, Inc";
  const char* kGLRendererMesa = "Mesa DRI Intel(R) G41";
  const char* kGLVersionMesa801 = "2.1 Mesa 8.0.1-DEVEL";

  GpuDataManagerImplPrivate manager;
  EXPECT_EQ(0u, manager.GetBlacklistedFeatureCount());
  EXPECT_TRUE(manager.GpuAccessAllowed(NULL));

  const std::string blacklist_json = LONG_STRING_CONST(
      {
        "name": "gpu blacklist",
        "version": "0.1",
        "entries": [
          {
            "id": 1,
            "vendor_id": "0x8086",
            "exceptions": [
              {
                "device_id": ["0x0042"],
                "driver_version": {
                  "op": ">=",
                  "number": "8.0.2"
                }
              }
            ],
            "features": [
              "webgl"
            ]
          }
        ]
      }
  );
  GPUInfo gpu_info;
  gpu_info.gpu.vendor_id = 0x8086;
  gpu_info.gpu.device_id = 0x0042;
  manager.InitializeForTesting(blacklist_json, gpu_info);

  // Not enough GPUInfo.
  EXPECT_TRUE(manager.GpuAccessAllowed(NULL));
  EXPECT_EQ(0u, manager.GetBlacklistedFeatureCount());

  // Now assume browser gets GL strings from local state.
  // The entry applies, blacklist more features than from the preliminary step.
  // However, GPU process is not blocked because this is all browser side and
  // happens before renderer launching.
  manager.SetGLStrings(kGLVendorMesa, kGLRendererMesa, kGLVersionMesa801);
  EXPECT_TRUE(manager.GpuAccessAllowed(NULL));
  EXPECT_EQ(1u, manager.GetBlacklistedFeatureCount());
  EXPECT_TRUE(manager.IsFeatureBlacklisted(GPU_FEATURE_TYPE_WEBGL));
}

TEST_F(GpuDataManagerImplPrivateTest, SetGLStringsNoEffects) {
  const char* kGLVendorMesa = "Tungsten Graphics, Inc";
  const char* kGLRendererMesa = "Mesa DRI Intel(R) G41";
  const char* kGLVersionMesa801 = "2.1 Mesa 8.0.1-DEVEL";
  const char* kGLVersionMesa802 = "2.1 Mesa 8.0.2-DEVEL";

  GpuDataManagerImplPrivate manager;
  EXPECT_EQ(0u, manager.GetBlacklistedFeatureCount());
  EXPECT_TRUE(manager.GpuAccessAllowed(NULL));

  const std::string blacklist_json = LONG_STRING_CONST(
      {
        "name": "gpu blacklist",
        "version": "0.1",
        "entries": [
          {
            "id": 1,
            "vendor_id": "0x8086",
            "exceptions": [
              {
                "device_id": ["0x0042"],
                "driver_version": {
                  "op": ">=",
                  "number": "8.0.2"
                }
              }
            ],
            "features": [
              "webgl"
            ]
          }
        ]
      }
  );
  GPUInfo gpu_info;
  gpu_info.gpu.vendor_id = 0x8086;
  gpu_info.gpu.device_id = 0x0042;
  gpu_info.gl_vendor = kGLVendorMesa;
  gpu_info.gl_renderer = kGLRendererMesa;
  gpu_info.gl_version = kGLVersionMesa801;
  gpu_info.driver_vendor = "Mesa";
  gpu_info.driver_version = "8.0.1";
  manager.InitializeForTesting(blacklist_json, gpu_info);

  // Full GPUInfo, the entry applies.
  EXPECT_TRUE(manager.GpuAccessAllowed(NULL));
  EXPECT_EQ(1u, manager.GetBlacklistedFeatureCount());
  EXPECT_TRUE(manager.IsFeatureBlacklisted(GPU_FEATURE_TYPE_WEBGL));

  // Now assume browser gets GL strings from local state.
  // SetGLStrings() has no effects because GPUInfo already got these strings.
  // (Otherwise the entry should not apply.)
  manager.SetGLStrings(kGLVendorMesa, kGLRendererMesa, kGLVersionMesa802);
  EXPECT_TRUE(manager.GpuAccessAllowed(NULL));
  EXPECT_EQ(1u, manager.GetBlacklistedFeatureCount());
  EXPECT_TRUE(manager.IsFeatureBlacklisted(GPU_FEATURE_TYPE_WEBGL));
}
#endif  // OS_LINUX

TEST_F(GpuDataManagerImplPrivateTest, GpuDriverBugListSingle) {
  GpuDataManagerImplPrivate manager;
  manager.gpu_driver_bugs_.insert(5);

  CommandLine command_line(0, NULL);
  manager.AppendGpuCommandLine(&command_line);

  EXPECT_TRUE(command_line.HasSwitch(switches::kGpuDriverBugWorkarounds));
  std::string args = command_line.GetSwitchValueASCII(
      switches::kGpuDriverBugWorkarounds);
  EXPECT_STREQ("5", args.c_str());
}

TEST_F(GpuDataManagerImplPrivateTest, GpuDriverBugListMultiple) {
  GpuDataManagerImplPrivate manager;
  manager.gpu_driver_bugs_.insert(5);
  manager.gpu_driver_bugs_.insert(7);

  CommandLine command_line(0, NULL);
  manager.AppendGpuCommandLine(&command_line);

  EXPECT_TRUE(command_line.HasSwitch(switches::kGpuDriverBugWorkarounds));
  std::string args = command_line.GetSwitchValueASCII(
      switches::kGpuDriverBugWorkarounds);
  EXPECT_STREQ("5,7", args.c_str());
}

TEST_F(GpuDataManagerImplPrivateTest, BlacklistAllFeatures) {
  GpuDataManagerImplPrivate manager;
  EXPECT_EQ(0u, manager.GetBlacklistedFeatureCount());
  std::string reason;
  EXPECT_TRUE(manager.GpuAccessAllowed(&reason));
  EXPECT_TRUE(reason.empty());

  const std::string blacklist_json = LONG_STRING_CONST(
      {
        "name": "gpu blacklist",
        "version": "0.1",
        "entries": [
          {
            "id": 1,
            "features": [
              "all"
            ]
          }
        ]
      }
  );

  GPUInfo gpu_info;
  gpu_info.gpu.vendor_id = 0x10de;
  gpu_info.gpu.device_id = 0x0640;
  manager.InitializeForTesting(blacklist_json, gpu_info);

  EXPECT_EQ(static_cast<size_t>(NUMBER_OF_GPU_FEATURE_TYPES),
            manager.GetBlacklistedFeatureCount());
  // TODO(zmo): remove the Linux specific behavior once we fix
  // crbug.com/238466.
#if defined(OS_LINUX)
  EXPECT_TRUE(manager.GpuAccessAllowed(&reason));
  EXPECT_TRUE(reason.empty());
#else
  EXPECT_FALSE(manager.GpuAccessAllowed(&reason));
  EXPECT_FALSE(reason.empty());
#endif
}

}  // namespace content
