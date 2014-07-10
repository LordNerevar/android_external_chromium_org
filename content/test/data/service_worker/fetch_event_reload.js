// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

this.onfetch = function(event) {
    var headers = new Headers;
    headers.set('Content-Type', 'text/html; charset=UTF-8');
    var blob = new Blob(['<title>reload='+ event.isReload + '</title>']);
    var response = new Response(blob);
    event.respondWith(response);
};
