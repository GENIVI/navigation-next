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

#include "MapViewController.h"
#include "nbtaskqueue.h"
#include "nbgmdevicelocation.h"
#include "nbgmandroidegl.h"
#include "nbgmandroidrendercontext.h"
#include <vector>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>

class InitMapViewTask: public Task
{
public:
    InitMapViewTask(MapViewController* thread):mWorkThread(thread)
    {

    }
    virtual ~InitMapViewTask()
    {
    }
public:
    virtual void Execute(void)
    {
        mWorkThread->InitializeMapView();
        delete this;
    }
private:
    MapViewController*        mWorkThread;
};

class FinalizeMapViewTask:public Task
{
public:
    FinalizeMapViewTask(MapViewController* thread):mWorkThread(thread)
    {

    }
    virtual ~FinalizeMapViewTask()
    {
    }
public:
    virtual void Execute(void)
    {
        mWorkThread->FinalizeMapView();
        delete this;
    }
private:
    MapViewController*        mWorkThread;
};

class CreateSurfaceTask: public Task
{
public:
    CreateSurfaceTask(MapViewController* thread, ANativeWindow *window):mWorkThread(thread), mWindow(window)
    {

    }
    virtual ~CreateSurfaceTask()
    {
    }
public:
    virtual void Execute(void)
    {
        mWorkThread->OnSurfaceCreated(mWindow);
        delete this;
    }
private:
    MapViewController*        mWorkThread;
    ANativeWindow*            mWindow;
};

class DestroySurfaceTask: public Task
{
public:
    DestroySurfaceTask(MapViewController* thread):mWorkThread(thread)
    {

    }
    virtual ~DestroySurfaceTask()
    {
    }
public:
    virtual void Execute(void)
    {
        mWorkThread->OnDestroySurface();
        delete this;
    }
private:
    MapViewController*        mWorkThread;
};

class ChangeSurfaceSizeTask: public Task
{
public:
    ChangeSurfaceSizeTask(MapViewController* thread):mWorkThread(thread)
    {

    }
    virtual ~ChangeSurfaceSizeTask()
    {
    }
public:
    virtual void Execute(void)
    {
        mWorkThread->OnChangeSurfaceSize();
        delete this;
    }
private:
    MapViewController*        mWorkThread;
};

MapViewController::MapViewController():
        mPalInstance(NULL),
        mInitializeEvent(NULL),
        mFinalizeEvent(NULL),
        mEixtEvent(NULL),
        mNBGMInstance(NULL),
        mMapView(NULL),
        mDefaultAvatar(NULL),
        mSurfaceWidth(-1),
        mSurfaceHeight(-1)
{
}

MapViewController::~MapViewController()
{
}


bool MapViewController::Initialize(JNIEnv* env, jobject surface, MapViewControllerConfiguration& controllerConfig)
{
    //Create nbpal
    PAL_Config palConfig = { 1, NULL };
    mPalInstance = PAL_Create(&palConfig);
    PAL_EventCreate(mPalInstance, &mInitializeEvent);
    PAL_EventCreate(mPalInstance, &mFinalizeEvent);
    PAL_EventCreate(mPalInstance, &mEixtEvent);

    //Create nbgm instance
    NBGM_Config config;
    nsl_memset(&config, 0, sizeof(NBGM_Config));
    config.nbmOffset = 0;
    config.useRoadBuildingVisibility = controllerConfig.enableLOD;
    config.dpi = controllerConfig.deviceDPI;
    config.pal = mPalInstance;
    config.workSpace = controllerConfig.workSpace.c_str();
    config.preferredLanguageCode = controllerConfig.preferredLanguageCode;
    config.relativeCameraDistance = 854.0f;  // camera height on zoom level 17, this value comes from Map Layering - Mapkit3D.xlsx.
    config.relativeZoomLevel = 17;
    config.metersPerPixelOnRelativeZoomLevel = 1.194329f;

    NBGM_Create(&config, &mNBGMInstance);
    NBGM_SetDebugLogSeverity(PAL_LogSeverityMajor);

    //Create map view
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    mEGL = new NBGMAndroidEGL(window);
    mRenderTaskQueue.reset(new WorkerTaskQueue(mPalInstance, shared_ptr<std::string>(new std::string("RenderTaskQueue"))));

    NBGM_MapViewConfig viewConfig;
    viewConfig.renderTaskQueue = mRenderTaskQueue;
    viewConfig.renderContext = shared_ptr<NBGM_AndroidRenderContext>(new NBGM_AndroidRenderContext(mEGL));
    viewConfig.drawSky = controllerConfig.drawSky;
    viewConfig.drawFlag = controllerConfig.drawFlag;
    viewConfig.drawAvatar = controllerConfig.drawAvatar;
    viewConfig.renderSystemType = controllerConfig.renderSystemType;
    viewConfig.renderInterval = controllerConfig.renderInterval;
    NBGM_CreateMapView(mNBGMInstance, &viewConfig, &mMapView);

    //Create render task queue to initialize map view
    mRenderTaskQueue->AddTask(new InitMapViewTask(this));

    PAL_EventWaitForEvent(mInitializeEvent);

    return true;
}

