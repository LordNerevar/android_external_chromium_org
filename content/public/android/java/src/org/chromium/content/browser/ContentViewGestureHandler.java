// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser;

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.SystemClock;
import android.util.Log;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.ViewConfiguration;

import org.chromium.content.browser.third_party.GestureDetector;
import org.chromium.content.browser.third_party.GestureDetector.OnGestureListener;
import org.chromium.content.browser.LongPressDetector.LongPressDelegate;
import org.chromium.content.browser.SnapScrollController;
import org.chromium.content.common.TraceEvent;

import java.util.ArrayDeque;
import java.util.Deque;

/**
 * This class handles all MotionEvent handling done in ContentViewCore including the gesture
 * recognition. It sends all related native calls through the interface MotionEventDelegate.
 */
class ContentViewGestureHandler implements LongPressDelegate {

    private static final String TAG = "ContentViewGestureHandler";
    /**
     * Used for GESTURE_FLING_START x velocity
     */
    static final String VELOCITY_X = "Velocity X";
    /**
     * Used for GESTURE_FLING_START y velocity
     */
    static final String VELOCITY_Y = "Velocity Y";
    /**
     * Used for GESTURE_SCROLL_BY x distance
     */
    static final String DISTANCE_X = "Distance X";
    /**
     * Used for GESTURE_SCROLL_BY y distance
     */
    static final String DISTANCE_Y = "Distance Y";
    /**
     * Used in GESTURE_SINGLE_TAP_CONFIRMED to check whether ShowPress has been called before.
     */
    static final String SHOW_PRESS = "ShowPress";
    /**
     * Used for GESTURE_PINCH_BY delta
     */
    static final String DELTA = "Delta";

    private final Bundle mExtraParamBundleSingleTap;
    private final Bundle mExtraParamBundleFling;
    private final Bundle mExtraParamBundleScroll;
    private final Bundle mExtraParamBundleDoubleTapDragZoom;
    private final Bundle mExtraParamBundlePinchBy;
    private GestureDetector mGestureDetector;
    private final ZoomManager mZoomManager;
    private LongPressDetector mLongPressDetector;
    private OnGestureListener mListener;
    private MotionEvent mCurrentDownEvent;
    private final MotionEventDelegate mMotionEventDelegate;

    // Queue of motion events.
    private final Deque<MotionEvent> mPendingMotionEvents = new ArrayDeque<MotionEvent>();

    // Has WebKit told us the current page requires touch events.
    private boolean mHasTouchHandlers = false;

    // True if the down event for the current gesture was returned back to the browser with
    // INPUT_EVENT_ACK_STATE_NO_CONSUMER_EXISTS
    private boolean mNoTouchHandlerForGesture = false;

    // True if JavaScript touch event handlers returned an ACK with
    // INPUT_EVENT_ACK_STATE_CONSUMED. In this case we should avoid, sending events from
    // this gesture to the Gesture Detector since it will have already missed at least
    // one event.
    private boolean mJavaScriptIsConsumingGesture = false;

    // Remember whether onShowPress() is called. If it is not, in onSingleTapConfirmed()
    // we will first show the press state, then trigger the click.
    private boolean mShowPressIsCalled;

    // This flag is used for ignoring the remaining touch events, i.e., All the events until the
    // next ACTION_DOWN. This is automatically set to false on the next ACTION_DOWN.
    private boolean mIgnoreRemainingTouchEvents;

    // TODO(klobag): this is to avoid a bug in GestureDetector. With multi-touch,
    // mAlwaysInTapRegion is not reset. So when the last finger is up, onSingleTapUp()
    // will be mistakenly fired.
    private boolean mIgnoreSingleTap;

    // True from right before we send the first scroll event until the last finger is raised.
    private boolean mTouchScrolling;

    // TODO(wangxianzhu): For now it is true after a fling is started until the next
    // touch. Should reset it to false on end of fling if the UI is able to know when the
    // fling ends.
    private boolean mFlingMayBeActive;

    private boolean mSeenFirstScrollEvent;

    private boolean mPinchInProgress = false;

    private static final int DOUBLE_TAP_TIMEOUT = ViewConfiguration.getDoubleTapTimeout();

    //On single tap this will store the x, y coordinates of the touch.
    private int mSingleTapX;
    private int mSingleTapY;

    // Indicate current double tap drag mode state.
    private int mDoubleTapDragMode = DOUBLE_TAP_DRAG_MODE_NONE;

    // x, y coordinates for an Anchor on double tap drag zoom.
    private float mDoubleTapDragZoomAnchorX;
    private float mDoubleTapDragZoomAnchorY;

    // On double tap this will store the y coordinates of the touch.
    private float mDoubleTapY;

    // Double tap drag zoom sensitive (speed).
    private static final float DOUBLE_TAP_DRAG_ZOOM_SPEED = 0.005f;

    // Used to track the last rawX/Y coordinates for moves.  This gives absolute scroll distance.
    // Useful for full screen tracking.
    private float mLastRawX = 0;
    private float mLastRawY = 0;

    // Cache of square of the scaled touch slop so we don't have to calculate it on every touch.
    private int mScaledTouchSlopSquare;

    // Object that keeps track of and updates scroll snapping behavior.
    private SnapScrollController mSnapScrollController;

    // Used to track the accumulated scroll error over time. This is used to remove the
    // rounding error we introduced by passing integers to webkit.
    private float mAccumulatedScrollErrorX = 0;
    private float mAccumulatedScrollErrorY = 0;

    // Whether input events are delivered right before vsync.
    private final boolean mInputEventsDeliveredAtVSync;

    // Certain touch-event generated gestures (PINCH_BY, SCROLL_BY) may trigger an expedited vsync
    // signal. Rather than sending the vsync immediately after such a gesture is sent, we
    // set this flag and defer the "last event for vsync" signal until the touch event is fully
    // processed. This allows multiple such gestures to be sent in a given frame.
    private boolean mSentGestureNeedsVSync;
    private long mLastVSyncGestureTimeMs;

    // Incremented and decremented when the methods onTouchEvent() and confirmTouchEvent() start
    // and finish execution, respectively. This provides accounting for synchronous calls to
    // confirmTouchEvent(), from either itself or onTouchEvent().
    private int mTouchEventHandlingStackDepth;

