// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is auto-generated from
// gpu/command_buffer/build_gles2_cmd_buffer.py
// DO NOT EDIT!

// This file contains Chromium-specific GLES2 extensions declarations.

#ifndef GPU_GLES2_GL2EXTCHROMIUM_H_
#define GPU_GLES2_GL2EXTCHROMIUM_H_

#include <GLES2/gl2chromium.h>
#ifdef __cplusplus
extern "C" {
#endif

/* GL_CHROMIUM_iosurface */
#ifndef GL_CHROMIUM_iosurface
#define GL_CHROMIUM_iosurface 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glTexImageIOSurface2DCHROMIUM(
    GLenum target, GLsizei width, GLsizei height, GLuint ioSurfaceId,
    GLuint plane);
#endif
typedef void (
    GL_APIENTRYP PFNGLTEXIMAGEIOSURFACE2DCHROMIUMPROC) (
        GLenum target, GLsizei width, GLsizei height, GLuint ioSurfaceId,
        GLuint plane);
#endif  /* GL_CHROMIUM_iosurface */

/* GL_CHROMIUM_gpu_memory_manager */
#ifndef GL_CHROMIUM_gpu_memory_manager
#define GL_CHROMIUM_gpu_memory_manager 1

#ifndef GL_TEXTURE_POOL_UNMANAGED_CHROMIUM
#define GL_TEXTURE_POOL_UNMANAGED_CHROMIUM 0x6002
#endif

#ifndef GL_TEXTURE_POOL_CHROMIUM
#define GL_TEXTURE_POOL_CHROMIUM 0x6000
#endif

#ifndef GL_TEXTURE_POOL_MANAGED_CHROMIUM
#define GL_TEXTURE_POOL_MANAGED_CHROMIUM 0x6001
#endif
#endif  /* GL_CHROMIUM_gpu_memory_manager */

/* GL_CHROMIUM_texture_mailbox */
#ifndef GL_CHROMIUM_texture_mailbox
#define GL_CHROMIUM_texture_mailbox 1

#ifndef GL_MAILBOX_SIZE_CHROMIUM
#define GL_MAILBOX_SIZE_CHROMIUM 64
#endif
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glGenMailboxCHROMIUM(GLbyte* mailbox);
#endif
typedef void (GL_APIENTRYP PFNGLGENMAILBOXCHROMIUMPROC) (GLbyte* mailbox);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glProduceTextureCHROMIUM(
    GLenum target, const GLbyte* mailbox);
#endif
typedef void (
    GL_APIENTRYP PFNGLPRODUCETEXTURECHROMIUMPROC) (
        GLenum target, const GLbyte* mailbox);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glConsumeTextureCHROMIUM(
    GLenum target, const GLbyte* mailbox);
#endif
typedef void (
    GL_APIENTRYP PFNGLCONSUMETEXTURECHROMIUMPROC) (
        GLenum target, const GLbyte* mailbox);
#endif  /* GL_CHROMIUM_texture_mailbox */

/* GL_CHROMIUM_pixel_transfer_buffer_object */
#ifndef GL_CHROMIUM_pixel_transfer_buffer_object
#define GL_CHROMIUM_pixel_transfer_buffer_object 1

#ifndef GL_PIXEL_UNPACK_TRANSFER_BUFFER_CHROMIUM
#define GL_PIXEL_UNPACK_TRANSFER_BUFFER_CHROMIUM 0x88EC
#endif

#ifndef GL_PIXEL_UNPACK_TRANSFER_BUFFER_BINDING_CHROMIUM
#define GL_PIXEL_UNPACK_TRANSFER_BUFFER_BINDING_CHROMIUM 0x88EF
#endif
#endif  /* GL_CHROMIUM_pixel_transfer_buffer_object */

/* GL_CHROMIUM_map_sub */
#ifndef GL_CHROMIUM_map_sub
#define GL_CHROMIUM_map_sub 1

#ifndef GL_READ_ONLY
#define GL_READ_ONLY 0x88B8
#endif

#ifndef GL_WRITE_ONLY
#define GL_WRITE_ONLY 0x88B9
#endif
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void* GL_APIENTRY glMapBufferSubDataCHROMIUM(
    GLuint target, GLintptr offset, GLsizeiptr size, GLenum access);
#endif
typedef void* (
    GL_APIENTRYP PFNGLMAPBUFFERSUBDATACHROMIUMPROC) (
        GLuint target, GLintptr offset, GLsizeiptr size, GLenum access);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glUnmapBufferSubDataCHROMIUM(const void* mem);
#endif
typedef void (
    GL_APIENTRYP PFNGLUNMAPBUFFERSUBDATACHROMIUMPROC) (const void* mem);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void* GL_APIENTRY glMapTexSubImage2DCHROMIUM(
    GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
    GLsizei height, GLenum format, GLenum type, GLenum access);
