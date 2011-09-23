// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/renderer_host/render_widget_host_view_views.h"

#include "base/bind.h"
#include "base/logging.h"
#include "chrome/browser/renderer_host/accelerated_surface_container_touch.h"
#include "content/browser/gpu/gpu_process_host_ui_shim.h"
#include "content/browser/renderer_host/render_widget_host.h"
#include "content/common/gpu/gpu_messages.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/gtk/WebInputEventFactory.h"
#include "ui/gfx/gl/gl_bindings.h"
#include "views/widget/widget.h"

static const char kRenderWidgetHostViewKey[] = "__RENDER_WIDGET_HOST_VIEW__";

using WebKit::WebInputEventFactory;
using WebKit::WebTouchEvent;

namespace {

WebKit::WebTouchPoint::State TouchPointStateFromEvent(
    const views::TouchEvent* event) {
  switch (event->type()) {
    case ui::ET_TOUCH_PRESSED:
      return WebKit::WebTouchPoint::StatePressed;
    case ui::ET_TOUCH_RELEASED:
      return WebKit::WebTouchPoint::StateReleased;
    case ui::ET_TOUCH_MOVED:
      return WebKit::WebTouchPoint::StateMoved;
    case ui::ET_TOUCH_CANCELLED:
      return WebKit::WebTouchPoint::StateCancelled;
    default:
      return WebKit::WebTouchPoint::StateUndefined;
  }
}

WebKit::WebInputEvent::Type TouchEventTypeFromEvent(
    const views::TouchEvent* event) {
  switch (event->type()) {
    case ui::ET_TOUCH_PRESSED:
      return WebKit::WebInputEvent::TouchStart;
    case ui::ET_TOUCH_RELEASED:
      return WebKit::WebInputEvent::TouchEnd;
    case ui::ET_TOUCH_MOVED:
      return WebKit::WebInputEvent::TouchMove;
    case ui::ET_TOUCH_CANCELLED:
      return WebKit::WebInputEvent::TouchCancel;
    default:
      return WebKit::WebInputEvent::Undefined;
  }
}

inline void UpdateTouchParams(const views::TouchEvent& event,
                              WebKit::WebTouchPoint* tpoint) {
  tpoint->radiusX = event.radius_x();
  tpoint->radiusY = event.radius_y();
  tpoint->rotationAngle = event.rotation_angle();
  tpoint->force = event.force();
}

void UpdateTouchPointPosition(const views::TouchEvent* event,
                              const gfx::Point& origin,
                              WebKit::WebTouchPoint* tpoint) {
  tpoint->position.x = event->x();
  tpoint->position.y = event->y();

  tpoint->screenPosition.x = tpoint->position.x + origin.x();
  tpoint->screenPosition.y = tpoint->position.y + origin.y();
}

void AcknowledgeSwapBuffers(int32 route_id, int gpu_host_id) {
  // It's possible that gpu_host_id is no longer valid at this point (like if
  // gpu process was restarted after a crash).  SendToGpuHost handles this.
  GpuProcessHostUIShim::SendToGpuHost(gpu_host_id,
      new AcceleratedSurfaceMsg_BuffersSwappedACK(route_id));
}

}  // namespace

// static
RenderWidgetHostView* RenderWidgetHostView::CreateViewForWidget(
    RenderWidgetHost* widget) {
  return new RenderWidgetHostViewViews(widget);
}

