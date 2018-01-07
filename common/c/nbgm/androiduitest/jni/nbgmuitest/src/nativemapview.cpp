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

#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <stdlib.h>
#include <string>
#include "nbgm.h"
#include "nbgmmapview.h"
#include "jniutility.h"
#include "mapviewcontroller.h"

#define LOG_LINE __android_log_print(ANDROID_LOG_INFO, "NBGM", "%s, %d", __func__, __LINE__)

jfieldID mapViewFieldID = 0;

static MapViewController*
GetInstance(JNIEnv*  env, jobject  thiz)
{
    if(mapViewFieldID)
    {
        return (MapViewController*)env->GetLongField(thiz, mapViewFieldID);
    }
    else
    {
        return NULL;
    }
}

static MapViewControllerConfiguration
GetControllerConfiguration(JNIEnv*  env, jobject configuration)
{
    MapViewControllerConfiguration result;
    jclass clazz = env->FindClass("com/navbuilder/nbgm/Configuration");

    if(clazz)
    {
        jfieldID field = env->GetFieldID(clazz, "preferredLanguageCode", "I");
        result.preferredLanguageCode = env->GetIntField(configuration, field);
        __android_log_print(ANDROID_LOG_INFO, "NBGM", "result.preferredLanguageCode:%d", result.preferredLanguageCode);

        field = env->GetFieldID(clazz, "enableLOD", "Z");
        result.enableLOD = env->GetBooleanField(configuration, field);
        __android_log_print(ANDROID_LOG_INFO, "NBGM", "result.enableLOD:%d", result.enableLOD);

        field = env->GetFieldID(clazz, "deviceDPI", "F");
        result.deviceDPI = env->GetFloatField(configuration, field);
        __android_log_print(ANDROID_LOG_INFO, "NBGM", "result.deviceDPI:%f", result.deviceDPI);

        field = env->GetFieldID(clazz, "workSpace", "Ljava/lang/String;");
        jstring jworkSpace = (jstring)env->GetObjectField(configuration, field);
        result.workSpace = NewStdString(env, jworkSpace);
        __android_log_print(ANDROID_LOG_INFO, "NBGM", "result.workSpace:%s", result.workSpace.c_str());

        field = env->GetFieldID(clazz, "renderSystemType", "I");
        result.renderSystemType = (NBGM_RenderSystemType)env->GetIntField(configuration, field);
        __android_log_print(ANDROID_LOG_INFO, "NBGM", "result.renderSystemType:%d", result.renderSystemType);

        field = env->GetFieldID(clazz, "drawSky", "Z");
        result.drawSky = env->GetBooleanField(configuration, field);
        __android_log_print(ANDROID_LOG_INFO, "NBGM", "result.drawSky:%d", result.drawSky);

        field = env->GetFieldID(clazz, "drawFlag", "Z");
        result.drawFlag = env->GetBooleanField(configuration, field);
        __android_log_print(ANDROID_LOG_INFO, "NBGM", "result.drawFlag:%d", result.drawFlag);

        field = env->GetFieldID(clazz, "drawAvatar", "Z");
        result.drawAvatar = env->GetBooleanField(configuration, field);
        __android_log_print(ANDROID_LOG_INFO, "NBGM", "result.drawAvatar:%d", result.drawAvatar);

        field = env->GetFieldID(clazz, "renderInterval", "I");
        result.renderInterval = env->GetIntField(configuration, field);
        __android_log_print(ANDROID_LOG_INFO, "NBGM", "result.renderInterval:%d", result.renderInterval);
    }

    return result;
}

void
com_navbuilder_native_mapview_initialize( JNIEnv*  env, jobject  thiz, jobject surface, jobject config)
{
    MapViewController* instance = new MapViewController();
    env->SetLongField(thiz, mapViewFieldID, (long)instance);

    MapViewControllerConfiguration configuration = GetControllerConfiguration(env, config);
    instance->Initialize(env, surface, configuration);
}

void
com_navbuilder_native_mapview_finalize( JNIEnv*  env, jobject  thiz)
{

    MapViewController* self = GetInstance(env, thiz);
    self->Finalize();

    env->SetLongField(thiz, mapViewFieldID, 0);
    delete self;
}

void
com_navbuilder_native_mapview_invalidate( JNIEnv*  env, jobject  thiz)
{
    GetInstance(env, thiz)->Invalidate();
}

void
com_navbuilder_native_mapview_setBackground( JNIEnv*  env, jobject  thiz, jboolean background)
{
    GetInstance(env, thiz)->SetBackground(background);
}

void
com_navbuilder_native_mapview_setViewCenter(JNIEnv*  env, jobject  thiz, jdouble mercatorX, jdouble mercatorY)
{
    GetInstance(env, thiz)->SetViewCenter(mercatorX, mercatorY);
}

void
com_navbuilder_native_mapview_setViewPointDistance(JNIEnv*  env, jobject  thiz, jfloat distance)
{
    GetInstance(env, thiz)->SetViewPointDistance(distance);
}

void
com_navbuilder_native_mapview_setViewSize(JNIEnv*  env, jobject  thiz, jint x, jint y, jint width, jint height)
{
    GetInstance(env, thiz)->SetViewSize(x, y, width, height);
}

void
com_navbuilder_native_mapview_setSurfaceSize(JNIEnv*  env, jobject  thiz, jint width, jint height)
{
    GetInstance(env, thiz)->SetSurfaceSize(width, height);
}

