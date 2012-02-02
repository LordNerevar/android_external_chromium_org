// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/browser_list.h"

#include "ui/views/widget/widget.h"

// static
void BrowserList::AllBrowsersClosedAndAppExiting() {
#if !defined(USE_AURA)
  views::Widget::CloseAllSecondaryWidgets();
#endif
}
