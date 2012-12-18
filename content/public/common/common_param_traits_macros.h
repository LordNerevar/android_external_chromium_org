// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Singly or Multiply-included shared traits file depending on circumstances.
// This allows the use of IPC serialization macros in more than one IPC message
// file.
#ifndef CONTENT_PUBLIC_COMMON_COMMON_PARAM_TRAITS_MACROS_H_
#define CONTENT_PUBLIC_COMMON_COMMON_PARAM_TRAITS_MACROS_H_

#include "content/public/common/console_message_level.h"
#include "content/public/common/page_transition_types.h"
#include "content/public/common/password_form.h"
#include "content/public/common/security_style.h"
#include "content/public/common/ssl_status.h"
#include "ipc/ipc_message_macros.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebPoint.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebReferrerPolicy.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebRect.h"
#include "webkit/glue/webpreferences.h"
#include "webkit/glue/window_open_disposition.h"
#include "webkit/plugins/webplugininfo.h"

#undef IPC_MESSAGE_EXPORT
#define IPC_MESSAGE_EXPORT CONTENT_EXPORT

IPC_ENUM_TRAITS(content::ConsoleMessageLevel)
IPC_ENUM_TRAITS(content::PageTransition)
IPC_ENUM_TRAITS(content::SecurityStyle)
IPC_ENUM_TRAITS(WebKit::WebReferrerPolicy)
IPC_ENUM_TRAITS(WindowOpenDisposition)
IPC_ENUM_TRAITS(webkit_glue::WebPreferences::EditingBehavior)

IPC_STRUCT_TRAITS_BEGIN(WebKit::WebPoint)
  IPC_STRUCT_TRAITS_MEMBER(x)
  IPC_STRUCT_TRAITS_MEMBER(y)
IPC_STRUCT_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(WebKit::WebRect)
  IPC_STRUCT_TRAITS_MEMBER(x)
  IPC_STRUCT_TRAITS_MEMBER(y)
  IPC_STRUCT_TRAITS_MEMBER(width)
  IPC_STRUCT_TRAITS_MEMBER(height)
IPC_STRUCT_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(content::PasswordForm)
IPC_STRUCT_TRAITS_MEMBER(signon_realm)
IPC_STRUCT_TRAITS_MEMBER(origin)
IPC_STRUCT_TRAITS_MEMBER(action)
IPC_STRUCT_TRAITS_MEMBER(submit_element)
IPC_STRUCT_TRAITS_MEMBER(username_element)
IPC_STRUCT_TRAITS_MEMBER(username_value)
IPC_STRUCT_TRAITS_MEMBER(password_element)
IPC_STRUCT_TRAITS_MEMBER(password_value)
IPC_STRUCT_TRAITS_MEMBER(old_password_element)
IPC_STRUCT_TRAITS_MEMBER(old_password_value)
IPC_STRUCT_TRAITS_MEMBER(ssl_valid)
IPC_STRUCT_TRAITS_MEMBER(preferred)
IPC_STRUCT_TRAITS_MEMBER(blacklisted_by_user)
IPC_STRUCT_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(content::SSLStatus)
  IPC_STRUCT_TRAITS_MEMBER(security_style)
  IPC_STRUCT_TRAITS_MEMBER(cert_id)
  IPC_STRUCT_TRAITS_MEMBER(cert_status)
  IPC_STRUCT_TRAITS_MEMBER(security_bits)
  IPC_STRUCT_TRAITS_MEMBER(connection_status)
  IPC_STRUCT_TRAITS_MEMBER(content_status)
IPC_STRUCT_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(webkit::WebPluginMimeType)
  IPC_STRUCT_TRAITS_MEMBER(mime_type)
  IPC_STRUCT_TRAITS_MEMBER(file_extensions)
  IPC_STRUCT_TRAITS_MEMBER(description)
  IPC_STRUCT_TRAITS_MEMBER(additional_param_names)
  IPC_STRUCT_TRAITS_MEMBER(additional_param_values)
IPC_STRUCT_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(webkit::WebPluginInfo)
  IPC_STRUCT_TRAITS_MEMBER(name)
  IPC_STRUCT_TRAITS_MEMBER(path)
  IPC_STRUCT_TRAITS_MEMBER(version)
  IPC_STRUCT_TRAITS_MEMBER(desc)
  IPC_STRUCT_TRAITS_MEMBER(mime_types)
  IPC_STRUCT_TRAITS_MEMBER(type)
  IPC_STRUCT_TRAITS_MEMBER(pepper_permissions)
