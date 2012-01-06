// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_UI_BASE_TYPES_H_
#define UI_BASE_UI_BASE_TYPES_H_
#pragma once

namespace ui {

// Window "show" state.  These values are written to disk so should not be
// changed.
enum WindowShowState {
  // A default un-set state.
  SHOW_STATE_DEFAULT    = 0,
  SHOW_STATE_NORMAL     = 1,
  SHOW_STATE_MINIMIZED  = 2,
  SHOW_STATE_MAXIMIZED  = 3,
  SHOW_STATE_INACTIVE   = 4,  // Views only, not persisted.
  SHOW_STATE_FULLSCREEN = 5,
  SHOW_STATE_END        = 6   // The end of show state enum.
};

// Dialog button identifiers used to specify which buttons to show the user.
enum DialogButton {
  DIALOG_BUTTON_NONE   = 0,
  DIALOG_BUTTON_OK     = 1,
  DIALOG_BUTTON_CANCEL = 2,
};

// Specifies the type of modality applied to a window. Different modal
// treatments may be handled differently by the window manager.
enum ModalType {
  MODAL_TYPE_NONE   = 0,  // Window is not modal.
  MODAL_TYPE_WINDOW = 1,  // Window is modal to its transient parent.
  MODAL_TYPE_SYSTEM = 2   // Window is modal to all other windows.
};

}  // namespace ui

#endif  // UI_BASE_UI_BASE_TYPES_H_
