// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/extensions_ui.h"

#include <algorithm>

#include "base/base64.h"
#include "base/callback.h"
#include "base/file_util.h"
#include "base/memory/singleton.h"
#include "base/string_number_conversions.h"
#include "base/string_util.h"
#include "base/threading/thread.h"
#include "base/utf_string_conversions.h"
#include "base/version.h"
#include "chrome/browser/debugger/devtools_window.h"
#include "chrome/browser/extensions/crx_installer.h"
#include "chrome/browser/extensions/extension_disabled_infobar_delegate.h"
#include "chrome/browser/extensions/extension_error_reporter.h"
#include "chrome/browser/extensions/extension_host.h"
#include "chrome/browser/extensions/extension_message_service.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/extension_updater.h"
#include "chrome/browser/google/google_util.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/tab_contents/background_contents.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/webui/chrome_web_ui_data_source.h"
#include "chrome/browser/ui/webui/extension_icon_source.h"
#include "chrome/common/chrome_notification_types.h"
#include "chrome/common/extensions/extension.h"
#include "chrome/common/extensions/extension_icon_set.h"
#include "chrome/common/extensions/url_pattern.h"
#include "chrome/common/extensions/user_script.h"
#include "chrome/common/jstemplate_builder.h"
#include "chrome/common/pref_names.h"
#include "chrome/common/url_constants.h"
#include "content/browser/renderer_host/render_process_host.h"
#include "content/browser/renderer_host/render_view_host.h"
#include "content/browser/renderer_host/render_widget_host.h"
#include "content/browser/tab_contents/tab_contents.h"
#include "content/browser/tab_contents/tab_contents_view.h"
#include "content/common/content_notification_types.h"
#include "content/common/notification_service.h"
#include "googleurl/src/gurl.h"
#include "grit/browser_resources.h"
#include "grit/chromium_strings.h"
#include "grit/generated_resources.h"
#include "grit/theme_resources.h"
#include "grit/theme_resources_standard.h"
#include "net/base/net_util.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"

namespace {

bool ShouldShowExtension(const Extension* extension) {
  // Don't show themes since this page's UI isn't really useful for themes.
  if (extension->is_theme())
    return false;

  // Don't show component extensions because they are only extensions as an
  // implementation detail of Chrome.
  if (extension->location() == Extension::COMPONENT)
    return false;

  // Always show unpacked extensions and apps.
  if (extension->location() == Extension::LOAD)
    return true;

  // Unless they are unpacked, never show hosted apps.
  if (extension->is_hosted_app())
    return false;

  return true;
}

}  // namespace

