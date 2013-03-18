// Copyright 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_PRIORITY_CALCULATOR_H_
#define CC_PRIORITY_CALCULATOR_H_

#include "cc/base/cc_export.h"

namespace gfx { class Rect; }

namespace cc {

class CC_EXPORT PriorityCalculator {
 public:
  static int UIPriority(bool draws_to_root_surface);
  static int VisiblePriority(bool draws_to_root_surface);
  static int RenderSurfacePriority();
  static int LingeringPriority(int previous_priority);
  static int PriorityFromDistance(gfx::Rect visible_rect,
                                  gfx::Rect texture_rect,
                                  bool draws_to_root_surface);
  static int SmallAnimatedLayerMinPriority();

  static int HighestPriority();
  static int LowestPriority();
  static inline bool priority_is_lower(int a, int b) { return a > b; }
  static inline bool priority_is_higher(int a, int b) { return a < b; }
  static inline int max_priority(int a, int b) {
    return priority_is_higher(a, b) ? a : b;
  }

  static int AllowNothingCutoff();
  static int AllowVisibleOnlyCutoff();
  static int AllowVisibleAndNearbyCutoff();
  static int AllowEverythingCutoff();
};

}

#endif  // CC_PRIORITY_CALCULATOR_H_
