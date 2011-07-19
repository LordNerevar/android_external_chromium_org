// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_EXTENSION_TABS_MODULE_H__
#define CHROME_BROWSER_EXTENSIONS_EXTENSION_TABS_MODULE_H__
#pragma once

#include <string>

#include "chrome/browser/extensions/extension_function.h"
#include "content/browser/tab_contents/tab_contents_observer.h"
#include "content/common/notification_observer.h"
#include "content/common/notification_registrar.h"

class BackingStore;
class Browser;
class SkBitmap;
class TabContents;
class TabContentsWrapper;
class TabStripModel;

namespace base {
class DictionaryValue;
class ListValue;
}

namespace ExtensionTabUtil {
int GetWindowId(const Browser* browser);
int GetTabId(const TabContents* tab_contents);
std::string GetTabStatusText(bool is_loading);
int GetWindowIdOfTab(const TabContents* tab_contents);
base::ListValue* CreateTabList(const Browser* browser);
base::DictionaryValue* CreateTabValue(const TabContents* tab_contents);
base::DictionaryValue* CreateTabValue(const TabContents* tab_contents,
                                      TabStripModel* tab_strip,
                                      int tab_index);
// Create a tab value, overriding its kSelectedKey to the provided boolean.
base::DictionaryValue* CreateTabValueActive(const TabContents* tab_contents,
                                            bool active);
base::DictionaryValue* CreateWindowValue(const Browser* browser,
                                         bool populate_tabs);
// Gets the |tab_strip_model| and |tab_index| for the given |tab_contents|.
bool GetTabStripModel(const TabContents* tab_contents,
                      TabStripModel** tab_strip_model,
                      int* tab_index);
bool GetDefaultTab(Browser* browser,
                   TabContentsWrapper** contents,
                   int* tab_id);
// Any out parameter (|browser|, |tab_strip|, |contents|, & |tab_index|) may
// be NULL and will not be set within the function.
bool GetTabById(int tab_id, Profile* profile, bool incognito_enabled,
                Browser** browser,
                TabStripModel** tab_strip,
                TabContentsWrapper** contents,
                int* tab_index);
}

// Windows
class GetWindowFunction : public SyncExtensionFunction {
  virtual ~GetWindowFunction() {}
  virtual bool RunImpl();
  DECLARE_EXTENSION_FUNCTION_NAME("windows.get")
};
class GetCurrentWindowFunction : public SyncExtensionFunction {
  virtual ~GetCurrentWindowFunction() {}
  virtual bool RunImpl();
  DECLARE_EXTENSION_FUNCTION_NAME("windows.getCurrent")
};
class GetLastFocusedWindowFunction : public SyncExtensionFunction {
  virtual ~GetLastFocusedWindowFunction() {}
  virtual bool RunImpl();
  DECLARE_EXTENSION_FUNCTION_NAME("windows.getLastFocused")
};
class GetAllWindowsFunction : public SyncExtensionFunction {
  virtual ~GetAllWindowsFunction() {}
  virtual bool RunImpl();
  DECLARE_EXTENSION_FUNCTION_NAME("windows.getAll")
};
class CreateWindowFunction : public SyncExtensionFunction {
  virtual ~CreateWindowFunction() {}
  virtual bool RunImpl();
  DECLARE_EXTENSION_FUNCTION_NAME("windows.create")
};
class UpdateWindowFunction : public SyncExtensionFunction {
  virtual ~UpdateWindowFunction() {}
  virtual bool RunImpl();
  DECLARE_EXTENSION_FUNCTION_NAME("windows.update")
};
class RemoveWindowFunction : public SyncExtensionFunction {
  virtual ~RemoveWindowFunction() {}
  virtual bool RunImpl();
  DECLARE_EXTENSION_FUNCTION_NAME("windows.remove")
};

// Tabs
class GetTabFunction : public SyncExtensionFunction {
  virtual ~GetTabFunction() {}
  virtual bool RunImpl();
  DECLARE_EXTENSION_FUNCTION_NAME("tabs.get")
};
class GetCurrentTabFunction : public SyncExtensionFunction {
  virtual ~GetCurrentTabFunction() {}
  virtual bool RunImpl();
  DECLARE_EXTENSION_FUNCTION_NAME("tabs.getCurrent")
};
class GetSelectedTabFunction : public SyncExtensionFunction {
  virtual ~GetSelectedTabFunction() {}
  virtual bool RunImpl();
  DECLARE_EXTENSION_FUNCTION_NAME("tabs.getSelected")
};
class GetAllTabsInWindowFunction : public SyncExtensionFunction {
  virtual ~GetAllTabsInWindowFunction() {}
  virtual bool RunImpl();
  DECLARE_EXTENSION_FUNCTION_NAME("tabs.getAllInWindow")
};
class CreateTabFunction : public SyncExtensionFunction {
  virtual ~CreateTabFunction() {}
  virtual bool RunImpl();
  DECLARE_EXTENSION_FUNCTION_NAME("tabs.create")
};
class UpdateTabFunction : public AsyncExtensionFunction,
                          public TabContentsObserver {
 public:
  UpdateTabFunction();
 private:
  virtual ~UpdateTabFunction() {}
  virtual bool RunImpl();
  virtual bool OnMessageReceived(const IPC::Message& message);
  void OnExecuteCodeFinished(int request_id,
                             bool success,
                             const std::string& error);
  DECLARE_EXTENSION_FUNCTION_NAME("tabs.update")
};
class MoveTabFunction : public SyncExtensionFunction {
  virtual ~MoveTabFunction() {}
  virtual bool RunImpl();
  DECLARE_EXTENSION_FUNCTION_NAME("tabs.move")
};
class RemoveTabFunction : public SyncExtensionFunction {
  virtual ~RemoveTabFunction() {}
  virtual bool RunImpl();
  DECLARE_EXTENSION_FUNCTION_NAME("tabs.remove")
};
class DetectTabLanguageFunction : public AsyncExtensionFunction,
                                  public NotificationObserver {
 private:
  virtual ~DetectTabLanguageFunction() {}
  virtual bool RunImpl();

  virtual void Observe(int type,
                       const NotificationSource& source,
                       const NotificationDetails& details);
  void GotLanguage(const std::string& language);
  NotificationRegistrar registrar_;
  DECLARE_EXTENSION_FUNCTION_NAME("tabs.detectLanguage")
};
class CaptureVisibleTabFunction : public AsyncExtensionFunction,
                                  public NotificationObserver {
 private:
  enum ImageFormat {
    FORMAT_JPEG,
    FORMAT_PNG
  };

  // The default quality setting used when encoding jpegs.
  static const int kDefaultQuality;

  virtual ~CaptureVisibleTabFunction() {}
  virtual bool RunImpl();
  virtual bool CaptureSnapshotFromBackingStore(BackingStore* backing_store);
  virtual void Observe(int type,
                       const NotificationSource& source,
                       const NotificationDetails& details);
  virtual void SendResultFromBitmap(const SkBitmap& screen_capture);

  NotificationRegistrar registrar_;

  // The format (JPEG vs PNG) of the resulting image.  Set in RunImpl().
  ImageFormat image_format_;

  // Quality setting to use when encoding jpegs.  Set in RunImpl().
  int image_quality_;

  DECLARE_EXTENSION_FUNCTION_NAME("tabs.captureVisibleTab")
};

#endif  // CHROME_BROWSER_EXTENSIONS_EXTENSION_TABS_MODULE_H__
