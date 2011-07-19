// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VIEWS_DESKTOP_DESKTOP_WINDOW_ROOT_VIEW_H_
#define VIEWS_DESKTOP_DESKTOP_WINDOW_ROOT_VIEW_H_

#include "views/widget/root_view.h"

namespace views {
namespace desktop {

class DesktopWindowView;

class DesktopWindowRootView : public internal::RootView {
 public:
  DesktopWindowRootView(DesktopWindowView* desktop_window_view, Widget* window);
  virtual ~DesktopWindowRootView();

 private:
  // Overridden from RootView:
  virtual bool OnMousePressed(const MouseEvent& event) OVERRIDE;

  DesktopWindowView* desktop_window_view_;

  DISALLOW_COPY_AND_ASSIGN(DesktopWindowRootView);
};

}  // namespace desktop
}  // namespace views

#endif  // VIEWS_DESKTOP_DESKTOP_WINDOW_ROOT_VIEW_H_
