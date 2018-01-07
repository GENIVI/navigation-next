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

#include "nbgmuitest.h"
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
#include <vector>
#include "tilemanager.h"
#include "palclock.h"
#include "qtimer.h"

using namespace bb::cascades::pickers;
using namespace bb::cascades;

static const char* NBGM_WORK_SPACE = "app/native/assets/nbgmresource/";

class InitMapViewTask: public Task
{
public:
    InitMapViewTask(NBGMUITest* thread):mWorkThread(thread)
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
    NBGMUITest*        mWorkThread;
};

class FinalizeMapViewTask:public Task
{
public:
    FinalizeMapViewTask(NBGMUITest* thread):mWorkThread(thread)
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
    NBGMUITest*        mWorkThread;
};

NBGMUITest::NBGMUITest():
        mNavigationPane(NULL),
        mScreenCtx(NULL),
        mGestureHandler(NULL),
        mEGL(NULL),
        mPalInstance(NULL),
        mInitializeEvent(NULL),
        mFinalizeEvent(NULL),
        mEixtEvent(NULL),
        mNBGMInstance(NULL),
        mMapView(NULL),
        mRenderTaskQueue(NULL),
        mTiltFlag(false),
        mTouchPointY(0),
        mTileManager(NULL),
        mResetMapCenter(false)
{
}

NBGMUITest::~NBGMUITest()
{
}


void NBGMUITest::onScreenshot()
{
//    mMapView->AsyncSnapScreenshot(&mSnapScreenshotCallback);
}

bool NBGMUITest::Initialize(NavigationPane* navPane)
{
    mNavigationPane = navPane;

    Page* page = Page::create();
    ActionItem* openNBMFileAction = ActionItem::create().title("Open NBM file");
    page->addAction(openNBMFileAction);
    page->connect(openNBMFileAction, SIGNAL(triggered()), this, SLOT(onOpenNBMFile()));

    ActionItem* unloadAction = ActionItem::create().title("Unload All File");
    page->addAction(unloadAction);
    page->connect(unloadAction, SIGNAL(triggered()), this, SLOT(onUnloadAllFile()));

    ActionItem* screenshotAction = ActionItem::create().title("Screenshot");
    page->addAction(screenshotAction);
    page->connect(screenshotAction, SIGNAL(triggered()), this, SLOT(onScreenshot()));

    mNavigationPane->push(page);

    Container* container = Container::create();
    page->setContent(container);
    ForeignWindowControl* foreignWindowControl = ForeignWindowControl::create().preferredSize(768, 1280);
    foreignWindowControl->setVisible(true);
    container->add(foreignWindowControl);

    DoubleTapHandler* doubleTapHandler = DoubleTapHandler::create().onDoubleTapped(this,
            SLOT(onDoubleTap(bb::cascades::DoubleTapEvent*)));
    PinchHandler *pinchHandler = PinchHandler::create().onPinch(this,
            SLOT(onPinchStart(bb::cascades::PinchEvent*)),
            SLOT(onPinchUpdate(bb::cascades::PinchEvent*)),
            SLOT(onPinchEnd(bb::cascades::PinchEvent*)), SLOT(onPinchCancel()));

    foreignWindowControl->addGestureHandler(doubleTapHandler);
    foreignWindowControl->addGestureHandler(pinchHandler);
    QObject::connect(foreignWindowControl, SIGNAL(touch(bb::cascades::TouchEvent*)), this, SLOT(onTouchEvent(bb::cascades::TouchEvent*)));

    bb::cascades::ActionItem* backAction = ActionItem::create().title("Back");
    page->setPaneProperties(NavigationPaneProperties::create().backButton(backAction));

    mTimer = new QTimer();
    QObject::connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
    mTimer->start(70);

    QObject::connect(backAction, SIGNAL(triggered()), this, SLOT(onBackClickedAction()));
    mTileManager = new TileManager();

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
    PAL_EventCreate(mPalInstance, &mEixtEvent);

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
    NBGM_SetDebugLogSeverity(PAL_LogSeverityMajor);

    //Create map view
    mEGL = new NBGM_Egl(mScreenCtx, foreignWindowControl->windowGroup(), 0, 0, 768, 1280);
    mRenderTaskQueue = new WorkerTaskQueue(mPalInstance, shared_ptr<std::string>(new std::string("RenderTaskQueue")));
    NBGM_MapViewConfig viewConfig;
    viewConfig.drawSky = TRUE;
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

    return true;
}

bool NBGMUITest::InitializeMapView()
{
    mMapView->Initialize();

    mMapView->SetViewPointDistance(13600/2);
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

    int surface_width, surface_height;
    mEGL->get_window_size(&surface_width, &surface_height);
    mMapView->SetViewSize(0, 0, surface_width, surface_height);
    mMapView->SetPerspective(55.f, surface_width/(float)surface_height);

    mMapView->LoadCommonMaterial("data_mat", "app/native/assets/nbgmresource/NBM/MAT/T_DMAT_1_NBM_23");
    mMapView->SetCurrentCommonMaterial("data_mat");

    mMapView->Invalidate();
    PAL_EventSet(mInitializeEvent);
    return true;
}

