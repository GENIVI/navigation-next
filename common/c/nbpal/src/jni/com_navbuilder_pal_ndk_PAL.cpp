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

    @file     com_navbuilder_pal_ndk_PAL.cpp
    @date     3/16/2012

    @brief    Entry point for Java com.navbuilder.pal.ndk.PAL class.
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

extern "C" {
#include "palconfig.h"
#include "pal.h"
#include "paldebuglog.h"
#include "jniconfig.h"
}

struct fields_t {
    jfieldID context;
};
static fields_t fields;

static jint com_navbuilder_pal_ndk_PAL_nativeInit(JNIEnv* env, jobject thiz,
        jobject jniRadioProxy) {
	DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_DEBUG, ("Init native PAL"));

    JNI_Config palProxyConfig = { env, jniRadioProxy };
    PAL_Config palConfig = { 1, &palProxyConfig };

    PAL_Instance* pal = PAL_Create(&palConfig);
    if (!pal) {
        DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_CRITICAL, ("Can't create native PAL"));
        return -1;
    }

    env->SetIntField(thiz, fields.context, (int) pal);

    return 0;
}

static void com_navbuilder_pal_ndk_PAL_nativeFinalize(JNIEnv* env, jobject thiz) {
    PAL_Instance* const p = (PAL_Instance*) env->GetIntField(thiz,
            fields.context);
    if (p != 0) {
        PAL_Destroy(p);

        env->SetIntField(thiz, fields.context, 0);
    }
}

static JNINativeMethod gMethods[] = {
	{ "nativeInit",     "(Ljava/lang/ref/WeakReference;)I",   (void *) &com_navbuilder_pal_ndk_PAL_nativeInit },
	{ "nativeFinalize", "()V",                     (void *) &com_navbuilder_pal_ndk_PAL_nativeFinalize },
};

extern int register_RadioProxy(JNIEnv *env);
extern int register_com_navbuilder_pal_ndk_radio_GSMInfo(JNIEnv *env);

static int register_com_navbuilder_nb_pal_android_ndk_JavaPAL(JNIEnv *env) {
    jclass clazz;

    clazz = env->FindClass("com/navbuilder/pal/ndk/PAL");
    if (clazz == NULL) {
        DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_CRITICAL, ("Can't find com/navbuilder/pal/ndk/PAL"));
        return JNI_ERR;
    }

    fields.context = env->GetFieldID(clazz, "mNativePtr", "I");
    if (fields.context == NULL) {
        DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_CRITICAL, ("Can't find JavaPAL.mNativePtr"));
        return JNI_ERR;
    }

    return env->RegisterNatives(clazz, gMethods, sizeof(gMethods) / sizeof(gMethods[0]));
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_CRITICAL, ("ERROR: GetEnv failed\n"));
        goto bail;
    }
    //assert(env != NULL);

    if (register_com_navbuilder_nb_pal_android_ndk_JavaPAL(env) < 0) {
        DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_CRITICAL, ("ERROR: JavaPAL native registration failed\n"));
        goto bail;
    }

    if (register_RadioProxy(env) < 0) {
        DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_CRITICAL, ("ERROR: Radio native registration failed\n"));
        goto bail;
    }

    if (register_com_navbuilder_pal_ndk_radio_GSMInfo(env) < 0) {
        DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_CRITICAL, ("ERROR: Radio native registration failed\n"));
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

    bail: return result;
}