IPC_STRUCT_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(webkit_glue::WebPreferences)
  IPC_STRUCT_TRAITS_MEMBER(standard_font_family_map)
  IPC_STRUCT_TRAITS_MEMBER(fixed_font_family_map)
  IPC_STRUCT_TRAITS_MEMBER(serif_font_family_map)
  IPC_STRUCT_TRAITS_MEMBER(sans_serif_font_family_map)
  IPC_STRUCT_TRAITS_MEMBER(cursive_font_family_map)
  IPC_STRUCT_TRAITS_MEMBER(fantasy_font_family_map)
  IPC_STRUCT_TRAITS_MEMBER(default_font_size)
  IPC_STRUCT_TRAITS_MEMBER(default_fixed_font_size)
  IPC_STRUCT_TRAITS_MEMBER(apply_default_device_scale_factor_in_compositor)
  IPC_STRUCT_TRAITS_MEMBER(apply_page_scale_factor_in_compositor)
  IPC_STRUCT_TRAITS_MEMBER(per_tile_painting_enabled)
  IPC_STRUCT_TRAITS_MEMBER(accelerated_animation_enabled)
  IPC_STRUCT_TRAITS_MEMBER(minimum_font_size)
  IPC_STRUCT_TRAITS_MEMBER(minimum_logical_font_size)
  IPC_STRUCT_TRAITS_MEMBER(default_encoding)
  IPC_STRUCT_TRAITS_MEMBER(javascript_enabled)
  IPC_STRUCT_TRAITS_MEMBER(web_security_enabled)
  IPC_STRUCT_TRAITS_MEMBER(javascript_can_open_windows_automatically)
  IPC_STRUCT_TRAITS_MEMBER(loads_images_automatically)
  IPC_STRUCT_TRAITS_MEMBER(images_enabled)
  IPC_STRUCT_TRAITS_MEMBER(plugins_enabled)
  IPC_STRUCT_TRAITS_MEMBER(dom_paste_enabled)
  IPC_STRUCT_TRAITS_MEMBER(developer_extras_enabled)
  IPC_STRUCT_TRAITS_MEMBER(inspector_settings)
  IPC_STRUCT_TRAITS_MEMBER(site_specific_quirks_enabled)
  IPC_STRUCT_TRAITS_MEMBER(shrinks_standalone_images_to_fit)
  IPC_STRUCT_TRAITS_MEMBER(uses_universal_detector)
  IPC_STRUCT_TRAITS_MEMBER(text_areas_are_resizable)
  IPC_STRUCT_TRAITS_MEMBER(java_enabled)
  IPC_STRUCT_TRAITS_MEMBER(allow_scripts_to_close_windows)
  IPC_STRUCT_TRAITS_MEMBER(uses_page_cache)
  IPC_STRUCT_TRAITS_MEMBER(remote_fonts_enabled)
  IPC_STRUCT_TRAITS_MEMBER(javascript_can_access_clipboard)
  IPC_STRUCT_TRAITS_MEMBER(xss_auditor_enabled)
  IPC_STRUCT_TRAITS_MEMBER(dns_prefetching_enabled)
  IPC_STRUCT_TRAITS_MEMBER(local_storage_enabled)
  IPC_STRUCT_TRAITS_MEMBER(databases_enabled)
  IPC_STRUCT_TRAITS_MEMBER(application_cache_enabled)
  IPC_STRUCT_TRAITS_MEMBER(tabs_to_links)
  IPC_STRUCT_TRAITS_MEMBER(hyperlink_auditing_enabled)
  IPC_STRUCT_TRAITS_MEMBER(is_online)
  IPC_STRUCT_TRAITS_MEMBER(user_style_sheet_enabled)
  IPC_STRUCT_TRAITS_MEMBER(user_style_sheet_location)
  IPC_STRUCT_TRAITS_MEMBER(author_and_user_styles_enabled)
  IPC_STRUCT_TRAITS_MEMBER(frame_flattening_enabled)
  IPC_STRUCT_TRAITS_MEMBER(allow_universal_access_from_file_urls)
  IPC_STRUCT_TRAITS_MEMBER(allow_file_access_from_file_urls)
  IPC_STRUCT_TRAITS_MEMBER(webaudio_enabled)
  IPC_STRUCT_TRAITS_MEMBER(experimental_webgl_enabled)
  IPC_STRUCT_TRAITS_MEMBER(flash_3d_enabled)
  IPC_STRUCT_TRAITS_MEMBER(flash_stage3d_enabled)
  IPC_STRUCT_TRAITS_MEMBER(gl_multisampling_enabled)
  IPC_STRUCT_TRAITS_MEMBER(privileged_webgl_extensions_enabled)
  IPC_STRUCT_TRAITS_MEMBER(show_composited_layer_borders)
  IPC_STRUCT_TRAITS_MEMBER(show_composited_layer_tree)
  IPC_STRUCT_TRAITS_MEMBER(show_fps_counter)
  IPC_STRUCT_TRAITS_MEMBER(accelerated_compositing_for_overflow_scroll_enabled)
  IPC_STRUCT_TRAITS_MEMBER(show_paint_rects)
  IPC_STRUCT_TRAITS_MEMBER(render_vsync_enabled)
  IPC_STRUCT_TRAITS_MEMBER(asynchronous_spell_checking_enabled)
  IPC_STRUCT_TRAITS_MEMBER(unified_textchecker_enabled)
  IPC_STRUCT_TRAITS_MEMBER(accelerated_compositing_enabled)
  IPC_STRUCT_TRAITS_MEMBER(force_compositing_mode)
  IPC_STRUCT_TRAITS_MEMBER(fixed_position_compositing_enabled)
  IPC_STRUCT_TRAITS_MEMBER(accelerated_2d_canvas_enabled)
  IPC_STRUCT_TRAITS_MEMBER(deferred_2d_canvas_enabled)
  IPC_STRUCT_TRAITS_MEMBER(accelerated_painting_enabled)
  IPC_STRUCT_TRAITS_MEMBER(accelerated_filters_enabled)
  IPC_STRUCT_TRAITS_MEMBER(gesture_tap_highlight_enabled)
  IPC_STRUCT_TRAITS_MEMBER(accelerated_compositing_for_plugins_enabled)
  IPC_STRUCT_TRAITS_MEMBER(accelerated_compositing_for_3d_transforms_enabled)
  IPC_STRUCT_TRAITS_MEMBER(accelerated_compositing_for_animation_enabled)
  IPC_STRUCT_TRAITS_MEMBER(accelerated_compositing_for_video_enabled)
  IPC_STRUCT_TRAITS_MEMBER(memory_info_enabled)
  IPC_STRUCT_TRAITS_MEMBER(fullscreen_enabled)
  IPC_STRUCT_TRAITS_MEMBER(allow_displaying_insecure_content)
  IPC_STRUCT_TRAITS_MEMBER(allow_running_insecure_content)
  IPC_STRUCT_TRAITS_MEMBER(enable_scroll_animator)
  IPC_STRUCT_TRAITS_MEMBER(visual_word_movement_enabled)
  IPC_STRUCT_TRAITS_MEMBER(password_echo_enabled)
  IPC_STRUCT_TRAITS_MEMBER(css_sticky_position_enabled)
  IPC_STRUCT_TRAITS_MEMBER(css_shaders_enabled)
  IPC_STRUCT_TRAITS_MEMBER(css_variables_enabled)
  IPC_STRUCT_TRAITS_MEMBER(css_grid_layout_enabled)
  IPC_STRUCT_TRAITS_MEMBER(touch_enabled)
  IPC_STRUCT_TRAITS_MEMBER(device_supports_touch)
  IPC_STRUCT_TRAITS_MEMBER(device_supports_mouse)
  IPC_STRUCT_TRAITS_MEMBER(touch_adjustment_enabled)
  IPC_STRUCT_TRAITS_MEMBER(default_tile_width)
  IPC_STRUCT_TRAITS_MEMBER(default_tile_height)
  IPC_STRUCT_TRAITS_MEMBER(max_untiled_layer_width)
  IPC_STRUCT_TRAITS_MEMBER(max_untiled_layer_height)
  IPC_STRUCT_TRAITS_MEMBER(fixed_position_creates_stacking_context)
  IPC_STRUCT_TRAITS_MEMBER(sync_xhr_in_documents_enabled)
  IPC_STRUCT_TRAITS_MEMBER(deferred_image_decoding_enabled)
  IPC_STRUCT_TRAITS_MEMBER(number_of_cpu_cores)
  IPC_STRUCT_TRAITS_MEMBER(editing_behavior)
  IPC_STRUCT_TRAITS_MEMBER(cookie_enabled)
  IPC_STRUCT_TRAITS_MEMBER(apply_page_scale_factor_in_compositor)
#if defined(OS_ANDROID)
  IPC_STRUCT_TRAITS_MEMBER(text_autosizing_enabled)
  IPC_STRUCT_TRAITS_MEMBER(font_scale_factor)
  IPC_STRUCT_TRAITS_MEMBER(force_enable_zoom)
  IPC_STRUCT_TRAITS_MEMBER(supports_multiple_windows)
#endif
IPC_STRUCT_TRAITS_END()

#endif  // CONTENT_PUBLIC_COMMON_COMMON_PARAM_TRAITS_MACROS_H_
