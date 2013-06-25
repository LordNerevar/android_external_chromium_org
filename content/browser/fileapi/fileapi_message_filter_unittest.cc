// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/fileapi/fileapi_message_filter.h"

#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "base/message_loop.h"
#include "content/browser/fileapi/chrome_blob_storage_context.h"
#include "content/common/fileapi/webblob_messages.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/common_param_traits.h"
#include "content/public/test/mock_render_process_host.h"
#include "content/public/test/test_browser_context.h"
#include "content/public/test/test_browser_thread.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "webkit/browser/blob/blob_storage_controller.h"
#include "webkit/browser/fileapi/file_system_context.h"
#include "webkit/browser/fileapi/mock_file_system_context.h"
#include "webkit/common/blob/blob_data.h"

namespace content {

class FileAPIMessageFilterTest : public testing::Test {
 public:
  FileAPIMessageFilterTest()
      : io_browser_thread_(BrowserThread::IO, &message_loop_) {
  }

 protected:
  virtual void SetUp() OVERRIDE {
    file_system_context_ =
        fileapi::CreateFileSystemContextForTesting(NULL, base::FilePath());
  }

  base::MessageLoop message_loop_;
  TestBrowserThread io_browser_thread_;

  TestBrowserContext browser_context_;
  scoped_refptr<fileapi::FileSystemContext> file_system_context_;
};

TEST_F(FileAPIMessageFilterTest, BuildEmptyBlob) {
  scoped_refptr<FileAPIMessageFilter> filter(
      new FileAPIMessageFilter(
          0 /* process_id */,
          browser_context_.GetRequestContext(),
          file_system_context_.get(),
          ChromeBlobStorageContext::GetFor(&browser_context_)));

  // Complete initialization.
  message_loop_.RunUntilIdle();

  webkit_blob::BlobStorageController* controller =
      ChromeBlobStorageContext::GetFor(&browser_context_)->controller();

  const GURL kUrl("blob:foobar");
  const GURL kDifferentUrl("blob:barfoo");
  const std::string kContentType = "fake/type";

  EXPECT_EQ(NULL, controller->GetBlobDataFromUrl(kUrl));

  // Test via OnMessageReceived(const IPC::Message&) which is called by the
  // channel proxy. Since OnMessageReceived is hidden on FileAPIMessageFilter,
  // cast it.
  IPC::ChannelProxy::MessageFilter* casted_filter =
      static_cast<IPC::ChannelProxy::MessageFilter*>(filter);

  BlobHostMsg_StartBuildingBlob start_message(kUrl);
  EXPECT_TRUE(casted_filter->OnMessageReceived(start_message));

  // Blob is still being built. Nothing should be returned.
  EXPECT_EQ(NULL, controller->GetBlobDataFromUrl(kUrl));

  BlobHostMsg_FinishBuildingBlob finish_message(kUrl, kContentType);
  EXPECT_TRUE(casted_filter->OnMessageReceived(finish_message));

  // Now, Blob is built.
  webkit_blob::BlobData* blob_data = controller->GetBlobDataFromUrl(kUrl);
  ASSERT_FALSE(blob_data == NULL);
  EXPECT_EQ(0U, blob_data->items().size());
  EXPECT_EQ(kContentType, blob_data->content_type());

  // Nothing should be returned for a URL we didn't use.
  EXPECT_TRUE(controller->GetBlobDataFromUrl(kDifferentUrl) == NULL);
}

}  // namespace fileapi
