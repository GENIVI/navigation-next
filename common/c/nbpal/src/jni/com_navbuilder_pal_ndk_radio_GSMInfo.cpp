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

/*!--------------------------------------------------------------------------

    @file     com_navbuilder_pal_ndk_radion_GSMInfo.cpp
    @date     3/16/2012

    @brief    Native implementation of com.navbuilder.pal.ndk.GSMInfo class.
*/
/*
    See file description in header file.

    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "jni.h"

#include "palradio.h"
#include "palstdlib.h"

extern "C" {
#include "paldebuglog.h"
}

struct fields_t {
    jfieldID context;
};
static fields_t fields;

static void com_navbuilder_pal_ndk_radio_GSMInfo_nativeInit(JNIEnv* env,
        jobject thiz, jlong mobileCountryCode, jlong mobileNetworkCode,
        jlong locationAreaCode, jlong cellId, jint signalStrength) {
    PAL_RadioGSMInfo gsmInfo;
    nsl_memset(&gsmInfo, 0, sizeof(PAL_RadioGSMInfo));

    gsmInfo.cellId = cellId;
    gsmInfo.locationAreaCode = locationAreaCode;
    gsmInfo.mobileCountryCode = mobileCountryCode;
    gsmInfo.mobileNetworkCode = mobileNetworkCode;
    gsmInfo.signalStrength = signalStrength;

    env->SetIntField(thiz, fields.context, (int) &gsmInfo);
}

/*
 PAL_RadioGSMInfo* GSMInfoProxy_getGSMInfo(JNIEnv* env, jobject jniObject)
 {
 if (jniObject != NULL)
 {
 PAL_RadioGSMInfo* const p = (PAL_RadioGSMInfo*)env->GetIntField(jniObject, fields.context);
 return p;
 }
 else
 {
 return NULL;
 }
 }
 */

static JNINativeMethod gMethods[] = {
	{ "nativeInit", "(JJJJI)V", (void *) &com_navbuilder_pal_ndk_radio_GSMInfo_nativeInit },
};

int register_com_navbuilder_pal_ndk_radio_GSMInfo(JNIEnv *env) {
    jclass clazz;

    clazz = env->FindClass("com/navbuilder/pal/ndk/radio/GSMInfo");
    if (clazz == NULL) {
        DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_CRITICAL, ("Can't find com/navbuilder/pal/ndk/radio/GSMInfo"));
        return JNI_ERR;
    }

    fields.context = env->GetFieldID(clazz, "mNativePtr", "I");
    if (fields.context == NULL) {
        DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_CRITICAL, ("Can't find GSMInfo.mNativePtr"));
        return JNI_ERR;
    }

    return env->RegisterNatives(clazz, gMethods, sizeof(gMethods) / sizeof(gMethods[0]));
}
