// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/basictypes.h"
#include "base/command_line.h"
#if defined(OS_MACOSX)
#include "base/mac/mac_util.h"
#endif
#include "base/strings/stringprintf.h"
#include "base/test/trace_event_analyzer.h"
#include "base/win/windows_version.h"
#include "chrome/browser/extensions/extension_apitest.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/extension_test_message_listener.h"
#include "chrome/browser/extensions/tab_helper.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/fullscreen/fullscreen_controller.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/chrome_version_info.h"
#include "chrome/common/extensions/features/base_feature_provider.h"
#include "chrome/common/extensions/features/complex_feature.h"
#include "chrome/common/extensions/features/simple_feature.h"
#include "chrome/test/base/test_launcher_utils.h"
#include "chrome/test/base/test_switches.h"
#include "chrome/test/base/tracing.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/common/content_switches.h"
#include "extensions/common/feature_switch.h"
#include "extensions/common/features/feature.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/perf/perf_test.h"
#include "ui/compositor/compositor_switches.h"
#include "ui/gl/gl_switches.h"

namespace {

const char kExtensionId[] = "ddchlicdkolnonkihahngkmmmjnjlkkf";

enum TestFlags {
  kUseGpu              = 1 << 0, // Only execute test if --enable-gpu was given
                                 // on the command line.  This is required for
                                 // tests that run on GPU.
  kForceGpuComposited  = 1 << 1, // Force the test to use the compositor.
  kDisableVsync        = 1 << 2, // Do not limit framerate to vertical refresh.
                                 // when on GPU, nor to 60hz when not on GPU.
  kTestThroughWebRTC   = 1 << 3, // Send captured frames through webrtc
  kSmallWindow         = 1 << 4, // 1 = 800x600, 0 = 2000x1000

