// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_POLICY_CORE_BROWSER_BROWSER_POLICY_CONNECTOR_IOS_H_
#define COMPONENTS_POLICY_CORE_BROWSER_BROWSER_POLICY_CONNECTOR_IOS_H_

#include <string>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "components/policy/core/browser/browser_policy_connector.h"
#include "components/policy/policy_export.h"

namespace policy {

// Extends BrowserPolicyConnector with the setup for iOS builds.
class POLICY_EXPORT BrowserPolicyConnectorIOS : public BrowserPolicyConnector {
 public:
  BrowserPolicyConnectorIOS(
      scoped_ptr<ConfigurationPolicyHandlerList> handler_list,
      const std::string& user_agent);

  virtual ~BrowserPolicyConnectorIOS();

  virtual void Init(
      PrefService* local_state,
      scoped_refptr<net::URLRequestContextGetter> request_context) OVERRIDE;

 private:
  std::string user_agent_;

  DISALLOW_COPY_AND_ASSIGN(BrowserPolicyConnectorIOS);
};

}  // namespace policy

#endif  // COMPONENTS_POLICY_CORE_BROWSER_BROWSER_POLICY_CONNECTOR_IOS_H_
