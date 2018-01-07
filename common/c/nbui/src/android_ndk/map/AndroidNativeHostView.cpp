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
 @file     NBGMMapViewAndroid.cpp
 */
/*
 (C) Copyright 2012 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#include "AndroidNativeHostView.h"
#include "paldebuglog.h"
#include "palstdlib.h"
#include "palclock.h"
#include "Lock.h"
#include <assert.h>
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "MapServicesConfiguration.h"
#include <nbnetwork.h>
#include <nbnetworkoptions.h>
#include <palfile.h>
#include "MetadataConfiguration.h"
#include <android/log.h>

using namespace nb;

#define USE_GLES20 1

struct fields_t
{
    jclass clazz;
    jfieldID context;
    jmethodID constructorID;
    jmethodID enableDebugInfoMethodID;
    jmethodID setDebugInfoMethodID;
    jmethodID updateStateMethodID;
    jmethodID updateTimestampLabelMethodID;
    jmethodID updateButtonStateMethodID;
    jmethodID addMapLengedBarID;
    jmethodID addMapLengedColorID;
    jmethodID showMapLengedID;
    jmethodID hideMapLengedID;
    jmethodID showTipID;

    jclass surfaceView_clazz;
    jmethodID surfaceViewConstructorID;

    jclass frameLayout_clazz;
    jmethodID addViewMethodID;
};

static fields_t fields;
static JavaVM* cachedVM = NULL;

JNIEnv* getJNIEnv()
{
    JNIEnv* env = NULL;
    if (cachedVM->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
        // Should never happen
        assert(false);
    }
    return env;
}

AndroidNativeHostView::AndroidNativeHostView():
    m_mapViewUI(NULL),
    m_width(0),
    m_height(0),
    m_lastUpdateTime(0),
    m_mapLegendCreated(false)
{
}

AndroidNativeHostView::~AndroidNativeHostView()
{
    ANativeWindow_release(m_window);
    JNIEnv* env = getJNIEnv();
    MapSurfaceViewContext* context = reinterpret_cast<MapSurfaceViewContext*>(env->GetIntField(m_androidObject, fields.context));
    if(context)
    {
        env->DeleteGlobalRef(context->frameLayoutObject);
        env->DeleteGlobalRef(context->nativeContentObject);
        env->DeleteGlobalRef(context->nativeMapControllerObject);
        env->DeleteGlobalRef(context->mapControllerObject);
        env->DeleteGlobalRef(context->mapToolbarObject);
        delete context;
    }

    env->DeleteGlobalRef(m_androidObject);

}

NB_Error
AndroidNativeHostView::Initialize(PAL_Instance* pal)
{
    NB_Error ret = NE_OK;

    m_pal = pal;

    return ret;
}

void
AndroidNativeHostView::DrawFrame()
{
    DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("AndroidNativeHostView::DrawFrame()"));

    uint32 cTime= PAL_ClockGetTimeMs();
    uint32 interval = cTime - m_lastUpdateTime;

    NB_Error error = m_mapViewUI->UI_Render(interval);

    DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("AndroidNativeHostView::DrawFrame() return %d", error));

    m_lastUpdateTime = cTime;

}

Render_Error
AndroidNativeHostView::Render(const bool skipDrawFrame)
{
    Render_Error error = RE_OK;
    DrawFrame();
    return error;
}

void
AndroidNativeHostView::EnableDebugInfo(bool isEnabled)
{
    JNIEnv* env = getJNIEnv();
    if (env)
    {
        env->CallVoidMethod(m_androidObject, fields.enableDebugInfoMethodID, isEnabled);
    }
}

void
AndroidNativeHostView::SetDebugInfo(const char* text)
{
    JNIEnv* env = getJNIEnv();
    if (env)
    {
        jstring str = env->NewStringUTF(text);
        env->CallVoidMethod(m_androidObject, fields.setDebugInfoMethodID, str);
        env->DeleteLocalRef(str);
    }
}

void*
AndroidNativeHostView::GetMapViewContext()
{
    return NULL;
}

void
AndroidNativeHostView::UpdateState(NBGMMapView_State state)
{
    JNIEnv* env = getJNIEnv();
    if (env)
    {
        env->CallVoidMethod(m_androidObject, fields.updateStateMethodID, static_cast<jint>(state));
    }
}

unsigned int*
AndroidNativeHostView::GetDefaultFrameBufferID()
{
    return 0;
}

void
AndroidNativeHostView::GetViewDimension(int& width, int& height)
{
    width = m_width;
    height = m_height;
}

void
AndroidNativeHostView::SetUIInterface(MapViewUIInterface* mapView)
{
    m_mapViewUI = mapView;
    m_mapViewUI->UI_SetViewPort(m_width, m_height);
}

MapViewUIInterface*
AndroidNativeHostView::GetUIInterface() const
{
    return m_mapViewUI;
}

NB_Error
AndroidNativeHostView::SetViewSize(int width, int height)
{
    NB_Error err = NE_OK;
    m_width = width;
    m_height = height;
    if(m_mapViewUI)
    {
        err = m_mapViewUI->UI_SetViewSize(width, height);
    }
    return err;
}

NB_Error
AndroidNativeHostView::SetViewPort(int width, int height)
{
    NB_Error err = NE_OK;

    m_width = width;
    m_height = height;
    if(m_mapViewUI)
    {
        err = m_mapViewUI->UI_SetViewPort(width, height);
    }
    return err;
}

NB_Error
AndroidNativeHostView::HandleViewGesture(MapView_GestureParameters* params)
{
    return m_mapViewUI->UI_HandleViewGesture(params);
}

void
AndroidNativeHostView::SetWindow(ANativeWindow* window)
{
    m_window = window;
}

ANativeWindow*
AndroidNativeHostView::GetWindow()
{
    return m_window;
}

PAL_Instance*
AndroidNativeHostView::GetPal()
{
    return m_pal;
}

void
AndroidNativeHostView::UpdateTimestampLabel(uint32 gpsTime)
{
    JNIEnv* env = getJNIEnv();
    if (env)
    {
        env->CallVoidMethod(m_androidObject, fields.updateTimestampLabelMethodID, gpsTime);
    }
}

void
AndroidNativeHostView::UpdateButtonState(bool animationCanPlay)
{
    JNIEnv* env = getJNIEnv();
    if (env)
    {
        env->CallVoidMethod(m_androidObject, fields.updateButtonStateMethodID, animationCanPlay);
    }
}

void
AndroidNativeHostView::OnMapInitialized()
{

}

void
AndroidNativeHostView::ShowMapLegend(shared_ptr <nbmap::MapLegendInfo> mapLegend)
{
    if(!m_mapLegendCreated)
    {
        CreateMapLegend(mapLegend);
        m_mapLegendCreated = true;
    }
    JNIEnv* env = getJNIEnv();
    if (env)
    {
        env->CallVoidMethod(m_androidObject, fields.showMapLengedID);
    }
}

void
AndroidNativeHostView::CreateMapLegend(shared_ptr <nbmap::MapLegendInfo> mapLegendInfo)
{
    JNIEnv* env = getJNIEnv();
    if (env == NULL)
    {
        return;
    }
    for(uint32 i=0; i<mapLegendInfo->GetTotalColorBars(); ++i)
    {
        const shared_ptr<nbmap::ColorBar> bar = mapLegendInfo->GetColorBar(i);
        jstring name = env->NewStringUTF(bar->first.c_str());
        int barIndex = env->CallIntMethod(m_androidObject, fields.addMapLengedBarID, name);
        for(uint32 j=0; j<bar->second.size(); ++j)
        {
            env->CallVoidMethod(m_androidObject, fields.addMapLengedColorID, barIndex, bar->second[j]);
        }
        env->DeleteLocalRef(name);
    }
}

void
AndroidNativeHostView::HideMapLegend()
{
    JNIEnv* env = getJNIEnv();
    if (env)
    {
        env->CallVoidMethod(m_androidObject, fields.hideMapLengedID);
    }
}

void
AndroidNativeHostView::UpdateSurface(ANativeWindow* window)
{   
    if(window != NULL && m_mapViewUI != NULL)
    {
        if(m_window != NULL)
        {
            ANativeWindow_release(m_window);
        }
        m_window = window;
        m_mapViewUI->UI_UpdateSurface();
    }
}

void
AndroidNativeHostView::DestroySurface()
{
    if(m_mapViewUI)
    {
        m_mapViewUI->UI_DestroySurface();
    }
}

/* See description in header file. */
void AndroidNativeHostView::ShowTip(int tipId)
{
    JNIEnv* env = getJNIEnv();
    if(env)
    {
        env->CallVoidMethod(m_androidObject, fields.showTipID, tipId);
    }
}

