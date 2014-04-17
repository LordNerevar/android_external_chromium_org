// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/base/test_data_util.h"
#include "media/formats/common/stream_parser_test_base.h"
#include "media/formats/mpeg/mp3_stream_parser.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace media {

class MP3StreamParserTest : public StreamParserTestBase, public testing::Test {
 public:
  MP3StreamParserTest()
      : StreamParserTestBase(
            scoped_ptr<StreamParser>(new MP3StreamParser()).Pass()) {}
  virtual ~MP3StreamParserTest() {}
};

// Test parsing with small prime sized chunks to smoke out "power of
// 2" field size assumptions.
TEST_F(MP3StreamParserTest, UnalignedAppend) {
  const std::string expected =
      "NewSegment"
      "{ 0K }"
      "{ 0K }"
      "{ 0K }"
      "{ 0K }"
      "{ 0K }"
      "{ 0K }"
      "{ 0K }"
      "EndOfSegment"
      "NewSegment"
      "{ 0K }"
      "{ 0K }"
      "{ 0K }"
      "EndOfSegment"
      "NewSegment"
      "{ 0K }"
      "{ 0K }"
      "EndOfSegment";
  EXPECT_EQ(expected, ParseFile("sfx.mp3", 17));
}

// Test parsing with a larger piece size to verify that multiple buffers
// are passed to |new_buffer_cb_|.
TEST_F(MP3StreamParserTest, UnalignedAppend512) {
  const std::string expected =
      "NewSegment"
      "{ 0K 26K 52K 78K }"
      "EndOfSegment"
      "NewSegment"
      "{ 0K 26K 52K }"
      "{ 0K 26K 52K 78K }"
      "{ 0K }"
      "EndOfSegment";
  EXPECT_EQ(expected, ParseFile("sfx.mp3", 512));
}

TEST_F(MP3StreamParserTest, MetadataParsing) {
  scoped_refptr<DecoderBuffer> buffer = ReadTestDataFile("sfx.mp3");
  const uint8_t* buffer_ptr = buffer->data();

  // The first 32 bytes of sfx.mp3 are an ID3 tag, so no segments should be
  // extracted after appending those bytes.
  const int kId3TagSize = 32;
  EXPECT_EQ("", ParseData(buffer_ptr, kId3TagSize));
  EXPECT_FALSE(last_audio_config().IsValidConfig());
  buffer_ptr += kId3TagSize;

  // The next 417 bytes are a Xing frame; with the identifier 21 bytes into
  // the frame.  Appending less than 21 bytes, should result in no segments
  // nor an AudioDecoderConfig being created.
  const int kXingTagPosition = 21;
  EXPECT_EQ("", ParseData(buffer_ptr, kXingTagPosition));
  EXPECT_FALSE(last_audio_config().IsValidConfig());
  buffer_ptr += kXingTagPosition;

  // Appending the rests of the Xing frame should result in no segments, but
  // should generate a valid AudioDecoderConfig.
  const int kXingRemainingSize = 417 - kXingTagPosition;
  EXPECT_EQ("", ParseData(buffer_ptr, kXingRemainingSize));
  EXPECT_TRUE(last_audio_config().IsValidConfig());
  buffer_ptr += kXingRemainingSize;

  // Append the first real frame and ensure we get a segment.
  const int kFirstRealFrameSize = 182;
  EXPECT_EQ("NewSegment{ 0K }EndOfSegment",
            ParseData(buffer_ptr, kFirstRealFrameSize));
  EXPECT_TRUE(last_audio_config().IsValidConfig());
}

}  // namespace media
