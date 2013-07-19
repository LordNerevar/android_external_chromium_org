# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

from render_servlet import RenderServlet
from server_instance import ServerInstance
from servlet import Request

class _LocalRenderServletDelegate(object):
  def CreateServerInstance(self):
    return ServerInstance.ForLocal()

class LocalRenderer(object):
  '''Renders pages fetched from the local file system.
  '''
  @staticmethod
  def Render(path):
    assert not '\\' in path
    return RenderServlet(Request.ForTest(path),
                         _LocalRenderServletDelegate()).Get()
