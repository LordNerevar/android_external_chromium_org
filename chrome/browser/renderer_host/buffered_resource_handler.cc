// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/renderer_host/buffered_resource_handler.h"

#include <vector>

#include "base/histogram.h"
#include "base/logging.h"
#include "base/string_util.h"
#include "chrome/browser/chrome_thread.h"
#include "chrome/browser/renderer_host/download_throttling_resource_handler.h"
#include "chrome/browser/renderer_host/resource_dispatcher_host.h"
#include "chrome/browser/renderer_host/resource_dispatcher_host_request_info.h"
#include "chrome/browser/renderer_host/x509_user_cert_resource_handler.h"
#include "chrome/common/extensions/user_script.h"
#include "chrome/common/resource_response.h"
#include "chrome/common/url_constants.h"
#include "net/base/io_buffer.h"
#include "net/base/mime_sniffer.h"
#include "net/base/mime_util.h"
#include "net/base/net_errors.h"
#include "net/http/http_response_headers.h"
#include "webkit/glue/plugins/plugin_list.h"

namespace {

void RecordSnifferMetrics(bool sniffing_blocked,
                          bool we_would_like_to_sniff,
                          const std::string& mime_type) {
  static scoped_refptr<Histogram> nosniff_usage = BooleanHistogram::FactoryGet(
      "nosniff.usage", Histogram::kUmaTargetedHistogramFlag);
  nosniff_usage->AddBoolean(sniffing_blocked);

  if (sniffing_blocked) {
    static scoped_refptr<Histogram> nosniff_otherwise =
        BooleanHistogram::FactoryGet("nosniff.otherwise",
                                     Histogram::kUmaTargetedHistogramFlag);
    nosniff_otherwise->AddBoolean(we_would_like_to_sniff);

    static scoped_refptr<Histogram> nosniff_empty_mime_type =
        BooleanHistogram::FactoryGet("nosniff.empty_mime_type",
                                     Histogram::kUmaTargetedHistogramFlag);
    nosniff_empty_mime_type->AddBoolean(mime_type.empty());
  }
}

}  // namespace

BufferedResourceHandler::BufferedResourceHandler(ResourceHandler* handler,
                                                 ResourceDispatcherHost* host,
                                                 URLRequest* request)
    : real_handler_(handler),
      host_(host),
      request_(request),
      read_buffer_size_(0),
      bytes_read_(0),
      sniff_content_(false),
      should_buffer_(false),
      wait_for_plugins_(false),
      buffering_(false),
      finished_(false) {
}

bool BufferedResourceHandler::OnUploadProgress(int request_id,
                                               uint64 position,
                                               uint64 size) {
  return real_handler_->OnUploadProgress(request_id, position, size);
}

bool BufferedResourceHandler::OnRequestRedirected(int request_id,
                                                  const GURL& new_url,
                                                  ResourceResponse* response,
                                                  bool* defer) {
  return real_handler_->OnRequestRedirected(
      request_id, new_url, response, defer);
}

bool BufferedResourceHandler::OnResponseStarted(int request_id,
                                                ResourceResponse* response) {
  response_ = response;
  if (!DelayResponse())
    return CompleteResponseStarted(request_id, false);
  return true;
}

bool BufferedResourceHandler::OnResponseCompleted(
    int request_id,
    const URLRequestStatus& status,
    const std::string& security_info) {
  return real_handler_->OnResponseCompleted(request_id, status, security_info);
}

void BufferedResourceHandler::OnRequestClosed() {
  request_ = NULL;
  real_handler_->OnRequestClosed();
}

bool BufferedResourceHandler::OnWillStart(int request_id,
                                          const GURL& url,
                                          bool* defer) {
  return real_handler_->OnWillStart(request_id, url, defer);
}

// We'll let the original event handler provide a buffer, and reuse it for
// subsequent reads until we're done buffering.
bool BufferedResourceHandler::OnWillRead(int request_id, net::IOBuffer** buf,
                                         int* buf_size, int min_size) {
  if (buffering_) {
    DCHECK(!my_buffer_.get());
    my_buffer_ = new net::IOBuffer(net::kMaxBytesToSniff);
    *buf = my_buffer_.get();
    *buf_size = net::kMaxBytesToSniff;
    return true;
  }

  if (finished_)
    return false;

  if (!real_handler_->OnWillRead(request_id, buf, buf_size, min_size)) {
    return false;
  }
  read_buffer_ = *buf;
  read_buffer_size_ = *buf_size;
  DCHECK_GE(read_buffer_size_, net::kMaxBytesToSniff * 2);
  bytes_read_ = 0;
  return true;
}

