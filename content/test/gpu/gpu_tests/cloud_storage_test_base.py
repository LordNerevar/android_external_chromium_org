# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Base classes for a test and validator which upload results
(reference images, error images) to cloud storage."""

import re
import tempfile

from telemetry import test
from telemetry.core import bitmap
from telemetry.page import cloud_storage
from telemetry.page import page_test

error_image_cloud_storage_bucket = 'chromium-browser-gpu-tests'

class ValidatorBase(page_test.PageTest):
  def __init__(self, test_method_name):
    super(ValidatorBase, self).__init__(test_method_name)
    # Parameters for cloud storage reference images.
    self.vendor_id = None
    self.device_id = None
    self.vendor_string = None
    self.device_string = None
    self.msaa = False

  def _ComputeGpuInfo(self, tab):
    if ((self.vendor_id and self.device_id) or
        (self.vendor_string and self.device_string)):
      return
    browser = tab.browser
    if not browser.supports_system_info:
      raise Exception('System info must be supported by the browser')
    system_info = browser.GetSystemInfo()
    if not system_info.gpu:
      raise Exception('GPU information was absent')
    device = system_info.gpu.devices[0]
    if device.vendor_id and device.device_id:
      self.vendor_id = device.vendor_id
      self.device_id = device.device_id
    elif device.vendor_string and device.device_string:
      self.vendor_string = device.vendor_string
      self.device_string = device.device_string
    else:
      raise Exception('GPU device information was incomplete')
    self.msaa = system_info.gpu.feature_status['multisampling'] == 'enabled'

  def _FormatGpuInfo(self, tab):
    self._ComputeGpuInfo(tab)
    msaa_string = '_msaa' if self.msaa else '_non_msaa'
    if self.vendor_id:
      return '%s_%04x_%04x%s' % (
        self.options.os_type, self.vendor_id, self.device_id, msaa_string)
    else:
      return '%s_%s_%s%s' % (
        self.options.os_type, self.vendor_string, self.device_string,
        msaa_string)

  def _FormatReferenceImageName(self, img_name, page, tab):
    return '%s_v%s_%s.png' % (
      img_name,
      page.revision,
      self._FormatGpuInfo(tab))

  def _UploadBitmapToCloudStorage(self, bucket, name, bitmap, public=False):
    # This sequence of steps works on all platforms to write a temporary
    # PNG to disk, following the pattern in bitmap_unittest.py. The key to
    # avoiding PermissionErrors seems to be to not actually try to write to
    # the temporary file object, but to re-open its name for all operations.
    temp_file = tempfile.NamedTemporaryFile().name
    bitmap.WritePngFile(temp_file)
    cloud_storage.Insert(bucket, name, temp_file, publicly_readable=public)

  def _ConditionallyUploadToCloudStorage(self, img_name, page, tab, screenshot):
    """Uploads the screenshot to cloud storage as the reference image
    for this test, unless it already exists. Returns True if the
    upload was actually performed."""
    if not self.options.refimg_cloud_storage_bucket:
      raise Exception('--refimg-cloud-storage-bucket argument is required')
    cloud_name = self._FormatReferenceImageName(img_name, page, tab)
    if not cloud_storage.Exists(self.options.refimg_cloud_storage_bucket,
                                cloud_name):
      self._UploadBitmapToCloudStorage(self.options.refimg_cloud_storage_bucket,
                                       cloud_name,
                                       screenshot)
      return True
    return False

  def _DownloadFromCloudStorage(self, img_name, page, tab):
    """Downloads the reference image for the given test from cloud
    storage, returning it as a Telemetry Bitmap object."""
    # TODO(kbr): there's a race condition between the deletion of the
    # temporary file and gsutil's overwriting it.
    if not self.options.refimg_cloud_storage_bucket:
      raise Exception('--refimg-cloud-storage-bucket argument is required')
    temp_file = tempfile.NamedTemporaryFile().name
    cloud_storage.Get(self.options.refimg_cloud_storage_bucket,
                      self._FormatReferenceImageName(img_name, page, tab),
                      temp_file)
    return bitmap.Bitmap.FromPngFile(temp_file)

  def _UploadErrorImagesToCloudStorage(self, image_name, ref_img, screenshot):
    """For a failing run, uploads the reference image, failing image,
    and diff image to cloud storage. This subsumes the functionality
    of the archive_gpu_pixel_test_results.py script."""
    machine_name = re.sub('\W+', '_', self.options.test_machine_name)
    upload_dir = '%s_%s_telemetry' % (self.options.build_revision, machine_name)
    base_bucket = '%s/runs/%s' % (error_image_cloud_storage_bucket, upload_dir)
    image_name_with_revision = '%s_%s.png' % (
      image_name, self.options.build_revision)
    self._UploadBitmapToCloudStorage(
      base_bucket + '/ref', image_name_with_revision, ref_img, public=True)
    self._UploadBitmapToCloudStorage(
      base_bucket + '/gen', image_name_with_revision, screenshot,
      public=True)
    diff_img = screenshot.Diff(ref_img)
    self._UploadBitmapToCloudStorage(
      base_bucket + '/diff', image_name_with_revision, diff_img,
      public=True)
    print ('See http://%s.commondatastorage.googleapis.com/'
           'view_test_results.html?%s for this run\'s test results') % (
      error_image_cloud_storage_bucket, upload_dir)

class TestBase(test.Test):
  @staticmethod
  def _AddTestCommandLineOptions(parser, option_group):
    option_group.add_option('--build-revision',
        help='Chrome revision being tested.',
        default="unknownrev")
    option_group.add_option('--upload-refimg-to-cloud-storage',
        dest='upload_refimg_to_cloud_storage',
        action='store_true', default=False,
        help='Upload resulting images to cloud storage as reference images')
    option_group.add_option('--download-refimg-from-cloud-storage',
        dest='download_refimg_from_cloud_storage',
        action='store_true', default=False,
        help='Download reference images from cloud storage')
    option_group.add_option('--refimg-cloud-storage-bucket',
        help='Name of the cloud storage bucket to use for reference images; '
        'required with --upload-refimg-to-cloud-storage and '
        '--download-refimg-from-cloud-storage. Example: '
        '"chromium-gpu-archive/reference-images"')
    option_group.add_option('--os-type',
        help='Type of operating system on which the pixel test is being run, '
        'used only to distinguish different operating systems with the same '
        'graphics card. Any value is acceptable, but canonical values are '
        '"win", "mac", and "linux", and probably, eventually, "chromeos" '
        'and "android").',
        default='')
    option_group.add_option('--test-machine-name',
        help='Name of the test machine. Specifying this argument causes this '
        'script to upload failure images and diffs to cloud storage directly, '
        'instead of relying on the archive_gpu_pixel_test_results.py script.',
        default='')
