/*
 * Copyright 2009, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


// This file contains the implementation of the GAPID3D9 class.

#include "command_buffer/service/win/d3d9/gapi_d3d9.h"

namespace o3d {
namespace command_buffer {

GAPID3D9::GAPID3D9()
    : d3d_module_(NULL),
      d3dx_module_(NULL),
      d3d_(NULL),
      d3d_device_(NULL),
      hwnd_(NULL),
      current_vertex_struct_(0),
      validate_streams_(true),
      max_vertices_(0),
      current_effect_id_(0),
      validate_effect_(true),
      current_effect_(NULL),
      vertex_buffers_(),
      index_buffers_(),
      vertex_structs_(),
      back_buffer_surface_(NULL),
      back_buffer_depth_surface_(NULL),
      current_surface_id_(kInvalidResource),
      current_depth_surface_id_(kInvalidResource),
      direct3d_create9_(NULL),
      get_shader_constant_table_(NULL),
      create_effect_(NULL),
      get_shader_input_semantics_(NULL) {}

GAPID3D9::~GAPID3D9() {}

// Initializes a D3D interface and device, and sets basic states.
bool GAPID3D9::Initialize() {
  if (!FindDirect3DFunctions()) {
    Destroy();
    return false;
  }

  d3d_ = Direct3DCreate(D3D_SDK_VERSION);
  if (NULL == d3d_) {
    LOG(ERROR) << "Failed to create the initial D3D9 Interface";
    Destroy();
    return false;
  }
  d3d_device_ = NULL;

  D3DDISPLAYMODE d3ddm;
  d3d_->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);
  // NOTE: make sure the backbuffer matches this format, as it is
  // currently currently assumed to be 32-bit 8X8R8G8B

  D3DPRESENT_PARAMETERS d3dpp;
  ZeroMemory(&d3dpp, sizeof(d3dpp));
  d3dpp.Windowed               = TRUE;
  d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
  d3dpp.BackBufferFormat       = d3ddm.Format;
  d3dpp.EnableAutoDepthStencil = TRUE;
  d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
  d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;  // wait for vsync
  // Note: SwapEffect=DISCARD is req. for multisample to function
  // Note: AutoDepthStencilFormat is 16-bit (not the usual 8-bit)

  // query multisampling
  const int kNumTypesToCheck = 4;
  D3DMULTISAMPLE_TYPE multisample_types[] = { D3DMULTISAMPLE_5_SAMPLES,
                                              D3DMULTISAMPLE_4_SAMPLES,
                                              D3DMULTISAMPLE_2_SAMPLES,
                                              D3DMULTISAMPLE_NONE };
  DWORD multisample_quality = 0;
  for (int i = 0; i < kNumTypesToCheck; ++i) {
    // check back-buffer for multisampling at level "i";
    // back buffer = 32-bit XRGB (i.e. no alpha)
    if (SUCCEEDED(d3d_->CheckDeviceMultiSampleType(
                      D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
                      D3DFMT_X8R8G8B8,
                      true,  // result is windowed
                      multisample_types[i],
                      &multisample_quality))) {
      // back buffer succeeded, now check depth-buffer
      // depth buffer = 24-bit, stencil = 8-bit
      // NOTE: 8-bit not 16-bit like the D3DPRESENT_PARAMETERS
      if (SUCCEEDED(d3d_->CheckDeviceMultiSampleType(
                        D3DADAPTER_DEFAULT,
                        D3DDEVTYPE_HAL,
                        D3DFMT_D24S8,
                        true,  // result is windowed
                        multisample_types[i],
                        &multisample_quality))) {
        d3dpp.MultiSampleType = multisample_types[i];
        d3dpp.MultiSampleQuality = multisample_quality - 1;
        break;
      }
    }
  }
  // D3DCREATE_FPU_PRESERVE is there because Firefox 3 relies on specific FPU
  // flags for its UI rendering. Apparently Firefox 2 does not, though we don't
  // currently propagate that info.
  // TODO: check if FPU_PRESERVE has a significant perf hit, in which
  // case find out if we can disable it for Firefox 2/other browsers, and/or if
  // it makes sense to switch FPU flags before/after every DX call.
  DWORD flags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE;
  if (!SUCCEEDED(d3d_->CreateDevice(D3DADAPTER_DEFAULT,
                                    D3DDEVTYPE_HAL,
                                    hwnd_,
                                    flags,
                                    &d3dpp,
                                    &d3d_device_))) {
    LOG(ERROR) << "Failed to create the D3D Device";
    Destroy();
    return false;
  }
  // initialise the d3d graphics state.
  HR(d3d_device_->SetRenderState(D3DRS_LIGHTING, FALSE));
  HR(d3d_device_->SetRenderState(D3DRS_ZENABLE, TRUE));
  HR(d3d_device_->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
  return true;
}

// Deletes the D3D9 Device and releases the D3D interface.
void GAPID3D9::Destroy() {
  vertex_buffers_.DestroyAllResources();
  index_buffers_.DestroyAllResources();
  vertex_structs_.DestroyAllResources();
  effects_.DestroyAllResources();
  effect_params_.DestroyAllResources();
  textures_.DestroyAllResources();
  samplers_.DestroyAllResources();
  render_surfaces_.DestroyAllResources();
  depth_surfaces_.DestroyAllResources();
  if (d3d_device_) {
    d3d_device_->Release();
    d3d_device_ = NULL;
  }
  if (d3d_) {
    d3d_->Release();
    d3d_ = NULL;
  }
  if (d3dx_module_) {
    FreeLibrary(d3dx_module_);
    d3dx_module_ = NULL;
    get_shader_constant_table_ = NULL;
    create_effect_ = NULL;
    get_shader_input_semantics_ = NULL;
  }
  if (d3d_module_) {
    FreeLibrary(d3d_module_);
    d3d_module_ = NULL;
    direct3d_create9_ = NULL;
  }
}

// Begins the frame.
void GAPID3D9::BeginFrame() {
  HR(d3d_device_->GetRenderTarget(0, &back_buffer_surface_));
  HR(d3d_device_->GetDepthStencilSurface(&back_buffer_depth_surface_));
  HR(d3d_device_->BeginScene());
}

// Ends the frame, presenting the back buffer.
void GAPID3D9::EndFrame() {
  DirtyEffect();
  HR(d3d_device_->EndScene());
  HR(d3d_device_->Present(NULL, NULL, NULL, NULL));

  // Release the back-buffer references.
  back_buffer_surface_ = NULL;
  back_buffer_depth_surface_ = NULL;
}

// Clears the selected buffers.
void GAPID3D9::Clear(unsigned int buffers,
                     const RGBA &color,
                     float depth,
                     unsigned int stencil) {
  DWORD flags = (buffers & kColor ? D3DCLEAR_TARGET : 0) |
                (buffers & kDepth ? D3DCLEAR_ZBUFFER : 0) |
                (buffers & kStencil ? D3DCLEAR_STENCIL : 0);
  HR(d3d_device_->Clear(0,
                        NULL,
                        flags,
                        D3DCOLOR_COLORVALUE(color.red,
                                            color.green,
                                            color.blue,
                                            color.alpha),
                        depth,
                        stencil));
}

// Sets the viewport.
void GAPID3D9::SetViewport(unsigned int x,
                           unsigned int y,
                           unsigned int width,
                           unsigned int height,
                           float z_min,
                           float z_max) {
  D3DVIEWPORT9 viewport = {x, y, width, height, z_min, z_max};
  HR(d3d_device_->SetViewport(&viewport));
}

// Converts an unsigned int RGBA color into an unsigned int ARGB (DirectX)
// color.
static unsigned int RGBAToARGB(unsigned int rgba) {
  return (rgba >> 8) | (rgba << 24);
}

// Sets the current VertexStruct. Just keep track of the ID.
parse_error::ParseError GAPID3D9::SetVertexStruct(ResourceId id) {
  current_vertex_struct_ = id;
  validate_streams_ = true;
  return parse_error::kParseNoError;
}

bool GAPID3D9::FindDirect3DFunctions() {
  d3d_module_ = LoadLibrary(TEXT("d3d9.dll"));
  if (NULL == d3d_module_) {
    LOG(ERROR) << "Failed to load d3d9.dll";
    return false;
  }

  direct3d_create9_ = reinterpret_cast<Direct3DCreate9Proc>(
      GetProcAddress(d3d_module_, "Direct3DCreate9"));
  if (NULL == direct3d_create9_) {
    LOG(ERROR) << "Failed to find Direct3DCreate9 in d3d9.dll";
    Destroy();
    return false;
  }

  d3dx_module_ = LoadLibrary(TEXT("d3dx9_36.dll"));
  if (NULL == d3d_module_) {
    LOG(ERROR) << "Failed to load d3dx9_36.dll";
    return false;
  }

  get_shader_constant_table_ = reinterpret_cast<D3DXGetShaderConstantTableProc>(
      GetProcAddress(d3dx_module_, "D3DXGetShaderConstantTable"));
  if (NULL == get_shader_constant_table_) {
    LOG(ERROR) << "Failed to find D3DXGetShaderConstantTable in d3dx9_36.dll";
    Destroy();
    return false;
  }

  create_effect_ = reinterpret_cast<D3DXCreateEffectProc>(
      GetProcAddress(d3dx_module_, "D3DXCreateEffect"));
  if (NULL == create_effect_) {
    LOG(ERROR) << "Failed to find D3DXCreateEffect in d3dx9_36.dll";
    Destroy();
    return false;
  }

  get_shader_input_semantics_ =
      reinterpret_cast<D3DXGetShaderInputSemanticsProc>(
          GetProcAddress(d3dx_module_, "D3DXGetShaderInputSemantics"));
  if (NULL == get_shader_input_semantics_) {
    LOG(ERROR) << "Failed to find D3DXGetShaderInputSemantics in d3dx9_36.dll";
    Destroy();
    return false;
  }

  return true;
}

// Sets in D3D the input streams of the current vertex struct.
bool GAPID3D9::ValidateStreams() {
  DCHECK(validate_streams_);
  VertexStructD3D9 *vertex_struct = vertex_structs_.Get(current_vertex_struct_);
  if (!vertex_struct) {
    LOG(ERROR) << "Drawing with invalid streams.";
    return false;
  }
  max_vertices_ = vertex_struct->SetStreams(this);
  validate_streams_ = false;
  return max_vertices_ > 0;
}

// Converts a GAPID3D9::PrimitiveType to a D3DPRIMITIVETYPE.
static D3DPRIMITIVETYPE D3DPrimitive(
    command_buffer::PrimitiveType primitive_type) {
  switch (primitive_type) {
    case command_buffer::kPoints:
      return D3DPT_POINTLIST;
    case command_buffer::kLines:
      return D3DPT_LINELIST;
    case command_buffer::kLineStrips:
      return D3DPT_LINESTRIP;
    case command_buffer::kTriangles:
      return D3DPT_TRIANGLELIST;
    case command_buffer::kTriangleStrips:
      return D3DPT_TRIANGLESTRIP;
    case command_buffer::kTriangleFans:
      return D3DPT_TRIANGLEFAN;
    default:
      LOG(FATAL) << "Invalid primitive type";
      return D3DPT_POINTLIST;
  }
}

// Draws with the current vertex struct.
parse_error::ParseError GAPID3D9::Draw(
    PrimitiveType primitive_type,
    unsigned int first,
    unsigned int count) {
  if (validate_streams_ && !ValidateStreams()) {
    // TODO: add proper error management
    return parse_error::kParseInvalidArguments;
  }
  if (validate_effect_ && !ValidateEffect()) {
    // TODO: add proper error management
    return parse_error::kParseInvalidArguments;
  }
  DCHECK(current_effect_);
  if (!current_effect_->CommitParameters()) {
    return parse_error::kParseInvalidArguments;
  }
  if (first + count > max_vertices_) {
    // TODO: add proper error management
    return parse_error::kParseInvalidArguments;
  }
  HR(d3d_device_->DrawPrimitive(D3DPrimitive(primitive_type), first, count));
  return parse_error::kParseNoError;
}

// Draws with the current vertex struct.
parse_error::ParseError GAPID3D9::DrawIndexed(
    PrimitiveType primitive_type,
    ResourceId index_buffer_id,
    unsigned int first,
    unsigned int count,
    unsigned int min_index,
    unsigned int max_index) {
  IndexBufferD3D9 *index_buffer = index_buffers_.Get(index_buffer_id);
  if (!index_buffer) return parse_error::kParseInvalidArguments;
  if (validate_streams_ && !ValidateStreams()) {
    // TODO: add proper error management
    return parse_error::kParseInvalidArguments;
  }
  if (validate_effect_ && !ValidateEffect()) {
    // TODO: add proper error management
    return parse_error::kParseInvalidArguments;
  }
  DCHECK(current_effect_);
  if (!current_effect_->CommitParameters()) {
    return parse_error::kParseInvalidArguments;
  }
  if ((min_index >= max_vertices_) || (max_index > max_vertices_)) {
    // TODO: add proper error management
    return parse_error::kParseInvalidArguments;
  }

  HR(d3d_device_->SetIndices(index_buffer->d3d_index_buffer()));
  HR(d3d_device_->DrawIndexedPrimitive(D3DPrimitive(primitive_type), 0,
                                       min_index, max_index - min_index + 1,
                                       first, count));
  return parse_error::kParseNoError;
}

}  // namespace command_buffer
}  // namespace o3d