ChromeWebUIDataSource* CreateExtensionsUIHTMLSource() {
  ChromeWebUIDataSource* source =
      new ChromeWebUIDataSource(chrome::kChromeUIExtensionsHost);

  source->AddLocalizedString("title", IDS_EXTENSIONS_TITLE);
  source->AddLocalizedString("devModeLink", IDS_EXTENSIONS_DEVELOPER_MODE_LINK);
  source->AddLocalizedString("devModePrefix",
                             IDS_EXTENSIONS_DEVELOPER_MODE_PREFIX);
  source->AddLocalizedString("loadUnpackedButton",
                             IDS_EXTENSIONS_LOAD_UNPACKED_BUTTON);
  source->AddLocalizedString("packButton", IDS_EXTENSIONS_PACK_BUTTON);
  source->AddLocalizedString("updateButton", IDS_EXTENSIONS_UPDATE_BUTTON);
  source->AddLocalizedString("noExtensions", IDS_EXTENSIONS_NONE_INSTALLED);
  source->AddLocalizedString("extensionCrashed",
                             IDS_EXTENSIONS_CRASHED_EXTENSION);
  source->AddLocalizedString("extensionDisabled",
                             IDS_EXTENSIONS_DISABLED_EXTENSION);
  source->AddLocalizedString("inDevelopment", IDS_EXTENSIONS_IN_DEVELOPMENT);
  source->AddLocalizedString("viewIncognito", IDS_EXTENSIONS_VIEW_INCOGNITO);
  source->AddLocalizedString("extensionId", IDS_EXTENSIONS_ID);
  source->AddLocalizedString("extensionPath", IDS_EXTENSIONS_PATH);
  source->AddLocalizedString("extensionVersion", IDS_EXTENSIONS_VERSION);
  source->AddLocalizedString("inspectViews", IDS_EXTENSIONS_INSPECT_VIEWS);
  source->AddLocalizedString("inspectPopupsInstructions",
                             IDS_EXTENSIONS_INSPECT_POPUPS_INSTRUCTIONS);
  source->AddLocalizedString("disable", IDS_EXTENSIONS_DISABLE);
  source->AddLocalizedString("enable", IDS_EXTENSIONS_ENABLE);
  source->AddLocalizedString("enableIncognito",
                             IDS_EXTENSIONS_ENABLE_INCOGNITO);
  source->AddLocalizedString("allowFileAccess",
                             IDS_EXTENSIONS_ALLOW_FILE_ACCESS);
  source->AddLocalizedString("reload", IDS_EXTENSIONS_RELOAD);
  source->AddLocalizedString("uninstall", IDS_EXTENSIONS_UNINSTALL);
  source->AddLocalizedString("options", IDS_EXTENSIONS_OPTIONS);
  source->AddLocalizedString("policyControlled",
                             IDS_EXTENSIONS_POLICY_CONTROLLED);
  source->AddLocalizedString("packDialogTitle",
                             IDS_EXTENSION_PACK_DIALOG_TITLE);
  source->AddLocalizedString("packDialogHeading",
                             IDS_EXTENSION_PACK_DIALOG_HEADING);
  source->AddLocalizedString("rootDirectoryLabel",
                             IDS_EXTENSION_PACK_DIALOG_ROOT_DIRECTORY_LABEL);
  source->AddLocalizedString("packDialogBrowse",
                             IDS_EXTENSION_PACK_DIALOG_BROWSE);
  source->AddLocalizedString("privateKeyLabel",
                             IDS_EXTENSION_PACK_DIALOG_PRIVATE_KEY_LABEL);
  source->AddLocalizedString("okButton", IDS_OK);
  source->AddLocalizedString("cancelButton", IDS_CANCEL);
  source->AddLocalizedString("showButton", IDS_EXTENSIONS_SHOW_BUTTON);

  source->AddString("incognitoWarning",
      l10n_util::GetStringFUTF16(IDS_EXTENSIONS_INCOGNITO_WARNING,
                                 l10n_util::GetStringUTF16(IDS_PRODUCT_NAME)));

  source->AddString("suggestGallery",
      l10n_util::GetStringFUTF16(IDS_EXTENSIONS_NONE_INSTALLED_SUGGEST_GALLERY,
          ASCIIToUTF16("<a href='") +
              ASCIIToUTF16(google_util::AppendGoogleLocaleParam(
                  GURL(extension_misc::GetWebstoreLaunchURL())).spec()) +
              ASCIIToUTF16("'>"),
          ASCIIToUTF16("</a>")));

  source->AddString("getMoreExtensions",
      ASCIIToUTF16("<a href='") +
      ASCIIToUTF16(google_util::AppendGoogleLocaleParam(
          GURL(extension_misc::GetWebstoreLaunchURL())).spec()) +
      ASCIIToUTF16("'>") +
      l10n_util::GetStringUTF16(IDS_GET_MORE_EXTENSIONS) +
      ASCIIToUTF16("</a>"));

  source->set_json_path("strings.js");
  source->add_resource_path("extensions_ui.js", IDR_EXTENSIONS_UI_JS);
  source->set_default_resource(IDR_EXTENSIONS_UI_HTML);
  return source;
}

///////////////////////////////////////////////////////////////////////////////
//
// ExtensionsDOMHandler
//
///////////////////////////////////////////////////////////////////////////////

ExtensionsDOMHandler::ExtensionsDOMHandler(ExtensionService* extension_service)
    : extension_service_(extension_service),
      ignore_notifications_(false),
      deleting_rvh_(NULL) {
  RegisterForNotifications();
}

