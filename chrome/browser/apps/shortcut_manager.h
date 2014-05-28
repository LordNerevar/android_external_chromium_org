// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_APPS_SHORTCUT_MANAGER_H_
#define CHROME_BROWSER_APPS_SHORTCUT_MANAGER_H_

#include "base/scoped_observer.h"
#include "chrome/browser/profiles/profile_info_cache_observer.h"
#include "components/keyed_service/core/keyed_service.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "extensions/browser/extension_registry_observer.h"
#include "extensions/common/extension.h"

class PrefService;
class Profile;

namespace extensions {
class ExtensionRegistry;
}

namespace user_prefs {
class PrefRegistrySyncable;
}

// This class manages the installation of shortcuts for platform apps.
class AppShortcutManager : public KeyedService,
                           public content::NotificationObserver,
                           public extensions::ExtensionRegistryObserver,
                           public ProfileInfoCacheObserver {
 public:
  static void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

  explicit AppShortcutManager(Profile* profile);

  virtual ~AppShortcutManager();

  // Checks if kShortcutsEnabled is set in prefs. If not, this sets it and
  // creates shortcuts for all apps.
  void OnceOffCreateShortcuts();

  // content::NotificationObserver.
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  // extensions::ExtensionRegistryObserver.
  virtual void OnExtensionWillBeInstalled(
      content::BrowserContext* browser_context,
      const extensions::Extension* extension,
      bool is_update,
      bool from_ephemeral,
      const std::string& old_name) OVERRIDE;
  virtual void OnExtensionUninstalled(
      content::BrowserContext* browser_context,
      const extensions::Extension* extension) OVERRIDE;

  // ProfileInfoCacheObserver.
  virtual void OnProfileWillBeRemoved(
      const base::FilePath& profile_path) OVERRIDE;

 private:
  void DeleteApplicationShortcuts(const extensions::Extension* extension);

  content::NotificationRegistrar registrar_;
  Profile* profile_;
  bool is_profile_info_cache_observer_;
  PrefService* prefs_;

  ScopedObserver<extensions::ExtensionRegistry,
                 extensions::ExtensionRegistryObserver>
      extension_registry_observer_;

  DISALLOW_COPY_AND_ASSIGN(AppShortcutManager);
};

#endif  // CHROME_BROWSER_APPS_SHORTCUT_MANAGER_H_
