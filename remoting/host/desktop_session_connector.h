// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_HOST_DESKTOP_SESSION_CONNECTOR_H_
#define REMOTING_HOST_DESKTOP_SESSION_CONNECTOR_H_

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/process.h"
#include "ipc/ipc_platform_file.h"

namespace remoting {

class DesktopSessionProxy;

// Provides a way to connect a terminal (i.e. a remote client) with a desktop
// session (i.e. the screen, keyboard and the rest).
class DesktopSessionConnector {
 public:
  DesktopSessionConnector() {}
  virtual ~DesktopSessionConnector() {}

  // Requests the daemon process to create a desktop session and associates
  // |desktop_session_proxy| with it.
  virtual void ConnectTerminal(
      scoped_refptr<DesktopSessionProxy> desktop_session_proxy) = 0;

  // Requests the daemon process disconnect |desktop_session_proxy| from
  // the associated desktop session.
  virtual void DisconnectTerminal(
      scoped_refptr<DesktopSessionProxy> desktop_session_proxy) = 0;

  // Notifies the network process that |terminal_id| is now attached to
  // a desktop integration process. |desktop_process| specifies the process
  // handle. |desktop_pipe| is the client end of the pipe opened by the desktop
  // process.
  virtual void OnDesktopSessionAgentAttached(
      int terminal_id,
      base::ProcessHandle desktop_process,
      IPC::PlatformFileForTransit desktop_pipe) = 0;

  // Notifies the network process that the daemon has disconnected the desktop
  // session from the associated descktop environment.
  virtual void OnTerminalDisconnected(int terminal_id) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(DesktopSessionConnector);
};

}  // namespace remoting

#endif  // REMOTING_HOST_DESKTOP_SESSION_CONNECTOR_H_
