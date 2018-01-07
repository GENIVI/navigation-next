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
 @file     tileproviderandroid.cpp
 */
/*
* (C) Copyright 2014 by TeleCommunication Systems, Inc.
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

#include "tileproviderandroid.h"
#include "maptile.h"
#include "maplayer.h"
#include "palstdlib.h"
#include <android/log.h>

using namespace nbmap;

#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, "Tile", __VA_ARGS__)

struct fields_tile_provider
{
    jclass clazzProvider;
    jmethodID interest;
    jmethodID disinterest;
    jmethodID getLayerId;
    jclass clazzTile;
    jmethodID getId;
    jmethodID detach;
};
static fields_tile_provider fields;

extern JNIEnv* getJNIEnv();

TileProviderAndroid::TileProviderAndroid(jobject tileProvider, MapController& controller):m_controller(controller)
{
    JNIEnv* env = getJNIEnv();
    if (env && tileProvider)
    {
        m_tileProvider = env->NewGlobalRef(tileProvider);
        nsl_assert(m_tileProvider);
    }
}

TileProviderAndroid::~TileProviderAndroid()
{
    JNIEnv* env = getJNIEnv();
    if (env && m_tileProvider)
    {
        env->DeleteGlobalRef(m_tileProvider);
    }
}

void
TileProviderAndroid::interest(int x, int y, int zoom)
{
    nsl_assert(fields.interest && m_tileProvider);
    JNIEnv* env = getJNIEnv();
    if(env == NULL)
    {
        return;
    }

    jobject tileObj = env->CallObjectMethod(m_tileProvider, fields.interest, x, y, zoom);
    if(tileObj == NULL)
    {
        return;
    }

    jobject gtileObj = env->NewGlobalRef(tileObj);
    if(gtileObj == NULL)
    {
        return;
    }
    /* The local reference is no longer useful */
    env->DeleteLocalRef(tileObj);

    MapTile *tile = reinterpret_cast<MapTile*>(env->CallIntMethod(gtileObj, fields.getId));
    MapLayer *layer = reinterpret_cast<MapLayer*>(env->CallIntMethod(m_tileProvider, fields.getLayerId));
    if(tile && layer)
    {
        layer->LoadTile(tile);
        // Once the tile is interested and provided by the user,
        // 1. Tile memory will be passed to the layer.
        // 2. Tile object will release the native id.
        env->CallVoidMethod(gtileObj, fields.detach);
    }
    env->DeleteGlobalRef(gtileObj);
}

void
TileProviderAndroid::disinterest(int x, int y, int zoom)
{
    nsl_assert(fields.disinterest);
    JNIEnv* env = getJNIEnv();
    if(env == NULL || m_tileProvider == NULL)
    {
        return;
    }
    env->CallVoidMethod(m_tileProvider, fields.disinterest, x, y, zoom);
}

int
register_tile_provider(JNIEnv* env)
{
    if(env == NULL)
    {
        return JNI_ERR;
    }

    jclass clazzProvider = env->FindClass("com/locationtoolkit/map3d/internal/model/TileProviderImpl");
    if(clazzProvider == NULL)
    {
        LOGE("Cannot find class TileProviderImpl!");
        env->ExceptionClear();
        return JNI_ERR;
    }

    fields.clazzProvider = (jclass) env->NewGlobalRef(clazzProvider);
    /* The local reference is no longer useful */
    env->DeleteLocalRef(clazzProvider);
    if (fields.clazzProvider == NULL)
    {
        LOGE("Cannot create global reference for class TileProviderImpl!");
        return JNI_ERR;
    }

    fields.interest = env->GetMethodID(fields.clazzProvider, "interest", "(III)Lcom/locationtoolkit/map3d/model/Tile;");
    if (fields.interest == NULL)
    {
        LOGE("Cannot find TileProviderImpl.interest!");
        return JNI_ERR;
    }

    fields.disinterest = env->GetMethodID(fields.clazzProvider, "disinterest", "(III)V");
    if (fields.disinterest == NULL)
    {
        LOGE("Cannot find TileProviderImpl.disinterest!");
        return JNI_ERR;
    }

    fields.getLayerId = env->GetMethodID(fields.clazzProvider, "getLayerId", "()I");
    if (fields.getLayerId == NULL)
    {
        LOGE("Cannot find TileProviderImpl.getLayerId!");
        return JNI_ERR;
    }

    jclass clazzTile = env->FindClass("com/locationtoolkit/map3d/internal/model/TileImpl");
    if(clazzTile == NULL)
    {
        LOGE("Cannot find class TileImpl!");
        env->ExceptionClear();
        return JNI_ERR;
    }

    fields.clazzTile = (jclass) env->NewGlobalRef(clazzTile);
    /* The local reference is no longer useful */
    env->DeleteLocalRef(clazzTile);
    if (fields.clazzTile == NULL)
    {
        LOGE("Cannot create global reference for class TileImpl!");
        return JNI_ERR;
    }

    fields.getId = env->GetMethodID(fields.clazzTile, "getId", "()I");
    if (fields.getId == NULL)
    {
        LOGE("Cannot find TileImpl.getId!");
        return JNI_ERR;
    }

    fields.detach = env->GetMethodID(fields.clazzTile, "detach", "()V");
    if (fields.detach == NULL)
    {
        LOGE("Cannot find TileImpl.detach!");
        return JNI_ERR;
    }
    return JNI_OK;
}

int
unregister_tile_provider(JNIEnv* env)
{
    if(env == NULL)
    {
        return JNI_ERR;
    }

    if (fields.clazzProvider)
    {
        env->DeleteGlobalRef(fields.clazzProvider);
        fields.clazzProvider = NULL;
    }

    if (fields.clazzTile)
    {
        env->DeleteGlobalRef(fields.clazzTile);
        fields.clazzTile = NULL;
    }
    return JNI_OK;
}

 /*! @} */