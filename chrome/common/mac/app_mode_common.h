// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_COMMON_MAC_APP_MODE_COMMON_H_
#define CHROME_COMMON_MAC_APP_MODE_COMMON_H_

#import <Foundation/Foundation.h>

#include "base/files/file_path.h"
#include "base/strings/string16.h"

// This file contains constants, interfaces, etc. which are common to the
// browser application and the app mode loader (a.k.a. shim).

namespace app_mode {

// These are keys for an Apple Event ping that the app shim process sends to
// Chrome to get confirmation that Chrome is alive. The main Chrome process
// doesn't need to register any handlers for them -- the event is just sent for
// the empty reply that's automatically returned by the system.
const AEEventClass kAEChromeAppClass = 'cApp';
const AEEventID kAEChromeAppPing = 'ping';

// The IPC socket used to communicate between app shims and Chrome will be
// created under a temporary directory with this name.
extern const char kAppShimSocketShortName[];
// A symlink to allow the app shim to find the socket will be created under the
// user data dir with this name.
extern const char kAppShimSocketSymlinkName[];

// Special app mode id used for the App Launcher.
extern const char kAppListModeId[];

// The process ID of the Chrome process that launched the app shim.
// The presence of this switch instructs the app shim to send LaunchApp with
// launch_now = false. This associates the shim without launching the app.
extern const char kLaunchedByChromeProcessId[];

// The display name of the bundle as shown in Finder and the Dock. For localized
// bundles, this overrides the bundle's file name.
extern NSString* const kCFBundleDisplayNameKey;

// The key specifying whether the display name should be localized. This makes
// Finder look in localization folders in the app bundle for a display name.
// (e.g. Content/Resources/en.lproj/)
extern NSString* const kLSHasLocalizedDisplayNameKey;

// The key under which the browser's bundle ID will be stored in the
// app mode launcher bundle's Info.plist.
extern NSString* const kBrowserBundleIDKey;

// Key for the shortcut ID.
extern NSString* const kCrAppModeShortcutIDKey;

// Key for the app's name.
extern NSString* const kCrAppModeShortcutNameKey;

// Key for the app's URL.
extern NSString* const kCrAppModeShortcutURLKey;

// Key for the app user data directory.
extern NSString* const kCrAppModeUserDataDirKey;

// Key for the app's extension path.
extern NSString* const kCrAppModeProfileDirKey;

// Key for the app's profile display name.
extern NSString* const kCrAppModeProfileNameKey;

// When the Chrome browser is run, it stores its location in the defaults
// system using this key.
extern NSString* const kLastRunAppBundlePathPrefsKey;

// Placeholders used in the app mode loader bundle' Info.plist:
extern NSString* const kShortcutIdPlaceholder; // Extension shortcut ID.
extern NSString* const kShortcutNamePlaceholder; // Extension name.
extern NSString* const kShortcutURLPlaceholder;
// Bundle ID of the Chrome browser bundle.
extern NSString* const kShortcutBrowserBundleIDPlaceholder;

// Current major/minor version numbers of |ChromeAppModeInfo| (defined below).
const unsigned kCurrentChromeAppModeInfoMajorVersion = 1;
const unsigned kCurrentChromeAppModeInfoMinorVersion = 0;

// The structure used to pass information from the app mode loader to the
// (browser) framework. This is versioned using major and minor version numbers,
// written below as v<major>.<minor>. Version-number checking is done by the
// framework, and the framework must accept all structures with the same major
// version number. It may refuse to load if the major version of the structure
// is different from the one it accepts.
struct ChromeAppModeInfo {
 public:
  ChromeAppModeInfo();
  ~ChromeAppModeInfo();

  // Major and minor version number of this structure.
  unsigned major_version;  // Required: all versions
  unsigned minor_version;  // Required: all versions

  // Original |argc| and |argv|.
  int argc;  // Required: v1.0
  char** argv;  // Required: v1.0

  // Versioned path to the browser which is being loaded.
  base::FilePath chrome_versioned_path;  // Required: v1.0

  // Path to Chrome app bundle.
  base::FilePath chrome_outer_bundle_path;  // Required: v1.0

  // Information about the App Mode shortcut:

  // Path to the App Mode Loader application bundle that launched the process.
  base::FilePath app_mode_bundle_path;  // Optional: v1.0

  // Short ID string, preferably derived from |app_mode_short_name|. Should be
  // safe for the file system.
  std::string app_mode_id;  // Required: v1.0

  // Unrestricted (e.g., several-word) UTF8-encoded name for the shortcut.
  base::string16 app_mode_name;  // Optional: v1.0

  // URL for the shortcut. Must be a valid URL.
  std::string app_mode_url;  // Required: v1.0

  // Path to the app's user data directory.
  base::FilePath user_data_dir;

  // Directory of the profile associated with the app.
  base::FilePath profile_dir;
};

// Check that the socket and its parent directory have the correct permissions
// and are owned by the user.
void VerifySocketPermissions(const base::FilePath& socket_path);

}  // namespace app_mode

#endif  // CHROME_COMMON_MAC_APP_MODE_COMMON_H_