  kScaleQualityMask    = 3 << 5, // two bits select which scaling quality
  kScaleQualityDefault = 0 << 5, // to use on aura.
  kScaleQualityFast    = 1 << 5,
  kScaleQualityGood    = 2 << 5,
  kScaleQualityBest    = 3 << 5,
};

class TabCapturePerformanceTest
    : public ExtensionApiTest,
      public testing::WithParamInterface<int> {
 public:
  TabCapturePerformanceTest() {}

  bool HasFlag(TestFlags flag) const {
    return (GetParam() & flag) == flag;
  }

  bool IsGpuAvailable() const {
    return CommandLine::ForCurrentProcess()->HasSwitch("enable-gpu");
  }

  std::string ScalingMethod() const {
    switch (GetParam() & kScaleQualityMask) {
      case kScaleQualityFast:
        return "fast";
      case kScaleQualityGood:
        return "good";
      case kScaleQualityBest:
        return "best";
      default:
        return "";
    }
  }

  std::string GetSuffixForTestFlags() {
    std::string suffix;
    if (HasFlag(kForceGpuComposited))
      suffix += "_comp";
    if (HasFlag(kUseGpu))
      suffix += "_gpu";
    if (HasFlag(kDisableVsync))
      suffix += "_novsync";
    if (HasFlag(kTestThroughWebRTC))
      suffix += "_webrtc";
    if (!ScalingMethod().empty())
      suffix += "_scale" + ScalingMethod();
    if (HasFlag(kSmallWindow))
      suffix += "_small";
    return suffix;
  }

  virtual void SetUp() OVERRIDE {
    EnablePixelOutput();
    ExtensionApiTest::SetUp();
  }

  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE {
    if (!ScalingMethod().empty()) {
      command_line->AppendSwitchASCII(switches::kTabCaptureUpscaleQuality,
                                      ScalingMethod());
      command_line->AppendSwitchASCII(switches::kTabCaptureDownscaleQuality,
                                      ScalingMethod());
    }

    // UI tests boot up render views starting from about:blank. This causes
    // the renderer to start up thinking it cannot use the GPU. To work
    // around that, and allow the frame rate test to use the GPU, we must
    // pass kAllowWebUICompositing.
    command_line->AppendSwitch(switches::kAllowWebUICompositing);

    // Some of the tests may launch http requests through JSON or AJAX
    // which causes a security error (cross domain request) when the page
    // is loaded from the local file system ( file:// ). The following switch
    // fixes that error.
    command_line->AppendSwitch(switches::kAllowFileAccessFromFiles);

    if (HasFlag(kSmallWindow)) {
      command_line->AppendSwitchASCII(switches::kWindowSize, "800,600");
    } else {
      command_line->AppendSwitchASCII(switches::kWindowSize, "2000,1500");
    }

    if (!HasFlag(kUseGpu)) {
      command_line->AppendSwitch(switches::kDisableGpu);
    } else {
      command_line->AppendSwitch(switches::kForceCompositingMode);
    }

    if (HasFlag(kDisableVsync))
      command_line->AppendSwitch(switches::kDisableGpuVsync);

    command_line->AppendSwitchASCII(switches::kWhitelistedExtensionID,
                                    kExtensionId);
    ExtensionApiTest::SetUpCommandLine(command_line);
  }

  bool PrintResults(trace_analyzer::TraceAnalyzer *analyzer,
                    const std::string& test_name,
                    const std::string& event_name,
                    const std::string& unit) {
    trace_analyzer::TraceEventVector events;
    trace_analyzer::Query query =
        trace_analyzer::Query::EventNameIs(event_name) &&
        (trace_analyzer::Query::EventPhaseIs(TRACE_EVENT_PHASE_BEGIN) ||
         trace_analyzer::Query::EventPhaseIs(TRACE_EVENT_PHASE_ASYNC_BEGIN) ||
         trace_analyzer::Query::EventPhaseIs(TRACE_EVENT_PHASE_FLOW_BEGIN) ||
         trace_analyzer::Query::EventPhaseIs(TRACE_EVENT_PHASE_INSTANT));
    analyzer->FindEvents(query, &events);
    if (events.size() < 20) {
      LOG(ERROR) << "Not enough events of type " << event_name << " found.";
      return false;
    }

    // Ignore some events for startup/setup/caching.
    trace_analyzer::TraceEventVector rate_events(events.begin() + 3,
                                                 events.end() - 3);
    trace_analyzer::RateStats stats;
    if (!GetRateStats(rate_events, &stats, NULL)) {
      LOG(ERROR) << "GetRateStats failed";
      return false;
    }
    double mean_ms = stats.mean_us / 1000.0;
    double std_dev_ms = stats.standard_deviation_us / 1000.0;
    std::string mean_and_error = base::StringPrintf("%f,%f", mean_ms,
                                                    std_dev_ms);
    perf_test::PrintResultMeanAndError(test_name,
                                       GetSuffixForTestFlags(),
                                       event_name,
                                       mean_and_error,
                                       unit,
                                       true);
    return true;
  }

  void RunTest(const std::string& test_name) {
    if (HasFlag(kUseGpu) && !IsGpuAvailable()) {
      LOG(WARNING) <<
          "Test skipped: requires gpu. Pass --enable-gpu on the command "
          "line if use of GPU is desired.";
      return;
    }

    std::string json_events;
    ASSERT_TRUE(tracing::BeginTracing("test_fps,mirroring"));
    std::string page = "performance.html";
    page += HasFlag(kTestThroughWebRTC) ? "?WebRTC=1" : "?WebRTC=0";
    // Ideally we'd like to run a higher capture rate when vsync is disabled,
    // but libjingle currently doesn't allow that.
    // page += HasFlag(kDisableVsync) ? "&fps=300" : "&fps=30";
    page += "&fps=30";
    ASSERT_TRUE(RunExtensionSubtest("tab_capture", page)) << message_;
    ASSERT_TRUE(tracing::EndTracing(&json_events));
    scoped_ptr<trace_analyzer::TraceAnalyzer> analyzer;
    analyzer.reset(trace_analyzer::TraceAnalyzer::Create(json_events));

    // Only one of these PrintResults should actually print something.
    // The printed result will be the average time between frames in the
    // browser window.
    bool sw_frames = PrintResults(analyzer.get(),
                                  test_name,
                                  "TestFrameTickSW",
                                  "frame_time");
    bool gpu_frames = PrintResults(analyzer.get(),
                                   test_name,
                                   "TestFrameTickGPU",
                                   "frame_time");
    EXPECT_TRUE(sw_frames || gpu_frames);
    EXPECT_NE(sw_frames, gpu_frames);

    // This prints out the average time between capture events.
    // As the capture frame rate is capped at 30fps, this score
    // cannot get any better than 33.33 ms.
    EXPECT_TRUE(PrintResults(analyzer.get(),
                             test_name,
                             "Capture",
                             "capture_time"));
  }
};

}  // namespace

IN_PROC_BROWSER_TEST_P(TabCapturePerformanceTest, Performance) {
  RunTest("TabCapturePerformance");
}

// Note: First argument is optional and intentionally left blank.
// (it's a prefix for the generated test cases)
INSTANTIATE_TEST_CASE_P(
    ,
    TabCapturePerformanceTest,
    testing::Values(
        0,
        kUseGpu | kForceGpuComposited,
        kDisableVsync,
        kDisableVsync | kUseGpu | kForceGpuComposited,
        kTestThroughWebRTC,
        kTestThroughWebRTC | kUseGpu | kForceGpuComposited,
        kTestThroughWebRTC | kDisableVsync,
        kTestThroughWebRTC | kDisableVsync | kUseGpu | kForceGpuComposited));

#if defined(USE_AURA)
// TODO(hubbe):
// These are temporary tests for the purpose of determining what the
// appropriate scaling quality is. Once that has been determined,
// these tests will be removed.

const int kScalingTestBase =
    kTestThroughWebRTC | kDisableVsync | kUseGpu | kForceGpuComposited;

INSTANTIATE_TEST_CASE_P(
    ScalingTests,
    TabCapturePerformanceTest,
    testing::Values(
        kScalingTestBase | kScaleQualityFast,
        kScalingTestBase | kScaleQualityGood,
        kScalingTestBase | kScaleQualityBest,
        kScalingTestBase | kScaleQualityFast | kSmallWindow,
        kScalingTestBase | kScaleQualityGood | kSmallWindow,
        kScalingTestBase | kScaleQualityBest | kSmallWindow));

#endif  // USE_AURA
