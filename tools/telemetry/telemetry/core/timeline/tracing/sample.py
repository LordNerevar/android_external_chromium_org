# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import telemetry.core.timeline.event as timeline_event

class Sample(timeline_event.TimelineEvent):
  ''' A Sample represents a sample taken at an instant in time
  plus parameters associated with that sample.

  NOTE: The Sample class implements the same interface as
  Slice. These must be kept in sync.

  All time units are stored in milliseconds.
  '''
  def __init__(self, category, name, timestamp, args=None, parent=None):
    super(Sample, self).__init__(
        name, timestamp, 0, args=args, parent=parent)
    self.category = category
