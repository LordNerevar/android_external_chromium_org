// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>

#include "base/command_line.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/common/view_messages.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "content/public/test/content_browser_test.h"
#include "content/public/test/content_browser_test_utils.h"
#include "content/public/test/test_navigation_observer.h"
#include "content/public/test/test_utils.h"
#include "content/shell/browser/shell.h"
#include "content/shell/common/shell_switches.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "ui/compositor/compositor_switches.h"

namespace content {

class ScreenOrientationBrowserTest : public ContentBrowserTest  {
 public:
  ScreenOrientationBrowserTest() {
  }

  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE {
    command_line->AppendSwitch(
        switches::kEnableExperimentalWebPlatformFeatures);
  }

  virtual void SetUp() OVERRIDE {
    // Painting has to happen otherwise the Resize messages will be added on top
    // of each other without properly ack-painting which will fail and crash.
    UseSoftwareCompositing();

    ContentBrowserTest::SetUp();
  }

 protected:
  void SendFakeScreenOrientation(unsigned angle, const std::string& strType) {
    RenderWidgetHost* rwh = shell()->web_contents()->GetRenderWidgetHostView()
        ->GetRenderWidgetHost();
    blink::WebScreenInfo screen_info;
    rwh->GetWebScreenInfo(&screen_info);
    screen_info.orientationAngle = angle;

    blink::WebScreenOrientationType type = blink::WebScreenOrientationUndefined;
    if (strType == "portrait-primary") {
      type = blink::WebScreenOrientationPortraitPrimary;
    } else if (strType == "portrait-secondary") {
      type = blink::WebScreenOrientationPortraitSecondary;
    } else if (strType == "landscape-primary") {
      type = blink::WebScreenOrientationLandscapePrimary;
    } else if (strType == "landscape-secondary") {
      type = blink::WebScreenOrientationLandscapeSecondary;
    }
    ASSERT_NE(blink::WebScreenOrientationUndefined, type);
    screen_info.orientationType = type;

    ViewMsg_Resize_Params params;
    params.screen_info = screen_info;
    params.new_size = gfx::Size(0, 0);
    params.physical_backing_size = gfx::Size(300, 300);
    params.overdraw_bottom_height = 0.f;
    params.resizer_rect = gfx::Rect();
    params.is_fullscreen = false;
    rwh->Send(new ViewMsg_Resize(rwh->GetRoutingID(), params));
  }

  int GetOrientationAngle() {
    int angle;
    ExecuteScriptAndGetValue(shell()->web_contents()->GetMainFrame(),
                             "screen.orientation.angle")->GetAsInteger(&angle);
    return angle;
  }

  std::string GetOrientationType() {
    std::string type;
    ExecuteScriptAndGetValue(shell()->web_contents()->GetMainFrame(),
                             "screen.orientation.type")->GetAsString(&type);
    return type;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ScreenOrientationBrowserTest);
};

// This test doesn't work on MacOS X but the reason is mostly because it is not
// used Aura. It could be set as !defined(OS_MACOSX) but the rule below will
// actually support MacOS X if and when it switches to Aura.
#if defined(USE_AURA) || defined(OS_ANDROID)
IN_PROC_BROWSER_TEST_F(ScreenOrientationBrowserTest, ScreenOrientationChange) {
  std::string types[] = { "portrait-primary",
                          "portrait-secondary",
                          "landscape-primary",
                          "landscape-secondary" };
  GURL test_url = GetTestUrl("screen_orientation",
                             "screen_orientation_orientationchange.html");

  TestNavigationObserver navigation_observer(
      shell()->web_contents(), 1
  // Android doesn't paint (ie. UseSoftwareCompositing() has no effect) so we
  // shouldn't wait for the first paint.
  #if !defined(OS_ANDROID)
    , TestNavigationObserver::FirstPaintRequired
  #endif
  );
  shell()->LoadURL(test_url);
  navigation_observer.Wait();

  int angle = GetOrientationAngle();

  for (int i = 0; i < 4; ++i) {
    angle = (angle + 90) % 360;
    SendFakeScreenOrientation(angle, types[i]);

    TestNavigationObserver navigation_observer(shell()->web_contents());
    navigation_observer.Wait();
    EXPECT_EQ(angle, GetOrientationAngle());
    EXPECT_EQ(types[i], GetOrientationType());
  }
}
#endif // defined(USE_AURA) || defined(OS_ANDROID)

} // namespace content
