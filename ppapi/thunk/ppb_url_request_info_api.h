// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_THUNK_URL_REQUEST_INFO_API_H_
#define PPAPI_THUNK_URL_REQUEST_INFO_API_H_

#include "ppapi/c/ppb_url_request_info.h"

namespace ppapi {
namespace thunk {

class PPB_URLRequestInfo_API {
 public:
  virtual PP_Bool SetProperty(PP_URLRequestProperty property,
                              PP_Var var) = 0;
  virtual PP_Bool AppendDataToBody(const void* data, uint32_t len) = 0;
  virtual PP_Bool AppendFileToBody(PP_Resource file_ref,
                                   int64_t start_offset,
                                   int64_t number_of_bytes,
                                   PP_Time expected_last_modified_time) = 0;
};

}  // namespace thunk
}  // namespace ppapi

#endif  // PPAPI_THUNK_URL_REQUEST_INFO_API_H_
