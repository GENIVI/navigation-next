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

package com.navbuilder.pal.ndk;

import java.lang.ref.WeakReference;

import com.navbuilder.pal.ndk.radio.Radio;

import android.content.Context;

public class PAL {
	static {
		System.loadLibrary("nbpal");
	}

	private Context mApplicationContext;
	private Radio mRadio;

	public PAL(Context context) {
		if (context == null) {
			throw new IllegalArgumentException("context can't be null");
		}

		mApplicationContext = context.getApplicationContext();

		mRadio = new Radio(mApplicationContext);

		int res = nativeInit(new WeakReference<Radio>(mRadio));

		if (res < 0) {
			System.out.println("");
			throw new RuntimeException("Can't initialize native PAL");
		}
	}

	public void destroy () {
		if (mRadio != null) {
			mRadio.destroy();
			mRadio = null;
		}

		nativeFinalize();
	}

	public void finalize() {
		destroy();
	}

	public Radio getRadioProxy() {
		return mRadio;
	}

	private native int nativeInit(WeakReference<Radio> radioProxy);

	private native void nativeFinalize();

	private int mNativePtr;
}