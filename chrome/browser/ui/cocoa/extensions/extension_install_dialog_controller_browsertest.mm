// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "chrome/browser/ui/cocoa/extensions/extension_install_dialog_controller.h"

#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/cocoa/constrained_window/constrained_window_sheet_controller.h"
#import "chrome/browser/ui/cocoa/extensions/extension_install_prompt_test_utils.h"
#import "chrome/browser/ui/cocoa/extensions/extension_install_view_controller.h"
#include "chrome/browser/ui/cocoa/run_loop_testing.h"
#include "chrome/browser/ui/tab_contents/tab_contents.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/common/extensions/extension.h"
#include "chrome/test/base/in_process_browser_test.h"

using extensions::Extension;

class ExtensionInstallDialogControllerTest : public InProcessBrowserTest {
public:
  ExtensionInstallDialogControllerTest() {
    extension_ = chrome::LoadInstallPromptExtension();
  }

 protected:
  scoped_refptr<Extension> extension_;
};

IN_PROC_BROWSER_TEST_F(ExtensionInstallDialogControllerTest, BasicTest) {
  TabContents* tab = browser()->tab_strip_model()->GetTabContentsAt(0);

  chrome::MockExtensionInstallPromptDelegate delegate;
  ExtensionInstallPrompt::Prompt prompt =
      chrome::BuildExtensionInstallPrompt(extension_);

  ExtensionInstallDialogController* controller =
      new ExtensionInstallDialogController(tab->web_contents(),
                                           browser(),
                                           &delegate,
                                           prompt);

  scoped_nsobject<ConstrainedWindowController> window_controller(
      [controller->window_controller() retain]);
  scoped_nsobject<NSWindow> window([[window_controller window] retain]);
  EXPECT_TRUE([window isVisible]);

  // Press cancel to close the window
  [[controller->view_controller() cancelButton] performClick:nil];

  // Wait for the window to finish closing.
  ConstrainedWindowSheetController* sheetController =
      [ConstrainedWindowSheetController controllerForSheet:window];
  EXPECT_TRUE(sheetController);
  [sheetController endAnimationForSheet:window];
  chrome::testing::NSRunLoopRunAllPending();
  EXPECT_FALSE([window isVisible]);
}
