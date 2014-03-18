// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_DOMAIN_RELIABILITY_TEST_UTIL_H_
#define COMPONENTS_DOMAIN_RELIABILITY_TEST_UTIL_H_

#include "base/callback.h"
#include "components/domain_reliability/monitor.h"
#include "components/domain_reliability/util.h"
#include "net/base/host_port_pair.h"

namespace net {
class URLRequestStatus;
}  // namespace net

namespace domain_reliability {

// A simple test callback that remembers whether it's been called.
class TestCallback {
 public:
  TestCallback();
  ~TestCallback();

  // Returns a callback that can be called only once.
  const base::Closure& callback() const { return callback_; }
  // Returns whether the callback returned by |callback()| has been called.
  bool called() const { return called_; }

 private:
  void OnCalled();

  base::Closure callback_;
  bool called_;
};

class MockTime : public MockableTime {
 public:
  MockTime();
  // N.B.: Tasks (and therefore Timers) scheduled to run in the future will
  // never be run if MockTime is destroyed before the mock time is advanced
  // to their scheduled time.
  virtual ~MockTime();

  virtual base::TimeTicks Now() OVERRIDE;
  virtual scoped_ptr<MockableTime::Timer> CreateTimer() OVERRIDE;

  // Pretends that |delta| has passed, and runs tasks that would've happened
  // during that interval (with |Now()| returning proper values while they
  // execute!)
  void Advance(base::TimeDelta delta);

  // Queues |task| to be run after |delay|. (Lighter-weight than mocking an
  // entire message pump.)
  void AddTask(base::TimeDelta delay, const base::Closure& task);

 private:
  // Key used to store tasks in the task map. Includes the time the task should
  // run and a sequence number to disambiguate tasks with the same time.
  struct TaskKey {
    TaskKey(base::TimeTicks time, int sequence_number)
        : time(time),
          sequence_number(sequence_number) {}

    base::TimeTicks time;
    int sequence_number;
  };

  // Comparator for TaskKey; sorts by time, then by sequence number.
  struct TaskKeyCompare {
    bool operator() (const TaskKey& lhs, const TaskKey& rhs) const {
      return lhs.time < rhs.time ||
             (lhs.time == rhs.time &&
              lhs.sequence_number < rhs.sequence_number);
    }
  };

  typedef std::map<TaskKey, base::Closure, TaskKeyCompare> TaskMap;

  int elapsed_sec() { return (now_ - epoch_).InSeconds(); }

  base::TimeTicks now_;
  base::TimeTicks epoch_;
  int task_sequence_number_;
  TaskMap tasks_;
};

}  // namespace domain_reliability

#endif  // COMPONENTS_DOMAIN_RELIABILITY_TEST_UTIL_H_
