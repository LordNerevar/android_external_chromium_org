// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "base/command_line.h"
#include "base/compiler_specific.h"
#include "base/containers/hash_tables.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/scoped_vector.h"
#include "base/pickle.h"
#include "base/posix/eintr_wrapper.h"
#include "base/safe_strerror_posix.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "tools/android/forwarder2/common.h"
#include "tools/android/forwarder2/daemon.h"
#include "tools/android/forwarder2/host_controller.h"
#include "tools/android/forwarder2/pipe_notifier.h"
#include "tools/android/forwarder2/socket.h"

namespace forwarder2 {
namespace {

const char kLogFilePath[] = "/tmp/host_forwarder_log";
const char kDaemonIdentifier[] = "chrome_host_forwarder_daemon";

const char kKillServerCommand[] = "kill-server";
const char kForwardCommand[] = "forward";

const int kBufSize = 256;

// Needs to be global to be able to be accessed from the signal handler.
PipeNotifier* g_notifier = NULL;

// Lets the daemon fetch the exit notifier file descriptor.
int GetExitNotifierFD() {
  DCHECK(g_notifier);
  return g_notifier->receiver_fd();
}

void KillHandler(int signal_number) {
  char buf[kBufSize];
  if (signal_number != SIGTERM && signal_number != SIGINT) {
    snprintf(buf, sizeof(buf), "Ignoring unexpected signal %d.", signal_number);
    SIGNAL_SAFE_LOG(WARNING, buf);
    return;
  }
  snprintf(buf, sizeof(buf), "Received signal %d.", signal_number);
  SIGNAL_SAFE_LOG(WARNING, buf);
  static int s_kill_handler_count = 0;
  CHECK(g_notifier);
  // If for some reason the forwarder get stuck in any socket waiting forever,
  // we can send a SIGKILL or SIGINT three times to force it die
  // (non-nicely). This is useful when debugging.
  ++s_kill_handler_count;
  if (!g_notifier->Notify() || s_kill_handler_count > 2)
    exit(1);
}

class ServerDelegate : public Daemon::ServerDelegate {
 public:
  ServerDelegate() : has_failed_(false) {}

  bool has_failed() const { return has_failed_; }

  // Daemon::ServerDelegate:
  virtual void Init() OVERRIDE {
    LOG(INFO) << "Starting host process daemon (pid=" << getpid() << ")";
    DCHECK(!g_notifier);
    g_notifier = new PipeNotifier();
    signal(SIGTERM, KillHandler);
    signal(SIGINT, KillHandler);
  }

  virtual void OnClientConnected(scoped_ptr<Socket> client_socket) OVERRIDE {
    char buf[kBufSize];
    const int bytes_read = client_socket->Read(buf, sizeof(buf));
    if (bytes_read <= 0) {
      if (client_socket->DidReceiveEvent())
        return;
      PError("Read()");
      has_failed_ = true;
      return;
    }
    const Pickle command_pickle(buf, bytes_read);
    PickleIterator pickle_it(command_pickle);
    std::string device_serial;
    CHECK(pickle_it.ReadString(&device_serial));
    int device_port;
    if (!pickle_it.ReadInt(&device_port)) {
      SendMessage("ERROR: missing device port", client_socket.get());
      return;
    }
    const int adb_port = GetAdbPortForDevice(device_serial);
    if (adb_port < 0) {
      SendMessage(
          "ERROR: could not get adb port for device. You might need to add "
          "'adb' to your PATH or provide the device serial id.",
          client_socket.get());
      return;
    }
    if (device_port < 0) {
      // Remove the previously created host controller.
      const std::string controller_key = MakeHostControllerMapKey(
          adb_port, -device_port);
      const HostControllerMap::size_type removed_elements = controllers_.erase(
          controller_key);
      SendMessage(
          !removed_elements ? "ERROR: could not unmap port" : "OK",
          client_socket.get());
      return;
    }
    int host_port;
    if (!pickle_it.ReadInt(&host_port)) {
      SendMessage("ERROR: missing host port", client_socket.get());
      return;
    }
    // Create a new host controller.
    scoped_ptr<HostController> host_controller(
        new HostController(device_port, "127.0.0.1", host_port, adb_port,
                           GetExitNotifierFD()));
    if (!host_controller->Connect()) {
      has_failed_ = true;
      SendMessage("ERROR: Connection to device failed.", client_socket.get());
      return;
    }
    // Get the current allocated port.
    device_port = host_controller->device_port();
    LOG(INFO) << "Forwarding device port " << device_port << " to host port "
              << host_port;
    const std::string msg = base::StringPrintf("%d:%d", device_port, host_port);
    if (!SendMessage(msg, client_socket.get()))
      return;
    host_controller->Start();
    const std::string controller_key = MakeHostControllerMapKey(
        adb_port, device_port);
    controllers_.insert(
        std::make_pair(controller_key,
                       linked_ptr<HostController>(host_controller.release())));
  }

  virtual void OnServerExited() OVERRIDE {
    for (HostControllerMap::iterator it = controllers_.begin();
         it != controllers_.end(); ++it) {
      linked_ptr<HostController> host_controller = it->second;
      host_controller->Join();
    }
    if (controllers_.size() == 0) {
      LOG(ERROR) << "No forwarder servers could be started. Exiting.";
      has_failed_ = true;
    }
  }

