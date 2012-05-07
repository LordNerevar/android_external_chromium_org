// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_APP_LIST_APP_LIST_MODEL_VIEW_H_
#define ASH_APP_LIST_APP_LIST_MODEL_VIEW_H_
#pragma once

#include "ash/ash_export.h"
#include "ash/app_list/pagination_model_observer.h"
#include "ui/base/models/list_model_observer.h"
#include "ui/views/view.h"

namespace views {
class ButtonListener;
}

namespace ash {

class AppListItemView;
class AppListModel;
class PaginationModel;

// AppListModelView displays the UI for an AppListModel.
class ASH_EXPORT AppListModelView : public views::View,
                                    public ui::ListModelObserver,
                                    public PaginationModelObserver {
 public:
  AppListModelView(views::ButtonListener* listener,
                   PaginationModel* pagination_model);
  virtual ~AppListModelView();

  // Sets fixed layout parameters. After setting this, CalculateLayout below
  // is no longer called to dynamically choosing those layout params.
  void SetLayout(int icon_size, int cols, int rows_per_page);

  // Calculate preferred icon size, rows and cols for given |content_size| and
  // |num_of_tiles|.
  static void CalculateLayout(const gfx::Size& content_size,
                              int num_of_tiles,
                              gfx::Size* icon_size,
                              int* rows,
                              int* cols);

  // Sets |model| to use. Note this does not take ownership of |model|.
  void SetModel(AppListModel* model);

  void SetSelectedItem(AppListItemView* item);
  void ClearSelectedItem(AppListItemView* item);

  int tiles_per_page() const {
    return cols_ * rows_per_page_;
  }

 private:
  // Updates from model.
  void Update();

  AppListItemView* GetItemViewAtIndex(int index);
  void SetSelectedItemByIndex(int index);

  // Overridden from views::View:
  virtual gfx::Size GetPreferredSize() OVERRIDE;
  virtual void Layout() OVERRIDE;
  virtual bool OnKeyPressed(const views::KeyEvent& event) OVERRIDE;
  virtual bool OnKeyReleased(const views::KeyEvent& event) OVERRIDE;
  virtual void OnPaintFocusBorder(gfx::Canvas* canvas) OVERRIDE;

  // Overridden from ListModelObserver:
  virtual void ListItemsAdded(int start, int count) OVERRIDE;
  virtual void ListItemsRemoved(int start, int count) OVERRIDE;
  virtual void ListItemsChanged(int start, int count) OVERRIDE;

  // Overridden from PaginationModelObserver:
  virtual void TotalPagesChanged() OVERRIDE;
  virtual void SelectedPageChanged(int old_selected, int new_selected) OVERRIDE;

  AppListModel* model_;  // Owned by parent AppListView.
  views::ButtonListener* listener_;
  PaginationModel* pagination_model_;  // Owned by AppListView.

  bool fixed_layout_;
  gfx::Size icon_size_;
  int cols_;
  int rows_per_page_;

  int selected_item_index_;

  DISALLOW_COPY_AND_ASSIGN(AppListModelView);
};

}  // namespace ash

#endif  // ASH_APP_LIST_APP_LIST_MODEL_VIEW_H_
