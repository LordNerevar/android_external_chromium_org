# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Presubmit script for changes affecting chrome/

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details about the presubmit API built into gcl.
"""

import re

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
  # Bogus ifdef tricks
  r'renderer_webkitplatformsupport_impl\.cc$',
  # Lines > 100 chars
  r'gcapi\.cc$',
)

def _CheckChangeLintsClean(input_api, output_api):
  """Makes sure that the chrome/ code is cpplint clean."""
  black_list = input_api.DEFAULT_BLACK_LIST + EXCLUDE
  sources = lambda x: input_api.FilterSourceFile(
    x, white_list=INCLUDE_CPP_FILES_ONLY, black_list=black_list)
  return input_api.canned_checks.CheckChangeLintsClean(
      input_api, output_api, sources)

def _CheckNoContentUnitTestsInChrome(input_api, output_api):
  """Makes sure that no unit tests from content/ are included in unit_tests."""
  problems = []
  for f in input_api.AffectedFiles():
    if not f.LocalPath().endswith('chrome_tests.gypi'):
      continue

    for line_num, line in f.ChangedContents():
      m = re.search(r"'(.*\/content\/.*unittest.*)'", line)
      if m:
        problems.append(m.group(1))

  if not problems:
    return []
  return [output_api.PresubmitPromptWarning(
      'Unit tests located in content/ should be added to the ' +
      'content_tests.gypi:content_unittests target.',
      items=problems)]

def _CommonChecks(input_api, output_api):
  """Checks common to both upload and commit."""
  results = []
  results.extend(_CheckNoContentUnitTestsInChrome(input_api, output_api))
  return results

def CheckChangeOnUpload(input_api, output_api):
  results = []
  results.extend(_CommonChecks(input_api, output_api))
  results.extend(_CheckChangeLintsClean(input_api, output_api))
  return results

def CheckChangeOnCommit(input_api, output_api):
  results = []
  results.extend(_CommonChecks(input_api, output_api))
  return results
