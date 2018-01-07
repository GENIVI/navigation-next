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

#include "lightweightmapview.h"
#include "nbgmbb10rendercontext.h"
#include "nbtaskqueue.h"
#include "bbutil.h"
#include "nbgmgesturehandler.h"
#include "nbgmdevicelocation.h"
#include "tilenameparser.h"
#include "paltimer.h"
#include <bb/cascades/PinchEvent>
#include <bb/cascades/pickers/FilePicker>
#include "nbrelog.h"
#include "nbremath.h"
#include <bb/cascades/Page>
#include <bb/cascades/ActionItem>
#include <bb/cascades/ForeignWindowControl>
#include <bb/cascades/NavigationPaneProperties>
#include <bb/cascades/AbsoluteLayoutProperties>

using namespace bb::cascades::pickers;
using namespace bb::cascades;

static const char* NBGM_WORK_SPACE = "app/native/assets/nbgmresource/";


LightWeightMapView::LightWeightMapView():
        mScreenCtx(NULL),
        mGestureHandler(NULL),
        mEGL(NULL),
        mPalInstance(NULL),
        mInitializeEvent(NULL),
        mFinalizeEvent(NULL),
        mExitEvent(NULL),
        mNBGMInstance(NULL),
        mMapView(NULL),
        mRenderTaskQueue(NULL)
{
    mInitialized = false;
}

LightWeightMapView::~LightWeightMapView()
{
}

void LightWeightMapView::onUnloadAllFile()
{
    if(mInitialized)
    {
        for(std::list<std::string>::iterator i = mLoadTile.begin(); i != mLoadTile.end(); ++i)
        {
            mMapView->UnLoadTile(i->c_str());
        }
        mLoadTile.clear();

    }
}

void LightWeightMapView::onFileSelected(const QStringList& files)
{
    if(mInitialized)
    {
        for (int i = 0; i < files.size(); ++i)
        {
            QByteArray byteArray = files.at(i).toUtf8();
            const char* tilename = byteArray.data();
            NBRE_DebugLog(PAL_LogSeverityInfo, "onFileSelected: %s\n", tilename);

            NBMTileInfo tileInfo;
            nsl_memset(&tileInfo, 0, sizeof(NBMTileInfo));
            if(GetNBMTileInfo(tilename, tileInfo) == PAL_Ok)
            {
                mMapView->SetViewCenter(TileToMercatorX(tileInfo.x, 0.5, tileInfo.z), TileToMercatorY(tileInfo.y, 0.5, tileInfo.z));
                mMapView->LoadNBMTile(tileInfo.name, tileInfo.layerType, tileInfo.labelType, tilename);
                mLoadTile.push_back(tileInfo.name);
            }
        }
        mMapView->Invalidate();

    }
}


bool LightWeightMapView::Initialize(Container* container, int x, int y, int w, int h)
{
    mWidth = w;
    mHeight = h;

    ForeignWindowControl* foreignWindowControl = ForeignWindowControl::create().preferredSize(w, h);
    foreignWindowControl->setLayoutProperties(AbsoluteLayoutProperties::create().x(x).y(y));
    foreignWindowControl->setVisible(true);
    container->add(foreignWindowControl);


    // All gestures in this recipe are handled by these gesture handlers. The gesture
    // handlers belong to the UI element they are in, in this case the RecipeContainer.
    // So events occurring inside that element can be captured by them.
    DoubleTapHandler* doubleTapHandler = DoubleTapHandler::create().onDoubleTapped(this,
            SLOT(onDoubleTap(bb::cascades::DoubleTapEvent*)));
    PinchHandler *pinchHandler = PinchHandler::create().onPinch(this,
            SLOT(onPinchStart(bb::cascades::PinchEvent*)),
            SLOT(onPinchUpdate(bb::cascades::PinchEvent*)),
            SLOT(onPinchEnd(bb::cascades::PinchEvent*)), SLOT(onPinchCancel()));

    foreignWindowControl->addGestureHandler(doubleTapHandler);
    foreignWindowControl->addGestureHandler(pinchHandler);


    QObject::connect(foreignWindowControl, SIGNAL(touch(bb::cascades::TouchEvent*)), this, SLOT(onTouchEvent(bb::cascades::TouchEvent*)));



    // We must create a context before you create a window.
    if (screen_create_context(&mScreenCtx, SCREEN_APPLICATION_CONTEXT) != 0) {
        return false;
    }

    //Create nbpal
    PAL_Config pc;
    pc.multiThreaded = TRUE;
    mPalInstance = PAL_Create(&pc);
    PAL_EventCreate(mPalInstance, &mInitializeEvent);
    PAL_EventCreate(mPalInstance, &mFinalizeEvent);
    PAL_EventCreate(mPalInstance, &mExitEvent);

    //Create nbgm instance
    NBGM_Config config;
    nsl_memset(&config, 0, sizeof(NBGM_Config));
    config.dpi = 373.f;
    config.pal = mPalInstance;
    config.workSpace = NBGM_WORK_SPACE;
    config.nbmOffset = 0;
    config.useRoadBuildingVisibility = TRUE;
    config.preferredLanguageCode = 0;
    config.relativeCameraDistance = 854.0f;  // camera height on zoom level 17, this value comes from Map Layering - Mapkit3D.xlsx.
    config.relativeZoomLevel = 17;
    config.metersPerPixelOnRelativeZoomLevel = 1.194329f;

    NBGM_Create(&config, &mNBGMInstance);
    NBGM_SetDebugLogSeverity(PAL_LogSeverityDebug);

    //Create map view
    mEGL = new NBGM_Egl(mScreenCtx, foreignWindowControl->windowGroup(), x, y, w, h);
    mRenderTaskQueue = new WorkerTaskQueue(mPalInstance, shared_ptr<std::string>(new std::string("RenderTaskQueue")));
    NBGM_MapViewConfig viewConfig;
    viewConfig.drawSky = FALSE;
    viewConfig.drawFlag = FALSE;
    viewConfig.drawAvatar = TRUE;
    viewConfig.renderTaskQueue = shared_ptr<TaskQueue>((WorkerTaskQueue*)mRenderTaskQueue);
    viewConfig.renderContext = shared_ptr<NBGM_BB10RenderContext>(new NBGM_BB10RenderContext(*mEGL));
    viewConfig.renderSystemType = NBGM_RS_GLES20;
    NBGM_CreateMapView(mNBGMInstance, &viewConfig, &mMapView);

    //Create render task queue to initialize map view
    mRenderTaskQueue->AddTask(new InitMapViewTask(this));

    PAL_EventWaitForEvent(mInitializeEvent);
    //create timer task queue

    mGestureHandler = new NBGM_GestureHandler(*mMapView);

    mInitialized = true;
    return true;
}