void
com_navbuilder_native_mapview_setPerspective(JNIEnv*  env, jobject  thiz, jfloat fov, jfloat aspect)
{
    GetInstance(env, thiz)->SetPerspective(fov, aspect);
}

void
com_navbuilder_native_mapview_surfaceCreated(JNIEnv*  env, jobject  thiz, jobject surface)
{
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    GetInstance(env, thiz)->SurfaceCreated(window);
}

void
com_navbuilder_native_mapview_surfaceDestroyed(JNIEnv*  env, jobject  thiz)
{
    GetInstance(env, thiz)->SurfaceDestroyed();
}

void
com_navbuilder_native_mapview_onTouchEvent(JNIEnv*  env, jobject  thiz, jfloat screenX, jfloat screenY)
{
    GetInstance(env, thiz)->OnTouchEvent(screenX, screenY);
}

void
com_navbuilder_native_mapview_onPaning(JNIEnv*  env, jobject  thiz, jfloat screenX, jfloat screenY)
{
    GetInstance(env, thiz)->OnPaning(screenX, screenY);
}

int
com_navbuilder_native_mapview_loadCommonMaterial(JNIEnv*  env, jobject  thiz, jstring materialName, jstring filePath)
{
    std::string mn = NewStdString(env, materialName);
    std::string fp = NewStdString(env, filePath);
    GetInstance(env, thiz)->LoadCommonMaterial(mn, fp);
    __android_log_print(ANDROID_LOG_INFO, "NBGM", "%s, %s, %s", __func__, mn.c_str(), fp.c_str());
    return 0;
}

int
com_navbuilder_native_mapview_setCurrentCommonMaterial(JNIEnv*  env, jobject  thiz, jstring materialName)
{
    std::string mn = NewStdString(env, materialName);
    GetInstance(env, thiz)->SetCurrentCommonMaterial(mn);
    return 0;
}

int
com_navbuilder_native_mapview_loadNBMTile(JNIEnv*  env, jobject  thiz, jstring nbmName, jint baseDrawOrder, jint labelDrawOrder, jstring filePath)
{
    std::string nn = NewStdString(env, nbmName);
    std::string fp = NewStdString(env, filePath);

    GetInstance(env, thiz)->LoadNBMTile(nn, baseDrawOrder, labelDrawOrder, fp);
    return 0;
}

void
com_navbuilder_native_mapview_unLoadTile(JNIEnv*  env, jobject  thiz, jstring nbmName)
{
    std::string nn = NewStdString(env, nbmName);
    GetInstance(env, thiz)->UnLoadTile(nn);
}

static JNINativeMethod gMethods[] = {
        { "Initialize",                       "(Ljava/lang/Object;Ljava/lang/Object;)V",         (void *) &com_navbuilder_native_mapview_initialize },
        { "Finalize",                         "()V",                                             (void *) &com_navbuilder_native_mapview_finalize },
        { "Invalidate",                       "()V",                                             (void *) &com_navbuilder_native_mapview_invalidate },
        { "SetBackground",                    "(Z)V",                                            (void *) &com_navbuilder_native_mapview_setBackground },
        { "SetViewCenter",                    "(DD)V",                                           (void *) &com_navbuilder_native_mapview_setViewCenter },
        { "SetViewPointDistance",             "(F)V",                                            (void *) &com_navbuilder_native_mapview_setViewPointDistance },
        { "SetViewSize",                      "(IIII)V",                                         (void *) &com_navbuilder_native_mapview_setViewSize },
        { "SetSurfaceSize",                   "(II)V",                                           (void *) &com_navbuilder_native_mapview_setSurfaceSize },
        { "SetPerspective",                   "(FF)V",                                           (void *) &com_navbuilder_native_mapview_setPerspective },
        { "SurfaceCreated",                   "(Ljava/lang/Object;)V",                           (void *) &com_navbuilder_native_mapview_surfaceCreated },
        { "SurfaceDestroyed",                 "()V",                                             (void *) &com_navbuilder_native_mapview_surfaceDestroyed },
        { "OnTouchEvent",                     "(FF)V",                                           (void *) &com_navbuilder_native_mapview_onTouchEvent },
        { "OnPaning",                         "(FF)V",                                           (void *) &com_navbuilder_native_mapview_onPaning },
        { "LoadCommonMaterial",               "(Ljava/lang/String;Ljava/lang/String;)I",         (void *) &com_navbuilder_native_mapview_loadCommonMaterial },
        { "SetCurrentCommonMaterial",         "(Ljava/lang/String;)I",                           (void *) &com_navbuilder_native_mapview_setCurrentCommonMaterial },
        { "LoadNBMTile",                      "(Ljava/lang/String;IILjava/lang/String;)I",       (void *) &com_navbuilder_native_mapview_loadNBMTile },
        { "UnLoadTile",                       "(Ljava/lang/String;)V",                           (void *) &com_navbuilder_native_mapview_unLoadTile },
};

int register_com_navbuilder_nbgm_NBGMMapView(JNIEnv *env)
{
    jclass clazz;

    clazz = env->FindClass("com/navbuilder/nbgm/NativeMapView");
    if (clazz == NULL) {
        return JNI_ERR;
    }

    mapViewFieldID = env->GetFieldID(clazz, "mNativePtr", "J");
    return env->RegisterNatives(clazz, gMethods, sizeof(gMethods) / sizeof(gMethods[0]));
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        goto bail;
    }
    //assert(env != NULL);

    if (register_com_navbuilder_nbgm_NBGMMapView(env) < 0) {
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;
    bail: return result;
}
