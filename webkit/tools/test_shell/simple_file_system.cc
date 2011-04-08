// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webkit/tools/test_shell/simple_file_system.h"

#include "base/file_path.h"
#include "base/memory/scoped_callback_factory.h"
#include "base/message_loop.h"
#include "base/message_loop_proxy.h"
#include "base/time.h"
#include "base/utf_string_conversions.h"
#include "googleurl/src/gurl.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFileInfo.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFileSystemCallbacks.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFileSystemEntry.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebSecurityOrigin.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebVector.h"
#include "webkit/fileapi/file_system_callback_dispatcher.h"
#include "webkit/fileapi/file_system_context.h"
#include "webkit/fileapi/file_system_file_util.h"
#include "webkit/fileapi/file_system_operation.h"
#include "webkit/fileapi/file_system_path_manager.h"
#include "webkit/fileapi/file_system_types.h"
#include "webkit/glue/webkit_glue.h"
#include "webkit/tools/test_shell/simple_file_writer.h"

using base::WeakPtr;

using WebKit::WebFileInfo;
using WebKit::WebFileSystem;
using WebKit::WebFileSystemCallbacks;
using WebKit::WebFileSystemEntry;
using WebKit::WebFileWriter;
using WebKit::WebFileWriterClient;
using WebKit::WebFrame;
using WebKit::WebSecurityOrigin;
using WebKit::WebString;
using WebKit::WebVector;

using fileapi::FileSystemCallbackDispatcher;
using fileapi::FileSystemContext;
using fileapi::FileSystemFileUtil;
using fileapi::FileSystemOperation;

namespace {

class SimpleFileSystemCallbackDispatcher
    : public FileSystemCallbackDispatcher {
 public:
  SimpleFileSystemCallbackDispatcher(
      const WeakPtr<SimpleFileSystem>& file_system,
      WebFileSystemCallbacks* callbacks)
      : file_system_(file_system),
        callbacks_(callbacks) {
  }

  ~SimpleFileSystemCallbackDispatcher() {
  }

  virtual void DidSucceed() {
    DCHECK(file_system_);
    callbacks_->didSucceed();
  }

  virtual void DidReadMetadata(const base::PlatformFileInfo& info,
      const FilePath& platform_path) {
    DCHECK(file_system_);
    WebFileInfo web_file_info;
    web_file_info.length = info.size;
    web_file_info.modificationTime = info.last_modified.ToDoubleT();
    web_file_info.type = info.is_directory ?
        WebFileInfo::TypeDirectory : WebFileInfo::TypeFile;
    web_file_info.platformPath =
        webkit_glue::FilePathToWebString(platform_path);
    callbacks_->didReadMetadata(web_file_info);
  }

  virtual void DidReadDirectory(
      const std::vector<base::FileUtilProxy::Entry>& entries,
      bool has_more) {
    DCHECK(file_system_);
    std::vector<WebFileSystemEntry> web_entries_vector;
    for (std::vector<base::FileUtilProxy::Entry>::const_iterator it =
            entries.begin(); it != entries.end(); ++it) {
      WebFileSystemEntry entry;
      entry.name = webkit_glue::FilePathStringToWebString(it->name);
      entry.isDirectory = it->is_directory;
      web_entries_vector.push_back(entry);
    }
    WebVector<WebKit::WebFileSystemEntry> web_entries =
        web_entries_vector;
    callbacks_->didReadDirectory(web_entries, has_more);
  }

  virtual void DidOpenFileSystem(
      const std::string& name, const GURL& root) {
    DCHECK(file_system_);
    if (!root.is_valid())
      callbacks_->didFail(WebKit::WebFileErrorSecurity);
    else
      callbacks_->didOpenFileSystem(
          WebString::fromUTF8(name), WebString::fromUTF8(root.spec()));
  }

  virtual void DidFail(base::PlatformFileError error_code) {
    DCHECK(file_system_);
    callbacks_->didFail(
        webkit_glue::PlatformFileErrorToWebFileError(error_code));
  }

  virtual void DidWrite(int64, bool) {
    NOTREACHED();
  }

 private:
  WeakPtr<SimpleFileSystem> file_system_;
  WebFileSystemCallbacks* callbacks_;
};

}  // namespace

