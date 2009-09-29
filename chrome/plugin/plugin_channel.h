// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_PLUGIN_PLUGIN_CHANNEL_H_
#define CHROME_PLUGIN_PLUGIN_CHANNEL_H_

#include <vector>
#include "base/ref_counted.h"
#include "base/scoped_handle.h"
#include "build/build_config.h"
#include "chrome/plugin/plugin_channel_base.h"
#include "chrome/plugin/webplugin_delegate_stub.h"

namespace base {
class WaitableEvent;
}

// Encapsulates an IPC channel between the plugin process and one renderer
// process.  On the renderer side there's a corresponding PluginChannelHost.
class PluginChannel : public PluginChannelBase {
 public:
  // Get a new PluginChannel object for the current process to talk to the
  // given renderer process. The renderer ID is an opaque unique ID generated
  // by the browser.
  //
  // POSIX only: If |channel_fd| > 0, use that file descriptor for the
  // channel socket.
  static PluginChannel* GetPluginChannel(int renderer_id,
                                         MessageLoop* ipc_message_loop);

  ~PluginChannel();

  virtual bool Send(IPC::Message* msg);
  virtual void OnMessageReceived(const IPC::Message& message);

  base::ProcessHandle renderer_handle() const { return renderer_handle_; }
  int renderer_id() { return renderer_id_; }

  int GenerateRouteID();

  // Returns the event that's set when a call to the renderer causes a modal
  // dialog to come up.
  base::WaitableEvent* GetModalDialogEvent(gfx::NativeViewId containing_window);

#if defined(OS_POSIX)
  // When first created, the PluginChannel gets assigned the file descriptor
  // for the renderer.
  // After the first time we pass it through the IPC, we don't need it anymore,
  // and we close it. At that time, we reset renderer_fd_ to -1.
  int DisownRendererFd() {
    int value = renderer_fd_;
    renderer_fd_ = -1;
    return value;
  }
#endif

  bool in_send() { return in_send_ != 0; }

  bool off_the_record() { return off_the_record_; }
  void set_off_the_record(bool value) { off_the_record_ = value; }

 protected:
  // IPC::Channel::Listener implementation:
  virtual void OnChannelConnected(int32 peer_pid);
  virtual void OnChannelError();

  virtual void CleanUp();

  // Overrides PluginChannelBase::Init.
  virtual bool Init(MessageLoop* ipc_message_loop, bool create_pipe_now);

 private:
  class MessageFilter;

  // Called on the plugin thread
  PluginChannel();

  void OnControlMessageReceived(const IPC::Message& msg);

  static PluginChannelBase* ClassFactory() { return new PluginChannel(); }

  void OnCreateInstance(const std::string& mime_type, int* instance_id);
  void OnDestroyInstance(int instance_id, IPC::Message* reply_msg);
  void OnGenerateRouteID(int* route_id);

  std::vector<scoped_refptr<WebPluginDelegateStub> > plugin_stubs_;

  // Handle to the renderer process who is on the other side of the channel.
  base::ProcessHandle renderer_handle_;

  // The id of the renderer who is on the other side of the channel.
  int renderer_id_;

#if defined(OS_POSIX)
  // FD for the renderer end of the pipe. It is stored until we send it over
  // IPC after which it is cleared. It will be closed by the IPC mechanism.
  int renderer_fd_;
#endif

  int in_send_;  // Tracks if we're in a Send call.
  bool log_messages_;  // True if we should log sent and received messages.
  bool off_the_record_; // True if the renderer is in off the record mode.
  scoped_refptr<MessageFilter> filter_;  // Handles the modal dialog events.

  DISALLOW_EVIL_CONSTRUCTORS(PluginChannel);
};

#endif  // CHROME_PLUGIN_PLUGIN_CHANNEL_H_
