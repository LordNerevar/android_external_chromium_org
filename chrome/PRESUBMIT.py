# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Makes sure that the chrome/ code is cpplint clean."""

INCLUDE_CPP_FILES_ONLY = (
  r'.*\.cc$', r'.*\.h$'
)

EXCLUDE = (
  # Objective C confuses everything.
  r'.*cocoa.*',
  r'.*_mac\.(cc|h)$',
  r'.*_mac_.*',
  # All the messages files do weird multiple include trickery
  r'.*_messages_internal\.h$',
  r'render_messages.h$',
  # Autogenerated window resources files are off limits
  r'.*resource.h$',
  # GTK macros in C-ish header code cause false positives
  r'gtk_.*\.h$',
  # Header trickery
  r'.*-inl\.h$',
  # Templates
  r'sigslotrepeater\.h$',
  # GCC attribute trickery
  r'sel_main\.cc$',
  # Mozilla code
  r'mork_reader\.h$',
  r'mork_reader\.cc$',
  r'nss_decryptor_linux\.cc$',
  # Has safe printf usage that cpplint complains about
  r'safe_browsing_util\.cc$',
  # Too much math on one line?
  r'bloom_filter\.cc$',
  # Bogus ifdef tricks
  r'renderer_webkitplatformsupport_impl\.cc$',
  # Lines > 100 chars
  r'gcapi\.cc$',
)

def CheckChangeOnUpload(input_api, output_api):
  results = []
  black_list = input_api.DEFAULT_BLACK_LIST + EXCLUDE
  sources = lambda x: input_api.FilterSourceFile(
    x, white_list=INCLUDE_CPP_FILES_ONLY, black_list=black_list)
  results.extend(input_api.canned_checks.CheckChangeLintsClean(
      input_api, output_api, sources))
  return results