ui::TouchStatus RenderWidgetHostViewViews::OnTouchEvent(
    const views::TouchEvent& event) {
  if (!host_)
    return ui::TOUCH_STATUS_UNKNOWN;

  // Update the list of touch points first.
  WebKit::WebTouchPoint* point = NULL;
  ui::TouchStatus status = ui::TOUCH_STATUS_UNKNOWN;

  switch (event.type()) {
    case ui::ET_TOUCH_PRESSED:
      // Add a new touch point.
      if (touch_event_.touchesLength <
          WebTouchEvent::touchesLengthCap) {
        point = &touch_event_.touches[touch_event_.touchesLength++];
        point->id = event.identity();

        if (touch_event_.touchesLength == 1) {
          // A new touch sequence has started.
          status = ui::TOUCH_STATUS_START;

          // We also want the focus.
          RequestFocus();

          // Confirm existing composition text on touch press events, to make
          // sure the input caret won't be moved with an ongoing composition
          // text.
          FinishImeCompositionSession();
        }
      }
      break;
    case ui::ET_TOUCH_RELEASED:
    case ui::ET_TOUCH_CANCELLED:
    case ui::ET_TOUCH_MOVED: {
      // The touch point should have been added to the event from an earlier
      // _PRESSED event. So find that.
      // At the moment, only a maximum of 4 touch-points are allowed. So a
      // simple loop should be sufficient.
      for (unsigned i = 0; i < touch_event_.touchesLength; ++i) {
        point = touch_event_.touches + i;
        if (point->id == event.identity()) {
          break;
        }
        point = NULL;
      }
      break;
    }
    default:
      DLOG(WARNING) << "Unknown touch event " << event.type();
      break;
  }

  if (!point)
    return ui::TOUCH_STATUS_UNKNOWN;

  if (status != ui::TOUCH_STATUS_START)
    status = ui::TOUCH_STATUS_CONTINUE;

  UpdateTouchParams(event, point);

  // Update the location and state of the point.
  point->state = TouchPointStateFromEvent(&event);
  if (point->state == WebKit::WebTouchPoint::StateMoved) {
    // It is possible for badly written touch drivers to emit Move events even
    // when the touch location hasn't changed. In such cases, consume the event
    // and pretend nothing happened.
    if (point->position.x == event.x() && point->position.y == event.y()) {
      return status;
    }
  }
  UpdateTouchPointPosition(&event, GetMirroredPosition(), point);

  // Mark the rest of the points as stationary.
  for (unsigned i = 0; i < touch_event_.touchesLength; ++i) {
    WebKit::WebTouchPoint* iter = touch_event_.touches + i;
    if (iter != point) {
      iter->state = WebKit::WebTouchPoint::StateStationary;
    }
  }

  // Update the type of the touch event.
  touch_event_.type = TouchEventTypeFromEvent(&event);
  touch_event_.timeStampSeconds = base::Time::Now().ToDoubleT();

  // The event and all the touches have been updated. Dispatch.
  host_->ForwardTouchEvent(touch_event_);

  // If the touch was released, then remove it from the list of touch points.
  if (event.type() == ui::ET_TOUCH_RELEASED) {
    --touch_event_.touchesLength;
    for (unsigned i = point - touch_event_.touches;
         i < touch_event_.touchesLength;
         ++i) {
      touch_event_.touches[i] = touch_event_.touches[i + 1];
    }
    if (touch_event_.touchesLength == 0)
      status = ui::TOUCH_STATUS_END;
  } else if (event.type() == ui::ET_TOUCH_CANCELLED) {
    status = ui::TOUCH_STATUS_CANCEL;
  }

  return status;
}

gfx::PluginWindowHandle RenderWidgetHostViewViews::GetCompositingSurface() {
  // On TOUCH_UI builds, the GPU process renders to an offscreen surface
  // (created by the GPU process), which is later displayed by the browser.
  // As the GPU process creates this surface, we can return any non-zero value.
  return 1;
}

void RenderWidgetHostViewViews::AcceleratedSurfaceNew(
    int32 width,
    int32 height,
    uint64* surface_id,
    TransportDIB::Handle* surface_handle) {
  scoped_ptr<AcceleratedSurfaceContainerTouch> surface(
      AcceleratedSurfaceContainerTouch::CreateAcceleratedSurfaceContainer(
          gfx::Size(width, height)));
  if (!surface->Initialize(surface_id)) {
    LOG(ERROR) << "Failed to create AcceleratedSurfaceContainer";
    return;
  }
  *surface_handle = surface->Handle();

  accelerated_surface_containers_[*surface_id] = surface.release();
}

void RenderWidgetHostViewViews::AcceleratedSurfaceRelease(uint64 surface_id) {
  accelerated_surface_containers_.erase(surface_id);
}

void RenderWidgetHostViewViews::AcceleratedSurfaceBuffersSwapped(
    uint64 surface_id,
    int32 route_id,
    int gpu_host_id) {
  SetExternalTexture(accelerated_surface_containers_[surface_id].get());
  glFlush();

  if (!GetWidget() || !GetWidget()->GetCompositor()) {
    // We have no compositor, so we have no way to display the surface
    AcknowledgeSwapBuffers(route_id, gpu_host_id);  // Must still send the ACK
  } else {
    // Add sending an ACK to the list of things to do OnCompositingEnded
    on_compositing_ended_callbacks_.push_back(
        base::Bind(AcknowledgeSwapBuffers, route_id, gpu_host_id));
    ui::Compositor *compositor = GetWidget()->GetCompositor();
    if (!compositor->HasObserver(this))
      compositor->AddObserver(this);
  }
}

void RenderWidgetHostViewViews::OnCompositingEnded(ui::Compositor* compositor) {
  for (std::vector< base::Callback<void(void)> >::const_iterator
      it = on_compositing_ended_callbacks_.begin();
      it != on_compositing_ended_callbacks_.end(); ++it) {
    it->Run();
  }
  on_compositing_ended_callbacks_.clear();
  compositor->RemoveObserver(this);
}

