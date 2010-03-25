// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_COCOA_EXTENSIONS_BROWSER_ACTION_BUTTON_H_
#define CHROME_BROWSER_COCOA_EXTENSIONS_BROWSER_ACTION_BUTTON_H_

#import <Cocoa/Cocoa.h>

#import "base/scoped_nsobject.h"
#include "base/scoped_ptr.h"
#include "chrome/browser/cocoa/toolbar_button_cell.h"

class Extension;
class ExtensionAction;
class ExtensionImageTrackerBridge;
class Profile;

// Fired when the Browser Action's state has changed. Usually the image needs to
// be updated.
extern const NSString* kBrowserActionButtonUpdatedNotification;

extern const CGFloat kBrowserActionWidth;

@interface BrowserActionButton : NSButton {
 @private
  // Bridge to proxy Chrome notifications to the Obj-C class as well as load the
  // extension's icon.
  scoped_ptr<ExtensionImageTrackerBridge> imageLoadingBridge_;

  // The default icon of the Button.
  scoped_nsobject<NSImage> defaultIcon_;

  // The icon specific to the active tab.
  scoped_nsobject<NSImage> tabSpecificIcon_;

  // The extension for this button. Weak.
  Extension* extension_;

  // The ID of the active tab.
  int tabId_;
}

- (id)initWithExtension:(Extension*)extension
                profile:(Profile*)profile
                  tabId:(int)tabId;

- (void)setDefaultIcon:(NSImage*)image;

- (void)setTabSpecificIcon:(NSImage*)image;

- (void)updateState;

// Returns a pointer to an autoreleased NSImage with the badge, shadow and
// cell image drawn into it.
- (NSImage*)compositedImage;

@property(readwrite, nonatomic) int tabId;
@property(readonly, nonatomic) Extension* extension;

@end

@interface BrowserActionCell : ToolbarButtonCell {
 @private
  // The current tab ID used when drawing the cell.
  int tabId_;

  // The action we're drawing the cell for. Weak.
  ExtensionAction* extensionAction_;
}

@property(readwrite, nonatomic) int tabId;
@property(readwrite, nonatomic) ExtensionAction* extensionAction;

@end

#endif  // CHROME_BROWSER_COCOA_EXTENSIONS_BROWSER_ACTION_BUTTON_H_
