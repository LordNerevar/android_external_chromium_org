// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_ACCOUNT_SCREEN_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_ACCOUNT_SCREEN_H_

#include <string>

#include "chrome/browser/chromeos/login/account_creation_view.h"
#include "chrome/browser/chromeos/login/view_screen.h"
#include "chrome/browser/tab_contents/tab_contents_delegate.h"

class WizardScreenDelegate;

namespace chromeos {

class AccountScreen : public ViewScreen<AccountCreationView>,
                      public TabContentsDelegate,
                      public AccountCreationViewDelegate {
 public:
  explicit AccountScreen(WizardScreenDelegate* delegate);
  virtual ~AccountScreen();

  // AccountCreationViewDelegate implementation:
  virtual void OnUserCreated(const std::string& username,
                             const std::string& password);
  virtual void OnPageLoadFailed(const std::string& url);

 private:
  // ViewScreen implementation:
  virtual void CreateView();
  virtual AccountCreationView* AllocateView();

  // TabContentsDelegate implementation:
  virtual void OpenURLFromTab(TabContents* source,
                              const GURL& url, const GURL& referrer,
                              WindowOpenDisposition disposition,
                              PageTransition::Type transition) {}
  virtual void NavigationStateChanged(const TabContents* source,
                                      unsigned changed_flags);
  virtual void AddNewContents(TabContents* source,
                              TabContents* new_contents,
                              WindowOpenDisposition disposition,
                              const gfx::Rect& initial_pos,
                              bool user_gesture) {}
  virtual void ActivateContents(TabContents* contents) {}
  virtual void LoadingStateChanged(TabContents* source);
  virtual void CloseContents(TabContents* source) {}
  virtual bool IsPopup(TabContents* source) { return false; }
  virtual void URLStarredChanged(TabContents* source, bool starred) {}
  virtual void UpdateTargetURL(TabContents* source, const GURL& url) {}
  virtual bool ShouldAddNavigationToHistory() const { return false; }
  virtual void MoveContents(TabContents* source, const gfx::Rect& pos) {}
  virtual void ToolbarSizeChanged(TabContents* source, bool is_animating) {}
  virtual bool HandleContextMenu(const ContextMenuParams& params);

  DISALLOW_COPY_AND_ASSIGN(AccountScreen);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_ACCOUNT_SCREEN_H_