    // Keeps track of the last long press event, if we end up opening a context menu, we would need
    // to potentially use the event to send GESUTRE_SHOW_PRESS_CANCEL to remove ::active styling
    private MotionEvent mLastLongPressEvent;

    static final int GESTURE_SHOW_PRESSED_STATE = 0;
    static final int GESTURE_DOUBLE_TAP = 1;
    static final int GESTURE_SINGLE_TAP_UP = 2;
    static final int GESTURE_SINGLE_TAP_CONFIRMED = 3;
    static final int GESTURE_SINGLE_TAP_UNCONFIRMED = 4;
    static final int GESTURE_LONG_PRESS = 5;
    static final int GESTURE_SCROLL_START = 6;
    static final int GESTURE_SCROLL_BY = 7;
    static final int GESTURE_SCROLL_END = 8;
    static final int GESTURE_FLING_START = 9;
    static final int GESTURE_FLING_CANCEL = 10;
    static final int GESTURE_PINCH_BEGIN = 11;
    static final int GESTURE_PINCH_BY = 12;
    static final int GESTURE_PINCH_END = 13;
    static final int GESTURE_SHOW_PRESS_CANCEL = 14;
    static final int GESTURE_LONG_TAP = 15;

    // These have to be kept in sync with content/port/common/input_event_ack_state.h
    static final int INPUT_EVENT_ACK_STATE_UNKNOWN = 0;
    static final int INPUT_EVENT_ACK_STATE_CONSUMED = 1;
    static final int INPUT_EVENT_ACK_STATE_NOT_CONSUMED = 2;
    static final int INPUT_EVENT_ACK_STATE_NO_CONSUMER_EXISTS = 3;

    // Return values of sendPendingEventToNative();
    static final int EVENT_FORWARDED_TO_NATIVE = 0;
    static final int EVENT_CONVERTED_TO_CANCEL = 1;
    static final int EVENT_NOT_FORWARDED = 2;

    private final float mPxToDp;

    static final int DOUBLE_TAP_DRAG_MODE_NONE = 0;
    static final int DOUBLE_TAP_DRAG_MODE_DETECTION_IN_PROGRESS = 1;
    static final int DOUBLE_TAP_DRAG_MODE_ZOOM = 2;
    static final int DOUBLE_TAP_DRAG_MODE_DISABLED = 3;

    private class TouchEventTimeoutHandler implements Runnable {
        private static final int TOUCH_EVENT_TIMEOUT = 200;
        private static final int PENDING_ACK_NONE = 0;
        private static final int PENDING_ACK_ORIGINAL_EVENT = 1;
        private static final int PENDING_ACK_CANCEL_EVENT = 2;

        private long mEventTime;
        private TouchPoint[] mTouchPoints;
        private Handler mHandler = new Handler();
        private int mPendingAckState;

        public void start(long eventTime, TouchPoint[] pts) {
            assert mTouchPoints == null;
            assert mPendingAckState == PENDING_ACK_NONE;
            mEventTime = eventTime;
            mTouchPoints = pts;
            mHandler.postDelayed(this, TOUCH_EVENT_TIMEOUT);
        }

        @Override
        public void run() {
            TraceEvent.begin("TouchEventTimeout");
            while (!mPendingMotionEvents.isEmpty()) {
                MotionEvent nextEvent = mPendingMotionEvents.removeFirst();
                processTouchEvent(nextEvent);
                recycleEvent(nextEvent);
            }
            // We are waiting for 2 ACKs: one for the timed-out event, the other for
            // the touchcancel event injected when the timed-out event is ACK'ed.
            mPendingAckState = PENDING_ACK_ORIGINAL_EVENT;
            TraceEvent.end();
        }

        public boolean hasTimeoutEvent() {
            return mPendingAckState != PENDING_ACK_NONE;
        }

        /**
         * @return Whether the ACK is consumed in this method.
         */
        public boolean confirmTouchEvent() {
            switch (mPendingAckState) {
                case PENDING_ACK_NONE:
                    // The ACK to the original event is received before timeout.
                    mHandler.removeCallbacks(this);
                    mTouchPoints = null;
                    return false;
                case PENDING_ACK_ORIGINAL_EVENT:
                    TraceEvent.instant("TouchEventTimeout:ConfirmOriginalEvent");
                    // The ACK to the original event is received after timeout.
                    // Inject a touchcancel event.
                    mPendingAckState = PENDING_ACK_CANCEL_EVENT;
                    mMotionEventDelegate.sendTouchEvent(mEventTime + TOUCH_EVENT_TIMEOUT,
                            TouchPoint.TOUCH_EVENT_TYPE_CANCEL, mTouchPoints);
                    mTouchPoints = null;
                    return true;
                case PENDING_ACK_CANCEL_EVENT:
                    TraceEvent.instant("TouchEventTimeout:ConfirmCancelEvent");
                    // The ACK to the injected touchcancel event is received.
                    mPendingAckState = PENDING_ACK_NONE;
                    drainAllPendingEventsUntilNextDown();
                    return true;
                default:
                    assert false : "Never reached";
                    return false;
            }
        }

        public void mockTimeout() {
            assert !hasTimeoutEvent();
            mHandler.removeCallbacks(this);
            run();
        }

        /**
         * This is for testing only.
         * @return Whether a timeout event has been scheduled but not yet run.
         */
        public boolean hasScheduledTimeoutEventForTesting() {
            return mTouchPoints != null && mPendingAckState == PENDING_ACK_NONE;
        }
    }

    private TouchEventTimeoutHandler mTouchEventTimeoutHandler = new TouchEventTimeoutHandler();

    /**
     * This is an interface to handle MotionEvent related communication with the native side also
     * access some ContentView specific parameters.
     */
    public interface MotionEventDelegate {
        /**
         * Send a raw {@link MotionEvent} to the native side
         * @param timeMs Time of the event in ms.
         * @param action The action type for the event.
         * @param pts The TouchPoint array to be sent for the event.
         * @return Whether the event was sent to the native side successfully or not.
         */
        public boolean sendTouchEvent(long timeMs, int action, TouchPoint[] pts);

