// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/geolocation/geolocation_permission_context.h"

#include "app/l10n_util.h"
#include "app/resource_bundle.h"
#include "base/file_util.h"
#include "googleurl/src/gurl.h"
#include "chrome/browser/browser.h"
#include "chrome/browser/browser_list.h"
#include "chrome/browser/chrome_thread.h"
#include "chrome/browser/geolocation/geolocation_dispatcher_host.h"
#include "chrome/browser/profile.h"
#include "chrome/browser/renderer_host/render_process_host.h"
#include "chrome/browser/renderer_host/render_view_host.h"
#include "chrome/browser/renderer_host/render_view_host_notification_task.h"
#include "chrome/browser/tab_contents/infobar_delegate.h"
#include "chrome/browser/tab_contents/tab_contents.h"
#include "chrome/common/json_value_serializer.h"
#include "chrome/common/render_messages.h"
#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

namespace {

const FilePath::CharType kGeolocationPermissionPath[] =
    FILE_PATH_LITERAL("Geolocation");

const wchar_t kAllowedDictionaryKey[] = L"allowed";

// This is the delegate used to display the confirmation info bar.
class GeolocationConfirmInfoBarDelegate : public ConfirmInfoBarDelegate {
 public:
  GeolocationConfirmInfoBarDelegate(
      TabContents* tab_contents, GeolocationPermissionContext* context,
      int render_process_id, int render_view_id, int bridge_id,
      const GURL& origin)
    : ConfirmInfoBarDelegate(tab_contents), context_(context),
      render_process_id_(render_process_id), render_view_id_(render_view_id),
      bridge_id_(bridge_id), origin_(origin) {
  }

  // ConfirmInfoBarDelegate
  virtual Type GetInfoBarType() { return INFO_TYPE; }
  virtual bool Accept() { return SetPermission(true); }
  virtual bool Cancel() { return SetPermission(false); }
  virtual int GetButtons() const { return BUTTON_OK | BUTTON_CANCEL; }

  virtual std::wstring GetButtonLabel(InfoBarButton button) const {
    switch (button) {
      case BUTTON_OK:
        return l10n_util::GetString(IDS_GEOLOCATION_ALLOW_BUTTON);
      case BUTTON_CANCEL:
        return l10n_util::GetString(IDS_GEOLOCATION_DENY_BUTTON);
      default:
        // All buttons are labeled above.
        NOTREACHED() << "Bad button id " << button;
        return L"";
    }
  }

  virtual std::wstring GetMessageText() const {
    return l10n_util::GetStringF(
        IDS_GEOLOCATION_INFOBAR_QUESTION, UTF8ToWide(origin_.host()));
  }

  virtual SkBitmap* GetIcon() const {
    return ResourceBundle::GetSharedInstance().GetBitmapNamed(
        IDR_GEOLOCATION_INFOBAR_ICON);
  }

 private:
  bool SetPermission(bool confirm) {
    context_->SetPermission(
        render_process_id_, render_view_id_, bridge_id_, origin_, confirm);
    return true;
  }

  scoped_refptr<GeolocationPermissionContext> context_;
  int render_process_id_;
  int render_view_id_;
  int bridge_id_;
  GURL origin_;
};

// TODO(bulach): use HostContentSettingsMap instead!
FilePath::StringType StdStringToFilePathString(const std::string& std_string) {
#if defined(OS_WIN)
  return UTF8ToWide(std_string);
#else
  return std_string;
#endif
}

std::string GURLToCacheKey(const GURL& gurl) {
  return gurl.host();
}

// Returns true if permission was successfully read from file, and *allowed
// be set accordingly.
// Returns false otherwise.
bool ReadPermissionFromFile(
    const GURL& origin, const FilePath& permissions_path, bool* allowed) {
  DCHECK(allowed);
  *allowed = false;
  // TODO(bulach): this is probably wrong! is there any utility to convert a URL
  // to FilePath?
  FilePath permission_file(
      permissions_path.Append(StdStringToFilePathString(
          GURLToCacheKey(origin))));
  if (!file_util::PathExists(permission_file))
    return false;
  JSONFileValueSerializer serializer(permission_file);
  scoped_ptr<Value> root_value(serializer.Deserialize(NULL));
  bool ret = root_value.get() &&
             root_value->GetType() == Value::TYPE_DICTIONARY;
  DictionaryValue* dictionary = static_cast<DictionaryValue*>(root_value.get());
  return ret &&
         dictionary->GetBoolean(kAllowedDictionaryKey, allowed);
}

void SavePermissionToFile(
    const GURL& origin, const FilePath& permissions_path, bool allowed) {
#if 0
  if (!file_util::DirectoryExists(permissions_path))
    file_util::CreateDirectory(permissions_path);
  // TODO(bulach): this is probably wrong! is there any utility to convert a URL
  // to FilePath?
  FilePath permission_file(
      permissions_path.Append(StdStringToFilePathString(
          GURLToCacheKey(origin))));
  DictionaryValue dictionary;
  dictionary.SetBoolean(kAllowedDictionaryKey, allowed);
  std::string permission_data;
  JSONStringValueSerializer serializer(&permission_data);
  serializer.Serialize(dictionary);
  file_util::WriteFile(
      permission_file, permission_data.c_str(), permission_data.length());
#endif  // if 0
}

}  // namespace

GeolocationPermissionContext::GeolocationPermissionContext(
    Profile* profile)
    : profile_(profile),
      is_off_the_record_(profile->IsOffTheRecord()),
      permissions_path_(profile->GetPath().Append(FilePath(
          kGeolocationPermissionPath))) {
}

