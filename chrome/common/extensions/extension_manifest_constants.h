// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_COMMON_EXTENSIONS_EXTENSION_MANIFEST_CONSTANTS_H_
#define CHROME_COMMON_EXTENSIONS_EXTENSION_MANIFEST_CONSTANTS_H_

#include <string>

#include "base/basictypes.h"
// TODO(yoz): Update includers to include this directly.
#include "extensions/common/manifest_constants.h"
#include "url/gurl.h"

// Some values expected in manifests.
namespace extension_manifest_values {
  extern const char kBrowserActionCommandEvent[];
  extern const char kIncognitoSplit[];
  extern const char kIncognitoSpanning[];
  extern const char kIntentDispositionWindow[];
  extern const char kIntentDispositionInline[];
  extern const char kIsolatedStorage[];
  extern const char kKeybindingPlatformChromeOs[];
  extern const char kKeybindingPlatformDefault[];
  extern const char kKeybindingPlatformLinux[];
  extern const char kKeybindingPlatformMac[];
  extern const char kKeybindingPlatformWin[];
  extern const char kKeyAlt[];
  extern const char kKeyShift[];
  extern const char kKeyCommand[];
  extern const char kKeyCtrl[];
  extern const char kKeyComma[];
  extern const char kKeyDel[];
  extern const char kKeyDown[];
  extern const char kKeyHome[];
  extern const char kKeyEnd[];
  extern const char kKeyIns[];
  extern const char kKeyLeft[];
  extern const char kKeyMacCtrl[];
  extern const char kKeyPgDwn[];
  extern const char kKeyPgUp[];
  extern const char kKeyPeriod[];
  extern const char kKeyRight[];
  extern const char kKeySeparator[];
  extern const char kKeyTab[];
  extern const char kKeyUp[];
  extern const char kLaunchContainerPanel[];
  extern const char kLaunchContainerTab[];
  extern const char kLaunchContainerWindow[];
  extern const char kPageActionCommandEvent[];
  extern const char kPageActionTypePermanent[];
  extern const char kPageActionTypeTab[];
  extern const char kScriptBadgeCommandEvent[];
  extern const char kRunAtDocumentEnd[];
  extern const char kRunAtDocumentIdle[];
  extern const char kRunAtDocumentStart[];
}  // namespace extension_manifest_values

