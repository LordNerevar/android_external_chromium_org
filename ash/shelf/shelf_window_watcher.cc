// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/shelf/shelf_window_watcher.h"

#include "ash/ash_switches.h"
#include "ash/display/display_controller.h"
#include "ash/shelf/shelf_constants.h"
#include "ash/shelf/shelf_item_delegate_manager.h"
#include "ash/shelf/shelf_model.h"
#include "ash/shelf/shelf_util.h"
#include "ash/shelf/shelf_window_watcher_item_delegate.h"
#include "ash/shell.h"
#include "ash/shell_window_ids.h"
#include "ash/wm/window_state.h"
#include "ash/wm/window_util.h"
#include "base/memory/scoped_ptr.h"
#include "ui/aura/client/activation_client.h"
#include "ui/aura/window.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/screen.h"

namespace {

// Sets LauncherItem property by using the value of |details|.
void SetShelfItemDetailsForLauncherItem(
    ash::LauncherItem* item,
    const ash::ShelfItemDetails& details) {
  item->type = details.type;
  if (details.image_resource_id != ash::kInvalidImageResourceID) {
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();
    item->image = *rb.GetImageSkiaNamed(details.image_resource_id);
  }
}

// Returns true if |window| has a LauncherItem added by ShelfWindowWatcher.
bool HasLauncherItemForWindow(aura::Window* window) {
  if (ash::GetShelfItemDetailsForWindow(window) != NULL &&
      ash::GetLauncherIDForWindow(window) != ash::kInvalidShelfID)
    return true;
  return false;
}

// Returns true if |window| is in the process of being dragged.
bool IsDragging(aura::Window* window) {
  return ash::wm::GetWindowState(window)->is_dragged();
}

}  // namespace