void ExtensionsDOMHandler::RegisterMessages() {
  web_ui_->RegisterMessageCallback("requestExtensionsData",
      NewCallback(this, &ExtensionsDOMHandler::HandleRequestExtensionsData));
  web_ui_->RegisterMessageCallback("toggleDeveloperMode",
      NewCallback(this, &ExtensionsDOMHandler::HandleToggleDeveloperMode));
  web_ui_->RegisterMessageCallback("inspect",
      NewCallback(this, &ExtensionsDOMHandler::HandleInspectMessage));
  web_ui_->RegisterMessageCallback("reload",
      NewCallback(this, &ExtensionsDOMHandler::HandleReloadMessage));
  web_ui_->RegisterMessageCallback("enable",
      NewCallback(this, &ExtensionsDOMHandler::HandleEnableMessage));
  web_ui_->RegisterMessageCallback("enableIncognito",
      NewCallback(this, &ExtensionsDOMHandler::HandleEnableIncognitoMessage));
  web_ui_->RegisterMessageCallback("allowFileAccess",
      NewCallback(this, &ExtensionsDOMHandler::HandleAllowFileAccessMessage));
  web_ui_->RegisterMessageCallback("uninstall",
      NewCallback(this, &ExtensionsDOMHandler::HandleUninstallMessage));
  web_ui_->RegisterMessageCallback("options",
      NewCallback(this, &ExtensionsDOMHandler::HandleOptionsMessage));
  web_ui_->RegisterMessageCallback("showButton",
      NewCallback(this, &ExtensionsDOMHandler::HandleShowButtonMessage));
  web_ui_->RegisterMessageCallback("load",
      NewCallback(this, &ExtensionsDOMHandler::HandleLoadMessage));
  web_ui_->RegisterMessageCallback("pack",
      NewCallback(this, &ExtensionsDOMHandler::HandlePackMessage));
  web_ui_->RegisterMessageCallback("autoupdate",
      NewCallback(this, &ExtensionsDOMHandler::HandleAutoUpdateMessage));
  web_ui_->RegisterMessageCallback("selectFilePath",
      NewCallback(this, &ExtensionsDOMHandler::HandleSelectFilePathMessage));
}

void ExtensionsDOMHandler::HandleRequestExtensionsData(const ListValue* args) {
  DictionaryValue results;

  // Add the extensions to the results structure.
  ListValue* extensions_list = new ListValue();

  const ExtensionList* extensions = extension_service_->extensions();
  for (ExtensionList::const_iterator extension = extensions->begin();
       extension != extensions->end(); ++extension) {
    if (ShouldShowExtension(*extension)) {
      extensions_list->Append(CreateExtensionDetailValue(
          extension_service_,
          *extension,
          GetActivePagesForExtension(*extension),
          true, false));  // enabled, terminated
    }
  }
  extensions = extension_service_->disabled_extensions();
  for (ExtensionList::const_iterator extension = extensions->begin();
       extension != extensions->end(); ++extension) {
    if (ShouldShowExtension(*extension)) {
      extensions_list->Append(CreateExtensionDetailValue(
          extension_service_,
          *extension,
          GetActivePagesForExtension(*extension),
          false, false));  // enabled, terminated
    }
  }
  extensions = extension_service_->terminated_extensions();
  std::vector<ExtensionPage> empty_pages;
  for (ExtensionList::const_iterator extension = extensions->begin();
       extension != extensions->end(); ++extension) {
    if (ShouldShowExtension(*extension)) {
      extensions_list->Append(CreateExtensionDetailValue(
          extension_service_,
          *extension,
          empty_pages,  // Terminated process has no active pages.
          false, true));  // enabled, terminated
    }
  }
  results.Set("extensions", extensions_list);

  bool developer_mode = Profile::FromWebUI(web_ui_)->GetPrefs()->
      GetBoolean(prefs::kExtensionsUIDeveloperMode);
  results.SetBoolean("developerMode", developer_mode);

  web_ui_->CallJavascriptFunction("returnExtensionsData", results);
}

void ExtensionsDOMHandler::RegisterForNotifications() {
  // Register for notifications that we need to reload the page.
  registrar_.Add(this, chrome::NOTIFICATION_EXTENSION_LOADED,
      NotificationService::AllSources());
  registrar_.Add(this, chrome::NOTIFICATION_EXTENSION_PROCESS_CREATED,
      NotificationService::AllSources());
  registrar_.Add(this, chrome::NOTIFICATION_EXTENSION_UNLOADED,
      NotificationService::AllSources());
  registrar_.Add(this, chrome::NOTIFICATION_EXTENSION_UPDATE_DISABLED,
      NotificationService::AllSources());
  registrar_.Add(this,
      content::NOTIFICATION_NAV_ENTRY_COMMITTED,
      NotificationService::AllSources());
  registrar_.Add(this,
      content::NOTIFICATION_RENDER_VIEW_HOST_CREATED,
      NotificationService::AllSources());
  registrar_.Add(this,
      content::NOTIFICATION_RENDER_VIEW_HOST_DELETED,
      NotificationService::AllSources());
  registrar_.Add(this,
      chrome::NOTIFICATION_BACKGROUND_CONTENTS_NAVIGATED,
      NotificationService::AllSources());
  registrar_.Add(this,
      chrome::NOTIFICATION_BACKGROUND_CONTENTS_DELETED,
      NotificationService::AllSources());
  registrar_.Add(this,
      chrome::NOTIFICATION_EXTENSION_BROWSER_ACTION_VISIBILITY_CHANGED,
      NotificationService::AllSources());
}

