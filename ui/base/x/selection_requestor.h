// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_X_SELECTION_REQUESTOR_H_
#define UI_BASE_X_SELECTION_REQUESTOR_H_

#include <X11/Xlib.h>

// Get rid of a macro from Xlib.h that conflicts with Aura's RootWindow class.
#undef RootWindow

#include <list>
#include <vector>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/event_types.h"
#include "base/memory/ref_counted_memory.h"
#include "ui/base/ui_base_export.h"
#include "ui/gfx/x/x11_atom_cache.h"
#include "ui/gfx/x/x11_types.h"

namespace ui {
class PlatformEventDispatcher;
class SelectionData;

// Requests and later receives data from the X11 server through the selection
// system.
//
// X11 uses a system called "selections" to implement clipboards and drag and
// drop. This class interprets messages from the statefull selection request
// API. SelectionRequestor should only deal with the X11 details; it does not
// implement per-component fast-paths.
class UI_BASE_EXPORT SelectionRequestor {
 public:
  SelectionRequestor(XDisplay* xdisplay,
                     XID xwindow,
                     XAtom selection_name,
                     PlatformEventDispatcher* dispatcher);
  ~SelectionRequestor();

  // Does the work of requesting |target| from the selection we handle,
  // spinning up the nested message loop, and reading the resulting data
  // back. The result is stored in |out_data|.
  // |out_data_items| is the length of |out_data| in |out_type| items.
  bool PerformBlockingConvertSelection(
      XAtom target,
      scoped_refptr<base::RefCountedMemory>* out_data,
      size_t* out_data_items,
      XAtom* out_type);

  // Requests |target| from the selection that we handle, passing |parameter|
  // as a parameter to XConvertSelection().
  void PerformBlockingConvertSelectionWithParameter(
      XAtom target,
      const std::vector<XAtom>& parameter);

  // Returns the first of |types| offered by the current selection holder, or
  // returns NULL if none of those types are available.
  SelectionData RequestAndWaitForTypes(const std::vector<XAtom>& types);

  // It is our owner's responsibility to plumb X11 SelectionNotify events on
  // |xwindow_| to us.
  void OnSelectionNotify(const XEvent& event);

 private:
  // A request that has been issued and we are waiting for a response to.
  struct PendingRequest {
    explicit PendingRequest(XAtom target);
    ~PendingRequest();

    // Data to the current XConvertSelection request. Used for error detection;
    // we verify it on the return message.
    XAtom target;

    // Called to terminate the nested message loop.
    base::Closure quit_closure;

    // The property in the returning SelectNotify message is used to signal
    // success. If None, our request failed somehow. If equal to the property
    // atom that we sent in the XConvertSelection call, we can read that
    // property on |x_window_| for the requested data.
    XAtom returned_property;

    // Set to true when return_property is populated.
    bool returned;
  };

  // Blocks till SelectionNotify is received for the target specified in
  // |request|.
  void BlockTillSelectionNotifyForRequest(PendingRequest* request);

  // Our X11 state.
  XDisplay* x_display_;
  XID x_window_;

  // The X11 selection that this instance communicates on.
  XAtom selection_name_;

  // Dispatcher which handles SelectionNotify and SelectionRequest for
  // |selection_name_|. PerformBlockingConvertSelection() calls the
  // dispatcher directly if PerformBlockingConvertSelection() is called after
  // the PlatformEventSource is destroyed.
  // Not owned.
  PlatformEventDispatcher* dispatcher_;

  // A list of requests for which we are waiting for responses.
  std::list<PendingRequest*> pending_requests_;

  X11AtomCache atom_cache_;

  DISALLOW_COPY_AND_ASSIGN(SelectionRequestor);
};

}  // namespace ui

#endif  // UI_BASE_X_SELECTION_REQUESTOR_H_
