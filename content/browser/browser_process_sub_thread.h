// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_BROWSER_PROCESS_SUB_THREAD_H_
#define CONTENT_BROWSER_BROWSER_PROCESS_SUB_THREAD_H_
#pragma once

#include "base/basictypes.h"
#include "content/browser/browser_thread_impl.h"
#include "content/common/content_export.h"

namespace content {
class NotificationService;
}

namespace content {

// ----------------------------------------------------------------------------
// BrowserProcessSubThread
//
// This simple thread object is used for the specialized threads that the
// BrowserProcess spins up.
//
// Applications must initialize the COM library before they can call
// COM library functions other than CoGetMalloc and memory allocation
// functions, so this class initializes COM for those users.
class CONTENT_EXPORT BrowserProcessSubThread : public BrowserThreadImpl {
 public:
  explicit BrowserProcessSubThread(BrowserThread::ID identifier);
  virtual ~BrowserProcessSubThread();

 protected:
  virtual void Init() OVERRIDE;
  virtual void CleanUp() OVERRIDE;

 private:
  // Each specialized thread has its own notification service.
  // Note: We don't use scoped_ptr because the destructor runs on the wrong
  // thread.
  NotificationService* notification_service_;

  DISALLOW_COPY_AND_ASSIGN(BrowserProcessSubThread);
};

}  // namespace content

#endif  // CONTENT_BROWSER_BROWSER_PROCESS_SUB_THREAD_H_
