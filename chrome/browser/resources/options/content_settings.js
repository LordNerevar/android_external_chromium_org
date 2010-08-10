// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

cr.define('options', function() {

  var OptionsPage = options.OptionsPage;

  //////////////////////////////////////////////////////////////////////////////
  // ContentSettings class:

  /**
   * Encapsulated handling of content settings page.
   * @constructor
   */
  function ContentSettings() {
    this.activeNavTab = null;
    OptionsPage.call(this, 'content', templateData.contentSettingsPage,
                     'contentSettingsPage');
  }

  cr.addSingletonGetter(ContentSettings);

  ContentSettings.prototype = {
    __proto__: OptionsPage.prototype,

    initializePage: function() {
      OptionsPage.prototype.initializePage.call(this);

      chrome.send('getContentFilterSettings');

      // Cookies filter page ---------------------------------------------------
      $('cookies-exceptions-button').onclick = function(event) {
        // TODO(estade): show exceptions page.
      };

      $('block-third-party-cookies').onclick = function(event) {
        chrome.send('setAllowThirdPartyCookies',
                    [String($('block-third-party-cookies').checked)]);
      };

      $('show-cookies-button').onclick = function(event) {
        // TODO(estade): show cookies and other site data page.
      };

      // Images filter page ----------------------------------------------------
      $('images-exceptions-button').onclick = function(event) {
        // TODO(estade): show a dialog.
        // TODO(estade): remove this hack.
        imagesExceptionsList.redraw();
      };

      var exceptionsAreas = document.querySelectorAll('div[contentType]');
      for (var i = 0; i < exceptionsAreas.length; i++) {
        options.contentSettings.ExceptionsArea.decorate(exceptionsAreas[i]);
      }
    },
  };

  /**
   * Sets the initial values for all the content settings radios.
   * @param {Object} dict A mapping from radio groups to the checked value for
   *     that group.
   */
  ContentSettings.setInitialContentFilterSettingsValue = function(dict) {
    for (var group in dict) {
      document.querySelector('input[type=radio][name=' + group +
                             '][value=' + dict[group] + ']').checked = true;
    }
  };

  /**
   * Initializes an exceptions list.
   * @param {string} type The content type that we are setting exceptions for.
   * @param {Array} list An array of pairs, where the first element of each pair
   *     is the filter string, and the second is the setting (allow/block).
   */
  ContentSettings.setExceptions = function(type, list) {
    var exceptionsList =
        document.querySelector('div[contentType=' + type + '] list');
    exceptionsList.clear();
    for (var i = 0; i < list.length; i++) {
      exceptionsList.addException(list[i]);
    }
  };

  /**
   * Sets the initial value for the Third Party Cookies checkbox.
   * @param {boolean=} block True if we are blocking third party cookies.
   */
  ContentSettings.setBlockThirdPartyCookies = function(block) {
    $('block-third-party-cookies').checked = block;
  };

  /**
   * The browser's response to a request to check the validity of a given URL
   * pattern.
   * @param {string} pattern The pattern.
   * @param {bool} valid Whether said pattern is valid in the context of
   *     a content exception setting.
   */
  ContentSettings.patternValidityCheckComplete =
      function(type, pattern, valid) {
    var exceptionsList =
        document.querySelector('div[contentType=' + type + '] list');
    exceptionsList.patternValidityCheckComplete(pattern, valid);
  };

  // Export
  return {
    ContentSettings: ContentSettings
  };

});

