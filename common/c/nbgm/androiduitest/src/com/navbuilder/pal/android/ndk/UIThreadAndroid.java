/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

package com.navbuilder.pal.android.ndk;

import android.os.Handler;
import android.os.Looper;


public class UIThreadAndroid {
    private static final String TAG = "UIThreadAndroid";

    private int nativePtr;

    private Handler m_Handler;
    private Runnable m_ExecuteTaskRunnable;

    public UIThreadAndroid(int nativePtr) {
        this.nativePtr = nativePtr;

        m_ExecuteTaskRunnable = new Runnable() {
            @Override public void run() {
                nativeExecuteTasks(UIThreadAndroid.this.nativePtr);
            }
        };
    }

    /*
     * Called from native
     */
    public void startThread() {
        m_Handler = new Handler(Looper.getMainLooper());
    }

    /*
     * Called from native. This method is called from native object destructor.
     * So we don't need nativePtr and Handler anymore
     */
    public void stopThread() {
        nativePtr = 0;

        if (m_Handler == null) {
            return;
        }

        m_Handler.removeCallbacks(m_ExecuteTaskRunnable);
        m_Handler = null;
    }

    /*
     * Called from native
     */
    public void newTaskNotify() {

        if (m_Handler == null) {
            return;
        }

        m_Handler.post(m_ExecuteTaskRunnable);
    }

    /*
     * Called from native
     */
    public void cancelRunningTaskNotify() {
    }

    private native void nativeExecuteTasks(int p);
}
