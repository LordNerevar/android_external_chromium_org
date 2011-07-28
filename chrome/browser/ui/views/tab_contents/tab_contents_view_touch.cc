// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/tab_contents/tab_contents_view_touch.h"

#include <X11/extensions/XInput2.h>
#undef Status

#include "base/string_util.h"
#include "build/build_config.h"
#include "chrome/browser/renderer_host/render_widget_host_view_views.h"
#include "chrome/browser/ui/views/sad_tab_view.h"
#include "chrome/browser/ui/views/tab_contents/render_view_context_menu_views.h"
#include "content/browser/renderer_host/render_view_host.h"
#include "content/browser/renderer_host/render_view_host_factory.h"
#include "content/browser/tab_contents/interstitial_page.h"
#include "content/browser/tab_contents/tab_contents.h"
#include "content/browser/tab_contents/tab_contents_delegate.h"
#include "ui/gfx/canvas_skia_paint.h"
#include "ui/gfx/point.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/size.h"
#include "views/controls/native/native_view_host.h"
#include "views/focus/focus_manager.h"
#include "views/focus/view_storage.h"
#include "views/layout/fill_layout.h"
#include "views/widget/widget.h"

using WebKit::WebDragOperation;
using WebKit::WebDragOperationsMask;
using WebKit::WebInputEvent;

// static
const char TabContentsViewTouch::kViewClassName[] =
    "browser/ui/views/tab_contents/TabContentsViewTouch";

// static
TabContentsView* TabContentsView::Create(TabContents* tab_contents) {
  return new TabContentsViewTouch(tab_contents);
}

TabContentsViewTouch::TabContentsViewTouch(TabContents* tab_contents)
    : tab_contents_(tab_contents),
      sad_tab_(NULL),
      ignore_next_char_event_(false) {
  last_focused_view_storage_id_ =
      views::ViewStorage::GetInstance()->CreateStorageID();
  SetLayoutManager(new views::FillLayout());
}

TabContentsViewTouch::~TabContentsViewTouch() {
  // Make sure to remove any stored view we may still have in the ViewStorage.
  //
  // It is possible the view went away before us, so we only do this if the
  // view is registered.
  views::ViewStorage* view_storage = views::ViewStorage::GetInstance();
  if (view_storage->RetrieveView(last_focused_view_storage_id_) != NULL)
    view_storage->RemoveView(last_focused_view_storage_id_);
}

void TabContentsViewTouch::AttachConstrainedWindow(
    ConstrainedWindowGtk* constrained_window) {
  // TODO(anicolao): reimplement all dialogs as WebUI
  NOTIMPLEMENTED();
}

void TabContentsViewTouch::RemoveConstrainedWindow(
    ConstrainedWindowGtk* constrained_window) {
  // TODO(anicolao): reimplement all dialogs as WebUI
  NOTIMPLEMENTED();
}

void TabContentsViewTouch::CreateView(const gfx::Size& initial_size) {
  SetBoundsRect(gfx::Rect(bounds().origin(), initial_size));
}

RenderWidgetHostView* TabContentsViewTouch::CreateViewForWidget(
    RenderWidgetHost* render_widget_host) {
  if (render_widget_host->view()) {
    // During testing, the view will already be set up in most cases to the
    // test view, so we don't want to clobber it with a real one. To verify that
    // this actually is happening (and somebody isn't accidentally creating the
    // view twice), we check for the RVH Factory, which will be set when we're
    // making special ones (which go along with the special views).
    DCHECK(RenderViewHostFactory::has_factory());
    return render_widget_host->view();
  }

  // If we were showing sad tab, remove it now.
  if (sad_tab_ != NULL) {
    RemoveChildView(sad_tab_.get());
    sad_tab_.reset();
  }

  RenderWidgetHostViewViews* view =
      new RenderWidgetHostViewViews(render_widget_host);
  AddChildView(view);
  view->Show();
  view->InitAsChild();

  // TODO(anicolao): implement drag'n'drop hooks if needed

  return view;
}

gfx::NativeView TabContentsViewTouch::GetNativeView() const {
  return GetWidget()->GetNativeView();
}

gfx::NativeView TabContentsViewTouch::GetContentNativeView() const {
  RenderWidgetHostView* rwhv = tab_contents_->GetRenderWidgetHostView();
  if (!rwhv)
    return NULL;
  return rwhv->GetNativeView();
}

gfx::NativeWindow TabContentsViewTouch::GetTopLevelNativeWindow() const {
  GtkWidget* window = gtk_widget_get_ancestor(GetNativeView(), GTK_TYPE_WINDOW);
  return window ? GTK_WINDOW(window) : NULL;
}

void TabContentsViewTouch::GetContainerBounds(gfx::Rect* out) const {
  *out = bounds();
}

void TabContentsViewTouch::StartDragging(const WebDropData& drop_data,
                                         WebDragOperationsMask ops,
                                         const SkBitmap& image,
                                         const gfx::Point& image_offset) {
  // TODO(anicolao): implement dragging
}

void TabContentsViewTouch::SetPageTitle(const std::wstring& title) {
  // TODO(anicolao): figure out if there's anything useful to do here
}

