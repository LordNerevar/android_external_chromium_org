// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_THUNK_PPB_TCP_SOCKET_API_H_
#define PPAPI_THUNK_PPB_TCP_SOCKET_API_H_

#include "base/memory/ref_counted.h"
#include "ppapi/c/dev/ppb_tcp_socket_dev.h"
#include "ppapi/thunk/ppapi_thunk_export.h"

namespace ppapi {

class TrackedCallback;

namespace thunk {

class PPAPI_THUNK_EXPORT PPB_TCPSocket_API {
 public:
  virtual ~PPB_TCPSocket_API() {}

  virtual int32_t Connect(PP_Resource addr,
                          scoped_refptr<TrackedCallback> callback) = 0;
  virtual PP_Resource GetLocalAddress() = 0;
  virtual PP_Resource GetRemoteAddress() = 0;
  virtual int32_t Read(char* buffer,
                       int32_t bytes_to_read,
                       scoped_refptr<TrackedCallback> callback) = 0;
  virtual int32_t Write(const char* buffer,
                        int32_t bytes_to_write,
                        scoped_refptr<TrackedCallback> callback) = 0;
  virtual void Close() = 0;
  virtual int32_t SetOption(PP_TCPSocket_Option_Dev name,
                            const PP_Var& value,
                            scoped_refptr<TrackedCallback> callback) = 0;
};

}  // namespace thunk
}  // namespace ppapi

#endif  // PPAPI_THUNK_PPB_TCP_SOCKET_API_H_
