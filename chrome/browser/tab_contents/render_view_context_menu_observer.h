// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_TAB_CONTENTS_RENDER_VIEW_CONTEXT_MENU_OBSERVER_H_
#define CHROME_BROWSER_TAB_CONTENTS_RENDER_VIEW_CONTEXT_MENU_OBSERVER_H_

namespace content {
struct ContextMenuParams;
}

// The interface used for implementing context-menu items. The following
// instruction describe how to implement a context-menu item with this
// interface.
//
// 1. Add command IDs for the context-menu items to 'chrome_command_ids.h'.
//
//   #define IDC_MY_COMMAND 99999
//
// 2. Add strings for the context-menu items to 'generated_sources.grd'.
//
//   <message name="IDS_MY_COMMAND" desc="...">
//     My command
//   </message>
//
// 3. Create a class that implements this interface. (It is a good idea to use
// the RenderViewContextMenuDelegate interface to avoid accessing the
// RenderViewContextMenu class directly.)
//
//  class MyMenuObserver : public RenderViewContextMenuObserver {
//   public:
//    MyMenuObserver(RenderViewContextMenuDelegate* d);
//    ~MyMenuObserver();
//
//    virtual void InitMenu(const content::ContextMenuParams& params) OVERRIDE;
//    virtual bool IsCommandIdSupported(int command_id) OVERRIDE;
//    virtual bool IsCommandIdEnabled(int command_id) OVERRIDE;
//    virtual void ExecuteCommand(int command_id) OVERRIDE;
//
//   private:
//    RenderViewContextMenuDelgate* delegate_;
//  }
//
//  void MyMenuObserver::InitMenu(const content::ContextMenuParams& params) {
//    delegate_->AddMenuItem(IDC_MY_COMMAND,...);
//  }
//
//  bool MyMenuObserver::IsCommandIdSupported(int command_id) {
//    return command_id == IDC_MY_COMMAND;
//  }
//
//  bool MyMenuObserver::IsCommandIdEnabled(int command_id) {
//    DCHECK(command_id == IDC_MY_COMMAND);
//    return true;
//  }
//
//  void MyMenuObserver::ExecuteCommand(int command_id) {
//    DCHECK(command_id == IDC_MY_COMMAND);
//  }
//
// 4. Add this observer class to the RenderViewContextMenu class. (It is good
// to use scoped_ptr<> so Chrome can create its instances only when it needs.)
//
//  class RenderViewContextMenu {
//    ...
//   private:
//    scoped_ptr<MyMenuObserver> my_menu_observer_;
//  };
//
// 5. Create its instance in InitMenu() and add it to the observer list of the
// RenderViewContextMenu class.
//
//  void RenderViewContextMenu::InitMenu() {
//    ...
//    my_menu_observer_.reset(new MyMenuObserver(this));
//    observers_.AddObserver(my_menu_observer_.get());
//  }
//
//
class RenderViewContextMenuObserver {
 public:
  virtual ~RenderViewContextMenuObserver() {}

  // Called when the RenderViewContextMenu class initializes a context menu. We
  // usually call RenderViewContextMenuDelegate::AddMenuItem() to add menu items
  // in this function.
  virtual void InitMenu(const content::ContextMenuParams& params);

  // Called when the RenderViewContextMenu class asks whether an observer
  // listens for the specified command ID. If this function returns true, the
  // RenderViewContextMenu class calls IsCommandIdEnabled() or ExecuteCommand().
  virtual bool IsCommandIdSupported(int command_id);

  // Called when the RenderViewContextMenu class sets the initial status of the
  // specified context-menu item. If we need to enable or disable a context-menu
  // item while showing, use RenderViewContextMenuDelegate::UpdateMenuItem().
  virtual bool IsCommandIdChecked(int command_id);
  virtual bool IsCommandIdEnabled(int command_id);

  // Called when a user selects the specified context-menu item.
  virtual void ExecuteCommand(int command_id);

  // Called when a user closes the context menu without selecting any items.
  virtual void OnMenuCancel();
};

#endif  // CHROME_BROWSER_TAB_CONTENTS_RENDER_VIEW_CONTEXT_MENU_OBSERVER_H_