        /**
         * Send a gesture event to the native side.
         * @param type The type of the gesture event.
         * @param timeMs The time the gesture event occurred at.
         * @param x The x location for the gesture event.
         * @param y The y location for the gesture event.
         * @param extraParams A bundle that holds specific extra parameters for certain gestures.
         *                    This is read-only and should not be modified in this function.
         * Refer to gesture type definition for more information.
         * @return Whether the gesture was sent successfully.
         */
        boolean sendGesture(int type, long timeMs, int x, int y, Bundle extraParams);

        /**
         * Indicate delivery of the final gesture for the current vsync interval.
         * @param eventTimeMs Time of the last sent gesture event in ms.
         */
        void onSentLastGestureForVSync(long eventTimeMs);

        /**
         * Gives the UI the chance to override each scroll event.
         * @param x The amount scrolled in the X direction.
         * @param y The amount scrolled in the Y direction.
         * @return Whether or not the UI consumed and handled this event.
         */
        boolean didUIStealScroll(float x, float y);

        /**
         * Show the zoom picker UI.
         */
        public void invokeZoomPicker();

        /**
         * @return Whether changing the page scale is not possible on the current page.
         */
        public boolean hasFixedPageScale();
    }

    ContentViewGestureHandler(
            Context context, MotionEventDelegate delegate, ZoomManager zoomManager,
            int inputEventDeliveryMode) {
        mExtraParamBundleSingleTap = new Bundle();
        mExtraParamBundleFling = new Bundle();
        mExtraParamBundleScroll = new Bundle();
        mExtraParamBundleDoubleTapDragZoom = new Bundle();
        mExtraParamBundlePinchBy = new Bundle();

        mLongPressDetector = new LongPressDetector(context, this);
        mMotionEventDelegate = delegate;
        mZoomManager = zoomManager;
        mSnapScrollController = new SnapScrollController(context, mZoomManager);
        mInputEventsDeliveredAtVSync =
                inputEventDeliveryMode == ContentViewCore.INPUT_EVENTS_DELIVERED_AT_VSYNC;
        mPxToDp = 1.0f / context.getResources().getDisplayMetrics().density;

        initGestureDetectors(context);
    }

    /**
     * Used to override the default long press detector, gesture detector and listener.
     * This is used for testing only.
     * @param longPressDetector The new LongPressDetector to be assigned.
     * @param gestureDetector The new GestureDetector to be assigned.
     * @param listener The new onGestureListener to be assigned.
     */
    void setTestDependencies(
            LongPressDetector longPressDetector, GestureDetector gestureDetector,
            OnGestureListener listener) {
        if (longPressDetector != null) mLongPressDetector = longPressDetector;
        if (gestureDetector != null) mGestureDetector = gestureDetector;
        if (listener != null) mListener = listener;
    }