void
AndroidNativeHostView::SetMapSurfaceViewContext(void* viewContext)
{
    MapSurfaceViewContext* context = reinterpret_cast<MapSurfaceViewContext*>(viewContext);
    JNIEnv* env = getJNIEnv();

    if (env == NULL)
    {
        return;
    }
    context->nativeHostView = this;

    jobject nativeMapSurfaceController = env->NewObject(fields.clazz, fields.constructorID, context->nativeContentObject, context->mapToolbarObject);
    m_androidObject =  env->NewGlobalRef(nativeMapSurfaceController);
    env->SetIntField(m_androidObject, fields.context, (int)context);

    jobject surfaceview = env->NewObject(fields.surfaceView_clazz, fields.surfaceViewConstructorID, context->nativeContentObject, context->nativeMapControllerObject, context->mapControllerObject, m_androidObject);
    env->CallVoidMethod(context->frameLayoutObject, fields.addViewMethodID, surfaceview);
    env->CallVoidMethod(context->frameLayoutObject, fields.addViewMethodID, context->mapToolbarObject);
    m_renderingConfig = context->renderingConfig;
}

//JNI methods
static jint JNI_HandleViewGesture(JNIEnv* env, jobject thiz,
        jint gestureType,
        jint state,
        jfloat scale,
        jfloat rotationAngle,
        jfloat tiltAngle,
        jfloat locationInViewX,
        jfloat locationInViewY,
        jfloat velocityInViewX,
        jfloat velocityInViewY,
        jfloat scaleVelocity,
        jfloat rotationVelocity,
        jfloat viewSizeX,
        jfloat viewSizeY
        )
{
    DEBUGLOG(LOG_SS_JNI, LOG_SEV_INFO, ("UI_HandleViewGesture: parameters"));

    MapSurfaceViewContext* context = reinterpret_cast<MapSurfaceViewContext*>(env->GetIntField(thiz, fields.context));
    jint ret = -1;

    if (context->nativeHostView)
    {
        MapView_GestureParameters gestureParam;
        nsl_memset(&gestureParam,0, sizeof(MapView_GestureParameters));
        gestureParam.gestureType      = static_cast<MapView_TransformationType>(gestureType);
        gestureParam.state            = static_cast<MapView_GestureRecognizerState>(state);
        gestureParam.scale            = static_cast<float>(scale);
        gestureParam.rotaionAngle     = static_cast<float>(rotationAngle);
        gestureParam.tiltAngle        = static_cast<float>(tiltAngle);
        gestureParam.locationInView.x = static_cast<float>(locationInViewX);
        gestureParam.locationInView.y = static_cast<float>(locationInViewY);
        gestureParam.velocityInView.x = static_cast<float>(velocityInViewX);
        gestureParam.velocityInView.y = static_cast<float>(velocityInViewY);
        gestureParam.scaleVelocity    = static_cast<float>(scaleVelocity);
        gestureParam.rotationVelocity = static_cast<float>(rotationVelocity);
        gestureParam.viewSize.x       = static_cast<float>(viewSizeX);
        gestureParam.viewSize.y       = static_cast<float>(viewSizeY);

        DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("UI_HandleViewGesture: gestureType=%u", gestureParam.gestureType));
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("UI_HandleViewGesture: state=%u", gestureParam.state));
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("UI_HandleViewGesture: scale=%f", gestureParam.scale));
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("UI_HandleViewGesture: rotationAngle=%f", gestureParam.rotaionAngle));
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("UI_HandleViewGesture: locationInViewX=%f", gestureParam.locationInView.x));
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("UI_HandleViewGesture: locationInViewY=%f", gestureParam.locationInView.y));
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("UI_HandleViewGesture: velocityInViewX=%f", gestureParam.velocityInView.x));
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("UI_HandleViewGesture: velocityInViewY=%f", gestureParam.velocityInView.y));
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("UI_HandleViewGesture: scaleVelocity=%f", gestureParam.scaleVelocity));
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("UI_HandleViewGesture: rotationVelocity=%f", gestureParam.rotationVelocity));
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("UI_HandleViewGesture: viewSize.x=%f", gestureParam.viewSize.x));
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("UI_HandleViewGesture: viewSize.y=%f", gestureParam.viewSize.y));

        ret = static_cast<jint>(context->nativeHostView->HandleViewGesture(&gestureParam));
    }
    else
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("UI_HandleViewGesture: nativeMapController is NULL"));
    }

    DEBUGLOG(LOG_SS_JNI, LOG_SEV_INFO, ("UI_HandleViewGesture: done(%d)", ret));

    return ret;
}