 private:
  typedef base::hash_map<
      std::string, linked_ptr<HostController> > HostControllerMap;

  static std::string MakeHostControllerMapKey(int adb_port, int device_port) {
    return base::StringPrintf("%d:%d", adb_port, device_port);
  }

  int GetAdbPortForDevice(const std::string& device_serial) {
    base::hash_map<std::string, int>::const_iterator it =
        device_serial_to_adb_port_map_.find(device_serial);
    if (it != device_serial_to_adb_port_map_.end())
      return it->second;
    Socket bind_socket;
    CHECK(bind_socket.BindTcp("127.0.0.1", 0));
    const int port = bind_socket.GetPort();
    bind_socket.Close();
    const std::string serial_part = device_serial.empty() ?
        std::string() : std::string("-s ") + device_serial;
    const std::string command = base::StringPrintf(
        "adb %s forward tcp:%d localabstract:chrome_device_forwarder",
        device_serial.empty() ? "" : serial_part.c_str(),
        port);
    LOG(INFO) << command;
    const int ret = system(command.c_str());
    if (ret < 0 || !WIFEXITED(ret) || WEXITSTATUS(ret) != 0)
      return -1;
    device_serial_to_adb_port_map_[device_serial] = port;
    return port;
  }

  bool SendMessage(const std::string& msg, Socket* client_socket) {
    bool result = client_socket->WriteString(msg);
    DCHECK(result);
    if (!result)
      has_failed_ = true;
    return result;
  }

  base::hash_map<std::string, int> device_serial_to_adb_port_map_;
  HostControllerMap controllers_;
  bool has_failed_;

  DISALLOW_COPY_AND_ASSIGN(ServerDelegate);
};

class ClientDelegate : public Daemon::ClientDelegate {
 public:
  ClientDelegate(const Pickle& command_pickle)
      : command_pickle_(command_pickle),
        has_failed_(false) {
  }

  bool has_failed() const { return has_failed_; }

  // Daemon::ClientDelegate:
  virtual void OnDaemonReady(Socket* daemon_socket) OVERRIDE {
    // Send the forward command to the daemon.
    CHECK_EQ(command_pickle_.size(),
             daemon_socket->WriteNumBytes(command_pickle_.data(),
                                          command_pickle_.size()));
    char buf[kBufSize];
    const int bytes_read = daemon_socket->Read(
        buf, sizeof(buf) - 1 /* leave space for null terminator */);
    CHECK_GT(bytes_read, 0);
    DCHECK(bytes_read < sizeof(buf));
    buf[bytes_read] = 0;
    base::StringPiece msg(buf, bytes_read);
    if (msg.starts_with("ERROR")) {
      LOG(ERROR) << msg;
      has_failed_ = true;
      return;
    }
    printf("%s\n", buf);
  }

 private:
  const Pickle command_pickle_;
  bool has_failed_;
};

void ExitWithUsage() {
  std::cerr << "Usage: host_forwarder [options]\n\n"
               "Options:\n"
               "  --serial-id=[0-9A-Z]{16}]\n"
               "  --map DEVICE_PORT HOST_PORT\n"
               "  --unmap DEVICE_PORT\n"
               "  --kill-server\n";
  exit(1);
}

int PortToInt(const std::string& s) {
  int value;
  // Note that 0 is a valid port (used for dynamic port allocation).
  if (!base::StringToInt(s, &value) || value < 0 ||
      value > std::numeric_limits<uint16>::max()) {
    LOG(ERROR) << "Could not convert string " << s << " to port";
    ExitWithUsage();
  }
  return value;
}

int RunHostForwarder(int argc, char** argv) {
  CommandLine::Init(argc, argv);
  const CommandLine& cmd_line = *CommandLine::ForCurrentProcess();
  bool kill_server = false;

  Pickle pickle;
  pickle.WriteString(
      cmd_line.HasSwitch("serial-id") ?
          cmd_line.GetSwitchValueASCII("serial-id") : std::string());

  const std::vector<std::string> args = cmd_line.GetArgs();
  if (cmd_line.HasSwitch("kill-server")) {
    kill_server = true;
  } else if (cmd_line.HasSwitch("unmap")) {
    if (args.size() != 1)
      ExitWithUsage();
    // Note the minus sign below.
    pickle.WriteInt(-PortToInt(args[0]));
  } else if (cmd_line.HasSwitch("map")) {
    if (args.size() != 2)
      ExitWithUsage();
    pickle.WriteInt(PortToInt(args[0]));
    pickle.WriteInt(PortToInt(args[1]));
  } else {
    ExitWithUsage();
  }

  if (kill_server && args.size() > 0)
    ExitWithUsage();

  ClientDelegate client_delegate(pickle);
  ServerDelegate daemon_delegate;
  Daemon daemon(
      kLogFilePath, kDaemonIdentifier, &client_delegate, &daemon_delegate,
      &GetExitNotifierFD);

  if (kill_server)
    return !daemon.Kill();
  if (!daemon.SpawnIfNeeded())
    return 1;

  return client_delegate.has_failed() || daemon_delegate.has_failed();
}

}  // namespace
}  // namespace forwarder2

int main(int argc, char** argv) {
  return forwarder2::RunHostForwarder(argc, argv);
}