void TabContentsViewTouch::OnTabCrashed(base::TerminationStatus status,
                                        int /* error_code */) {
  if (sad_tab_ != NULL)
    return;

  sad_tab_.reset(new SadTabView(
      tab_contents_,
      status == base::TERMINATION_STATUS_PROCESS_WAS_KILLED ?
          SadTabView::KILLED : SadTabView::CRASHED));
  RemoveAllChildViews(true);
  AddChildView(sad_tab_.get());
  Layout();
}

void TabContentsViewTouch::SizeContents(const gfx::Size& size) {
  WasSized(size);

  // We need to send this immediately.
  RenderWidgetHostView* rwhv = tab_contents_->GetRenderWidgetHostView();
  if (rwhv)
    rwhv->SetSize(size);
}

void TabContentsViewTouch::RenderViewCreated(RenderViewHost* host) {
}

void TabContentsViewTouch::Focus() {
  if (tab_contents_->interstitial_page()) {
    tab_contents_->interstitial_page()->Focus();
    return;
  }

  if (tab_contents_->is_crashed() && sad_tab_ != NULL) {
    sad_tab_->RequestFocus();
    return;
  }

  RenderWidgetHostView* rwhv = tab_contents_->GetRenderWidgetHostView();
  if (rwhv)
    rwhv->Focus();
}

void TabContentsViewTouch::SetInitialFocus() {
  if (tab_contents_->FocusLocationBarByDefault())
    tab_contents_->SetFocusToLocationBar(false);
  else
    Focus();
}

void TabContentsViewTouch::StoreFocus() {
  views::ViewStorage* view_storage = views::ViewStorage::GetInstance();

  if (view_storage->RetrieveView(last_focused_view_storage_id_) != NULL)
    view_storage->RemoveView(last_focused_view_storage_id_);

  views::FocusManager* focus_manager =
      views::FocusManager::GetFocusManagerForNativeView(GetNativeView());
  if (focus_manager) {
    // |focus_manager| can be NULL if the tab has been detached but still
    // exists.
    views::View* focused_view = focus_manager->GetFocusedView();
    if (focused_view)
      view_storage->StoreView(last_focused_view_storage_id_, focused_view);
  }
}

void TabContentsViewTouch::RestoreFocus() {
  views::ViewStorage* view_storage = views::ViewStorage::GetInstance();
  views::View* last_focused_view =
      view_storage->RetrieveView(last_focused_view_storage_id_);
  if (!last_focused_view) {
    SetInitialFocus();
  } else {
    views::FocusManager* focus_manager =
        views::FocusManager::GetFocusManagerForNativeView(GetNativeView());

    // If you hit this DCHECK, please report it to Jay (jcampan).
    DCHECK(focus_manager != NULL) << "No focus manager when restoring focus.";

    if (last_focused_view->IsFocusableInRootView() && focus_manager &&
        focus_manager->ContainsView(last_focused_view)) {
      last_focused_view->RequestFocus();
    } else {
      // The focused view may not belong to the same window hierarchy (e.g.
      // if the location bar was focused and the tab is dragged out), or it may
      // no longer be focusable (e.g. if the location bar was focused and then
      // we switched to fullscreen mode).  In that case we default to the
      // default focus.
      SetInitialFocus();
    }
    view_storage->RemoveView(last_focused_view_storage_id_);
  }
}

void TabContentsViewTouch::UpdatePreferredSize(const gfx::Size& pref_size) {
}

bool TabContentsViewTouch::IsDoingDrag() const {
  return false;
}

void TabContentsViewTouch::CancelDragAndCloseTab() {
}

bool TabContentsViewTouch::IsEventTracking() const {
  return false;
}

void TabContentsViewTouch::CloseTabAfterEventTracking() {
}

void TabContentsViewTouch::GetViewBounds(gfx::Rect* out) const {
  out->SetRect(x(), y(), width(), height());
}

bool TabContentsViewTouch::OnMousePressed(const views::MouseEvent& event) {
  if ((event.flags() & (ui::EF_LEFT_BUTTON_DOWN |
                        ui::EF_RIGHT_BUTTON_DOWN |
                        ui::EF_MIDDLE_BUTTON_DOWN))) {
    return false;
  }

  // It is necessary to look at the native event to determine what special
  // button was pressed.
  views::NativeEvent2 native_event = event.native_event_2();
  if (!native_event)
    return false;

  int button = 0;
  switch (native_event->type) {
    case ButtonPress: {
      button = native_event->xbutton.button;
      break;
    }
    case GenericEvent: {
      XIDeviceEvent* xievent =
          static_cast<XIDeviceEvent*>(native_event->xcookie.data);
      button = xievent->detail;
      break;
    }
    default:
      break;
  }

  switch (button) {
    case 8:
      tab_contents_->controller().GoBack();
      return true;
    case 9:
      tab_contents_->controller().GoForward();
      return true;
  }

  return false;
}

void TabContentsViewTouch::OnBoundsChanged(const gfx::Rect& previous_bounds) {
  if (IsVisibleInRootView())
    WasSized(size());
}