GeolocationPermissionContext::~GeolocationPermissionContext() {
}

void GeolocationPermissionContext::RequestGeolocationPermission(
    int render_process_id, int render_view_id, int bridge_id,
    const GURL& origin) {
  DCHECK(ChromeThread::CurrentlyOn(ChromeThread::IO));
  std::map<std::string, bool>::const_iterator permission =
      permissions_.find(GURLToCacheKey(origin));
  if (permission != permissions_.end()) {
    NotifyPermissionSet(
        render_process_id, render_view_id, bridge_id, permission->second);
  } else {
    HandlePermissionMemoryCacheMiss(
        render_process_id, render_view_id, bridge_id, origin);
  }
}

void GeolocationPermissionContext::SetPermission(
    int render_process_id, int render_view_id, int bridge_id,
    const GURL& origin, bool allowed) {
  SetPermissionMemoryCacheOnIOThread(origin, allowed);
  SetPermissionOnFileThread(origin, allowed);
  NotifyPermissionSet(render_process_id, render_view_id, bridge_id, allowed);
}

void GeolocationPermissionContext::HandlePermissionMemoryCacheMiss(
    int render_process_id, int render_view_id, int bridge_id,
    const GURL& origin) {
  if (!ChromeThread::CurrentlyOn(ChromeThread::FILE)) {
    ChromeThread::PostTask(
        ChromeThread::FILE, FROM_HERE,
        NewRunnableMethod(
            this,
            &GeolocationPermissionContext::HandlePermissionMemoryCacheMiss,
            render_process_id, render_view_id, bridge_id, origin));
    return;
  }

  DCHECK(ChromeThread::CurrentlyOn(ChromeThread::FILE));
  // TODO(bulach): should we save a file per origin or have some smarter
  // storage? Use HostContentSettingsMap instead.
  bool allowed;
  if (is_off_the_record_ ||
      !ReadPermissionFromFile(origin, permissions_path_, &allowed)) {
    RequestPermissionFromUI(
        render_process_id, render_view_id, bridge_id, origin);
  } else {
    SetPermissionMemoryCacheOnIOThread(origin, allowed);
    NotifyPermissionSet(render_process_id, render_view_id, bridge_id, allowed);
  }
}

void GeolocationPermissionContext::RequestPermissionFromUI(
    int render_process_id, int render_view_id, int bridge_id,
    const GURL& origin) {
  if (!ChromeThread::CurrentlyOn(ChromeThread::UI)) {
    ChromeThread::PostTask(
        ChromeThread::UI, FROM_HERE,
        NewRunnableMethod(
            this, &GeolocationPermissionContext::RequestPermissionFromUI,
            render_process_id, render_view_id, bridge_id, origin));
    return;
  }
  DCHECK(ChromeThread::CurrentlyOn(ChromeThread::UI));

  Browser* browser = BrowserList::GetLastActiveWithProfile(profile_);
  for (int i = 0; i < browser->tab_count(); ++i) {
    TabContents* tab_contents = browser->GetTabContentsAt(i);
    RenderViewHost* render_view_host = tab_contents->render_view_host();
    if (render_view_host->process()->id() == render_process_id &&
        render_view_host->routing_id() == render_view_id &&
        tab_contents->GetURL().GetOrigin() == origin) {
      tab_contents->AddInfoBar(
          new GeolocationConfirmInfoBarDelegate(
              tab_contents, this, render_process_id, render_view_id,
              bridge_id, origin));
      break;
    }
  }
}

void GeolocationPermissionContext::NotifyPermissionSet(
    int render_process_id, int render_view_id, int bridge_id, bool allowed) {
  if (!ChromeThread::CurrentlyOn(ChromeThread::UI)) {
    ChromeThread::PostTask(
        ChromeThread::UI, FROM_HERE,
        NewRunnableMethod(
            this, &GeolocationPermissionContext::NotifyPermissionSet,
            render_process_id, render_view_id, bridge_id, allowed));
    return;
  }

  DCHECK(ChromeThread::CurrentlyOn(ChromeThread::UI));
  CallRenderViewHost(
      render_process_id, render_view_id,
      &RenderViewHost::Send,
      new ViewMsg_Geolocation_PermissionSet(
          render_view_id, bridge_id, allowed));
}

void GeolocationPermissionContext::SetPermissionMemoryCacheOnIOThread(
    const GURL& origin, bool allowed) {
  if (!ChromeThread::CurrentlyOn(ChromeThread::IO)) {
    ChromeThread::PostTask(
        ChromeThread::IO, FROM_HERE,
        NewRunnableMethod(
            this,
            &GeolocationPermissionContext::SetPermissionMemoryCacheOnIOThread,
            origin, allowed));
    return;
  }

  DCHECK(ChromeThread::CurrentlyOn(ChromeThread::IO));
  permissions_[GURLToCacheKey(origin)] = allowed;
}

void GeolocationPermissionContext::SetPermissionOnFileThread(
    const GURL& origin, bool allowed) {
  if (is_off_the_record_)
    return;
  if (!ChromeThread::CurrentlyOn(ChromeThread::FILE)) {
    ChromeThread::PostTask(
        ChromeThread::FILE, FROM_HERE,
        NewRunnableMethod(
            this, &GeolocationPermissionContext::SetPermissionOnFileThread,
            origin, allowed));
    return;
  }

  DCHECK(ChromeThread::CurrentlyOn(ChromeThread::FILE));

  // TODO(bulach): should we save a file per origin or have some smarter
  // storage? Use HostContentSettingsMap instead.
#if 0
  SavePermissionToFile(origin, permissions_path_, allowed);
#endif
}
