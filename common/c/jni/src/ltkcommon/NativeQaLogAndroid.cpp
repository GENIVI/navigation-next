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
   @file         NativeQaLogAndroid.cpp
*/
/*
   (C) Copyright 2014 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */

#include "NativeQaLogAndroid.h"
#include "nbcontextprotected.h"
#include <assert.h>
#include <jni.h>
#include <android/log.h>
#include "JNIUtility.h"
#include "QaLogTask.h"
#include "nativeltkcontext.h"

using namespace std;

#define  LOG_TAG    "NativeQaLog"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOG_LINE   __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s, %d", __func__, __LINE__)

struct fields_t
{
    jclass qalog_clazz;
    jfieldID qalog;
};

static fields_t fields;
static JavaVM* cachedVM = NULL;

static QaLogTask* GetQaLogTask(JNIEnv* env, jobject thiz)
{
    if (!env || !thiz || !fields.qalog)
    {
        return NULL;
    }
    return reinterpret_cast<QaLogTask*>(env->GetLongField(thiz, fields.qalog));
}

static void JNI_InitQaLog(JNIEnv* env, jobject thiz, jint ltkContext)
{
    NativeLTKContext* nativenbcontext = reinterpret_cast<NativeLTKContext*>(ltkContext);
    if (nativenbcontext)
    {
        QaLogTask* qalog = new QaLogTask(nativenbcontext->nbContext);

        env->SetLongField(thiz, fields.qalog, (long)qalog);
    }
}

static void JNI_WriteBuffer(JNIEnv* env, jobject thiz, jbyteArray data)
{
    QaLogTask* qalogTask = GetQaLogTask(env, thiz);
    if (qalogTask)
    {
        jbyte* logData = env->GetByteArrayElements(data, NULL);
        jsize logDataSize = env->GetArrayLength(data);

        qalogTask->WriteBuffer((const uint8*)logData, (uint32)logDataSize);

        if(logData)
        {
            env->ReleaseByteArrayElements(data, logData, JNI_ABORT);
        }
    }

}

static JNINativeMethod gMethods[] =
{
    { "createNativeQALog", "(I)V",                                    (void *)&JNI_InitQaLog},
    { "writeBuffer",       "([B)V",                                   (void *)&JNI_WriteBuffer},
};

int register_qalog(JavaVM* vm, JNIEnv* env)
{
    jint result = JNI_ERR;

    if (!env)
    {
        return result;
    }

    jclass clazz = env->FindClass("com/navbuilder/jni/NativeQALog");
    if (clazz != NULL)
    {
        fields.qalog_clazz = static_cast<jclass>(env->NewGlobalRef(clazz));
        if (fields.qalog_clazz == NULL)
        {
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("cannot't create global ref for com/navbuilder/jni/NativeQALog"));
            return result;
        }

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
    }
    else
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find com/navbuilder/jni/NativeQALog"));
        return result;
    }

    fields.qalog = env->GetFieldID(fields.qalog_clazz, "nativePtr", "J");

    if (env->RegisterNatives(fields.qalog_clazz, gMethods, sizeof(gMethods) / sizeof(gMethods[0])) != JNI_OK)
    {
        return result;
    }

    cachedVM = vm;
    result = JNI_VERSION_1_4;

    return result;
}

int unregister_qalog(JNIEnv* env)
{
    return JNI_VERSION_1_4;
}
