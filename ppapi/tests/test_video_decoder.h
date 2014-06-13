// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_TESTS_TEST_VIDEO_DECODER_H_
#define PPAPI_TESTS_TEST_VIDEO_DECODER_H_

#include <string>

#include "ppapi/c/pp_stdint.h"
#include "ppapi/c/ppb_video_decoder.h"
#include "ppapi/cpp/graphics_3d.h"
#include "ppapi/tests/test_case.h"

class TestVideoDecoder : public TestCase {
 public:
  explicit TestVideoDecoder(TestingInstance* instance) : TestCase(instance) {}

 private:
  // TestCase implementation.
  virtual bool Init();
  virtual void RunTests(const std::string& filter);

  std::string TestCreate();

  // Used by the tests that access the C API directly.
  const PPB_VideoDecoder_0_1* video_decoder_interface_;

  pp::Graphics3D graphics_3d_;
};

#endif  // PPAPI_TESTS_TEST_VIDEO_DECODER_H_
