// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/input_method/candidate_window_view.h"

#include <string>

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/chromeos/input_method/candidate_view.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/views/test/views_test_base.h"
#include "ui/views/widget/widget.h"

namespace chromeos {
namespace input_method {

namespace {
const char* kSampleCandidate[] = {
  "Sample Candidate 1",
  "Sample Candidate 2",
  "Sample Candidate 3"
};
const char* kSampleAnnotation[] = {
  "Sample Annotation 1",
  "Sample Annotation 2",
  "Sample Annotation 3"
};
const char* kSampleDescriptionTitle[] = {
  "Sample Description Title 1",
  "Sample Description Title 2",
  "Sample Description Title 3",
};
const char* kSampleDescriptionBody[] = {
  "Sample Description Body 1",
  "Sample Description Body 2",
  "Sample Description Body 3",
};

void InitCandidateWindow(size_t page_size,
                         ui::CandidateWindow* candidate_window) {
  candidate_window->set_cursor_position(0);
  candidate_window->set_page_size(page_size);
  candidate_window->mutable_candidates()->clear();
  candidate_window->set_orientation(ui::CandidateWindow::VERTICAL);
}

void InitCandidateWindowWithCandidatesFilled(
    size_t page_size,
    ui::CandidateWindow* candidate_window) {
  InitCandidateWindow(page_size, candidate_window);
  for (size_t i = 0; i < page_size; ++i) {
    ui::CandidateWindow::Entry entry;
    entry.value = base::StringPrintf("value %lld",
                                     static_cast<unsigned long long>(i));
    entry.label = base::StringPrintf("%lld",
                                     static_cast<unsigned long long>(i));
    candidate_window->mutable_candidates()->push_back(entry);
  }
}

}  // namespace

class CandidateWindowViewTest : public views::ViewsTestBase {
 public:
  CandidateWindowViewTest() {}
  virtual ~CandidateWindowViewTest() {}

 protected:
  virtual void SetUp() {
    views::ViewsTestBase::SetUp();
    candidate_window_view_ = new CandidateWindowView(GetContext());
    candidate_window_view_->InitWidget();
  }

  CandidateWindowView* candidate_window_view() {
    return candidate_window_view_;
  }

  int selected_candidate_index_in_page() {
    return candidate_window_view_->selected_candidate_index_in_page_;
  }

  size_t GetCandidatesSize() const {
    return candidate_window_view_->candidate_views_.size();
  }

  CandidateView* GetCandidateAt(size_t i) {
    return candidate_window_view_->candidate_views_[i];
  }

  void SelectCandidateAt(int index_in_page) {
    candidate_window_view_->SelectCandidateAt(index_in_page);
  }

  void MaybeInitializeCandidateViews(
      const ui::CandidateWindow& candidate_window) {
    candidate_window_view_->MaybeInitializeCandidateViews(candidate_window);
  }

  void ExpectLabels(const std::string& shortcut,
                    const std::string& candidate,
                    const std::string& annotation,
                    const CandidateView* row) {
    EXPECT_EQ(shortcut, base::UTF16ToUTF8(row->shortcut_label_->text()));
    EXPECT_EQ(candidate, base::UTF16ToUTF8(row->candidate_label_->text()));
    EXPECT_EQ(annotation, base::UTF16ToUTF8(row->annotation_label_->text()));
  }

 private:
  // owned by |parent_|.
  CandidateWindowView* candidate_window_view_;

