// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_SYSTEM_USER_TRAY_USER_H_
#define ASH_SYSTEM_USER_TRAY_USER_H_
#pragma once

#include "ash/system/tray/system_tray_item.h"
#include "ash/system/user/update_controller.h"

namespace ash {
namespace internal {

namespace tray {
class UserView;
}

class TrayUser : public SystemTrayItem,
                 public UpdateController {
 public:
  TrayUser();
  virtual ~TrayUser();

 private:
  // Overridden from SystemTrayItem
  virtual views::View* CreateTrayView(user::LoginStatus status) OVERRIDE;
  virtual views::View* CreateDefaultView(user::LoginStatus status) OVERRIDE;
  virtual views::View* CreateDetailedView(user::LoginStatus status) OVERRIDE;
  virtual void DestroyTrayView() OVERRIDE;
  virtual void DestroyDefaultView() OVERRIDE;
  virtual void DestroyDetailedView() OVERRIDE;

  // Overridden from UpdateController.
  virtual void OnUpdateRecommended() OVERRIDE;

  scoped_ptr<tray::UserView> user_;

  DISALLOW_COPY_AND_ASSIGN(TrayUser);
};

}  // namespace internal
}  // namespace ash

#endif  // ASH_SYSTEM_USER_TRAY_USER_H_
