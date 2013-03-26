# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      'target_name': 'aura',
      'type': '<(component)',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:base_i18n',
        '../../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../../skia/skia.gyp:skia',
        '../compositor/compositor.gyp:compositor',
        '../ui.gyp:ui',
        '../ui.gyp:ui_resources',
      ],
      'defines': [
        'AURA_IMPLEMENTATION',
      ],
      'sources': [
        'client/activation_change_observer.h',
        'client/activation_change_observer.cc',
        'client/activation_client.cc',
        'client/activation_client.h',
        'client/activation_delegate.cc',
        'client/activation_delegate.h',
        'client/animation_host.cc',
        'client/animation_host.h',
        'client/aura_constants.cc',
        'client/aura_constants.h',
        'client/capture_client.cc',
        'client/capture_client.h',
        'client/capture_delegate.h',
        'client/cursor_client.cc',
        'client/cursor_client.h',
        'client/default_capture_client.cc',
        'client/default_capture_client.h',
        'client/dispatcher_client.cc',
        'client/dispatcher_client.h',
        'client/drag_drop_client.cc',
        'client/drag_drop_client.h',
        'client/drag_drop_delegate.cc',
        'client/drag_drop_delegate.h',
        'client/event_client.cc',
        'client/event_client.h',
        'client/focus_change_observer.cc',
        'client/focus_change_observer.h',
        'client/focus_client.cc',
        'client/focus_client.h',
        'client/screen_position_client.cc',
        'client/screen_position_client.h',
        'client/stacking_client.cc',
        'client/stacking_client.h',
        'client/tooltip_client.cc',
        'client/tooltip_client.h',
        'client/user_action_client.cc',
        'client/user_action_client.h',
        'client/visibility_client.cc',
        'client/visibility_client.h',
        'client/window_move_client.cc',
        'client/window_move_client.h',
        'client/window_types.h',
        'device_list_updater_aurax11.cc',
        'device_list_updater_aurax11.h',
        'dispatcher_win.cc',
        'env.cc',
        'env.h',
        'env_observer.h',
        'focus_manager.cc',
        'focus_manager.h',
        'layout_manager.cc',
        'layout_manager.h',
        'remote_root_window_host_win.cc',
        'remote_root_window_host_win.h',
        'root_window_host.h',
        'root_window_host_delegate.h',
        'root_window_host_mac.h',
        'root_window_host_mac.mm',
        'root_window_host_win.cc',
        'root_window_host_win.h',
        'root_window_host_x11.cc',
        'root_window_host_x11.h',
        'root_window_mac.h',
        'root_window_mac.mm',
        'root_window_view_mac.h',
        'root_window_view_mac.mm',
        'root_window.cc',
        'root_window.h',
        'window.cc',
        'window.h',
        'window_delegate.h',
        'window_destruction_observer.cc',
        'window_destruction_observer.h',
        'window_observer.h',
        'window_tracker.cc',
        'window_tracker.h',
      ],
      'conditions': [
        ['OS=="mac"', {
          'sources/': [
            ['exclude', 'client/dispatcher_client.cc'],
            ['exclude', 'client/dispatcher_client.h'],
          ],
        }],
        ['use_x11==1', {
          'link_settings': {
            'libraries': [
              '-lX11',
              '-lXi',
              '-lXfixes',
              '-lXrandr',
            ],
          },
        }],
        ['OS=="win"', {
          'dependencies': [
            '../metro_viewer/metro_viewer.gyp:metro_viewer',
            '../../ipc/ipc.gyp:ipc',         
          ],
        }],
      ],
    },
    {
      'target_name': 'aura_test_support',
      'type': 'static_library',
      'dependencies': [
        '../../skia/skia.gyp:skia',
        '../../testing/gtest.gyp:gtest',
        '../ui.gyp:ui',
        '../ui.gyp:ui_test_support',
        'aura',
        'aura_test_support_pak',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'test/aura_test_base.cc',
        'test/aura_test_base.h',
        'test/aura_test_helper.cc',
        'test/aura_test_helper.h',
        'test/event_generator.cc',
        'test/event_generator.h',
        'test/test_activation_client.cc',
        'test/test_activation_client.h',
        'test/test_aura_initializer.cc',
        'test/test_aura_initializer.h',
        'test/test_event_handler.cc',
        'test/test_event_handler.h',
        'test/test_screen.cc',
        'test/test_screen.h',
        'test/test_stacking_client.cc',
        'test/test_stacking_client.h',
        'test/test_windows.cc',
        'test/test_windows.h',
        'test/test_window_delegate.cc',
        'test/test_window_delegate.h',
        'test/window_test_api.cc',
        'test/window_test_api.h',
      ],
      # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
      'msvs_disabled_warnings': [ 4267, ],
    },
    {
      # We build a minimal set of resources required for aura_test_support.
      'target_name': 'aura_test_support_pak',
      'type': 'none',
      'dependencies': [
        '<(DEPTH)/ui/ui.gyp:ui_resources',
      ],
      'variables': {
        'repack_path': '<(DEPTH)/tools/grit/grit/format/repack.py',
      },
      'actions': [
        {
          'action_name': 'repack_aura_test_support_pack',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources_100_percent.pak',
            ],
          },
          'inputs': [
            '<(repack_path)',
            '<@(pak_inputs)',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/aura_test_support_resources.pak',
          ],
          'action': ['python', '<(repack_path)', '<@(_outputs)',
                     '<@(pak_inputs)'],
        },
      ],
    },
    {
      'target_name': 'aura_demo',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:base_i18n',
        '../../skia/skia.gyp:skia',
        '../../third_party/icu/icu.gyp:icui18n',
        '../../third_party/icu/icu.gyp:icuuc',
        '../compositor/compositor.gyp:compositor',
        '../compositor/compositor.gyp:compositor_test_support',
        '../ui.gyp:ui',
        '../ui.gyp:ui_resources',
        '../../ipc/ipc.gyp:ipc',
        'aura',
        'aura_test_support',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'demo/demo_main.cc',
      ],
    },
    {
      'target_name': 'aura_bench',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:base_i18n',
        '../../skia/skia.gyp:skia',
        '../../third_party/icu/icu.gyp:icui18n',
        '../../third_party/icu/icu.gyp:icuuc',
        '../compositor/compositor.gyp:compositor',
        '../compositor/compositor.gyp:compositor_test_support',
        '../ui.gyp:ui',
        '../ui.gyp:ui_resources',
        'aura',
        'aura_test_support',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'bench/bench_main.cc',
      ],
    },
    {
      'target_name': 'aura_unittests',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:test_support_base',
        '../../chrome/chrome_resources.gyp:packed_resources',
        '../../skia/skia.gyp:skia',
        '../../testing/gtest.gyp:gtest',
        '../compositor/compositor.gyp:compositor_test_support',
        '../compositor/compositor.gyp:compositor',
        '../gl/gl.gyp:gl',
        '../ui.gyp:ui',
        '../ui.gyp:ui_resources',
        '../ui.gyp:ui_test_support',
        'aura_test_support',
        'aura',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'gestures/gesture_recognizer_unittest.cc',
        'test/run_all_unittests.cc',
        'test/test_suite.cc',
        'test/test_suite.h',
        'root_window_unittest.cc',
        'window_unittest.cc',
      ],
      'conditions': [
        # osmesa GL implementation is used on linux.
        ['OS=="linux"', {
          'dependencies': [
            '<(DEPTH)/third_party/mesa/mesa.gyp:osmesa',
          ],
        }],
      ],
    },
  ],
}
