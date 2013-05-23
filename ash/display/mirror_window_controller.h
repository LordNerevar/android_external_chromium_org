// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_DISPLAY_MIRROR_WINDOW_CONTROLLER_H_
#define ASH_DISPLAY_MIRROR_WINDOW_CONTROLLER_H_

#include "ash/ash_export.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/point.h"

namespace aura {
class RootWindow;
class Window;
}

namespace ash {
namespace test{
class MirrorWindowTestApi;
}

namespace internal {
class DisplayInfo;
class CursorWindowDelegate;

// An object that copies the content of the primary root window to a
// mirror window. This also draws a mouse cursor as the mouse cursor
// is typically drawn by the window system.
class MirrorWindowController {
 public:
  MirrorWindowController();
  ~MirrorWindowController();

  // Updates the root window's bounds using |display_info|.
  // Creates the new root window if one doesn't exist.
  void UpdateWindow(const DisplayInfo& display_info);

  void Close();

  // Updates the mirrored cursor location,shape and
  // visibility.
  void UpdateCursorLocation();
  void SetMirroredCursor(gfx::NativeCursor cursor);
  void SetMirroredCursorVisibility(bool visible);

 private:
  friend class test::MirrorWindowTestApi;

  int current_cursor_type_;
  aura::Window* cursor_window_;  // owned by root window.
  scoped_ptr<aura::RootWindow> root_window_;
  scoped_ptr<CursorWindowDelegate> cursor_window_delegate_;
  gfx::Point hot_point_;

  DISALLOW_COPY_AND_ASSIGN(MirrorWindowController);
};

}  // namespace internal
}  // namespace ash

#endif  // ASH_DISPLAY_MIRROR_WINDOW_CONTROLLER_H_
