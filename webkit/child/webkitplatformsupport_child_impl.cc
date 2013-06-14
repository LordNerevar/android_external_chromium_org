// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webkit/child/webkitplatformsupport_child_impl.h"

#include "third_party/WebKit/Source/WebKit/chromium/public/WebInputEvent.h"
#include "webkit/child/fling_curve_configuration.h"
#include "webkit/child/worker_task_runner.h"

#if defined(OS_ANDROID)
#include "webkit/child/fling_animator_impl_android.h"
#endif

namespace webkit_glue {

WebKitPlatformSupportChildImpl::WebKitPlatformSupportChildImpl()
    : fling_curve_configuration_(new FlingCurveConfiguration) {}

WebKitPlatformSupportChildImpl::~WebKitPlatformSupportChildImpl() {}

void WebKitPlatformSupportChildImpl::SetFlingCurveParameters(
    const std::vector<float>& new_touchpad,
    const std::vector<float>& new_touchscreen) {
  fling_curve_configuration_->SetCurveParameters(new_touchpad, new_touchscreen);
}

WebKit::WebGestureCurve*
WebKitPlatformSupportChildImpl::createFlingAnimationCurve(
    int device_source,
    const WebKit::WebFloatPoint& velocity,
    const WebKit::WebSize& cumulative_scroll) {
#if defined(OS_ANDROID)
  return FlingAnimatorImpl::CreateAndroidGestureCurve(velocity,
                                                      cumulative_scroll);
#endif

  if (device_source == WebKit::WebGestureEvent::Touchscreen)
    return fling_curve_configuration_->CreateForTouchScreen(velocity,
                                                            cumulative_scroll);

  return fling_curve_configuration_->CreateForTouchPad(velocity,
                                                       cumulative_scroll);
}

void WebKitPlatformSupportChildImpl::didStartWorkerRunLoop(
    const WebKit::WebWorkerRunLoop& runLoop) {
  WorkerTaskRunner* worker_task_runner = WorkerTaskRunner::Instance();
  worker_task_runner->OnWorkerRunLoopStarted(runLoop);
}

void WebKitPlatformSupportChildImpl::didStopWorkerRunLoop(
    const WebKit::WebWorkerRunLoop& runLoop) {
  WorkerTaskRunner* worker_task_runner = WorkerTaskRunner::Instance();
  worker_task_runner->OnWorkerRunLoopStopped(runLoop);
}

}  // namespace webkit_glue
