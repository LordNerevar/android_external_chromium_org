// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_ROOT_WINDOW_H_
#define UI_AURA_ROOT_WINDOW_H_
#pragma once

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop.h"
#include "ui/aura/aura_export.h"
#include "ui/aura/cursor.h"
#include "ui/aura/focus_manager.h"
#include "ui/aura/gestures/gesture_recognizer.h"
#include "ui/aura/window.h"
#include "ui/base/events.h"
#include "ui/gfx/compositor/compositor.h"
#include "ui/gfx/compositor/compositor_observer.h"
#include "ui/gfx/compositor/layer_animation_observer.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/point.h"

namespace gfx {
class Size;
}

namespace ui {
class LayerAnimationSequence;
class Transform;
}

namespace aura {

class RootWindowHost;
class RootWindowObserver;
class KeyEvent;
class MouseEvent;
class ScreenAura;
class StackingClient;
class ScrollEvent;
class TouchEvent;
class GestureEvent;

// RootWindow is responsible for hosting a set of windows.
class AURA_EXPORT RootWindow : public ui::CompositorDelegate,
                               public ui::CompositorObserver,
                               public Window,
                               public internal::FocusManager,
                               public ui::LayerAnimationObserver {
 public:
  RootWindow();
  virtual ~RootWindow();

  static void set_use_fullscreen_host_window(bool use_fullscreen) {
    use_fullscreen_host_window_ = use_fullscreen;
  }
  static bool use_fullscreen_host_window() {
    return use_fullscreen_host_window_;
  }

  static void set_hide_host_cursor(bool hide) {
    hide_host_cursor_ = hide;
  }
  static bool hide_host_cursor() {
    return hide_host_cursor_;
  }

  ui::Compositor* compositor() { return compositor_.get(); }
  gfx::Point last_mouse_location() const { return last_mouse_location_; }
  gfx::NativeCursor last_cursor() const { return last_cursor_; }
  bool cursor_shown() const { return cursor_shown_; }
  Window* mouse_pressed_handler() { return mouse_pressed_handler_; }
  Window* capture_window() { return capture_window_; }
  const ScreenAura* screen() { return screen_; }

  // Shows the root window host.
  void ShowRootWindow();

  // Sets the size of the root window.
  void SetHostSize(const gfx::Size& size);
  gfx::Size GetHostSize() const;

  // Sets the screen's work area insets, this notifies observers too.
  void SetScreenWorkAreaInsets(const gfx::Insets& insets);

  // Sets the currently-displayed cursor. If the cursor was previously hidden
  // via ShowCursor(false), it will remain hidden until ShowCursor(true) is
  // called, at which point the cursor that was last set via SetCursor() will be
  // used.
  void SetCursor(gfx::NativeCursor cursor);

  // Shows or hides the cursor.
  void ShowCursor(bool show);

  // Moves the cursor to the specified location relative to the root window.
  void MoveCursorTo(const gfx::Point& location);

  // Clips the cursor movement to |capture_window_|. Should be invoked only
  // after SetCapture(). ReleaseCapture() implicitly removes any confines set
  // using this function. Returns true if successful.
  bool ConfineCursorToWindow();

  // Draws the necessary set of windows.
  void Draw();

  // Draw the whole screen.
  void ScheduleFullDraw();

  // Handles a mouse event. Returns true if handled.
  bool DispatchMouseEvent(MouseEvent* event);

  // Handles a key event. Returns true if handled.
  bool DispatchKeyEvent(KeyEvent* event);

  // Handles a scroll event. Returns true if handled.
  bool DispatchScrollEvent(ScrollEvent* event);

  // Handles a touch event. Returns true if handled.
  bool DispatchTouchEvent(TouchEvent* event);

  // Handles a gesture event. Returns true if handled. Unlike the other
  // event-dispatching function (e.g. for touch/mouse/keyboard events), gesture
  // events are dispatched from GestureRecognizer instead of RootWindowHost.
  bool DispatchGestureEvent(GestureEvent* event);

  // Called when the host changes size.
  void OnHostResized(const gfx::Size& size);

  // Called when the native screen's resolution changes.
  void OnNativeScreenResized(const gfx::Size& size);

  // Invoked when |window| is being destroyed.
  void OnWindowDestroying(Window* window);

  // Invokved when |window|'s bounds is changed. |contained_mouse| indicates if
  // the bounds before change contained the |last_moust_location()|.
  void OnWindowBoundsChanged(Window* window, bool contained_mouse);

  // Invokved when |window|'s visibility is changed.
  void OnWindowVisibilityChanged(Window* window, bool is_visible);

  // Invokved when |window|'s tranfrom has changed. |contained_mouse|
  // indicates if the bounds before change contained the
  // |last_moust_location()|.
  void OnWindowTransformed(Window* window, bool contained_mouse);

  // Add/remove observer.
  void AddRootWindowObserver(RootWindowObserver* observer);
  void RemoveRootWindowObserver(RootWindowObserver* observer);

  // Posts |native_event| to the platform's event queue.
  void PostNativeEvent(const base::NativeEvent& native_event);

  // Converts |point| from the root window's coordinate system to native
  // screen's.
  void ConvertPointToNativeScreen(gfx::Point* point) const;

  // Capture -------------------------------------------------------------------

  // Sets capture to the specified window.
  void SetCapture(Window* window);

  // If |window| has mouse capture, the current capture window is set to NULL.
  void ReleaseCapture(Window* window);

  // Gesture Recognition -------------------------------------------------------

  // When a touch event is dispatched to a Window, it can notify the RootWindow
  // to queue the touch event for asynchronous gesture recognition. These are
  // the entry points for the asynchronous processing of the queued touch
  // events.
  // Process the next touch event for gesture recognition. |processed| indicates
  // whether the queued event was processed by the window or not.
  void AdvanceQueuedTouchEvent(Window* window, bool processed);

  GestureRecognizer* gesture_recognizer() const {
    return gesture_recognizer_.get();
  }

  // Provided only for testing:
  void SetGestureRecognizerForTesting(GestureRecognizer* gr);

#if !defined(NDEBUG)
  // Toggles the host's full screen state.
  void ToggleFullScreen();
#endif

  // Overridden from Window:
  virtual RootWindow* GetRootWindow() OVERRIDE;
  virtual void SetTransform(const ui::Transform& transform) OVERRIDE;

  // Overridden from ui::CompositorDelegate:
  virtual void ScheduleDraw() OVERRIDE;

  // Overridden from ui::CompositorObserver:
  virtual void OnCompositingEnded(ui::Compositor*) OVERRIDE;

 private:
  friend class Window;

  // Called whenever the mouse moves, tracks the current |mouse_moved_handler_|,
  // sending exited and entered events as its value changes.
  void HandleMouseMoved(const MouseEvent& event, Window* target);

  // Called whenever the |capture_window_| changes.
  // Sends capture changed events to event filters for old capture window.
  void HandleMouseCaptureChanged(Window* old_capture_window);

  bool ProcessMouseEvent(Window* target, MouseEvent* event);
  bool ProcessKeyEvent(Window* target, KeyEvent* event);
  ui::TouchStatus ProcessTouchEvent(Window* target, TouchEvent* event);
  ui::GestureStatus ProcessGestureEvent(Window* target, GestureEvent* event);
  bool ProcessGestures(GestureRecognizer::Gestures* gestures);

  // Called when a Window is attached or detached from the RootWindow.
  void OnWindowAddedToRootWindow(Window* window);
  void OnWindowRemovedFromRootWindow(Window* window);

  // Called when a window becomes invisible, either by being removed
  // from root window hierachy, via SetVisible(false) or being destroyed.
  // |destroyed| is set to true when the window is being destroyed.
  void OnWindowHidden(Window* invisible, bool destroyed);

  // Overridden from Window:
  virtual bool CanFocus() const OVERRIDE;
  virtual bool CanReceiveEvents() const OVERRIDE;
  virtual internal::FocusManager* GetFocusManager() OVERRIDE;

  // Overridden from ui::LayerAnimationObserver:
  virtual void OnLayerAnimationEnded(
      ui::LayerAnimationSequence* animation) OVERRIDE;
  virtual void OnLayerAnimationScheduled(
      ui::LayerAnimationSequence* animation) OVERRIDE;
  virtual void OnLayerAnimationAborted(
      ui::LayerAnimationSequence* animation) OVERRIDE;

  // Overridden from FocusManager:
  virtual void SetFocusedWindow(Window* window) OVERRIDE;
  virtual Window* GetFocusedWindow() OVERRIDE;
  virtual bool IsFocusedWindow(const Window* window) const OVERRIDE;

  // Initializes the root window.
  void Init();

  // Parses the switch describing the initial size for the host window and
  // returns bounds for the window.
  gfx::Rect GetInitialHostWindowBounds() const;

  // Posts a task to send synthesized mouse move event if there
  // is no a pending task.
  void PostMouseMoveEventAfterWindowChange();

  // Creates and dispatches synthesized mouse move event using the
  // current mouse location.
  void SynthesizeMouseMoveEvent();

  scoped_ptr<ui::Compositor> compositor_;

  scoped_ptr<RootWindowHost> host_;

  static RootWindow* instance_;

  // If set before the RootWindow is created, the host window will cover the
  // entire screen.  Note that this can still be overridden via the
  // switches::kAuraHostWindowSize flag.
  static bool use_fullscreen_host_window_;

  // If set before the RootWindow is created, the cursor will be drawn within
  // the Aura root window but hidden outside of it, and it'll remain hidden
  // after the Aura window is closed.
  static bool hide_host_cursor_;

  // Used to schedule painting.
  base::WeakPtrFactory<RootWindow> schedule_paint_factory_;

  // Use to post mouse move event.
  base::WeakPtrFactory<RootWindow> event_factory_;

  // Last location seen in a mouse event.
  gfx::Point last_mouse_location_;

  // ui::EventFlags containing the current state of the mouse buttons.
  int mouse_button_flags_;

  // Last cursor set.  Used for testing.
  gfx::NativeCursor last_cursor_;

  // Is the cursor currently shown?  Used for testing.
  bool cursor_shown_;

  ObserverList<RootWindowObserver> observers_;

  ScreenAura* screen_;

  // The capture window. When not-null, this window receives all the mouse and
  // touch events.
  Window* capture_window_;

  Window* mouse_pressed_handler_;
  Window* mouse_moved_handler_;
  Window* focused_window_;
  Window* touch_event_handler_;
  Window* gesture_handler_;

  // The gesture_recognizer_ for this.
  scoped_ptr<GestureRecognizer> gesture_recognizer_;

  bool synthesize_mouse_move_;
  bool waiting_on_compositing_end_;
  bool draw_on_compositing_end_;

  DISALLOW_COPY_AND_ASSIGN(RootWindow);
};

}  // namespace aura

#endif  // UI_AURA_ROOT_WINDOW_H_
