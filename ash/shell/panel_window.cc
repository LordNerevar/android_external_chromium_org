// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/shell/panel_window.h"

#include "ash/wm/panel_frame_view.h"
#include "ash/wm/toplevel_frame_view.h"
#include "base/utf_string_conversions.h"
#include "ui/aura/window.h"
#include "ui/gfx/canvas.h"
#include "ui/views/widget/widget.h"

namespace {
const int kMinWidth = 100;
const int kMinHeight = 100;
const int kDefaultWidth = 200;
const int kDefaultHeight = 300;
}

namespace ash {

// static
views::Widget* PanelWindow::CreatePanelWindow(const gfx::Rect& rect) {
  PanelWindow* panel_window = new PanelWindow("Example Panel Window");
  panel_window->params().bounds = rect;
  return panel_window->CreateWidget();
}

PanelWindow::PanelWindow(const std::string& name)
    : name_(name),
      params_(views::Widget::InitParams::TYPE_PANEL) {
  params_.delegate = this;
}

PanelWindow::~PanelWindow() {
}

views::Widget* PanelWindow::CreateWidget() {
  views::Widget* widget = new views::Widget;

  if (params().bounds.width() == 0)
    params().bounds.set_width(kDefaultWidth);
  if (params().bounds.height() == 0)
    params().bounds.set_height(kDefaultHeight);

  widget->Init(params());
  widget->GetNativeView()->SetName(name_);
  widget->Show();

  return widget;
}

gfx::Size PanelWindow::GetPreferredSize() {
  return gfx::Size(kMinWidth, kMinHeight);
}

void PanelWindow::OnPaint(gfx::Canvas* canvas) {
  canvas->FillRect(GetLocalBounds(), SK_ColorGREEN);
}

string16 PanelWindow::GetWindowTitle() const {
  return ASCIIToUTF16(name_);
}

views::View* PanelWindow::GetContentsView() {
  return this;
}

bool PanelWindow::CanResize() const {
  return true;
}

bool PanelWindow::CanMaximize() const {
  return false;
}

views::NonClientFrameView* PanelWindow::CreateNonClientFrameView() {
  return new PanelFrameView();
}

}  // namespace ash
