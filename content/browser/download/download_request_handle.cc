// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/download/download_request_handle.h"

#include "base/bind.h"
#include "base/stringprintf.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"

using content::BrowserThread;
using content::DownloadManager;
using content::RenderViewHostImpl;

DownloadRequestHandle::~DownloadRequestHandle() {
}

DownloadRequestHandle::DownloadRequestHandle()
    : child_id_(-1),
      render_view_id_(-1),
      request_id_(-1) {
}

DownloadRequestHandle::DownloadRequestHandle(DownloadResourceHandler* handler,
                                             int child_id,
                                             int render_view_id,
                                             int request_id)
    : handler_(handler),
      child_id_(child_id),
      render_view_id_(render_view_id),
      request_id_(request_id) {
  DCHECK(handler_);
}

content::WebContents* DownloadRequestHandle::GetWebContents() const {
  RenderViewHostImpl* render_view_host =
      RenderViewHostImpl::FromID(child_id_, render_view_id_);
  if (!render_view_host)
    return NULL;

  return render_view_host->GetDelegate()->GetAsWebContents();
}

DownloadManager* DownloadRequestHandle::GetDownloadManager() const {
  RenderViewHostImpl* rvh = RenderViewHostImpl::FromID(
      child_id_, render_view_id_);
  if (rvh == NULL)
    return NULL;
  content::RenderProcessHost* rph = rvh->GetProcess();
  if (rph == NULL)
    return NULL;
  content::BrowserContext* context = rph->GetBrowserContext();
  if (context == NULL)
    return NULL;
  return context->GetDownloadManager();
}

void DownloadRequestHandle::PauseRequest() const {
  if (handler_) {
    BrowserThread::PostTask(
        BrowserThread::IO, FROM_HERE,
        base::Bind(&DownloadResourceHandler::PauseRequest, handler_));
  }
}

void DownloadRequestHandle::ResumeRequest() const {
  if (handler_) {
    BrowserThread::PostTask(
        BrowserThread::IO, FROM_HERE,
        base::Bind(&DownloadResourceHandler::ResumeRequest, handler_));
  }
}

void DownloadRequestHandle::CancelRequest() const {
  if (handler_) {
    BrowserThread::PostTask(
        BrowserThread::IO, FROM_HERE,
        base::Bind(&DownloadResourceHandler::CancelRequest, handler_));
  }
}

std::string DownloadRequestHandle::DebugString() const {
  return base::StringPrintf("{"
                            " child_id = %d"
                            " render_view_id = %d"
                            " request_id = %d"
                            "}",
                            child_id_,
                            render_view_id_,
                            request_id_);
}
