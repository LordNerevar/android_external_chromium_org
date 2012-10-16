// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var checkSrc = function(element, expectedValue) {
  // Note that element.getAttribute('src') should not be used, it can be out of
  // sync with element.src.
  chrome.test.assertEq(expectedValue, element.src);
};

onload = function() {
  chrome.test.runTests([
    function browserTag() {
      var expectedSrcOne = 'data:text/html,<body>One</body>';
      var expectedSrcTwo = 'data:text/html,<body>Two</body>';
      var expectedSrcThree = 'data:text/html,<body>Three</body>';

      // For setting src, we check if both browserTag.setAttribute('src', ?);
      // and browserTag.src = ?; works propertly.
      var browserTag = document.querySelector('browser');

      // Check if initial src is set correctly.
      checkSrc(browserTag, expectedSrcOne);

      // Change the src.
      browserTag.setAttribute('src', expectedSrcTwo);

      // Timeout is necessary to give the mutation observers a chance to fire.
      setTimeout(function() {
        // Expect the src change to be reflected.
        checkSrc(browserTag, expectedSrcTwo);
        // Set src attribute directly on the element.
        browserTag.src = expectedSrcThree;
        setTimeout(function() {
          // Expect the src change to be reflected.
          checkSrc(browserTag, expectedSrcThree);
          // Set empty src, this will be ignored.
          browserTag.setAttribute('src', '');

          setTimeout(function() {
            // Expect empty src to be ignored.
            checkSrc(browserTag, expectedSrcThree);
            // Set empty src again, directly changing the src attribute.
            browserTag.src = '';

            setTimeout(function() {
              // Expect empty src to be ignored.
              checkSrc(browserTag, expectedSrcThree);
              chrome.test.succeed();
            }, 0);
          }, 0);
        }, 0);
      }, 0);
    }
  ]);
};
