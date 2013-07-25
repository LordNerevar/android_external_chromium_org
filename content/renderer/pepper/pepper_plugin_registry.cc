// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/pepper/pepper_plugin_registry.h"

#include "base/logging.h"
#include "content/common/pepper_plugin_list.h"
#include "content/renderer/pepper/plugin_module.h"
#include "ppapi/shared_impl/ppapi_permissions.h"

namespace content {

// static
PepperPluginRegistry* PepperPluginRegistry::GetInstance() {
  static PepperPluginRegistry* registry = NULL;
  // This object leaks.  It is a temporary hack to work around a crash.
  // http://code.google.com/p/chromium/issues/detail?id=63234
  if (!registry)
    registry = new PepperPluginRegistry;
  return registry;
}

const PepperPluginInfo* PepperPluginRegistry::GetInfoForPlugin(
    const WebPluginInfo& info) {
  for (size_t i = 0; i < plugin_list_.size(); ++i) {
    if (info.path == plugin_list_[i].path)
      return &plugin_list_[i];
  }
  // We did not find the plugin in our list. But wait! the plugin can also
  // be a latecomer, as it happens with pepper flash. This information
  // is actually in |info| and we can use it to construct it and add it to
  // the list. This same deal needs to be done in the browser side in
  // PluginService.
  PepperPluginInfo plugin;
  if (!MakePepperPluginInfo(info, &plugin))
    return NULL;

  plugin_list_.push_back(plugin);
  return &plugin_list_[plugin_list_.size() - 1];
}

PluginModule* PepperPluginRegistry::GetLiveModule(const base::FilePath& path) {
  NonOwningModuleMap::iterator it = live_modules_.find(path);
  if (it == live_modules_.end())
    return NULL;
  return it->second;
}

void PepperPluginRegistry::AddLiveModule(const base::FilePath& path,
                                         PluginModule* module) {
  DCHECK(live_modules_.find(path) == live_modules_.end());
  live_modules_[path] = module;
}

void PepperPluginRegistry::PluginModuleDead(PluginModule* dead_module) {
  // DANGER: Don't dereference the dead_module pointer! It may be in the
  // process of being deleted.

  // Modules aren't destroyed very often and there are normally at most a
  // couple of them. So for now we just do a brute-force search.
  for (NonOwningModuleMap::iterator i = live_modules_.begin();
       i != live_modules_.end(); ++i) {
    if (i->second == dead_module) {
      live_modules_.erase(i);
      return;
    }
  }
  // Can occur in tests.
}

PepperPluginRegistry::~PepperPluginRegistry() {
  // Explicitly clear all preloaded modules first. This will cause callbacks
  // to erase these modules from the live_modules_ list, and we don't want
  // that to happen implicitly out-of-order.
  preloaded_modules_.clear();

  DCHECK(live_modules_.empty());
}

PepperPluginRegistry::PepperPluginRegistry() {
  ComputePepperPluginList(&plugin_list_);

  // Note that in each case, AddLiveModule must be called before completing
  // initialization. If we bail out (in the continue clauses) before saving
  // the initialized module, it will still try to unregister itself in its
  // destructor.
  for (size_t i = 0; i < plugin_list_.size(); i++) {
    const PepperPluginInfo& current = plugin_list_[i];
    if (current.is_out_of_process)
      continue;  // Out of process plugins need no special pre-initialization.

    scoped_refptr<PluginModule> module = new PluginModule(
        current.name, current.path,
        ppapi::PpapiPermissions(current.permissions));
    AddLiveModule(current.path, module.get());
    if (current.is_internal) {
      if (!module->InitAsInternalPlugin(current.internal_entry_points)) {
        DLOG(ERROR) << "Failed to load pepper module: " << current.path.value();
        continue;
      }
    } else {
      // Preload all external plugins we're not running out of process.
      if (!module->InitAsLibrary(current.path)) {
        DLOG(ERROR) << "Failed to load pepper module: " << current.path.value();
        continue;
      }
    }
    preloaded_modules_[current.path] = module;
  }
}

}  // namespace content
