# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Utilities for managing files in Google Cloud Storage."""

import collections
import itertools
import json
import os
import posixpath
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir))
from tools import image_tools


class RenderingTestManager(object):
  """The manager from which GCS utility functions are called."""

  def __init__(self, cloud_bucket):
    """Initialize with a cloud_bucket subclass to supply GCS functionality.

    Args:
      cloud_bucket: A subclass of cloud_bucket.CloudBucket.

    Returns:
      An instance of Manager.
    """
    self.cloud_bucket = cloud_bucket

  def UploadImage(self, full_path, image):
    """Uploads an image to a location in GCS.

    Args:
      full_path: the path to the file in GCS including the file extension.
      image: a RGB PIL.Image to be uploaded.
    """
    self.cloud_bucket.UploadFile(
        full_path, image_tools.SerializeImage(image).decode('base64'),
        'image/png')

  def DownloadImage(self, full_path):
    """Downloads an image from a location in GCS.

    Args:
      full_path: the path to the file in GCS including the file extension.

    Returns:
      The downloaded RGB PIL.Image.

    Raises:
      cloud_bucket.NotFoundError: if the path to the image is not valid.
    """
    return image_tools.DeserializeImage(
        self.cloud_bucket.DownloadFile(full_path).encode('base64'))

  def UploadTest(self, batch_name, test_name, images):
    """Creates and uploads a test to GCS from a set of images and name.

    This method generates a mask from the uploaded images, then
      uploads the mask and first of the images to GCS as a test.

    Args:
      batch_name: the name of the batch.
      test_name: the name of the test.
      images: a list of RGB encoded PIL.Images
    """
    path = posixpath.join('tests', batch_name, test_name)
    mask = image_tools.InflateMask(image_tools.CreateMask(images), 7)
    self.UploadImage(posixpath.join(path, 'expected.png'), images[0])
    self.UploadImage(posixpath.join(path, 'mask.png'), mask)

  def RunTest(self, batch_name, test_name, actual):
    """Runs an image comparison, and uploads discrepancies to GCS.

    Args:
      batch_name: the name of the batch.
      test_name: the name of the test to run.
      run_name: the name of this run of the test.
      actual: an RGB-encoded PIL.Image that is the actual result of the
        test.

    Raises:
      cloud_bucket.NotFoundError: if the given test_name is not found.
    """
    path = posixpath.join('failures', batch_name, test_name)
    test = self.GetTest(batch_name, test_name)
    if not image_tools.SameImage(actual, test.expected, mask=test.mask):
      self.UploadImage(posixpath.join(path, 'actual.png'), actual)
      diff = image_tools.VisualizeImageDifferences(
          test.expected, actual, mask=test.mask)
      diff_pxls = sum(1 if pxl == (255, 255, 255, 255) else 0
                      for pxl in diff.getdata())
      self.UploadImage(posixpath.join(path, 'diff.png'), diff)
      self.cloud_bucket.UploadFile(
          posixpath.join(path, 'info.txt'),
          json.dumps({'different_pixels': diff_pxls}), 'application/json')

  def GetTest(self, batch_name, test_name):
    """Returns given test from GCS.

    Args:
      batch_name: the name of the batch.
      test_name: the name of the test to get from GCS.

    Returns:
      A named tuple: 'Test', containing two images: expected and mask.

    Raises:
      cloud_bucket.NotFoundError: if the test is not found in GCS.
    """
    path = posixpath.join('tests', batch_name, test_name)
    Test = collections.namedtuple('Test', ['expected', 'mask'])
    return Test(self.DownloadImage(posixpath.join(path, 'expected.png')),
                self.DownloadImage(posixpath.join(path, 'mask.png')))

  def TestExists(self, batch_name, test_name):
    """Returns whether the given test exists in GCS.

    Args:
      batch_name: the name of the batch.
      test_name: the name of the test to look for.

    Returns:
      A boolean indicating whether the test exists.
    """
    path = posixpath.join('tests', batch_name, test_name)
    expected_image_exists = self.cloud_bucket.FileExists(
        posixpath.join(path, 'expected.png'))
    mask_image_exists = self.cloud_bucket.FileExists(
        posixpath.join(path, 'mask.png'))
    return expected_image_exists and mask_image_exists

  def FailureExists(self, batch_name, test_name):
    """Returns whether the given run exists in GCS.

    Args:
      batch_name: the name of the batch.
      test_name: the name of the test that failed.
      run_name: the name of the run that the given test failed on.

    Returns:
      A boolean indicating whether the failure exists.
    """
    failure_path = posixpath.join('failures', batch_name, test_name)
    actual_image_exists = self.cloud_bucket.FileExists(
        posixpath.join(failure_path, 'actual.png'))
    test_exists = self.TestExists(batch_name, test_name)
    info_exists = self.cloud_bucket.FileExists(
        posixpath.join(failure_path, 'info.txt'))
    return test_exists and actual_image_exists and info_exists

  def RemoveTest(self, batch_name, test_name):
    """Removes a Test from GCS, and all associated failures with that test.

    Args:
      batch_name: the name of the batch.
      test_name: the name of the test to remove.
    """
    test_path = posixpath.join('tests', batch_name, test_name)
    failure_path = posixpath.join('failures', batch_name, test_name)
    test_paths = self.cloud_bucket.GetAllPaths(test_path)
    failure_paths = self.cloud_bucket.GetAllPaths(failure_path)
    for path in itertools.chain(failure_paths, test_paths):
      self.cloud_bucket.RemoveFile(path)

  def RemoveFailure(self, batch_name, test_name):
    """Removes a failure from GCS.

    Args:
      batch_name: the name of the batch.
      test_name: the test on which the failure to be removed occured.
      run_name: the name of the run on the given test that failed.
    """
    failure_path = posixpath.join('failures', batch_name, test_name)
    failure_paths = self.cloud_bucket.GetAllPaths(failure_path)
    for path in failure_paths:
      self.cloud_bucket.RemoveFile(path)

  def GetFailure(self, batch_name, test_name):
    """Returns a given test failure's expected, diff, and actual images.

    Args:
      batch_name: the name of the batch.
      test_name: the name of the test the result corresponds to.
      run_name: the name of the result on the given test.

    Returns:
      A named tuple: Failure containing three images: expected, diff, and
        actual.

    Raises:
      cloud_bucket.NotFoundError: if the result is not found in GCS.
    """
    test_path = posixpath.join('tests', batch_name, test_name)
    failure_path = posixpath.join('failures', batch_name, test_name)
    expected = self.DownloadImage(posixpath.join(test_path, 'expected.png'))
    actual = self.DownloadImage(posixpath.join(failure_path, 'actual.png'))
    diff = self.DownloadImage(posixpath.join(failure_path, 'diff.png'))
    info = json.loads(self.cloud_bucket.DownloadFile(
        posixpath.join(failure_path, 'info.txt')))
    Failure = collections.namedtuple(
        'Failure',
        ['expected', 'diff', 'actual', 'info'])
    return Failure(expected, diff, actual, info)

  def GetAllPaths(self, prefix):
    """Gets urls to all files in GCS whose path starts with a given prefix.

    Args:
      prefix: the prefix to filter files in GCS by.

    Returns:
      a list containing urls to all objects that started with
         the prefix.
    """
    return self.cloud_bucket.GetAllPaths(prefix)
