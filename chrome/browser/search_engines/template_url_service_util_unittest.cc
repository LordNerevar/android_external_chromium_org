// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/scoped_vector.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/search_engines/template_url.h"
#include "chrome/browser/search_engines/template_url_service.h"
#include "chrome/browser/search_engines/util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

// Creates a TemplateURL with default values except for the prepopulate ID,
// keyword and TemplateURLID. Only use this in tests if your tests do not
// care about other fields. The caller is the owner of this TemplateURL.
TemplateURL* CreatePrepopulateTemplateURL(int prepopulate_id,
                                          const std::string& keyword,
                                          TemplateURLID id) {
  TemplateURLData data;
  data.prepopulate_id = prepopulate_id;
  data.SetKeyword(base::ASCIIToUTF16(keyword));
  data.id = id;
  return new TemplateURL(NULL, data);
}

};  // namespace

TEST(TemplateURLServiceUtilTest, RemoveDuplicatePrepopulateIDs) {
  ScopedVector<TemplateURL> prepopulated_turls;
  TemplateURLService::TemplateURLVector local_turls;
  STLElementDeleter<TemplateURLService::TemplateURLVector> local_turls_deleter(
      &local_turls);

  prepopulated_turls.push_back(CreatePrepopulateTemplateURL(1, "winner4", 1));
  prepopulated_turls.push_back(CreatePrepopulateTemplateURL(2, "xxx", 2));
  prepopulated_turls.push_back(CreatePrepopulateTemplateURL(3, "yyy", 3));

  // Create a sets of different TURLs grouped by prepopulate ID. Each group
  // will test a different heuristic of RemoveDuplicatePrepopulateIDs.
  // Ignored set - These should be left alone as they do not have valid
  // prepopulate IDs.
  local_turls.push_back(CreatePrepopulateTemplateURL(0, "winner1", 4));
  local_turls.push_back(CreatePrepopulateTemplateURL(0, "winner2", 5));
  local_turls.push_back(CreatePrepopulateTemplateURL(0, "winner3", 6));
  size_t num_non_prepopulated_urls = local_turls.size();

  // Keyword match set - Prefer the one that matches the keyword of the
  // prepopulate ID.
  local_turls.push_back(CreatePrepopulateTemplateURL(1, "loser1", 7));
  local_turls.push_back(CreatePrepopulateTemplateURL(1, "loser2", 8));
  local_turls.push_back(CreatePrepopulateTemplateURL(1, "winner4", 9));

  // Default set - Prefer the default search engine over all other criteria.
  // The last one is the default. It will be passed as the
  // default_search_provider parameter to RemoveDuplicatePrepopulateIDs.
  local_turls.push_back(CreatePrepopulateTemplateURL(2, "loser3", 10));
  local_turls.push_back(CreatePrepopulateTemplateURL(2, "xxx", 11));
  TemplateURL* default_turl = CreatePrepopulateTemplateURL(2, "winner5", 12);
  local_turls.push_back(default_turl);

  // ID set - Prefer the lowest TemplateURLID if the keywords don't match and if
  // none are the default.
  local_turls.push_back(CreatePrepopulateTemplateURL(3, "winner6", 13));
  local_turls.push_back(CreatePrepopulateTemplateURL(3, "loser5", 14));
  local_turls.push_back(CreatePrepopulateTemplateURL(3, "loser6", 15));

  RemoveDuplicatePrepopulateIDs(NULL, prepopulated_turls, default_turl,
      &local_turls, NULL);

  // Verify that the expected local TURLs survived the process.
  EXPECT_EQ(local_turls.size(),
            prepopulated_turls.size() + num_non_prepopulated_urls);
  for (TemplateURLService::TemplateURLVector::const_iterator itr =
       local_turls.begin(); itr != local_turls.end(); ++itr) {
    EXPECT_TRUE(
        StartsWith((*itr)->keyword(), base::ASCIIToUTF16("winner"), true));
  }
}
