// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_SIGNIN_SCOPED_GAIA_AUTH_EXTENSION_H_
#define CHROME_BROWSER_EXTENSIONS_SIGNIN_SCOPED_GAIA_AUTH_EXTENSION_H_

#include "base/basictypes.h"

class Profile;

// ScopedGaiaAuthExtension loads gaia auth extension on construction if the
// extension is not registered yet, and unloads it on destruction if it is no
// longer used.
class ScopedGaiaAuthExtension {
 public:
  explicit ScopedGaiaAuthExtension(Profile* profile);
  ~ScopedGaiaAuthExtension();

 private:
  Profile* profile_;

  DISALLOW_COPY_AND_ASSIGN(ScopedGaiaAuthExtension);
};

#endif  // CHROME_BROWSER_EXTENSIONS_SIGNIN_SCOPED_GAIA_AUTH_EXTENSION_H_
