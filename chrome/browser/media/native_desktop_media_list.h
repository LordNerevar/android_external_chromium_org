// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_MEDIA_NATIVE_DESKTOP_MEDIA_LIST_H_
#define CHROME_BROWSER_MEDIA_NATIVE_DESKTOP_MEDIA_LIST_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/sequenced_task_runner.h"
#include "chrome/browser/media/desktop_media_list.h"
#include "content/public/browser/desktop_media_id.h"
#include "ui/gfx/image/image_skia.h"

namespace webrtc {
class ScreenCapturer;
class WindowCapturer;
}

// Implementation of DesktopMediaList that shows native screens and
// native windows.
class NativeDesktopMediaList : public DesktopMediaList {
 public:
  // Caller may pass NULL for either of the arguments in case when only some
  // types of sources the model should be populated with (e.g. it will only
  // contain windows, if |screen_capturer| is NULL).
  NativeDesktopMediaList(
      scoped_ptr<webrtc::ScreenCapturer> screen_capturer,
      scoped_ptr<webrtc::WindowCapturer> window_capturer);
  virtual ~NativeDesktopMediaList();

  // DesktopMediaList interface.
  virtual void SetUpdatePeriod(base::TimeDelta period) OVERRIDE;
  virtual void SetThumbnailSize(const gfx::Size& thumbnail_size) OVERRIDE;
  virtual void StartUpdating(DesktopMediaListObserver* observer) OVERRIDE;
  virtual int GetSourceCount() const OVERRIDE;
  virtual const Source& GetSource(int index) const OVERRIDE;
  virtual void SetViewDialogWindowId(
      content::DesktopMediaID::Id dialog_id) OVERRIDE;

 private:
  class Worker;
  friend class Worker;

  // Struct used to represent sources list the model gets from the Worker.
  struct SourceDescription {
    SourceDescription(content::DesktopMediaID id, const base::string16& name);

    content::DesktopMediaID id;
    base::string16 name;
  };

  // Order comparator for sources. Used to sort list of sources.
  static bool CompareSources(const SourceDescription& a,
                             const SourceDescription& b);

  // Post a task for the |worker_| to update list of windows and get thumbnails.
  void Refresh();

  // Called by |worker_| to refresh the model. First it posts tasks for
  // OnSourcesList() with the fresh list of sources, then follows with
  // OnSourceThumbnail() for each changed thumbnail and then calls
  // OnRefreshFinished() at the end.
  void OnSourcesList(const std::vector<SourceDescription>& sources);
  void OnSourceThumbnail(int index, const gfx::ImageSkia& thumbnail);
  void OnRefreshFinished();

  // Capturers specified in SetCapturers() and passed to the |worker_| later.
  scoped_ptr<webrtc::ScreenCapturer> screen_capturer_;
  scoped_ptr<webrtc::WindowCapturer> window_capturer_;

  // Time interval between mode updates.
  base::TimeDelta update_period_;

  // Size of thumbnails generated by the model.
  gfx::Size thumbnail_size_;

  // ID of the hosting dialog.
  content::DesktopMediaID::Id view_dialog_id_;

  // The observer passed to StartUpdating().
  DesktopMediaListObserver* observer_;

  // Task runner used for the |worker_|.
  scoped_refptr<base::SequencedTaskRunner> capture_task_runner_;

  // An object that does all the work of getting list of sources on a background
  // thread (see |capture_task_runner_|). Destroyed on |capture_task_runner_|
  // after the model is destroyed.
  scoped_ptr<Worker> worker_;

  // Current list of sources.
  std::vector<Source> sources_;

  base::WeakPtrFactory<NativeDesktopMediaList> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(NativeDesktopMediaList);
};

#endif  // CHROME_BROWSER_MEDIA_NATIVE_DESKTOP_MEDIA_LIST_H_