    private void initGestureDetectors(final Context context) {
        final int scaledTouchSlop = ViewConfiguration.get(context).getScaledTouchSlop();
        mScaledTouchSlopSquare = scaledTouchSlop * scaledTouchSlop;
        try {
            TraceEvent.begin();
            GestureDetector.SimpleOnGestureListener listener =
                new GestureDetector.SimpleOnGestureListener() {
                    @Override
                    public boolean onDown(MotionEvent e) {
                        mShowPressIsCalled = false;
                        mIgnoreSingleTap = false;
                        mTouchScrolling = false;
                        mSeenFirstScrollEvent = false;
                        mSnapScrollController.resetSnapScrollMode();
                        mLastRawX = e.getRawX();
                        mLastRawY = e.getRawY();
                        mAccumulatedScrollErrorX = 0;
                        mAccumulatedScrollErrorY = 0;
                        // Return true to indicate that we want to handle touch
                        return true;
                    }

                    @Override
                    public boolean onScroll(MotionEvent e1, MotionEvent e2,
                            float distanceX, float distanceY) {
                        if (!mSeenFirstScrollEvent) {
                            // Remove the touch slop region from the first scroll event to avoid a
                            // jump.
                            mSeenFirstScrollEvent = true;
                            double distance = Math.sqrt(
                                    distanceX * distanceX + distanceY * distanceY);
                            double epsilon = 1e-3;
                            if (distance > epsilon) {
                                double ratio = Math.max(0, distance - scaledTouchSlop) / distance;
                                distanceX *= ratio;
                                distanceY *= ratio;
                            }
                        }
                        mSnapScrollController.updateSnapScrollMode(distanceX, distanceY);
                        if (mSnapScrollController.isSnappingScrolls()) {
                            if (mSnapScrollController.isSnapHorizontal()) {
                                distanceY = 0;
                            } else {
                                distanceX = 0;
                            }
                        }

                        boolean didUIStealScroll = mMotionEventDelegate.didUIStealScroll(
                                e2.getRawX() - mLastRawX, e2.getRawY() - mLastRawY);

                        mLastRawX = e2.getRawX();
                        mLastRawY = e2.getRawY();
                        if (didUIStealScroll) return true;
                        if (!mTouchScrolling) {
                            sendShowPressCancelIfNecessary(e1);
                            endFlingIfNecessary(e2.getEventTime());
                            if (sendMotionEventAsGesture(GESTURE_SCROLL_START, e1, null)) {
                                mTouchScrolling = true;
                            }
                        }
                        // distanceX and distanceY is the scrolling offset since last onScroll.
                        // Because we are passing integers to webkit, this could introduce
                        // rounding errors. The rounding errors will accumulate overtime.
                        // To solve this, we should be adding back the rounding errors each time
                        // when we calculate the new offset.
                        int x = (int) e2.getX();
                        int y = (int) e2.getY();
                        int dx = (int) (distanceX + mAccumulatedScrollErrorX);
                        int dy = (int) (distanceY + mAccumulatedScrollErrorY);
                        mAccumulatedScrollErrorX = distanceX + mAccumulatedScrollErrorX - dx;
                        mAccumulatedScrollErrorY = distanceY + mAccumulatedScrollErrorY - dy;

                        mExtraParamBundleScroll.putInt(DISTANCE_X, dx);
                        mExtraParamBundleScroll.putInt(DISTANCE_Y, dy);
                        assert mExtraParamBundleScroll.size() == 2;

                        if ((dx | dy) != 0) {
                            sendGestureAndRequestVSync(GESTURE_SCROLL_BY,
                                    e2.getEventTime(), x, y, mExtraParamBundleScroll);
                        }

                        mMotionEventDelegate.invokeZoomPicker();

                        return true;
                    }

                    @Override
                    public boolean onFling(MotionEvent e1, MotionEvent e2,
                            float velocityX, float velocityY) {
                        if (mSnapScrollController.isSnappingScrolls()) {
                            if (mSnapScrollController.isSnapHorizontal()) {
                                velocityY = 0;
                            } else {
                                velocityX = 0;
                            }
                        }

                        fling(e1.getEventTime(),(int) e1.getX(0), (int) e1.getY(0),
                                        (int) velocityX, (int) velocityY);
                        return true;
                    }

                    @Override
                    public void onShowPress(MotionEvent e) {
                        mShowPressIsCalled = true;
                        sendMotionEventAsGesture(GESTURE_SHOW_PRESSED_STATE, e, null);
                    }

                    @Override
                    public boolean onSingleTapUp(MotionEvent e) {
                        if (isDistanceBetweenDownAndUpTooLong(e.getRawX(), e.getRawY())) {
                            mIgnoreSingleTap = true;
                            return true;
                        }
                        // This is a hack to address the issue where user hovers
                        // over a link for longer than DOUBLE_TAP_TIMEOUT, then
                        // onSingleTapConfirmed() is not triggered. But we still
                        // want to trigger the tap event at UP. So we override
                        // onSingleTapUp() in this case. This assumes singleTapUp
                        // gets always called before singleTapConfirmed.
                        if (!mIgnoreSingleTap && !mLongPressDetector.isInLongPress()) {
                            if (e.getEventTime() - e.getDownTime() > DOUBLE_TAP_TIMEOUT) {
                                float x = e.getX();
                                float y = e.getY();
                                if (sendMotionEventAsGesture(GESTURE_SINGLE_TAP_UP, e, null)) {
                                    mIgnoreSingleTap = true;
                                }
                                setClickXAndY((int) x, (int) y);
                                return true;
                            } else if (mMotionEventDelegate.hasFixedPageScale()) {
                                // If page is not user scalable, we don't need to wait
                                // for double tap timeout.
                                float x = e.getX();
                                float y = e.getY();
                                mExtraParamBundleSingleTap.putBoolean(SHOW_PRESS,
                                        mShowPressIsCalled);
                                assert mExtraParamBundleSingleTap.size() == 1;

                                if (sendMotionEventAsGesture(GESTURE_SINGLE_TAP_CONFIRMED, e,
                                        mExtraParamBundleSingleTap)) {
                                    mIgnoreSingleTap = true;
                                }
                                setClickXAndY((int) x, (int) y);
                            } else {
                                // Notify Blink about this tapUp event anyway,
                                // when none of the above conditions applied.
                                sendMotionEventAsGesture(GESTURE_SINGLE_TAP_UNCONFIRMED, e, null);
                            }
                        }

                        return triggerLongTapIfNeeded(e);
                    }

                    @Override
                    public boolean onSingleTapConfirmed(MotionEvent e) {
                        // Long taps in the edges of the screen have their events delayed by
                        // ContentViewHolder for tab swipe operations. As a consequence of the delay
                        // this method might be called after receiving the up event.
                        // These corner cases should be ignored.
                        if (mLongPressDetector.isInLongPress() || mIgnoreSingleTap) return true;

                        int x = (int) e.getX();
                        int y = (int) e.getY();
                        mExtraParamBundleSingleTap.putBoolean(SHOW_PRESS, mShowPressIsCalled);
                        sendMotionEventAsGesture(GESTURE_SINGLE_TAP_CONFIRMED, e,
                            mExtraParamBundleSingleTap);
                        assert mExtraParamBundleSingleTap.size() == 1;

                        setClickXAndY(x, y);
                        return true;
                    }

                    @Override
                    public boolean onDoubleTapEvent(MotionEvent e) {
                        if (isDoubleTapDragDisabled()) return false;
                        switch (e.getActionMasked()) {
                            case MotionEvent.ACTION_DOWN:
                                sendShowPressCancelIfNecessary(e);
                                mDoubleTapDragZoomAnchorX = e.getX();
                                mDoubleTapDragZoomAnchorY = e.getY();
                                mDoubleTapDragMode = DOUBLE_TAP_DRAG_MODE_DETECTION_IN_PROGRESS;
                                break;
                            case MotionEvent.ACTION_MOVE:
                                if (mDoubleTapDragMode
                                        == DOUBLE_TAP_DRAG_MODE_DETECTION_IN_PROGRESS) {
                                    float distanceX = mDoubleTapDragZoomAnchorX - e.getX();
                                    float distanceY = mDoubleTapDragZoomAnchorY - e.getY();

                                    // Begin double tap drag zoom mode if the move distance is
                                    // further than the threshold.
                                    if (distanceX * distanceX + distanceY * distanceY >
                                            mScaledTouchSlopSquare) {
                                        sendGesture(GESTURE_SCROLL_START, e.getEventTime(),
                                                (int) e.getX(), (int) e.getY(), null);
                                        pinchBegin(e.getEventTime(),
                                                Math.round(mDoubleTapDragZoomAnchorX),
                                                Math.round(mDoubleTapDragZoomAnchorY));
                                        mDoubleTapDragMode = DOUBLE_TAP_DRAG_MODE_ZOOM;
                                    }
                                } else if (mDoubleTapDragMode == DOUBLE_TAP_DRAG_MODE_ZOOM) {
                                    assert mExtraParamBundleDoubleTapDragZoom.isEmpty();
                                    sendGesture(GESTURE_SCROLL_BY, e.getEventTime(),
                                            (int) e.getX(), (int) e.getY(),
                                            mExtraParamBundleDoubleTapDragZoom);

                                    float dy = mDoubleTapY - e.getY();
                                    pinchBy(e.getEventTime(),
                                            Math.round(mDoubleTapDragZoomAnchorX),
                                            Math.round(mDoubleTapDragZoomAnchorY),
                                            (float) Math.pow(dy > 0 ?
                                                    1.0f - DOUBLE_TAP_DRAG_ZOOM_SPEED :
                                                    1.0f + DOUBLE_TAP_DRAG_ZOOM_SPEED,
                                                    Math.abs(dy * mPxToDp)));
                                }
                                break;
                            case MotionEvent.ACTION_UP:
                                if (mDoubleTapDragMode != DOUBLE_TAP_DRAG_MODE_ZOOM) {
                                    // Normal double tap gesture.
                                    sendMotionEventAsGesture(GESTURE_DOUBLE_TAP, e, null);
                                }
                                endDoubleTapDragMode(e);
                                break;
                            case MotionEvent.ACTION_CANCEL:
                                endDoubleTapDragMode(e);
                                break;
                            default:
                                break;
                        }
                        mDoubleTapY = e.getY();
                        return true;
                    }

                    @Override
                    public void onLongPress(MotionEvent e) {
                        if (!mZoomManager.isScaleGestureDetectionInProgress() &&
                                (mDoubleTapDragMode == DOUBLE_TAP_DRAG_MODE_NONE ||
                                 isDoubleTapDragDisabled())) {
                            mLastLongPressEvent = e;
                            sendMotionEventAsGesture(GESTURE_LONG_PRESS, e, null);
                        }
                    }

                    /**
                     * This method inspects the distance between where the user started touching
                     * the surface, and where she released. If the points are too far apart, we
                     * should assume that the web page has consumed the scroll-events in-between,
                     * and as such, this should not be considered a single-tap.
                     *
                     * We use the Android frameworks notion of how far a touch can wander before
                     * we think the user is scrolling.
                     *
                     * @param x the new x coordinate
                     * @param y the new y coordinate
                     * @return true if the distance is too long to be considered a single tap
                     */
                    private boolean isDistanceBetweenDownAndUpTooLong(float x, float y) {
                        double deltaX = mLastRawX - x;
                        double deltaY = mLastRawY - y;
                        return deltaX * deltaX + deltaY * deltaY > mScaledTouchSlopSquare;
                    }
                };
                mListener = listener;
                mGestureDetector = new GestureDetector(context, listener);
                mGestureDetector.setIsLongpressEnabled(false);
        } finally {
            TraceEvent.end();
        }
    }