  DISALLOW_COPY_AND_ASSIGN(CandidateWindowViewTest);
};

TEST_F(CandidateWindowViewTest, UpdateCandidatesTest_CursorVisibility) {
  // Visible (by default) cursor.
  ui::CandidateWindow candidate_window;
  const int candidate_window_size = 9;
  InitCandidateWindowWithCandidatesFilled(candidate_window_size,
                                          &candidate_window);
  candidate_window_view()->UpdateCandidates(candidate_window);
  EXPECT_EQ(0, selected_candidate_index_in_page());

  // Invisible cursor.
  candidate_window.set_is_cursor_visible(false);
  candidate_window_view()->UpdateCandidates(candidate_window);
  EXPECT_EQ(-1, selected_candidate_index_in_page());

  // Move the cursor to the end.
  candidate_window.set_cursor_position(candidate_window_size - 1);
  candidate_window_view()->UpdateCandidates(candidate_window);
  EXPECT_EQ(-1, selected_candidate_index_in_page());

  // Change the cursor to visible.  The cursor must be at the end.
  candidate_window.set_is_cursor_visible(true);
  candidate_window_view()->UpdateCandidates(candidate_window);
  EXPECT_EQ(candidate_window_size - 1, selected_candidate_index_in_page());
}

TEST_F(CandidateWindowViewTest, SelectCandidateAtTest) {
  // Set 9 candidates.
  ui::CandidateWindow candidate_window_large;
  const int candidate_window_large_size = 9;
  InitCandidateWindowWithCandidatesFilled(candidate_window_large_size,
                                          &candidate_window_large);
  candidate_window_large.set_cursor_position(candidate_window_large_size - 1);
  candidate_window_view()->UpdateCandidates(candidate_window_large);

  // Select the last candidate.
  SelectCandidateAt(candidate_window_large_size - 1);

  // Reduce the number of candidates to 3.
  ui::CandidateWindow candidate_window_small;
  const int candidate_window_small_size = 3;
  InitCandidateWindowWithCandidatesFilled(candidate_window_small_size,
                                          &candidate_window_small);
  candidate_window_small.set_cursor_position(candidate_window_small_size - 1);
  // Make sure the test doesn't crash if the candidate window reduced
  // its size. (crbug.com/174163)
  candidate_window_view()->UpdateCandidates(candidate_window_small);
  SelectCandidateAt(candidate_window_small_size - 1);
}

TEST_F(CandidateWindowViewTest, ShortcutSettingTest) {
  const char* kEmptyLabel = "";
  const char* kCustomizedLabel[] = { "a", "s", "d" };
  const char* kExpectedHorizontalCustomizedLabel[] = { "a.", "s.", "d." };

  {
    SCOPED_TRACE("candidate_views allocation test");
    const size_t kMaxPageSize = 16;
    for (size_t i = 1; i < kMaxPageSize; ++i) {
      ui::CandidateWindow candidate_window;
      InitCandidateWindow(i, &candidate_window);
      candidate_window_view()->UpdateCandidates(candidate_window);
      EXPECT_EQ(i, GetCandidatesSize());
    }
  }
  {
    SCOPED_TRACE("Empty string for each labels expects empty labels(vertical)");
    const size_t kPageSize = 3;
    ui::CandidateWindow candidate_window;
    InitCandidateWindow(kPageSize, &candidate_window);

    candidate_window.set_orientation(ui::CandidateWindow::VERTICAL);
    for (size_t i = 0; i < kPageSize; ++i) {
      ui::CandidateWindow::Entry entry;
      entry.value = kSampleCandidate[i];
      entry.annotation = kSampleAnnotation[i];
      entry.description_title = kSampleDescriptionTitle[i];
      entry.description_body = kSampleDescriptionBody[i];
      entry.label = kEmptyLabel;
      candidate_window.mutable_candidates()->push_back(entry);
    }

    candidate_window_view()->UpdateCandidates(candidate_window);

    ASSERT_EQ(kPageSize, GetCandidatesSize());
    for (size_t i = 0; i < kPageSize; ++i) {
      ExpectLabels(kEmptyLabel, kSampleCandidate[i], kSampleAnnotation[i],
                   GetCandidateAt(i));
    }
  }
  {
    SCOPED_TRACE(
        "Empty string for each labels expect empty labels(horizontal)");
    const size_t kPageSize = 3;
    ui::CandidateWindow candidate_window;
    InitCandidateWindow(kPageSize, &candidate_window);

    candidate_window.set_orientation(ui::CandidateWindow::HORIZONTAL);
    for (size_t i = 0; i < kPageSize; ++i) {
      ui::CandidateWindow::Entry entry;
      entry.value = kSampleCandidate[i];
      entry.annotation = kSampleAnnotation[i];
      entry.description_title = kSampleDescriptionTitle[i];
      entry.description_body = kSampleDescriptionBody[i];
      entry.label = kEmptyLabel;
      candidate_window.mutable_candidates()->push_back(entry);
    }

    candidate_window_view()->UpdateCandidates(candidate_window);

    ASSERT_EQ(kPageSize, GetCandidatesSize());
    // Confirm actual labels not containing ".".
    for (size_t i = 0; i < kPageSize; ++i) {
      ExpectLabels(kEmptyLabel, kSampleCandidate[i], kSampleAnnotation[i],
                   GetCandidateAt(i));
    }
  }
  {
    SCOPED_TRACE("Vertical customized label case");
    const size_t kPageSize = 3;
    ui::CandidateWindow candidate_window;
    InitCandidateWindow(kPageSize, &candidate_window);

    candidate_window.set_orientation(ui::CandidateWindow::VERTICAL);
    for (size_t i = 0; i < kPageSize; ++i) {
      ui::CandidateWindow::Entry entry;
      entry.value = kSampleCandidate[i];
      entry.annotation = kSampleAnnotation[i];
      entry.description_title = kSampleDescriptionTitle[i];
      entry.description_body = kSampleDescriptionBody[i];
      entry.label = kCustomizedLabel[i];
      candidate_window.mutable_candidates()->push_back(entry);
    }

    candidate_window_view()->UpdateCandidates(candidate_window);

    ASSERT_EQ(kPageSize, GetCandidatesSize());
    // Confirm actual labels not containing ".".
    for (size_t i = 0; i < kPageSize; ++i) {
      ExpectLabels(kCustomizedLabel[i],
                   kSampleCandidate[i],
                   kSampleAnnotation[i],
                   GetCandidateAt(i));
    }
  }
  {
    SCOPED_TRACE("Horizontal customized label case");
    const size_t kPageSize = 3;
    ui::CandidateWindow candidate_window;
    InitCandidateWindow(kPageSize, &candidate_window);

    candidate_window.set_orientation(ui::CandidateWindow::HORIZONTAL);
    for (size_t i = 0; i < kPageSize; ++i) {
      ui::CandidateWindow::Entry entry;
      entry.value = kSampleCandidate[i];
      entry.annotation = kSampleAnnotation[i];
      entry.description_title = kSampleDescriptionTitle[i];
      entry.description_body = kSampleDescriptionBody[i];
      entry.label = kCustomizedLabel[i];
      candidate_window.mutable_candidates()->push_back(entry);
    }

    candidate_window_view()->UpdateCandidates(candidate_window);

    ASSERT_EQ(kPageSize, GetCandidatesSize());
    // Confirm actual labels not containing ".".
    for (size_t i = 0; i < kPageSize; ++i) {
      ExpectLabels(kExpectedHorizontalCustomizedLabel[i],
                   kSampleCandidate[i],
                   kSampleAnnotation[i],
                   GetCandidateAt(i));
    }
  }
}

TEST_F(CandidateWindowViewTest, DoNotChangeRowHeightWithLabelSwitchTest) {
  const size_t kPageSize = 10;
  ui::CandidateWindow candidate_window;
  ui::CandidateWindow no_shortcut_candidate_window;

  const char kSampleCandidate1[] = "Sample String 1";
  const char kSampleCandidate2[] = "\xE3\x81\x82";  // multi byte string.
  const char kSampleCandidate3[] = ".....";

  const char kSampleShortcut1[] = "1";
  const char kSampleShortcut2[] = "b";
  const char kSampleShortcut3[] = "C";

  const char kSampleAnnotation1[] = "Sample Annotation 1";
  const char kSampleAnnotation2[] = "\xE3\x81\x82";  // multi byte string.
  const char kSampleAnnotation3[] = "......";

  // Create CandidateWindow object.
  InitCandidateWindow(kPageSize, &candidate_window);

  candidate_window.set_cursor_position(0);
  candidate_window.set_page_size(3);
  candidate_window.mutable_candidates()->clear();
  candidate_window.set_orientation(ui::CandidateWindow::VERTICAL);
  no_shortcut_candidate_window.CopyFrom(candidate_window);

  ui::CandidateWindow::Entry entry;
  entry.value = kSampleCandidate1;
  entry.annotation = kSampleAnnotation1;
  candidate_window.mutable_candidates()->push_back(entry);
  entry.label = kSampleShortcut1;
  no_shortcut_candidate_window.mutable_candidates()->push_back(entry);

  entry.value = kSampleCandidate2;
  entry.annotation = kSampleAnnotation2;
  candidate_window.mutable_candidates()->push_back(entry);
  entry.label = kSampleShortcut2;
  no_shortcut_candidate_window.mutable_candidates()->push_back(entry);

  entry.value = kSampleCandidate3;
  entry.annotation = kSampleAnnotation3;
  candidate_window.mutable_candidates()->push_back(entry);
  entry.label = kSampleShortcut3;
  no_shortcut_candidate_window.mutable_candidates()->push_back(entry);

  int before_height = 0;

  // Test for shortcut mode to no-shortcut mode.
  // Initialize with a shortcut mode candidate window.
  MaybeInitializeCandidateViews(candidate_window);
  ASSERT_EQ(3UL, GetCandidatesSize());
  // Check the selected index is invalidated.
  EXPECT_EQ(-1, selected_candidate_index_in_page());
  before_height =
      GetCandidateAt(0)->GetContentsBounds().height();
  // Checks all entry have same row height.
  for (size_t i = 1; i < GetCandidatesSize(); ++i)
    EXPECT_EQ(before_height, GetCandidateAt(i)->GetContentsBounds().height());

  // Initialize with a no shortcut mode candidate window.
  MaybeInitializeCandidateViews(no_shortcut_candidate_window);
  ASSERT_EQ(3UL, GetCandidatesSize());
  // Check the selected index is invalidated.
  EXPECT_EQ(-1, selected_candidate_index_in_page());
  EXPECT_EQ(before_height, GetCandidateAt(0)->GetContentsBounds().height());
  // Checks all entry have same row height.
  for (size_t i = 1; i < GetCandidatesSize(); ++i)
    EXPECT_EQ(before_height, GetCandidateAt(i)->GetContentsBounds().height());

  // Test for no-shortcut mode to shortcut mode.
  // Initialize with a no shortcut mode candidate window.
  MaybeInitializeCandidateViews(no_shortcut_candidate_window);
  ASSERT_EQ(3UL, GetCandidatesSize());
  // Check the selected index is invalidated.
  EXPECT_EQ(-1, selected_candidate_index_in_page());
  before_height = GetCandidateAt(0)->GetContentsBounds().height();
  // Checks all entry have same row height.
  for (size_t i = 1; i < GetCandidatesSize(); ++i)
    EXPECT_EQ(before_height, GetCandidateAt(i)->GetContentsBounds().height());

  // Initialize with a shortcut mode candidate window.
  MaybeInitializeCandidateViews(candidate_window);
  ASSERT_EQ(3UL, GetCandidatesSize());
  // Check the selected index is invalidated.
  EXPECT_EQ(-1, selected_candidate_index_in_page());
  EXPECT_EQ(before_height, GetCandidateAt(0)->GetContentsBounds().height());
  // Checks all entry have same row height.
  for (size_t i = 1; i < GetCandidatesSize(); ++i)
    EXPECT_EQ(before_height, GetCandidateAt(i)->GetContentsBounds().height());
}
}  // namespace input_method
}  // namespace chromeos
