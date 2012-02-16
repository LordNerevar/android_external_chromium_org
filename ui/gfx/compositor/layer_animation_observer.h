// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_COMPOSITOR_LAYER_ANIMATION_OBSERVER_H_
#define UI_GFX_COMPOSITOR_LAYER_ANIMATION_OBSERVER_H_
#pragma once

#include <set>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/gfx/compositor/compositor_export.h"

namespace ui {

class LayerAnimationSequence;
class ScopedLayerAnimationSettings;
class ImplicitAnimationObserver;

// LayerAnimationObservers are notified when animations complete.
class COMPOSITOR_EXPORT LayerAnimationObserver  {
 public:
  // Called when the |sequence| ends. Not called if |sequence| is aborted.
  virtual void OnLayerAnimationEnded(
      LayerAnimationSequence* sequence) = 0;

  // Called if |sequence| is aborted for any reason. Should never do anything
  // that may cause another animation to be started.
  virtual void OnLayerAnimationAborted(
      LayerAnimationSequence* sequence) = 0;

  // Called when the animation is scheduled.
  virtual void OnLayerAnimationScheduled(
      LayerAnimationSequence* sequence) = 0;

 protected:
  typedef std::set<LayerAnimationSequence*> AttachedSequences;

  LayerAnimationObserver();
  virtual ~LayerAnimationObserver();

  // If the animator is destroyed during an animation, the animations are
  // aborted. The resulting NotifyAborted notifications will NOT be sent to
  // this observer if this function returns false. NOTE: IF YOU OVERRIDE THIS
  // FUNCTION TO RETURN TRUE, YOU MUST REMEMBER TO REMOVE YOURSELF AS AN
  // OBSERVER WHEN YOU ARE DESTROYED.
  virtual bool RequiresNotificationWhenAnimatorDestroyed() const;

  // Called when |this| is added to |sequence|'s observer list.
  virtual void OnAttachedToSequence(LayerAnimationSequence* sequence);

  // Called when |this| is removed to |sequence|'s observer list.
  virtual void OnDetachedFromSequence(LayerAnimationSequence* sequence);

  // Detaches this observer from all sequences it is currently observing.
  void StopObserving();

  const AttachedSequences& attached_sequences() const {
    return attached_sequences_;
  }

 private:
  friend class LayerAnimationSequence;

  // Called when |this| is added to |sequence|'s observer list.
  void AttachedToSequence(LayerAnimationSequence* sequence);

  // Called when |this| is removed to |sequence|'s observer list.
  // This will only result in notifications if |send_notification| is true.
  void DetachedFromSequence(LayerAnimationSequence* sequence,
                            bool send_notification);

  AttachedSequences attached_sequences_;
};

// An implicit animation observer is intended to be used in conjunction with a
// ScopedLayerAnimationSettings object in order to receive a notification when
// all implicit animations complete.
class COMPOSITOR_EXPORT ImplicitAnimationObserver
    : public LayerAnimationObserver {
 public:
  ImplicitAnimationObserver();
  virtual ~ImplicitAnimationObserver();

  virtual void OnImplicitAnimationsCompleted() = 0;

 protected:
  // Deactivates the observer and clears the collection of animations it is
  // waiting for.
  void StopObservingImplicitAnimations();

 private:
  friend class ScopedLayerAnimationSettings;

  // LayerAnimationObserver implementation
  virtual void OnLayerAnimationEnded(
      LayerAnimationSequence* sequence) OVERRIDE;
  virtual void OnLayerAnimationAborted(
      LayerAnimationSequence* sequence) OVERRIDE;
  virtual void OnLayerAnimationScheduled(
      LayerAnimationSequence* sequence) OVERRIDE;
  virtual void OnAttachedToSequence(
      LayerAnimationSequence* sequence) OVERRIDE;
  virtual void OnDetachedFromSequence(
      LayerAnimationSequence* sequence) OVERRIDE;

  // OnImplicitAnimationsCompleted is not fired unless the observer is active.
  bool active() const { return active_; }
  void SetActive(bool active);

  void CheckCompleted();

  bool active_;
};

}  // namespace ui

#endif  // UI_GFX_COMPOSITOR_LAYER_ANIMATION_OBSERVER_H_
