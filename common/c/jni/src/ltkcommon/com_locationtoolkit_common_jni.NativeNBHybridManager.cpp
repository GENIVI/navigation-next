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
 @file     NativeNBHybridManager.cpp
 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#include "nbhybridmanager.h"
#include "nativeltkcontext.h"
#include "JNIUtility.h"
#include <jni.h>
#include <android/log.h>

#define  LOG_TAG    "NativeNBHybridManager"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOG_LINE   __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s, %d", __func__, __LINE__)


extern "C" {
    JNIEXPORT jint JNICALL Java_com_locationtoolkit_common_jni_NativeNBHybridManager_EnableOnboard(JNIEnv* env, jobject thiz, jint context, jboolean enable);
    JNIEXPORT jboolean JNICALL Java_com_locationtoolkit_common_jni_NativeNBHybridManager_IsOnboardEnabled(JNIEnv* env, jobject thiz, jint context);
    JNIEXPORT jint JNICALL Java_com_locationtoolkit_common_jni_NativeNBHybridManager_SetLanguage(JNIEnv* env, jobject thiz, jint context, jstring lang);
    JNIEXPORT jint JNICALL Java_com_locationtoolkit_common_jni_NativeNBHybridManager_SetMapConfig(JNIEnv* env, jobject thiz, jint context, jstring mapConfig);
};

JNIEXPORT jint JNICALL Java_com_locationtoolkit_common_jni_NativeNBHybridManager_EnableOnboard(JNIEnv* env, jobject thiz, jint context, jboolean enable)
{
    NativeLTKContext* nativenbcontext = reinterpret_cast<NativeLTKContext*>(context);
    if(nativenbcontext)
    {
        return (jint)NB_HybridManagerEnableOnboard(nativenbcontext->nbContext, TRUE==enable);
    }

    return (jint)NE_INVAL;
}

JNIEXPORT jboolean JNICALL Java_com_locationtoolkit_common_jni_NativeNBHybridManager_IsOnboardEnabled(JNIEnv* env, jobject thiz, jint context)
{
    NativeLTKContext* nativenbcontext = reinterpret_cast<NativeLTKContext*>(context);
    if(nativenbcontext)
    {
        return (jboolean)NB_HybridManagerIsOnboardEnabled(nativenbcontext->nbContext);
    }

    return JNI_FALSE;
}

JNIEXPORT jint JNICALL Java_com_locationtoolkit_common_jni_NativeNBHybridManager_SetLanguage(JNIEnv* env, jobject thiz, jint context, jstring lang)
{
    NativeLTKContext* nativenbcontext = reinterpret_cast<NativeLTKContext*>(context);
    if(nativenbcontext)
    {
        std::string strLang = NewStdString(env, lang);
        return (jint)NB_HybridManagerSetLanguage(nativenbcontext->nbContext, strLang.c_str());
    }

    return (jint)NE_INVAL;
}

JNIEXPORT jint JNICALL Java_com_locationtoolkit_common_jni_NativeNBHybridManager_SetMapConfig(JNIEnv* env, jobject thiz, jint context, jstring mapConfig)
{
    NativeLTKContext* nativenbcontext = reinterpret_cast<NativeLTKContext*>(context);
    if(nativenbcontext)
    {
        std::string strMapConfig = NewStdString(env, mapConfig);
        LOGI("Map data config file:%s", strMapConfig.c_str());
        NB_HybridManagerSetMapConfig(nativenbcontext->nbContext, strMapConfig.c_str());
        return (jint)NE_OK;
    }

    return (jint)NE_INVAL;
}
