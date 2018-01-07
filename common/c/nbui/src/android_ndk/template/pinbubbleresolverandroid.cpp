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
   (C) Copyright 2013 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */
#include "pinbubbleresolverandroid.h"
#include <jni.h>
#include "palstdlib.h"
#include "BubbleAndroid.h"

extern JNIEnv* getJNIEnv();

struct fields_bubble {
    jclass clazz;
    jmethodID getBubble;
    jmethodID getStaticPoiBubble;
    jmethodID getTrafficIncidentBubble;
};
static fields_bubble fields;

/* See description in header file. */
PinBubbleResolverAndroid::PinBubbleResolverAndroid()
{
}

/* See description in header file. */
PinBubbleResolverAndroid::~PinBubbleResolverAndroid()
{
}

/* See description in header file. */
shared_ptr<nbmap::BubbleInterface> PinBubbleResolverAndroid::GetPinBubble(double latitude, double longitude)
{
    JNIEnv* env = getJNIEnv();
    if(env == NULL)
    {
        return shared_ptr <nbmap::BubbleInterface>();
    }

    jobject bubble = env->CallStaticObjectMethod(fields.clazz, fields.getBubble, latitude, longitude);
    if(bubble == NULL)
    {
        return shared_ptr <nbmap::BubbleInterface>();
    }

    jobject retBubble = (jobject) env->NewGlobalRef(bubble);
    env->DeleteLocalRef(bubble);
    if(retBubble == NULL)
    {
        return shared_ptr <nbmap::BubbleInterface>();
    }

    return shared_ptr<nbmap::BubbleInterface>(new BubbleAndroid(retBubble));
}

shared_ptr<nbmap::BubbleInterface>
PinBubbleResolverAndroid::GetStaticPoiBubble(const std::string& id, const std::string& name, double latitude, double longitude)
{
    nsl_assert(fields.getStaticPoiBubble);
    JNIEnv* env = getJNIEnv();
    if(env == NULL)
    {
        return shared_ptr<nbmap::BubbleInterface>();
    }

    jstring strId = env->NewStringUTF(id.c_str());
    jstring strName = env->NewStringUTF(name.c_str());
    jobject bubble = env->CallStaticObjectMethod(fields.clazz, fields.getStaticPoiBubble, strId, strName);
    env->DeleteLocalRef(strId);
    env->DeleteLocalRef(strName);

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

shared_ptr<nbmap::BubbleInterface>
PinBubbleResolverAndroid::GetTrafficIncidentBubble(int type, int severity, long entryTime, long startTime, long endTime, const std::string& description, const std::string& road, double lat, double lon)
{
    nsl_assert(fields.getTrafficIncidentBubble);
    JNIEnv* env = getJNIEnv();
    if(env == NULL)
    {
        return shared_ptr<nbmap::BubbleInterface>();
    }

    jstring strDescription = env->NewStringUTF(description.c_str());
    jstring strRoad= env->NewStringUTF(road.c_str());
    jobject bubble = env->CallStaticObjectMethod(fields.clazz, fields.getTrafficIncidentBubble, type, severity, (jlong)entryTime, (jlong)startTime, (jlong)endTime, strDescription, strRoad, lat, lon);
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
register_nbui_bubble_resolver(JNIEnv* env)
{
    if(env == NULL)
    {
        return JNI_ERR;
    }

    jclass clazz = env->FindClass("com/locationtoolkit/map3d/internal/nbui/BubbleView");
    if (clazz == NULL)
    {
        env->ExceptionClear();
        return JNI_ERR;
    }

    fields.clazz = (jclass) env->NewGlobalRef(clazz);
    /* The local reference is no longer useful */
    env->DeleteLocalRef(clazz);
    if (fields.clazz == NULL)
    {
       return JNI_ERR;
    }

    fields.getBubble = env->GetStaticMethodID(fields.clazz, "getBubble", "(DD)Lcom/locationtoolkit/map3d/internal/nbui/BubbleView;");
    if (fields.getBubble == NULL)
    {
      return JNI_ERR;
    }

    fields.getStaticPoiBubble = env->GetStaticMethodID(fields.clazz, "getStaticPoiBubble", "(Ljava/lang/String;Ljava/lang/String;)Lcom/locationtoolkit/map3d/internal/nbui/BubbleView;");
    if (fields.getStaticPoiBubble == NULL)
    {
        return JNI_ERR;
    }

    fields.getTrafficIncidentBubble = env->GetStaticMethodID(fields.clazz, "getTrafficIncidentBubble", "(IIJJJLjava/lang/String;Ljava/lang/String;DD)Lcom/locationtoolkit/map3d/internal/nbui/BubbleView;");
    if (fields.getTrafficIncidentBubble == NULL)
    {
        return JNI_ERR;
    }
    return JNI_OK;
}

int unregister_nbui_bubble_resolver(JNIEnv* env)
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
