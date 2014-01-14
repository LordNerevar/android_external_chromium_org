// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/controls/menu/menu_model_adapter.h"

#include "base/logging.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/models/menu_model.h"
#include "ui/gfx/image/image.h"
#include "ui/views/controls/menu/submenu_view.h"
#include "ui/views/views_delegate.h"

namespace views {

MenuModelAdapter::MenuModelAdapter(ui::MenuModel* menu_model)
    : menu_model_(menu_model),
      triggerable_event_flags_(ui::EF_LEFT_MOUSE_BUTTON |
                               ui::EF_RIGHT_MOUSE_BUTTON) {
  DCHECK(menu_model);
}

MenuModelAdapter::~MenuModelAdapter() {
}

void MenuModelAdapter::BuildMenu(MenuItemView* menu) {
  DCHECK(menu);

  // Clear the menu.
  if (menu->HasSubmenu()) {
    const int subitem_count = menu->GetSubmenu()->child_count();
    for (int i = 0; i < subitem_count; ++i)
      menu->RemoveMenuItemAt(0);
  }

  // Leave entries in the map if the menu is being shown.  This
  // allows the map to find the menu model of submenus being closed
  // so ui::MenuModel::MenuClosed() can be called.
  if (!menu->GetMenuController())
    menu_map_.clear();
  menu_map_[menu] = menu_model_;

  // Repopulate the menu.
  BuildMenuImpl(menu, menu_model_);
  menu->ChildrenChanged();
}

MenuItemView* MenuModelAdapter::CreateMenu() {
  MenuItemView* item = new MenuItemView(this);
  BuildMenu(item);
  return item;
}

// Static.
MenuItemView* MenuModelAdapter::AddMenuItemFromModelAt(ui::MenuModel* model,
                                                       int model_index,
                                                       MenuItemView* menu,
                                                       int menu_index,
                                                       int item_id) {
  gfx::Image icon;
  model->GetIconAt(model_index, &icon);
  base::string16 label, sublabel, minor_text;
  ui::MenuSeparatorType separator_style = ui::NORMAL_SEPARATOR;
  MenuItemView::Type type;
  ui::MenuModel::ItemType menu_type = model->GetTypeAt(model_index);

  switch (menu_type) {
    case ui::MenuModel::TYPE_COMMAND:
      type = MenuItemView::NORMAL;
      label = model->GetLabelAt(model_index);
      sublabel = model->GetSublabelAt(model_index);
      minor_text = model->GetMinorTextAt(model_index);
      break;
    case ui::MenuModel::TYPE_CHECK:
      type = MenuItemView::CHECKBOX;
      label = model->GetLabelAt(model_index);
      sublabel = model->GetSublabelAt(model_index);
      minor_text = model->GetMinorTextAt(model_index);
      break;
    case ui::MenuModel::TYPE_RADIO:
      type = MenuItemView::RADIO;
      label = model->GetLabelAt(model_index);
      sublabel = model->GetSublabelAt(model_index);
      minor_text = model->GetMinorTextAt(model_index);
      break;
    case ui::MenuModel::TYPE_SEPARATOR:
      icon = gfx::Image();
      type = MenuItemView::SEPARATOR;
      separator_style = model->GetSeparatorTypeAt(model_index);
      break;
    case ui::MenuModel::TYPE_SUBMENU:
      type = MenuItemView::SUBMENU;
      label = model->GetLabelAt(model_index);
      sublabel = model->GetSublabelAt(model_index);
      minor_text = model->GetMinorTextAt(model_index);
      break;
    default:
      NOTREACHED();
      type = MenuItemView::NORMAL;
      break;
  }

  return menu->AddMenuItemAt(
      menu_index,
      item_id,
      label,
      sublabel,
      minor_text,
      icon.IsEmpty() ? gfx::ImageSkia() : *icon.ToImageSkia(),
      type,
      separator_style);
}

// Static.
MenuItemView* MenuModelAdapter::AppendMenuItemFromModel(ui::MenuModel* model,
                                                        int model_index,
                                                        MenuItemView* menu,
                                                        int item_id) {
  const int menu_index = menu->HasSubmenu() ?
      menu->GetSubmenu()->child_count() : 0;
  return AddMenuItemFromModelAt(model, model_index, menu, menu_index, item_id);
}


MenuItemView* MenuModelAdapter::AppendMenuItem(MenuItemView* menu,
                                               ui::MenuModel* model,
                                               int index) {
  return AppendMenuItemFromModel(model, index, menu,
                                 model->GetCommandIdAt(index));
}

// MenuModelAdapter, MenuDelegate implementation:

void MenuModelAdapter::ExecuteCommand(int id) {
  ui::MenuModel* model = menu_model_;
  int index = 0;
  if (ui::MenuModel::GetModelAndIndexForCommandId(id, &model, &index)) {
    model->ActivatedAt(index);
    return;
  }

  NOTREACHED();
}

void MenuModelAdapter::ExecuteCommand(int id, int mouse_event_flags) {
  ui::MenuModel* model = menu_model_;
  int index = 0;
  if (ui::MenuModel::GetModelAndIndexForCommandId(id, &model, &index)) {
    model->ActivatedAt(index, mouse_event_flags);
    return;
  }

  NOTREACHED();
}

bool MenuModelAdapter::IsTriggerableEvent(MenuItemView* source,
                                          const ui::Event& e) {
  return e.type() == ui::ET_GESTURE_TAP ||
         e.type() == ui::ET_GESTURE_TAP_DOWN ||
         (e.IsMouseEvent() && (triggerable_event_flags_ & e.flags()) != 0);
}

bool MenuModelAdapter::GetAccelerator(int id,
                                      ui::Accelerator* accelerator) {
  ui::MenuModel* model = menu_model_;
  int index = 0;
  if (ui::MenuModel::GetModelAndIndexForCommandId(id, &model, &index))
    return model->GetAcceleratorAt(index, accelerator);

  NOTREACHED();
  return false;
}

base::string16 MenuModelAdapter::GetLabel(int id) const {
  ui::MenuModel* model = menu_model_;
  int index = 0;
  if (ui::MenuModel::GetModelAndIndexForCommandId(id, &model, &index))
    return model->GetLabelAt(index);

  NOTREACHED();
  return base::string16();
}

const gfx::FontList* MenuModelAdapter::GetLabelFontList(int id) const {
  ui::MenuModel* model = menu_model_;
  int index = 0;
  if (ui::MenuModel::GetModelAndIndexForCommandId(id, &model, &index)) {
    const gfx::FontList* font_list = model->GetLabelFontListAt(index);
    if (font_list)
      return font_list;
  }

  // This line may be reached for the empty menu item.
  return MenuDelegate::GetLabelFontList(id);
}

bool MenuModelAdapter::IsCommandEnabled(int id) const {
  ui::MenuModel* model = menu_model_;
  int index = 0;
  if (ui::MenuModel::GetModelAndIndexForCommandId(id, &model, &index))
    return model->IsEnabledAt(index);

  NOTREACHED();
  return false;
}

bool MenuModelAdapter::IsItemChecked(int id) const {
  ui::MenuModel* model = menu_model_;
  int index = 0;
  if (ui::MenuModel::GetModelAndIndexForCommandId(id, &model, &index))
    return model->IsItemCheckedAt(index);

  NOTREACHED();
  return false;
}

void MenuModelAdapter::SelectionChanged(MenuItemView* menu) {
  // Ignore selection of the root menu.
  if (menu == menu->GetRootMenuItem())
    return;

  const int id = menu->GetCommand();
  ui::MenuModel* model = menu_model_;
  int index = 0;
  if (ui::MenuModel::GetModelAndIndexForCommandId(id, &model, &index)) {
    model->HighlightChangedTo(index);
    return;
  }

  NOTREACHED();
}

void MenuModelAdapter::WillShowMenu(MenuItemView* menu) {
  // Look up the menu model for this menu.
  const std::map<MenuItemView*, ui::MenuModel*>::const_iterator map_iterator =
      menu_map_.find(menu);
  if (map_iterator != menu_map_.end()) {
    map_iterator->second->MenuWillShow();
    return;
  }

  NOTREACHED();
}

void MenuModelAdapter::WillHideMenu(MenuItemView* menu) {
  // Look up the menu model for this menu.
  const std::map<MenuItemView*, ui::MenuModel*>::const_iterator map_iterator =
      menu_map_.find(menu);
  if (map_iterator != menu_map_.end()) {
    map_iterator->second->MenuClosed();
    return;
  }

  NOTREACHED();
}

// MenuModelAdapter, private:

void MenuModelAdapter::BuildMenuImpl(MenuItemView* menu, ui::MenuModel* model) {
  DCHECK(menu);
  DCHECK(model);
  bool has_icons = model->HasIcons();
  const int item_count = model->GetItemCount();
  for (int i = 0; i < item_count; ++i) {
    MenuItemView* item = AppendMenuItem(menu, model, i);

    if (item)
      item->SetVisible(model->IsVisibleAt(i));

    if (model->GetTypeAt(i) == ui::MenuModel::TYPE_SUBMENU) {
      DCHECK(item);
      DCHECK_EQ(MenuItemView::SUBMENU, item->GetType());
      ui::MenuModel* submodel = model->GetSubmenuModelAt(i);
      DCHECK(submodel);
      BuildMenuImpl(item, submodel);
      has_icons = has_icons || item->has_icons();

      menu_map_[item] = submodel;
    }
  }

  menu->set_has_icons(has_icons);
}

}  // namespace views
