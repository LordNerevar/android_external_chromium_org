// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/url_request/url_request_redirect_job.h"

#include <string>

#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/stringprintf.h"
#include "net/base/load_timing_info.h"
#include "net/base/net_errors.h"
#include "net/base/net_log.h"
#include "net/http/http_response_headers.h"
#include "net/http/http_util.h"
#include "net/url_request/url_request.h"

namespace net {

URLRequestRedirectJob::URLRequestRedirectJob(URLRequest* request,
                                             NetworkDelegate* network_delegate,
                                             const GURL& redirect_destination,
                                             ResponseCode response_code,
                                             const std::string& redirect_reason)
    : URLRequestJob(request, network_delegate),
      redirect_destination_(redirect_destination),
      response_code_(response_code),
      redirect_reason_(redirect_reason),
      weak_factory_(this) {
  DCHECK(!redirect_reason_.empty());
}

void URLRequestRedirectJob::GetResponseInfo(HttpResponseInfo* info) {
  // Should only be called after the URLRequest has been notified there's header
  // information.
  DCHECK(fake_headers_);

  // This assumes |info| is a freshly constructed HttpResponseInfo.
  info->headers = fake_headers_;
  info->request_time = response_time_;
  info->response_time = response_time_;
}

void URLRequestRedirectJob::GetLoadTimingInfo(
    LoadTimingInfo* load_timing_info) const {
  // Set send_start and send_end to receive_headers_end_ to be consistent
  // with network cache behavior.
  load_timing_info->send_start = receive_headers_end_;
  load_timing_info->send_end = receive_headers_end_;
  load_timing_info->receive_headers_end = receive_headers_end_;
}

void URLRequestRedirectJob::Start() {
  request()->net_log().AddEvent(
      NetLog::TYPE_URL_REQUEST_REDIRECT_JOB,
      NetLog::StringCallback("reason", &redirect_reason_));
  base::MessageLoop::current()->PostTask(
      FROM_HERE,
      base::Bind(&URLRequestRedirectJob::StartAsync,
                 weak_factory_.GetWeakPtr()));
}

bool URLRequestRedirectJob::CopyFragmentOnRedirect(const GURL& location) const {
  // The instantiators have full control over the desired redirection target,
  // including the reference fragment part of the URL.
  return false;
}

int URLRequestRedirectJob::GetResponseCode() const {
  // Should only be called after the URLRequest has been notified there's header
  // information.
  DCHECK(fake_headers_);
  return response_code_;
}

URLRequestRedirectJob::~URLRequestRedirectJob() {}

void URLRequestRedirectJob::StartAsync() {
  receive_headers_end_ = base::TimeTicks::Now();
  response_time_ = base::Time::Now();

  std::string header_string =
      base::StringPrintf("HTTP/1.1 %i Internal Redirect\n"
                             "Location: %s\n"
                             "Non-Authoritative-Reason: %s",
                         response_code_,
                         redirect_destination_.spec().c_str(),
                         redirect_reason_.c_str());
  fake_headers_ = new HttpResponseHeaders(
      HttpUtil::AssembleRawHeaders(header_string.c_str(),
                                   header_string.length()));
  DCHECK(fake_headers_->IsRedirect(NULL));

  request()->net_log().AddEvent(
      NetLog::TYPE_URL_REQUEST_FAKE_RESPONSE_HEADERS_CREATED,
      base::Bind(
          &HttpResponseHeaders::NetLogCallback,
          base::Unretained(fake_headers_.get())));

  // TODO(mmenke):  Consider calling the NetworkDelegate with the headers here.
  // There's some weirdness about how to handle the case in which the delegate
  // tries to modify the redirect location, in terms of how IsSafeRedirect
  // should behave, and whether the fragment should be copied.
  URLRequestJob::NotifyHeadersComplete();
}

}  // namespace net
