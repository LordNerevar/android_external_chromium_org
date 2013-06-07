// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_MEDIA_GALLERIES_FILEAPI_ITUNES_ITUNES_FILE_UTIL_H_
#define CHROME_BROWSER_MEDIA_GALLERIES_FILEAPI_ITUNES_ITUNES_FILE_UTIL_H_

#include "base/memory/weak_ptr.h"
#include "chrome/browser/media_galleries/fileapi/native_media_file_util.h"

namespace chrome {
class ImportedMediaGalleryRegistry;
}

namespace itunes {

class ITunesDataProvider;

class ItunesFileUtil : public chrome::NativeMediaFileUtil {
 public:
  ItunesFileUtil();
  virtual ~ItunesFileUtil();

 protected:
  // NativeMediaFileUtil overrides.
  virtual void GetFileInfoOnTaskRunnerThread(
      fileapi::FileSystemOperationContext* context,
      const fileapi::FileSystemURL& url,
      const GetFileInfoCallback& callback) OVERRIDE;
  virtual void ReadDirectoryOnTaskRunnerThread(
      fileapi::FileSystemOperationContext* context,
      const fileapi::FileSystemURL& url,
      const ReadDirectoryCallback& callback) OVERRIDE;
  virtual base::PlatformFileError GetFileInfoSync(
      fileapi::FileSystemOperationContext* context,
      const fileapi::FileSystemURL& url,
      base::PlatformFileInfo* file_info,
      base::FilePath* platform_path) OVERRIDE;
  virtual base::PlatformFileError ReadDirectorySync(
      fileapi::FileSystemOperationContext* context,
      const fileapi::FileSystemURL& url,
      EntryList* file_list) OVERRIDE;
  virtual base::PlatformFileError GetLocalFilePath(
      fileapi::FileSystemOperationContext* context,
      const fileapi::FileSystemURL& url,
      base::FilePath* local_file_path) OVERRIDE;

 private:
  void GetFileInfoWithFreshDataProvider(
      fileapi::FileSystemOperationContext* context,
      const fileapi::FileSystemURL& url,
      const GetFileInfoCallback& callback);
  void ReadDirectoryWithFreshDataProvider(
      fileapi::FileSystemOperationContext* context,
      const fileapi::FileSystemURL& url,
      const ReadDirectoryCallback& callback);

  ITunesDataProvider* GetDataProvider();

  base::WeakPtrFactory<ItunesFileUtil> weak_factory_;

  chrome::ImportedMediaGalleryRegistry* imported_registry_;

  DISALLOW_COPY_AND_ASSIGN(ItunesFileUtil);
};

}  // namespace itunes

#endif  // CHROME_BROWSER_MEDIA_GALLERIES_FILEAPI_ITUNES_ITUNES_FILE_UTIL_H_