    /**
     * @return LongPressDetector handling setting up timers for and canceling LongPress gestures.
     */
    LongPressDetector getLongPressDetector() {
        return mLongPressDetector;
    }

    /**
     * @param event Start a LongPress gesture event from the listener.
     */
    @Override
    public void onLongPress(MotionEvent event) {
        mListener.onLongPress(event);
    }

    /**
     * Cancels any ongoing LongPress timers.
     */
    void cancelLongPress() {
        mLongPressDetector.cancelLongPress();
    }

    /**
     * Fling the ContentView from the current position.
     * @param x Fling touch starting position
     * @param y Fling touch starting position
     * @param velocityX Initial velocity of the fling (X) measured in pixels per second.
     * @param velocityY Initial velocity of the fling (Y) measured in pixels per second.
     */
    void fling(long timeMs, int x, int y, int velocityX, int velocityY) {
        endFlingIfNecessary(timeMs);

        if (velocityX == 0 && velocityY == 0) {
            endTouchScrollIfNecessary(timeMs, true);
            return;
        }

        if (!mTouchScrolling) {
            // The native side needs a GESTURE_SCROLL_BEGIN before GESTURE_FLING_START
            // to send the fling to the correct target. Send if it has not sent.
            sendGesture(GESTURE_SCROLL_START, timeMs, x, y, null);
        }
        endTouchScrollIfNecessary(timeMs, false);

        mFlingMayBeActive = true;

        mExtraParamBundleFling.putInt(VELOCITY_X, velocityX);
        mExtraParamBundleFling.putInt(VELOCITY_Y, velocityY);
        assert mExtraParamBundleFling.size() == 2;
        sendGesture(GESTURE_FLING_START, timeMs, x, y, mExtraParamBundleFling);
    }

    /**
     * Send a GESTURE_FLING_CANCEL event if necessary.
     * @param timeMs The time in ms for the event initiating this gesture.
     */
    void endFlingIfNecessary(long timeMs) {
        if (!mFlingMayBeActive) return;
        mFlingMayBeActive = false;
        sendGesture(GESTURE_FLING_CANCEL, timeMs, 0, 0, null);
    }

    /**
     * End DOUBLE_TAP_DRAG_MODE_ZOOM by sending GESTURE_SCROLL_END and GESTURE_PINCH_END events.
     * @param event A hint event that its x, y, and eventTime will be used for the ending events
     *              to send. This argument is an optional and can be null.
     */
    void endDoubleTapDragMode(MotionEvent event) {
        if (isDoubleTapDragDisabled()) return;
        if (mDoubleTapDragMode == DOUBLE_TAP_DRAG_MODE_ZOOM) {
            if (event == null) event = obtainActionCancelMotionEvent();
            pinchEnd(event.getEventTime());
            sendGesture(GESTURE_SCROLL_END, event.getEventTime(),
                    (int) event.getX(), (int) event.getY(), null);
        }
        mDoubleTapDragMode = DOUBLE_TAP_DRAG_MODE_NONE;
    }

    /**
     * Reset touch scroll flag and optionally send a GESTURE_SCROLL_END event if necessary.
     * @param timeMs The time in ms for the event initiating this gesture.
     * @param sendScrollEndEvent Whether to send GESTURE_SCROLL_END event.
     */
    private void endTouchScrollIfNecessary(long timeMs, boolean sendScrollEndEvent) {
        if (!mTouchScrolling) return;
        mTouchScrolling = false;
        if (sendScrollEndEvent) {
            sendGesture(GESTURE_SCROLL_END, timeMs, 0, 0, null);
        }
    }

    /**
     * @return Whether native is tracking a scroll.
     */
    boolean isNativeScrolling() {
        // TODO(wangxianzhu): Also return true when fling is active once the UI knows exactly when
        // the fling ends.
        return mTouchScrolling;
    }

    /**
     * @return Whether native is tracking a pinch (i.e. between sending GESTURE_PINCH_BEGIN and
     *         GESTURE_PINCH_END).
     */
    boolean isNativePinching() {
        return mPinchInProgress;
    }

