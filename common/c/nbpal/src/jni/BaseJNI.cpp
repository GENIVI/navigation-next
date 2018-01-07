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

    @file     BaseJNI.cpp
    @date     3/16/2012
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

#include "BaseJNI.h"

extern "C" {
#include "paldebuglog.h"
}

BaseJNI::BaseJNI(JNIEnv* env, jobject jniObject) {
    jint err = env->GetJavaVM(&(this->m_javaVM));
    if (err < 0) {
        DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_CRITICAL, ("Can't retrieve java VM ref"));
        return;
    }

    this->m_jniObject = env->NewGlobalRef(jniObject);
}

BaseJNI::~BaseJNI() {
    JNIEnv* env;

    jint res = this->AttachCurrentThread(&env);

    if (res == JNI_OK) {
        env->DeleteGlobalRef(this->m_jniObject);
        this->DetachCurrentThread();
    }
}

jint BaseJNI::AttachCurrentThread(JNIEnv** env) {
    //TODO check value
    jint res;
#if !defined(JNI_VERSION_1_2) || defined(ANDROID_NDK)
    res = this->m_javaVM->AttachCurrentThread(env, 0);
#else
    res = this->m_javaVM->AttachCurrentThread((void **) env, NULL);
#endif

    if (res != JNI_OK) {
        DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_CRITICAL, ("Can't attach to current thread (%d)", res));
    }

    return res;
}

void BaseJNI::DetachCurrentThread() {
    jint res = this->m_javaVM->DetachCurrentThread();
    if (res < 0) {
        DEBUGLOG(LOG_SS_NB_PAL, LOG_SEV_IMPORTANT, ("Detach Current Thread failed (%d)!!!", res));
    }
}

jobject BaseJNI::callObjectMethod(jclass clazz, jmethodID mid) {
    JNIEnv* env;

    jint res = this->AttachCurrentThread(&env);

    if (res == JNI_OK) {
        jobject jniObject = env->CallStaticObjectMethod(clazz, mid,
                this->m_jniObject);

        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
        }

        this->DetachCurrentThread();
        return jniObject;
    }

    return NULL;
}

jint BaseJNI::callIntMethod(jclass clazz, jmethodID mid) {
    JNIEnv* env;

    jint res = this->AttachCurrentThread(&env);
    if (res == JNI_OK) {
        jint jniObject =
                env->CallStaticIntMethod(clazz, mid, this->m_jniObject);

        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
        }

        this->DetachCurrentThread();
        return jniObject;
    }

    return 0;
}
