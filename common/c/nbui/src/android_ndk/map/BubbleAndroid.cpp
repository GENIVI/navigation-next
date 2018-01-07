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
    @file     BubbleAndroid.cpp
*/
/*
* (C) Copyright 2013 by TeleCommunication Systems, Inc.
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
/*! @{ */

#include "BubbleAndroid.h"
#include <android/log.h>
#include "palstdlib.h"

#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, "Bubble", __VA_ARGS__)

struct fields_bubble
{
    jclass clazz;
    jmethodID showBubble;
    jmethodID hideBubble;
    jmethodID updateBubble;
    jmethodID removeBubble;
};
static fields_bubble fields;

extern JNIEnv* getJNIEnv();

BubbleAndroid::BubbleAndroid(jobject bubble):m_bubble(bubble)
{
    nsl_assert(m_bubble);
}

BubbleAndroid::~BubbleAndroid()
{
    Remove();
}

void
BubbleAndroid::Show(float x, float y, int orientation)
{
    JNIEnv* env = getJNIEnv();
    if (env && m_bubble && fields.showBubble)
    {
        env->CallVoidMethod(m_bubble, fields.showBubble, x, y, orientation);
    }
}

void
BubbleAndroid::Update(float x, float y, int orientation)
{
    JNIEnv* env = getJNIEnv();
    if (env && m_bubble && fields.updateBubble)
    {
        env->CallVoidMethod(m_bubble, fields.updateBubble, x, y, orientation);
    }
}

void
BubbleAndroid::Hide()
{
    JNIEnv* env = getJNIEnv();
    if (env && m_bubble && fields.hideBubble)
    {
        env->CallVoidMethod(m_bubble, fields.hideBubble);
    }
}

void
BubbleAndroid::Remove()
{
    JNIEnv* env = getJNIEnv();
    if (env && m_bubble && fields.removeBubble)
    {
        env->CallVoidMethod(m_bubble, fields.removeBubble);
        env->DeleteGlobalRef(m_bubble);
        m_bubble = NULL;
    }
}

int
register_nbui_bubble(JNIEnv* env)
{
    if(env == NULL)
    {
        return JNI_ERR;
    }

    jclass clazz = env->FindClass("com/locationtoolkit/map3d/model/Bubble");
    if(clazz == NULL)
    {
        LOGE("Cannot find class Bubble!");
        env->ExceptionClear();
        return JNI_ERR;
    }
    fields.clazz = (jclass) env->NewGlobalRef(clazz);

    /* The local reference is no longer useful */
    env->DeleteLocalRef(clazz);
    if (fields.clazz == NULL)
    {
        LOGE("Cannot create global reference for class Bubble!");
        return JNI_ERR;
    }

    fields.showBubble = env->GetMethodID(fields.clazz, "show", "(FFI)V");
    if (fields.showBubble == NULL)
    {
        LOGE("Cannot find Bubble.show!");
        return JNI_ERR;
    }

    fields.updateBubble = env->GetMethodID(fields.clazz, "update", "(FFI)V");
    if (fields.updateBubble == NULL)
    {
        LOGE("Cannot find Bubble.update!");
        return JNI_ERR;
    }

    fields.hideBubble = env->GetMethodID(fields.clazz, "hide", "()V");
    if (fields.hideBubble == NULL)
    {
        LOGE("Cannot find Bubble.hide!");
        return JNI_ERR;
    }

    fields.removeBubble = env->GetMethodID(fields.clazz, "remove", "()V");
    if (fields.removeBubble == NULL)
    {
        LOGE("Cannot find Bubble.remove!");
        return JNI_ERR;
    }

    return JNI_OK;
}

int
unregister_nbui_bubble(JNIEnv* env)
{
    if(env == NULL)
    {
        return JNI_ERR;
    }

    if (fields.clazz)
    {
        env->DeleteGlobalRef(fields.clazz);
        fields.clazz = NULL;
    }
    return JNI_OK;
}

/*! @} */
