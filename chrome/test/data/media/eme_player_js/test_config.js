// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Global test configuration used by media players.  On each test run call
// TestConfig.init() to load new test configuration from the test page.
var TestConfig = new function() {
  this.mediaFile = null;
  this.keySystem = null;
  this.mediaType = null;
  this.licenseServerURL = null;
  this.useSRC = false;
  this.usePrefixedEME = false;
  this.runFPS = false
}

TestConfig.loadQueryParams = function() {
  // Load query parameters and set default values.
  var r = /([^&=]+)=?([^&]*)/g;
  // Lambda function for decoding extracted match values. Replaces '+' with
  // space so decodeURIComponent functions properly.
  var decodeURI = function decodeURI(s) {
      return decodeURIComponent(s.replace(/\+/g, ' '));
  };
  var match;
  while (match = r.exec(window.location.search.substring(1)))
    this[decodeURI(match[1])] = decodeURI(match[2]);

  this.useSRC = this.useSRC == '1' || this.useSRC == 'true';
  this.usePrefixedEME =
      this.usePrefixedEME == '1' || this.usePrefixedEME == 'true';
};

TestConfig.updateDocument = function() {
  this.loadQueryParams();
  Utils.addOptions(KEYSYSTEM_ELEMENT_ID, KEY_SYSTEMS);
  Utils.addOptions(MEDIA_TYPE_ELEMENT_ID, MEDIA_TYPES);
  Utils.addOptions(USE_PREFIXED_EME_ID, EME_VERSIONS_OPTIONS,
                   EME_DISABLED_OPTIONS);

  document.getElementById(MEDIA_FILE_ELEMENT_ID).value =
      this.mediaFile || DEFAULT_MEDIA_FILE;

  document.getElementById(LICENSE_SERVER_ELEMENT_ID).value =
      this.licenseServerURL || DEFAULT_LICENSE_SERVER;

  if (this.keySystem)
    Utils.ensureOptionInList(KEYSYSTEM_ELEMENT_ID, this.keySystem);
  if (this.mediaType)
    Utils.ensureOptionInList(MEDIA_TYPE_ELEMENT_ID, this.mediaType);
  document.getElementById(USE_SRC_ELEMENT_ID).value = this.useSRC;
  if (this.usePrefixedEME)
    document.getElementById(USE_PREFIXED_EME_ID).value = EME_PREFIXED_VERSION;
};

TestConfig.init = function() {
  // Reload test configuration from document.
  this.mediaFile = document.getElementById(MEDIA_FILE_ELEMENT_ID).value;
  this.keySystem = document.getElementById(KEYSYSTEM_ELEMENT_ID).value;
  this.mediaType = document.getElementById(MEDIA_TYPE_ELEMENT_ID).value;
  this.useSRC = document.getElementById(USE_SRC_ELEMENT_ID).value == 'true';
  this.usePrefixedEME = document.getElementById(USE_PREFIXED_EME_ID).value ==
      EME_PREFIXED_VERSION;
  this.licenseServerURL =
      document.getElementById(LICENSE_SERVER_ELEMENT_ID).value;
};