void TabContentsViewTouch::OnPaint(gfx::Canvas* canvas) {
}

std::string TabContentsViewTouch::GetClassName() const {
  return kViewClassName;
}

void TabContentsViewTouch::UpdateDragCursor(WebDragOperation operation) {
  NOTIMPLEMENTED();
  // It's not even clear a drag cursor will make sense for touch.
  // TODO(anicolao): implement dragging
}

void TabContentsViewTouch::GotFocus() {
  if (tab_contents_->delegate())
    tab_contents_->delegate()->TabContentsFocused(tab_contents_);
}

void TabContentsViewTouch::TakeFocus(bool reverse) {
  if (tab_contents_->delegate() &&
      !tab_contents_->delegate()->TakeFocus(reverse)) {

    views::FocusManager* focus_manager =
        views::FocusManager::GetFocusManagerForNativeView(GetNativeView());

    // We may not have a focus manager if the tab has been switched before this
    // message arrived.
    if (focus_manager)
      focus_manager->AdvanceFocus(reverse);
  }
}

void TabContentsViewTouch::VisibilityChanged(views::View *, bool is_visible) {
  if (is_visible) {
    WasShown();
  } else {
    WasHidden();
  }
}


void TabContentsViewTouch::CreateNewWindow(
    int route_id,
    const ViewHostMsg_CreateWindow_Params& params) {
  delegate_view_helper_.CreateNewWindowFromTabContents(
      tab_contents_, route_id, params);
}

void TabContentsViewTouch::CreateNewWidget(
    int route_id, WebKit::WebPopupType popup_type) {
  delegate_view_helper_.CreateNewWidget(route_id, popup_type,
      tab_contents_->render_view_host()->process());
}

void TabContentsViewTouch::CreateNewFullscreenWidget(int route_id) {
  delegate_view_helper_.CreateNewFullscreenWidget(
      route_id, tab_contents_->render_view_host()->process());
}

void TabContentsViewTouch::ShowCreatedWindow(int route_id,
                                             WindowOpenDisposition disposition,
                                             const gfx::Rect& initial_pos,
                                             bool user_gesture) {
  delegate_view_helper_.ShowCreatedWindow(
      tab_contents_, route_id, disposition, initial_pos, user_gesture);
}

void TabContentsViewTouch::ShowCreatedWidget(
    int route_id, const gfx::Rect& initial_pos) {
  delegate_view_helper_.ShowCreatedWidget(
      tab_contents_, route_id, initial_pos);
}

void TabContentsViewTouch::ShowCreatedFullscreenWidget(int route_id) {
  delegate_view_helper_.ShowCreatedFullscreenWidget(tab_contents_, route_id);
}

void TabContentsViewTouch::ShowContextMenu(const ContextMenuParams& params) {
  // Allow delegates to handle the context menu operation first.
  if (tab_contents_->delegate() &&
      tab_contents_->delegate()->HandleContextMenu(params))
    return;

  context_menu_.reset(new RenderViewContextMenuViews(tab_contents_, params));
  context_menu_->Init();

  gfx::Point screen_point(params.x, params.y);
  RenderWidgetHostViewViews* rwhv = static_cast<RenderWidgetHostViewViews*>
      (tab_contents_->GetRenderWidgetHostView());
  if (rwhv) {
    views::View::ConvertPointToScreen(rwhv, &screen_point);
  }

  // Enable recursive tasks on the message loop so we can get updates while
  // the context menu is being displayed.
  bool old_state = MessageLoop::current()->NestableTasksAllowed();
  MessageLoop::current()->SetNestableTasksAllowed(true);
  context_menu_->RunMenuAt(screen_point.x(), screen_point.y());
  MessageLoop::current()->SetNestableTasksAllowed(old_state);
}

void TabContentsViewTouch::ShowPopupMenu(const gfx::Rect& bounds,
                                         int item_height,
                                         double item_font_size,
                                         int selected_item,
                                         const std::vector<WebMenuItem>& items,
                                         bool right_aligned) {
  // External popup menus are only used on Mac.
  NOTREACHED();
}

void TabContentsViewTouch::WasHidden() {
  tab_contents_->HideContents();
}

void TabContentsViewTouch::WasShown() {
  tab_contents_->ShowContents();
}

void TabContentsViewTouch::WasSized(const gfx::Size& size) {
  // We have to check that the RenderWidgetHostView is the proper size.
  // It can be wrong in cases where the renderer has died and the host
  // view needed to be recreated.
  bool needs_resize = size != size_;

  if (needs_resize) {
    size_ = size;
    if (tab_contents_->interstitial_page())
      tab_contents_->interstitial_page()->SetSize(size);
  }

  RenderWidgetHostView* rwhv = tab_contents_->GetRenderWidgetHostView();
  if (rwhv && rwhv->GetViewBounds().size() != size)
    rwhv->SetSize(size);

  if (needs_resize)
    SetFloatingPosition(size);
}

void TabContentsViewTouch::SetFloatingPosition(const gfx::Size& size) {
  // TODO(anicolao): rework this once we have WebUI views for dialogs
  SetBounds(x(), y(), size.width(), size.height());
}