ExtensionUninstallDialog* ExtensionsDOMHandler::GetExtensionUninstallDialog() {
  if (!extension_uninstall_dialog_.get()) {
    extension_uninstall_dialog_.reset(
        new ExtensionUninstallDialog(Profile::FromWebUI(web_ui_)));
  }
  return extension_uninstall_dialog_.get();
}

void ExtensionsDOMHandler::HandleToggleDeveloperMode(const ListValue* args) {
  Profile* profile = Profile::FromWebUI(web_ui_);
  bool developer_mode =
      profile->GetPrefs()->GetBoolean(prefs::kExtensionsUIDeveloperMode);
  profile->GetPrefs()->SetBoolean(prefs::kExtensionsUIDeveloperMode,
                                  !developer_mode);
}

void ExtensionsDOMHandler::HandleInspectMessage(const ListValue* args) {
  std::string render_process_id_str;
  std::string render_view_id_str;
  int render_process_id;
  int render_view_id;
  CHECK(args->GetSize() == 2);
  CHECK(args->GetString(0, &render_process_id_str));
  CHECK(args->GetString(1, &render_view_id_str));
  CHECK(base::StringToInt(render_process_id_str, &render_process_id));
  CHECK(base::StringToInt(render_view_id_str, &render_view_id));
  RenderViewHost* host = RenderViewHost::FromID(render_process_id,
                                                render_view_id);
  if (!host) {
    // This can happen if the host has gone away since the page was displayed.
    return;
  }

  DevToolsWindow::OpenDevToolsWindow(host);
}

void ExtensionsDOMHandler::HandleReloadMessage(const ListValue* args) {
  std::string extension_id = UTF16ToASCII(ExtractStringValue(args));
  CHECK(!extension_id.empty());
  extension_service_->ReloadExtension(extension_id);
}

void ExtensionsDOMHandler::HandleEnableMessage(const ListValue* args) {
  CHECK(args->GetSize() == 2);
  std::string extension_id, enable_str;
  CHECK(args->GetString(0, &extension_id));
  CHECK(args->GetString(1, &enable_str));
  const Extension* extension =
      extension_service_->GetExtensionById(extension_id, true);
  DCHECK(extension);
  if (!Extension::UserMayDisable(extension->location())) {
    LOG(ERROR) << "Attempt to enable an extension that is non-usermanagable was"
               << "made. Extension id: " << extension->id();
    return;
  }

  if (enable_str == "true") {
    ExtensionPrefs* prefs = extension_service_->extension_prefs();
    if (prefs->DidExtensionEscalatePermissions(extension_id)) {
      ShowExtensionDisabledDialog(extension_service_,
                                  Profile::FromWebUI(web_ui_), extension);
    } else {
      extension_service_->EnableExtension(extension_id);
    }
  } else {
    extension_service_->DisableExtension(extension_id);
  }
}

void ExtensionsDOMHandler::HandleEnableIncognitoMessage(const ListValue* args) {
  CHECK(args->GetSize() == 2);
  std::string extension_id, enable_str;
  CHECK(args->GetString(0, &extension_id));
  CHECK(args->GetString(1, &enable_str));
  const Extension* extension =
      extension_service_->GetExtensionById(extension_id, true);
  DCHECK(extension);

  // Flipping the incognito bit will generate unload/load notifications for the
  // extension, but we don't want to reload the page, because a) we've already
  // updated the UI to reflect the change, and b) we want the yellow warning
  // text to stay until the user has left the page.
  //
  // TODO(aa): This creates crapiness in some cases. For example, in a main
  // window, when toggling this, the browser action will flicker because it gets
  // unloaded, then reloaded. It would be better to have a dedicated
  // notification for this case.
  //
  // Bug: http://crbug.com/41384
  ignore_notifications_ = true;
  extension_service_->SetIsIncognitoEnabled(extension_id,
                                            enable_str == "true");
  ignore_notifications_ = false;
}

