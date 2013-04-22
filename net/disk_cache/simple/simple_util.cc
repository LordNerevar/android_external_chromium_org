// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/disk_cache/simple/simple_util.h"

#include <limits>

#include "base/format_macros.h"
#include "base/logging.h"
#include "base/sha1.h"
#include "base/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "net/disk_cache/simple/simple_entry_format.h"

namespace {

// Size of the uint64 hash_key number in Hex format in a string.
const size_t kEntryHashKeyAsHexStringSize = 2 * sizeof(uint64);

}  // namespace

namespace disk_cache {

namespace simple_util {

std::string ConvertEntryHashKeyToHexString(uint64 hash_key) {
  const std::string hash_key_str = base::StringPrintf("%016" PRIx64, hash_key);
  DCHECK_EQ(kEntryHashKeyAsHexStringSize, hash_key_str.size());
  return hash_key_str;
}

std::string GetEntryHashKeyAsHexString(const std::string& key) {
  std::string hash_key_str =
      ConvertEntryHashKeyToHexString(GetEntryHashKey(key));
  DCHECK_EQ(kEntryHashKeyAsHexStringSize, hash_key_str.size());
  return hash_key_str;
}

bool GetEntryHashKeyFromHexString(const std::string& hash_key,
                                  uint64* hash_key_out) {
  if (hash_key.size() != kEntryHashKeyAsHexStringSize) {
    return false;
  }
  return base::HexStringToUInt64(hash_key, hash_key_out);
}

uint64 GetEntryHashKey(const std::string& key) {
  const std::string sha_hash = base::SHA1HashString(key);
  uint64 hash_key = 0;
  sha_hash.copy(reinterpret_cast<char*>(&hash_key), sizeof(hash_key));
  return hash_key;
}

std::string GetFilenameFromHexStringAndIndex(const std::string& hex_key,
                                             int index) {
  return hex_key + base::StringPrintf("_%1d", index);
}

std::string GetFilenameFromKeyAndIndex(const std::string& key, int index) {
  return GetEntryHashKeyAsHexString(key) + base::StringPrintf("_%1d", index);
}

int32 GetDataSizeFromKeyAndFileSize(const std::string& key, int64 file_size) {
  int64 data_size = file_size - key.size() - sizeof(SimpleFileHeader) -
                    sizeof(SimpleFileEOF);
  DCHECK_GE(implicit_cast<int64>(std::numeric_limits<int32>::max()), data_size);
  return data_size;
}

int64 GetFileSizeFromKeyAndDataSize(const std::string& key, int32 data_size) {
  return data_size + key.size() + sizeof(SimpleFileHeader) +
      sizeof(SimpleFileEOF);
}

int64 GetFileOffsetFromKeyAndDataOffset(const std::string& key,
                                        int data_offset) {
  const int64 headers_size = sizeof(disk_cache::SimpleFileHeader) + key.size();
  return headers_size + data_offset;
}

}  // namespace simple_backend

}  // namespace disk_cache
