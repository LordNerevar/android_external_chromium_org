// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The routines in this file are windows-specific helper functions to deal
// with critical errors which the user can do something about.

#ifndef CHROME_APP_HARD_ERROR_HANDLER_WIN_H_
#define CHROME_APP_HARD_ERROR_HANDLER_WIN_H_
#pragma once

#include <windows.h>

// Two exceptions can be thrown when delay-laoding DLLs:
// - Failed to load a DLL. The common reason is because it cannot be found.
// - Failed to bind to an entry point. Typically because the DLL is too old.
// These exceptions cause the termination of the program, but it is desirable
// to first inform the user the name of the module or the name of the entry
// point so he or she can get help.
// This function does exactly that. When an exception is captured and passed
// to this function, it will display a message box with the relevant
// information and return true if the exception is generated by the delay-load
// feature or else do nothing and return false.
bool DelayLoadFailureExceptionMessageBox(EXCEPTION_POINTERS* ex_info);

// Generates a Popup dialog indicating that the entry point |entry| could
// not be found in dll |module|. The dialog is generated by CSRSS so this
// function can be called inside an exception handler.
void EntryPointNotFoundHardError(const char* entry, const char* module);

// Generates a Popup dialog indicating that the dll |module| could not be found.
// The dialog is generated by CSRSS so this function can be called inside an
// exception handler.
void ModuleNotFoundHardError(const char* module);

#endif  // CHROME_APP_HARD_ERROR_HANDLER_WIN_H_

