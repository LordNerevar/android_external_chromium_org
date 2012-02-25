// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/bundle_installer.h"

#include <algorithm>
#include <string>
#include <vector>

#include "base/command_line.h"
#include "base/values.h"
#include "chrome/browser/extensions/crx_installer.h"
#include "chrome/browser/extensions/extension_install_dialog.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/extensions/extension.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/web_contents.h"
#include "grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"

using content::NavigationController;

namespace extensions {

namespace {

enum AutoApproveForTest {
  DO_NOT_SKIP = 0,
  PROCEED,
  ABORT
};

AutoApproveForTest g_auto_approve_for_test = DO_NOT_SKIP;

// Creates a dummy extension and sets the manifest's name to the item's
// localized name.
scoped_refptr<Extension> CreateDummyExtension(BundleInstaller::Item item,
                                              DictionaryValue* manifest) {
  // We require localized names so we can have nice error messages when we can't
  // parse an extension manifest.
  CHECK(!item.localized_name.empty());

  manifest->SetString(extension_manifest_keys::kName, item.localized_name);

  std::string error;
  return Extension::Create(FilePath(),
                           Extension::INTERNAL,
                           *manifest,
                           Extension::NO_FLAGS,
                           item.id,
                           &error);
}

bool IsAppPredicate(scoped_refptr<const Extension> extension) {
  return extension->is_app();
}

struct MatchIdFunctor {
  explicit MatchIdFunctor(const std::string& id) : id(id) {}
  bool operator()(scoped_refptr<const Extension> extension) {
    return extension->id() == id;
  }
  std::string id;
};

// Holds the message IDs for BundleInstaller::GetHeadingTextFor.
const int kHeadingIds[3][4] = {
  {
    0,
    IDS_EXTENSION_BUNDLE_INSTALL_PROMPT_HEADING_EXTENSIONS,
    IDS_EXTENSION_BUNDLE_INSTALL_PROMPT_HEADING_APPS,
    IDS_EXTENSION_BUNDLE_INSTALL_PROMPT_HEADING_EXTENSION_APPS
  },
  {
    0,
    IDS_EXTENSION_BUNDLE_INSTALLED_HEADING_EXTENSIONS,
    IDS_EXTENSION_BUNDLE_INSTALLED_HEADING_APPS,
    IDS_EXTENSION_BUNDLE_INSTALLED_HEADING_EXTENSION_APPS
  },
  { IDS_EXTENSION_BUNDLE_ERROR_HEADING, 0, 0, 0 }
};

}  // namespace

// static
void BundleInstaller::SetAutoApproveForTesting(bool auto_approve) {
  CHECK(CommandLine::ForCurrentProcess()->HasSwitch(switches::kTestType));
  g_auto_approve_for_test = auto_approve ? PROCEED : ABORT;
}

BundleInstaller::Item::Item() : state(STATE_PENDING) {}

BundleInstaller::BundleInstaller(Profile* profile,
                                 const BundleInstaller::ItemList& items)
    : approved_(false),
      browser_(NULL),
      profile_(profile),
      delegate_(NULL) {
  BrowserList::AddObserver(this);
  for (size_t i = 0; i < items.size(); ++i) {
    items_[items[i].id] = items[i];
    items_[items[i].id].state = Item::STATE_PENDING;
  }
}

BundleInstaller::~BundleInstaller() {
  BrowserList::RemoveObserver(this);
}

BundleInstaller::ItemList BundleInstaller::GetItemsWithState(
    Item::State state) const {
  ItemList list;

  for (ItemMap::const_iterator i = items_.begin(); i != items_.end(); ++i) {
    if (i->second.state == state)
      list.push_back(i->second);
  }

  return list;
}

void BundleInstaller::PromptForApproval(Delegate* delegate) {
  delegate_ = delegate;

  AddRef();  // Balanced in ReportApproved() and ReportCanceled().

  ParseManifests();
}

void BundleInstaller::CompleteInstall(NavigationController* controller,
                                      Browser* browser,
                                      Delegate* delegate) {
  CHECK(approved_);

  browser_ = browser;
  delegate_ = delegate;

  AddRef();  // Balanced in ReportComplete();

  if (GetItemsWithState(Item::STATE_PENDING).empty()) {
    ReportComplete();
    return;
  }

  // Start each WebstoreInstaller.
  for (ItemMap::iterator i = items_.begin(); i != items_.end(); ++i) {
    if (i->second.state != Item::STATE_PENDING)
      continue;

    scoped_refptr<WebstoreInstaller> installer = new WebstoreInstaller(
        profile_,
        this,
        controller,
        i->first,
        WebstoreInstaller::FLAG_NONE);
    installer->Start();
  }
}

string16 BundleInstaller::GetHeadingTextFor(Item::State state) const {
  size_t total = 0;
  size_t apps = 0;

  // For STATE_FAILED, we can't tell if the items were apps or extensions
  // so we always show the same message.
  if (state == Item::STATE_INSTALLED || state == Item::STATE_PENDING) {
    total = GetItemsWithState(state).size();
    apps = std::count_if(
        dummy_extensions_.begin(), dummy_extensions_.end(), &IsAppPredicate);
  }

  bool has_apps = apps > 0;
  bool has_extensions = apps < total;
  size_t index = (has_extensions << 0) + (has_apps << 1);

  CHECK_LT(static_cast<size_t>(state), arraysize(kHeadingIds));
  CHECK_LT(index, arraysize(kHeadingIds[state]));

  int msg_id = kHeadingIds[state][index];
  if (!msg_id)
    return string16();

  return l10n_util::GetStringUTF16(msg_id);
}

#if !defined(TOOLKIT_GTK)
// static
void BundleInstaller::ShowInstalledBubble(
    const BundleInstaller* bundle, Browser* browser) {
  // TODO(jstritar): provide platform specific implementations.
}
#endif

void BundleInstaller::ParseManifests() {
  if (items_.empty()) {
    ReportCanceled(false);
    return;
  }

  for (ItemMap::iterator i = items_.begin(); i != items_.end(); ++i) {
    scoped_refptr<WebstoreInstallHelper> helper = new WebstoreInstallHelper(
        this, i->first, i->second.manifest, "", GURL(), NULL);
    helper->Start();
  }
}

void BundleInstaller::ReportApproved() {
  if (delegate_)
    delegate_->OnBundleInstallApproved();

  Release();  // Balanced in PromptForApproval().
}

void BundleInstaller::ReportCanceled(bool user_initiated) {
  if (delegate_)
    delegate_->OnBundleInstallCanceled(user_initiated);

  Release();  // Balanced in PromptForApproval().
}

void BundleInstaller::ReportComplete() {
  if (delegate_)
    delegate_->OnBundleInstallCompleted();

  Release();  // Balanced in CompleteInstall().
}

void BundleInstaller::ShowPromptIfDoneParsing() {
  // We don't prompt until all the manifests have been parsed.
  ItemList pending_items = GetItemsWithState(Item::STATE_PENDING);
  if (pending_items.size() != dummy_extensions_.size())
    return;

  ShowPrompt();
}

void BundleInstaller::ShowPrompt() {
  // Abort if we couldn't create any Extensions out of the manifests.
  if (dummy_extensions_.empty()) {
    ReportCanceled(false);
    return;
  }

  scoped_refptr<ExtensionPermissionSet> permissions;
  for (size_t i = 0; i < dummy_extensions_.size(); ++i) {
    permissions = ExtensionPermissionSet::CreateUnion(
          permissions, dummy_extensions_[i]->required_permission_set());
  }

  if (g_auto_approve_for_test == PROCEED) {
    InstallUIProceed();
  } else if (g_auto_approve_for_test == ABORT) {
    InstallUIAbort(true);
  } else {
    ExtensionInstallUI::Prompt prompt(
        ExtensionInstallUI::BUNDLE_INSTALL_PROMPT);
    prompt.SetPermissions(permissions->GetWarningMessages());
    prompt.set_bundle(this);

    ShowExtensionInstallDialog(profile_, this, prompt);
  }
}

void BundleInstaller::ShowInstalledBubbleIfDone() {
  // We're ready to show the installed bubble when no items are pending.
  if (!GetItemsWithState(Item::STATE_PENDING).empty())
    return;

  if (browser_)
    ShowInstalledBubble(this, browser_);

  ReportComplete();
}

void BundleInstaller::OnWebstoreParseSuccess(
    const std::string& id,
    const SkBitmap& icon,
    DictionaryValue* manifest) {
  dummy_extensions_.push_back(CreateDummyExtension(items_[id], manifest));
  parsed_manifests_[id] = linked_ptr<DictionaryValue>(manifest);

  ShowPromptIfDoneParsing();
}

void BundleInstaller::OnWebstoreParseFailure(
    const std::string& id,
    WebstoreInstallHelper::Delegate::InstallHelperResultCode result_code,
    const std::string& error_message) {
  items_[id].state = Item::STATE_FAILED;

  ShowPromptIfDoneParsing();
}

void BundleInstaller::InstallUIProceed() {
  approved_ = true;
  for (ItemMap::iterator i = items_.begin(); i != items_.end(); ++i) {
    if (i->second.state != Item::STATE_PENDING)
      continue;

    // Create a whitelist entry for each of the approved extensions.
    CrxInstaller::WhitelistEntry* entry = new CrxInstaller::WhitelistEntry;
    entry->parsed_manifest.reset(parsed_manifests_[i->first]->DeepCopy());
    entry->localized_name = i->second.localized_name;
    entry->use_app_installed_bubble = false;
    entry->skip_post_install_ui = true;
    CrxInstaller::SetWhitelistEntry(i->first, entry);
  }
  ReportApproved();
}

void BundleInstaller::InstallUIAbort(bool user_initiated) {
  for (ItemMap::iterator i = items_.begin(); i != items_.end(); ++i)
    i->second.state = Item::STATE_FAILED;

  ReportCanceled(user_initiated);
}

void BundleInstaller::OnExtensionInstallSuccess(const std::string& id) {
  items_[id].state = Item::STATE_INSTALLED;

  ShowInstalledBubbleIfDone();
}

void BundleInstaller::OnExtensionInstallFailure(const std::string& id,
                                                const std::string& error) {
  items_[id].state = Item::STATE_FAILED;

  std::remove_if(dummy_extensions_.begin(), dummy_extensions_.end(),
                 MatchIdFunctor(id));

  ShowInstalledBubbleIfDone();
}

void BundleInstaller::OnBrowserAdded(const Browser* browser) {
}

void BundleInstaller::OnBrowserRemoved(const Browser* browser) {
  if (browser_ == browser)
    browser_ = NULL;
}

void BundleInstaller::OnBrowserSetLastActive(const Browser* browser) {
}

}  // namespace extensions