// Error messages returned from Extension::InitFromValue().
namespace extension_manifest_errors {
  extern const char kAppsNotEnabled[];
  extern const char kBackgroundPermissionNeeded[];
  extern const char kBackgroundRequiredForPlatformApps[];
  extern const char kCannotAccessChromeUrl[];
  extern const char kCannotAccessExtensionUrl[];
  extern const char kCannotAccessPage[];
  extern const char kCannotChangeExtensionID[];
  extern const char kCannotClaimAllHostsInExtent[];
  extern const char kCannotClaimAllURLsInExtent[];
  extern const char kCannotScriptGallery[];
  extern const char kCannotScriptSigninPage[];
  extern const char kCannotUninstallManagedExtension[];
  extern const char kChromeVersionTooLow[];
  extern const char kDevToolsExperimental[];
  extern const char kDisabledByPolicy[];
  extern const char kExperimentalFlagRequired[];
  extern const char kExpectString[];
  extern const char kInvalidAllFrames[];
  extern const char kInvalidBackground[];
  extern const char kInvalidBackgroundAllowJsAccess[];
  extern const char kInvalidBackgroundCombination[];
  extern const char kInvalidBackgroundScript[];
  extern const char kInvalidBackgroundScripts[];
  extern const char kInvalidBackgroundInHostedApp[];
  extern const char kInvalidBackgroundPersistent[];
  extern const char kInvalidBackgroundPersistentNoPage[];
  extern const char kInvalidBrowserAction[];
  extern const char kInvalidBrowseURL[];
  extern const char kInvalidBrowseURLs[];
  extern const char kInvalidChromeURLOverrides[];
  extern const char kInvalidCommandsKey[];
  extern const char kInvalidContentPack[];
  extern const char kInvalidContentPackSites[];
  extern const char kInvalidContentScript[];
  extern const char kInvalidContentScriptsList[];
  extern const char kInvalidContentSecurityPolicy[];
  extern const char kInvalidCss[];
  extern const char kInvalidCssList[];
  extern const char kInvalidDefaultLocale[];
  extern const char kInvalidDescription[];
  extern const char kInvalidDevToolsPage[];
  extern const char kInvalidDisplayInLauncher[];
  extern const char kInvalidDisplayInNewTabPage[];
  extern const char kInvalidExcludeMatch[];
  extern const char kInvalidExcludeMatches[];
  extern const char kInvalidExport[];
  extern const char kInvalidExportPermissions[];
  extern const char kInvalidExportResources[];
  extern const char kInvalidExportResourcesString[];
  extern const char kInvalidFileAccessList[];
  extern const char kInvalidFileAccessValue[];
  extern const char kInvalidFileBrowserHandler[];
  extern const char kInvalidFileBrowserHandlerMIMETypes[];
  extern const char kInvalidMediaGalleriesHandler[];
  extern const char kInvalidFileFiltersList[];
  extern const char kInvalidFileFilterValue[];
  extern const char kInvalidFileHandlers[];
  extern const char kInvalidFileHandlerExtension[];
  extern const char kInvalidFileHandlerExtensionElement[];
  extern const char kInvalidFileHandlerNoTypeOrExtension[];
  extern const char kInvalidFileHandlerTitle[];
  extern const char kInvalidFileHandlerType[];
  extern const char kInvalidFileHandlerTypeElement[];
  extern const char kInvalidGlob[];
  extern const char kInvalidGlobList[];
  extern const char kInvalidHomepageURL[];
  extern const char kInvalidIconPath[];
  extern const char kInvalidIcons[];
  extern const char kInvalidImport[];
  extern const char kInvalidImportAndExport[];
  extern const char kInvalidImportId[];
  extern const char kInvalidImportVersion[];
  extern const char kInvalidIncognitoBehavior[];
  extern const char kInvalidInputComponents[];
  extern const char kInvalidInputComponentDescription[];
  extern const char kInvalidInputComponentLayoutName[];
  extern const char kInvalidInputComponentName[];
  extern const char kInvalidInputComponentShortcutKey[];
  extern const char kInvalidInputComponentShortcutKeycode[];
  extern const char kInvalidInputComponentType[];
  extern const char kInvalidIntent[];
  extern const char kInvalidIntentDisposition[];
  extern const char kInvalidIntentDispositionInPlatformApp[];
  extern const char kInvalidIntentHref[];
  extern const char kInvalidIntentHrefEmpty[];
  extern const char kInvalidIntentHrefInPlatformApp[];
  extern const char kInvalidIntentHrefOldAndNewKey[];
  extern const char kInvalidIntentPageInHostedApp[];
  extern const char kInvalidIntents[];
  extern const char kInvalidIntentType[];
  extern const char kInvalidIntentTypeElement[];
  extern const char kInvalidIntentTitle[];
  extern const char kInvalidIsolation[];
  extern const char kInvalidIsolationValue[];
  extern const char kInvalidJs[];
  extern const char kInvalidJsList[];
  extern const char kInvalidKey[];
  extern const char kInvalidKeyBinding[];
  extern const char kInvalidKeyBindingDescription[];
  extern const char kInvalidKeyBindingDictionary[];
  extern const char kInvalidKeyBindingMissingPlatform[];
  extern const char kInvalidKeyBindingTooMany[];
  extern const char kInvalidKeyBindingUnknownPlatform[];
  extern const char kInvalidKioskEnabled[];
  extern const char kInvalidLaunchContainer[];
  extern const char kInvalidLaunchValue[];
  extern const char kInvalidLaunchValueContainer[];
  extern const char kInvalidManifest[];
  extern const char kInvalidManifestVersion[];
  extern const char kInvalidManifestVersionOld[];
  extern const char kInvalidMatch[];
  extern const char kInvalidMatchCount[];
  extern const char kInvalidMatches[];
  extern const char kInvalidMIMETypes[];
  extern const char kInvalidMimeTypesHandler[];
  extern const char kInvalidMinimumChromeVersion[];
  extern const char kInvalidNaClModules[];
  extern const char kInvalidNaClModulesMIMEType[];
  extern const char kInvalidNaClModulesPath[];
  extern const char kInvalidName[];
  extern const char kInvalidOAuth2AutoApprove[];
  extern const char kInvalidOAuth2ClientId[];
  extern const char kInvalidOAuth2Scopes[];
  extern const char kInvalidOfflineEnabled[];
  extern const char kInvalidOmniboxKeyword[];
  extern const char kInvalidOptionsPage[];
  extern const char kInvalidOptionsPageExpectUrlInPackage[];
  extern const char kInvalidOptionsPageInHostedApp[];
  extern const char kInvalidPageAction[];
  extern const char kInvalidPageActionDefaultTitle[];
  extern const char kInvalidPageActionIconPath[];
  extern const char kInvalidPageActionId[];
  extern const char kInvalidPageActionName[];
  extern const char kInvalidPageActionOldAndNewKeys[];
  extern const char kInvalidPageActionPopup[];
  extern const char kInvalidPageActionPopupHeight[];
  extern const char kInvalidPageActionPopupPath[];
  extern const char kInvalidPageActionsList[];
  extern const char kInvalidPageActionsListSize[];
  extern const char kInvalidPageActionTypeValue[];
  extern const char kInvalidPermission[];
  extern const char kInvalidPermissions[];
  extern const char kInvalidPermissionScheme[];
  extern const char kInvalidPlugins[];
  extern const char kInvalidPluginsPath[];
  extern const char kInvalidPluginsPublic[];
  extern const char kInvalidRequirement[];
  extern const char kInvalidRequirements[];
  extern const char kInvalidRunAt[];
  extern const char kInvalidSandboxedPagesList[];
  extern const char kInvalidSandboxedPage[];
  extern const char kInvalidSandboxedPagesCSP[];
  extern const char kInvalidScriptBadge[];
  extern const char kInvalidShortName[];
  extern const char kInvalidSignature[];
  extern const char kInvalidSpellcheck[];
  extern const char kInvalidSpellcheckDictionaryFormat[];
  extern const char kInvalidSpellcheckDictionaryLanguage[];
  extern const char kInvalidSpellcheckDictionaryLocale[];
  extern const char kInvalidSpellcheckDictionaryPath[];
  extern const char kInvalidSystemIndicator[];
  extern const char kInvalidTheme[];
  extern const char kInvalidThemeColors[];
  extern const char kInvalidThemeImages[];
  extern const char kInvalidThemeImagesMissing[];
  extern const char kInvalidThemeTints[];
  extern const char kInvalidTts[];
  extern const char kInvalidTtsVoices[];
  extern const char kInvalidTtsVoicesEventTypes[];
  extern const char kInvalidTtsVoicesGender[];
  extern const char kInvalidTtsVoicesLang[];
  extern const char kInvalidTtsVoicesVoiceName[];
  extern const char kInvalidUpdateURL[];
  extern const char kInvalidURLPatternError[];
  extern const char kInvalidVersion[];
  extern const char kInvalidWebAccessibleResourcesList[];
  extern const char kInvalidWebAccessibleResource[];
  extern const char kInvalidWebURL[];
  extern const char kInvalidWebURLs[];
  extern const char kInvalidZipHash[];
  extern const char kInsecureContentSecurityPolicy[];
  extern const char kLaunchPathAndExtentAreExclusive[];
  extern const char kLaunchPathAndURLAreExclusive[];
  extern const char kLaunchURLRequired[];
  extern const char kLocalesMessagesFileMissing[];
  extern const char kLocalesNoDefaultLocaleSpecified[];
  extern const char kLocalesNoDefaultMessages[];
  extern const char kLocalesNoValidLocaleNamesListed[];
  extern const char kLocalesTreeMissing[];
  extern const char kManifestParseError[];
  extern const char kManifestUnreadable[];
  extern const char kMissingFile[];
  extern const char kMultipleOverrides[];
  extern const char kNoPermissionForMIMETypes[];
  extern const char kNoWildCardsInPaths[];
  extern const char kPermissionMustBeOptional[];
  extern const char kPermissionNotAllowed[];
  extern const char kPermissionNotAllowedInManifest[];
  extern const char kOneUISurfaceOnly[];
  extern const char kReservedMessageFound[];
  extern const char kScriptBadgeRequiresFlag[];
  extern const char kScriptBadgeIconIgnored[];
  extern const char kScriptBadgeTitleIgnored[];
  extern const char kWebRequestConflictsWithLazyBackground[];
#if defined(OS_CHROMEOS)
  extern const char kIllegalPlugins[];
#endif
}  // namespace extension_manifest_errors

#endif  // CHROME_COMMON_EXTENSIONS_EXTENSION_MANIFEST_CONSTANTS_H_
