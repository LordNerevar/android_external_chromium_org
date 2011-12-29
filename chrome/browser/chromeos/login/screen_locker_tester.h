// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_SCREEN_LOCKER_TESTER_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_SCREEN_LOCKER_TESTER_H_
#pragma once

#include <string>

#include "base/basictypes.h"

namespace views {
class Widget;
}  // namespace views

namespace chromeos {

namespace test {

// ScreenLockerTester provides access to the private state/function
// of ScreenLocker class. Used to implement unit tests.
class ScreenLockerTester {
 public:
  ScreenLockerTester();
  virtual ~ScreenLockerTester();

  // Returns true if the screen is locked.
  virtual bool IsLocked();

  // Injects MockAuthenticate that uses given |user| and |password|.
  virtual void InjectMockAuthenticator(const std::string& user,
                                       const std::string& password);

  // Sets the password text.
  virtual void SetPassword(const std::string& password) = 0;

  // Gets the password text.
  virtual std::string GetPassword() = 0;

  // Emulates entring a password.
  virtual void EnterPassword(const std::string& password) = 0;

  // Emulates the ready message from window manager.
  virtual void EmulateWindowManagerReady() = 0;

  // Returns the widget for screen locker window.
  virtual views::Widget* GetWidget() const = 0;

  virtual views::Widget* GetChildWidget() const = 0;
};

}  // namespace test

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_SCREEN_LOCKER_TESTER_H_
