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
 @file     custompinbubbleresolverandroid.cpp
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

#include "custompinbubbleresolverandroid.h"
#include "BubbleAndroid.h"
#include "palstdlib.h"
#include <android/log.h>

#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, "PinBubble", __VA_ARGS__)

struct fields_bubble_resolver
{
    jclass clazz;
    jmethodID getPinBubble;
    jmethodID getStaticPoiBubble;
    jmethodID getTrafficIncidentBubble;
};
static fields_bubble_resolver fields;

extern JNIEnv* getJNIEnv();

CustomPinBubbleResolverAndroid::CustomPinBubbleResolverAndroid(jobject bubbleResolver)
{
    JNIEnv* env = getJNIEnv();
    if (env && bubbleResolver)
    {
        m_bubbleResolver = env->NewGlobalRef(bubbleResolver);
        nsl_assert(m_bubbleResolver);
    }
}

CustomPinBubbleResolverAndroid::~CustomPinBubbleResolverAndroid()
{
    JNIEnv* env = getJNIEnv();
    if (env && m_bubbleResolver)
    {
        env->DeleteGlobalRef(m_bubbleResolver);
    }
}

shared_ptr<nbmap::BubbleInterface>
CustomPinBubbleResolverAndroid::GetPinBubble(double latitude, double longitude)
{
    nsl_assert(fields.getPinBubble);
    JNIEnv* env = getJNIEnv();
    if(env == NULL || m_bubbleResolver == NULL)
    {
        return shared_ptr<nbmap::BubbleInterface>();
    }

    jobject bubble = env->CallObjectMethod(m_bubbleResolver, fields.getPinBubble, latitude, longitude);
    if(bubble == NULL)
    {
        LOGE("Get PinBubble object Failed!");
        return shared_ptr<nbmap::BubbleInterface>();
    }

    jobject retBubble = (jobject) env->NewGlobalRef(bubble);
    /* The local reference is no longer useful */
    env->DeleteLocalRef(bubble);
    if(retBubble == NULL)
    {
        LOGE("Cannot create global reference for Bubble object!");
        return shared_ptr<nbmap::BubbleInterface>();
    }

    return shared_ptr <nbmap::BubbleInterface>(new BubbleAndroid(retBubble));
}

shared_ptr<nbmap::BubbleInterface>
CustomPinBubbleResolverAndroid::GetStaticPoiBubble(const std::string& id, const std::string& name, double latitude, double longitude)
{
    nsl_assert(fields.getStaticPoiBubble);
    JNIEnv* env = getJNIEnv();
    if(env == NULL || m_bubbleResolver == NULL)
    {
        return shared_ptr<nbmap::BubbleInterface>();
    }

    jstring strId = env->NewStringUTF(id.c_str());
    jstring strName = env->NewStringUTF(name.c_str());
    jobject bubble = env->CallObjectMethod(m_bubbleResolver, fields.getStaticPoiBubble, strId, strName, latitude, longitude);
    env->DeleteLocalRef(strId);
    env->DeleteLocalRef(strName);

    if(bubble == NULL)
    {
        LOGE("Get PinBubble object Failed!");
        return shared_ptr<nbmap::BubbleInterface>();
    }

    jobject retBubble = (jobject) env->NewGlobalRef(bubble);
    /* The local reference is no longer useful */
    env->DeleteLocalRef(bubble);
    if(retBubble == NULL)
    {
        LOGE("Cannot create global reference for Bubble object!");
        return shared_ptr<nbmap::BubbleInterface>();
    }

    return shared_ptr <nbmap::BubbleInterface>(new BubbleAndroid(retBubble));
}

shared_ptr<nbmap::BubbleInterface>
CustomPinBubbleResolverAndroid::GetTrafficIncidentBubble(int type, int severity, long entryTime, long startTime, long endTime, const std::string& description, const std::string& road, double lat, double lon)
{
    nsl_assert(fields.getTrafficIncidentBubble);
    JNIEnv* env = getJNIEnv();
    if(env == NULL)
    {
        return shared_ptr<nbmap::BubbleInterface>();
    }

    jstring strDescription = env->NewStringUTF(description.c_str());
    jstring strRoad= env->NewStringUTF(road.c_str());
    jobject bubble = env->CallObjectMethod(m_bubbleResolver, fields.getTrafficIncidentBubble, type, severity,
                                          (jlong)entryTime, (jlong)startTime, (jlong)endTime, strDescription, strRoad, lat, lon);
    env->DeleteLocalRef(strDescription);
    env->DeleteLocalRef(strRoad);

    if(bubble == NULL)
    {
        return shared_ptr<nbmap::BubbleInterface>();
    }

    jobject retBubble = (jobject) env->NewGlobalRef(bubble);

    env->DeleteLocalRef(bubble);
    if(retBubble == NULL)
    {
        return shared_ptr<nbmap::BubbleInterface>();
    }

    return shared_ptr <nbmap::BubbleInterface>(new BubbleAndroid(retBubble));
}

int
register_custom_pin_bubble_resolver(JNIEnv* env)
{
    if(env == NULL)
    {
        return JNI_ERR;
    }

    jclass clazz = env->FindClass("com/locationtoolkit/map3d/internal/jni/BubbleResolverWrapper");
    if(clazz == NULL)
    {
        LOGE("Cannot find class PinBubbleResolver!");
        env->ExceptionClear();
        return JNI_ERR;
    }

    fields.clazz = (jclass) env->NewGlobalRef(clazz);
    /* The local reference is no longer useful */
    env->DeleteLocalRef(clazz);
    if (fields.clazz == NULL)
    {
        LOGE("Cannot create global reference for class PinBubbleResolver!");
        return JNI_ERR;
    }

    fields.getPinBubble = env->GetMethodID(fields.clazz, "getPinBubble", "(DD)Lcom/locationtoolkit/map3d/model/Bubble;");
    if (fields.getPinBubble == NULL)
    {
        LOGE("Cannot find PinBubbleResolver.getPinBubble!");
        return JNI_ERR;
    }

    fields.getStaticPoiBubble = env->GetMethodID(fields.clazz, "getStaticPoiBubble", "(Ljava/lang/String;Ljava/lang/String;DD)Lcom/locationtoolkit/map3d/model/Bubble;");
    if (fields.getStaticPoiBubble == NULL)
    {
        LOGE("Cannot find PinBubbleResolver.getStaticPoiBubble!");
        return JNI_ERR;
    }

    fields.getTrafficIncidentBubble = env->GetMethodID(fields.clazz, "getTrafficIncidentBubble", "(IIJJJLjava/lang/String;Ljava/lang/String;DD)Lcom/locationtoolkit/map3d/model/Bubble;");
    if (fields.getTrafficIncidentBubble == NULL)
    {
        LOGE("Cannot find PinBubbleResolver.getTrafficIncidentBubble!");
        return JNI_ERR;
    }
    return JNI_OK;
}

int
unregister_custom_pin_bubble_resolver(JNIEnv* env)
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