// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/examples/webview_example.h"

#include "base/utf_string_conversions.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_controller.h"
#include "grit/ui_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/controls/webview/webview.h"

namespace views {
namespace examples {

WebViewExample::WebViewExample(content::BrowserContext* browser_context)
    : ExampleBase("WebView"),
      webview_(NULL),
      browser_context_(browser_context) {
}

WebViewExample::~WebViewExample() {
}

void WebViewExample::CreateExampleView(View* container) {
  webview_ = new WebView(browser_context_);
  container->SetLayoutManager(new FillLayout);
  container->AddChildView(webview_);

  webview_->LoadInitialURL(GURL("http://www.google.com/"));
  webview_->web_contents()->Focus();
}

}  // namespace examples
}  // namespace views
