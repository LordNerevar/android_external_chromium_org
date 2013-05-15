// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_MESSAGE_CENTER_COCOA_STATUS_ITEM_VIEW_H_
#define UI_MESSAGE_CENTER_COCOA_STATUS_ITEM_VIEW_H_

#import <AppKit/AppKit.h>

#include "base/mac/scoped_block.h"
#include "base/memory/scoped_nsobject.h"
#include "ui/message_center/message_center_export.h"

namespace message_center {

// Callback block for when the status item is clicked.
typedef void(^StatusItemClickedCallack)();

}  // namespace message_center

// This view is meant to be used with a NSStatusItem. It will fire a callback
// when it is clicked. It draws a small icon and the unread count, if greater
// than zero, to the icon's right. It can also paint the highlight background
// pattern outisde of a mouse event sequence, for when an attached window is
// open.
MESSAGE_CENTER_EXPORT
@interface MCStatusItemView : NSView {
 @private
  // The status item.
  scoped_nsobject<NSStatusItem> statusItem_;

  // Callback issued when the status item is clicked.
  base::mac::ScopedBlock<message_center::StatusItemClickedCallack> callback_;

  // The unread count number to be drawn next to the icon.
  size_t unreadCount_;

  // Whether or not to force the highlight pattern to be drawn.
  BOOL highlight_;

  // Whether or not the view is currently handling mouse events and should
  // draw the highlight pattern.
  BOOL inMouseEventSequence_;
}

@property(copy, nonatomic) message_center::StatusItemClickedCallack callback;
@property(nonatomic) size_t unreadCount;
@property(nonatomic) BOOL highlight;

// Designated initializer. This will take ownership of |item| and set self as
// its view.
- (id)initWithStatusItem:(NSStatusItem*)item;

@end

#endif  // UI_MESSAGE_CENTER_COCOA_STATUS_ITEM_VIEW_H_
