// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_ANDROID_CONTENT_VIEW_CORE_H_
#define CONTENT_PUBLIC_BROWSER_ANDROID_CONTENT_VIEW_CORE_H_

#include "base/android/scoped_java_ref.h"
#include <jni.h>

#include "content/public/browser/navigation_controller.h"

namespace ui {
class WindowAndroid;
}

namespace content {
class WebContents;

// Native side of the ContentViewCore.java, which is the primary way of
// communicating with the native Chromium code on Android.  This is a
// public interface used by native code outside of the content module.
class ContentViewCore {
 public:
  virtual void Destroy(JNIEnv* env, jobject obj) = 0;
  virtual WebContents* GetWebContents() const = 0;
  virtual base::android::ScopedJavaLocalRef<jobject> GetJavaObject() = 0;
  virtual ui::WindowAndroid* GetWindowAndroid() = 0;
  virtual void LoadUrl(NavigationController::LoadURLParams& params) = 0;

  static ContentViewCore* Create(
      JNIEnv* env, jobject obj, WebContents* web_contents);
  static ContentViewCore* GetNativeContentViewCore(JNIEnv* env, jobject obj);
 protected:
  virtual ~ContentViewCore() {};
};

};  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_ANDROID_CONTENT_VIEW_CORE_H_
