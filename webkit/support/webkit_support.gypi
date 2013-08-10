# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      'target_name': 'webkit_support',
      'type': 'static_library',
      'variables': { 'enable_wexit_time_destructors': 1, },
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/base.gyp:base_i18n',
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '<(DEPTH)/cc/cc.gyp:cc',
        '<(DEPTH)/media/media.gyp:media',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/ui/gl/gl.gyp:gl',
        '<(DEPTH)/ui/ui.gyp:shell_dialogs',
        '<(DEPTH)/ui/ui.gyp:ui',
        '<(DEPTH)/webkit/common/gpu/webkit_gpu.gyp:webkit_gpu',
        '<(DEPTH)/webkit/common/user_agent/webkit_user_agent.gyp:user_agent',
        '<(DEPTH)/webkit/common/webkit_common.gyp:webkit_common',
        '<(DEPTH)/webkit/renderer/compositor_bindings/compositor_bindings.gyp:webkit_compositor_bindings',
        '<(DEPTH)/webkit/renderer/compositor_bindings/compositor_bindings.gyp:webkit_compositor_support',
        '<(DEPTH)/webkit/renderer/webkit_renderer.gyp:webkit_renderer',
        '<(DEPTH)/webkit/storage_browser.gyp:webkit_storage_browser',
        '<(DEPTH)/webkit/storage_common.gyp:webkit_storage_common',
        '<(DEPTH)/webkit/storage_renderer.gyp:webkit_storage_renderer',
        'glue',
        'glue_child',
        'webkit_support_common',
      ],
      'include_dirs': [
        '<(SHARED_INTERMEDIATE_DIR)/webkit', # for a header generated by grit
      ],
      'defines': [
        # Technically not a unit test but require functions available only to
        # unit tests.
        'UNIT_TEST'
      ],
      'sources': [
        'drt_application_mac.h',
        'drt_application_mac.mm',
        'gc_extension.cc',
        'gc_extension.h',
        'platform_support.h',
        'platform_support_android.cc',
        'platform_support_linux.cc',
        'platform_support_mac.mm',
        'platform_support_win.cc',
        'test_webkit_platform_support.cc',
        'test_webkit_platform_support.h',
        'webkit_support.cc',
        'webkit_support.h',
        'webkit_support_glue.cc',
        'weburl_loader_mock.cc',
        'weburl_loader_mock.h',
        'weburl_loader_mock_factory.cc',
        'weburl_loader_mock_factory.h',
        'web_gesture_curve_mock.cc',
        'web_gesture_curve_mock.h',
        'web_layer_tree_view_impl_for_testing.cc',
        'web_layer_tree_view_impl_for_testing.h',
      ],
      'conditions': [
        ['OS=="mac"', {
          'copies': [{
            'destination': '<(SHARED_INTERMEDIATE_DIR)/webkit',
            'files': [
              '../tools/test_shell/resources/missingImage.png',
              '../tools/test_shell/resources/textAreaResizeCorner.png',
            ],
          }],
        },{ # OS!="mac"
          'copies': [{
            'destination': '<(PRODUCT_DIR)/DumpRenderTree_resources',
            'files': [
              '../tools/test_shell/resources/missingImage.gif',
              '../tools/test_shell/resources/textAreaResizeCorner.png',
            ],
          }],
        }],
      ],
      # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
      'msvs_disabled_warnings': [ 4267, ],
    },

    {
      'target_name': 'webkit_support_common',
      'type': 'static_library',
      'variables': { 'enable_wexit_time_destructors': 1, },
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/third_party/zlib/zlib.gyp:zlib',
        '<(DEPTH)/ui/ui.gyp:ui',
        '<(DEPTH)/webkit/common/user_agent/webkit_user_agent.gyp:user_agent',
        '<(DEPTH)/webkit/renderer/webkit_renderer.gyp:webkit_renderer',
        'glue',
      ],
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
      ],
      'sources': [
        '<(DEPTH)/webkit/support/mac/DumpRenderTreePasteboard.h',
        '<(DEPTH)/webkit/support/mac/DumpRenderTreePasteboard.m',
        '<(DEPTH)/webkit/support/mock_webclipboard_impl.cc',
        '<(DEPTH)/webkit/support/mock_webclipboard_impl.h',
      ],
      # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
      'msvs_disabled_warnings': [ 4267, ],
    },
  ],
}
