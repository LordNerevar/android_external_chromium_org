// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PROFILES_PROFILE_DOWNLOADER_H_
#define CHROME_BROWSER_PROFILES_PROFILE_DOWNLOADER_H_
#pragma once

#include <string>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/string16.h"
#include "chrome/browser/image_decoder.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/common/url_fetcher_delegate.h"
#include "googleurl/src/gurl.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "chrome/common/net/gaia/oauth2_access_token_consumer.h"

class ProfileDownloaderDelegate;
class OAuth2AccessTokenFetcher;

// Downloads user profile information. The profile picture is decoded in a
// sandboxed process.
class ProfileDownloader : public content::URLFetcherDelegate,
                          public ImageDecoder::Delegate,
                          public content::NotificationObserver,
                          public OAuth2AccessTokenConsumer {
 public:
  enum PictureStatus {
    PICTURE_SUCCESS,
    PICTURE_FAILED,
    PICTURE_DEFAULT,
    PICTURE_CACHED,
  };

  explicit ProfileDownloader(ProfileDownloaderDelegate* delegate);
  virtual ~ProfileDownloader();

  // Starts downloading profile information if the necessary authorization token
  // is ready. If not, subscribes to token service and starts fetching if the
  // token is available. Should not be called more than once.
  virtual void Start();

  // On successful download this returns the full name of the user. For example
  // "Pat Smith".
  virtual string16 GetProfileFullName() const;

  // On successful download this returns the profile picture of the user.
  // For users with no profile picture set (that is, they have the default
  // profile picture) this will return an Null bitmap.
  virtual SkBitmap GetProfilePicture() const;

  // Gets the profile picture status.
  virtual PictureStatus GetProfilePictureStatus() const;

  // Gets the URL for the profile picture. This can be cached so that the same
  // picture is not downloaded multiple times. This value should only be used
  // when the picture status is PICTURE_SUCCESS.
  virtual std::string GetProfilePictureURL() const;

 private:
  // Overriden from content::URLFetcherDelegate:
  virtual void OnURLFetchComplete(const content::URLFetcher* source) OVERRIDE;

  // Overriden from ImageDecoder::Delegate:
  virtual void OnImageDecoded(const ImageDecoder* decoder,
                              const SkBitmap& decoded_image) OVERRIDE;
  virtual void OnDecodeImageFailed(const ImageDecoder* decoder) OVERRIDE;

  // Overriden from content::NotificationObserver:
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  // Overriden from OAuth2AccessTokenConsumer:
  virtual void OnGetTokenSuccess(const std::string& access_token) OVERRIDE;
  virtual void OnGetTokenFailure(const GoogleServiceAuthError& error) OVERRIDE;

  // Parses the entry response from Picasa and gets the nick name and
  // and profile image URL. Returns false to indicate a parsing error.
  bool GetProfileNickNameAndImageURL(const std::string& data,
                                     string16* nick_name,
                                     std::string* url) const;

  // Returns true if the image url is url of the default profile picture.
  bool IsDefaultProfileImageURL(const std::string& url) const;

  // Issues the first request to get user profile image.
  void StartFetchingImage();

  // Gets the authorization header.
  const char* GetAuthorizationHeader() const;

  // Starts fetching OAuth2 access token. This is needed before the GAIA info
  // can be downloaded.
  void StartFetchingOAuth2AccessToken();

  ProfileDownloaderDelegate* delegate_;
  std::string auth_token_;
  scoped_ptr<content::URLFetcher> user_entry_fetcher_;
  scoped_ptr<content::URLFetcher> profile_image_fetcher_;
  scoped_ptr<OAuth2AccessTokenFetcher> oauth2_access_token_fetcher_;
  content::NotificationRegistrar registrar_;
  string16 profile_full_name_;
  SkBitmap profile_picture_;
  PictureStatus picture_status_;
  std::string picture_url_;

  DISALLOW_COPY_AND_ASSIGN(ProfileDownloader);
};

#endif  // CHROME_BROWSER_PROFILES_PROFILE_DOWNLOADER_H_
