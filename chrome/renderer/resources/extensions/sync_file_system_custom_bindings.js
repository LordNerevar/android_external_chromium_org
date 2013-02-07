// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Custom bindings for the syncFileSystem API.

var chromeHidden = requireNative('chrome_hidden').GetChromeHidden();
var fileSystemNatives = requireNative('file_system_natives');
var syncFileSystemNatives = requireNative('sync_file_system');

chromeHidden.registerCustomHook('syncFileSystem', function(bindingsAPI) {
  var apiFunctions = bindingsAPI.apiFunctions;

  // Functions which take in an [instanceOf=FileEntry].
  function bindFileEntryFunction(functionName) {
    apiFunctions.setUpdateArgumentsPostValidate(
        functionName, function(entry, callback) {
      var fileSystemUrl = entry.toURL();
      return [fileSystemUrl, callback];
    });
  }
  ['getFileStatus']
      .forEach(bindFileEntryFunction);

  // Functions which take in an [instanceOf=DOMFileSystem].
  function bindFileSystemFunction(functionName) {
    apiFunctions.setUpdateArgumentsPostValidate(
        functionName, function(filesystem, callback) {
      var fileSystemUrl = filesystem.root.toURL();
      return [fileSystemUrl, callback];
    });
  }
  ['deleteFileSystem', 'getUsageAndQuota']
      .forEach(bindFileSystemFunction);

  // Functions which return an [instanceOf=DOMFileSystem].
  apiFunctions.setCustomCallback('requestFileSystem',
                                 function(name, request, response) {
    var result = null;
    if (response) {
      result = syncFileSystemNatives.GetSyncFileSystemObject(
          response.name, response.root);
    }
    if (request.callback)
      request.callback(result);
    request.callback = null;
  });
});

chromeHidden.Event.registerArgumentMassager(
    'syncFileSystem.onFileStatusChanged', function(args, dispatch) {
  // Make FileEntry object using all the base string fields.
  var fileSystemType = args[0];
  var fileSystemName = args[1];
  var rootUrl = args[2];
  var filePath = args[3];
  var fileEntry = fileSystemNatives.GetFileEntry(fileSystemType,
      fileSystemName, rootUrl, filePath, false);

  var syncOperationResult = args[4];
  dispatch([fileEntry, syncOperationResult]);
});