#endif
typedef void* (
    GL_APIENTRYP PFNGLMAPTEXSUBIMAGE2DCHROMIUMPROC) (
        GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
        GLsizei height, GLenum format, GLenum type, GLenum access);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glUnmapTexSubImage2DCHROMIUM(const void* mem);
#endif
typedef void (
    GL_APIENTRYP PFNGLUNMAPTEXSUBIMAGE2DCHROMIUMPROC) (const void* mem);
#endif  /* GL_CHROMIUM_map_sub */

/* GL_CHROMIUM_request_extension */
#ifndef GL_CHROMIUM_request_extension
#define GL_CHROMIUM_request_extension 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL const GLchar* GL_APIENTRY glGetRequestableExtensionsCHROMIUM();
#endif
typedef const GLchar* (
    GL_APIENTRYP PFNGLGETREQUESTABLEEXTENSIONSCHROMIUMPROC) ();
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glRequestExtensionCHROMIUM(const char* extension);
#endif
typedef void (
    GL_APIENTRYP PFNGLREQUESTEXTENSIONCHROMIUMPROC) (const char* extension);
#endif  /* GL_CHROMIUM_request_extension */

/* GL_CHROMIUM_get_error_query */
#ifndef GL_CHROMIUM_get_error_query
#define GL_CHROMIUM_get_error_query 1

#ifndef GL_GET_ERROR_QUERY_CHROMIUM
#define GL_GET_ERROR_QUERY_CHROMIUM 0x84F3
#endif
#endif  /* GL_CHROMIUM_get_error_query */

/* GL_CHROMIUM_texture_from_image */
#ifndef GL_CHROMIUM_texture_from_image
#define GL_CHROMIUM_texture_from_image 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glBindTexImage2DCHROMIUM(
    GLenum target, GLint imageId);
#endif
typedef void (
    GL_APIENTRYP PFNGLBINDTEXIMAGE2DCHROMIUMPROC) (
        GLenum target, GLint imageId);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glReleaseTexImage2DCHROMIUM(
    GLenum target, GLint imageId);
#endif
typedef void (
    GL_APIENTRYP PFNGLRELEASETEXIMAGE2DCHROMIUMPROC) (
        GLenum target, GLint imageId);
#endif  /* GL_CHROMIUM_texture_from_image */

/* GL_CHROMIUM_rate_limit_offscreen_context */
#ifndef GL_CHROMIUM_rate_limit_offscreen_context
#define GL_CHROMIUM_rate_limit_offscreen_context 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glRateLimitOffscreenContextCHROMIUM();
#endif
typedef void (GL_APIENTRYP PFNGLRATELIMITOFFSCREENCONTEXTCHROMIUMPROC) ();
#endif  /* GL_CHROMIUM_rate_limit_offscreen_context */

/* GL_CHROMIUM_post_sub_buffer */
#ifndef GL_CHROMIUM_post_sub_buffer
#define GL_CHROMIUM_post_sub_buffer 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glPostSubBufferCHROMIUM(
    GLint x, GLint y, GLint width, GLint height);
#endif
typedef void (
    GL_APIENTRYP PFNGLPOSTSUBBUFFERCHROMIUMPROC) (
        GLint x, GLint y, GLint width, GLint height);
#endif  /* GL_CHROMIUM_post_sub_buffer */

/* GL_CHROMIUM_bind_uniform_location */
#ifndef GL_CHROMIUM_bind_uniform_location
#define GL_CHROMIUM_bind_uniform_location 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glBindUniformLocationCHROMIUM(
    GLuint program, GLint location, const char* name);
#endif
typedef void (
    GL_APIENTRYP PFNGLBINDUNIFORMLOCATIONCHROMIUMPROC) (
        GLuint program, GLint location, const char* name);
#endif  /* GL_CHROMIUM_bind_uniform_location */

/* GL_CHROMIUM_command_buffer_query */
#ifndef GL_CHROMIUM_command_buffer_query
#define GL_CHROMIUM_command_buffer_query 1

#ifndef GL_COMMANDS_ISSUED_CHROMIUM
#define GL_COMMANDS_ISSUED_CHROMIUM 0x84F2
#endif
#endif  /* GL_CHROMIUM_command_buffer_query */

/* GL_EXT_framebuffer_multisample */
#ifndef GL_EXT_framebuffer_multisample
#define GL_EXT_framebuffer_multisample 1

#ifndef GL_FRAMEBUFFER_BINDING_EXT
#define GL_FRAMEBUFFER_BINDING_EXT GL_FRAMEBUFFER_BINDING
#endif

#ifndef GL_DRAW_FRAMEBUFFER_BINDING_EXT
#define GL_DRAW_FRAMEBUFFER_BINDING_EXT GL_DRAW_FRAMEBUFFER_BINDING
#endif