void ExtensionsDOMHandler::HandleAllowFileAccessMessage(const ListValue* args) {
  CHECK(args->GetSize() == 2);
  std::string extension_id, allow_str;
  CHECK(args->GetString(0, &extension_id));
  CHECK(args->GetString(1, &allow_str));
  const Extension* extension =
      extension_service_->GetExtensionById(extension_id, true);
  DCHECK(extension);
  if (!Extension::UserMayDisable(extension->location())) {
    LOG(ERROR) << "Attempt to change allow file access of an extension that is "
               << "non-usermanagable was made. Extension id : "
               << extension->id();
    return;
  }

  extension_service_->SetAllowFileAccess(extension, allow_str == "true");
}

void ExtensionsDOMHandler::HandleUninstallMessage(const ListValue* args) {
  std::string extension_id = UTF16ToASCII(ExtractStringValue(args));
  CHECK(!extension_id.empty());
  const Extension* extension =
      extension_service_->GetExtensionById(extension_id, true);
  if (!extension)
    extension = extension_service_->GetTerminatedExtension(extension_id);
  if (!extension)
    return;

  if (!Extension::UserMayDisable(extension->location())) {
    LOG(ERROR) << "Attempt to uninstall an extension that is non-usermanagable "
               << "was made. Extension id : " << extension->id();
    return;
  }

  if (!extension_id_prompting_.empty())
    return;  // Only one prompt at a time.

  extension_id_prompting_ = extension_id;

  GetExtensionUninstallDialog()->ConfirmUninstall(this, extension);
}

void ExtensionsDOMHandler::ExtensionDialogAccepted() {
  DCHECK(!extension_id_prompting_.empty());

  bool was_terminated = false;

  // The extension can be uninstalled in another window while the UI was
  // showing. Do nothing in that case.
  const Extension* extension =
      extension_service_->GetExtensionById(extension_id_prompting_, true);
  if (!extension) {
    extension = extension_service_->GetTerminatedExtension(
        extension_id_prompting_);
    was_terminated = true;
  }
  if (!extension)
    return;

  extension_service_->UninstallExtension(extension_id_prompting_,
                                         false /* external_uninstall */, NULL);
  extension_id_prompting_ = "";

  // There will be no EXTENSION_UNLOADED notification for terminated
  // extensions as they were already unloaded.
  if (was_terminated)
    HandleRequestExtensionsData(NULL);
}

void ExtensionsDOMHandler::ExtensionDialogCanceled() {
  extension_id_prompting_ = "";
}

void ExtensionsDOMHandler::HandleOptionsMessage(const ListValue* args) {
  const Extension* extension = GetExtension(args);
  if (!extension || extension->options_url().is_empty())
    return;
  Profile::FromWebUI(web_ui_)->GetExtensionProcessManager()->OpenOptionsPage(
      extension, NULL);
}

void ExtensionsDOMHandler::HandleShowButtonMessage(const ListValue* args) {
  const Extension* extension = GetExtension(args);
  extension_service_->SetBrowserActionVisibility(extension, true);
}

void ExtensionsDOMHandler::HandleLoadMessage(const ListValue* args) {
  FilePath::StringType string_path;
  CHECK(args->GetSize() == 1) << args->GetSize();
  CHECK(args->GetString(0, &string_path));
  extension_service_->LoadExtension(FilePath(string_path));
}

void ExtensionsDOMHandler::ShowAlert(const std::string& message) {
  ListValue arguments;
  arguments.Append(Value::CreateStringValue(message));
  web_ui_->CallJavascriptFunction("alert", arguments);
}

void ExtensionsDOMHandler::HandlePackMessage(const ListValue* args) {
  std::string extension_path;
  std::string private_key_path;
  CHECK(args->GetSize() == 2);
  CHECK(args->GetString(0, &extension_path));
  CHECK(args->GetString(1, &private_key_path));

  FilePath root_directory =
      FilePath::FromWStringHack(UTF8ToWide(extension_path));
  FilePath key_file = FilePath::FromWStringHack(UTF8ToWide(private_key_path));

  if (root_directory.empty()) {
    if (extension_path.empty()) {
      ShowAlert(l10n_util::GetStringUTF8(
          IDS_EXTENSION_PACK_DIALOG_ERROR_ROOT_REQUIRED));
    } else {
      ShowAlert(l10n_util::GetStringUTF8(
          IDS_EXTENSION_PACK_DIALOG_ERROR_ROOT_INVALID));
    }

    return;
  }

  if (!private_key_path.empty() && key_file.empty()) {
    ShowAlert(l10n_util::GetStringUTF8(
        IDS_EXTENSION_PACK_DIALOG_ERROR_KEY_INVALID));
    return;
  }

  pack_job_ = new PackExtensionJob(this, root_directory, key_file);
  pack_job_->Start();
}

