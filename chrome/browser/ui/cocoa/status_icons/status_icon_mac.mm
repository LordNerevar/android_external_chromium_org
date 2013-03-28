// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/cocoa/status_icons/status_icon_mac.h"

#import <AppKit/AppKit.h>

#include "base/logging.h"
#include "base/strings/sys_string_conversions.h"
#import "chrome/browser/ui/cocoa/menu_controller.h"
#include "skia/ext/skia_utils_mac.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/image/image_skia.h"

@interface StatusItemController : NSObject {
  StatusIconMac* statusIcon_; // weak
}
- (id)initWithIcon:(StatusIconMac*)icon;
- (void)handleClick:(id)sender;

@end // @interface StatusItemController

@implementation StatusItemController

- (id)initWithIcon:(StatusIconMac*)icon {
  statusIcon_ = icon;
  return self;
}

- (void)handleClick:(id)sender {
  // Pass along the click notification to our owner.
  DCHECK(statusIcon_);
  // Bring up the status icon menu if there is one, relay the click event
  // otherwise.
  if (!statusIcon_->HasStatusIconMenu())
    statusIcon_->DispatchClickEvent();
}

@end

StatusIconMac::StatusIconMac()
    : item_(NULL) {
  controller_.reset([[StatusItemController alloc] initWithIcon:this]);
}

StatusIconMac::~StatusIconMac() {
  // Remove the status item from the status bar.
  if (item_)
    [[NSStatusBar systemStatusBar] removeStatusItem:item_];
}

NSStatusItem* StatusIconMac::item() {
  if (!item_.get()) {
    // Create a new status item.
    item_.reset([[[NSStatusBar systemStatusBar]
                  statusItemWithLength:NSSquareStatusItemLength] retain]);
    [item_ setEnabled:YES];
    [item_ setTarget:controller_];
    [item_ setAction:@selector(handleClick:)];
    [item_ setHighlightMode:YES];
  }
  return item_.get();
}

void StatusIconMac::SetImage(const gfx::ImageSkia& image) {
  if (!image.isNull()) {
    NSImage* ns_image = gfx::SkBitmapToNSImage(*image.bitmap());
    if (ns_image)
      [item() setImage:ns_image];
  }
}

void StatusIconMac::SetPressedImage(const gfx::ImageSkia& image) {
  if (!image.isNull()) {
    NSImage* ns_image = gfx::SkBitmapToNSImage(*image.bitmap());
    if (ns_image)
      [item() setAlternateImage:ns_image];
  }
}

void StatusIconMac::SetToolTip(const string16& tool_tip) {
  // If we have a status icon menu, make the tool tip part of the menu instead
  // of a pop-up tool tip when hovering the mouse over the image.
  toolTip_.reset([base::SysUTF16ToNSString(tool_tip) retain]);
  if (menu_.get()) {
    SetToolTip(nil);
    CreateMenu([menu_ model], toolTip_.get());
  } else {
    SetToolTip(toolTip_.get());
  }
}

void StatusIconMac::DisplayBalloon(const gfx::ImageSkia& icon,
                                   const string16& title,
                                   const string16& contents) {
  notification_.DisplayBalloon(icon, title, contents);
}

bool StatusIconMac::HasStatusIconMenu() {
  return menu_.get() != nil;
}

void StatusIconMac::UpdatePlatformContextMenu(ui::MenuModel* model) {
  if (!model) {
    menu_.reset();
  } else {
    SetToolTip(nil);
    CreateMenu(model, toolTip_.get());
  }
}

void StatusIconMac::CreateMenu(ui::MenuModel* model, NSString* toolTip) {
  DCHECK(model);

  if (!toolTip) {
    menu_.reset([[MenuController alloc] initWithModel:model
                               useWithPopUpButtonCell:NO]);
  } else {
    // When using a popup button cell menu controller, an extra blank item is
    // added at index 0. Use this item for the tooltip.
    menu_.reset([[MenuController alloc] initWithModel:model
                               useWithPopUpButtonCell:YES]);
    NSMenuItem* toolTipItem = [[menu_ menu] itemAtIndex:0];
    [toolTipItem setTitle:toolTip];
  }
  [item() setMenu:[menu_ menu]];
}

void StatusIconMac::SetToolTip(NSString* toolTip) {
  [item() setToolTip:toolTip];
}
