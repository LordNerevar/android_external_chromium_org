#!/usr/bin/env python

# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script updates the address_input_strings.grdp file based on the strings
# in libaddressinput.

import os
import sys

HEADER = """<!--

DO NOT MODIFY.

This file is generated by "gclient runhooks" from
src/third_party/libaddressinput/src/cpp/res/messages.grdp. Submit modifications
to the upstream library at https://libaddressinput.googlecode.com/.

-->
"""

script_dir = os.path.dirname(os.path.realpath(__file__))
from_file = os.path.abspath(os.path.join(
    script_dir, os.pardir, os.pardir, 'src', 'cpp', 'res', 'messages.grdp'))

if not os.path.isfile(from_file):
  # Android and iOS do not use src/third_party/libaddressinput/src/. Gclient
  # cannot filter out hooks based on OS or (when using git) based on file name
  # patterns.
  print('No libaddressinput for this target OS.')
  sys.exit()

to_file = os.path.abspath(os.path.join(
    script_dir, os.pardir, os.pardir, os.pardir, os.pardir, 'chrome', 'app',
   'address_input_strings.grdp'))

with open(from_file, 'r') as source:
  with open(to_file, 'w') as destination:
    destination.write(source.readline())  # XML declaration.
    destination.write(HEADER)
    destination.write(source.read())
