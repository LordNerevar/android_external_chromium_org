// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/gtk/options/content_exception_editor.h"

#include "app/l10n_util.h"
#include "app/resource_bundle.h"
#include "base/message_loop.h"
#include "chrome/browser/content_exceptions_table_model.h"
#include "chrome/browser/gtk/gtk_util.h"
#include "chrome/browser/host_content_settings_map.h"
#include "googleurl/src/url_canon.h"
#include "googleurl/src/url_parse.h"
#include "grit/app_resources.h"
#include "grit/generated_resources.h"
#include "net/base/net_util.h"

namespace {

// The settings shown in the combobox if show_ask_ is false;
const ContentSetting kNoAskSettings[] = { CONTENT_SETTING_ALLOW,
                                          CONTENT_SETTING_BLOCK };

// The settings shown in the combobox if show_ask_ is true;
const ContentSetting kAskSettings[] = { CONTENT_SETTING_ALLOW,
                                        CONTENT_SETTING_ASK,
                                        CONTENT_SETTING_BLOCK };

// Returns true if the host name is valid.
bool ValidHost(const std::string& host) {
  if (host.empty())
    return false;

  url_canon::CanonHostInfo host_info;
  return !net::CanonicalizeHost(host, &host_info).empty();
}

GtkWidget* CreateEntryImageHBox(GtkWidget* entry, GtkWidget* image) {
  GtkWidget* hbox = gtk_hbox_new(FALSE, gtk_util::kControlSpacing);
  gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
  return hbox;
}

}  // namespace

ContentExceptionEditor::ContentExceptionEditor(
    GtkWindow* parent,
    ContentExceptionEditor::Delegate* delegate,
    ContentExceptionsTableModel* model,
    int index,
    const std::string& host,
    ContentSetting setting)
    : delegate_(delegate),
      model_(model),
      show_ask_(model->content_type() == CONTENT_SETTINGS_TYPE_COOKIES),
      index_(index),
      host_(host),
      setting_(setting) {
  dialog_ = gtk_dialog_new_with_buttons(
      l10n_util::GetStringUTF8(is_new() ?
                               IDS_EXCEPTION_EDITOR_NEW_TITLE :
                               IDS_EXCEPTION_EDITOR_TITLE).c_str(),
      parent,
      // Non-modal.
      static_cast<GtkDialogFlags>(GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR),
      GTK_STOCK_CANCEL,
      GTK_RESPONSE_CANCEL,
      GTK_STOCK_OK,
      GTK_RESPONSE_OK,
      NULL);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog_), GTK_RESPONSE_OK);

  entry_ = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(entry_), host_.c_str());
  g_signal_connect(entry_, "changed", G_CALLBACK(OnEntryChanged), this);
  gtk_entry_set_activates_default(GTK_ENTRY(entry_), TRUE);

  host_image_ = gtk_image_new_from_pixbuf(NULL);

  action_combo_ = gtk_combo_box_new_text();
  for (int i = 0; i < GetItemCount(); ++i) {
    gtk_combo_box_append_text(GTK_COMBO_BOX(action_combo_),
                              GetTitleFor(i).c_str());
  }
  gtk_combo_box_set_active(GTK_COMBO_BOX(action_combo_),
                           IndexForSetting(setting_));

  GtkWidget* table = gtk_util::CreateLabeledControlsGroup(
      NULL,
      l10n_util::GetStringUTF8(IDS_EXCEPTION_EDITOR_HOST_TITLE).c_str(),
      CreateEntryImageHBox(entry_, host_image_),
      l10n_util::GetStringUTF8(IDS_EXCEPTION_EDITOR_ACTION_TITLE).c_str(),
      action_combo_,
      NULL);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog_)->vbox), table,
                     FALSE, FALSE, 0);

  gtk_box_set_spacing(GTK_BOX(GTK_DIALOG(dialog_)->vbox),
                      gtk_util::kContentAreaSpacing);

  // Prime the state of the buttons.
  OnEntryChanged(GTK_EDITABLE(entry_), this);

  gtk_widget_show_all(dialog_);

  g_signal_connect(dialog_, "response", G_CALLBACK(OnResponse), this);
  g_signal_connect(dialog_, "destroy", G_CALLBACK(OnWindowDestroy), this);
}

int ContentExceptionEditor::GetItemCount() {
  return show_ask_ ? arraysize(kAskSettings) : arraysize(kNoAskSettings);
}

std::string ContentExceptionEditor::GetTitleFor(int index) {
  switch (SettingForIndex(index)) {
    case CONTENT_SETTING_ALLOW:
      return l10n_util::GetStringUTF8(IDS_EXCEPTIONS_ALLOW_BUTTON);
    case CONTENT_SETTING_BLOCK:
      return l10n_util::GetStringUTF8(IDS_EXCEPTIONS_BLOCK_BUTTON);
    case CONTENT_SETTING_ASK:
      return l10n_util::GetStringUTF8(IDS_EXCEPTIONS_ASK_BUTTON);
    default:
      NOTREACHED();
  }
  return std::string();
}

ContentSetting ContentExceptionEditor::SettingForIndex(int index) {
  return show_ask_ ? kAskSettings[index] : kNoAskSettings[index];
}

int ContentExceptionEditor::IndexForSetting(ContentSetting setting) {
  for (int i = 0; i < GetItemCount(); ++i)
    if (SettingForIndex(i) == setting)
      return i;
  NOTREACHED();
  return 0;
}

bool ContentExceptionEditor::IsHostValid(const std::string& host) const {
  bool is_valid_host = ValidHost(host) &&
      (model_->IndexOfExceptionByHost(host) == -1);

  return is_new() ? is_valid_host : (!host.empty() &&
      ((host_ == host) || is_valid_host));
}

void ContentExceptionEditor::UpdateImage(GtkWidget* image, bool is_valid) {
  return gtk_image_set_from_pixbuf(GTK_IMAGE(image),
      ResourceBundle::GetSharedInstance().GetPixbufNamed(
          is_valid ? IDR_INPUT_GOOD : IDR_INPUT_ALERT));
}

// static
void ContentExceptionEditor::OnEntryChanged(GtkEditable* entry,
                                            ContentExceptionEditor* window) {
  std::string new_host = gtk_entry_get_text(GTK_ENTRY(window->entry_));
  bool is_valid = window->IsHostValid(new_host);
  gtk_dialog_set_response_sensitive(GTK_DIALOG(window->dialog_),
                                    GTK_RESPONSE_OK, is_valid);
  window->UpdateImage(window->host_image_, is_valid);
}

// static
void ContentExceptionEditor::OnResponse(
    GtkWidget* sender,
    int response_id,
    ContentExceptionEditor* window) {
  if (response_id == GTK_RESPONSE_OK) {
    // Notify our delegate to update everything.
    std::string new_host = gtk_entry_get_text(GTK_ENTRY(window->entry_));
    ContentSetting setting = window->SettingForIndex(gtk_combo_box_get_active(
        GTK_COMBO_BOX(window->action_combo_)));
    window->delegate_->AcceptExceptionEdit(new_host, setting, window->index_,
                                           window->is_new());
  }

  gtk_widget_destroy(window->dialog_);
}

// static
void ContentExceptionEditor::OnWindowDestroy(
    GtkWidget* widget,
    ContentExceptionEditor* editor) {
  MessageLoop::current()->DeleteSoon(FROM_HERE, editor);
}
