// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function emptyCursorSuccess()
{
  debug('Empty cursor opened successfully.')
  done();
}

function openEmptyCursor()
{
  debug('Opening an empty cursor.');
  keyRange = webkitIDBKeyRange.lowerBound('InexistentKey');
  request = objectStore.openCursor(keyRange);
  request.onsuccess = emptyCursorSuccess;
  request.onerror = unexpectedErrorCallback;
}

function cursorSuccess()
{
  var cursor = event.target.result;
  if (cursor === null) {
    debug('Cursor reached end of range.');
    openEmptyCursor();
    return;
  }

  debug('Cursor opened successfully.');
  shouldBe("event.target.result.direction", "0");
  shouldBe("event.target.result.key", "3.14");
  shouldBe("event.target.result.value", "'myValue'");

  cursor.continue();
}

function openCursor(objectStore)
{
  debug('Opening cursor');
  var keyRange = webkitIDBKeyRange.lowerBound(3.12);
  var request = objectStore.openCursor(keyRange);
  request.onsuccess = cursorSuccess;
  request.onerror = unexpectedErrorCallback;
}

function dataAddedSuccess()
{
  debug('Data added');
  openCursor(objectStore);
}

function populateObjectStore()
{
  debug('Populating object store');
  deleteAllObjectStores(db);
  window.objectStore = db.createObjectStore('test');
  var request = objectStore.add('myValue', 3.14);
  request.onsuccess = dataAddedSuccess;
  request.onerror = unexpectedErrorCallback;
}

function setVersion()
{
  debug('setVersion');
  window.db = event.target.result;
  var request = db.setVersion('new version');
  request.onsuccess = populateObjectStore;
  request.onerror = unexpectedErrorCallback;
}

function test()
{
  debug('Connecting to indexedDB');
  var request = webkitIndexedDB.open('name');
  request.onsuccess = setVersion;
  request.onerror = unexpectedErrorCallback;
}
