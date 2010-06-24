// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_COCOA_NOTIFICATIONS_BALLOON_CONTROLLER_H_
#define CHROME_BROWSER_COCOA_NOTIFICATIONS_BALLOON_CONTROLLER_H_

#import <Cocoa/Cocoa.h>

#include "base/scoped_nsobject.h"
#include "base/cocoa_protocols_mac.h"
#import "chrome/browser/cocoa/hover_close_button.h"
#import "chrome/browser/cocoa/notifications/balloon_view.h"
#import "chrome/browser/cocoa/notifications/balloon_view_host_mac.h"
#include "chrome/browser/notifications/balloon.h"

// The Balloon controller creates the view elements to display a
// notification balloon, resize it if the HTML contents of that
// balloon change, and move it when the collection of balloons is
// modified.
@interface BalloonController : NSWindowController<NSWindowDelegate> {
 @private
  // The balloon which represents the contents of this view. Weak pointer
  // owned by the browser's NotificationUIManager.
  Balloon* balloon_;

  // The view that contains the contents of the notification
  IBOutlet BalloonContentViewCocoa* htmlContainer_;

  // The view that contains the controls of the notification
  IBOutlet BalloonShelfViewCocoa* shelf_;

  // The close button.
  IBOutlet HoverCloseButton* closeButton_;

  // The origin label.
  IBOutlet NSTextField* originLabel_;

  // The options menu that appears when "options" is pressed.
  IBOutlet NSButton* optionsButton_;
  scoped_nsobject<NSMenu> optionsMenu_;

  // The host for the renderer of the HTML contents.
  scoped_ptr<BalloonViewHost> htmlContents_;
}

// Initialize with a balloon object containing the notification data.
- (id)initWithBalloon:(Balloon*)balloon;

// Callback function for the close button.
- (IBAction)closeButtonPressed:(id)sender;

// Callback function for the options button.
- (IBAction)optionsButtonPressed:(id)sender;

// Callback function for the "revoke" option in the menu.
- (IBAction)permissionRevoked:(id)sender;

// Closes the balloon.  Can be called by the bridge or by the close
// button handler.
- (void)closeBalloon:(bool)byUser;

// Repositions the view to match the position and size of the balloon.
// Called by the bridge when the size changes.
- (void)repositionToBalloon;

// The current size of the view, possibly subject to an animation completing.
- (int)desiredTotalWidth;
- (int)desiredTotalHeight;

// The BalloonHost
- (BalloonViewHost*)getHost;
@end

@interface BalloonController (UnitTesting)
- (void)initializeHost;
@end

#endif  // CHROME_BROWSER_COCOA_NOTIFICATIONS_BALLOON_CONTROLLER_H_