bool BufferedResourceHandler::OnReadCompleted(int request_id, int* bytes_read) {
  if (sniff_content_ || should_buffer_) {
    if (KeepBuffering(*bytes_read))
      return true;

    LOG(INFO) << "Finished buffering " << request_->url().spec();
    *bytes_read = bytes_read_;

    // Done buffering, send the pending ResponseStarted event.
    if (!CompleteResponseStarted(request_id, true))
      return false;
  } else if (wait_for_plugins_) {
    return true;
  }

  // Release the reference that we acquired at OnWillRead.
  read_buffer_ = NULL;
  return real_handler_->OnReadCompleted(request_id, bytes_read);
}

bool BufferedResourceHandler::DelayResponse() {
  std::string mime_type;
  request_->GetMimeType(&mime_type);

  std::string content_type_options;
  request_->GetResponseHeaderByName("x-content-type-options",
                                    &content_type_options);

  const bool sniffing_blocked =
      LowerCaseEqualsASCII(content_type_options, "nosniff");
  const bool not_modified_status =
      response_->response_head.headers &&
      response_->response_head.headers->response_code() == 304;
  const bool we_would_like_to_sniff = not_modified_status ?
      false : net::ShouldSniffMimeType(request_->url(), mime_type);

  RecordSnifferMetrics(sniffing_blocked, we_would_like_to_sniff, mime_type);

  if (!sniffing_blocked && we_would_like_to_sniff) {
    // We're going to look at the data before deciding what the content type
    // is.  That means we need to delay sending the ResponseStarted message
    // over the IPC channel.
    sniff_content_ = true;
    LOG(INFO) << "To buffer: " << request_->url().spec();
    return true;
  }

  if (sniffing_blocked && mime_type.empty() && !not_modified_status) {
    // Ugg.  The server told us not to sniff the content but didn't give us a
    // mime type.  What's a browser to do?  Turns out, we're supposed to treat
    // the response as "text/plain".  This is the most secure option.
    mime_type.assign("text/plain");
    response_->response_head.mime_type.assign(mime_type);
  }

  if (mime_type == "application/rss+xml" ||
      mime_type == "application/atom+xml") {
    // Sad face.  The server told us that they wanted us to treat the response
    // as RSS or Atom.  Unfortunately, we don't have a built-in feed previewer
    // like other browsers.  We can't just render the content as XML because
    // web sites let third parties inject arbitrary script into their RSS
    // feeds.  That leaves us with little choice but to practically ignore the
    // response.  In the future, when we have an RSS feed previewer, we can
    // remove this logic.
    mime_type.assign("text/plain");
    response_->response_head.mime_type.assign(mime_type);
  }

  if (ShouldBuffer(request_->url(), mime_type)) {
    // This is a temporary fix for the fact that webkit expects to have
    // enough data to decode the doctype in order to select the rendering
    // mode.
    should_buffer_ = true;
    LOG(INFO) << "To buffer: " << request_->url().spec();
    return true;
  }

  if (!not_modified_status && ShouldWaitForPlugins()) {
    wait_for_plugins_ = true;
    return true;
  }

  return false;
}

bool BufferedResourceHandler::ShouldBuffer(const GURL& url,
                                           const std::string& mime_type) {
  // We are willing to buffer for HTTP and HTTPS.
  bool sniffable_scheme = url.is_empty() ||
                          url.SchemeIs(chrome::kHttpScheme) ||
                          url.SchemeIs(chrome::kHttpsScheme);
  if (!sniffable_scheme)
    return false;

  // Today, the only reason to buffer the request is to fix the doctype decoding
  // performed by webkit: if there is not enough data it will go to quirks mode.
  // We only expect the doctype check to apply to html documents.
  return mime_type == "text/html";
}

bool BufferedResourceHandler::DidBufferEnough(int bytes_read) {
  const int kRequiredLength = 256;

  return bytes_read >= kRequiredLength;
}