    /**
     * Starts a pinch gesture.
     * @param timeMs The time in ms for the event initiating this gesture.
     * @param x The x coordinate for the event initiating this gesture.
     * @param y The x coordinate for the event initiating this gesture.
     */
    void pinchBegin(long timeMs, int x, int y) {
        sendGesture(GESTURE_PINCH_BEGIN, timeMs, x, y, null);
    }

    /**
     * Pinch by a given percentage.
     * @param timeMs The time in ms for the event initiating this gesture.
     * @param anchorX The x coordinate for the anchor point to be used in pinch.
     * @param anchorY The y coordinate for the anchor point to be used in pinch.
     * @param delta The percentage to pinch by.
     */
    void pinchBy(long timeMs, int anchorX, int anchorY, float delta) {
        mExtraParamBundlePinchBy.putFloat(DELTA, delta);
        assert mExtraParamBundlePinchBy.size() == 1;
        sendGestureAndRequestVSync(
                GESTURE_PINCH_BY, timeMs, anchorX, anchorY, mExtraParamBundlePinchBy);
        mPinchInProgress = true;
    }

    /**
     * End a pinch gesture.
     * @param timeMs The time in ms for the event initiating this gesture.
     */
    void pinchEnd(long timeMs) {
        sendGesture(GESTURE_PINCH_END, timeMs, 0, 0, null);
        mPinchInProgress = false;
    }

    /**
     * Ignore singleTap gestures.
     */
    void setIgnoreSingleTap(boolean value) {
        mIgnoreSingleTap = value;
    }

    private void setClickXAndY(int x, int y) {
        mSingleTapX = x;
        mSingleTapY = y;
    }

    /**
     * @return The x coordinate for the last point that a singleTap gesture was initiated from.
     */
    public int getSingleTapX()  {
        return mSingleTapX;
    }

    /**
     * @return The y coordinate for the last point that a singleTap gesture was initiated from.
     */
    public int getSingleTapY()  {
        return mSingleTapY;
    }

    /**
     * Cancel the current touch event sequence by sending ACTION_CANCEL and ignore all the
     * subsequent events until the next ACTION_DOWN.
     *
     * One example usecase is stop processing the touch events when showing context popup menu.
     */
    public void setIgnoreRemainingTouchEvents() {
        onTouchEvent(obtainActionCancelMotionEvent());
        mIgnoreRemainingTouchEvents = true;
    }

    /**
     * Handle the incoming MotionEvent.
     * @return Whether the event was handled.
     */
    boolean onTouchEvent(MotionEvent event) {
        try {
            TraceEvent.begin("onTouchEvent");
            onTouchEventHandlingBegin();

            if (mIgnoreRemainingTouchEvents) {
                if (event.getActionMasked() == MotionEvent.ACTION_DOWN) {
                    mIgnoreRemainingTouchEvents = false;
                } else {
                    return false;
                }
            }

            mLongPressDetector.cancelLongPressIfNeeded(event);
            mSnapScrollController.setSnapScrollingMode(event);
            // Notify native that scrolling has stopped whenever a down action is processed prior to
            // passing the event to native as it will drop them as an optimization if scrolling is
            // enabled.  Ending the fling ensures scrolling has stopped as well as terminating the
            // current fling if applicable.
            if (event.getActionMasked() == MotionEvent.ACTION_DOWN) {
                mNoTouchHandlerForGesture = false;
                mJavaScriptIsConsumingGesture = false;
                endFlingIfNecessary(event.getEventTime());
            } else if (event.getActionMasked() == MotionEvent.ACTION_POINTER_DOWN) {
                endDoubleTapDragMode(null);
            }

            if (offerTouchEventToJavaScript(event)) {
                // offerTouchEventToJavaScript returns true to indicate the event was sent
                // to the render process. If it is not subsequently handled, it will
                // be returned via confirmTouchEvent(false) and eventually passed to
                // processTouchEvent asynchronously.
                return true;
            }
            return processTouchEvent(event);
        } finally {
            onTouchEventHandlingEnd();
            TraceEvent.end("onTouchEvent");
        }
    }

    /**
     * Handle content view losing focus -- ensure that any remaining active state is removed.
     */
    void onWindowFocusLost() {
        if (mLongPressDetector.isInLongPress() && mLastLongPressEvent != null) {
            sendShowPressCancelIfNecessary(mLastLongPressEvent);
        }
    }

    private MotionEvent obtainActionCancelMotionEvent() {
        return MotionEvent.obtain(
                SystemClock.uptimeMillis(),
                SystemClock.uptimeMillis(),
                MotionEvent.ACTION_CANCEL, 0.0f,  0.0f,  0);
    }

    /**
     * Resets gesture handlers state; called on didStartLoading().
     * Note that this does NOT clear the pending motion events queue;
     * it gets cleared in hasTouchEventHandlers() called from WebKit
     * FrameLoader::transitionToCommitted iff the page ever had touch handlers.
     */
    void resetGestureHandlers() {
        MotionEvent me = obtainActionCancelMotionEvent();
        me.setSource(InputDevice.SOURCE_CLASS_POINTER);
        mGestureDetector.onTouchEvent(me);
        mZoomManager.processTouchEvent(me);
        me.recycle();
        mLongPressDetector.cancelLongPress();
    }

    /**
     * Sets the flag indicating that the content has registered listeners for touch events.
     */
    void hasTouchEventHandlers(boolean hasTouchHandlers) {
        mHasTouchHandlers = hasTouchHandlers;
        // When mainframe is loading, FrameLoader::transitionToCommitted will
        // call this method to set mHasTouchHandlers to false. We use this as
        // an indicator to clear the pending motion events so that events from
        // the previous page will not be carried over to the new page.
        if (!mHasTouchHandlers) mPendingMotionEvents.clear();
    }

