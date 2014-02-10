// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/app_shim/app_shim_host_manager_mac.h"

#include <unistd.h>

#include "apps/app_shim/app_shim_messages.h"
#include "apps/app_shim/test/app_shim_host_manager_test_api_mac.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/common/mac/app_mode_common.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "content/public/test/test_utils.h"
#include "ipc/ipc_channel_proxy.h"
#include "ipc/ipc_listener.h"
#include "ipc/ipc_message.h"

namespace {

const char kTestAppMode[] = "test_app";

// A test version of the AppShimController IPC client in chrome_main_app_mode.
class TestShimClient : public IPC::Listener {
 public:
  TestShimClient();
  virtual ~TestShimClient();

  template <class T>
  void Send(const T& message) {
    channel_->Send(message);
  }

 private:
  // IPC::Listener overrides:
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void OnChannelError() OVERRIDE;

  base::Thread io_thread_;
  scoped_ptr<IPC::ChannelProxy> channel_;

  DISALLOW_COPY_AND_ASSIGN(TestShimClient);
};

TestShimClient::TestShimClient() : io_thread_("TestShimClientIO") {
  base::Thread::Options io_thread_options;
  io_thread_options.message_loop_type = base::MessageLoop::TYPE_IO;
  io_thread_.StartWithOptions(io_thread_options);

  base::FilePath user_data_dir;
  CHECK(PathService::Get(chrome::DIR_USER_DATA, &user_data_dir));
  base::FilePath symlink_path =
      user_data_dir.Append(app_mode::kAppShimSocketSymlinkName);

  base::FilePath socket_path;
  CHECK(base::ReadSymbolicLink(symlink_path, &socket_path));
  app_mode::VerifySocketPermissions(socket_path);

  IPC::ChannelHandle handle(socket_path.value());
  channel_.reset(new IPC::ChannelProxy(handle, IPC::Channel::MODE_NAMED_CLIENT,
      this, io_thread_.message_loop_proxy().get()));
}

TestShimClient::~TestShimClient() {}

bool TestShimClient::OnMessageReceived(const IPC::Message& message) {
  return true;
}

void TestShimClient::OnChannelError() {
  // Client should not get any channel errors for the current set of tests.
  PLOG(FATAL) << "ChannelError";
}

// Browser Test for AppShimHostManager to test IPC interactions across the
// UNIX domain socket.
class AppShimHostManagerBrowserTest : public InProcessBrowserTest,
                                      public apps::AppShimHandler {
 public:
  AppShimHostManagerBrowserTest();
  virtual ~AppShimHostManagerBrowserTest();

 protected:
  // Wait for OnShimLaunch, then send a quit, and wait for the response. Used to
  // test launch behavior.
  void RunAndExitGracefully();

  // InProcessBrowserTest overrides:
  virtual void SetUpOnMainThread() OVERRIDE;
  virtual void TearDownOnMainThread() OVERRIDE;

  // AppShimHandler overrides:
  virtual void OnShimLaunch(apps::AppShimHandler::Host* host,
                            apps::AppShimLaunchType launch_type,
                            const std::vector<base::FilePath>& files) OVERRIDE;
  virtual void OnShimClose(apps::AppShimHandler::Host* host) OVERRIDE {}
  virtual void OnShimFocus(apps::AppShimHandler::Host* host,
                           apps::AppShimFocusType focus_type,
                           const std::vector<base::FilePath>& files) OVERRIDE {}
  virtual void OnShimSetHidden(apps::AppShimHandler::Host* host,
                               bool hidden) OVERRIDE {}
  virtual void OnShimQuit(apps::AppShimHandler::Host* host) OVERRIDE;

  scoped_ptr<TestShimClient> test_client_;
  std::vector<base::FilePath> last_launch_files_;
  apps::AppShimLaunchType last_launch_type_;

private:
  scoped_refptr<content::MessageLoopRunner> runner_;

  int launch_count_;
  int quit_count_;

  DISALLOW_COPY_AND_ASSIGN(AppShimHostManagerBrowserTest);
};

AppShimHostManagerBrowserTest::AppShimHostManagerBrowserTest()
    : last_launch_type_(apps::APP_SHIM_LAUNCH_NUM_TYPES),
      launch_count_(0),
      quit_count_(0) {
}

AppShimHostManagerBrowserTest::~AppShimHostManagerBrowserTest() {
}

void AppShimHostManagerBrowserTest::RunAndExitGracefully() {
  runner_ = new content::MessageLoopRunner();
  EXPECT_EQ(0, launch_count_);
  runner_->Run();  // Will stop in OnShimLaunch().
  EXPECT_EQ(1, launch_count_);

  runner_ = new content::MessageLoopRunner();
  test_client_->Send(new AppShimHostMsg_QuitApp);
  EXPECT_EQ(0, quit_count_);
  runner_->Run();  // Will stop in OnShimQuit().
  EXPECT_EQ(1, quit_count_);

  test_client_.reset();
}

void AppShimHostManagerBrowserTest::SetUpOnMainThread() {
  // Can't do this in the constructor, it needs a BrowserProcess.
  apps::AppShimHandler::RegisterHandler(kTestAppMode, this);
}

void AppShimHostManagerBrowserTest::TearDownOnMainThread() {
  apps::AppShimHandler::RemoveHandler(kTestAppMode);
}

void AppShimHostManagerBrowserTest::OnShimLaunch(
    apps::AppShimHandler::Host* host,
    apps::AppShimLaunchType launch_type,
    const std::vector<base::FilePath>& files) {
  host->OnAppLaunchComplete(apps::APP_SHIM_LAUNCH_SUCCESS);
  ++launch_count_;
  last_launch_type_ = launch_type;
  last_launch_files_ = files;
  runner_->Quit();
}

void AppShimHostManagerBrowserTest::OnShimQuit(
    apps::AppShimHandler::Host* host) {
  ++quit_count_;
  runner_->Quit();
}

// Test regular launch, which would ask Chrome to launch the app.
IN_PROC_BROWSER_TEST_F(AppShimHostManagerBrowserTest, LaunchNormal) {
  test_client_.reset(new TestShimClient());
  test_client_->Send(new AppShimHostMsg_LaunchApp(
      browser()->profile()->GetPath(),
      kTestAppMode,
      apps::APP_SHIM_LAUNCH_NORMAL,
      std::vector<base::FilePath>()));

  RunAndExitGracefully();
  EXPECT_EQ(apps::APP_SHIM_LAUNCH_NORMAL, last_launch_type_);
  EXPECT_TRUE(last_launch_files_.empty());
}

// Test register-only launch, used when Chrome has already launched the app.
IN_PROC_BROWSER_TEST_F(AppShimHostManagerBrowserTest, LaunchRegisterOnly) {
  test_client_.reset(new TestShimClient());
  test_client_->Send(new AppShimHostMsg_LaunchApp(
      browser()->profile()->GetPath(),
      kTestAppMode,
      apps::APP_SHIM_LAUNCH_REGISTER_ONLY,
      std::vector<base::FilePath>()));

  RunAndExitGracefully();
  EXPECT_EQ(apps::APP_SHIM_LAUNCH_REGISTER_ONLY, last_launch_type_);
  EXPECT_TRUE(last_launch_files_.empty());
}

// Ensure the domain socket can be created in a fresh user data dir.
IN_PROC_BROWSER_TEST_F(AppShimHostManagerBrowserTest,
                       PRE_ReCreate) {
  test::AppShimHostManagerTestApi test_api(
      g_browser_process->platform_part()->app_shim_host_manager());
  EXPECT_TRUE(test_api.factory());
}

// Ensure the domain socket can be re-created after a prior browser process has
// quit.
IN_PROC_BROWSER_TEST_F(AppShimHostManagerBrowserTest,
                       ReCreate) {
  test::AppShimHostManagerTestApi test_api(
      g_browser_process->platform_part()->app_shim_host_manager());
  EXPECT_TRUE(test_api.factory());
}

// Tests for the files created by AppShimHostManager.
class AppShimHostManagerBrowserTestSocketFiles
    : public AppShimHostManagerBrowserTest {
 public:
  AppShimHostManagerBrowserTestSocketFiles() {}

 protected:
  base::FilePath directory_in_tmp_;
  base::FilePath symlink_path_;

 private:
  virtual bool SetUpUserDataDirectory() OVERRIDE;
  virtual void TearDownInProcessBrowserTestFixture() OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(AppShimHostManagerBrowserTestSocketFiles);
};

bool AppShimHostManagerBrowserTestSocketFiles::SetUpUserDataDirectory() {
  // Create an existing symlink. It should be replaced by AppShimHostManager.
  base::FilePath user_data_dir;
  EXPECT_TRUE(PathService::Get(chrome::DIR_USER_DATA, &user_data_dir));
  symlink_path_ = user_data_dir.Append(app_mode::kAppShimSocketSymlinkName);
  base::FilePath temp_dir;
  PathService::Get(base::DIR_TEMP, &temp_dir);
  EXPECT_TRUE(base::CreateSymbolicLink(temp_dir.Append("chrome-XXXXXX"),
                                       symlink_path_));
  return AppShimHostManagerBrowserTest::SetUpUserDataDirectory();
}

void AppShimHostManagerBrowserTestSocketFiles::
    TearDownInProcessBrowserTestFixture() {
  // Check that created files have been deleted.
  EXPECT_FALSE(base::PathExists(directory_in_tmp_));
  EXPECT_FALSE(base::PathExists(symlink_path_));
}

IN_PROC_BROWSER_TEST_F(AppShimHostManagerBrowserTestSocketFiles,
                       ReplacesSymlinkAndCleansUpFiles) {
  // Get the directory created by AppShimHostManager.
  test::AppShimHostManagerTestApi test_api(
      g_browser_process->platform_part()->app_shim_host_manager());
  directory_in_tmp_ = test_api.directory_in_tmp();

  // Check that socket files have been created.
  EXPECT_TRUE(base::PathExists(directory_in_tmp_));
  EXPECT_TRUE(base::PathExists(symlink_path_));

  // Check that the symlink has been replaced.
  base::FilePath socket_path;
  ASSERT_TRUE(base::ReadSymbolicLink(symlink_path_, &socket_path));
  EXPECT_EQ(app_mode::kAppShimSocketShortName, socket_path.BaseName().value());
}

}  // namespace