bool LightWeightMapView::InitializeMapView()
{
    mMapView->Initialize();

    mMapView->SetViewCenter(-2.05553412f, 0.6227962f);
    mMapView->SetViewPointDistance(13600);
    mMapView->SetRotateAngle(0);

//    NBGM_Location64 avatarLocation;
//    memset(&avatarLocation, 0, sizeof(avatarLocation));
//    avatarLocation.position.x = -2.05553412f;
//    avatarLocation.position.y = 0.6227962f;
//    NBGM_DeviceLocation* avatar = mMapView->CreateDeviceLocation("default_avatar");
//    avatar->SetLocation(avatarLocation);
//    avatar->SetState(NBGM_AS_MAP_FOLLOW_ME);
//    avatar->SetMode(NBGM_AM_MAP_FOLLOW_ME);
//    mMapView->ReleaseDeviceLocation(avatar);

    mMapView->SetViewSize(0, 0, mWidth, mHeight);
    mMapView->SetPerspective(55.f, mWidth/(float)mHeight);

    mMapView->LoadCommonMaterial("data_mat", "app/native/assets/nbgmresource/NBM/MAT/T_DMAT_1_NBM_23");
    mMapView->SetCurrentCommonMaterial("data_mat");


    PAL_EventSet(mInitializeEvent);
    return true;
}

void LightWeightMapView::Finalize()
{
    mRenderTaskQueue->AddTask(new FinalizeMapViewTask(this));
    PAL_EventWaitForEvent(mFinalizeEvent);
    delete mMapView;
    delete mEGL;

    NBGM_Destroy(mNBGMInstance);
    PAL_EventDestroy(mFinalizeEvent);
    PAL_EventDestroy(mInitializeEvent);
    PAL_Destroy(mPalInstance);

    delete mGestureHandler;
    screen_destroy_context(mScreenCtx);
    PAL_EventSet(mExitEvent);
}

void LightWeightMapView::FinalizeMapView()
{
    mMapView->Finalize();
    PAL_EventSet(mFinalizeEvent);
}

void LightWeightMapView::onTouchEvent(bb::cascades::TouchEvent* event)
{
    mGestureHandler->OnScreenEvent(event);

}

void LightWeightMapView::onDoubleTap(bb::cascades::DoubleTapEvent*)
{
    mGestureHandler->onDoubleTap();

}

void LightWeightMapView::onPinchStart(bb::cascades::PinchEvent* event)
{
    mGestureHandler->onPinchStart(event);
}

void LightWeightMapView::onPinchUpdate(bb::cascades::PinchEvent* event)
{
    mGestureHandler->onPinchUpdate(event);
}

void LightWeightMapView::onPinchEnd(bb::cascades::PinchEvent* event)
{
    mGestureHandler->onPinchEnd(event);

}

void LightWeightMapView::onPinchCancel()
{
    mGestureHandler->onPinchCancel();

}


void LightWeightMapView::onBackClickedAction()
{
//    Finalize();
//    delete this;
}