    private boolean offerTouchEventToJavaScript(MotionEvent event) {
        mLongPressDetector.onOfferTouchEventToJavaScript(event);

        if (!mHasTouchHandlers || mNoTouchHandlerForGesture) return false;

        if (event.getActionMasked() == MotionEvent.ACTION_MOVE) {
            // Only send move events if the move has exceeded the slop threshold.
            if (!mLongPressDetector.confirmOfferMoveEventToJavaScript(event)) {
                return true;
            }
            // Avoid flooding the renderer process with move events: if the previous pending
            // command is also a move (common case) that has not yet been forwarded, skip sending
            //  this event to the webkit side and collapse it into the pending event.
            MotionEvent previousEvent = mPendingMotionEvents.peekLast();
            if (previousEvent != null
                    && previousEvent != mPendingMotionEvents.peekFirst()
                    && previousEvent.getActionMasked() == MotionEvent.ACTION_MOVE
                    && previousEvent.getPointerCount() == event.getPointerCount()) {
                TraceEvent.instant("offerTouchEventToJavaScript:EventCoalesced",
                                   "QueueSize = " + mPendingMotionEvents.size());
                MotionEvent.PointerCoords[] coords =
                        new MotionEvent.PointerCoords[event.getPointerCount()];
                for (int i = 0; i < coords.length; ++i) {
                    coords[i] = new MotionEvent.PointerCoords();
                    event.getPointerCoords(i, coords[i]);
                }
                previousEvent.addBatch(event.getEventTime(), coords, event.getMetaState());
                return true;
            }
        }
        if (mPendingMotionEvents.isEmpty()) {
            // Add the event to the pending queue prior to calling sendPendingEventToNative.
            // When sending an event to native, the callback to confirmTouchEvent can be
            // synchronous or asynchronous and confirmTouchEvent expects the event to be
            // in the queue when it is called.
            MotionEvent clone = MotionEvent.obtain(event);
            mPendingMotionEvents.add(clone);

            int forward = sendPendingEventToNative();
            if (forward == EVENT_NOT_FORWARDED) mPendingMotionEvents.remove(clone);
            return forward != EVENT_NOT_FORWARDED;
        } else {
            TraceEvent.instant("offerTouchEventToJavaScript:EventQueued",
                               "QueueSize = " + mPendingMotionEvents.size());
            // Copy the event, as the original may get mutated after this method returns.
            MotionEvent clone = MotionEvent.obtain(event);
            mPendingMotionEvents.add(clone);
            return true;
        }
    }

    private int sendPendingEventToNative() {
        MotionEvent event = mPendingMotionEvents.peekFirst();
        if (event == null) {
            assert false : "Cannot send from an empty pending event queue";
            return EVENT_NOT_FORWARDED;
        }

        if (mTouchEventTimeoutHandler.hasTimeoutEvent()) return EVENT_NOT_FORWARDED;

        TouchPoint[] pts = new TouchPoint[event.getPointerCount()];
        int type = TouchPoint.createTouchPoints(event, pts);

        if (type == TouchPoint.CONVERSION_ERROR) return EVENT_NOT_FORWARDED;

        if (!mTouchScrolling && !mPinchInProgress) {
            if (mMotionEventDelegate.sendTouchEvent(event.getEventTime(), type, pts)) {
                // If confirmTouchEvent() is called synchronously with respect to sendTouchEvent(),
                // then |event| will have been recycled. Only start the timer if the sent event has
                // not yet been confirmed.
                if (!mJavaScriptIsConsumingGesture
                        && event == mPendingMotionEvents.peekFirst()
                        && event.getAction() != MotionEvent.ACTION_UP
                        && event.getAction() != MotionEvent.ACTION_CANCEL) {
                    mTouchEventTimeoutHandler.start(event.getEventTime(), pts);
                }
                return EVENT_FORWARDED_TO_NATIVE;
            }
        }
        return EVENT_NOT_FORWARDED;
    }

    private boolean processTouchEvent(MotionEvent event) {
        boolean handled = false;
        // The last "finger up" is an end to scrolling but may not be
        // an end to movement (e.g. fling scroll).  We do not tell
        // native code to end scrolling until we are sure we did not
        // fling.
        boolean possiblyEndMovement = false;
        // "Last finger raised" could be an end to movement.  However,
        // give the mSimpleTouchDetector a chance to continue
        // scrolling with a fling.
        if (event.getAction() == MotionEvent.ACTION_UP) {
            if (mTouchScrolling) {
                possiblyEndMovement = true;
            }
        }

        mLongPressDetector.cancelLongPressIfNeeded(event);
        mLongPressDetector.startLongPressTimerIfNeeded(event);

        // Use the framework's GestureDetector to detect pans and zooms not already
        // handled by the WebKit touch events gesture manager.
        if (canHandle(event)) {
            handled |= mGestureDetector.onTouchEvent(event);
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                mCurrentDownEvent = MotionEvent.obtain(event);
            }
        }

        handled |= mZoomManager.processTouchEvent(event);

        if (possiblyEndMovement && !handled) {
            endTouchScrollIfNecessary(event.getEventTime(), true);
        }

