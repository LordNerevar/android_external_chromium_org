// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ppapi/proxy/plugin_dispatcher.h"

#include <map>

#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/message_loop.h"
#include "ipc/ipc_message.h"
#include "ipc/ipc_sync_channel.h"
#include "base/debug/trace_event.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/c/ppp_instance.h"
#include "ppapi/proxy/interface_list.h"
#include "ppapi/proxy/interface_proxy.h"
#include "ppapi/proxy/plugin_message_filter.h"
#include "ppapi/proxy/plugin_resource_tracker.h"
#include "ppapi/proxy/plugin_var_serialization_rules.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/proxy/ppb_cursor_control_proxy.h"
#include "ppapi/proxy/ppb_instance_proxy.h"
#include "ppapi/proxy/ppp_class_proxy.h"
#include "ppapi/proxy/resource_creation_proxy.h"
#include "ppapi/shared_impl/proxy_lock.h"
#include "ppapi/shared_impl/resource.h"

#if defined(OS_POSIX)
#include "base/eintr_wrapper.h"
#include "ipc/ipc_channel_posix.h"
#endif

namespace ppapi {
namespace proxy {

namespace {

typedef std::map<PP_Instance, PluginDispatcher*> InstanceToDispatcherMap;
InstanceToDispatcherMap* g_instance_to_dispatcher = NULL;

typedef std::set<PluginDispatcher*> DispatcherSet;
DispatcherSet* g_live_dispatchers = NULL;

}  // namespace

InstanceData::InstanceData()
    : flash_fullscreen(PP_FALSE),
      mouse_lock_callback(PP_BlockUntilComplete()) {
}

InstanceData::~InstanceData() {
  // Run any pending mouse lock callback to prevent leaks.
  if (mouse_lock_callback.func) {
    CallWhileUnlocked(PP_RunAndClearCompletionCallback,
                      &mouse_lock_callback,
                      static_cast<int32_t>(PP_ERROR_ABORTED));
  }
}

PluginDispatcher::PluginDispatcher(base::ProcessHandle remote_process_handle,
                                   GetInterfaceFunc get_interface)
    : Dispatcher(remote_process_handle, get_interface),
      plugin_delegate_(NULL),
      received_preferences_(false),
      plugin_dispatcher_id_(0) {
  SetSerializationRules(new PluginVarSerializationRules);

  if (!g_live_dispatchers)
    g_live_dispatchers = new DispatcherSet;
  g_live_dispatchers->insert(this);
}

PluginDispatcher::~PluginDispatcher() {
  if (plugin_delegate_)
    plugin_delegate_->Unregister(plugin_dispatcher_id_);

  g_live_dispatchers->erase(this);
  if (g_live_dispatchers->empty()) {
    delete g_live_dispatchers;
    g_live_dispatchers = NULL;
  }
}

// static
PluginDispatcher* PluginDispatcher::GetForInstance(PP_Instance instance) {
  if (!g_instance_to_dispatcher)
    return NULL;
  InstanceToDispatcherMap::iterator found = g_instance_to_dispatcher->find(
      instance);
  if (found == g_instance_to_dispatcher->end())
    return NULL;
  return found->second;
}

// static
PluginDispatcher* PluginDispatcher::GetForResource(const Resource* resource) {
  return GetForInstance(resource->pp_instance());
}

// static
const void* PluginDispatcher::GetBrowserInterface(const char* interface_name) {
  return InterfaceList::GetInstance()->GetInterfaceForPPB(interface_name);
}

// static
void PluginDispatcher::LogWithSource(PP_Instance instance,
                                     PP_LogLevel_Dev level,
                                     const std::string& source,
                                     const std::string& value) {
  if (!g_live_dispatchers || !g_instance_to_dispatcher)
    return;

  if (instance) {
    InstanceToDispatcherMap::iterator found =
        g_instance_to_dispatcher->find(instance);
    if (found != g_instance_to_dispatcher->end()) {
      // Send just to this specific dispatcher.
      found->second->Send(new PpapiHostMsg_LogWithSource(
          instance, static_cast<int>(level), source, value));
      return;
    }
  }

  // Instance 0 or invalid, send to all dispatchers.
  for (DispatcherSet::iterator i = g_live_dispatchers->begin();
       i != g_live_dispatchers->end(); ++i) {
    (*i)->Send(new PpapiHostMsg_LogWithSource(
        instance, static_cast<int>(level), source, value));
  }
}

const void* PluginDispatcher::GetPluginInterface(
    const std::string& interface_name) {
  InterfaceMap::iterator found = plugin_interfaces_.find(interface_name);
  if (found == plugin_interfaces_.end()) {
    const void* ret = local_get_interface()(interface_name.c_str());
    plugin_interfaces_.insert(std::make_pair(interface_name, ret));
    return ret;
  }
  return found->second;
}

bool PluginDispatcher::InitPluginWithChannel(
    PluginDelegate* delegate,
    const IPC::ChannelHandle& channel_handle,
    bool is_client) {
  if (!Dispatcher::InitWithChannel(delegate, channel_handle, is_client))
    return false;
  plugin_delegate_ = delegate;
  plugin_dispatcher_id_ = plugin_delegate_->Register(this);

  // The message filter will intercept and process certain messages directly
  // on the I/O thread.
  channel()->AddFilter(
      new PluginMessageFilter(delegate->GetGloballySeenInstanceIDSet()));
  return true;
}

bool PluginDispatcher::IsPlugin() const {
  return true;
}

bool PluginDispatcher::Send(IPC::Message* msg) {
  TRACE_EVENT2("ppapi proxy", "PluginDispatcher::Send",
               "Class", IPC_MESSAGE_ID_CLASS(msg->type()),
               "Line", IPC_MESSAGE_ID_LINE(msg->type()));
  // We always want plugin->renderer messages to arrive in-order. If some sync
  // and some async messages are sent in response to a synchronous
  // renderer->plugin call, the sync reply will be processed before the async
  // reply, and everything will be confused.
  //
  // Allowing all async messages to unblock the renderer means more reentrancy
  // there but gives correct ordering.
  msg->set_unblock(true);
  if (msg->is_sync()) {
    // Synchronous messages might be re-entrant, so we need to drop the lock.
    ProxyAutoUnlock unlock;
    return Dispatcher::Send(msg);
  }
  return Dispatcher::Send(msg);
}

bool PluginDispatcher::OnMessageReceived(const IPC::Message& msg) {
  // We need to grab the proxy lock to ensure that we don't collide with the
  // plugin making pepper calls on a different thread.
  ProxyAutoLock lock;
  TRACE_EVENT2("ppapi proxy", "PluginDispatcher::OnMessageReceived",
               "Class", IPC_MESSAGE_ID_CLASS(msg.type()),
               "Line", IPC_MESSAGE_ID_LINE(msg.type()));
  if (msg.routing_id() == MSG_ROUTING_CONTROL) {
    // Handle some plugin-specific control messages.
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(PluginDispatcher, msg)
      IPC_MESSAGE_HANDLER(PpapiMsg_SupportsInterface, OnMsgSupportsInterface)
      IPC_MESSAGE_HANDLER(PpapiMsg_SetPreferences, OnMsgSetPreferences)
      IPC_MESSAGE_UNHANDLED(handled = false);
    IPC_END_MESSAGE_MAP()
    if (handled)
      return true;
  }
  return Dispatcher::OnMessageReceived(msg);
}

void PluginDispatcher::OnChannelError() {
  Dispatcher::OnChannelError();

  // The renderer has crashed or exited. This channel and all instances
  // associated with it are no longer valid.
  ForceFreeAllInstances();
  // TODO(brettw) free resources too!
  delete this;
}

void PluginDispatcher::DidCreateInstance(PP_Instance instance) {
  if (!g_instance_to_dispatcher)
    g_instance_to_dispatcher = new InstanceToDispatcherMap;
  (*g_instance_to_dispatcher)[instance] = this;

  instance_map_[instance] = InstanceData();
}

void PluginDispatcher::DidDestroyInstance(PP_Instance instance) {
  InstanceDataMap::iterator it = instance_map_.find(instance);
  if (it != instance_map_.end())
    instance_map_.erase(it);

  if (g_instance_to_dispatcher) {
    InstanceToDispatcherMap::iterator found = g_instance_to_dispatcher->find(
        instance);
    if (found != g_instance_to_dispatcher->end()) {
      DCHECK(found->second == this);
      g_instance_to_dispatcher->erase(found);
    } else {
      NOTREACHED();
    }
  }
}

InstanceData* PluginDispatcher::GetInstanceData(PP_Instance instance) {
  InstanceDataMap::iterator it = instance_map_.find(instance);
  return (it == instance_map_.end()) ? NULL : &it->second;
}

FunctionGroupBase* PluginDispatcher::GetFunctionAPI(ApiID id) {
  return GetInterfaceProxy(id);
}

void PluginDispatcher::ForceFreeAllInstances() {
  if (!g_instance_to_dispatcher)
    return;

  // Iterating will remove each item from the map, so we need to make a copy
  // to avoid things changing out from under is.
  InstanceToDispatcherMap temp_map = *g_instance_to_dispatcher;
  for (InstanceToDispatcherMap::iterator i = temp_map.begin();
       i != temp_map.end(); ++i) {
    if (i->second == this) {
      // Synthesize an "instance destroyed" message, this will notify the
      // plugin and also remove it from our list of tracked plugins.
      PpapiMsg_PPPInstance_DidDestroy msg(API_ID_PPP_INSTANCE, i->first);
      OnMessageReceived(msg);
    }
  }
}

void PluginDispatcher::OnMsgSupportsInterface(
    const std::string& interface_name,
    bool* result) {
  *result = !!GetPluginInterface(interface_name);

  // Do fallback for PPP_Instance. This is a hack here and if we have more
  // cases like this it should be generalized. The PPP_Instance proxy always
  // proxies the 1.1 interface, and then does fallback to 1.0 inside the
  // plugin process (see PPP_Instance_Proxy). So here we return true for
  // supporting the 1.1 interface if either 1.1 or 1.0 is supported.
  if (!*result && interface_name == PPP_INSTANCE_INTERFACE)
    *result = !!GetPluginInterface(PPP_INSTANCE_INTERFACE_1_0);
}

void PluginDispatcher::OnMsgSetPreferences(const Preferences& prefs) {
  // The renderer may send us preferences more than once (currently this
  // happens every time a new plugin instance is created). Since we don't have
  // a way to signal to the plugin that the preferences have changed, changing
  // the default fonts and such in the middle of a running plugin could be
  // confusing to it. As a result, we never allow the preferences to be changed
  // once they're set. The user will have to restart to get new font prefs
  // propogated to plugins.
  if (!received_preferences_) {
    received_preferences_ = true;
    preferences_ = prefs;
  }
}

}  // namespace proxy
}  // namespace ppapi
