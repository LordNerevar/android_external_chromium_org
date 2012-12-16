// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GL_EGL_API_IMPLEMENTATION_H_
#define UI_GL_EGL_API_IMPLEMENTATION_H_

#include "base/compiler_specific.h"
#include "gl_bindings.h"
#include "ui/gl/gl_export.h"

namespace gfx {

class GLContext;

void InitializeGLBindingsEGL();
void InitializeGLExtensionBindingsEGL(GLContext* context);
void InitializeDebugGLBindingsEGL();
void ClearGLBindingsEGL();

class GL_EXPORT EGLApiBase : public EGLApi {
 public:
  // Include the auto-generated part of this class. We split this because
  // it means we can easily edit the non-auto generated parts right here in
  // this file instead of having to edit some template or the code generator.
  #include "gl_bindings_api_autogen_egl.h"

 protected:
  EGLApiBase();
  virtual ~EGLApiBase();
  void InitializeBase(DriverEGL* driver);

  DriverEGL* driver_;
};

class GL_EXPORT RealEGLApi : public EGLApiBase {
 public:
  RealEGLApi();
  virtual ~RealEGLApi();
  void Initialize(DriverEGL* driver);
};

}  // namespace gfx

#endif  // UI_GL_EGL_API_IMPLEMENTATION_H_