        return handled;
    }

    private void onTouchEventHandlingBegin() {
        mTouchEventHandlingStackDepth++;
    }

    private void onTouchEventHandlingEnd() {
        assert mTouchEventHandlingStackDepth > 0;
        mTouchEventHandlingStackDepth--;
        // Signal the vsync after we've finished handling touch events.
        if (mSentGestureNeedsVSync && mTouchEventHandlingStackDepth == 0) {
            assert mLastVSyncGestureTimeMs != 0;
            mSentGestureNeedsVSync = false;
            mMotionEventDelegate.onSentLastGestureForVSync(mLastVSyncGestureTimeMs);
        }
    }

    /**
     * For testing to simulate a timeout of a touch event handler.
     */
    void mockTouchEventTimeout() {
        mTouchEventTimeoutHandler.mockTimeout();
    }

    /**
     * Respond to a MotionEvent being returned from the native side.
     * @param ackResult The status acknowledgment code.
     */
    void confirmTouchEvent(int ackResult) {
        try {
            TraceEvent.begin("confirmTouchEvent");
            onTouchEventHandlingBegin();

            if (mTouchEventTimeoutHandler.confirmTouchEvent()) return;
            if (mPendingMotionEvents.isEmpty()) {
                Log.w(TAG, "confirmTouchEvent with Empty pending list!");
                return;
            }

            MotionEvent ackedEvent = mPendingMotionEvents.removeFirst();
            switch (ackResult) {
                case INPUT_EVENT_ACK_STATE_UNKNOWN:
                    // This should never get sent.
                    assert(false);
                    break;
                case INPUT_EVENT_ACK_STATE_CONSUMED:
                    mJavaScriptIsConsumingGesture = true;
                    mZoomManager.passTouchEventThrough(ackedEvent);
                    trySendPendingEventsToNative();
                    break;
                case INPUT_EVENT_ACK_STATE_NOT_CONSUMED:
                    if (!mJavaScriptIsConsumingGesture) processTouchEvent(ackedEvent);
                    trySendPendingEventsToNative();
                    break;
                case INPUT_EVENT_ACK_STATE_NO_CONSUMER_EXISTS:
                    mNoTouchHandlerForGesture = true;
                    processTouchEvent(ackedEvent);
                    drainAllPendingEventsUntilNextDown();
                    break;
                default:
                    break;
            }

            mLongPressDetector.cancelLongPressIfNeeded(mPendingMotionEvents.iterator());
            recycleEvent(ackedEvent);
        } finally {
            onTouchEventHandlingEnd();
            TraceEvent.end("confirmTouchEvent");
        }
    }

    private void trySendPendingEventsToNative() {
        while (!mPendingMotionEvents.isEmpty()) {
            int forward = sendPendingEventToNative();
            if (forward != EVENT_NOT_FORWARDED) break;

            // Even though we missed sending one event to native, as long as we haven't
            // received INPUT_EVENT_ACK_STATE_NO_CONSUMER_EXISTS, we should keep sending
            // events on the queue to native.
            MotionEvent event = mPendingMotionEvents.removeFirst();
            if (!mJavaScriptIsConsumingGesture) processTouchEvent(event);
            recycleEvent(event);
        }
    }

    private void drainAllPendingEventsUntilNextDown() {
        // Now process all events that are in the queue until the next down event.
        MotionEvent nextEvent = mPendingMotionEvents.peekFirst();
        while (nextEvent != null && nextEvent.getActionMasked() != MotionEvent.ACTION_DOWN) {
            processTouchEvent(nextEvent);
            mPendingMotionEvents.removeFirst();
            recycleEvent(nextEvent);
            nextEvent = mPendingMotionEvents.peekFirst();
        }

        if (nextEvent == null) return;

        mNoTouchHandlerForGesture = false;
        trySendPendingEventsToNative();
    }

    private void recycleEvent(MotionEvent event) {
        event.recycle();
    }

    private boolean sendMotionEventAsGesture(
            int type, MotionEvent event, Bundle extraParams) {
        return mMotionEventDelegate.sendGesture(type, event.getEventTime(),
            (int) event.getX(), (int) event.getY(), extraParams);
    }

    private boolean sendGesture(
            int type, long timeMs, int x, int y, Bundle extraParams) {
        return mMotionEventDelegate.sendGesture(type, timeMs, x, y, extraParams);
    }

    private boolean sendGestureAndRequestVSync(
            int type, long timeMs, int x, int y, Bundle extraParams) {
        // VSync should only be signalled if the sent gesture was generated from a touch event.
        mSentGestureNeedsVSync = mInputEventsDeliveredAtVSync && mTouchEventHandlingStackDepth > 0;
        mLastVSyncGestureTimeMs = timeMs;
        return mMotionEventDelegate.sendGesture(type, timeMs, x, y, extraParams);
    }

    void sendShowPressCancelIfNecessary(MotionEvent e) {
        if (!mShowPressIsCalled) return;

        if (sendMotionEventAsGesture(GESTURE_SHOW_PRESS_CANCEL, e, null)) {
            mShowPressIsCalled = false;
            mLastLongPressEvent = null;
        }
    }

    /**
     * @return Whether the ContentViewGestureHandler can handle a MotionEvent right now. True only
     * if it's the start of a new stream (ACTION_DOWN), or a continuation of the current stream.
     */
    boolean canHandle(MotionEvent ev) {
        return ev.getAction() == MotionEvent.ACTION_DOWN ||
                (mCurrentDownEvent != null && mCurrentDownEvent.getDownTime() == ev.getDownTime());
    }

    /**
     * @return Whether the event can trigger a LONG_TAP gesture. True when it can and the event
     * will be consumed.
     */
    boolean triggerLongTapIfNeeded(MotionEvent ev) {
        if (mLongPressDetector.isInLongPress() && ev.getAction() == MotionEvent.ACTION_UP &&
                !mZoomManager.isScaleGestureDetectionInProgress()) {
            sendShowPressCancelIfNecessary(ev);
            sendMotionEventAsGesture(GESTURE_LONG_TAP, ev, null);
            return true;
        }
        return false;
    }

    /**
     * This is for testing only.
     * @return The first motion event on the pending motion events queue.
     */
    MotionEvent peekFirstInPendingMotionEventsForTesting() {
        return mPendingMotionEvents.peekFirst();
    }

    /**
     * This is for testing only.
     * @return The number of motion events on the pending motion events queue.
     */
    int getNumberOfPendingMotionEventsForTesting() {
        return mPendingMotionEvents.size();
    }

    /**
     * This is for testing only.
     * Sends a show pressed state gesture through mListener. This should always be called after
     * a down event;
     */
    void sendShowPressedStateGestureForTesting() {
        if (mCurrentDownEvent == null) return;
        mListener.onShowPress(mCurrentDownEvent);
    }

    /**
     * This is for testing only.
     * @return Whether a touch timeout event has been scheduled.
     */
    boolean hasScheduledTouchTimeoutEventForTesting() {
        return mTouchEventTimeoutHandler.hasScheduledTimeoutEventForTesting();
    }

    public void updateDoubleTapDragSupport(boolean supportDoubleTapDrag) {
        assert (mDoubleTapDragMode == DOUBLE_TAP_DRAG_MODE_DISABLED ||
                mDoubleTapDragMode == DOUBLE_TAP_DRAG_MODE_NONE);
        mDoubleTapDragMode = supportDoubleTapDrag ?
                DOUBLE_TAP_DRAG_MODE_NONE : DOUBLE_TAP_DRAG_MODE_DISABLED;
    }

    private boolean isDoubleTapDragDisabled() {
        return mDoubleTapDragMode == DOUBLE_TAP_DRAG_MODE_DISABLED;
    }
}
