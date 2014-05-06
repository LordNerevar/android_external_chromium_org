// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_SERVICE_WORKER_SERVICE_WORKER_DISK_CACHE_
#define CONTENT_BROWSER_SERVICE_WORKER_SERVICE_WORKER_DISK_CACHE_

#include "content/common/content_export.h"
#include "webkit/browser/appcache/appcache_disk_cache.h"

namespace content {

// Wholesale reusage of the appcache code for response reading,
// writing, and storage. See the corresponding class in that
// library for doc comments and other details.
// TODO(michaeln): If this reuse sticks, refactor/move the
// resused classes to a more common location.

class ServiceWorkerDiskCache
    : public appcache::AppCacheDiskCache {
};

class ServiceWorkerResponseReader
    : public appcache::AppCacheResponseReader {
 protected:
  // Should only be constructed by the storage class.
  friend class ServiceWorkerStorage;
  ServiceWorkerResponseReader(
      int64 response_id,
      ServiceWorkerDiskCache* disk_cache);
};

class ServiceWorkerResponseWriter
    : public appcache::AppCacheResponseWriter {
 protected:
  // Should only be constructed by the storage class.
  friend class ServiceWorkerStorage;
  ServiceWorkerResponseWriter(
      int64 response_id,
      ServiceWorkerDiskCache* disk_cache);
};

struct HttpResponseInfoIOBuffer
    : public appcache::HttpResponseInfoIOBuffer {
 protected:
  virtual ~HttpResponseInfoIOBuffer();
};

}  // namespace content

#endif  // CONTENT_BROWSER_SERVICE_WORKER_SERVICE_WORKER_DISK_CACHE_
