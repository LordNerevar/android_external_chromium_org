# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import file_verifier
import registry_verifier


def Verify(property):
  """Verifies that the current machine states match the property dictionary.

  A property dictionary is a dictionary where each key is a verifier's name and
  the associated value is the input to that verifier. For details about the
  input format for each verifier, take a look at http://goo.gl/1P85WL

  Args:
    property: A property dictionary.
  """
  for verifier_name, value in property.iteritems():
    if verifier_name == 'Files':
      file_verifier.VerifyFiles(value)
    elif verifier_name == 'RegistryEntries':
      registry_verifier.VerifyRegistryEntries(value)
    else:
      # TODO(sukolsak): Implement other verifiers
      raise KeyError('Unknown verifier %s' % verifier_name)