void ExtensionsDOMHandler::OnPackSuccess(const FilePath& crx_file,
                                         const FilePath& pem_file) {
  ShowAlert(UTF16ToUTF8(PackExtensionJob::StandardSuccessMessage(crx_file,
                                                                 pem_file)));

  ListValue results;
  web_ui_->CallJavascriptFunction("hidePackDialog", results);
}

void ExtensionsDOMHandler::OnPackFailure(const std::string& error) {
  ShowAlert(error);
}

void ExtensionsDOMHandler::HandleAutoUpdateMessage(const ListValue* args) {
  ExtensionUpdater* updater = extension_service_->updater();
  if (updater)
    updater->CheckNow();
}

void ExtensionsDOMHandler::HandleSelectFilePathMessage(const ListValue* args) {
  std::string select_type;
  std::string operation;
  CHECK(args->GetSize() == 2);
  CHECK(args->GetString(0, &select_type));
  CHECK(args->GetString(1, &operation));

  SelectFileDialog::Type type = SelectFileDialog::SELECT_FOLDER;
  SelectFileDialog::FileTypeInfo info;
  int file_type_index = 0;
  if (select_type == "file")
    type = SelectFileDialog::SELECT_OPEN_FILE;

  string16 select_title;
  if (operation == "load") {
    select_title = l10n_util::GetStringUTF16(IDS_EXTENSION_LOAD_FROM_DIRECTORY);
  } else if (operation == "packRoot") {
    select_title = l10n_util::GetStringUTF16(
        IDS_EXTENSION_PACK_DIALOG_SELECT_ROOT);
  } else if (operation == "pem") {
    select_title = l10n_util::GetStringUTF16(
        IDS_EXTENSION_PACK_DIALOG_SELECT_KEY);
    info.extensions.push_back(std::vector<FilePath::StringType>());
        info.extensions.front().push_back(FILE_PATH_LITERAL("pem"));
        info.extension_description_overrides.push_back(
            l10n_util::GetStringUTF16(
                IDS_EXTENSION_PACK_DIALOG_KEY_FILE_TYPE_DESCRIPTION));
        info.include_all_files = true;
    file_type_index = 1;
  } else {
    NOTREACHED();
    return;
  }

  load_extension_dialog_ = SelectFileDialog::Create(this);
  load_extension_dialog_->SelectFile(type, select_title, FilePath(), &info,
      file_type_index, FILE_PATH_LITERAL(""), web_ui_->tab_contents(),
      web_ui_->tab_contents()->view()->GetTopLevelNativeWindow(), NULL);
}


void ExtensionsDOMHandler::FileSelected(const FilePath& path, int index,
                                        void* params) {
  // Add the extensions to the results structure.
  ListValue results;
  results.Append(Value::CreateStringValue(path.value()));
  web_ui_->CallJavascriptFunction("window.handleFilePathSelected", results);
}

void ExtensionsDOMHandler::MultiFilesSelected(
    const std::vector<FilePath>& files, void* params) {
  NOTREACHED();
}

