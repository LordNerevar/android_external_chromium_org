// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CCActiveAnimation_h
#define CCActiveAnimation_h

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"

namespace cc {

class CCAnimationCurve;

// A CCActiveAnimation, contains all the state required to play a CCAnimationCurve.
// Specifically, the affected property, the run state (paused, finished, etc.),
// loop count, last pause time, and the total time spent paused.
class CCActiveAnimation {
public:
    // Animations begin in one of the 'waiting' states. Animations waiting for the next tick
    // will start the next time the controller animates. Animations waiting for target
    // availibility will run as soon as their target property is free (and all the animations
    // animating with it are also able to run). Animations waiting for their start time to
    // come have be scheduled to run at a particular point in time. When this time arrives,
    // the controller will move the animations into the Running state. Running animations
    // may toggle between Running and Paused, and may be stopped by moving into either the
    // Aborted or Finished states. A Finished animation was allowed to run to completion, but
    // an Aborted animation was not.
    enum RunState {
        WaitingForNextTick = 0,
        WaitingForTargetAvailability,
        WaitingForStartTime,
        WaitingForDeletion,
        Running,
        Paused,
        Finished,
        Aborted,
        // This sentinel must be last.
        RunStateEnumSize
    };

    enum TargetProperty {
        Transform = 0,
        Opacity,
        // This sentinel must be last.
        TargetPropertyEnumSize
    };

    static scoped_ptr<CCActiveAnimation> create(scoped_ptr<CCAnimationCurve>, int animationId, int groupId, TargetProperty);

    virtual ~CCActiveAnimation();

    int id() const { return m_id; }
    int group() const { return m_group; }
    TargetProperty targetProperty() const { return m_targetProperty; }

    RunState runState() const { return m_runState; }
    void setRunState(RunState, double monotonicTime);

    // This is the number of times that the animation will play. If this
    // value is zero the animation will not play. If it is negative, then
    // the animation will loop indefinitely.
    int iterations() const { return m_iterations; }
    void setIterations(int n) { m_iterations = n; }

    double startTime() const { return m_startTime; }
    void setStartTime(double monotonicTime) { m_startTime = monotonicTime; }
    bool hasSetStartTime() const { return m_startTime; }

    double timeOffset() const { return m_timeOffset; }
    void setTimeOffset(double monotonicTime) { m_timeOffset = monotonicTime; }

    void suspend(double monotonicTime);
    void resume(double monotonicTime);

    // If alternatesDirection is true, on odd numbered iterations we reverse the curve.
    bool alternatesDirection() const { return m_alternatesDirection; }
    void setAlternatesDirection(bool alternates) { m_alternatesDirection = alternates; }

    bool isFinishedAt(double monotonicTime) const;
    bool isFinished() const { return m_runState == Finished
                                  || m_runState == Aborted
                                  || m_runState == WaitingForDeletion; }

    CCAnimationCurve* curve() { return m_curve.get(); }
    const CCAnimationCurve* curve() const { return m_curve.get(); }

    // If this is true, even if the animation is running, it will not be tickable until
    // it is given a start time. This is true for animations running on the main thread.
    bool needsSynchronizedStartTime() const { return m_needsSynchronizedStartTime; }
    void setNeedsSynchronizedStartTime(bool needsSynchronizedStartTime) { m_needsSynchronizedStartTime = needsSynchronizedStartTime; }

    // Takes the given absolute time, and using the start time and the number
    // of iterations, returns the relative time in the current iteration.
    double trimTimeToCurrentIteration(double monotonicTime) const;

    enum InstanceType {
        ControllingInstance = 0,
        NonControllingInstance
    };

    scoped_ptr<CCActiveAnimation> clone(InstanceType) const;
    scoped_ptr<CCActiveAnimation> cloneAndInitialize(InstanceType, RunState initialRunState, double startTime) const;
    bool isControllingInstance() const { return m_isControllingInstance; }

    void pushPropertiesTo(CCActiveAnimation*) const;

private:
    CCActiveAnimation(scoped_ptr<CCAnimationCurve>, int animationId, int groupId, TargetProperty);

    scoped_ptr<CCAnimationCurve> m_curve;

    // IDs are not necessarily unique.
    int m_id;

    // Animations that must be run together are called 'grouped' and have the same group id
    // Grouped animations are guaranteed to start at the same time and no other animations
    // may animate any of the group's target properties until all animations in the
    // group have finished animating. Note: an active animation's group id and target
    // property uniquely identify that animation.
    int m_group;

    TargetProperty m_targetProperty;
    RunState m_runState;
    int m_iterations;
    double m_startTime;
    bool m_alternatesDirection;

    // The time offset effectively pushes the start of the animation back in time. This is
    // used for resuming paused animations -- an animation is added with a non-zero time
    // offset, causing the animation to skip ahead to the desired point in time.
    double m_timeOffset;

    bool m_needsSynchronizedStartTime;

    // When an animation is suspended, it behaves as if it is paused and it also ignores
    // all run state changes until it is resumed. This is used for testing purposes.
    bool m_suspended;

    // These are used in trimTimeToCurrentIteration to account for time
    // spent while paused. This is not included in AnimationState since it
    // there is absolutely no need for clients of this controller to know
    // about these values.
    double m_pauseTime;
    double m_totalPausedTime;

    // Animations lead dual lives. An active animation will be conceptually owned by
    // two controllers, one on the impl thread and one on the main. In reality, there
    // will be two separate CCActiveAnimation instances for the same animation. They
    // will have the same group id and the same target property (these two values
    // uniquely identify an animation). The instance on the impl thread is the instance
    // that ultimately controls the values of the animating layer and so we will refer
    // to it as the 'controlling instance'.
    bool m_isControllingInstance;

    DISALLOW_COPY_AND_ASSIGN(CCActiveAnimation);
};

} // namespace cc

#endif // CCActiveAnimation_h
