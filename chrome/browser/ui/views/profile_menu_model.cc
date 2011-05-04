// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/profile_menu_model.h"

#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/sync/profile_sync_service.h"
#include "grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/models/accelerator.h"
#include "ui/base/models/simple_menu_model.h"
#include "views/controls/menu/menu_2.h"

ProfileMenuModel::ProfileMenuModel()
    : ALLOW_THIS_IN_INITIALIZER_LIST(ui::SimpleMenuModel(this)) {
  AddItem(COMMAND_CREATE_NEW_PROFILE, l10n_util::GetStringUTF16(
      IDS_PROFILES_CREATE_NEW_PROFILE_OPTION));
  menu_.reset(new views::Menu2(this));
}

ProfileMenuModel::~ProfileMenuModel() {
}

void ProfileMenuModel::RunMenuAt(const gfx::Point& point) {
  menu_->RunMenuAt(point, views::Menu2::ALIGN_TOPRIGHT);
}

bool ProfileMenuModel::IsCommandIdChecked(int command_id) const {
  return false;
}

bool ProfileMenuModel::IsCommandIdEnabled(int command_id) const {
  return true;
}

bool ProfileMenuModel::GetAcceleratorForCommandId(int command_id,
    ui::Accelerator* accelerator) {
  return false;
}

void ProfileMenuModel::ExecuteCommand(int command_id) {
  switch (command_id) {
    case COMMAND_CREATE_NEW_PROFILE:
      ProfileManager::CreateMultiProfileAsync();
      break;
    default:
      NOTIMPLEMENTED();
      break;
  }
}
