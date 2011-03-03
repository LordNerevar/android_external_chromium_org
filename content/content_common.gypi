# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'content_common',
      'type': '<(library)',
      'dependencies': [
        '../ipc/ipc.gyp:ipc',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        "common/content_switches.cc",
        "common/content_switches.h",
        'common/message_router.cc',
        'common/message_router.h',
        "common/p2p_messages.cc",
        "common/p2p_messages.h",
        "common/p2p_sockets.cc",
        "common/p2p_sockets.h",
      ],
      'conditions': [
        ['OS=="win"', {
          'msvs_guid': '062E9260-304A-4657-A74C-0D3AA1A0A0A4',
        }],
      ],
    },
  ],
}
