// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/download/download_file_picker_chromeos.h"

#include "base/bind.h"
#include "base/file_util.h"
#include "base/memory/ref_counted.h"
#include "base/threading/sequenced_worker_pool.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/download_item.h"
#include "content/public/browser/download_manager.h"
#include "chrome/browser/chromeos/gdata/gdata_download_observer.h"
#include "chrome/browser/chromeos/gdata/gdata_file_system.h"
#include "chrome/browser/chromeos/gdata/gdata_util.h"
#include "chrome/browser/profiles/profile_manager.h"

using content::BrowserThread;
using content::DownloadManager;

namespace {

// Callbacks for PostTaskAndReply.

// |temp_path| is set to a temporary local download path in
// ~/GCache/v1/tmp/downloads/
// Must be called on the FILE thread.
void GetGDataTempDownloadPath(const FilePath& download_cache_path,
                              FilePath* temp_path) {
  if (!file_util::CreateDirectory(download_cache_path)) {
    NOTREACHED();
    LOG(ERROR) << "Can not create temp download directory at "
               << download_cache_path.value();
  }
  bool created = file_util::CreateTemporaryFileInDir(download_cache_path,
                                                     temp_path);
  // TODO(achuith): Handle failure in CreateTemporaryFileInDir.
  DCHECK(created);
}

// Call FileSelected on |download_manager|.
void GDataTempFileSelected(DownloadManager* download_manager,
                           FilePath* file_path,
                           int32 download_id) {
  download_manager->FileSelected(*file_path, download_id);
}

}  // namespace

DownloadFilePickerChromeOS::DownloadFilePickerChromeOS(
    content::DownloadManager* download_manager,
    content::WebContents* web_contents,
    const FilePath& suggested_path,
    int32 download_id)
    : DownloadFilePicker(download_manager,
                         web_contents,
                         suggested_path,
                         download_id) {
}

DownloadFilePickerChromeOS::~DownloadFilePickerChromeOS() {
}

void DownloadFilePickerChromeOS::FileSelected(const FilePath& path,
                                              int index,
                                              void* params) {
  RecordFileSelected(path);

  if (download_manager_) {
    gdata::GDataFileSystem* file_system =
        gdata::GDataFileSystemFactory::GetForProfile(
            ProfileManager::GetDefaultProfile());
    if (file_system && gdata::util::IsUnderGDataMountPoint(path)) {
      // If we're trying to download a file into gdata, save path in external
      // data.
      content::DownloadItem* download =
          download_manager_->GetActiveDownloadItem(download_id_);
      if (download) {
        gdata::GDataDownloadObserver::SetGDataPath(download, path);
        download->SetDisplayName(path.BaseName());

        const FilePath download_cache_path =
            file_system->GetGDataTempDownloadFolderPath();

        // Swap the gdata path with a local path. Local path must be created
        // on the IO thread pool.
        FilePath* download_path(new FilePath());
        BrowserThread::GetBlockingPool()->PostTaskAndReply(FROM_HERE,
            base::Bind(&GetGDataTempDownloadPath,
                       download_cache_path,
                       download_path),
            base::Bind(&GDataTempFileSelected,
                       download_manager_,
                       base::Owned(download_path),
                       download_id_));
      }
    } else {
      download_manager_->FileSelected(path, download_id_);
    }
  }
  delete this;
}
