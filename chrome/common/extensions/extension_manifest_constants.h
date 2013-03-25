// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_COMMON_EXTENSIONS_EXTENSION_MANIFEST_CONSTANTS_H_
#define CHROME_COMMON_EXTENSIONS_EXTENSION_MANIFEST_CONSTANTS_H_

#include <string>

#include "base/basictypes.h"
#include "googleurl/src/gurl.h"

// Keys used in JSON representation of extensions.
namespace extension_manifest_keys {
  extern const char kAllFrames[];
  extern const char kAltKey[];
  extern const char kApp[];
  extern const char kBackgroundAllowJsAccess[];
  extern const char kBackgroundPage[];
  extern const char kBackgroundPageLegacy[];
  extern const char kBackgroundPersistent[];
  extern const char kBackgroundScripts[];
  extern const char kBrowserAction[];
  extern const char kBrowseURLs[];
  extern const char kChromeURLOverrides[];
  extern const char kCommands[];
  extern const char kContentPack[];
  extern const char kContentPackSites[];
  extern const char kContentScripts[];
  extern const char kContentSecurityPolicy[];
  extern const char kConvertedFromUserScript[];
  extern const char kCss[];
  extern const char kCtrlKey[];
  extern const char kCurrentLocale[];
  extern const char kDefaultLocale[];
  extern const char kDescription[];
  extern const char kDevToolsPage[];
  extern const char kDisplayInLauncher[];
  extern const char kDisplayInNewTabPage[];
  extern const char kEventName[];
  extern const char kExcludeGlobs[];
  extern const char kExcludeMatches[];
  extern const char kFileAccessList[];
  extern const char kFileHandlers[];
  extern const char kFileHandlerTitle[];
  extern const char kFileHandlerTypes[];
  extern const char kFileFilters[];
  extern const char kFileBrowserHandlers[];
  extern const char kMediaGalleriesHandlers[];
  extern const char kHomepageURL[];
  extern const char kIcons[];
  extern const char kId[];
  extern const char kIncognito[];
  extern const char kIncludeGlobs[];
  extern const char kInputComponents[];
  extern const char kIntentDisposition[];
  extern const char kIntentHref[];
  extern const char kIntentPath[];
  extern const char kIntents[];
  extern const char kIntentTitle[];
  extern const char kIntentType[];
  extern const char kIsolation[];
  extern const char kJs[];
  extern const char kKey[];
  extern const char kKeycode[];
  extern const char kKioskEnabled[];
  extern const char kLanguage[];
  extern const char kLaunch[];
  extern const char kLaunchContainer[];
  extern const char kLaunchHeight[];
  extern const char kLaunchLocalPath[];
  extern const char kLaunchMaxHeight[];
  extern const char kLaunchMaxWidth[];
  extern const char kLaunchMinHeight[];
  extern const char kLaunchMinWidth[];
  extern const char kLaunchWebURL[];
  extern const char kLaunchWidth[];
  extern const char kLayouts[];
  extern const char kManifestVersion[];
  extern const char kMatches[];
  extern const char kMIMETypes[];
  extern const char kMimeTypesHandler[];
  extern const char kMinimumChromeVersion[];
  extern const char kNaClModules[];
  extern const char kNaClModulesMIMEType[];
  extern const char kNaClModulesPath[];
  extern const char kName[];
  extern const char kOAuth2[];
  extern const char kOAuth2ClientId[];
  extern const char kOAuth2Scopes[];
  extern const char kOfflineEnabled[];
  extern const char kOmnibox[];
  extern const char kOmniboxKeyword[];
  extern const char kOptionalPermissions[];
  extern const char kOptionsPage[];
  extern const char kPageAction[];
  extern const char kPageActionDefaultIcon[];
  extern const char kPageActionDefaultPopup[];
  extern const char kPageActionDefaultTitle[];
  extern const char kPageActionIcons[];
  extern const char kPageActionId[];
  extern const char kPageActionPopup[];
  extern const char kPageActionPopupPath[];
  extern const char kPageActions[];
  extern const char kPageLauncher[];
  extern const char kPermissions[];
  extern const char kPlatformAppBackground[];
  extern const char kPlatformAppBackgroundPage[];
  extern const char kPlatformAppBackgroundScripts[];
  extern const char kPlatformAppContentSecurityPolicy[];
  extern const char kPlugins[];
  extern const char kPluginsPath[];
  extern const char kPluginsPublic[];
  extern const char kPublicKey[];
  extern const char kRequirements[];
  extern const char kRunAt[];
  extern const char kSandboxedPages[];
  extern const char kSandboxedPagesCSP[];
  extern const char kScriptBadge[];
  extern const char kShiftKey[];
  extern const char kShortcutKey[];
  extern const char kSignature[];
  extern const char kSuggestedKey[];
  extern const char kSystemIndicator[];
  extern const char kTheme[];
  extern const char kThemeColors[];
  extern const char kThemeDisplayProperties[];
  extern const char kThemeImages[];
  extern const char kThemeTints[];
  extern const char kTtsEngine[];
  extern const char kTtsGenderFemale[];
  extern const char kTtsGenderMale[];
  extern const char kTtsVoices[];
  extern const char kTtsVoicesEventTypeEnd[];
  extern const char kTtsVoicesEventTypeError[];
  extern const char kTtsVoicesEventTypeMarker[];
  extern const char kTtsVoicesEventTypeSentence[];
  extern const char kTtsVoicesEventTypeStart[];
  extern const char kTtsVoicesEventTypeWord[];
  extern const char kTtsVoicesEventTypes[];
  extern const char kTtsVoicesGender[];
  extern const char kTtsVoicesLang[];
  extern const char kTtsVoicesVoiceName[];
  extern const char kType[];
  extern const char kUpdateURL[];
  extern const char kVersion[];
  extern const char kWebAccessibleResources[];
  extern const char kWebURLs[];
}  // namespace extension_manifest_keys

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
  extern const char kCannotAccessPage[];
  extern const char kCannotChangeExtensionID[];
  extern const char kCannotClaimAllHostsInExtent[];
  extern const char kCannotClaimAllURLsInExtent[];
  extern const char kCannotScriptGallery[];
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
  extern const char kInvalidFileAccessList[];
  extern const char kInvalidFileAccessValue[];
  extern const char kInvalidFileBrowserHandler[];
  extern const char kInvalidFileBrowserHandlerMIMETypes[];
  extern const char kInvalidMediaGalleriesHandler[];
  extern const char kInvalidFileFiltersList[];
  extern const char kInvalidFileFilterValue[];
  extern const char kInvalidFileHandlers[];
  extern const char kInvalidFileHandlerTitle[];
  extern const char kInvalidFileHandlerType[];
  extern const char kInvalidFileHandlerTypeElement[];
  extern const char kInvalidGlob[];
  extern const char kInvalidGlobList[];
  extern const char kInvalidHomepageURL[];
  extern const char kInvalidIconPath[];
  extern const char kInvalidIcons[];
  extern const char kInvalidIncognitoBehavior[];
  extern const char kInvalidIncognitoModeForPlatformApp[];
  extern const char kInvalidInputComponents[];
  extern const char kInvalidInputComponentDescription[];
  extern const char kInvalidInputComponentLayoutName[];
  extern const char kInvalidInputComponentLayouts[];
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
  extern const char kInvalidPageLauncher[];
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
  extern const char kInvalidSignature[];
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
  extern const char kPlatformAppNeedsManifestVersion2[];
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
