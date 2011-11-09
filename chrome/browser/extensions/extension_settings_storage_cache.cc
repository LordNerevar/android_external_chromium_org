// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/extension_settings_storage_cache.h"

ExtensionSettingsStorageCache::ExtensionSettingsStorageCache(
    ExtensionSettingsStorage* delegate) : delegate_(delegate) {}

ExtensionSettingsStorageCache::~ExtensionSettingsStorageCache() {}

ExtensionSettingsStorage::ReadResult ExtensionSettingsStorageCache::Get(
    const std::string& key) {
  Value *value;
  if (GetFromCache(key, &value)) {
    DictionaryValue* settings = new DictionaryValue();
    settings->SetWithoutPathExpansion(key, value);
    return ReadResult(settings);
  }

  ReadResult result = delegate_->Get(key);
  if (result.HasError()) {
    return result;
  }

  cache_.MergeDictionary(&result.settings());
  return result;
}

ExtensionSettingsStorage::ReadResult ExtensionSettingsStorageCache::Get(
    const std::vector<std::string>& keys) {
  scoped_ptr<DictionaryValue> from_cache(new DictionaryValue());
  std::vector<std::string> missing_keys;

  for (std::vector<std::string>::const_iterator it = keys.begin();
      it != keys.end(); ++it) {
    Value *value;
    if (GetFromCache(*it, &value)) {
      from_cache->SetWithoutPathExpansion(*it, value);
    } else {
      missing_keys.push_back(*it);
    }
  }

  if (missing_keys.empty()) {
    return ReadResult(from_cache.release());
  }

  ReadResult result = delegate_->Get(missing_keys);
  if (result.HasError()) {
    return result;
  }

  cache_.MergeDictionary(&result.settings());
  from_cache->MergeDictionary(&result.settings());
  return ReadResult(from_cache.release());
}

ExtensionSettingsStorage::ReadResult ExtensionSettingsStorageCache::Get() {
  ReadResult result = delegate_->Get();
  if (!result.HasError()) {
    cache_.MergeDictionary(&result.settings());
  }
  return result;
}

ExtensionSettingsStorage::WriteResult ExtensionSettingsStorageCache::Set(
    const std::string& key, const Value& value) {
  WriteResult result = delegate_->Set(key, value);
  if (!result.HasError()) {
    cache_.SetWithoutPathExpansion(key, value.DeepCopy());
  }
  return result;
}

ExtensionSettingsStorage::WriteResult ExtensionSettingsStorageCache::Set(
    const DictionaryValue& settings) {
  WriteResult result = delegate_->Set(settings);
  if (result.HasError()) {
    return result;
  }

  for (ExtensionSettingChangeList::const_iterator it = result.changes().begin();
      it != result.changes().end(); ++it) {
    DCHECK(it->new_value());
    cache_.SetWithoutPathExpansion(it->key(), it->new_value()->DeepCopy());
  }

  return result;
}

ExtensionSettingsStorage::WriteResult ExtensionSettingsStorageCache::Remove(
    const std::string& key) {
  WriteResult result = delegate_->Remove(key);
  if (!result.HasError()) {
    cache_.RemoveWithoutPathExpansion(key, NULL);
  }
  return result;
}

ExtensionSettingsStorage::WriteResult ExtensionSettingsStorageCache::Remove(
    const std::vector<std::string>& keys) {
  WriteResult result = delegate_->Remove(keys);
  if (result.HasError()) {
    return result;
  }

  for (ExtensionSettingChangeList::const_iterator it = result.changes().begin();
      it != result.changes().end(); ++it) {
    cache_.RemoveWithoutPathExpansion(it->key(), NULL);
  }

  return result;
}

ExtensionSettingsStorage::WriteResult ExtensionSettingsStorageCache::Clear() {
  WriteResult result = delegate_->Clear();
  if (!result.HasError()) {
    cache_.Clear();
  }
  return result;
}

bool ExtensionSettingsStorageCache::GetFromCache(
    const std::string& key, Value** value) {
  Value* cached_value;
  if (!cache_.GetWithoutPathExpansion(key, &cached_value)) {
    return false;
  }

  *value = cached_value->DeepCopy();
  return true;
}
