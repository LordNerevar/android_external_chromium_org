// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function moduleDidLoad() {
}

function postThreadFunc(numThreads) {
  return function () {
    common.naclModule.postMessage('threads: ' + numThreads);
  }
}

// Add event listeners after the NaCl module has loaded.  These listeners will
// forward messages to the NaCl module via postMessage()
function attachListeners() {
  document.getElementById('benchmark').addEventListener('click',
    function() {
      common.naclModule.postMessage('run benchmark');
      common.updateStatus('BENCHMARKING... (please wait)');
    });
  document.getElementById('drawPoints').addEventListener('click',
    function() {
      var checked = document.getElementById('drawPoints').checked;
      if (checked)
        common.naclModule.postMessage('with points');
      else
        common.naclModule.postMessage('without points');
    });
  document.getElementById('drawInteriors').addEventListener('click',
    function() {
      var checked = document.getElementById('drawInteriors').checked;
      if (checked)
        common.naclModule.postMessage('with interiors');
      else
        common.naclModule.postMessage('without interiors');
    });
  var threads = [0, 1, 2, 4, 6, 8, 12, 16, 24, 32];
  for (var i = 0; i < threads.length; i++) {
    document.getElementById('radio'+i).addEventListener('click',
        postThreadFunc(threads[i]));
  }
  document.getElementById('pointRange').addEventListener('change',
    function() {
      var value = document.getElementById('pointRange').value;
      common.naclModule.postMessage('points: ' + value);
      document.getElementById('pointCount').textContent = value + ' points';
    });
}

// Handle a message coming from the NaCl module.
// In the Voronoi example, the only message will be the benchmark result.
function handleMessage(message_event) {
  var x = (Math.round(message_event.data * 1000) / 1000).toFixed(3);
  document.getElementById('result').textContent =
    'Result: ' + x + ' seconds';
  common.updateStatus('SUCCESS')
}