#ifndef GL_RENDERBUFFER_BINDING_EXT
#define GL_RENDERBUFFER_BINDING_EXT GL_RENDERBUFFER_BINDING
#endif

#ifndef GL_RENDERBUFFER_SAMPLES
#define GL_RENDERBUFFER_SAMPLES 0x8CAB
#endif

#ifndef GL_READ_FRAMEBUFFER_EXT
#define GL_READ_FRAMEBUFFER_EXT GL_READ_FRAMEBUFFER
#endif

#ifndef GL_RENDERBUFFER_SAMPLES_EXT
#define GL_RENDERBUFFER_SAMPLES_EXT GL_RENDERBUFFER_SAMPLES
#endif

#ifndef GL_RENDERBUFFER_BINDING
#define GL_RENDERBUFFER_BINDING 0x8CA7
#endif

#ifndef GL_READ_FRAMEBUFFER_BINDING
#define GL_READ_FRAMEBUFFER_BINDING 0x8CAA
#endif

#ifndef GL_MAX_SAMPLES
#define GL_MAX_SAMPLES 0x8D57
#endif

#ifndef GL_READ_FRAMEBUFFER_BINDING_EXT
#define GL_READ_FRAMEBUFFER_BINDING_EXT GL_READ_FRAMEBUFFER_BINDING
#endif

#ifndef GL_DRAW_FRAMEBUFFER_BINDING
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#endif

#ifndef GL_MAX_SAMPLES_EXT
#define GL_MAX_SAMPLES_EXT GL_MAX_SAMPLES
#endif

#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#endif

#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER 0x8CA8
#endif

#ifndef GL_DRAW_FRAMEBUFFER_EXT
#define GL_DRAW_FRAMEBUFFER_EXT GL_DRAW_FRAMEBUFFER
#endif

#ifndef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
#endif

#ifndef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE  // NOLINT
#endif

#ifndef GL_FRAMEBUFFER_BINDING
#define GL_FRAMEBUFFER_BINDING 0x8CA6
#endif
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glRenderbufferStorageMultisampleEXT(
    GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
    GLsizei height);
#endif
typedef void (
    GL_APIENTRYP PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) (
        GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
        GLsizei height);
#endif  /* GL_EXT_framebuffer_multisample */

/* GL_CHROMIUM_texture_compression_dxt3 */
#ifndef GL_CHROMIUM_texture_compression_dxt3
#define GL_CHROMIUM_texture_compression_dxt3 1

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#endif  /* GL_CHROMIUM_texture_compression_dxt3 */

/* GL_CHROMIUM_texture_compression_dxt5 */
#ifndef GL_CHROMIUM_texture_compression_dxt5
#define GL_CHROMIUM_texture_compression_dxt5 1

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif
#endif  /* GL_CHROMIUM_texture_compression_dxt5 */

/* GL_CHROMIUM_async_pixel_transfers */
#ifndef GL_CHROMIUM_async_pixel_transfers
#define GL_CHROMIUM_async_pixel_transfers 1

#ifndef GL_ASYNC_PIXEL_TRANSFERS_COMPLETED_CHROMIUM
#define GL_ASYNC_PIXEL_TRANSFERS_COMPLETED_CHROMIUM 0x84F5
#endif
#endif  /* GL_CHROMIUM_async_pixel_transfers */

/* GL_CHROMIUM_copy_texture */
#ifndef GL_CHROMIUM_copy_texture
#define GL_CHROMIUM_copy_texture 1

#ifndef GL_UNPACK_COLORSPACE_CONVERSION_CHROMIUM
#define GL_UNPACK_COLORSPACE_CONVERSION_CHROMIUM 0x9243
#endif

#ifndef GL_UNPACK_UNPREMULTIPLY_ALPHA_CHROMIUM
#define GL_UNPACK_UNPREMULTIPLY_ALPHA_CHROMIUM 0x9242
#endif

#ifndef GL_UNPACK_PREMULTIPLY_ALPHA_CHROMIUM
#define GL_UNPACK_PREMULTIPLY_ALPHA_CHROMIUM 0x9241
#endif
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glCopyTextureCHROMIUM(
    GLenum target, GLenum source_id, GLenum dest_id, GLint level,
    GLint internalformat);
#endif
typedef void (
    GL_APIENTRYP PFNGLCOPYTEXTURECHROMIUMPROC) (
        GLenum target, GLenum source_id, GLenum dest_id, GLint level,
        GLint internalformat);
#endif  /* GL_CHROMIUM_copy_texture */

/* GL_CHROMIUM_lose_context */
#ifndef GL_CHROMIUM_lose_context
#define GL_CHROMIUM_lose_context 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glLoseContextCHROMIUM(
    GLenum current, GLenum other);