bool BufferedResourceHandler::KeepBuffering(int bytes_read) {
  DCHECK(read_buffer_);
  if (my_buffer_) {
    // We are using our own buffer to read, update the main buffer.
    // TODO(darin): We should handle the case where read_buffer_size_ is small!
    // See RedirectToFileResourceHandler::BufIsFull to see how this impairs
    // downstream ResourceHandler implementations.
    CHECK_LT(bytes_read + bytes_read_, read_buffer_size_);
    memcpy(read_buffer_->data() + bytes_read_, my_buffer_->data(), bytes_read);
    my_buffer_ = NULL;
  }
  bytes_read_ += bytes_read;
  finished_ = (bytes_read == 0);

  if (sniff_content_) {
    std::string type_hint, new_type;
    request_->GetMimeType(&type_hint);

    if (!net::SniffMimeType(read_buffer_->data(), bytes_read_,
                            request_->url(), type_hint, &new_type)) {
      // SniffMimeType() returns false if there is not enough data to determine
      // the mime type. However, even if it returns false, it returns a new type
      // that is probably better than the current one.
      DCHECK_LT(bytes_read_, net::kMaxBytesToSniff);
      if (!finished_) {
        buffering_ = true;
        return true;
      }
    }
    sniff_content_ = false;
    response_->response_head.mime_type.assign(new_type);

    // We just sniffed the mime type, maybe there is a doctype to process.
    if (ShouldBuffer(request_->url(), new_type)) {
      should_buffer_ = true;
    } else if (ShouldWaitForPlugins()) {
      wait_for_plugins_ = true;
    }
  }

  if (should_buffer_) {
    if (!finished_ && !DidBufferEnough(bytes_read_)) {
      buffering_ = true;
      return true;
    }

    should_buffer_ = false;
    if (ShouldWaitForPlugins())
      wait_for_plugins_ = true;
  }

  buffering_ = false;

  if (wait_for_plugins_)
    return true;

  return false;
}

bool BufferedResourceHandler::CompleteResponseStarted(int request_id,
                                                      bool in_complete) {
  ResourceDispatcherHostRequestInfo* info =
      ResourceDispatcherHost::InfoForRequest(request_);
  std::string mime_type;
  request_->GetMimeType(&mime_type);

  // Check if this is an X.509 certificate, if yes, let it be handled
  // by X509UserCertResourceHandler.
  if (mime_type == "application/x-x509-user-cert") {
    // This is entirely similar to how DownloadThrottlingResourceHandler
    // works except we are doing it for an X.509 client certificates.

    if (response_->response_head.headers &&  // Can be NULL if FTP.
        response_->response_head.headers->response_code() / 100 != 2) {
      // The response code indicates that this is an error page, but we are
      // expecting an X.509 user certificate. We follow Firefox here and show
      // our own error page instead of handling the error page as a
      // certificate.
      // TODO(abarth): We should abstract the response_code test, but this kind
      //               of check is scattered throughout our codebase.
      request_->SimulateError(net::ERR_FILE_NOT_FOUND);
      return false;
    }

    X509UserCertResourceHandler* x509_cert_handler =
        new X509UserCertResourceHandler(host_, request_,
                                        info->child_id(), info->route_id());
    UseAlternateResourceHandler(request_id, x509_cert_handler);
  }

  // Check to see if we should forward the data from this request to the
  // download thread.
  // TODO(paulg): Only download if the context from the renderer allows it.
  if (info->allow_download() && ShouldDownload(NULL)) {
    if (response_->response_head.headers &&  // Can be NULL if FTP.
        response_->response_head.headers->response_code() / 100 != 2) {
      // The response code indicates that this is an error page, but we don't
      // know how to display the content.  We follow Firefox here and show our
      // own error page instead of triggering a download.
      // TODO(abarth): We should abstract the response_code test, but this kind
      //               of check is scattered throughout our codebase.
      request_->SimulateError(net::ERR_FILE_NOT_FOUND);
      return false;
    }

    info->set_is_download(true);

    DownloadThrottlingResourceHandler* download_handler =
        new DownloadThrottlingResourceHandler(host_,
                                              request_,
                                              request_->url(),
                                              info->child_id(),
                                              info->route_id(),
                                              request_id,
                                              in_complete);
    UseAlternateResourceHandler(request_id, download_handler);
  }
  return real_handler_->OnResponseStarted(request_id, response_);
}

bool BufferedResourceHandler::ShouldWaitForPlugins() {
  bool need_plugin_list;
  if (!ShouldDownload(&need_plugin_list) || !need_plugin_list)
    return false;

  // We don't want to keep buffering as our buffer will fill up.
  ResourceDispatcherHostRequestInfo* info =
      ResourceDispatcherHost::InfoForRequest(request_);
  host_->PauseRequest(info->child_id(), info->request_id(), true);

  // Schedule plugin loading on the file thread.
  ChromeThread::PostTask(
      ChromeThread::FILE, FROM_HERE,
      NewRunnableMethod(this, &BufferedResourceHandler::LoadPlugins));
  return true;
}

