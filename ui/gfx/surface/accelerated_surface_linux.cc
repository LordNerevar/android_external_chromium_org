// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/surface/accelerated_surface_linux.h"

#include <X11/Xlib.h>

#include "third_party/angle/include/EGL/egl.h"
#include "third_party/angle/include/EGL/eglext.h"
#include "ui/gfx/gl/gl_surface_egl.h"
#include "ui/gfx/gl/gl_bindings.h"

AcceleratedSurface::AcceleratedSurface(const gfx::Size& size)
    : size_(size) {
  Display* dpy = gfx::GLSurfaceEGL::GetNativeDisplay();
  EGLDisplay edpy = gfx::GLSurfaceEGL::GetDisplay();

  XID window = XDefaultRootWindow(dpy);
  XWindowAttributes gwa;
  XGetWindowAttributes(dpy, window, &gwa);
  pixmap_ = XCreatePixmap(
      dpy, window, size_.width(), size_.height(), gwa.depth);

  image_ = eglCreateImageKHR(
      edpy, EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR, (void*) pixmap_, NULL);

  glGenTextures(1, &texture_);

  GLint current_texture = 0;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_texture);

  glBindTexture(GL_TEXTURE_2D, texture_);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image_);

  glBindTexture(GL_TEXTURE_2D, current_texture);
}

AcceleratedSurface::~AcceleratedSurface() {
  glDeleteTextures(1, &texture_);
  eglDestroyImageKHR(gfx::GLSurfaceEGL::GetDisplay(), image_);
  XFreePixmap(gfx::GLSurfaceEGL::GetNativeDisplay(), pixmap_);
}
