// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_COMMON_FAVICON_FAVICON_URL_PARSER_H_
#define CHROME_COMMON_FAVICON_FAVICON_URL_PARSER_H_

#include <string>

#include "ui/base/layout.h"

namespace chrome {

struct ParsedFaviconPath {
  // Whether the URL has the "iconurl" parameter.
  bool is_icon_url;

  // The URL from which the favicon is being requested.
  std::string url;

  // The size of the requested favicon in dip.
  int size_in_dip;

  // The scale factor of the requested favicon.
  ui::ScaleFactor scale_factor;

  // The index of the first character (relative to the path) where the the URL
  // from which the favicon is being requested is located.
  size_t path_index;
};

// Parses |path|, which should be in the format described at the top of the
// file "chrome/browser/ui/webui/favicon_source.h". |icon_types| indicates
// which icon types are supported. Returns true if |path| could be parsed.
// The result of the parsing will be stored in a ParsedFaviconPath struct.
bool ParseFaviconPath(const std::string& path,
                      int icon_types,
                      ParsedFaviconPath* parsed);

}  // namespace chrome

#endif  // CHROME_COMMON_FAVICON_FAVICON_URL_PARSER_H_