#endif
typedef void (
    GL_APIENTRYP PFNGLLOSECONTEXTCHROMIUMPROC) (GLenum current, GLenum other);
#endif  /* GL_CHROMIUM_lose_context */

/* GL_CHROMIUM_flipy */
#ifndef GL_CHROMIUM_flipy
#define GL_CHROMIUM_flipy 1

#ifndef GL_UNPACK_FLIP_Y_CHROMIUM
#define GL_UNPACK_FLIP_Y_CHROMIUM 0x9240
#endif
#endif  /* GL_CHROMIUM_flipy */

/* GL_ARB_texture_rectangle */
#ifndef GL_ARB_texture_rectangle
#define GL_ARB_texture_rectangle 1

#ifndef GL_SAMPLER_2D_RECT_ARB
#define GL_SAMPLER_2D_RECT_ARB 0x8B63
#endif

#ifndef GL_TEXTURE_BINDING_RECTANGLE_ARB
#define GL_TEXTURE_BINDING_RECTANGLE_ARB 0x84F6
#endif

#ifndef GL_TEXTURE_RECTANGLE_ARB
#define GL_TEXTURE_RECTANGLE_ARB 0x84F5
#endif
#endif  /* GL_ARB_texture_rectangle */

/* GL_CHROMIUM_enable_feature */
#ifndef GL_CHROMIUM_enable_feature
#define GL_CHROMIUM_enable_feature 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL GLboolean GL_APIENTRY glEnableFeatureCHROMIUM(const char* feature);
#endif
typedef GLboolean (
    GL_APIENTRYP PFNGLENABLEFEATURECHROMIUMPROC) (const char* feature);
#endif  /* GL_CHROMIUM_enable_feature */

/* GL_CHROMIUM_command_buffer_latency_query */
#ifndef GL_CHROMIUM_command_buffer_latency_query
#define GL_CHROMIUM_command_buffer_latency_query 1

#ifndef GL_LATENCY_QUERY_CHROMIUM
#define GL_LATENCY_QUERY_CHROMIUM 0x84F4
#endif
#endif  /* GL_CHROMIUM_command_buffer_latency_query */

/* GL_ARB_robustness */
#ifndef GL_ARB_robustness
#define GL_ARB_robustness 1

#ifndef GL_GUILTY_CONTEXT_RESET_ARB
#define GL_GUILTY_CONTEXT_RESET_ARB 0x8253
#endif

#ifndef GL_UNKNOWN_CONTEXT_RESET_ARB
#define GL_UNKNOWN_CONTEXT_RESET_ARB 0x8255
#endif

#ifndef GL_INNOCENT_CONTEXT_RESET_ARB
#define GL_INNOCENT_CONTEXT_RESET_ARB 0x8254
#endif
#endif  /* GL_ARB_robustness */

/* GL_EXT_framebuffer_blit */
#ifndef GL_EXT_framebuffer_blit
#define GL_EXT_framebuffer_blit 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glBlitFramebufferEXT(
    GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0,
    GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
#endif
typedef void (
    GL_APIENTRYP PFNGLBLITFRAMEBUFFEREXTPROC) (
        GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0,
        GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
#endif  /* GL_EXT_framebuffer_blit */

/* GL_CHROMIUM_resize */
#ifndef GL_CHROMIUM_resize
#define GL_CHROMIUM_resize 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glResizeCHROMIUM(GLuint width, GLuint height);
#endif
typedef void (
    GL_APIENTRYP PFNGLRESIZECHROMIUMPROC) (GLuint width, GLuint height);
#endif  /* GL_CHROMIUM_resize */

/* GL_CHROMIUM_get_multiple */
#ifndef GL_CHROMIUM_get_multiple
#define GL_CHROMIUM_get_multiple 1
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glGetMultipleIntegervCHROMIUM(
    const GLenum* pnames, GLuint count, GLint* results, GLsizeiptr size);
#endif
typedef void (
    GL_APIENTRYP PFNGLGETMULTIPLEINTEGERVCHROMIUMPROC) (
        const GLenum* pnames, GLuint count, GLint* results, GLsizeiptr size);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glGetProgramInfoCHROMIUM(
    GLuint program, GLsizei bufsize, GLsizei* size, void* info);
#endif
typedef void (
    GL_APIENTRYP PFNGLGETPROGRAMINFOCHROMIUMPROC) (
        GLuint program, GLsizei bufsize, GLsizei* size, void* info);
#endif  /* GL_CHROMIUM_get_multiple */

/* GL_CHROMIUM_front_buffer_cached */
#ifndef GL_CHROMIUM_front_buffer_cached
#define GL_CHROMIUM_front_buffer_cached 1
#endif  /* GL_CHROMIUM_front_buffer_cached */
#ifdef __cplusplus
}
#endif

#endif  // GPU_GLES2_GL2EXTCHROMIUM_H_