SimpleFileSystem::SimpleFileSystem() {
  if (file_system_dir_.CreateUniqueTempDir()) {
    file_system_context_ = new FileSystemContext(
        base::MessageLoopProxy::CreateForCurrentThread(),
        base::MessageLoopProxy::CreateForCurrentThread(),
        NULL /* special storage policy */,
        file_system_dir_.path(),
        false /* incognito */,
        true /* allow_file_access */,
        true /* unlimited_quota */);
  } else {
    LOG(WARNING) << "Failed to create a temp dir for the filesystem."
                    "FileSystem feature will be disabled.";
  }
}

SimpleFileSystem::~SimpleFileSystem() {
}

void SimpleFileSystem::OpenFileSystem(
    WebFrame* frame, WebFileSystem::Type web_filesystem_type,
    long long, bool create,
    WebFileSystemCallbacks* callbacks) {
  if (!frame || !file_system_context_.get()) {
    // The FileSystem temp directory was not initialized successfully.
    callbacks->didFail(WebKit::WebFileErrorSecurity);
    return;
  }

  fileapi::FileSystemType type;
  if (web_filesystem_type == WebFileSystem::TypeTemporary)
    type = fileapi::kFileSystemTypeTemporary;
  else if (web_filesystem_type == WebFileSystem::TypePersistent)
    type = fileapi::kFileSystemTypePersistent;
  else {
    // Unknown type filesystem is requested.
    callbacks->didFail(WebKit::WebFileErrorSecurity);
    return;
  }

  GURL origin_url(frame->securityOrigin().toString());
  GetNewOperation(callbacks)->OpenFileSystem(origin_url, type, create);
}

void SimpleFileSystem::move(
    const WebString& src_path,
    const WebString& dest_path, WebFileSystemCallbacks* callbacks) {
  GetNewOperation(callbacks)->Move(GURL(src_path), GURL(dest_path));
}

void SimpleFileSystem::copy(
    const WebString& src_path, const WebString& dest_path,
    WebFileSystemCallbacks* callbacks) {
  GetNewOperation(callbacks)->Copy(GURL(src_path), GURL(dest_path));
}

void SimpleFileSystem::remove(
    const WebString& path, WebFileSystemCallbacks* callbacks) {
  GetNewOperation(callbacks)->Remove(GURL(path), false /* recursive */);
}

void SimpleFileSystem::removeRecursively(
    const WebString& path, WebFileSystemCallbacks* callbacks) {
  GetNewOperation(callbacks)->Remove(GURL(path), true /* recursive */);
}

void SimpleFileSystem::readMetadata(
    const WebString& path, WebFileSystemCallbacks* callbacks) {
  GetNewOperation(callbacks)->GetMetadata(GURL(path));
}

void SimpleFileSystem::createFile(
    const WebString& path, bool exclusive, WebFileSystemCallbacks* callbacks) {
  GetNewOperation(callbacks)->CreateFile(GURL(path), exclusive);
}

void SimpleFileSystem::createDirectory(
    const WebString& path, bool exclusive, WebFileSystemCallbacks* callbacks) {
  GetNewOperation(callbacks)->CreateDirectory(GURL(path), exclusive, false);
}

void SimpleFileSystem::fileExists(
    const WebString& path, WebFileSystemCallbacks* callbacks) {
  GetNewOperation(callbacks)->FileExists(GURL(path));
}

void SimpleFileSystem::directoryExists(
    const WebString& path, WebFileSystemCallbacks* callbacks) {
  GetNewOperation(callbacks)->DirectoryExists(GURL(path));
}

void SimpleFileSystem::readDirectory(
    const WebString& path, WebFileSystemCallbacks* callbacks) {
  GetNewOperation(callbacks)->ReadDirectory(GURL(path));
}

WebFileWriter* SimpleFileSystem::createFileWriter(
    const WebString& path, WebFileWriterClient* client) {
  return new SimpleFileWriter(GURL(path), client, file_system_context_.get());
}

FileSystemOperation* SimpleFileSystem::GetNewOperation(
    WebFileSystemCallbacks* callbacks) {
  SimpleFileSystemCallbackDispatcher* dispatcher =
      new SimpleFileSystemCallbackDispatcher(AsWeakPtr(), callbacks);
  FileSystemOperation* operation = new FileSystemOperation(
      dispatcher, base::MessageLoopProxy::CreateForCurrentThread(),
      file_system_context_.get(), NULL);
  return operation;
}
