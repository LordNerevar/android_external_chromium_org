# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'sqlite',
      'type': 'none',
      'direct_dependent_settings': {
        'cflags': [
          '<!@(pkg-config --cflags sqlite3)',
        ],
        'defines': [
          'USE_SYSTEM_SQLITE',
        ],
      },
      'link_settings': {
        'ldflags': [
          '<!@(pkg-config --libs-only-L --libs-only-other sqlite3)',
        ],
        'libraries': [
          '<!@(pkg-config --libs-only-l sqlite3)',
        ],
      },
    },
  ],
}
