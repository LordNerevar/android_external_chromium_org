// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.app;

import android.content.Context;

/**
 * This class is used to initialize all types of process. It corresponds to
 * content/public/app/content_main.h which is not used in Android as it has
 * the different initialization process.
 *
 * TODO (michaelbai): Refactorying the BrowserProcessMain.java and the
 * SandboxedProcessService.java to start ContentMain, and run the process
 * specific initialization code in ContentMainRunner::Initialize.
 *
 **/
public class ContentMain {
    /**
     * Initialize application context in native side.
     **/
    public static void initApplicationContext(Context context) {
        nativeInitApplicationContext(context);
    }

    /**
     * Start the ContentMainRunner in native side.
     **/
    public static int start() {
        return nativeStart();
    }

    private static native void nativeInitApplicationContext(Context context);
    private static native int nativeStart();
};
