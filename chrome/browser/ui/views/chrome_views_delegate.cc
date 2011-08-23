// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/chrome_views_delegate.h"

#include "base/memory/scoped_ptr.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/browser_shutdown.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/prefs/scoped_user_pref_update.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/views/accessibility_event_router_views.h"
#include "chrome/browser/ui/views/event_utils.h"
#include "chrome/browser/ui/window_sizer.h"
#include "chrome/common/pref_names.h"
#include "ui/base/clipboard/clipboard.h"
#include "ui/gfx/rect.h"
#include "views/widget/native_widget.h"
#include "views/widget/widget.h"

#if defined(OS_WIN)
#include "chrome/browser/app_icon_win.h"
#endif

namespace {

// If the given window has a profile associated with it, use that profile's
// preference service. Otherwise, store and retrieve the data from Local State.
// This function may return NULL if the necessary pref service has not yet
// been initialized.
// TODO(mirandac): This function will also separate windows by profile in a
// multi-profile environment.
// TODO(sky): remove is_incognito. Used in tracking 91396.
PrefService* GetPrefsForWindow(const views::Widget* window,
                               bool* is_incognito) {
  Profile* profile = reinterpret_cast<Profile*>(
      window->GetNativeWindowProperty(Profile::kProfileKey));
  *is_incognito = false;
  if (!profile) {
    // Use local state for windows that have no explicit profile.
    return g_browser_process->local_state();
  }
  *is_incognito = profile->IsOffTheRecord();
  return profile->GetPrefs();
}

}  // namespace

// static
views::View* ChromeViewsDelegate::default_parent_view = NULL;

///////////////////////////////////////////////////////////////////////////////
// ChromeViewsDelegate, views::ViewsDelegate implementation:

ui::Clipboard* ChromeViewsDelegate::GetClipboard() const {
  return g_browser_process->clipboard();
}

views::View* ChromeViewsDelegate::GetDefaultParentView() {
  return default_parent_view;
}

void ChromeViewsDelegate::SaveWindowPlacement(const views::Widget* window,
                                              const std::wstring& window_name,
                                              const gfx::Rect& bounds,
                                              bool maximized) {
  bool is_incognito = false;
  PrefService* prefs = GetPrefsForWindow(window, &is_incognito);
  if (!prefs)
    return;

  volatile browser_shutdown::ShutdownType shutdown_type =
      browser_shutdown::GetShutdownType();
  CHECK(prefs->FindPreference(WideToUTF8(window_name).c_str())) <<
      is_incognito << " " << shutdown_type;
  DictionaryPrefUpdate update(prefs, WideToUTF8(window_name).c_str());
  DictionaryValue* window_preferences = update.Get();
  CHECK(window_preferences) << is_incognito << " " << shutdown_type;
  window_preferences->SetInteger("left", bounds.x());
  window_preferences->SetInteger("top", bounds.y());
  window_preferences->SetInteger("right", bounds.right());
  window_preferences->SetInteger("bottom", bounds.bottom());
  window_preferences->SetBoolean("maximized", maximized);

  scoped_ptr<WindowSizer::MonitorInfoProvider> monitor_info_provider(
      WindowSizer::CreateDefaultMonitorInfoProvider());
  gfx::Rect work_area(
      monitor_info_provider->GetMonitorWorkAreaMatching(bounds));
  window_preferences->SetInteger("work_area_left", work_area.x());
  window_preferences->SetInteger("work_area_top", work_area.y());
  window_preferences->SetInteger("work_area_right", work_area.right());
  window_preferences->SetInteger("work_area_bottom", work_area.bottom());
}

bool ChromeViewsDelegate::GetSavedWindowBounds(const std::wstring& window_name,
                                               gfx::Rect* bounds) const {
  PrefService* prefs = g_browser_process->local_state();
  if (!prefs)
    return false;

  DCHECK(prefs->FindPreference(WideToUTF8(window_name).c_str()));
  const DictionaryValue* dictionary =
      prefs->GetDictionary(WideToUTF8(window_name).c_str());
  int left, top, right, bottom;
  if (!dictionary || !dictionary->GetInteger("left", &left) ||
      !dictionary->GetInteger("top", &top) ||
      !dictionary->GetInteger("right", &right) ||
      !dictionary->GetInteger("bottom", &bottom))
    return false;

  bounds->SetRect(left, top, right - left, bottom - top);
  return true;
}

bool ChromeViewsDelegate::GetSavedMaximizedState(
    const std::wstring& window_name,
    bool* maximized) const {
  PrefService* prefs = g_browser_process->local_state();
  if (!prefs)
    return false;

  DCHECK(prefs->FindPreference(WideToUTF8(window_name).c_str()));
  const DictionaryValue* dictionary =
      prefs->GetDictionary(WideToUTF8(window_name).c_str());

  return dictionary && dictionary->GetBoolean("maximized", maximized) &&
      maximized;
}

void ChromeViewsDelegate::NotifyAccessibilityEvent(
    views::View* view, ui::AccessibilityTypes::Event event_type) {
  AccessibilityEventRouterViews::GetInstance()->HandleAccessibilityEvent(
      view, event_type);
}

void ChromeViewsDelegate::NotifyMenuItemFocused(
      const std::wstring& menu_name,
      const std::wstring& menu_item_name,
      int item_index,
      int item_count,
      bool has_submenu) {
  AccessibilityEventRouterViews::GetInstance()->HandleMenuItemFocused(
      menu_name, menu_item_name, item_index, item_count, has_submenu);
}

#if defined(OS_WIN)
HICON ChromeViewsDelegate::GetDefaultWindowIcon() const {
  return GetAppIcon();
}
#endif

void ChromeViewsDelegate::AddRef() {
  g_browser_process->AddRefModule();
}

void ChromeViewsDelegate::ReleaseRef() {
  g_browser_process->ReleaseModule();
}

int ChromeViewsDelegate::GetDispositionForEvent(int event_flags) {
  return event_utils::DispositionFromEventFlags(event_flags);
}
