// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_OUTPUT_PROGRAM_BINDING_H_
#define CC_OUTPUT_PROGRAM_BINDING_H_

#include <string>

#include "base/logging.h"
#include "cc/output/context_provider.h"
#include "cc/output/shader.h"

namespace blink { class WebGraphicsContext3D; }

namespace cc {

class ProgramBindingBase {
 public:
  ProgramBindingBase();
  ~ProgramBindingBase();

  bool Init(blink::WebGraphicsContext3D* context,
            const std::string& vertex_shader,
            const std::string& fragment_shader);
  bool Link(blink::WebGraphicsContext3D* context);
  void Cleanup(blink::WebGraphicsContext3D* context);

  unsigned program() const { return program_; }
  bool initialized() const { return initialized_; }

 protected:
  unsigned LoadShader(blink::WebGraphicsContext3D* context,
                      unsigned type,
                      const std::string& shader_source);
  unsigned CreateShaderProgram(blink::WebGraphicsContext3D* context,
                               unsigned vertex_shader,
                               unsigned fragment_shader);
  void CleanupShaders(blink::WebGraphicsContext3D* context);

  unsigned program_;
  unsigned vertex_shader_id_;
  unsigned fragment_shader_id_;
  bool initialized_;

 private:
  DISALLOW_COPY_AND_ASSIGN(ProgramBindingBase);
};

template <class VertexShader, class FragmentShader>
class ProgramBinding : public ProgramBindingBase {
 public:
  ProgramBinding() {}

  void Initialize(ContextProvider* context_provider,
                  TexCoordPrecision precision,
                  SamplerType sampler) {
    DCHECK(context_provider);
    DCHECK(!initialized_);

    if (context_provider->IsContextLost())
      return;

    if (!ProgramBindingBase::Init(
            context_provider->Context3d(),
            vertex_shader_.GetShaderString(),
            fragment_shader_.GetShaderString(precision, sampler))) {
      DCHECK(context_provider->IsContextLost());
      return;
    }

    int base_uniform_index = 0;
    vertex_shader_.Init(context_provider->Context3d(),
                        program_, &base_uniform_index);
    fragment_shader_.Init(context_provider->Context3d(),
                          program_, &base_uniform_index);

    // Link after binding uniforms
    if (!Link(context_provider->Context3d())) {
      DCHECK(context_provider->IsContextLost());
      return;
    }

    initialized_ = true;
  }

  const VertexShader& vertex_shader() const { return vertex_shader_; }
  const FragmentShader& fragment_shader() const { return fragment_shader_; }

 private:
  VertexShader vertex_shader_;
  FragmentShader fragment_shader_;

  DISALLOW_COPY_AND_ASSIGN(ProgramBinding);
};

}  // namespace cc

#endif  // CC_OUTPUT_PROGRAM_BINDING_H_
