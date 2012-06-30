// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ppapi/proxy/dispatcher.h"

#include <string.h>  // For memset.

#include <map>

#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/proxy/var_serialization_rules.h"

namespace ppapi {
namespace proxy {

Dispatcher::Dispatcher(PP_GetInterface_Func local_get_interface)
    : disallow_trusted_interfaces_(false),  // TODO(brettw) make this settable.
      local_get_interface_(local_get_interface) {
}

Dispatcher::~Dispatcher() {
}

void Dispatcher::AddFilter(IPC::Listener* listener) {
  filters_.push_back(listener);
}

InterfaceProxy* Dispatcher::GetInterfaceProxy(ApiID id) {
  InterfaceProxy* proxy = proxies_[id].get();
  if (!proxy) {
    // Handle the first time for a given API by creating the proxy for it.
    InterfaceProxy::Factory factory =
        InterfaceList::GetInstance()->GetFactoryForID(id);
    if (!factory) {
      NOTREACHED();
      return NULL;
    }
    proxy = factory(this);
    DCHECK(proxy);
    proxies_[id].reset(proxy);
  }
  return proxy;
}

base::MessageLoopProxy* Dispatcher::GetIPCMessageLoop() {
  return delegate()->GetIPCMessageLoop();
}

void Dispatcher::AddIOThreadMessageFilter(
    IPC::ChannelProxy::MessageFilter* filter) {
  // Our filter is refcounted. The channel will call the destruct method on the
  // filter when the channel is done with it, so the corresponding Release()
  // happens there.
  channel()->AddFilter(filter);
}

bool Dispatcher::OnMessageReceived(const IPC::Message& msg) {
  if (msg.routing_id() <= 0 || msg.routing_id() >= API_ID_COUNT) {
    OnInvalidMessageReceived();
    return true;
  }

  InterfaceProxy* proxy = GetInterfaceProxy(
      static_cast<ApiID>(msg.routing_id()));
  if (!proxy) {
    NOTREACHED();
    return true;
  }
  return proxy->OnMessageReceived(msg);
}

void Dispatcher::SetSerializationRules(
    VarSerializationRules* var_serialization_rules) {
  serialization_rules_ = var_serialization_rules;
}

void Dispatcher::OnInvalidMessageReceived() {
}

}  // namespace proxy
}  // namespace ppapi