bool MapViewController::InitializeMapView()
{
    mMapView->Initialize();

    NBGM_Location64 avatarLocation;
    memset(&avatarLocation, 0, sizeof(avatarLocation));
    avatarLocation.position.x = 2.1168935461902f;
    avatarLocation.position.y =  0.58291271561759f;
    mDefaultAvatar = mMapView->CreateDeviceLocation("default_avatar");
    mDefaultAvatar->SetLocation(avatarLocation);
    mDefaultAvatar->SetState(NBGM_AS_MAP_FOLLOW_ME);

    mMapView->Invalidate();
    PAL_EventSet(mInitializeEvent);
    return true;
}

void MapViewController::Finalize()
{
    mRenderTaskQueue->AddTask(new FinalizeMapViewTask(this));
    PAL_EventWaitForEvent(mFinalizeEvent);
    delete mMapView;
    delete mEGL;

    NBGM_Destroy(mNBGMInstance);
    PAL_EventDestroy(mFinalizeEvent);
    PAL_EventDestroy(mInitializeEvent);
    PAL_Destroy(mPalInstance);

}

void MapViewController::FinalizeMapView()
{
    mMapView->Finalize();
    PAL_EventSet(mFinalizeEvent);
}

void MapViewController::Invalidate()
{
	mMapView->Invalidate();
}

void MapViewController::SetBackground(bool background)
{
	mMapView->SetBackground(background);
}

void MapViewController::SetViewCenter(double mercatorX, double mercatorY)
{
    NBGM_Location64 avatarLocation;
    memset(&avatarLocation, 0, sizeof(avatarLocation));
    avatarLocation.position.x = (float)mercatorX;
    avatarLocation.position.y =  (float)mercatorY;
    mDefaultAvatar->SetLocation(avatarLocation);

	mMapView->SetViewCenter(mercatorX, mercatorY);
}

void MapViewController::SetViewPointDistance(float distance)
{
	mMapView->SetViewPointDistance(distance);
}

void MapViewController::SetViewSize(int x, int y, int width, int height)
{
	mMapView->SetViewSize(x, y, width, height);
}

void MapViewController::SetSurfaceSize(int width, int height)
{
    mSurfaceWidth = width;
    mSurfaceHeight = height;
    mRenderTaskQueue->AddTask(new ChangeSurfaceSizeTask(this));
}

void MapViewController::SetPerspective(float fov, float aspect)
{
	mMapView->SetPerspective(fov, aspect);
}

void MapViewController::SurfaceCreated(ANativeWindow *window)
{
    mRenderTaskQueue->AddTask(new CreateSurfaceTask(this, window));
}

void MapViewController::OnSurfaceCreated(ANativeWindow *window)
{
    mEGL->CreateSurface(window);
}

void MapViewController::SurfaceDestroyed()
{
    mRenderTaskQueue->AddTask(new DestroySurfaceTask(this));
}

void MapViewController::OnDestroySurface()
{
	mEGL->DestroySurface();
}

void MapViewController::OnChangeSurfaceSize()
{
    if((mSurfaceWidth != -1 && mSurfaceHeight != -1))
    {
        mEGL->Swap();
        Invalidate();
    }
}

void MapViewController::OnTouchEvent(float screenX, float screenY)
{
	mMapView->OnTouchEvent(screenX, screenY);
}

void MapViewController::OnPaning(float screenX, float screenY)
{
	mMapView->OnPaning(screenX, screenY);
}

int MapViewController::LoadCommonMaterial(std::string materialName, std::string filePath)
{
	return mMapView->LoadCommonMaterial(materialName, filePath);
}

int MapViewController::SetCurrentCommonMaterial(std::string materialName)
{
	return mMapView->SetCurrentCommonMaterial(materialName);
}

int MapViewController::LoadNBMTile(std::string nbmName, int baseDrawOrder, int labelDrawOrder, std::string filePath)
{
	return mMapView->LoadNBMTile(nbmName, baseDrawOrder, labelDrawOrder, filePath);
}

void MapViewController::UnLoadTile(std::string nbmName)
{
	return mMapView->UnLoadTile(nbmName);

}
