// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_NET_PREF_PROXY_CONFIG_TRACKER_H_
#define CHROME_BROWSER_NET_PREF_PROXY_CONFIG_TRACKER_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"

namespace net {
class ProxyConfigService;
}

// Interface for a class that tracks proxy preferences. The purpose of the
// concrete class is to track changes in the Preferences, to translates the
// preferences to net::ProxyConfig and to push the result over to a
// net::ProxyConfigService onto the IO thread.
class PrefProxyConfigTracker {
 public:
  PrefProxyConfigTracker();
  virtual ~PrefProxyConfigTracker();

  // Creates a net::ProxyConfigService and keeps a pointer to it. After this
  // call, this tracker forwards any changes of proxy preferences to the created
  // ProxyConfigService. The returned ProxyConfigService must not be deleted
  // before DetachFromPrefService was called. Takes ownership of the passed
  // |base_service|, which can be NULL. This |base_service| provides the proxy
  // settings of the OS (except of ChromeOS). This must be called on the
  // UI thread.
  virtual scoped_ptr<net::ProxyConfigService> CreateTrackingProxyConfigService(
      scoped_ptr<net::ProxyConfigService> base_service) = 0;

  // Releases the PrefService passed upon construction and the |base_service|
  // passed to CreateTrackingProxyConfigService. This must be called on the UI
  // thread.
  virtual void DetachFromPrefService() = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(PrefProxyConfigTracker);
};

#endif  // CHROME_BROWSER_NET_PREF_PROXY_CONFIG_TRACKER_H_
