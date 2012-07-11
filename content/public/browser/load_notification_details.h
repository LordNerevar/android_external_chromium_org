// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_LOAD_NOTIFICATION_DETAILS_H_
#define CONTENT_PUBLIC_BROWSER_LOAD_NOTIFICATION_DETAILS_H_

#include "base/time.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/common/page_transition_types.h"
#include "googleurl/src/gurl.h"

namespace content {

// The LoadNotificationDetails object contains additional details about a
// page load that has been completed.  It was created to let the MetricsService
// log page load metrics.
struct LoadNotificationDetails {
  LoadNotificationDetails(const GURL& url,
                          PageTransition origin,
                          base::TimeDelta load_time,
                          NavigationController* controller,
                          int session_index)
      : url(url),
        load_time(load_time),
        session_index(session_index),
        origin(origin),
        controller(controller) {}

  // The URL loaded.
  GURL url;

  // The length of time the page load took.
  base::TimeDelta load_time;

  // The index of the load within the tab session.
  int session_index;

  // The type of action that caused the load.
  PageTransition origin;

  // The NavigationController for the load.
  NavigationController* controller;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_LOAD_NOTIFICATION_DETAILS_H_
