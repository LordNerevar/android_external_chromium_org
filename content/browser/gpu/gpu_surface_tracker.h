// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_GPU_GPU_SURFACE_TRACKER_H_
#define CONTENT_BROWSER_GPU_GPU_SURFACE_TRACKER_H_

#include <map>

#include "base/basictypes.h"
#include "base/memory/singleton.h"
#include "base/synchronization/lock.h"
#include "content/common/gpu/gpu_surface_lookup.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/size.h"

namespace content {

// This class is responsible for managing rendering surfaces exposed to the
// GPU process. Every surface gets registered to this class, and gets an ID.
// All calls to and from the GPU process, with the exception of
// CreateViewCommandBuffer, refer to the rendering surface by its ID.
// This class is thread safe.
//
// Note: The ID can exist before the actual native handle for the surface is
// created, for example to allow giving a reference to it to a renderer, so that
// it is unamibiguously identified.
class GpuSurfaceTracker : public GpuSurfaceLookup {
 public:
  // GpuSurfaceLookup implementation:
  // Returns the native widget associated with a given surface_id.
  virtual gfx::AcceleratedWidget AcquireNativeWidget(int surface_id) OVERRIDE;

  // Gets the global instance of the surface tracker.
  static GpuSurfaceTracker* Get() { return GetInstance(); }

  // Adds a surface for a given RenderWidgetHost. |renderer_id| is the renderer
  // process ID, |render_widget_id| is the RenderWidgetHost route id within that
  // renderer. Returns the surface ID.
  int AddSurfaceForRenderer(int renderer_id, int render_widget_id);

  // Looks up a surface for a given RenderWidgetHost. Returns the surface
  // ID, or 0 if not found.
  // Note: This is an O(N) lookup.
  int LookupSurfaceForRenderer(int renderer_id, int render_widget_id);

  // Adds a surface for a native widget. Returns the surface ID.
  int AddSurfaceForNativeWidget(gfx::AcceleratedWidget widget);

  // Removes a given existing surface.
  void RemoveSurface(int surface_id);

  // Gets the renderer process ID and RenderWidgetHost route id for a given
  // surface, returning true if the surface is found (and corresponds to a
  // RenderWidgetHost), or false if not.
  bool GetRenderWidgetIDForSurface(int surface_id,
                                   int* renderer_id,
                                   int* render_widget_id);

  // Sets the native handle for the given surface.
  // Note: This is an O(log N) lookup.
  void SetSurfaceHandle(int surface_id, const gfx::GLSurfaceHandle& handle);

  // Sets the native widget associated with the surface_id.
  void SetNativeWidget(int surface_id, gfx::AcceleratedWidget widget);

  // Gets the native handle for the given surface.
  // Note: This is an O(log N) lookup.
  gfx::GLSurfaceHandle GetSurfaceHandle(int surface_id);

  // Returns the number of surfaces currently registered with the tracker.
  std::size_t GetSurfaceCount();

  // Gets the global instance of the surface tracker. Identical to Get(), but
  // named that way for the implementation of Singleton.
  static GpuSurfaceTracker* GetInstance();

 private:
  struct SurfaceInfo {
    int renderer_id;
    int render_widget_id;
    gfx::AcceleratedWidget native_widget;
    gfx::GLSurfaceHandle handle;
  };
  typedef std::map<int, SurfaceInfo> SurfaceMap;

  friend struct DefaultSingletonTraits<GpuSurfaceTracker>;

  GpuSurfaceTracker();
  virtual ~GpuSurfaceTracker();

  base::Lock lock_;
  SurfaceMap surface_map_;
  int next_surface_id_;

  DISALLOW_COPY_AND_ASSIGN(GpuSurfaceTracker);
};

}  // namespace content

#endif  // CONTENT_BROWSER_GPU_GPU_SURFACE_TRACKER_H_
