// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/wm/toplevel_window_event_filter.h"

#include "ash/shell.h"
#include "ash/wm/default_window_resizer.h"
#include "ash/wm/property_util.h"
#include "ash/wm/resize_shadow_controller.h"
#include "ash/wm/window_resizer.h"
#include "ash/wm/window_util.h"
#include "ash/wm/workspace/snap_sizer.h"
#include "base/message_loop.h"
#include "base/run_loop.h"
#include "ui/aura/client/aura_constants.h"
#include "ui/aura/env.h"
#include "ui/aura/root_window.h"
#include "ui/aura/window.h"
#include "ui/aura/window_delegate.h"
#include "ui/aura/window_observer.h"
#include "ui/base/cursor/cursor.h"
#include "ui/base/events/event.h"
#include "ui/base/gestures/gesture_recognizer.h"
#include "ui/base/hit_test.h"
#include "ui/base/ui_base_types.h"
#include "ui/compositor/layer.h"
#include "ui/compositor/scoped_layer_animation_settings.h"
#include "ui/gfx/screen.h"

namespace {
const double kMinHorizVelocityForWindowSwipe = 1100;
const double kMinVertVelocityForWindowMinimize = 1000;
}

namespace ash {

namespace {

gfx::Point ConvertPointToParent(aura::Window* window,
                                const gfx::Point& point) {
  gfx::Point result(point);
  aura::Window::ConvertPointToTarget(window, window->parent(), &result);
  return result;
}

}  // namespace

// ScopedWindowResizer ---------------------------------------------------------

// Wraps a WindowResizer and installs an observer on its target window.  When
// the window is destroyed ResizerWindowDestroyed() is invoked back on the
// ToplevelWindowEventFilter to clean up.
class ToplevelWindowEventFilter::ScopedWindowResizer
    : public aura::WindowObserver {
 public:
  ScopedWindowResizer(ToplevelWindowEventFilter* filter,
                      WindowResizer* resizer);
  virtual ~ScopedWindowResizer();

  WindowResizer* resizer() { return resizer_.get(); }

  // WindowObserver overrides:
  virtual void OnWindowDestroying(aura::Window* window) OVERRIDE;

 private:
  ToplevelWindowEventFilter* filter_;
  scoped_ptr<WindowResizer> resizer_;

  DISALLOW_COPY_AND_ASSIGN(ScopedWindowResizer);
};

ToplevelWindowEventFilter::ScopedWindowResizer::ScopedWindowResizer(
    ToplevelWindowEventFilter* filter,
    WindowResizer* resizer)
    : filter_(filter),
      resizer_(resizer) {
  if (resizer_.get())
    resizer_->GetTarget()->AddObserver(this);
}

ToplevelWindowEventFilter::ScopedWindowResizer::~ScopedWindowResizer() {
  if (resizer_.get())
    resizer_->GetTarget()->RemoveObserver(this);
}

void ToplevelWindowEventFilter::ScopedWindowResizer::OnWindowDestroying(
    aura::Window* window) {
  DCHECK(resizer_.get());
  DCHECK_EQ(resizer_->GetTarget(), window);
  filter_->ResizerWindowDestroyed();
}


// ToplevelWindowEventFilter ---------------------------------------------------

ToplevelWindowEventFilter::ToplevelWindowEventFilter(aura::Window* owner)
    : in_move_loop_(false),
      move_cancelled_(false),
      in_gesture_resize_(false),
      grid_size_(0) {
  aura::client::SetWindowMoveClient(owner, this);
  Shell::GetInstance()->display_controller()->AddObserver(this);
}

ToplevelWindowEventFilter::~ToplevelWindowEventFilter() {
  Shell::GetInstance()->display_controller()->RemoveObserver(this);
}

bool ToplevelWindowEventFilter::PreHandleKeyEvent(aura::Window* target,
                                                  ui::KeyEvent* event) {
  if (window_resizer_.get() && event->type() == ui::ET_KEY_PRESSED &&
      event->key_code() == ui::VKEY_ESCAPE) {
    CompleteDrag(DRAG_REVERT, event->flags());
  }
  return false;
}

bool ToplevelWindowEventFilter::PreHandleMouseEvent(aura::Window* target,
                                                    ui::MouseEvent* event) {
  if ((event->flags() &
      (ui::EF_MIDDLE_MOUSE_BUTTON | ui::EF_RIGHT_MOUSE_BUTTON)) != 0)
    return false;

  switch (event->type()) {
    case ui::ET_MOUSE_PRESSED: {
      // We also update the current window component here because for the
      // mouse-drag-release-press case, where the mouse is released and
      // pressed without mouse move event.
      int component =
          target->delegate()->GetNonClientComponent(event->location());
      if ((event->flags() &
           (ui::EF_IS_DOUBLE_CLICK | ui::EF_IS_TRIPLE_CLICK)) == 0 &&
          WindowResizer::GetBoundsChangeForWindowComponent(component)) {
        gfx::Point location_in_parent(
            ConvertPointToParent(target, event->location()));
        CreateScopedWindowResizer(target, location_in_parent, component);
      } else {
        window_resizer_.reset();
      }
      return WindowResizer::GetBoundsChangeForWindowComponent(component) != 0;
    }
    case ui::ET_MOUSE_DRAGGED:
      return HandleDrag(target, event);
    case ui::ET_MOUSE_CAPTURE_CHANGED:
    case ui::ET_MOUSE_RELEASED:
      CompleteDrag(event->type() == ui::ET_MOUSE_RELEASED ?
                       DRAG_COMPLETE : DRAG_REVERT,
                   event->flags());
      if (in_move_loop_) {
        quit_closure_.Run();
        in_move_loop_ = false;
      }
      // Completing the drag may result in hiding the window. If this happens
      // return true so no other filters/observers see the event. Otherwise they
      // see the event on a hidden window.
      if (event->type() == ui::ET_MOUSE_CAPTURE_CHANGED &&
          !target->IsVisible()) {
        return true;
      }
      break;
    case ui::ET_MOUSE_MOVED:
      return HandleMouseMoved(target, event);
    case ui::ET_MOUSE_EXITED:
      return HandleMouseExited(target, event);
    default:
      break;
  }
  return false;
}

ui::TouchStatus ToplevelWindowEventFilter::PreHandleTouchEvent(
    aura::Window* target,
    ui::TouchEvent* event) {
  return ui::TOUCH_STATUS_UNKNOWN;
}

ui::EventResult ToplevelWindowEventFilter::PreHandleGestureEvent(
    aura::Window* target,
    ui::GestureEvent* event) {
  switch (event->type()) {
    case ui::ET_GESTURE_SCROLL_BEGIN: {
      int component =
          target->delegate()->GetNonClientComponent(event->location());
      if (WindowResizer::GetBoundsChangeForWindowComponent(component) == 0) {
        window_resizer_.reset();
        return ui::ER_UNHANDLED;
      }
      in_gesture_resize_ = true;
      gfx::Point location_in_parent(
          ConvertPointToParent(target, event->location()));
      CreateScopedWindowResizer(target, location_in_parent, component);
      break;
    }
    case ui::ET_GESTURE_SCROLL_UPDATE: {
      if (!in_gesture_resize_)
        return ui::ER_UNHANDLED;
      HandleDrag(target, event);
      break;
    }
    case ui::ET_GESTURE_SCROLL_END:
    case ui::ET_SCROLL_FLING_START: {
      ui::EventResult status = ui::ER_UNHANDLED;
      if (in_gesture_resize_) {
        // If the window was being resized, then just complete the resize.
        CompleteDrag(DRAG_COMPLETE, event->flags());
        if (in_move_loop_) {
          quit_closure_.Run();
          in_move_loop_ = false;
        }
        in_gesture_resize_ = false;
        status = ui::ER_CONSUMED;
      }

      if (event->type() == ui::ET_GESTURE_SCROLL_END)
        return status;

      int component =
          target->delegate()->GetNonClientComponent(event->location());
      if (WindowResizer::GetBoundsChangeForWindowComponent(component) == 0)
        return ui::ER_UNHANDLED;
      if (!wm::IsWindowNormal(target))
        return ui::ER_UNHANDLED;

      if (fabs(event->details().velocity_y()) >
          kMinVertVelocityForWindowMinimize) {
        // Minimize/maximize.
        target->SetProperty(aura::client::kShowStateKey,
            event->details().velocity_y() > 0 ? ui::SHOW_STATE_MINIMIZED :
                                      ui::SHOW_STATE_MAXIMIZED);
      } else if (fabs(event->details().velocity_x()) >
                 kMinHorizVelocityForWindowSwipe) {
        // Snap left/right.
        internal::SnapSizer sizer(target,
            gfx::Point(),
            event->details().velocity_x() < 0 ? internal::SnapSizer::LEFT_EDGE :
            internal::SnapSizer::RIGHT_EDGE,
            Shell::GetInstance()->GetGridSize());

        ui::ScopedLayerAnimationSettings scoped_setter(
            target->layer()->GetAnimator());
        scoped_setter.SetPreemptionStrategy(
            ui::LayerAnimator::REPLACE_QUEUED_ANIMATIONS);
        target->SetBounds(sizer.target_bounds());
      }
      break;
    }
    default:
      return ui::ER_UNHANDLED;
  }

  return ui::ER_CONSUMED;
}

aura::client::WindowMoveResult ToplevelWindowEventFilter::RunMoveLoop(
    aura::Window* source,
    const gfx::Point& drag_offset) {
  DCHECK(!in_move_loop_);  // Can only handle one nested loop at a time.
  in_move_loop_ = true;
  move_cancelled_ = false;
  aura::RootWindow* root_window = source->GetRootWindow();
  DCHECK(root_window);
  gfx::Point drag_location;
  if (aura::Env::GetInstance()->is_touch_down()) {
    in_gesture_resize_ = true;
    bool has_point = root_window->gesture_recognizer()->
        GetLastTouchPointForTarget(source, &drag_location);
    DCHECK(has_point);
  } else {
    drag_location = root_window->GetLastMouseLocationInRoot();
    aura::Window::ConvertPointToTarget(
        root_window, source->parent(), &drag_location);
  }
  CreateScopedWindowResizer(source, drag_location, HTCAPTION);
  source->GetRootWindow()->SetCursor(ui::kCursorPointer);
#if !defined(OS_MACOSX)
  MessageLoopForUI* loop = MessageLoopForUI::current();
  MessageLoop::ScopedNestableTaskAllower allow_nested(loop);
  base::RunLoop run_loop(aura::Env::GetInstance()->GetDispatcher());
  quit_closure_ = run_loop.QuitClosure();
  run_loop.Run();
#endif  // !defined(OS_MACOSX)
  in_gesture_resize_ = in_move_loop_ = false;
  return move_cancelled_ ? aura::client::MOVE_CANCELED :
      aura::client::MOVE_SUCCESSFUL;
}

void ToplevelWindowEventFilter::EndMoveLoop() {
  if (!in_move_loop_)
    return;

  in_move_loop_ = false;
  if (window_resizer_.get()) {
    window_resizer_->resizer()->RevertDrag();
    window_resizer_.reset();
  }
  quit_closure_.Run();
}

void ToplevelWindowEventFilter::OnDisplayConfigurationChanging() {
  if (in_move_loop_) {
    move_cancelled_ = true;
    EndMoveLoop();
  } else if (window_resizer_.get()) {
    window_resizer_->resizer()->RevertDrag();
    window_resizer_.reset();
  }
}

// static
WindowResizer* ToplevelWindowEventFilter::CreateWindowResizer(
    aura::Window* window,
    const gfx::Point& point_in_parent,
    int window_component) {
  if (!wm::IsWindowNormal(window))
    return NULL;  // Don't allow resizing/dragging maximized/fullscreen windows.
  return DefaultWindowResizer::Create(
      window, point_in_parent, window_component);
}


void ToplevelWindowEventFilter::CreateScopedWindowResizer(
    aura::Window* window,
    const gfx::Point& point_in_parent,
    int window_component) {
  window_resizer_.reset();
  WindowResizer* resizer =
      CreateWindowResizer(window, point_in_parent, window_component);
  if (resizer)
    window_resizer_.reset(new ScopedWindowResizer(this, resizer));
}

void ToplevelWindowEventFilter::CompleteDrag(DragCompletionStatus status,
                                             int event_flags) {
  scoped_ptr<ScopedWindowResizer> resizer(window_resizer_.release());
  if (resizer.get()) {
    if (status == DRAG_COMPLETE)
      resizer->resizer()->CompleteDrag(event_flags);
    else
      resizer->resizer()->RevertDrag();
  }
}

bool ToplevelWindowEventFilter::HandleDrag(aura::Window* target,
                                           ui::LocatedEvent* event) {
  // This function only be triggered to move window
  // by mouse drag or touch move event.
  DCHECK(event->type() == ui::ET_MOUSE_DRAGGED ||
         event->type() == ui::ET_TOUCH_MOVED ||
         event->type() == ui::ET_GESTURE_SCROLL_UPDATE);

  if (!window_resizer_.get())
    return false;
  window_resizer_->resizer()->Drag(
      ConvertPointToParent(target, event->location()), event->flags());
  return true;
}

bool ToplevelWindowEventFilter::HandleMouseMoved(aura::Window* target,
                                                 ui::LocatedEvent* event) {
  // TODO(jamescook): Move the resize cursor update code into here from
  // CompoundEventFilter?
  internal::ResizeShadowController* controller =
      Shell::GetInstance()->resize_shadow_controller();
  if (controller) {
    if (event->flags() & ui::EF_IS_NON_CLIENT) {
      int component =
          target->delegate()->GetNonClientComponent(event->location());
        controller->ShowShadow(target, component);
    } else {
      controller->HideShadow(target);
    }
  }
  return false;
}

bool ToplevelWindowEventFilter::HandleMouseExited(aura::Window* target,
                                                  ui::LocatedEvent* event) {
  internal::ResizeShadowController* controller =
      Shell::GetInstance()->resize_shadow_controller();
  if (controller)
    controller->HideShadow(target);
  return false;
}

void ToplevelWindowEventFilter::ResizerWindowDestroyed() {
  // We explicitly don't invoke RevertDrag() since that may do things to window.
  // Instead we destroy the resizer.
  window_resizer_.reset();

  // End the move loop. This does nothing if we're not in a move loop.
  EndMoveLoop();
}

}  // namespace ash
