// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_SYSTEM_WEB_NOTIFICATION_WEB_NOTIFICATION_TRAY_H_
#define ASH_SYSTEM_WEB_NOTIFICATION_WEB_NOTIFICATION_TRAY_H_

#include "ash/ash_export.h"
#include "ash/system/tray/tray_background_view.h"
#include "ash/system/user/login_status.h"
#include "base/gtest_prod_util.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "ui/message_center/message_center_tray.h"
#include "ui/message_center/message_center_tray_delegate.h"
#include "ui/views/bubble/tray_bubble_view.h"
#include "ui/views/controls/button/button.h"

// Status area tray for showing browser and app notifications. This hosts
// a MessageCenter class which manages the notification list. This class
// contains the Ash specific tray implementation.
//
// Note: These are not related to system notifications (i.e NotificationView
// generated by SystemTrayItem). Visibility of one notification type or other
// is controlled by StatusAreaWidget.

namespace views {
class ImageButton;
class MenuRunner;
}

namespace message_center {
class MessageBubbleBase;
class MessageCenter;
class MessageCenterBubble;
class MessagePopupCollection;
}

namespace ash {
namespace internal {
class StatusAreaWidget;
class WebNotificationBubbleWrapper;
class WebNotificationButton;
class WorkAreaObserver;
}

class ASH_EXPORT WebNotificationTray
    : public internal::TrayBackgroundView,
      public views::TrayBubbleView::Delegate,
      public message_center::MessageCenterTrayDelegate,
      public views::ButtonListener,
      public base::SupportsWeakPtr<WebNotificationTray> {
 public:
  explicit WebNotificationTray(
      internal::StatusAreaWidget* status_area_widget);
  virtual ~WebNotificationTray();

  // Set whether or not the popup notifications should be hidden.
  void SetHidePopupBubble(bool hide);

  // Updates tray visibility login status of the system changes.
  void UpdateAfterLoginStatusChange(user::LoginStatus login_status);

  // Returns true if it should block the auto hide behavior of the launcher.
  bool ShouldBlockLauncherAutoHide() const;

  // Returns true if the message center bubble is visible.
  bool IsMessageCenterBubbleVisible() const;

  // Returns true if the mouse is inside the notification bubble.
  bool IsMouseInNotificationBubble() const;

  // Shows the message center bubble.
  void ShowMessageCenterBubble();

  // Overridden from TrayBackgroundView.
  virtual void SetShelfAlignment(ShelfAlignment alignment) OVERRIDE;
  virtual void AnchorUpdated() OVERRIDE;
  virtual base::string16 GetAccessibleNameForTray() OVERRIDE;
  virtual void HideBubbleWithView(
      const views::TrayBubbleView* bubble_view) OVERRIDE;
  virtual bool ClickedOutsideBubble() OVERRIDE;

  // Overridden from internal::ActionableView.
  virtual bool PerformAction(const ui::Event& event) OVERRIDE;

  // Overridden from views::TrayBubbleView::Delegate.
  virtual void BubbleViewDestroyed() OVERRIDE;
  virtual void OnMouseEnteredView() OVERRIDE;
  virtual void OnMouseExitedView() OVERRIDE;
  virtual base::string16 GetAccessibleNameForBubble() OVERRIDE;
  virtual gfx::Rect GetAnchorRect(views::Widget* anchor_widget,
                                  AnchorType anchor_type,
                                  AnchorAlignment anchor_alignment) OVERRIDE;
  virtual void HideBubble(const views::TrayBubbleView* bubble_view) OVERRIDE;

  // Overridden from ButtonListener.
  virtual void ButtonPressed(views::Button* sender,
                             const ui::Event& event) OVERRIDE;

  // Overridden from MessageCenterTrayDelegate.
  virtual void OnMessageCenterTrayChanged() OVERRIDE;
  virtual bool ShowMessageCenter() OVERRIDE;
  virtual void HideMessageCenter() OVERRIDE;
  virtual bool ShowPopups() OVERRIDE;
  virtual void HidePopups() OVERRIDE;
  virtual bool ShowNotifierSettings() OVERRIDE;

  // Overridden from TrayBackgroundView.
  virtual bool IsPressed() OVERRIDE;

  message_center::MessageCenter* message_center();

 private:
  FRIEND_TEST_ALL_PREFIXES(WebNotificationTrayTest, WebNotifications);
  FRIEND_TEST_ALL_PREFIXES(WebNotificationTrayTest, WebNotificationPopupBubble);
  FRIEND_TEST_ALL_PREFIXES(WebNotificationTrayTest,
                           ManyMessageCenterNotifications);
  FRIEND_TEST_ALL_PREFIXES(WebNotificationTrayTest, ManyPopupNotifications);

  void UpdateTrayContent();

  // The actual process to show the message center. Set |show_settings| to true
  // if the message center should be initialized with the settings visible.
  // Returns true if the center is successfully created.
  bool ShowMessageCenterInternal(bool show_settings);

  // Queries login status and the status area widget to determine visibility of
  // the message center.
  bool ShouldShowMessageCenter();

  // Returns true if it should show the quiet mode menu.
  bool ShouldShowQuietModeMenu(const ui::Event& event);

  // Shows the quiet mode menu.
  void ShowQuietModeMenu(const ui::Event& event);

  internal::WebNotificationBubbleWrapper* message_center_bubble() const {
    return message_center_bubble_.get();
  }

  // Testing accessors.
  bool IsPopupVisible() const;
  message_center::MessageCenterBubble* GetMessageCenterBubbleForTest();

  scoped_ptr<message_center::MessageCenterTray> message_center_tray_;
  scoped_ptr<internal::WebNotificationBubbleWrapper> message_center_bubble_;
  scoped_ptr<message_center::MessagePopupCollection> popup_collection_;
  scoped_ptr<views::MenuRunner> quiet_mode_menu_runner_;
  internal::WebNotificationButton* button_;

  bool show_message_center_on_unlock_;

  bool should_update_tray_content_;

  // True when the shelf auto hide behavior has to be blocked. Previously
  // this was done by checking |message_center_bubble_| but actually
  // the check can be called when creating this object, so it would cause
  // flickers of the shelf from hidden to shown. See: crbug.com/181213
  bool should_block_shelf_auto_hide_;

  // Observes the work area for |popup_collection_| and notifies to it.
  scoped_ptr<internal::WorkAreaObserver> work_area_observer_;

  DISALLOW_COPY_AND_ASSIGN(WebNotificationTray);
};

}  // namespace ash

#endif  // ASH_SYSTEM_WEB_NOTIFICATION_WEB_NOTIFICATION_TRAY_H_
