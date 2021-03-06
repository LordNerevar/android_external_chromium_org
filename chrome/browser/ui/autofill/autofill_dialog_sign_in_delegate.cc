// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/autofill/autofill_dialog_sign_in_delegate.h"

#include "chrome/browser/ui/autofill/autofill_dialog_view.h"
#include "components/autofill/content/browser/wallet/wallet_service_url.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/renderer_preferences.h"
#include "ui/base/window_open_disposition.h"

namespace autofill {

AutofillDialogSignInDelegate::AutofillDialogSignInDelegate(
    AutofillDialogView* dialog_view,
    content::WebContents* web_contents,
    content::WebContentsDelegate* wrapped_delegate,
    const gfx::Size& minimum_size,
    const gfx::Size& maximum_size)
    : WebContentsObserver(web_contents),
      dialog_view_(dialog_view),
      wrapped_delegate_(wrapped_delegate) {
  DCHECK(dialog_view_);
  DCHECK(wrapped_delegate_);
  web_contents->SetDelegate(this);

  content::RendererPreferences* prefs = web_contents->GetMutableRendererPrefs();
  prefs->browser_handles_all_top_level_link_clicks = true;
  web_contents->GetRenderViewHost()->SyncRendererPrefs();

  UpdateLimitsAndEnableAutoResize(minimum_size, maximum_size);
}

void AutofillDialogSignInDelegate::ResizeDueToAutoResize(
    content::WebContents* source,
    const gfx::Size& preferred_size) {
  dialog_view_->OnSignInResize(preferred_size);
}

content::WebContents* AutofillDialogSignInDelegate::OpenURLFromTab(
    content::WebContents* source,
    const content::OpenURLParams& params) {
  content::OpenURLParams new_params = params;
  if (params.transition == content::PAGE_TRANSITION_LINK &&
      params.disposition == CURRENT_TAB &&
      !wallet::IsSignInRelatedUrl(params.url)) {
    new_params.disposition = NEW_FOREGROUND_TAB;
  }
  return wrapped_delegate_->OpenURLFromTab(source, new_params);
}

void AutofillDialogSignInDelegate::AddNewContents(
    content::WebContents* source,
    content::WebContents* new_contents,
    WindowOpenDisposition disposition,
    const gfx::Rect& initial_pos,
    bool user_gesture,
    bool* was_blocked) {
  wrapped_delegate_->AddNewContents(source, new_contents, disposition,
                                    initial_pos, user_gesture, was_blocked);
}

void AutofillDialogSignInDelegate::RenderViewCreated(
    content::RenderViewHost* render_view_host) {
  EnableAutoResize();

  // Set the initial size as soon as we have an RVH to avoid bad size jumping.
  dialog_view_->OnSignInResize(minimum_size_);
}

void AutofillDialogSignInDelegate::UpdateLimitsAndEnableAutoResize(
    const gfx::Size& minimum_size,
    const gfx::Size& maximum_size) {
  minimum_size_ = minimum_size;
  maximum_size_ = maximum_size;
  EnableAutoResize();
}

void AutofillDialogSignInDelegate::EnableAutoResize() {
  DCHECK(!minimum_size_.IsEmpty());
  DCHECK(!maximum_size_.IsEmpty());
  content::RenderViewHost* host = web_contents()->GetRenderViewHost();
  if (host)
    host->EnableAutoResize(minimum_size_, maximum_size_);
}

}  // namespace autofill
