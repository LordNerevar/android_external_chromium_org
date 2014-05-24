// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_LOCAL_DISCOVERY_PRIVETV2_SETUP_OPERATION_H_
#define CHROME_BROWSER_LOCAL_DISCOVERY_PRIVETV2_SETUP_OPERATION_H_

#include <string>

#include "base/callback.h"
#include "base/memory/scoped_ptr.h"

namespace local_discovery {

class PrivetV2Session;

class PrivetV2SetupOperation {
 public:
  enum Status { STATUS_SUCCESS, STATUS_SETUP_ERROR, STATUS_SESSION_ERROR };

  virtual ~PrivetV2SetupOperation() {}

  typedef base::Callback<void(Status status)> SetupStatusCallback;

  static scoped_ptr<PrivetV2SetupOperation> Create(
      PrivetV2Session* session,
      const SetupStatusCallback& callback,
      const std::string& ticket_id);

  virtual void AddWifiCredentials(const std::string& ssid,
                                  const std::string& passwd) = 0;
  virtual void Start() = 0;
};

}  // namespace local_discovery

#endif  // CHROME_BROWSER_LOCAL_DISCOVERY_PRIVETV2_SETUP_OPERATION_H_
