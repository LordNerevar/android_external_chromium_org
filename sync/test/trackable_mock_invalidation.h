// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYNC_TEST_TRACKABLE_MOCK_INVALIDATION_H_
#define SYNC_TEST_TRACKABLE_MOCK_INVALIDATION_H_

#include "sync/test/mock_invalidation.h"

namespace syncer {

class MockInvalidationTracker;

// A variant of MockInvalidation that supports acknowledgements.
//
// With the help of a MockInvalidationTracker, this can be used to test
// sync's use of the Drop() and Acknowledge() methods.
class TrackableMockInvalidation : public MockInvalidation {
 public:
  TrackableMockInvalidation(bool is_unknown_version,
                            int64 version,
                            const std::string& payload,
                            MockInvalidationTracker* tracker,
                            int tracking_id);
  virtual ~TrackableMockInvalidation();

  // Forwards notice of the acknowledgement of this invalidation to the
  // |tracker_|.
  virtual void Acknowledge() OVERRIDE;

  // Forwards notice of the drop of this invalidation to the |tracker_|.
  virtual void Drop() OVERRIDE;

  // Returns the integer used to identify this object with the |tracker_|.
  int GetTrackingId();

 private:
  // The MockInvalidationTracker that initialized this object, and which keeps
  // track of its acknowledgement status.  It is expected to outlive the
  // invalidations.  The data required for unit test assertions lives there.
  MockInvalidationTracker* tracker_;

  // An identifier that uniquely identifies this invalidation to its
  // |tracker_|.
  //
  // This is necessary in part because invalidations may be short lived; the
  // invalidation may be deleted by the time we want to make assertions about
  // its state.
  int tracking_id_;
};

}  // namespace syncer

#endif  // SYNC_TEST_TRACKABLE_MOCK_INVALIDATION_H_
