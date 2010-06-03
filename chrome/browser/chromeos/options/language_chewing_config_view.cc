// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/options/language_chewing_config_view.h"

#include "app/combobox_model.h"
#include "app/l10n_util.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/chromeos/cros/cros_library.h"
#include "chrome/browser/chromeos/cros/language_library.h"
#include "chrome/browser/chromeos/options/language_config_util.h"
#include "chrome/browser/chromeos/preferences.h"
#include "chrome/browser/profile.h"
#include "chrome/common/notification_type.h"
#include "chrome/common/pref_names.h"
#include "grit/generated_resources.h"
#include "grit/locale_settings.h"
#include "views/controls/button/checkbox.h"
#include "views/controls/label.h"
#include "views/grid_layout.h"
#include "views/standard_layout.h"
#include "views/window/window.h"

namespace chromeos {

LanguageChewingConfigView::LanguageChewingConfigView(Profile* profile)
    : OptionsPageView(profile), contents_(NULL) {
  for (size_t i = 0; i < kNumChewingBooleanPrefs; ++i) {
    chewing_boolean_prefs_[i].Init(
        kChewingBooleanPrefs[i].pref_name, profile->GetPrefs(), this);
    chewing_boolean_checkboxes_[i] = NULL;
  }
  for (size_t i = 0; i < kNumChewingMultipleChoicePrefs; ++i) {
    ChewingPrefAndAssociatedCombobox& current = prefs_and_comboboxes_[i];
    current.multiple_choice_pref.Init(
        kChewingMultipleChoicePrefs[i].pref_name, profile->GetPrefs(), this);
    current.combobox_model =
        new LanguageComboboxModel(&kChewingMultipleChoicePrefs[i]);
    current.combobox = NULL;
  }
  for (size_t i = 0; i < kNumChewingIntegerPrefs; ++i) {
    chewing_integer_prefs_[i].Init(
        kChewingIntegerPrefs[i].pref_name, profile->GetPrefs(), this);
    chewing_integer_sliders_[i] = NULL;
  }
}

LanguageChewingConfigView::~LanguageChewingConfigView() {
}

void LanguageChewingConfigView::ButtonPressed(
    views::Button* sender, const views::Event& event) {
  views::Checkbox* checkbox = static_cast<views::Checkbox*>(sender);
  const int pref_id = checkbox->tag();
  DCHECK(pref_id >= 0 && pref_id < static_cast<int>(kNumChewingBooleanPrefs));
  chewing_boolean_prefs_[pref_id].SetValue(checkbox->checked());
}

void LanguageChewingConfigView::ItemChanged(
    views::Combobox* sender, int prev_index, int new_index) {
  for (size_t i = 0; i < kNumChewingMultipleChoicePrefs; ++i) {
    ChewingPrefAndAssociatedCombobox& current = prefs_and_comboboxes_[i];
    if (current.combobox == sender) {
      const std::wstring config_value =
          current.combobox_model->GetConfigValueAt(new_index);
      LOG(INFO) << "Changing Chewing pref to " << config_value;
      // Update the Chrome pref.
      current.multiple_choice_pref.SetValue(config_value);
      break;
    }
  }
}

void LanguageChewingConfigView::SliderValueChanged(views::Slider* sender) {
  size_t pref_id;
  for (pref_id = 0; pref_id < kNumChewingIntegerPrefs; ++pref_id) {
    if (chewing_integer_sliders_[pref_id] == sender)
      break;
  }
  DCHECK(pref_id < kNumChewingIntegerPrefs);
  chewing_integer_prefs_[pref_id].SetValue(sender->value());
}


void LanguageChewingConfigView::Layout() {
  // Not sure why but this is needed to show contents in the dialog.
  contents_->SetBounds(0, 0, width(), height());
}

std::wstring LanguageChewingConfigView::GetWindowTitle() const {
  return l10n_util::GetString(
      IDS_OPTIONS_SETTINGS_LANGUAGES_CHEWING_SETTINGS_TITLE);
}

gfx::Size LanguageChewingConfigView::GetPreferredSize() {
  return gfx::Size(views::Window::GetLocalizedContentsSize(
      IDS_LANGUAGES_INPUT_DIALOG_WIDTH_CHARS,
      IDS_LANGUAGES_INPUT_DIALOG_HEIGHT_LINES));
}

void LanguageChewingConfigView::InitControlLayout() {
  using views::ColumnSet;
  using views::GridLayout;

  contents_ = new views::View;
  AddChildView(contents_);

  GridLayout* layout = new GridLayout(contents_);
  layout->SetInsets(kPanelVertMargin, kPanelHorizMargin,
                    kPanelVertMargin, kPanelHorizMargin);
  contents_->SetLayoutManager(layout);

  const int kColumnSetId = 0;
  ColumnSet* column_set = layout->AddColumnSet(kColumnSetId);
  column_set->AddColumn(GridLayout::LEADING, GridLayout::CENTER, 0,
                        GridLayout::USE_PREF, 0, 0);
  column_set->AddPaddingColumn(0, kRelatedControlHorizontalSpacing);
  column_set->AddColumn(GridLayout::FILL, GridLayout::FILL, 0,
                        GridLayout::USE_PREF, 0, 0);

  for (size_t i = 0; i < kNumChewingBooleanPrefs; ++i) {
    chewing_boolean_checkboxes_[i] = new views::Checkbox(
        l10n_util::GetString(kChewingBooleanPrefs[i].message_id));
    chewing_boolean_checkboxes_[i]->set_listener(this);
    chewing_boolean_checkboxes_[i]->set_tag(i);
  }
  for (size_t i = 0; i < kNumChewingMultipleChoicePrefs; ++i) {
    ChewingPrefAndAssociatedCombobox& current = prefs_and_comboboxes_[i];
    current.combobox = new LanguageCombobox(current.combobox_model);
    current.combobox->set_listener(this);
  }
  for (size_t i = 0; i < kNumChewingIntegerPrefs; ++i) {
    chewing_integer_sliders_[i] = new views::Slider(
        kChewingIntegerPrefs[i].min_pref_value,
        kChewingIntegerPrefs[i].max_pref_value,
        1,
        static_cast<views::Slider::StyleFlags>(
            views::Slider::STYLE_DRAW_VALUE |
            views::Slider::STYLE_UPDATE_ON_RELEASE),
        this);
  }
  for (size_t i = 0; i < kNumChewingBooleanPrefs; ++i) {
    layout->StartRow(0, kColumnSetId);
    layout->AddView(chewing_boolean_checkboxes_[i]);
  }

  for (size_t i = 0; i < kNumChewingIntegerPrefs; ++i) {
    layout->StartRow(0, kColumnSetId);
    layout->AddView(new views::Label(
        l10n_util::GetString(kChewingIntegerPrefs[i].message_id)));
    layout->AddView(chewing_integer_sliders_[i]);
  }
  NotifyPrefChanged();

  // Show the comboboxes.
  for (size_t i = 0; i < kNumChewingMultipleChoicePrefs; ++i) {
    const ChewingPrefAndAssociatedCombobox& current = prefs_and_comboboxes_[i];
    layout->StartRow(0, kColumnSetId);
    layout->AddView(new views::Label(current.combobox_model->GetLabel()));
    layout->AddView(current.combobox);
  }
}

void LanguageChewingConfigView::Observe(NotificationType type,
                                        const NotificationSource& source,
                                        const NotificationDetails& details) {
  if (type == NotificationType::PREF_CHANGED) {
    NotifyPrefChanged();
  }
}

void LanguageChewingConfigView::NotifyPrefChanged() {
  for (size_t i = 0; i < kNumChewingBooleanPrefs; ++i) {
    const bool checked = chewing_boolean_prefs_[i].GetValue();
    chewing_boolean_checkboxes_[i]->SetChecked(checked);
  }
  for (size_t i = 0; i < kNumChewingIntegerPrefs; ++i) {
    const int value = chewing_integer_prefs_[i].GetValue();
    chewing_integer_sliders_[i]->SetValue(value);
  }
  for (size_t i = 0; i < kNumChewingMultipleChoicePrefs; ++i) {
    ChewingPrefAndAssociatedCombobox& current = prefs_and_comboboxes_[i];
    const std::wstring value = current.multiple_choice_pref.GetValue();
    const int combo_index =
        current.combobox_model->GetIndexFromConfigValue(value);
    if (combo_index >= 0) {
      current.combobox->SetSelectedItem(combo_index);
    }
  }
}

}  // namespace chromeos