static void JNI_OnCreateSurface(JNIEnv* env, jobject thiz, jobject surface)
{
    MapSurfaceViewContext* context = reinterpret_cast<MapSurfaceViewContext*>(env->GetIntField(thiz, fields.context));
    if(context)
    {
        context->nativeHostView->SetWindow(ANativeWindow_fromSurface(env, surface));
    }
    context->nativeHostView->GetUIInterface()->UI_MapNativeViewCreated();
}

static void JNI_WindowResize(JNIEnv* env, jobject thiz, jint w, jint h)
{
    MapSurfaceViewContext* context = reinterpret_cast<MapSurfaceViewContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->nativeHostView)
    {
        context->nativeHostView->SetViewSize(w, h);
    }
}

static void JNI_OnUpdateSurface(JNIEnv* env, jobject thiz, jobject surface)
{
    MapSurfaceViewContext* context = reinterpret_cast<MapSurfaceViewContext*>(env->GetIntField(thiz, fields.context));
    if(context && surface)
    {
        context->nativeHostView->UpdateSurface(ANativeWindow_fromSurface(env, surface));
    }
}

static void JNI_OnDestroySurface(JNIEnv* env, jobject thiz)
{
    MapSurfaceViewContext* context = reinterpret_cast<MapSurfaceViewContext*>(env->GetIntField(thiz, fields.context));
    if(context)
    {
        context->nativeHostView->DestroySurface();
    }
}

