// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/input_method/xkeyboard.h"

#include <queue>
#include <set>
#include <string>
#include <utility>

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/process_util.h"
#include "base/string_util.h"
#include "base/stringprintf.h"
#include "chrome/browser/chromeos/cros/cros_library.h"
#include "chrome/browser/chromeos/input_method/input_method_util.h"
#include "content/browser/browser_thread.h"
#include "ui/base/x/x11_util.h"

namespace chromeos {
namespace input_method {
namespace {

// The default keyboard layout name in the xorg config file.
const char kDefaultLayoutName[] = "us";

// The command we use to set the current XKB layout and modifier key mapping.
// TODO(yusukes): Use libxkbfile.so instead of the command (crosbug.com/13105)
const char kSetxkbmapCommand[] = "/usr/bin/setxkbmap";

// See the comment at ModifierKey in the .h file.
ModifierKey kCustomizableKeys[] = {
  kSearchKey,
  kLeftControlKey,
  kLeftAltKey
};

// These arrays are generated by 'gen_keyboard_overlay_data.py --altgr'
// These are the input method IDs that shouldn't remap the right alt key.
const char* kKeepRightAltInputMethods[] = {
  "mozc-hangul",
  "xkb:be::fra",
  "xkb:be::ger",
  "xkb:be::nld",
  "xkb:bg::bul",
  "xkb:bg:phonetic:bul",
  "xkb:br::por",
  "xkb:ca::fra",
  "xkb:ca:eng:eng",
  "xkb:ch::ger",
  "xkb:ch:fr:fra",
  "xkb:cz::cze",
  "xkb:de::ger",
  "xkb:de:neo:ger",
  "xkb:dk::dan",
  "xkb:ee::est",
  "xkb:es::spa",
  "xkb:es:cat:cat",
  "xkb:fi::fin",
  "xkb:fr::fra",
  "xkb:gb:dvorak:eng",
  "xkb:gb:extd:eng",
  "xkb:gr::gre",
  "xkb:hr::scr",
  "xkb:il::heb",
  "xkb:it::ita",
  "xkb:kr:kr104:kor",
  "xkb:latam::spa",
  "xkb:lt::lit",
  "xkb:no::nob",
  "xkb:pl::pol",
  "xkb:pt::por",
  "xkb:ro::rum",
  "xkb:se::swe",
  "xkb:si::slv",
  "xkb:sk::slo",
  "xkb:tr::tur",
  "xkb:ua::ukr",
  "xkb:us:altgr-intl:eng",
  "xkb:us:intl:eng",
};

// These are the overlay names with caps lock remapped
const char* kCapsLockRemapped[] = {
  "xkb:de:neo:ger",
  "xkb:us:colemak:eng",
};

}  // namespace

XKeyboard::XKeyboard(const InputMethodUtil& util) {
  for (size_t i = 0; i < arraysize(kCustomizableKeys); ++i) {
    ModifierKey key = kCustomizableKeys[i];
    current_modifier_map_.push_back(ModifierKeyPair(key, key));
  }
  std::string layout;
  for (size_t i = 0; i < arraysize(kKeepRightAltInputMethods); ++i) {
    layout = util.GetKeyboardLayoutName(kKeepRightAltInputMethods[i]);
    // The empty check is necessary since TOUCH_UI build does not support some
    // of the kKeepRightAltInputMethods elements. For example, when TOUCH_UI is
    // defined, util.GetKeyboardLayoutName("xkb:us:intl:eng") would return "".
    if (!layout.empty()) {
      keep_right_alt_xkb_layout_names_.insert(layout);
    }
  }
  for (size_t i = 0; i < arraysize(kCapsLockRemapped); ++i) {
    layout = util.GetKeyboardLayoutName(kCapsLockRemapped[i]);
    // The empty check is for TOUCH_UI build. See above.
    if (!layout.empty()) {
      caps_lock_remapped_xkb_layout_names_.insert(layout);
    }
  }
}

XKeyboard::~XKeyboard() {
}

bool XKeyboard::SetLayoutInternal(const std::string& layout_name,
                                  const ModifierMap& modifier_map,
                                  bool force) {
  if (!CrosLibrary::Get()->EnsureLoaded()) {
    // We should not try to change a layout inside ui_tests.
    return false;
  }

  const std::string layout_to_set = CreateFullXkbLayoutName(
      layout_name, modifier_map);
  if (layout_to_set.empty()) {
    return false;
  }

  if (!current_layout_name_.empty()) {
    const std::string current_layout = CreateFullXkbLayoutName(
        current_layout_name_, current_modifier_map_);
    if (!force && (current_layout == layout_to_set)) {
      DLOG(INFO) << "The requested layout is already set: " << layout_to_set;
      return true;
    }
  }

  // Turn off caps lock if there is no kCapsLockKey in the remapped keys.
  if (!ContainsModifierKeyAsReplacement(modifier_map, kCapsLockKey)) {
    SetCapsLockEnabled(false);
  }

  VLOG(1) << (force ? "Reapply" : "Set") << " layout: " << layout_to_set;

  const bool start_execution = execute_queue_.empty();
  // If no setxkbmap command is in flight (i.e. start_execution is true),
  // start the first one by explicitly calling MaybeExecuteSetLayoutCommand().
  // If one or more setxkbmap commands are already in flight, just push the
  // layout name to the queue. setxkbmap command for the layout will be called
  // via OnSetLayoutFinish() callback later.
  execute_queue_.push(layout_to_set);
  if (start_execution) {
    MaybeExecuteSetLayoutCommand();
  }
  return true;
}

// Executes 'setxkbmap -layout ...' command asynchronously using a layout name
// in the |execute_queue_|. Do nothing if the queue is empty.
// TODO(yusukes): Use libxkbfile.so instead of the command (crosbug.com/13105)
void XKeyboard::MaybeExecuteSetLayoutCommand() {
  if (execute_queue_.empty()) {
    return;
  }
  const std::string layout_to_set = execute_queue_.front();

  std::vector<std::string> argv;
  base::ProcessHandle handle = base::kNullProcessHandle;

  argv.push_back(kSetxkbmapCommand);
  argv.push_back("-layout");
  argv.push_back(layout_to_set);
  argv.push_back("-synch");

  if (!base::LaunchProcess(argv, base::LaunchOptions(), &handle)) {
    LOG(ERROR) << "Failed to execute setxkbmap: " << layout_to_set;
    execute_queue_ = std::queue<std::string>();  // clear the queue.
    return;
  }

  // g_child_watch_add is necessary to prevent the process from becoming a
  // zombie.
  const base::ProcessId pid = base::GetProcId(handle);
  g_child_watch_add(pid,
                    reinterpret_cast<GChildWatchFunc>(OnSetLayoutFinish),
                    this);
  VLOG(1) << "ExecuteSetLayoutCommand: " << layout_to_set << ": pid=" << pid;
}

// static
void XKeyboard::OnSetLayoutFinish(pid_t pid, int status, XKeyboard* self) {
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  VLOG(1) << "OnSetLayoutFinish: pid=" << pid;
  if (self->execute_queue_.empty()) {
    LOG(ERROR) << "OnSetLayoutFinish: execute_queue_ is empty. "
               << "base::LaunchProcess failed? pid=" << pid;
    return;
  }
  self->execute_queue_.pop();
  self->MaybeExecuteSetLayoutCommand();
}

std::string XKeyboard::CreateFullXkbLayoutName(
    const std::string& layout_name, const ModifierMap& modifier_map) {
  static const char kValidLayoutNameCharacters[] =
      "abcdefghijklmnopqrstuvwxyz0123456789()-_";

  if (layout_name.empty()) {
    LOG(ERROR) << "Invalid layout_name: " << layout_name;
    return "";
  }

  if (layout_name.find_first_not_of(kValidLayoutNameCharacters) !=
      std::string::npos) {
    LOG(ERROR) << "Invalid layout_name: " << layout_name;
    return "";
  }

  std::string use_search_key_as_str;
  std::string use_left_control_key_as_str;
  std::string use_left_alt_key_as_str;

  for (size_t i = 0; i < modifier_map.size(); ++i) {
    std::string* target = NULL;
    switch (modifier_map[i].original) {
      case kSearchKey:
        target = &use_search_key_as_str;
        break;
      case kLeftControlKey:
        target = &use_left_control_key_as_str;
        break;
      case kLeftAltKey:
        target = &use_left_alt_key_as_str;
        break;
      default:
        break;
    }
    if (!target) {
      LOG(ERROR) << "We don't support remaping "
                 << ModifierKeyToString(modifier_map[i].original);
      return "";
    }
    if (!(target->empty())) {
      LOG(ERROR) << ModifierKeyToString(modifier_map[i].original)
                 << " appeared twice";
      return "";
    }
    *target = ModifierKeyToString(modifier_map[i].replacement);
  }

  if (use_search_key_as_str.empty() ||
      use_left_control_key_as_str.empty() ||
      use_left_alt_key_as_str.empty()) {
    LOG(ERROR) << "Incomplete ModifierMap: size=" << modifier_map.size();
    return "";
  }

  if (KeepCapsLock(layout_name)) {
    use_search_key_as_str = ModifierKeyToString(kSearchKey);
  }

  std::string full_xkb_layout_name =
      base::StringPrintf("%s+chromeos(%s_%s_%s%s)",
                         layout_name.c_str(),
                         use_search_key_as_str.c_str(),
                         use_left_control_key_as_str.c_str(),
                         use_left_alt_key_as_str.c_str(),
                         (KeepRightAlt(layout_name) ? "_keepralt" : ""));

  if ((full_xkb_layout_name.substr(0, 3) != "us+") &&
      (full_xkb_layout_name.substr(0, 3) != "us(")) {
    full_xkb_layout_name += ",us";
  }

  return full_xkb_layout_name;
}

// static
bool XKeyboard::SetAutoRepeatEnabled(bool enabled) {
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (enabled) {
    XAutoRepeatOn(ui::GetXDisplay());
  } else {
    XAutoRepeatOff(ui::GetXDisplay());
  }
  DLOG(INFO) << "Set auto-repeat mode to: " << (enabled ? "on" : "off");
  return true;
}

// static
bool XKeyboard::SetAutoRepeatRate(const AutoRepeatRate& rate) {
  // TODO(yusukes): write auto tests for the function.
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  DLOG(INFO) << "Set auto-repeat rate to: "
             << rate.initial_delay_in_ms << " ms delay, "
             << rate.repeat_interval_in_ms << " ms interval";
  if (XkbSetAutoRepeatRate(ui::GetXDisplay(), XkbUseCoreKbd,
                           rate.initial_delay_in_ms,
                           rate.repeat_interval_in_ms) != True) {
    LOG(ERROR) << "Failed to set auto-repeat rate";
    return false;
  }
  return true;
}

// static
bool XKeyboard::CapsLockIsEnabled() {
  // Do not call CHECK(BrowserThread::CurrentlyOn(BrowserThread::UI)); to make
  // unit_tests happy.
  XkbStateRec status;
  XkbGetState(ui::GetXDisplay(), XkbUseCoreKbd, &status);
  return status.locked_mods & LockMask;
}

// static
void XKeyboard::SetCapsLockEnabled(bool enable_caps_lock) {
  // Do not call CHECK(BrowserThread::CurrentlyOn(BrowserThread::UI)); to make
  // unit_tests happy.
  XkbLockModifiers(ui::GetXDisplay(), XkbUseCoreKbd, LockMask,
                   enable_caps_lock ? LockMask : 0);
}

// static
bool XKeyboard::ContainsModifierKeyAsReplacement(
    const ModifierMap& modifier_map, ModifierKey key) {
  for (size_t i = 0; i < modifier_map.size(); ++i) {
    if (modifier_map[i].replacement == key) {
      return true;
    }
  }
  return false;
}

bool XKeyboard::SetCurrentKeyboardLayoutByName(const std::string& layout_name) {
  if (SetLayoutInternal(layout_name, current_modifier_map_, false)) {
    current_layout_name_ = layout_name;
    return true;
  }
  return false;
}

bool XKeyboard::ReapplyCurrentKeyboardLayout() {
  if (current_layout_name_.empty()) {
    LOG(ERROR) << "Can't reapply XKB layout: layout unknown";
    return false;
  }
  return SetLayoutInternal(
      current_layout_name_, current_modifier_map_, true /* force */);
}

bool XKeyboard::RemapModifierKeys(const ModifierMap& modifier_map) {
  const std::string layout_name = current_layout_name_.empty() ?
      kDefaultLayoutName : current_layout_name_;
  if (SetLayoutInternal(layout_name, modifier_map, false)) {
    current_layout_name_ = layout_name;
    current_modifier_map_ = modifier_map;
    return true;
  }
  return false;
}

bool XKeyboard::KeepRightAlt(const std::string& xkb_layout_name) const {
  return keep_right_alt_xkb_layout_names_.count(xkb_layout_name) > 0;
}

bool XKeyboard::KeepCapsLock(const std::string& xkb_layout_name) const {
  return caps_lock_remapped_xkb_layout_names_.count(xkb_layout_name) > 0;
}

// static
std::string XKeyboard::ModifierKeyToString(ModifierKey key) {
  switch (key) {
    case kSearchKey:
      return "search";
    case kLeftControlKey:
      return "leftcontrol";
    case kLeftAltKey:
      return "leftalt";
    case kVoidKey:
      return "disabled";
    case kCapsLockKey:
      return "capslock";
    case kNumModifierKeys:
      break;
  }
  return "";
}

}  // namespace input_method
}  // namespace chromeos
