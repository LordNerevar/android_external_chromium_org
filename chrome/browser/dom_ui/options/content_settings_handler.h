// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_DOM_UI_OPTIONS_CONTENT_SETTINGS_HANDLER_H_
#define CHROME_BROWSER_DOM_UI_OPTIONS_CONTENT_SETTINGS_HANDLER_H_
#pragma once

#include "chrome/browser/dom_ui/options/options_ui.h"
#include "chrome/common/content_settings_types.h"
#include "chrome/common/notification_observer.h"
#include "chrome/common/notification_registrar.h"

class HostContentSettingsMap;

class ContentSettingsHandler : public OptionsPageUIHandler {
 public:
  ContentSettingsHandler();
  virtual ~ContentSettingsHandler();

  // OptionsUIHandler implementation.
  virtual void GetLocalizedValues(DictionaryValue* localized_strings);

  virtual void Initialize();

  virtual void RegisterMessages();

  // NotificationObserver implementation.
  virtual void Observe(NotificationType type,
                       const NotificationSource& source,
                       const NotificationDetails& details);

  // Gets a string identifier for the group name, for use in HTML.
  static std::string ContentSettingsTypeToGroupName(ContentSettingsType type);

 private:
  // Functions that call into the page -----------------------------------------

  // Updates the page with the default settings (allow, ask, block, etc.)
  void UpdateSettingDefaultFromModel(ContentSettingsType type);

  // Clobbers and rebuilds the specific content setting type exceptions table.
  void UpdateExceptionsViewFromModel(ContentSettingsType type);
  // Clobbers and rebuilds all the exceptions tables in the page.
  void UpdateAllExceptionsViewsFromModel();
  // Clobbers and rebuilds just the geolocation exception table.
  void UpdateGeolocationExceptionsView();
  // Clobbers and rebuilds just the desktop notification exception table.
  void UpdateNotificationExceptionsView();
  // Clobbers and rebuilds an exception table that's managed by the
  // host content settings map.
  void UpdateExceptionsViewFromHostContentSettingsMap(ContentSettingsType type);

  // Callbacks used by the page ------------------------------------------------

  // Sets the default value for a specific content type. |args| includes the
  // content type and a string describing the new default the user has
  // chosen.
  void SetContentFilter(const ListValue* args);

  // Removes the given rows from the table. The first entry in |args| is the
  // content type, and the rest of the arguments describe individual exceptions
  // to be removed.
  void RemoveExceptions(const ListValue* args);

  // Changes the value of an exception. Called after the user is done editing an
  // exception.
  void SetException(const ListValue* args);

  // Called to decide whether a given pattern is valid, or if it should be
  // rejected. Called while the user is editing an exception pattern.
  void CheckExceptionPatternValidity(const ListValue* args);

  // Show the about:plugins page in a new tab.
  void OpenPluginsTab(const ListValue* args);

  // Sets the global 3rd party cookies pref.
  void SetAllowThirdPartyCookies(const ListValue* args);

  // Utility functions ---------------------------------------------------------

  // Gets the HostContentSettingsMap for the normal profile.
  HostContentSettingsMap* GetContentSettingsMap();

  // Gets the HostContentSettingsMap for the incognito profile, or NULL if there
  // is no active incognito session.
  HostContentSettingsMap* GetOTRContentSettingsMap();

  // Gets the default setting in string form.
  std::string GetSettingDefaultFromModel(ContentSettingsType type);

  // Member variables ---------------------------------------------------------

  NotificationRegistrar notification_registrar_;

  DISALLOW_COPY_AND_ASSIGN(ContentSettingsHandler);
};

#endif  // CHROME_BROWSER_DOM_UI_OPTIONS_CONTENT_SETTINGS_HANDLER_H_