void ExtensionsDOMHandler::Observe(int type,
                                   const NotificationSource& source,
                                   const NotificationDetails& details) {
  switch (type) {
    // We listen for notifications that will result in the page being
    // repopulated with data twice for the same event in certain cases.
    // For instance, EXTENSION_LOADED & EXTENSION_PROCESS_CREATED because
    // we don't know about the views for an extension at EXTENSION_LOADED, but
    // if we only listen to EXTENSION_PROCESS_CREATED, we'll miss extensions
    // that don't have a process at startup. Similarly, NAV_ENTRY_COMMITTED &
    // RENDER_VIEW_HOST_CREATED because we want to handle both
    // the case of navigating from a non-extension page to an extension page in
    // a TabContents (which will generate NAV_ENTRY_COMMITTED) as well as
    // extension content being shown in popups and balloons (which will generate
    // RENDER_VIEW_HOST_CREATED but no NAV_ENTRY_COMMITTED).
    //
    // Doing it this way gets everything but causes the page to be rendered
    // more than we need. It doesn't seem to result in any noticeable flicker.
    case content::NOTIFICATION_RENDER_VIEW_HOST_DELETED:
      deleting_rvh_ = Source<RenderViewHost>(source).ptr();
      MaybeUpdateAfterNotification();
      break;
    case chrome::NOTIFICATION_BACKGROUND_CONTENTS_DELETED:
      deleting_rvh_ = Details<BackgroundContents>(details)->render_view_host();
      MaybeUpdateAfterNotification();
      break;
    case chrome::NOTIFICATION_EXTENSION_LOADED:
    case chrome::NOTIFICATION_EXTENSION_PROCESS_CREATED:
    case chrome::NOTIFICATION_EXTENSION_UNLOADED:
    case chrome::NOTIFICATION_EXTENSION_UPDATE_DISABLED:
    case content::NOTIFICATION_RENDER_VIEW_HOST_CREATED:
    case content::NOTIFICATION_NAV_ENTRY_COMMITTED:
    case chrome::NOTIFICATION_BACKGROUND_CONTENTS_NAVIGATED:
    case chrome::NOTIFICATION_EXTENSION_BROWSER_ACTION_VISIBILITY_CHANGED:
      MaybeUpdateAfterNotification();
      break;
    default:
      NOTREACHED();
  }
}

const Extension* ExtensionsDOMHandler::GetExtension(const ListValue* args) {
  std::string extension_id = UTF16ToASCII(ExtractStringValue(args));
  CHECK(!extension_id.empty());
  return extension_service_->GetExtensionById(extension_id, true);
}

void ExtensionsDOMHandler::MaybeUpdateAfterNotification() {
  if (!ignore_notifications_ &&
      web_ui_->tab_contents() &&
      web_ui_->tab_contents()->render_view_host()) {
    HandleRequestExtensionsData(NULL);
  }
  deleting_rvh_ = NULL;
}

// Static
DictionaryValue* ExtensionsDOMHandler::CreateExtensionDetailValue(
    ExtensionService* service, const Extension* extension,
    const std::vector<ExtensionPage>& pages, bool enabled, bool terminated) {
  DictionaryValue* extension_data = new DictionaryValue();
  GURL icon =
      ExtensionIconSource::GetIconURL(extension,
                                      Extension::EXTENSION_ICON_MEDIUM,
                                      ExtensionIconSet::MATCH_BIGGER,
                                      !enabled, NULL);
  extension_data->SetString("id", extension->id());
  extension_data->SetString("name", extension->name());
  extension_data->SetString("description", extension->description());
  if (extension->location() == Extension::LOAD)
    extension_data->SetString("path", extension->path().value());
  extension_data->SetString("version", extension->version()->GetString());
  extension_data->SetString("icon", icon.spec());
  extension_data->SetBoolean("isUnpacked",
                             extension->location() == Extension::LOAD);
  extension_data->SetBoolean("mayDisable",
                             Extension::UserMayDisable(extension->location()));
  extension_data->SetBoolean("enabled", enabled);
  extension_data->SetBoolean("terminated", terminated);
  extension_data->SetBoolean("enabledIncognito",
      service ? service->IsIncognitoEnabled(extension->id()) : false);
  extension_data->SetBoolean("wantsFileAccess", extension->wants_file_access());
  extension_data->SetBoolean("allowFileAccess",
      service ? service->AllowFileAccess(extension) : false);
  extension_data->SetBoolean("allow_reload",
                             extension->location() == Extension::LOAD);
  extension_data->SetBoolean("is_hosted_app", extension->is_hosted_app());

  // Determine the sort order: Extensions loaded through --load-extensions show
  // up at the top. Disabled extensions show up at the bottom.
  if (extension->location() == Extension::LOAD)
    extension_data->SetInteger("order", 1);
  else
    extension_data->SetInteger("order", 2);

  if (!extension->options_url().is_empty())
    extension_data->SetString("options_url", extension->options_url().spec());

  if (service && !service->GetBrowserActionVisibility(extension))
    extension_data->SetBoolean("enable_show_button", true);

  // Add views
  ListValue* views = new ListValue;
  for (std::vector<ExtensionPage>::const_iterator iter = pages.begin();
       iter != pages.end(); ++iter) {
    DictionaryValue* view_value = new DictionaryValue;
    if (iter->url.scheme() == chrome::kExtensionScheme) {
      // No leading slash.
      view_value->SetString("path", iter->url.path().substr(1));
    } else {
      // For live pages, use the full URL.
      view_value->SetString("path", iter->url.spec());
    }
    view_value->SetInteger("renderViewId", iter->render_view_id);
    view_value->SetInteger("renderProcessId", iter->render_process_id);
    view_value->SetBoolean("incognito", iter->incognito);
    views->Append(view_value);
  }
  extension_data->Set("views", views);
  extension_data->SetBoolean("hasPopupAction",
      extension->browser_action() || extension->page_action());
  extension_data->SetString("homepageUrl", extension->GetHomepageURL().spec());

  return extension_data;
}