static JNINativeMethod gMethods[] =
{
    { "onSurfaceCreated",               "(Ljava/lang/Object;)V",                      (void *) &JNI_OnCreateSurface       },
    { "onSurfaceDestroyed",             "()V",                                        (void *) &JNI_OnDestroySurface      },
    { "onSurfaceSizeChanged",           "(II)V",                                      (void *) &JNI_WindowResize          },
    { "handleViewGesture",              "(IIFFFFFFFFFFF)I",                           (void *) &JNI_HandleViewGesture     },
    { "onSurfaceUpdated",               "(Ljava/lang/Object;)V",                      (void *) &JNI_OnUpdateSurface       }
};

int register_nbui_native_hostview(JavaVM* vm, JNIEnv* env)
{
    jint result = JNI_ERR;
    if (env == NULL)
    {
        return result;
    }

    jclass clazz = env->FindClass("com/locationtoolkit/map3d/internal/nbui/NativeMapSurfaceController");
    if (clazz != NULL)
    {
        fields.clazz = static_cast<jclass>(env->NewGlobalRef(clazz));
        if (fields.clazz == NULL)
        {
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("cannot't create global ref for com/locationtoolkit/map3d/internal/nbui/NativeMapSurfaceController"));
            goto fail;
        }

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
    }
    else
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find com/locationtoolkit/map3d/internal/nbui/NativeMapSurfaceController"));
        goto fail;
    }

    fields.context = env->GetFieldID(fields.clazz, "mNativeSurfaceViewContext", "I");

    if (env->RegisterNatives(fields.clazz, gMethods, sizeof(gMethods) / sizeof(gMethods[0])) != JNI_OK)
    {
        goto fail;
    }

    fields.constructorID = env->GetMethodID(fields.clazz, "<init>", "(Landroid/content/Context;Lcom/locationtoolkit/map3d/internal/nbui/MapToolBar;)V");
    if (fields.constructorID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMapSurfaceController.constructorID"));
        goto fail;
    }

    fields.enableDebugInfoMethodID = env->GetMethodID(fields.clazz, "enableDebugInfo", "(Z)V");
    if (fields.enableDebugInfoMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMapSurfaceController.enableDebugInfo"));
        goto fail;
    }

    fields.setDebugInfoMethodID = env->GetMethodID(fields.clazz, "setDebugInfo", "(Ljava/lang/String;)V");
    if (fields.setDebugInfoMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMapSurfaceController.setDebugInfo"));
        goto fail;
    }

    fields.updateStateMethodID = env->GetMethodID(fields.clazz, "updateState", "(I)V");
    if (fields.updateStateMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMapSurfaceController.updateStateMethodID"));
        goto fail;
    }

    fields.updateTimestampLabelMethodID = env->GetMethodID(fields.clazz, "updateTimestampLabel", "(I)V");
    if (fields.updateTimestampLabelMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMapSurfaceController.updateTimestampLabelMethodID"));
        goto fail;
    }

    fields.updateButtonStateMethodID = env->GetMethodID(fields.clazz, "updateButtonState", "(Z)V");
    if (fields.updateButtonStateMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMapSurfaceController.updateButtonStateMethodID"));
        goto fail;
    }

    fields.addMapLengedColorID = env->GetMethodID(fields.clazz, "addMapLengedColor", "(II)V");
    if (fields.addMapLengedColorID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMapSurfaceController.addMapLengedColorID"));
        goto fail;
    }
    fields.addMapLengedBarID = env->GetMethodID(fields.clazz, "addMapLengedBar", "(Ljava/lang/String;)I");
    if (fields.addMapLengedBarID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMapSurfaceController.addMapLengedBarID"));
        goto fail;
    }
    fields.showMapLengedID = env->GetMethodID(fields.clazz, "showMapLenged", "()V");
    if (fields.showMapLengedID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMapSurfaceController.showMapLengedID"));
        goto fail;
    }
    fields.hideMapLengedID = env->GetMethodID(fields.clazz, "hideMapLenged", "()V");
    if (fields.hideMapLengedID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMapSurfaceController.hideMapLengedID"));
        goto fail;
    }

    fields.showTipID = env->GetMethodID(fields.clazz, "showTip", "(I)V");
    if (fields.showTipID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMapSurfaceController.showTipID"));
        goto fail;
    }

    clazz = env->FindClass("com/locationtoolkit/map3d/internal/MapSurfaceView");
    if (clazz != NULL)
    {
        fields.surfaceView_clazz = static_cast<jclass>(env->NewGlobalRef(clazz));
        if (fields.surfaceView_clazz == NULL)
        {
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("cannot't create global ref for com/locationtoolkit/map3d/internal/MapSurfaceView"));
            goto fail;
        }

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
    }
    else
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find com/locationtoolkit/map3d/internal/MapSurfaceView"));
        goto fail;
    }

    fields.surfaceViewConstructorID = env->GetMethodID(fields.surfaceView_clazz, "<init>", "(Landroid/content/Context;Lcom/locationtoolkit/map3d/internal/jni/NativeMapController;Lcom/locationtoolkit/map3d/MapController;Lcom/locationtoolkit/map3d/internal/nbui/NativeMapSurfaceController;)V");
    if (fields.surfaceViewConstructorID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find MapSurfaceView constructor"));
        goto fail;
    }

    clazz = env->FindClass("android/widget/FrameLayout");
    if (clazz != NULL)
    {
        fields.frameLayout_clazz = static_cast<jclass>(env->NewGlobalRef(clazz));
        if (fields.frameLayout_clazz == NULL)
        {
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("cannot't create global ref for android/widget/FrameLayout"));
            goto fail;
        }

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
    }
    else
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find android/widget/FrameLayout"));
        goto fail;
    }

    fields.addViewMethodID = env->GetMethodID(fields.frameLayout_clazz, "addView", "(Landroid/view/View;)V");
    if (fields.addViewMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find FrameLayout.addView"));
        goto fail;
    }

    cachedVM = vm;
    result = JNI_OK;

fail:
    return result;
}

int unregister_nbui_native_hostview(JNIEnv* env)
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

    if (fields.surfaceView_clazz)
    {
        env->DeleteGlobalRef(fields.surfaceView_clazz);
        fields.surfaceView_clazz = NULL;
    }

    if (fields.frameLayout_clazz)
    {
        env->DeleteGlobalRef(fields.frameLayout_clazz);
        fields.frameLayout_clazz = NULL;
    }
    return JNI_OK;
}
