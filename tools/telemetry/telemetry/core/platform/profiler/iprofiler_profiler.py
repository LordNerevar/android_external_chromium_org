# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import signal
import sys

from telemetry.core import util
from telemetry.core.platform import profiler

# pexpect is not available on all platforms so use the third_party version.
sys.path.append(os.path.join(
    os.path.abspath(os.path.dirname(__file__)), '..', '..', '..', '..', '..',
    '..', 'third_party', 'pexpect'))
import pexpect  # pylint: disable=F0401


class IprofilerProfiler(profiler.Profiler):

  def __init__(self, browser_backend, pid, output_path):
    super(IprofilerProfiler, self).__init__(output_path)
    self._browser_backend = browser_backend
    output_dir = os.path.dirname(self.output_path)
    output_file = os.path.basename(self.output_path)
    self._proc = pexpect.spawn(
        'iprofiler', ['-timeprofiler', '-T', '300', '-a', str(pid),
                      '-d', output_dir, '-o', output_file],
        timeout=300)
    while True:
      if self._proc.getecho():
        output = self._proc.readline().strip()
        if not output:
          continue
        if 'iprofiler: Profiling process' in output:
          break
        print output
      self._proc.interact(escape_character='\x0d')
      self._proc.write('\x0d')
      print
      def Echo():
        return self._proc.getecho()
      util.WaitFor(Echo, timeout=5)

  @classmethod
  def name(cls):
    return 'iprofiler'

  @classmethod
  def is_supported(cls, options):
    return (sys.platform == 'darwin'
            and not options.browser_type.startswith('android')
            and not options.browser_type.startswith('cros'))

  def CollectProfile(self):
    self._proc.kill(signal.SIGINT)
    try:
      self._proc.wait()
    except pexpect.ExceptionPexpect:
      pass
    finally:
      self._proc = None

    print 'To view the profile, run:'
    print '  open -a Instruments %s.dtps' % self.output_path
