// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_LANGUAGE_USAGE_METRICS_H_
#define CHROME_BROWSER_LANGUAGE_USAGE_METRICS_H_

#include <set>
#include <string>

#include "base/gtest_prod_util.h"

// Methods to record language usage as UMA histograms.
// Language codes are defined in third_party/cld/languages/proto/languages.pb.h
class LanguageUsageMetrics {
 public:
  // Records accept languages as a UMA histogram. |accept_languages| is a
  // case-insensitive comma-separated list of languages/locales of either xx,
  // xx-YY, or xx_YY format where xx is iso-639 language code and YY is iso-3166
  // country code. Country code is ignored. That is, xx and XX-YY are considered
  // identical and recorded once.
  static void RecordAcceptLanguages(const std::string& accept_languages);

  // Records the application language as a UMA histogram. |application_locale|
  // is a case-insensitive locale string of either xx, xx-YY, or xx_YY format.
  // Only the language part (xx in the example) is considered.
  static void RecordApplicationLanguage(const std::string& application_locale);

 private:
  // This class must not be instantiated.
  LanguageUsageMetrics();

  // Parses |accept_languages| and returns a set of language codes in
  // |languages|.
  static void ParseAcceptLanguages(const std::string& accept_languages,
                                   std::set<int>* languages);

  // Parses |locale| and returns the language code. Returns 0 in case of errors.
  // The language code is calculated from two alphabets. For example, if
  // |locale| is 'en' which represents 'English', the codes of 'e' and 'n' are
  // 101 and 110 respectively, and the language code will be 101 * 256 + 100 =
  // 25966.
  // |locale| should consist of only lower-case letters. This function doesn't
  // check whether |locale| is valid locale or not strictly.
  static int ToLanguageCode(const std::string &locale);

  FRIEND_TEST_ALL_PREFIXES(LanguageUsageMetricsTest, ParseAcceptLanguages);
  FRIEND_TEST_ALL_PREFIXES(LanguageUsageMetricsTest, ToLanguageCode);
};

#endif  // CHROME_BROWSER_LANGUAGE_USAGE_METRICS_H_