// This test mirrors the decision that WebKit makes in
// WebFrameLoaderClient::dispatchDecidePolicyForMIMEType.
bool BufferedResourceHandler::ShouldDownload(bool* need_plugin_list) {
  if (need_plugin_list)
    *need_plugin_list = false;
  std::string type = StringToLowerASCII(response_->response_head.mime_type);
  std::string disposition;
  request_->GetResponseHeaderByName("content-disposition", &disposition);
  disposition = StringToLowerASCII(disposition);

  // First, examine content-disposition.
  if (!disposition.empty()) {
    bool should_download = true;

    // Some broken sites just send ...
    //    Content-Disposition: ; filename="file"
    // ... screen those out here.
    if (disposition[0] == ';')
      should_download = false;

    if (disposition.compare(0, 6, "inline") == 0)
      should_download = false;

    // Some broken sites just send ...
    //    Content-Disposition: filename="file"
    // ... without a disposition token... Screen those out.
    if (disposition.compare(0, 8, "filename") == 0)
      should_download = false;

    // Also in use is Content-Disposition: name="file"
    if (disposition.compare(0, 4, "name") == 0)
      should_download = false;

    // We have a content-disposition of "attachment" or unknown.
    // RFC 2183, section 2.8 says that an unknown disposition
    // value should be treated as "attachment".
    if (should_download)
      return true;
  }

  // Special-case user scripts to get downloaded instead of viewed.
  if (UserScript::HasUserScriptFileExtension(request_->url()))
    return true;

  // MIME type checking.
  if (net::IsSupportedMimeType(type))
    return false;

  if (need_plugin_list) {
    if (!NPAPI::PluginList::Singleton()->PluginsLoaded()) {
      *need_plugin_list = true;
      return true;
    }
  } else {
    DCHECK(NPAPI::PluginList::Singleton()->PluginsLoaded());
  }

  // Finally, check the plugin list.
  WebPluginInfo info;
  bool allow_wildcard = false;
  return !NPAPI::PluginList::Singleton()->GetPluginInfo(
      GURL(), type, allow_wildcard, &info, NULL) || !info.enabled;
}

void BufferedResourceHandler::UseAlternateResourceHandler(
    int request_id,
    ResourceHandler* handler) {
  ResourceDispatcherHostRequestInfo* info =
      ResourceDispatcherHost::InfoForRequest(request_);
  if (bytes_read_) {
    // A Read has already occured and we need to copy the data into the new
    // ResourceHandler.
    net::IOBuffer* buf = NULL;
    int buf_len = 0;
    handler->OnWillRead(request_id, &buf, &buf_len, bytes_read_);
    CHECK((buf_len >= bytes_read_) && (bytes_read_ >= 0));
    memcpy(buf->data(), read_buffer_->data(), bytes_read_);
  }

  // Inform the original ResourceHandler that this will be handled entirely by
  // the new ResourceHandler.
  real_handler_->OnResponseStarted(info->request_id(), response_);
  URLRequestStatus status(URLRequestStatus::HANDLED_EXTERNALLY, 0);
  real_handler_->OnResponseCompleted(info->request_id(), status, std::string());

  // Remove the non-owning pointer to the CrossSiteResourceHandler, if any,
  // from the extra request info because the CrossSiteResourceHandler (part of
  // the original ResourceHandler chain) will be deleted by the next statement.
  info->set_cross_site_handler(NULL);

  // This is handled entirely within the new ResourceHandler, so just reset the
  // original ResourceHandler.
  real_handler_ = handler;
}

void BufferedResourceHandler::LoadPlugins() {
  std::vector<WebPluginInfo> plugins;
  NPAPI::PluginList::Singleton()->GetPlugins(false, &plugins);

  ChromeThread::PostTask(
      ChromeThread::IO, FROM_HERE,
      NewRunnableMethod(this, &BufferedResourceHandler::OnPluginsLoaded));
}

void BufferedResourceHandler::OnPluginsLoaded() {
  wait_for_plugins_ = false;
  if (!request_)
    return;

  ResourceDispatcherHostRequestInfo* info =
      ResourceDispatcherHost::InfoForRequest(request_);
  host_->PauseRequest(info->child_id(), info->request_id(), false);
  if (!CompleteResponseStarted(info->request_id(), false))
    host_->CancelRequest(info->child_id(), info->request_id(), false);
}
