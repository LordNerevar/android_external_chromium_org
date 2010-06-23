// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/basictypes.h"
#include "base/scoped_vector.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/history/query_parser.h"
#include "testing/gtest/include/gtest/gtest.h"

class QueryParserTest : public testing::Test {
 public:
  struct TestData {
    const char* input;
    const int expected_word_count;
  };

  std::string QueryToString(const std::string& query);

 protected:
  QueryParser query_parser_;
};

// Test helper: Convert a user query string in 8-bit (for hardcoding
// convenience) to a SQLite query string.
std::string QueryParserTest::QueryToString(const std::string& query) {
  string16 sqlite_query;
  query_parser_.ParseQuery(UTF8ToUTF16(query), &sqlite_query);
  return UTF16ToUTF8(sqlite_query);
}

// Basic multi-word queries, including prefix matching.
TEST_F(QueryParserTest, SimpleQueries) {
  EXPECT_EQ("", QueryToString(" "));
  EXPECT_EQ("singleword*", QueryToString("singleword"));
  EXPECT_EQ("spacedout*", QueryToString("  spacedout "));
  EXPECT_EQ("foo* bar*", QueryToString("foo bar"));
  // Short words aren't prefix matches. For Korean Hangul
  // the minimum is 2 while for other scripts, it's 3.
  EXPECT_EQ("f b", QueryToString(" f b"));
  // KA JANG
  EXPECT_EQ(WideToUTF8(L"\xAC00 \xC7A5"),
            QueryToString(WideToUTF8(L" \xAC00 \xC7A5")));
  EXPECT_EQ("foo* bar*", QueryToString(" foo   bar "));
  // KA-JANG BICH-GO
  EXPECT_EQ(WideToUTF8(L"\xAC00\xC7A5* \xBE5B\xACE0*"),
            QueryToString(WideToUTF8(L"\xAC00\xC7A5 \xBE5B\xACE0")));
}

// Quoted substring parsing.
TEST_F(QueryParserTest, Quoted) {
  // ASCII quotes
  EXPECT_EQ("\"Quoted\"", QueryToString("\"Quoted\""));
  // Missing end quotes
  EXPECT_EQ("\"miss end\"", QueryToString("\"miss end"));
  // Missing begin quotes
  EXPECT_EQ("miss* beg*", QueryToString("miss beg\""));
  // Weird formatting
  EXPECT_EQ("\"Many\" \"quotes\"", QueryToString("\"Many   \"\"quotes"));
}

// Apostrophes within words should be preserved, but otherwise stripped.
TEST_F(QueryParserTest, Apostrophes) {
  EXPECT_EQ("foo* bar's*", QueryToString("foo bar's"));
  EXPECT_EQ("l'foo*", QueryToString("l'foo"));
  EXPECT_EQ("foo*", QueryToString("'foo"));
}

// Special characters.
TEST_F(QueryParserTest, SpecialChars) {
  EXPECT_EQ("foo* the* bar*", QueryToString("!#:/*foo#$*;'* the!#:/*bar"));
}

TEST_F(QueryParserTest, NumWords) {
  TestData data[] = {
    { "blah",                  1 },
    { "foo \"bar baz\"",       3 },
    { "foo \"baz\"",           2 },
    { "foo \"bar baz\"  blah", 4 },
  };

  for (size_t i = 0; i < arraysize(data); ++i) {
    string16 query_string;
    EXPECT_EQ(data[i].expected_word_count,
              query_parser_.ParseQuery(UTF8ToUTF16(data[i].input),
                                       &query_string));
  }
}

TEST_F(QueryParserTest, ParseQueryNodesAndMatch) {
  struct TestData2 {
    const std::string query;
    const std::string text;
    const bool matches;
    const size_t m1_start;
    const size_t m1_end;
    const size_t m2_start;
    const size_t m2_end;
  } data[] = {
    { "foo foo",       "foo",              true,  0, 3, 0, 0 },
    { "foo fooey",     "fooey",            true,  0, 5, 0, 0 },
    { "foo fooey bar", "bar fooey",        true,  0, 3, 4, 9 },
    { "blah",          "blah",             true,  0, 4, 0, 0 },
    { "blah",          "foo",              false, 0, 0, 0, 0 },
    { "blah",          "blahblah",         true,  0, 4, 0, 0 },
    { "blah",          "foo blah",         true,  4, 8, 0, 0 },
    { "foo blah",      "blah",             false, 0, 0, 0, 0 },
    { "foo blah",      "blahx foobar",     true,  0, 4, 6, 9 },
    { "\"foo blah\"",  "foo blah",         true,  0, 8, 0, 0 },
    { "\"foo blah\"",  "foox blahx",       false, 0, 0, 0, 0 },
    { "\"foo blah\"",  "foo blah",         true,  0, 8, 0, 0 },
    { "\"foo blah\"",  "\"foo blah\"",     true,  1, 9, 0, 0 },
    { "foo blah",      "\"foo bar blah\"", true,  1, 4, 9, 13 },
  };
  for (size_t i = 0; i < ARRAYSIZE_UNSAFE(data); ++i) {
    QueryParser parser;
    ScopedVector<QueryNode> query_nodes;
    parser.ParseQuery(UTF8ToUTF16(data[i].query), &query_nodes.get());
    Snippet::MatchPositions match_positions;
    ASSERT_EQ(data[i].matches,
              parser.DoesQueryMatch(UTF8ToUTF16(data[i].text),
                                    query_nodes.get(),
                                    &match_positions));
    size_t offset = 0;
    if (data[i].m1_start != 0 || data[i].m1_end != 0) {
      ASSERT_TRUE(match_positions.size() >= 1);
      EXPECT_EQ(data[i].m1_start, match_positions[0].first);
      EXPECT_EQ(data[i].m1_end, match_positions[0].second);
      offset++;
    }
    if (data[i].m2_start != 0 || data[i].m2_end != 0) {
      ASSERT_TRUE(match_positions.size() == 1 + offset);
      EXPECT_EQ(data[i].m2_start, match_positions[offset].first);
      EXPECT_EQ(data[i].m2_end, match_positions[offset].second);
    }
  }
}

TEST_F(QueryParserTest, ExtractQueryWords) {
  struct TestData2 {
    const std::string text;
    const std::string w1;
    const std::string w2;
    const std::string w3;
    const size_t word_count;
  } data[] = {
    { "foo",           "foo", "",    "",  1 },
    { "foo bar",       "foo", "bar", "",  2 },
    { "\"foo bar\"",   "foo", "bar", "",  2 },
    { "\"foo bar\" a", "foo", "bar", "a", 3 },
  };
  for (size_t i = 0; i < ARRAYSIZE_UNSAFE(data); ++i) {
    std::vector<string16> results;
    QueryParser parser;
    parser.ExtractQueryWords(UTF8ToUTF16(data[i].text), &results);
    ASSERT_EQ(data[i].word_count, results.size());
    EXPECT_EQ(data[i].w1, UTF16ToUTF8(results[0]));
    if (results.size() == 2)
      EXPECT_EQ(data[i].w2, UTF16ToUTF8(results[1]));
    if (results.size() == 3)
      EXPECT_EQ(data[i].w3, UTF16ToUTF8(results[2]));
  }
}
