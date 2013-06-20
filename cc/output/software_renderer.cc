// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/output/software_renderer.h"

#include "base/debug/trace_event.h"
#include "cc/base/math_util.h"
#include "cc/output/compositor_frame.h"
#include "cc/output/compositor_frame_ack.h"
#include "cc/output/compositor_frame_metadata.h"
#include "cc/output/copy_output_request.h"
#include "cc/output/output_surface.h"
#include "cc/output/software_output_device.h"
#include "cc/quads/checkerboard_draw_quad.h"
#include "cc/quads/debug_border_draw_quad.h"
#include "cc/quads/picture_draw_quad.h"
#include "cc/quads/render_pass_draw_quad.h"
#include "cc/quads/solid_color_draw_quad.h"
#include "cc/quads/texture_draw_quad.h"
#include "cc/quads/tile_draw_quad.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkDevice.h"
#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkShader.h"
#include "third_party/skia/include/effects/SkLayerRasterizer.h"
#include "ui/gfx/rect_conversions.h"
#include "ui/gfx/skia_util.h"
#include "ui/gfx/transform.h"

namespace cc {

namespace {

static inline bool IsScalarNearlyInteger(SkScalar scalar) {
  return SkScalarNearlyZero(scalar - SkScalarRound(scalar));
}

bool IsScaleAndIntegerTranslate(const SkMatrix& matrix) {
  return IsScalarNearlyInteger(matrix[SkMatrix::kMTransX]) &&
         IsScalarNearlyInteger(matrix[SkMatrix::kMTransY]) &&
         SkScalarNearlyZero(matrix[SkMatrix::kMSkewX]) &&
         SkScalarNearlyZero(matrix[SkMatrix::kMSkewY]) &&
         SkScalarNearlyZero(matrix[SkMatrix::kMPersp0]) &&
         SkScalarNearlyZero(matrix[SkMatrix::kMPersp1]) &&
         SkScalarNearlyZero(matrix[SkMatrix::kMPersp2] - 1.0f);
}

}  // anonymous namespace

scoped_ptr<SoftwareRenderer> SoftwareRenderer::Create(
    RendererClient* client,
    OutputSurface* output_surface,
    ResourceProvider* resource_provider) {
  return make_scoped_ptr(
      new SoftwareRenderer(client, output_surface, resource_provider));
}

SoftwareRenderer::SoftwareRenderer(RendererClient* client,
                                   OutputSurface* output_surface,
                                   ResourceProvider* resource_provider)
  : DirectRenderer(client, output_surface, resource_provider),
    visible_(true),
    is_scissor_enabled_(false),
    output_device_(output_surface->software_device()),
    current_canvas_(NULL) {
  if (resource_provider_) {
    capabilities_.max_texture_size = resource_provider_->max_texture_size();
    capabilities_.best_texture_format =
        resource_provider_->best_texture_format();
  }
  capabilities_.using_set_visibility = true;
  // The updater can access bitmaps while the SoftwareRenderer is using them.
  capabilities_.allow_partial_texture_updates = true;
  capabilities_.using_partial_swap = true;

  capabilities_.using_map_image =
      Settings().use_map_image &&
      output_surface->capabilities().deferred_gl_initialization;
}

SoftwareRenderer::~SoftwareRenderer() {}

const RendererCapabilities& SoftwareRenderer::Capabilities() const {
  return capabilities_;
}

void SoftwareRenderer::BeginDrawingFrame(DrawingFrame* frame) {
  TRACE_EVENT0("cc", "SoftwareRenderer::BeginDrawingFrame");
  root_canvas_ = output_device_->BeginPaint(
      gfx::ToEnclosingRect(frame->root_damage_rect));
}

void SoftwareRenderer::FinishDrawingFrame(DrawingFrame* frame) {
  TRACE_EVENT0("cc", "SoftwareRenderer::FinishDrawingFrame");
  current_framebuffer_lock_.reset();
  current_canvas_ = NULL;
  root_canvas_ = NULL;

  current_frame_data_.reset(new SoftwareFrameData);
  output_device_->EndPaint(current_frame_data_.get());
}

void SoftwareRenderer::SwapBuffers() {
  CompositorFrame compositor_frame;
  compositor_frame.metadata = client_->MakeCompositorFrameMetadata();
  compositor_frame.software_frame_data = current_frame_data_.Pass();
  output_surface_->SwapBuffers(&compositor_frame);
}

void SoftwareRenderer::ReceiveSwapBuffersAck(const CompositorFrameAck& ack) {
  output_device_->ReclaimSoftwareFrame(ack.last_software_frame_id);
}

bool SoftwareRenderer::FlippedFramebuffer() const {
  return false;
}

void SoftwareRenderer::EnsureScissorTestEnabled() {
  is_scissor_enabled_ = true;
  SetClipRect(scissor_rect_);
}

void SoftwareRenderer::EnsureScissorTestDisabled() {
  // There is no explicit notion of enabling/disabling scissoring in software
  // rendering, but the underlying effect we want is to clear any existing
  // clipRect on the current SkCanvas. This is done by setting clipRect to
  // the viewport's dimensions.
  is_scissor_enabled_ = false;
  SkDevice* device = current_canvas_->getDevice();
  SetClipRect(gfx::Rect(device->width(), device->height()));
}

void SoftwareRenderer::Finish() {}

void SoftwareRenderer::BindFramebufferToOutputSurface(DrawingFrame* frame) {
  current_framebuffer_lock_.reset();
  current_canvas_ = root_canvas_;
}

bool SoftwareRenderer::BindFramebufferToTexture(
    DrawingFrame* frame,
    const ScopedResource* texture,
    gfx::Rect target_rect) {
  current_framebuffer_lock_.reset();
  current_framebuffer_lock_ = make_scoped_ptr(
      new ResourceProvider::ScopedWriteLockSoftware(
          resource_provider_, texture->id()));
  current_canvas_ = current_framebuffer_lock_->sk_canvas();
  InitializeViewport(frame,
                     target_rect,
                     gfx::Rect(target_rect.size()),
                     target_rect.size());
  return true;
}

void SoftwareRenderer::SetScissorTestRect(gfx::Rect scissor_rect) {
  is_scissor_enabled_ = true;
  scissor_rect_ = scissor_rect;
  SetClipRect(scissor_rect);
}

void SoftwareRenderer::SetClipRect(gfx::Rect rect) {
  // Skia applies the current matrix to clip rects so we reset it temporary.
  SkMatrix current_matrix = current_canvas_->getTotalMatrix();
  current_canvas_->resetMatrix();
  current_canvas_->clipRect(gfx::RectToSkRect(rect), SkRegion::kReplace_Op);
  current_canvas_->setMatrix(current_matrix);
}

void SoftwareRenderer::ClearCanvas(SkColor color) {
  // SkCanvas::clear doesn't respect the current clipping region
  // so we SkCanvas::drawColor instead if scissoring is active.
  if (is_scissor_enabled_)
    current_canvas_->drawColor(color, SkXfermode::kSrc_Mode);
  else
    current_canvas_->clear(color);
}

void SoftwareRenderer::ClearFramebuffer(DrawingFrame* frame) {
  if (frame->current_render_pass->has_transparent_background) {
    ClearCanvas(SkColorSetARGB(0, 0, 0, 0));
  } else {
#ifndef NDEBUG
    // On DEBUG builds, opaque render passes are cleared to blue
    // to easily see regions that were not drawn on the screen.
    ClearCanvas(SkColorSetARGB(255, 0, 0, 255));
#endif
  }
}

void SoftwareRenderer::SetDrawViewport(gfx::Rect window_space_viewport) {}

bool SoftwareRenderer::IsSoftwareResource(
    ResourceProvider::ResourceId resource_id) const {
  switch (resource_provider_->GetResourceType(resource_id)) {
    case ResourceProvider::GLTexture:
      return false;
    case ResourceProvider::Bitmap:
      return true;
  }

  LOG(FATAL) << "Invalid resource type.";
  return false;
}

void SoftwareRenderer::DoDrawQuad(DrawingFrame* frame, const DrawQuad* quad) {
  TRACE_EVENT0("cc", "SoftwareRenderer::DoDrawQuad");
  gfx::Transform quad_rect_matrix;
  QuadRectTransform(&quad_rect_matrix, quad->quadTransform(), quad->rect);
  gfx::Transform contents_device_transform =
      frame->window_matrix * frame->projection_matrix * quad_rect_matrix;
  contents_device_transform.FlattenTo2d();
  SkMatrix sk_device_matrix;
  gfx::TransformToFlattenedSkMatrix(contents_device_transform,
                                    &sk_device_matrix);
  current_canvas_->setMatrix(sk_device_matrix);

  current_paint_.reset();
  if (!IsScaleAndIntegerTranslate(sk_device_matrix)) {
    // TODO(danakj): Until we can enable AA only on exterior edges of the
    // layer, disable AA if any interior edges are present. crbug.com/248175
    bool all_four_edges_are_exterior = quad->IsTopEdge() &&
                                       quad->IsLeftEdge() &&
                                       quad->IsBottomEdge() &&
                                       quad->IsRightEdge();
    if (all_four_edges_are_exterior)
      current_paint_.setAntiAlias(true);
    current_paint_.setFilterBitmap(true);
  }

  if (quad->ShouldDrawWithBlending()) {
    current_paint_.setAlpha(quad->opacity() * 255);
    current_paint_.setXfermodeMode(SkXfermode::kSrcOver_Mode);
  } else {
    current_paint_.setXfermodeMode(SkXfermode::kSrc_Mode);
  }

  switch (quad->material) {
    case DrawQuad::CHECKERBOARD:
      DrawCheckerboardQuad(frame, CheckerboardDrawQuad::MaterialCast(quad));
      break;
    case DrawQuad::DEBUG_BORDER:
      DrawDebugBorderQuad(frame, DebugBorderDrawQuad::MaterialCast(quad));
      break;
    case DrawQuad::PICTURE_CONTENT:
      DrawPictureQuad(frame, PictureDrawQuad::MaterialCast(quad));
      break;
    case DrawQuad::RENDER_PASS:
      DrawRenderPassQuad(frame, RenderPassDrawQuad::MaterialCast(quad));
      break;
    case DrawQuad::SOLID_COLOR:
      DrawSolidColorQuad(frame, SolidColorDrawQuad::MaterialCast(quad));
      break;
    case DrawQuad::TEXTURE_CONTENT:
      DrawTextureQuad(frame, TextureDrawQuad::MaterialCast(quad));
      break;
    case DrawQuad::TILED_CONTENT:
      DrawTileQuad(frame, TileDrawQuad::MaterialCast(quad));
      break;
    case DrawQuad::INVALID:
    case DrawQuad::IO_SURFACE_CONTENT:
    case DrawQuad::YUV_VIDEO_CONTENT:
    case DrawQuad::STREAM_VIDEO_CONTENT:
      DrawUnsupportedQuad(frame, quad);
      NOTREACHED();
      break;
  }

  current_canvas_->resetMatrix();
}

void SoftwareRenderer::DrawCheckerboardQuad(const DrawingFrame* frame,
                                            const CheckerboardDrawQuad* quad) {
  current_paint_.setColor(quad->color);
  current_paint_.setAlpha(quad->opacity() * SkColorGetA(quad->color));
  current_canvas_->drawRect(gfx::RectFToSkRect(QuadVertexRect()),
                            current_paint_);
}

void SoftwareRenderer::DrawDebugBorderQuad(const DrawingFrame* frame,
                                           const DebugBorderDrawQuad* quad) {
  // We need to apply the matrix manually to have pixel-sized stroke width.
  SkPoint vertices[4];
  gfx::RectFToSkRect(QuadVertexRect()).toQuad(vertices);
  SkPoint transformed_vertices[4];
  current_canvas_->getTotalMatrix().mapPoints(transformed_vertices,
                                              vertices,
                                              4);
  current_canvas_->resetMatrix();

  current_paint_.setColor(quad->color);
  current_paint_.setAlpha(quad->opacity() * SkColorGetA(quad->color));
  current_paint_.setStyle(SkPaint::kStroke_Style);
  current_paint_.setStrokeWidth(quad->width);
  current_canvas_->drawPoints(SkCanvas::kPolygon_PointMode,
                              4, transformed_vertices, current_paint_);
}

void SoftwareRenderer::DrawPictureQuad(const DrawingFrame* frame,
                                       const PictureDrawQuad* quad) {
  SkMatrix content_matrix;
  content_matrix.setRectToRect(
      gfx::RectFToSkRect(quad->tex_coord_rect),
      gfx::RectFToSkRect(QuadVertexRect()),
      SkMatrix::kFill_ScaleToFit);
  current_canvas_->concat(content_matrix);

  if (quad->ShouldDrawWithBlending()) {
    TRACE_EVENT0("cc", "SoftwareRenderer::DrawPictureQuad with blending");
    SkBitmap temp_bitmap;
    temp_bitmap.setConfig(SkBitmap::kARGB_8888_Config,
                          quad->texture_size.width(),
                          quad->texture_size.height());
    temp_bitmap.allocPixels();
    SkDevice temp_device(temp_bitmap);
    SkCanvas temp_canvas(&temp_device);

    quad->picture_pile->RasterToBitmap(
        &temp_canvas, quad->content_rect, quad->contents_scale, NULL);

    current_paint_.setFilterBitmap(true);
    current_canvas_->drawBitmap(temp_bitmap, 0, 0, &current_paint_);
  } else {
    TRACE_EVENT0("cc",
                 "SoftwareRenderer::DrawPictureQuad direct from PicturePile");
    quad->picture_pile->RasterDirect(
        current_canvas_, quad->content_rect, quad->contents_scale, NULL);
  }
}

void SoftwareRenderer::DrawSolidColorQuad(const DrawingFrame* frame,
                                          const SolidColorDrawQuad* quad) {
  current_paint_.setColor(quad->color);
  current_paint_.setAlpha(quad->opacity() * SkColorGetA(quad->color));
  current_canvas_->drawRect(gfx::RectFToSkRect(QuadVertexRect()),
                            current_paint_);
}

void SoftwareRenderer::DrawTextureQuad(const DrawingFrame* frame,
                                       const TextureDrawQuad* quad) {
  if (!IsSoftwareResource(quad->resource_id)) {
    DrawUnsupportedQuad(frame, quad);
    return;
  }

  // FIXME: Add support for non-premultiplied alpha.
  ResourceProvider::ScopedReadLockSoftware lock(resource_provider_,
                                                quad->resource_id);
  const SkBitmap* bitmap = lock.sk_bitmap();
  gfx::RectF uv_rect = gfx::ScaleRect(gfx::BoundingRect(quad->uv_top_left,
                                                        quad->uv_bottom_right),
                                      bitmap->width(),
                                      bitmap->height());
  SkRect sk_uv_rect = gfx::RectFToSkRect(uv_rect);
  if (quad->flipped)
    current_canvas_->scale(1, -1);
  current_canvas_->drawBitmapRectToRect(*bitmap, &sk_uv_rect,
                                        gfx::RectFToSkRect(QuadVertexRect()),
                                        &current_paint_);
}

void SoftwareRenderer::DrawTileQuad(const DrawingFrame* frame,
                                    const TileDrawQuad* quad) {
  DCHECK(!output_surface_->ForcedDrawToSoftwareDevice());
  DCHECK(IsSoftwareResource(quad->resource_id));
  ResourceProvider::ScopedReadLockSoftware lock(resource_provider_,
                                                quad->resource_id);

  SkRect uv_rect = gfx::RectFToSkRect(quad->tex_coord_rect);
  current_paint_.setFilterBitmap(true);
  current_canvas_->drawBitmapRectToRect(*lock.sk_bitmap(), &uv_rect,
                                        gfx::RectFToSkRect(QuadVertexRect()),
                                        &current_paint_);
}

void SoftwareRenderer::DrawRenderPassQuad(const DrawingFrame* frame,
                                          const RenderPassDrawQuad* quad) {
  CachedResource* content_texture =
      render_pass_textures_.get(quad->render_pass_id);
  if (!content_texture || !content_texture->id())
    return;

  DCHECK(IsSoftwareResource(content_texture->id()));
  ResourceProvider::ScopedReadLockSoftware lock(resource_provider_,
                                                content_texture->id());

  SkRect dest_rect = gfx::RectFToSkRect(QuadVertexRect());
  SkRect content_rect = SkRect::MakeWH(quad->rect.width(), quad->rect.height());

  SkMatrix content_mat;
  content_mat.setRectToRect(content_rect, dest_rect,
                            SkMatrix::kFill_ScaleToFit);

  const SkBitmap* content = lock.sk_bitmap();
  skia::RefPtr<SkShader> shader = skia::AdoptRef(
      SkShader::CreateBitmapShader(*content,
                                   SkShader::kClamp_TileMode,
                                   SkShader::kClamp_TileMode));
  shader->setLocalMatrix(content_mat);
  current_paint_.setShader(shader.get());

  SkImageFilter* filter = quad->filter.get();
  if (filter)
    current_paint_.setImageFilter(filter);

  if (quad->mask_resource_id) {
    ResourceProvider::ScopedReadLockSoftware mask_lock(resource_provider_,
                                                       quad->mask_resource_id);

    const SkBitmap* mask = mask_lock.sk_bitmap();

    SkRect mask_rect = SkRect::MakeXYWH(
        quad->mask_uv_rect.x() * mask->width(),
        quad->mask_uv_rect.y() * mask->height(),
        quad->mask_uv_rect.width() * mask->width(),
        quad->mask_uv_rect.height() * mask->height());

    SkMatrix mask_mat;
    mask_mat.setRectToRect(mask_rect, dest_rect, SkMatrix::kFill_ScaleToFit);

    skia::RefPtr<SkShader> mask_shader = skia::AdoptRef(
        SkShader::CreateBitmapShader(*mask,
                                     SkShader::kClamp_TileMode,
                                     SkShader::kClamp_TileMode));
    mask_shader->setLocalMatrix(mask_mat);

    SkPaint mask_paint;
    mask_paint.setShader(mask_shader.get());

    skia::RefPtr<SkLayerRasterizer> mask_rasterizer =
        skia::AdoptRef(new SkLayerRasterizer);
    mask_rasterizer->addLayer(mask_paint);

    current_paint_.setRasterizer(mask_rasterizer.get());
    current_canvas_->drawRect(dest_rect, current_paint_);
  } else {
    // FIXME: Apply background filters and blend with content
    current_canvas_->drawRect(dest_rect, current_paint_);
  }
}

void SoftwareRenderer::DrawUnsupportedQuad(const DrawingFrame* frame,
                                           const DrawQuad* quad) {
#ifdef NDEBUG
  current_paint_.setColor(SK_ColorWHITE);
#else
  current_paint_.setColor(SK_ColorMAGENTA);
#endif
  current_paint_.setAlpha(quad->opacity() * 255);
  current_canvas_->drawRect(gfx::RectFToSkRect(QuadVertexRect()),
                            current_paint_);
}

void SoftwareRenderer::CopyCurrentRenderPassToBitmap(
    DrawingFrame* frame,
    scoped_ptr<CopyOutputRequest> request) {
  scoped_ptr<SkBitmap> bitmap(new SkBitmap);
  bitmap->setConfig(SkBitmap::kARGB_8888_Config,
                    current_viewport_rect_.width(),
                    current_viewport_rect_.height());
  current_canvas_->readPixels(
      bitmap.get(), current_viewport_rect_.x(), current_viewport_rect_.y());

  request->SendBitmapResult(bitmap.Pass());
}

void SoftwareRenderer::GetFramebufferPixels(void* pixels, gfx::Rect rect) {
  TRACE_EVENT0("cc", "SoftwareRenderer::GetFramebufferPixels");
  SkBitmap subset_bitmap;
  rect += current_viewport_rect_.OffsetFromOrigin();
  output_device_->CopyToBitmap(rect, &subset_bitmap);
  subset_bitmap.copyPixelsTo(pixels,
                             4 * rect.width() * rect.height(),
                             4 * rect.width());
}

void SoftwareRenderer::SetVisible(bool visible) {
  if (visible_ == visible)
    return;
  visible_ = visible;
}

}  // namespace cc
