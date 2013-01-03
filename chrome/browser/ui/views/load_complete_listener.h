// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_LOAD_COMPLETE_LISTENER_H_
#define CHROME_BROWSER_UI_VIEWS_LOAD_COMPLETE_LISTENER_H_

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_observer.h"

namespace content {
class NotificationSource;
class NotificationRegistrar;
class NotificationDetails;
}

// A class which takes  a delegate which can be notified after the first page
// load has been completed. This is particularly useful for triggering
// IO-intensive tasks which should not be run until start-up is complete.
class LoadCompleteListener
    : public content::NotificationObserver {
 public:
  class Delegate {
   public:
    // Invoked when initial page load has completed.
    virtual void OnLoadCompleted() = 0;

   protected:
    virtual ~Delegate() {}
  };

  explicit LoadCompleteListener(Delegate* delegate);

  virtual ~LoadCompleteListener();

  // NotificationObserver implementation.
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

 private:
  content::NotificationRegistrar* registrar_;

  // Delegate to be notified after the first page load has completed.
  Delegate* delegate_;

  DISALLOW_COPY_AND_ASSIGN(LoadCompleteListener);
};

#endif  // CHROME_BROWSER_UI_VIEWS_LOAD_COMPLETE_LISTENER_H_
