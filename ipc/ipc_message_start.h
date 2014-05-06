// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IPC_IPC_MESSAGE_START_H_
#define IPC_IPC_MESSAGE_START_H_

// Used by IPC_BEGIN_MESSAGES so that each message class starts from a unique
// base.  Messages have unique IDs across channels in order for the IPC logging
// code to figure out the message class from its ID.
enum IPCMessageStart {
  AutomationMsgStart = 0,
  FrameMsgStart,
  ViewMsgStart,
  InputMsgStart,
  PluginMsgStart,
  PluginProcessMsgStart,
  ProfileImportMsgStart,
  TestMsgStart,
  DevToolsMsgStart,
  WorkerMsgStart,
  NaClMsgStart,
  UtilityMsgStart,
  GpuMsgStart,
  ServiceMsgStart,
  PpapiMsgStart,
  FirefoxImporterUnittestMsgStart,
  FileUtilitiesMsgStart,
  MimeRegistryMsgStart,
  DatabaseMsgStart,
  DOMStorageMsgStart,
  IndexedDBMsgStart,
  PepperFileMsgStart,
  SpeechRecognitionMsgStart,
  PepperMsgStart,
  AutofillMsgStart,
  SafeBrowsingMsgStart,
  P2PMsgStart,
  SocketStreamMsgStart,
  ResourceMsgStart,
  FileSystemMsgStart,
  ChildProcessMsgStart,
  ClipboardMsgStart,
  BlobMsgStart,
  AppCacheMsgStart,
  DeviceMotionMsgStart,
  DeviceOrientationMsgStart,
  DesktopNotificationMsgStart,
  GeolocationMsgStart,
  AudioMsgStart,
  MidiMsgStart,
  ChromeMsgStart,
  DragMsgStart,
  PrintMsgStart,
  SpellCheckMsgStart,
  ExtensionMsgStart,
  VideoCaptureMsgStart,
  QuotaMsgStart,
  ImageMsgStart,
  TextInputClientMsgStart,
  ChromeUtilityMsgStart,
  MediaStreamMsgStart,
  ChromeBenchmarkingMsgStart,
  IntentsMsgStart,
  JavaBridgeMsgStart,
  GamepadMsgStart,
  ShellMsgStart,
  AccessibilityMsgStart,
  PrerenderMsgStart,
  ChromotingMsgStart,
  OldBrowserPluginMsgStart,
  BrowserPluginMsgStart,
  HyphenatorMsgStart,
  AndroidWebViewMsgStart,
  MetroViewerMsgStart,
  CCMsgStart,
  MediaPlayerMsgStart,
  TracingMsgStart,
  PeerConnectionTrackerMsgStart,
  VisitedLinkMsgStart,
  OneClickSigninMsgStart,
  AppShimMsgStart,
  ValidationMessageMsgStart,
  WebRtcLoggingMsgStart,
  TtsMsgStart,
  MemoryBenchmarkMsgStart,
  WebSocketMsgStart,
  NaClHostMsgStart,
  WebRTCIdentityMsgStart,
  LocalDiscoveryMsgStart,
  PowerMonitorMsgStart,
  EncryptedMediaMsgStart,
  ServiceWorkerMsgStart,
  MessagePortMsgStart,
  EmbeddedWorkerMsgStart,
  EmbeddedWorkerContextMsgStart,
  CastMsgStart,
  CdmMsgStart,
  ScreenOrientationMsgStart,
  MediaStreamTrackMetricsHostMsgStart,
  ChromeExtensionMsgStart,
  MojoMsgStart,
  TranslateMsgStart,
  PushMessagingMsgStart,
  GinJavaBridgeMsgStart,
  LastIPCMsgStart  // Must come last.
};

#endif  // IPC_IPC_MESSAGE_START_H_
