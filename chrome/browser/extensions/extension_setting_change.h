// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_EXTENSION_SETTING_CHANGE_H_
#define CHROME_BROWSER_EXTENSIONS_EXTENSION_SETTING_CHANGE_H_
#pragma once

#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/values.h"

class ExtensionSettingChange;
typedef std::vector<ExtensionSettingChange> ExtensionSettingChangeList;

// A change to a setting.  Safe/efficient to copy.
class ExtensionSettingChange {
 public:
  // Converts an ExtensionSettingChangeList into JSON of the form:
  // [ { "key": "foo", "oldValue": "bar", "newValue": "baz" } ]
  static std::string GetEventJson(const ExtensionSettingChangeList& changes);

  // Ownership of |old_value| and |new_value| taken.
  ExtensionSettingChange(
      const std::string& key, base::Value* old_value, base::Value* new_value);

  ~ExtensionSettingChange();

  // Gets the key of the setting which changed.
  const std::string& key() const;

  // Gets the value of the setting before the change, or NULL if there was no
  // old value.
  const base::Value* old_value() const;

  // Gets the value of the setting after the change, or NULL if there is no new
  // value.
  const base::Value* new_value() const;

 private:
  class Inner : public base::RefCountedThreadSafe<Inner> {
   public:
    Inner(
        const std::string& key, base::Value* old_value, base::Value* new_value);

    const std::string key_;
    const scoped_ptr<base::Value> old_value_;
    const scoped_ptr<base::Value> new_value_;

   private:
    friend class base::RefCountedThreadSafe<Inner>;
    virtual ~Inner();
  };

  scoped_refptr<Inner> inner_;
};

#endif  // CHROME_BROWSER_EXTENSIONS_EXTENSION_SETTING_CHANGE_H_
