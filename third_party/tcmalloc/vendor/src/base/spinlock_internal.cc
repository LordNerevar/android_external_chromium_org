/* Copyright (c) 2010, Google Inc.
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

// The OS-specific header included below must provide two calls:
// base::internal::SpinLockDelay() and base::internal::SpinLockWake().
// See spinlock_internal.h for the spec of SpinLockWake().

// void SpinLockDelay(volatile Atomic32 *w, int32 value, int loop)
// SpinLockDelay() generates an apprproate spin delay on iteration "loop" of a
// spin loop on location *w, whose previously observed value was "value".
// SpinLockDelay() may do nothing, may yield the CPU, may sleep a clock tick,
// or may wait for a delay that can be truncated by a call to SpinlockWake(w).
// In all cases, it must return in bounded time even if SpinlockWake() is not
// called.

#include "base/spinlock_internal.h"

#if defined(_WIN32)
#include "base/spinlock_win32-inl.h"
#elif defined(__linux__) && !defined(__native_client__)
#include "base/spinlock_linux-inl.h"
#else
#include "base/spinlock_posix-inl.h"
#endif

namespace base {
namespace internal {

// See spinlock_internal.h for spec.
int32 SpinLockWait(volatile Atomic32 *w, int n,
                   const SpinLockWaitTransition trans[]) {
  int32 v;
  bool done = false;
  for (int loop = 0; !done; loop++) {
    v = base::subtle::Acquire_Load(w);
    int i;
    for (i = 0; i != n && v != trans[i].from; i++) {
    }
    if (i == n) {
      SpinLockDelay(w, v, loop);     // no matching transition
    } else if (trans[i].to == v ||   // null transition
               base::subtle::Acquire_CompareAndSwap(w, v, trans[i].to) == v) {
      done = trans[i].done;
    }
  }
  return v;
}

} // namespace internal
} // namespace base