namespace ash {
namespace internal {

ShelfWindowWatcher::RootWindowObserver::RootWindowObserver(
    ShelfWindowWatcher* window_watcher)
    : window_watcher_(window_watcher) {
}

ShelfWindowWatcher::RootWindowObserver::~RootWindowObserver() {
}

void ShelfWindowWatcher::RootWindowObserver::OnWindowDestroying(
    aura::Window* window) {
  window_watcher_->OnRootWindowRemoved(window);
}

ShelfWindowWatcher::RemovedWindowObserver::RemovedWindowObserver(
    ShelfWindowWatcher* window_watcher)
    : window_watcher_(window_watcher) {
}

ShelfWindowWatcher::RemovedWindowObserver::~RemovedWindowObserver() {
}

void ShelfWindowWatcher::RemovedWindowObserver::OnWindowParentChanged(
    aura::Window* window,
    aura::Window* parent) {
  // When |parent| is NULL, this |window| will be destroyed. In that case, its
  // item will be removed at OnWindowDestroyed().
  if (!parent)
    return;

  // When |parent| is changed from default container to docked container
  // during the dragging, |window|'s item should not be removed because it will
  // be re-parented to default container again after finishing the dragging.
  // We don't need to check |parent| is default container because this observer
  // is already removed from |window| when |window| is re-parented to default
  // container.
  if (switches::UseDockedWindows() &&
      IsDragging(window) &&
      parent->id() == kShellWindowId_DockedContainer)
    return;

  // When |window| is re-parented to other containers or |window| is re-parented
  // not to |docked_container| during the dragging, its item should be removed
  // and stop observing this |window|.
  window_watcher_->FinishObservingRemovedWindow(window);
}

void ShelfWindowWatcher::RemovedWindowObserver::OnWindowDestroyed(
    aura::Window* window) {
  DCHECK(HasLauncherItemForWindow(window));
  window_watcher_->FinishObservingRemovedWindow(window);
}

ShelfWindowWatcher::ShelfWindowWatcher(
    ShelfModel* model,
    ShelfItemDelegateManager* item_delegate_manager)
    : model_(model),
      item_delegate_manager_(item_delegate_manager),
      root_window_observer_(this),
      removed_window_observer_(this),
      observed_windows_(this),
      observed_root_windows_(&root_window_observer_),
      observed_removed_windows_(&removed_window_observer_),
      observed_activation_clients_(this) {
  // We can't assume all RootWindows have the same ActivationClient.
  // Add a RootWindow and its ActivationClient to the observed list.
  aura::Window::Windows root_windows = Shell::GetAllRootWindows();
  for (aura::Window::Windows::const_iterator it = root_windows.begin();
       it != root_windows.end(); ++it)
    OnRootWindowAdded(*it);

  Shell::GetScreen()->AddObserver(this);
}

ShelfWindowWatcher::~ShelfWindowWatcher() {
  Shell::GetScreen()->RemoveObserver(this);
}

void ShelfWindowWatcher::AddLauncherItem(aura::Window* window) {
  const ShelfItemDetails* item_details =
      GetShelfItemDetailsForWindow(window);
  LauncherItem item;
  LauncherID id = model_->next_id();
  item.status = wm::IsActiveWindow(window) ? STATUS_ACTIVE: STATUS_RUNNING;
  SetShelfItemDetailsForLauncherItem(&item, *item_details);
  SetLauncherIDForWindow(id, window);
  scoped_ptr<ShelfItemDelegate> item_delegate(
      new ShelfWindowWatcherItemDelegate(window, model_));
  // |item_delegate| is owned by |item_delegate_manager_|.
  item_delegate_manager_->SetShelfItemDelegate(id, item_delegate.Pass());
  model_->Add(item);
}

void ShelfWindowWatcher::RemoveLauncherItem(aura::Window* window) {
  model_->RemoveItemAt(model_->ItemIndexByID(GetLauncherIDForWindow(window)));
  SetLauncherIDForWindow(kInvalidShelfID, window);
}

void ShelfWindowWatcher::OnRootWindowAdded(aura::Window* root_window) {
  // |observed_activation_clients_| can have the same ActivationClient multiple
  // times - which would be handled by the |observed_activation_clients_|.
  observed_activation_clients_.Add(
      aura::client::GetActivationClient(root_window));
  observed_root_windows_.Add(root_window);

  aura::Window* default_container = Shell::GetContainer(
      root_window,
      kShellWindowId_DefaultContainer);
  observed_windows_.Add(default_container);
  for (size_t i = 0; i < default_container->children().size(); ++i)
    observed_windows_.Add(default_container->children()[i]);
}

void ShelfWindowWatcher::OnRootWindowRemoved(aura::Window* root_window) {
  observed_root_windows_.Remove(root_window);
  observed_activation_clients_.Remove(
      aura::client::GetActivationClient(root_window));
}

void ShelfWindowWatcher::UpdateLauncherItemStatus(aura::Window* window,
                                                  bool is_active) {
  int index = GetLauncherItemIndexForWindow(window);
  DCHECK_GE(index, 0);

  LauncherItem item = model_->items()[index];
  item.status = is_active ? STATUS_ACTIVE : STATUS_RUNNING;
  model_->Set(index, item);
}

int ShelfWindowWatcher::GetLauncherItemIndexForWindow(
    aura::Window* window) const {
  return model_->ItemIndexByID(GetLauncherIDForWindow(window));
}

void ShelfWindowWatcher::StartObservingRemovedWindow(aura::Window* window) {
  observed_removed_windows_.Add(window);
}

void ShelfWindowWatcher::FinishObservingRemovedWindow(aura::Window* window) {
  observed_removed_windows_.Remove(window);
  RemoveLauncherItem(window);
}

void ShelfWindowWatcher::OnWindowActivated(aura::Window* gained_active,
                                           aura::Window* lost_active) {
  if (gained_active && HasLauncherItemForWindow(gained_active))
    UpdateLauncherItemStatus(gained_active, true);
  if (lost_active && HasLauncherItemForWindow(lost_active))
    UpdateLauncherItemStatus(lost_active, false);
}

void ShelfWindowWatcher::OnWindowAdded(aura::Window* window) {
  observed_windows_.Add(window);

  if (observed_removed_windows_.IsObserving(window)) {
    // When |window| is added and it is already observed by
    // |dragged_window_observer_|, |window| already has its item.
    DCHECK(HasLauncherItemForWindow(window));
    observed_removed_windows_.Remove(window);
    return;
  }

  // Add LauncherItem if |window| already has a ShelfItemDetails when it is
  // created. Don't make a new LauncherItem for the re-parented |window| that
  // already has a LauncherItem.
  if (GetLauncherIDForWindow(window) == kInvalidShelfID &&
      GetShelfItemDetailsForWindow(window))
    AddLauncherItem(window);
}

void ShelfWindowWatcher::OnWillRemoveWindow(aura::Window* window) {
  // Remove a child window of default container.
  if (observed_windows_.IsObserving(window))
    observed_windows_.Remove(window);

  // Don't remove |window| item immediately. Instead, defer handling of removing
  // |window|'s item to RemovedWindowObserver because |window| could be added
  // again to default container.
  if (HasLauncherItemForWindow(window))
    StartObservingRemovedWindow(window);
}

void ShelfWindowWatcher::OnWindowDestroying(aura::Window* window) {
  // Remove the default container.
  if (observed_windows_.IsObserving(window))
    observed_windows_.Remove(window);
}

void ShelfWindowWatcher::OnWindowPropertyChanged(aura::Window* window,
                                                 const void* key,
                                                 intptr_t old) {
  if (key != kShelfItemDetailsKey)
    return;

  if (GetShelfItemDetailsForWindow(window) == NULL) {
    // Removes LauncherItem for |window| when it has a LauncherItem.
    if (reinterpret_cast<ShelfItemDetails*>(old) != NULL)
      RemoveLauncherItem(window);
    return;
  }

  // When ShelfItemDetails is changed, update LauncherItem.
  if (HasLauncherItemForWindow(window)) {
    int index = GetLauncherItemIndexForWindow(window);
    DCHECK_GE(index, 0);
    LauncherItem item = model_->items()[index];
    const ShelfItemDetails* details =
        GetShelfItemDetailsForWindow(window);
    SetShelfItemDetailsForLauncherItem(&item, *details);
    model_->Set(index, item);
    return;
  }

  // Creates a new LauncherItem for |window|.
  AddLauncherItem(window);
}

void ShelfWindowWatcher::OnDisplayBoundsChanged(const gfx::Display& display) {
}

void ShelfWindowWatcher::OnDisplayAdded(const gfx::Display& new_display) {
  // Add a new RootWindow and its ActivationClient to observed list.
  aura::Window* root_window = Shell::GetInstance()->display_controller()->
      GetRootWindowForDisplayId(new_display.id());

  // When the primary root window's display get removed, the existing root
  // window is taken over by the new display and the observer is already set.
  if (!observed_root_windows_.IsObserving(root_window))
    OnRootWindowAdded(root_window);
}

void ShelfWindowWatcher::OnDisplayRemoved(const gfx::Display& old_display) {
  // When this is called, RootWindow of |old_display| is already removed.
  // Instead, we remove an observer from RootWindow and ActivationClient in the
  // OnRootWindowDestroyed().
  // Do nothing here.
}

}  // namespace internal
}  // namespace ash