void NBGMUITest::Finalize()
{
	mTimer->stop();
    mRenderTaskQueue->AddTask(new FinalizeMapViewTask(this));
    PAL_EventWaitForEvent(mFinalizeEvent);
    delete mMapView;
    delete mEGL;

    delete mTimer;

    NBGM_Destroy(mNBGMInstance);
    PAL_EventDestroy(mFinalizeEvent);
    PAL_EventDestroy(mInitializeEvent);
    PAL_Destroy(mPalInstance);

    delete mGestureHandler;
    screen_destroy_context(mScreenCtx);
    PAL_EventSet(mEixtEvent);

    delete mTileManager;
}

void NBGMUITest::FinalizeMapView()
{
    mMapView->Finalize();
    PAL_EventSet(mFinalizeEvent);
}

void NBGMUITest::onOpenNBMFile()
{
    FilePicker* filePicker = new FilePicker();
    filePicker->setType(FileType::Other);
    filePicker->setTitle("SelectNBM Folder");
    filePicker->setMode(FilePickerMode::SaverMultiple);
    filePicker->open();

    // Connect the fileSelected() signal with the slot.
    QObject::connect(filePicker,
        SIGNAL(fileSelected(const QStringList&)),
        this,
        SLOT(onFileSelected(const QStringList&)));
}

void NBGMUITest::onUnloadAllFile()
{
    mMapView->UnLoadAllTiles();
    mLoadTile.clear();
}

void NBGMUITest::onTimer()
{
	if(mTileInvalide)
	{
		UpdateTiles();
		mTileInvalide = false;
	}
}

void NBGMUITest::onFileSelected(const QStringList& files)
{
	onUnloadAllFile();
	mTileManager->Reset();
    for (int i = 0; i < files.size(); ++i)
    {
        QByteArray byteArray = files.at(i).toUtf8();
        const char* tilename = byteArray.data();
        mTileManager->AddFolder(tilename);
    }
    mResetMapCenter = true;
    UpdateTiles();
    mMapView->Invalidate();
}

void NBGMUITest::UpdateTiles()
{
    if(mTileManager == NULL)
    {
        return;
    }
    NBMTileInfo info;
    if(mResetMapCenter)
    {
        if(mTileManager->GetTile(0, info))
        {
            mMapView->SetViewCenter(TileToMercatorX(info.x, 0.5, info.z), TileToMercatorY(info.y, 0.5, info.z));
        }
        mResetMapCenter = false;
    }
    else
    {
        double mx=0;
        double my=0;
        mMapView->GetViewCenter(mx, my);
        info.z = 15;
        info.x = MercatorToTileX(mx, info.z);
        info.y = MercatorToTileY(my, info.z);
    }

    std::vector<NBMTileInfo> currTileList;

    mTileManager->GetTile(info.x, info.y, info.z, currTileList);

    //Unload unnecessary tiles
    for(std::list<std::string>::iterator loadedTile = mLoadTile.begin();
    		loadedTile != mLoadTile.end();
    		)
    {
		bool found = false;
		for(std::vector<NBMTileInfo>::iterator newIndex=currTileList.begin();
				newIndex!=currTileList.end();
				++newIndex)
		{
			if(*loadedTile == newIndex->name)
			{
				currTileList.erase(newIndex);
				found = true;
				break;
			}
		}
		if(!found)
		{
			mMapView->UnLoadTile(*loadedTile);
			loadedTile = mLoadTile.erase(loadedTile);
		}
		else
		{
			loadedTile++;
		}
    }

	//load new
    for(unsigned int i = 0; i<currTileList.size(); ++i)
    {
        mMapView->LoadNBMTile(currTileList[i].name,
        		currTileList[i].layerType,
        		currTileList[i].labelType,
        		currTileList[i].path);
        mLoadTile.push_back(currTileList[i].name);
    }
}

void NBGMUITest::onTouchEvent(bb::cascades::TouchEvent* event)
{
    mGestureHandler->OnScreenEvent(event);
    mTileInvalide = true;
}

void NBGMUITest::onDoubleTap(bb::cascades::DoubleTapEvent*)
{
    mGestureHandler->onDoubleTap();
    mTileInvalide = true;
}

void NBGMUITest::onPinchStart(bb::cascades::PinchEvent* event)
{
    mGestureHandler->onPinchStart(event);
    mTileInvalide = true;
}

void NBGMUITest::onPinchUpdate(bb::cascades::PinchEvent* event)
{
    mGestureHandler->onPinchUpdate(event);
    mTileInvalide = true;
}

void NBGMUITest::onPinchEnd(bb::cascades::PinchEvent* event)
{
    mGestureHandler->onPinchEnd(event);
    mTileInvalide = true;
}

void NBGMUITest::onPinchCancel()
{
    mGestureHandler->onPinchCancel();
    mTileInvalide = true;
}

void NBGMUITest::onBackClickedAction()
{
    Finalize();
    mNavigationPane->pop();
    delete this;
}
