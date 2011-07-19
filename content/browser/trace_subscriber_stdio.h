// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_TRACE_SUBSCRIBER_STDIO_H_
#define CONTENT_BROWSER_TRACE_SUBSCRIBER_STDIO_H_
#pragma once

#include <string>

#include "base/file_util.h"
#include "content/browser/trace_controller.h"

// Stdio implementation of TraceSubscriber. Use this to write traces to a file.
class TraceSubscriberStdio : public TraceSubscriber {
 public:
  // Creates or overwrites the specified file. Check IsValid() for success.
  explicit TraceSubscriberStdio(const FilePath& path);

  // Returns TRUE if we're currently writing data to a file.
  bool IsValid();

  // Implementation of TraceSubscriber
  virtual void OnEndTracingComplete();
  virtual void OnTraceDataCollected(const std::string& json_events);

  virtual ~TraceSubscriberStdio();

 private:
  FILE* file_;
};

#endif  // CONTENT_BROWSER_TRACE_SUBSCRIBER_STDIO_H_
