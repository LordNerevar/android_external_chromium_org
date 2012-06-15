# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'includes': [
    '../native_client/build/untrusted.gypi',
    'ipc.gypi',
  ],
  'conditions': [
    ['disable_nacl_untrusted==0', {
      'targets': [
        {
          'target_name': 'ipc_untrusted',
          'type': 'none',
          'variables': {
            'ipc_target': 1,
            'nacl_untrusted_build': 1,
            'nlib_target': 'libipc_untrusted.a',
            'build_glibc': 0,
            'build_newlib': 1,
          },
          'dependencies': [
            '<(DEPTH)/native_client/tools.gyp:prep_toolchain',
            '<(DEPTH)/native_client/src/untrusted/pthread/pthread.gyp:pthread_lib',
            '<(DEPTH)/native_client/src/untrusted/nacl/nacl.gyp:nacl_lib_newlib',
            '../base/base_untrusted.gyp:base_untrusted',
          ],
        },
      ],
    }],
  ],
}