std::vector<ExtensionPage> ExtensionsDOMHandler::GetActivePagesForExtension(
    const Extension* extension) {
  std::vector<ExtensionPage> result;

  // Get the extension process's active views.
  ExtensionProcessManager* process_manager =
      extension_service_->profile()->GetExtensionProcessManager();
  GetActivePagesForExtensionProcess(
      process_manager->GetExtensionProcess(extension->url()),
      extension, &result);

  // Repeat for the incognito process, if applicable.
  if (extension_service_->profile()->HasOffTheRecordProfile() &&
      extension->incognito_split_mode()) {
    ExtensionProcessManager* process_manager =
        extension_service_->profile()->GetOffTheRecordProfile()->
            GetExtensionProcessManager();
    GetActivePagesForExtensionProcess(
        process_manager->GetExtensionProcess(extension->url()),
        extension, &result);
  }

  return result;
}

void ExtensionsDOMHandler::GetActivePagesForExtensionProcess(
    RenderProcessHost* process,
    const Extension* extension,
    std::vector<ExtensionPage> *result) {
  if (!process)
    return;

  RenderProcessHost::listeners_iterator iter = process->ListenersIterator();
  for (; !iter.IsAtEnd(); iter.Advance()) {
    const RenderWidgetHost* widget =
        static_cast<const RenderWidgetHost*>(iter.GetCurrentValue());
    DCHECK(widget);
    if (!widget || !widget->IsRenderView())
      continue;
    const RenderViewHost* host = static_cast<const RenderViewHost*>(widget);
    if (host == deleting_rvh_ ||
        ViewType::EXTENSION_POPUP == host->delegate()->GetRenderViewType() ||
        ViewType::EXTENSION_DIALOG == host->delegate()->GetRenderViewType())
      continue;

    GURL url = host->delegate()->GetURL();
    if (url.SchemeIs(chrome::kExtensionScheme)) {
      if (url.host() != extension->id())
        continue;
    } else if (!extension->web_extent().MatchesURL(url)) {
      continue;
    }

    result->push_back(
        ExtensionPage(url, process->id(), host->routing_id(),
                      process->browser_context()->IsOffTheRecord()));
  }
}

ExtensionsDOMHandler::~ExtensionsDOMHandler() {
  // There may be pending file dialogs, we need to tell them that we've gone
  // away so they don't try and call back to us.
  if (load_extension_dialog_.get())
    load_extension_dialog_->ListenerDestroyed();

  if (pack_job_.get())
    pack_job_->ClearClient();

  registrar_.RemoveAll();
}

// ExtensionsDOMHandler, public: -----------------------------------------------

ExtensionsUI::ExtensionsUI(TabContents* contents) : ChromeWebUI(contents) {
  ExtensionService *extension_service =
      GetProfile()->GetOriginalProfile()->GetExtensionService();

  ExtensionsDOMHandler* handler = new ExtensionsDOMHandler(extension_service);
  AddMessageHandler(handler);
  handler->Attach(this);

  // Set up the chrome://extensions/ source.
  Profile* profile = Profile::FromBrowserContext(contents->browser_context());
  profile->GetChromeURLDataManager()->AddDataSource(
      CreateExtensionsUIHTMLSource());
}

// static
RefCountedMemory* ExtensionsUI::GetFaviconResourceBytes() {
  return ResourceBundle::GetSharedInstance().
      LoadDataResourceBytes(IDR_PLUGIN);
}

// static
void ExtensionsUI::RegisterUserPrefs(PrefService* prefs) {
  prefs->RegisterBooleanPref(prefs::kExtensionsUIDeveloperMode,
                             false,
                             PrefService::SYNCABLE_PREF);
}
