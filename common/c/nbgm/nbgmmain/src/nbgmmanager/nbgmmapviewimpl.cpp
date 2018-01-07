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

/*--------------------------------------------------------------------------

(C) Copyright 2012 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "nbgmmapviewimpl.h"
#include "nbrerenderengine.h"
#include "nbredefaultspatialdividemanager.h"
#include "nbrespatialdividemanagerdebug.h"
#include "nbreentity.h"
#include "nbgmmapmaterialmanager.h"
#include "nbrelog.h"
#include "nbrebillboardnode.h"
#include "nbrefilestream.h"
#include "nbgmprotected.h"
#include "nbgmlayoutmanager.h"
#include "nbgmlabellayer.h"
#include "nbgmpoilayer.h"
#include "nbgmtilemaplayer.h"
#include "nbrespatialdividemanagerdebug.h"
#include "nbretypeconvert.h"
#include "nbretransformation.h"
#include "nbgmavatar.h"
#include "palclock.h"
#include "nbrelinearspatialdividemanager.h"
#include "nbgmbuildmodelcontext.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmmapviewdatacache.h"
#include "nbgmconst.h"
#include "nbreruntimeinfo.h"
#include "nbgmsky.h"
#include "nbgm.h"
#include "nbgmnaviconlayer.h"
#include "nbgmflag.h"
#include "nbgmgridentity.h"
#include "nbgmnavvectorroutelayer.h"
#include "nbgmtransparentoverlay.h"
#include "nbgmglowoverlay.h"
#include "nbgmnavecmroutelayer.h"
#include "nbgmconfig.h"
#include "nbgmanimation.h"
#include "nbgmrendercontext.h"
#include "nbgmtileanimationlayerimpl.h"
#include "nbgmoptionallayerimpl.h"
#include "nbreutility.h"
#include "nbrenoneaxisalignedbox3.h"
#include "nbgmtaskqueue.h"
#include "paltestlog.h"
#include "nbrefilelogger.h"
#include "nbgmcompass.h"
#include "nbgmcircleimpl.h"
#include "nbgmrect2dimpl.h"
#include "nbgmmapmaterialutility.h"
#include "nbgmicon.h"

NBGM_MapViewImpl::NBGM_MapViewImpl(const NBGM_MapViewImplConfig& config, NBRE_RenderEngine* renderEngine):
    mInitialized(FALSE),
    mConfig(config),
    mCamera(NBRE_NEW NBRE_Camera()),
    mTransUtil(NULL),
    mIsLayoutInvalidate(TRUE),
    mPickingRadius(20.0f),
    mSurface(NULL),
    mSubView(NULL),
    mSceneManager(NULL),
    mTileMapLayer(NULL),
    mLabelLayer(NULL),
    mPoiLayer(NULL),
    mScreenOverlay(NULL),
    mBillboardSet(NULL),
    mAvatar(NULL),
    mSky(NULL),
    mLabelTransparentSky(NULL),
    mIsTouched(FALSE),
    mMapCenter(NBRE_NEW NBRE_Node()),
    mHorizonDistance(NBRE_Math::Infinity),
    mSkyHeightOnViewport(0.0f),
    mTiltAngle(0.0),
    mRotateAngle(0.0),
    mFontScale(1.0f),
    mEnableProfiling(FALSE), // Disable by default
    mProfiler(LAYER_STRIDE, DO_MAIN),
    mStartFlag(NULL),
    mEndFlag(NULL),
    mNavIconLayer(NULL),
    mEnableDebugDraw(FALSE),
    mGridEntity(NULL),
    mNavVecRouteLayer(NULL),
    mNavEcmRouteLayer(NULL),
    mPathArrowLayer(NULL),
    mLastUpdateMs(0),
    mSyncRenderDelay(0),
    mCurrentViewPointDistance(0.0f),
    mTransparentLayer(NULL),
    mGlowLayer(NULL),
    mIsInvalidate(FALSE),
    mBackground(FALSE),
    mSizeValid(FALSE),
    mLastSkyHeight(0),
    mTileIdCache(NULL),
    mLoadingThreadFinalizeEvent(NULL),
    mScreenCamera(NBRE_NEW NBRE_Camera()),
    mCurrentZoomLevel(INVALID_CHAR_INDEX),
    mChangeZoomLevel(FALSE),
    mCustomObjectesContext(mNBGMContext),
    mRenderExpectedTime(0),
    mEnableRenderLog(FALSE),
    mLayoutTaskCount(0),
    mWaitingLayoutFinished(FALSE),
    mRenderThreadHandle(NULL),
    mTimerCallback(NULL),
    mTimerUserData(NULL),
    mCompass(NULL),
    mOverlayManager(NULL),
    mBackgoundDrawOrder(0, DO_BACKGROUND, 0),
    mSkyDrawOrder(0, DO_SKIES, 0),
    mTransparentWallDrawOrder(255, DO_TRANSPARENT_WALL, 0),
    mBuildingDrawOrder(255, DO_BUILDINGOVERLAY, 0),
    mVectorRouteDrawOrder(255, DO_NAV_VECTOR_ROUTE, 0),
    mECMRouteDrawOrder(255, DO_NAV_ECM_ROUTE, 0),
    mECMArrowDrawOrder(255, DO_NAV_ECM_ARROW, 0),
    mTransparentOverlayDrawOrder(255, DO_TRANSPARENTOVERLAY, 0),
    mGlowOverlayDrawOrder(143, 255, 255),//hardcoded value a little smaller than buildings
    mLabelTransparentWallDrawOrder(255, DO_LABEL_TRANSPARENT_WALL, 0),
    mLabelTransparentSkyDrawOrder(255, DO_LABEL_TRANSPARENT_WALL, 0),
    mLabelDrawOrder(255, DO_LABLE, 0),
    mNavLabelDrawOrder(255, DO_NAV_LABLE, 0),
    mNavElementDrawOrder(255, DO_NAV_ELEMENTS, 0),
    mAvatarDrawOrder(NBM_TOP_LAYER, 0, 0),
    mCompassDrawOrder(255, DO_COMPASS, 0),
    mCustomObj2DDrawOrder(NBM_OVERLAY_LAYER, DO_TOPSETOVERLAY, 0)
{
    mNBGMContext.renderingEngine = renderEngine;
    mNBGMContext.buildModelContext = NBRE_NEW NBGM_BuildModelContext();
    mNBGMContext.SetModelScaleFactor(config.modelScaleFactor);
    mNBGMContext.renderContext = config.resourceManager->GetConfig().renderContext;

    uint32 now = PAL_ClockGetTimeMs();
    NBRE_String renderingLog = NBRE_Utility::FormatString("logs/NBGM_RenderingLog_%d.csv", now);
    mNBGMContext.fileLogger = NBRE_NEW NBRE_FileLogger(mNBGMContext.renderingEngine->Context().mPalInstance, renderingLog.c_str());

}

NBGM_MapViewImpl::~NBGM_MapViewImpl()
{
}

void NBGM_MapViewImpl::InitializeEnvironment()
{
    if (mInitialized)
    {
        return;
    }


    mTileIdCache = NBRE_NEW NBGM_MapViewDataCache(100);

    mOverlayManager = NBRE_NEW NBRE_DefaultOverlayManager<DrawOrderStruct>(mNBGMContext.renderingEngine->Context(), &mNBGMContext.maxOverlayId);
    DrawOrderStruct roadBgDrawOrder(255, DO_TRANSPARENT_ROAD_BG_OVERLAY, 0);
    mTransparentLayer = NBRE_NEW NBGM_TransparentLayer(mNBGMContext,
        mBuildingDrawOrder,
        roadBgDrawOrder,
        mTransparentOverlayDrawOrder,
        mTransparentWallDrawOrder,
        *mOverlayManager);

    mGlowLayer = NBRE_NEW NBGM_GlowLayer(mNBGMContext,
        mBuildingDrawOrder,
        mGlowOverlayDrawOrder,
        *mOverlayManager);

    mSceneManager = NBRE_NEW NBRE_SceneManager();
    NBRE_SceneManagerPtr sceneManagerPtr(mSceneManager);
    NBRE_LinearSpatialDivideManager* spatialDivideManager = NBRE_NEW NBRE_LinearSpatialDivideManager();
    mSceneManager->SetSpatialDivideStrategy(spatialDivideManager);

    mCamera->LookAt(NBRE_Vector3f(0.0f, 0.0f, mNBGMContext.WorldToModel(1.0f)),
                    NBRE_Vector3f(0.0f, 0.0f, 0.0f),
                    NBRE_Vector3f(0.0f, 1.0f, 0));
    mSceneManager->RootSceneNode()->AddChild(mMapCenter);
    mMapCenter->AddChild(mCamera);

    mSurface = mNBGMContext.renderingEngine->Context().mSurfaceManager->CreateSurface();
    mNBGMContext.renderingEngine->SetActiveRenderSurface(mSurface);

    mSubView = &mSurface->CreateSubView(0, 0, 0, 0);
    mSubView->Viewport().SetCamera(mCamera);
    mSubView->SetSceneManager(sceneManagerPtr);
    mSubView->SetOverlayManager(mOverlayManager);

    mTransUtil = NBRE_NEW NBRE_TransformUtil(&mSubView->Viewport());
    mNBGMContext.transUtil = mTransUtil;

    if(mRenderThread.get() == NULL)
    {
        mRenderThread = shared_ptr<NBGM_TaskQueue>(NBRE_NEW NBGM_TaskQueue(mConfig.viewConfig.renderTaskQueue, mNBGMContext.renderingEngine->Context().mPalInstance));
    }

    PAL_WorkerTaskQueueGetHandler(mNBGMContext.renderingEngine->Context().mPalInstance,
                                  "nbgmrendering",
                                  &mRenderThreadHandle);

    mNBGMContext.layoutManager = NBRE_NEW NBGM_LayoutManager(mNBGMContext, &mSubView->Viewport(), 10);
    mNBGMContext.layoutManager->SetMaxLabels(mConfig.maxLabels);
    mNBGMContext.layoutManager->SetMaxAreaBuildingLabels(mConfig.maxAreaBuildingLabels);

    if(mTransparentLayer)
    {
        mTransparentLayer->Initialize(sceneManagerPtr, mCamera, mSubView, mProfiler, mConfig.viewConfig.drawSky);
    }

    mGlowLayer->Initialize(sceneManagerPtr, mCamera, mSubView, mProfiler, mConfig.viewConfig.drawSky);
    
    // disable ao effect by default
    NBGM_HBAOParameters hbaoParams;
    hbaoParams.enable = FALSE;
    mTransparentLayer->SetHBAOParameters(hbaoParams);

    // disable glow effect by default
    NBGM_GlowParameters glowParams;
    glowParams.enable = FALSE;
    mGlowLayer->SetGlowParameters(glowParams);

    InitDrawOrder();
    CreateLayer();

    if(mConfig.viewConfig.drawAvatar)
    {
        mAvatar = NBRE_NEW NBGM_Avatar(mNBGMContext,
            mSceneManager,
            mSubView,
            *mOverlayManager,
            mAvatarDrawOrder,
            mGlowLayer->BuildingOverlayId(),
            NBGM_AM_MAP_FOLLOW_ME,
            mConfig.avatarSizeFactor,
            mConfig.referenceAvatarHeight,
            NBRE_Vector3f(0, 0, 0),
            "default_avatar");
        mAvatar->AddToSence();
    }

    if(mConfig.viewConfig.drawSky)
    {
        int32 overlayId =  mOverlayManager->FindId(mSkyDrawOrder);
        mSky = NBRE_NEW NBGM_Sky(mNBGMContext, overlayId, FALSE, FALSE, mSubView);
        mSky->AddToSence();
        overlayId = mOverlayManager->FindId(mLabelTransparentSkyDrawOrder);
        mLabelTransparentSky = NBRE_NEW NBGM_Sky(mNBGMContext, overlayId, TRUE, FALSE, mSubView);
        mLabelTransparentSky->AddToSence();
        mOverlayManager->FindOverlay(overlayId)->SetClearFlag(NBRE_CLF_CLEAR_DEPTH);
    }
    else
    {
        mOverlayManager->FindOverlay(mOverlayManager->FindId(mLabelDrawOrder))->SetClearFlag(NBRE_CLF_CLEAR_DEPTH);
    }

    if (mConfig.viewConfig.drawFlag)
    {
        int32 overlayId = mOverlayManager->FindId(mNavElementDrawOrder);
        mStartFlag = NBRE_NEW NBGM_Flag(mNBGMContext, mSceneManager, overlayId, FALSE);
        mEndFlag = NBRE_NEW NBGM_Flag(mNBGMContext, mSceneManager, overlayId, TRUE);
        mStartFlag->AddToSence();
        mEndFlag->AddToSence();
    }

    // use default icon
    mCompass = NBRE_NEW NBGM_Compass(mNBGMContext, mSubView, *mOverlayManager, mCompassDrawOrder);
    if(mCompass == NULL)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_MapViewImpl::InitializeEnvironment create compass failed!");
    }

    PAL_Error err = PAL_EventCreate(mNBGMContext.renderingEngine->Context().mPalInstance, &mLoadingThreadFinalizeEvent);
    if(err != PAL_Ok)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_MapViewImpl::InitializeEnvironment create loading thread finalize event failed");
    }

    //mPickingRadius = MM_TO_PIXEL(10.0f);


    mInitialized = TRUE;
    ScheduleTimer(mConfig.renderInterval, RenderCallBack, this);
    mRenderExpectedTime = PAL_ClockGetTimeMs() + mConfig.renderInterval;
    //NBRE_SpatialDivideManagerDebug* spatialDivideDebug = NBRE_NEW NBRE_SpatialDivideManagerDebug(*spatialDivideManager, LT_LabelDebug);
    ////spatialDivideDebug->SetOnlyShowHasObj(TRUE);
    //mSubView->DebugEngine().AddDebugElement(spatialDivideDebug);
}

void NBGM_MapViewImpl::Finalize()
{
    mConfig.resourceManager->GetLoadingTaskQueue()->AddTask(NBRE_NEW NBGM_FinalizeTask(mLoadingThreadFinalizeEvent));

    PAL_EventWaitForEvent(mLoadingThreadFinalizeEvent);

    CancelTimer(RenderCallBack, this);
    //to prevent render thread to execute some task which is generated by mapviewimpl, so we must remove all tasks in render thread.
    mRenderThread->RemoveAllTask();

    NBRE_Vector<NBGM_VectorTileID> releasedTiles;
    mTileIdCache->UnloadAllTiles(releasedTiles);
    mConfig.resourceManager->ReleaseData(releasedTiles);

    mConfig.resourceManager->GetConfig().renderContext->SwitchTo();
    mCustomObjectesContext.Clean();
    for (uint32 i = 0; i < mOptionalLayers.size(); ++i)
    {
        NBRE_DELETE mOptionalLayers[i];
    }
    for (uint32 i = 0; i < mTileAnimationLayers.size(); ++i)
    {
        NBRE_DELETE mTileAnimationLayers[i];
    }

    for(uint32 i = 0; i <mCreatedAvatar.size(); ++i)
    {
        NBRE_DELETE mCreatedAvatar[i];
    }
    mCreatedAvatar.clear();

    for(uint32 i = 0; i <mCreatedCircles.size(); ++i)
    {
        NBRE_DELETE mCreatedCircles[i];
    }
    mCreatedCircles.clear();

    for(uint32 i = 0; i <mCreatedRect2ds.size(); ++i)
    {
        NBRE_DELETE mCreatedRect2ds[i];
    }
    mCreatedRect2ds.clear();

    for (NBRE_Map<NBRE_String, NBGM_Icon*>::iterator iter = mCreatedIcons.begin(); iter != mCreatedIcons.end(); ++iter)
    {
        if (iter->second)
        {
            NBRE_DELETE iter->second;
        }
    }
    mCreatedIcons.clear();

    NBRE_DELETE mGlowLayer;
    NBRE_DELETE mTransparentLayer;
    NBRE_DELETE mGridEntity;
    NBRE_DELETE mNavIconLayer;
    NBRE_DELETE mStartFlag;
    NBRE_DELETE mEndFlag;
    NBRE_DELETE mAvatar;
    NBRE_DELETE mTileIdCache;
    NBRE_DELETE mTileMapLayer;
    NBRE_DELETE mLabelLayer;
    NBRE_DELETE mPoiLayer;
    NBRE_DELETE mNavVecRouteLayer;
    NBRE_DELETE mNavEcmRouteLayer;
    NBRE_DELETE mSky;
    NBRE_DELETE mBillboardSet;
    NBRE_DELETE mLabelTransparentSky;
    NBRE_DELETE mPathArrowLayer;

    NBRE_DELETE mTransUtil;
    NBRE_DELETE mNBGMContext.buildModelContext;
    NBRE_DELETE mNBGMContext.layoutManager;
    NBRE_DELETE mNBGMContext.fileLogger;
    NBRE_DELETE mCompass;
    NBRE_DELETE mOverlayManager;

    PAL_EventDestroy(mLoadingThreadFinalizeEvent);
}

void NBGM_MapViewImpl::ActiveSurface()
{
    mNBGMContext.renderingEngine->SetActiveRenderSurface(mSurface);
}

NBRE_RenderSurface* NBGM_MapViewImpl::Surface()
{
    return mSurface;
}

void NBGM_MapViewImpl::CreateLayer()
{
    mBillboardSet = NBRE_NEW NBRE_BillboardSet(mNBGMContext.renderingEngine->Context(), mOverlayManager->FindId(mNavElementDrawOrder));
    mBillboardSet->EnableZOrder(FALSE);

    mSubView->AddVisitable(mBillboardSet);
    mLabelLayer = NBRE_NEW NBGM_LabelLayer(mNBGMContext, *mSceneManager->RootSceneNode(), mOverlayManager->FindId(mLabelDrawOrder), mBillboardSet, mScreenOverlay, mConfig.useNavRoadLabel, mSubView, *mOverlayManager, mScreenCamera, &mProfiler, LAYER_STRIDE, DO_MAIN, mConfig.resourceManager->GetLoadingTaskQueue(), mRenderThread, this);
    mPoiLayer = NBRE_NEW NBGM_PoiLayer(mNBGMContext, *mSceneManager->RootSceneNode(), mBillboardSet);
    mTileMapLayer = NBRE_NEW NBGM_TileMapLayer(mNBGMContext, *mSceneManager->RootSceneNode(), mSubView, *mOverlayManager, &mProfiler, mTransparentLayer, mGlowLayer, mNavVecRouteLayer, LAYER_STRIDE, mTransparentLayer->RoadBackgroundOverlayId(), mTransparentLayer->BuildingOverlayId(), mGlowLayer->BuildingOverlayId());
    mPathArrowLayer = NBRE_NEW NBGM_PathArrowLayer(mNBGMContext, *mSceneManager->RootSceneNode(), mSubView, *mOverlayManager, &mProfiler);

    if (mConfig.drawNavPoi)
    {
        mNavIconLayer = NBRE_NEW NBGM_NavIconLayer(mNBGMContext, *mSceneManager->RootSceneNode(), mBillboardSet);
        mNavIconLayer->AddToSence();
    }

    if (mConfig.drawVectorNavRoute)
    {
        mNavVecRouteLayer = NBRE_NEW NBGM_NavVectorRouteLayer(mNBGMContext, *mSceneManager->RootSceneNode(), mSubView, *mOverlayManager, 120*LAYER_STRIDE+DO_MAIN);
    }

    if (mConfig.drawEcmNavRoute)
    {
        mNavEcmRouteLayer = NBRE_NEW NBGM_NavEcmRouteLayer(mNBGMContext, *mSceneManager->RootSceneNode(), mSubView, *mOverlayManager, mECMRouteDrawOrder, mECMArrowDrawOrder);
    }

    mMapLayers.push_back(mTileMapLayer);
    mMapLayers.push_back(mLabelLayer);
    mMapLayers.push_back(mPoiLayer);
    if (mConfig.useNavRoadLabel)
    {
        mMapLayers.push_back(mPathArrowLayer);
    }
}

void NBGM_MapViewImpl::InitDrawOrder()
{
    int32 backgroundId = mOverlayManager->AssignOverlayId(mBackgoundDrawOrder);
    NBRE_Overlay* overlay = mOverlayManager->FindOverlay(backgroundId);
    overlay->SetClearColor(NBRE_Color(0.9372f, 0.9490f, 0.9137f, 1.0f));
    overlay->SetClearFlag(NBRE_CLF_CLEAR_COLOR|NBRE_CLF_CLEAR_DEPTH);

    mOverlayManager->AssignOverlayId(mSkyDrawOrder);
    mOverlayManager->AssignOverlayId(mNavElementDrawOrder);
    int32 labelId = mOverlayManager->AssignOverlayId(mLabelDrawOrder);
    mOverlayManager->AssignOverlayId(mLabelTransparentWallDrawOrder);
    mOverlayManager->AssignOverlayId(mLabelTransparentSkyDrawOrder);
    int32 navLabelId =  mOverlayManager->AssignOverlayId(mNavLabelDrawOrder);

    int32 avatarId = mOverlayManager->AssignOverlayId(mAvatarDrawOrder);

    mScreenOverlay = mOverlayManager->FindOverlay(navLabelId);
    mScreenOverlay->SetAutoClearRenderProvider(FALSE);

    NBRE_RenderQueue*queue = mOverlayManager->FindOverlay(labelId);
    mProfiler.AddRenderQueue(TID_LABLE, *queue);
    queue = mOverlayManager->FindOverlay(avatarId);
    mProfiler.AddRenderQueue(TID_OTHERS, *queue);
}

static nb_boolean HitTest(Polygon2f polygons1[], Polygon2f polygons2[], int count)
{
    for(int i = 0; i < count; ++i)
    {
        for(int j = 0; j < count; ++j)
        {
            if(NBRE_Intersectionf::HitTest(polygons1[i], polygons2[j]))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

void NBGM_MapViewImpl::PrepareRender()
{
    for (uint32 i = 0; i < mTileAnimationLayers.size(); ++i)
    {
        mTileAnimationLayers[i]->SyncData();
    }
    for (uint32 i = 0; i < mOptionalLayers.size(); ++i)
    {
        mOptionalLayers[i]->SyncData();
    }

    if(mChangeZoomLevel)
    {
        mTileMapLayer->SetCurrentZoomLevel(mCurrentZoomLevel);
    }

    mTransparentLayer->SwitchRenderTarget();

    //check boundary
    CheckBoundary();
    mProfiler.PrepareFrame();
    mProfiler.SetLoadedTileNum(mTileIdCache->GetActivedTileNum());
}

void NBGM_MapViewImpl::UpdatePosition()
{
    mSceneManager->UpdatePosition();
    mTransUtil->UpdateCameraMatrix();

    mProfiler.UpdatePosition();
}

void NBGM_MapViewImpl::UpdateLabel()
{
    //printf(" --- NBGM_MapViewImpl::UpdateLabel() --- \n"); fflush(stdout);

    //update tile far/near visibility
    mTileMapLayer->UpdateEntityVisibility(mCamera);
    mLabelLayer->UpdateLayoutElementVisibility(static_cast<float>(mNBGMContext.ModelToWorld(MERCATER_TO_METER(mCurrentViewPointDistance))));
    if (mConfig.useNavRoadLabel)
    {
        mPathArrowLayer->UpdatePathArrowVisibility(mCamera);
    }

    //update label layout
    if (mIsLayoutInvalidate || mNBGMContext.layoutManager->IsNavMode())
    {
        mIsLayoutInvalidate = mNBGMContext.layoutManager->Update(1000);
    }

    /// update per frame
    mNBGMContext.layoutManager->UpdateVisibleElements();

    if (mGridEntity != NULL)
    {
        mGridEntity->UpdateGrid();
    }

    const NBRE_Vector2i& viewportSize = mSubView->Viewport().GetRect().GetSize();
    mPoiLayer->Update();
    if (mLastSkyHeight != mSkyHeightOnViewport || viewportSize != mLastViewportSize)
    {
        mPoiLayer->Refresh(viewportSize.y - mSkyHeightOnViewport);
        mLastSkyHeight = mSkyHeightOnViewport;
        mLastViewportSize = viewportSize;
    }
    if (mNavIconLayer)
    {
        mNavIconLayer->Update();
    }
    NBRE_Vector3f position = NBRE_TypeConvertf::Convert(mCamera->Position());
    //update avatar
    if(mAvatar)
    {
        mAvatar->Update(mNBGMContext.ModelToWorld(position.Length()));
    }
    for(uint32 i=0; i<mCreatedAvatar.size(); ++i)
    {
        NBGM_Avatar* avatar = mCreatedAvatar[i];
        avatar->Update(mNBGMContext.ModelToWorld(position.Length()));
    }

    if(mStartFlag)
    {
        mStartFlag->Update();
    }

    if(mEndFlag)
    {
        mEndFlag->Update();
    }

    if(mStartFlag && mAvatar)
    {
        if(mAvatar->IsScreenPointAvailable() && mStartFlag->IsScreenPointAvailable())
        {
            Polygon2f polygons1[5];
            Polygon2f polygons2[5];
            mAvatar->GetPolygon(polygons1);
            mStartFlag->GetPolygon(polygons2);
            if(HitTest(polygons1, polygons2, 5))
            {
                mStartFlag->Hide();
            }
        }
    }

    if(mEndFlag && mAvatar)
    {
        if(mAvatar->IsScreenPointAvailable() && mEndFlag->IsScreenPointAvailable())
        {
            Polygon2f polygons1[5];
            Polygon2f polygons2[5];
            mAvatar->GetPolygon(polygons1);
            mEndFlag->GetPolygon(polygons2);
            if(HitTest(polygons1, polygons2, 5))
            {
                mEndFlag->Hide();
            }
        }
    }
    mProfiler.UpdateLabel();
}

void NBGM_MapViewImpl::EndRender(float secondsPassed)
{
//	printf(" --- NBGM_MapViewImpl::EndRender() --- \n"); fflush(stdout);
//    nbre_assert(FALSE);
//    return;

    if (mSyncRenderDelay > 0)
    {
        mNBGMContext.needMoreRenderCycle = TRUE;
        mSyncRenderDelay -= secondsPassed;
        if (mSyncRenderDelay < 0) {
            mSyncRenderDelay = 0;
        }
    }
    mProfiler.PostRender();
}

nb_boolean NBGM_MapViewImpl::RenderFrame()
{
//	printf(" --- NBGM_MapViewImpl::RenderFrame() -start- \n"); fflush(stdout);

    //if view has been set to background, do nothing
    if(mBackground)
    {
        mIsInvalidate = FALSE;
        return FALSE;
    }

    //if render system is not ready, retry
    if(!mNBGMContext.renderContext)
    {
        return FALSE;
    }
    //make current faild, need more render loop
    if(!mNBGMContext.renderContext->BeginRender())
    {
        return TRUE;
    }

    mNBGMContext.renderingEngine->SetRenderTarget(mNBGMContext.renderContext->GetRenderTarget());

    //perpare render
//	printf(" --- NBGM_MapViewImpl::RenderFrame() -HANDLE- \n"); fflush(stdout);
    NBRE_List<NBGM_FrameListener*> renderListeners;
    renderListeners.swap(mPendingRenderListeners);

    for(NBRE_List<NBGM_FrameListener*>::iterator iter = renderListeners.begin(); iter != renderListeners.end(); ++iter)
    {
        (*iter)->OnRenderFrameBegin();
    }

    mProfiler.BeginFrame();
    mNBGMContext.needMoreRenderCycle = FALSE;

    uint32 now = PAL_ClockGetTimeMs();
    float secondsPassed = (now - mLastUpdateMs) * 0.001f;

    PrepareRender();

    //update position
    UpdatePosition();

    UpdateLabel();
    if(mTileMapLayer->CheckIfHasLM3D())
    {
        mTileMapLayer->DisableUnTexturedLandMarks();
    }

    if(mCompass)
    {
        mCompass->Update();
    }

    // calculate scale based on map center
    double pixelPerUnit = 0.0;
    mNBGMContext.transUtil->PixelsPerUnit(mMapCenter->Position(), pixelPerUnit);

    //update circles
    for(uint32 i=0; i<mCreatedCircles.size(); ++i)
    {
        NBGM_CircleImpl* circle = mCreatedCircles[i];
        if(circle != NULL)
        {
            circle->Update(pixelPerUnit);
        }
    }

    //update rect2ds
    for(uint32 i=0; i<mCreatedRect2ds.size(); ++i)
    {
        NBGM_Rect2dImpl* rect = mCreatedRect2ds[i];
        if(rect != NULL)
        {
            rect->Update(pixelPerUnit);
        }
    }

    for (NBRE_Map<NBRE_String, NBGM_Icon*>::iterator iter = mCreatedIcons.begin(); iter != mCreatedIcons.end(); ++iter)
    {
        if (iter->second)
        {
            iter->second->Update();
        }
    }

    //render
    mNBGMContext.renderingEngine->Render();
    mProfiler.Render();

    //end render
    EndRender(secondsPassed);

    mLastUpdateMs = now;
    mIsInvalidate = FALSE;

    //swap faild, need more render loop
    if(!mNBGMContext.renderContext->EndRender())
    {
        return TRUE;
    }

    mProfiler.EndFrame(*mNBGMContext.fileLogger);

    for(NBRE_List<NBGM_FrameListener*>::iterator iter = renderListeners.begin(); iter != renderListeners.end(); ++iter)
    {
        (*iter)->OnRenderFrameEnd();
    }

    return mNBGMContext.needMoreRenderCycle;
}

void NBGM_MapViewImpl::SetBackground(bool background)
{
    mIsInvalidate = !background;
    mBackground = background;
}

void NBGM_MapViewImpl::RequestRenderFrame(NBGM_FrameListener* listener)
{
    //if the render operation has been requested and the render has not been done, this render request will be ignored.
    if (mBackground || (!mSizeValid))
    {
        if(listener)
        {
            listener->OnDiscard();
        }
        return;
    }

    if(listener)
    {
        mPendingRenderListeners.push_back(listener);
    }
    mIsInvalidate = TRUE;
}

void NBGM_MapViewImpl::InvalidateLayout()
{
    if (!mSizeValid)
    {
        return;
    }
    mIsLayoutInvalidate = TRUE;
}

void NBGM_MapViewImpl::SetViewCenter(double mercatorX, double mercatorY)
{
    mMapCenter->SetPosition(NBRE_Vector3d(mNBGMContext.WorldToModel(mercatorX), mNBGMContext.WorldToModel(mercatorY), 0));
    mPoiLayer->Refresh(mSubView->Viewport().GetRect().GetSize().y - mSkyHeightOnViewport);
    InvalidateLayout();
}

void NBGM_MapViewImpl::LoadExternalMaterial(const NBRE_String& categoryName, const NBRE_String& materialName, shared_ptr<NBRE_IOStream> istream, const NBRE_Vector<NBRE_String>& dependsMaterial)
{
    NBGM_LoadMaterialTask* task = NBRE_NEW NBGM_LoadMaterialTask(mConfig.resourceManager->GetResourceContext(), mNBGMContext, categoryName, materialName, istream, mRenderThread, dependsMaterial);
    mConfig.resourceManager->GetLoadingTaskQueue()->AddTask(task);

    SetCurrentExternalMaterial(categoryName, materialName);
}

void NBGM_MapViewImpl::SetCurrentExternalMaterial(const NBRE_String& categoryName, const NBRE_String& materialName)
{
    NBRE_Overlay* overlay = mOverlayManager->FindOverlay(mOverlayManager->FindId(mBackgoundDrawOrder));

    NBRE_Set<uint32>* fontIds = NBRE_NEW NBRE_Set<uint32>();
    mLabelLayer->GetFontMaterialIds(categoryName, *fontIds);

    NBGM_SeCurrenttMaterialTask* task = NBRE_NEW NBGM_SeCurrenttMaterialTask(mConfig.resourceManager->GetResourceContext(), this, mNBGMContext, categoryName, materialName, mRenderThread, overlay, fontIds);

    mConfig.resourceManager->GetLoadingTaskQueue()->AddTask(task);
}

void NBGM_MapViewImpl::SetSkyDayNight(nb_boolean isDay)
{
    if(mSky)
    {
        mSky->SetDayNight(isDay);
    }
}

void NBGM_MapViewImpl::SetIsSatelliteMode(nb_boolean isSatellite)
{
    if (mTileMapLayer && mTransparentLayer)
    {
        mTileMapLayer->SetRenderRoadToFrameBuffer(isSatellite);
        mTransparentLayer->SetSatelliteMode(isSatellite);
    }
}

void NBGM_MapViewImpl::LoadExternalMaterial(const NBRE_String& categoryName, const NBRE_String& materialName, std::vector<NBGM_IMapMaterial*>& materialTable)
{
    NBGM_LoadMaterialTask* task = NBRE_NEW NBGM_LoadMaterialTask(mConfig.resourceManager->GetResourceContext(), mNBGMContext, categoryName, materialName, materialTable, mRenderThread);
    mConfig.resourceManager->GetLoadingTaskQueue()->AddTask(task);
    SetCurrentExternalMaterial(categoryName, materialName);
}

void NBGM_MapViewImpl::UnloadExternalMaterial(const NBRE_String& categoryName, const NBRE_String& materialName)
{
    NBGM_UnloadMaterialTask* task = NBRE_NEW NBGM_UnloadMaterialTask(mConfig.resourceManager->GetResourceContext(), mNBGMContext, categoryName, materialName, mRenderThread);
    mConfig.resourceManager->GetLoadingTaskQueue()->AddTask(task);
}

void NBGM_MapViewImpl::OnSizeChanged(int32 x, int32 y, uint32 w, uint32 h)
{
    printf(" --- NBGM_MapViewImpl::OnSizeChanged(): %d, %d --- \n", w, h); fflush(stdout);

    mScreenSize.x = w;
    mScreenSize.y = h;

    mSubView->Viewport().Update(x, y, w, h);

    if(mTransparentLayer)
    {
        mTransparentLayer->OnSizeChanged(x, y, w, h);
    }

    if (mGlowLayer)
    {
        mGlowLayer->OnSizeChanged(x, y, w, h);
    }

    if(mCompass)
    {
        mCompass->OnSizeChanged(x, y, w, h);
    }

    // Update screen overlay
    NBRE_OrthoConfig config;
    config.mLeft = static_cast<float>(x);
    config.mRight = static_cast<float>(x + w);
    config.mBottom = static_cast<float>(y);
    config.mTop = static_cast<float>(y + h);

    mScreenCamera->GetFrustum().SetAsOrtho(config, -1, 1);

    mScreenOverlay->SetCamera(mScreenCamera);

    // Update layout buffer
    mNBGMContext.layoutManager->ResizeBuffer(static_cast<uint32>(x + w), static_cast<uint32>(y + h));
    if (mEnableDebugDraw)
    {
        if (w != 0 && h != 0)
        {
            NBGM_LayoutBuffer& lb = mNBGMContext.layoutManager->GetLayoutBuffer();
            if (mGridEntity == NULL)
            {
                mGridEntity = NBRE_NEW NBGM_GridEntity(mNBGMContext.renderingEngine->Context(), lb.Buffer(), lb.CellCount(), lb.CellSize());
                mScreenOverlay->AppendRenderProvider(mGridEntity);
            }
            else
            {
                mGridEntity->UpdateBufferSize(lb.Buffer(), lb.CellCount(), lb.CellSize());
            }
        }
    }

    mSkyHeightOnViewport = CalculateViewPointSkyHeight();
    mNBGMContext.transUtil->UpdateCameraMatrix();
    // Only allow rendering if the width/height is more then one. At startup the default is 1/1
    if ((w > 1) && (h > 1))
    {
        mSizeValid = TRUE;
    }
}

void NBGM_MapViewImpl::SetPerspective(float fov, float aspect)
{
    NBRE_PerspectiveConfig perspective = {fov, aspect};
    mCamera->GetFrustum().SetAsPerspective(perspective, mCamera->GetFrustum().GetZNear(), mCamera->GetFrustum().GetZFar());
    SetHorizonDistance(mHorizonDistance);
}

nb_boolean NBGM_MapViewImpl::IsNbmDataExist(const NBRE_String& /*nbmName*/)
{
    NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_MapViewImpl::IsNbmDataExist not support yet");
    nbre_assert(FALSE);
    return FALSE;
}

void NBGM_MapViewImpl::LoadNbmData(const NBGM_NBMDataLoadInfo& info)
{
    NBRE_Vector<NBGM_NBMDataLoadInfo> tiles;
    tiles.push_back(info);

    if(info.enableLog)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "Load Start Time = %u", PAL_ClockGetTimeMs());
    }

    mConfig.resourceManager->RequestNBMData(tiles, NBRE_NEW ResourceCallBack(this), mRenderThread);
}

void NBGM_MapViewImpl::LoadNbmData(const NBRE_Vector<NBGM_NBMDataLoadInfo>& tiles)
{
    mConfig.resourceManager->RequestNBMData(tiles, NBRE_NEW ResourceCallBack(this), mRenderThread);
}

void NBGM_MapViewImpl::LoadBinData(const NBGM_NBMDataLoadInfo& info)
{
    NBRE_Vector<NBGM_NBMDataLoadInfo> tiles;
    tiles.push_back(info);

    if(info.enableLog)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "Load Start Time = %u", PAL_ClockGetTimeMs());
    }

    mConfig.resourceManager->RequestBINData(tiles, NBRE_NEW ResourceCallBack(this), mRenderThread);
}

void NBGM_MapViewImpl::UnLoadTile(const NBRE_String& nbmName)
{
    NBGM_UnloadTask* task = NBRE_NEW NBGM_UnloadTask(this, mRenderThread, FALSE);
    NBRE_Vector<NBGM_VectorTileID> ids;
    ids.push_back(nbmName);
    task->AssignUnloadIds(ids);
    mConfig.resourceManager->GetLoadingTaskQueue()->AddTask(task);
}

void NBGM_MapViewImpl::UnLoadTile(const NBRE_Vector<NBRE_String>& nbmNames)
{
    NBGM_UnloadTask* task = NBRE_NEW NBGM_UnloadTask(this, mRenderThread, FALSE);
    task->AssignUnloadIds(nbmNames);
    mConfig.resourceManager->GetLoadingTaskQueue()->AddTask(task);
}

void NBGM_MapViewImpl::UnLoadAllTiles()
{
    NBGM_UnloadTask* task = NBRE_NEW NBGM_UnloadTask(this, mRenderThread, TRUE);
    mConfig.resourceManager->GetLoadingTaskQueue()->AddTask(task);
}

nb_boolean NBGM_MapViewImpl::GetFrustumPositionInWorld(double frustumHeight, NBRE_Vector<NBGM_Point2d64>& positions)
{
    positions.resize(4);
    NBRE_Camera& camera  = *mSubView->Viewport().Camera().get();
    camera.UpdatePosition();
    NBRE_Frustum &frustum = camera.GetFrustum();
    NBRE_Frustum3d frustum3D(NBRE_TypeConvertd::Convert(frustum.ExtractMatrix()) * camera.ExtractMatrix());
    NBRE_Vector3d corners[8];
    frustum3D.GetCorners(corners);
    NBRE_Planed plane(NBRE_Vector3d(0, 0, 1), 0);
    NBRE_Vector3d points[4];
    double epsilon = mNBGMContext.WorldToModel(VECTOR_EPSILON);
    // left top
    nb_boolean ret = NBRE_Intersectiond::HitTest(plane, corners[3], corners[7], points, epsilon);
    if(!ret)
    {
        ret = NBRE_Intersectiond::HitTest(plane, corners[4], corners[7], points, epsilon);
    }

    if(!ret)
        return FALSE;

    // right top
    ret = NBRE_Intersectiond::HitTest(plane, corners[2], corners[6], points+1, epsilon);
    if(!ret)
    {
        ret = NBRE_Intersectiond::HitTest(plane, corners[5], corners[6], points+1, epsilon);
    }
    // right bottom
    ret = ret?NBRE_Intersectiond::HitTest(plane, corners[1], corners[5], points+2, epsilon):FALSE;
    // left bottom
    ret = ret?NBRE_Intersectiond::HitTest(plane, corners[0], corners[4], points+3, epsilon):FALSE;

    if(ret)
    {
        NBRE_Vector3f pos = NBRE_TypeConvertf::Convert(mCamera->Position());
        pos.Normalise();
        float cosValue = pos.DotProduct(NBRE_Vector3f(0, 0, 1));
        float titleAngle = static_cast<float> (NBRE_Math::RadToDeg(nsl_acos(cosValue)));
        NBRE_Ray3d ray(points[0], points[1]-points[0]);
        double origin = ray.DistanceTo(points[2]);
        double refDistance = mNBGMContext.WorldToModel(frustumHeight);
        float horizonDistance = mNBGMContext.WorldToModel(mHorizonDistance);
        float halfFov = frustum.GetPerspectiveConfig().mFov/2.f;
        float maxAngle = titleAngle + halfFov;
        float minAngle = titleAngle - halfFov;
        double minDistance = origin;
        if(frustum.GetProjectionType() == NBRE_PT_PERSPECTIVE && maxAngle < 90 && minAngle > 0)
        {
            double realDistance = camera.Position().z*(nsl_tan(NBRE_Math::DegToRad(maxAngle)) - nsl_tan(NBRE_Math::DegToRad(minAngle)));
            minDistance = NBRE_Math::Min<double>(minDistance, realDistance);
        }
        if(refDistance > 0)
        {
            minDistance = NBRE_Math::Min<double>(minDistance, refDistance);
        }
        if(horizonDistance > 0)
        {
            minDistance = NBRE_Math::Min<double>(minDistance, horizonDistance);
        }
        nbre_assert(origin != 0);
        double t = minDistance/origin;
        points[0] = NBRE_LinearInterpolated::Lerp(points[3], points[0], t);
        points[1] = NBRE_LinearInterpolated::Lerp(points[2], points[1], t);

        for(uint32 i = 0; i < 4; i++)
        {
            positions[i].x = mNBGMContext.ModelToWorld(points[i].x);
            positions[i].y = mNBGMContext.ModelToWorld(points[i].y);
        }
    }

    return ret;
}

void NBGM_MapViewImpl::SetHorizonDistance(double horizonDistance)
{
    if (NBRE_Math::IsZero(static_cast<float>(horizonDistance), 1e-6f))
    {
        return;
    }

    NBRE_Frustum& frustum = mCamera->GetFrustum();
    float width = 0.f;
    float height = 0.f;
    float distance = 0.f;
    if(frustum.GetProjectionType() == NBRE_PT_PERSPECTIVE)
    {
        NBRE_Vector3f dir = mCamera->Direction();
        dir.Normalise();
        float tileAngle = NBRE_Math::RadToDeg(nsl_acos(dir.DotProduct(NBRE_Vector3f(0, 0, 1))));
        float angle = tileAngle - frustum.GetPerspectiveConfig().mFov/2.f;
        distance = static_cast<float>(mNBGMContext.WorldToModel(horizonDistance) + nsl_tan(NBRE_Math::DegToRad(angle))*mCamera->WorldPosition().z);
        width = distance*nsl_tan(NBRE_Math::DegToRad(frustum.GetPerspectiveConfig().mFov*frustum.GetPerspectiveConfig().mAspect/2.f))*4.2f;
        height = distance*nsl_tan(NBRE_Math::DegToRad(frustum.GetPerspectiveConfig().mFov/2.f))*1.2f;
        if(mScreenSize.y > 0)
        {
            const float ratio = 1024.0f /mScreenSize.y;
            if(ratio > 1.0f)
            {
                //make the height slightly bigger
                height *= (ratio*1.1f);
            }
        }
    }
    SetSkySize(width, height, distance);
    float hd = (float) mNBGMContext.WorldToModel(horizonDistance);
    if (hd != mHorizonDistance)
    {
        InvalidateLayout();
    }
    mHorizonDistance = hd;

    mSkyHeightOnViewport = CalculateViewPointSkyHeight();
}

void NBGM_MapViewImpl::SetRotateAngle(float angle)
{
    if (mRotateAngle != angle)
    {
        mMapCenter->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0, 0, 1), -angle));
        InvalidateLayout();
        mRotateAngle = angle;
    }

    if(mCompass)
    {
        mCompass->SetRotateAngle(-angle);
    }
}

void NBGM_MapViewImpl::SetTiltAngle(float angle)
{
    if(angle < 0)
    {
        angle = 0;
    }

    if (mTiltAngle == angle)
    {
        return;
    }
    mTiltAngle = angle;

    mCamera->SetOrientation(NBRE_Orientation(mCamera->Right(), angle));

    NBRE_Matrix4x4d transMat = NBRE_TypeConvertd::Convert(NBRE_Transformationf::BuildRotateDegreesMatrix(angle, mCamera->Right()));

    NBRE_Vector3d pos = mCamera->Position();
    NBRE_Point3d newPos(0, 0, pos.Length());
    newPos = transMat * newPos;
    mCamera->SetPosition(newPos);

    mSkyHeightOnViewport = CalculateViewPointSkyHeight();
    InvalidateLayout();
}

float NBGM_MapViewImpl::ViewPointDistanceWithScale()
{
    const NBRE_Vector3d& position = mCamera->Position();
    return static_cast<float>(position.Length());
}

void NBGM_MapViewImpl::SetCurrentZoomLevel(int8 zoomlevel)
{
    if(mCurrentZoomLevel != zoomlevel)
    {
        mNBGMContext.layoutManager->Invalidate();
        mCurrentZoomLevel = zoomlevel;
        mChangeZoomLevel = TRUE;
        mTransparentLayer->SetZoomLevel(zoomlevel);
    }
}

void NBGM_MapViewImpl::SetViewPointDistance(float distance)
{
    double modelDistance = mNBGMContext.WorldToModel(distance);

    NBRE_Vector3d position = mCamera->Position();

    if (mCurrentViewPointDistance != modelDistance)
    {
        position.Normalise();
        position *= modelDistance;
        mCamera->SetPosition(position);
        mCurrentViewPointDistance = modelDistance;
        mNBGMContext.layoutManager->Invalidate();
        InvalidateLayout();
    }

    NBRE_PerspectiveConfig perspective = mCamera->GetFrustum().GetPerspectiveConfig();
    double nearDis = modelDistance/50;
    if(nearDis > position.z)
    {
        nearDis = position.z/2;
    }
    double farDis = nearDis*20000;
    mCamera->GetFrustum().SetAsPerspective(perspective, static_cast<float> (nearDis), static_cast<float> (farDis));

    mSkyHeightOnViewport = CalculateViewPointSkyHeight();
}

void NBGM_MapViewImpl::SetAvatarLocation(const NBGM_Location64& location)
{
    if(mAvatar != NULL)
    {
        mAvatar->SetLocation(location);
    }
}

float NBGM_MapViewImpl::GetFontScale(void)
{
    return mFontScale;
}

void NBGM_MapViewImpl::SetFontScale(float scaleValue)
{
    if(mFontScale != scaleValue)
    {
        mFontScale = scaleValue;
        InvalidateLayout();
    }
}

void NBGM_MapViewImpl::SetAvatarScale(float scaleValue)
{
    if(mAvatar != NULL)
    {
        mAvatar->SetScale(scaleValue);
    }
}

void NBGM_MapViewImpl::SetAvatarState(NBGM_AvatarState state)
{
    if(mAvatar != NULL)
    {
        mAvatar->SetState(state);
    }
}

void NBGM_MapViewImpl::SetAvatarMode(NBGM_AvatarMode mode)
{
    if(mAvatar != NULL)
    {
        mAvatar->SetMode(mode);
    }
}

void NBGM_MapViewImpl::EnableProfiling(nb_boolean enabled)
{
    mEnableProfiling = enabled;
}

void NBGM_MapViewImpl::EnableVerboseProfiling(nb_boolean enabled)
{
    mProfiler.EnableVerboseProfiling(enabled);
}

nb_boolean NBGM_MapViewImpl::GetPoiPosition(const NBRE_String& poiId, NBRE_Point2f& coordinates)
{
    if(mPoiLayer->GetIconPosition(poiId, coordinates))
    {
        coordinates.y = mSubView->Viewport().GetRect().GetSize().y - coordinates.y;
        return TRUE;
    }

    if(mCreatedIcons.find(poiId) != mCreatedIcons.end())
    {
        if(mCreatedIcons[poiId]->GetScreenPosition(coordinates))
        {
            coordinates.y = mSubView->Viewport().GetRect().GetSize().y - coordinates.y;
            return TRUE;
        }
    }
    return FALSE;
}

nb_boolean NBGM_MapViewImpl::GetPoiBubblePosition(const NBRE_String& poiId, NBRE_Point2f& coordinates)
{
    if(mPoiLayer->GetIconBubblePosition(poiId, coordinates))
    {
        coordinates.y = mSubView->Viewport().GetRect().GetSize().y - coordinates.y;
        return TRUE;
    }

    if(mCreatedIcons.find(poiId) != mCreatedIcons.end())
    {
        if (mCreatedIcons[poiId]->GetScreenBubblePosition(coordinates))
        {
            NBRE_Vector2i screenSize = mSubView->Viewport().GetRect().GetSize();
            coordinates.y = screenSize.y - coordinates.y;

            if(coordinates.x < 0.0 || coordinates.x > screenSize.x)
            {
                return FALSE;
            }
            else if(coordinates.y < 0.0 || coordinates.y > screenSize.y)
            {
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

nb_boolean NBGM_MapViewImpl::SelectAndTrackPoi(const NBRE_String& poiId)
{
    nb_boolean ret =  mPoiLayer->SelectIcon(poiId, TRUE);
    mPoiLayer->Update();

    if((ret == FALSE) && (mCreatedIcons.find(poiId) != mCreatedIcons.end()))
    {
        mCreatedIcons[poiId]->SetSelected(TRUE);
        mCreatedIcons[poiId]->Update();
        ret = TRUE;
    }

    return ret;
}

nb_boolean NBGM_MapViewImpl::UnselectAndStopTrackingPoi(const NBRE_String& poiId)
{
    nb_boolean ret = mPoiLayer->SelectIcon(poiId, FALSE);
    mPoiLayer->Update();

    if((ret == FALSE) && (mCreatedIcons.find(poiId) != mCreatedIcons.end()))
    {
        mCreatedIcons[poiId]->SetSelected(FALSE);
        mCreatedIcons[poiId]->Update();
        ret = TRUE;
    }
    return ret;
}

uint32 NBGM_MapViewImpl::GetInteractedPois(NBRE_Vector<NBRE_String>& ids, const NBRE_Point2f& screenPosition)
{
    NBRE_Point2f sp = screenPosition;
    sp.y = mSubView->Viewport().GetRect().GetSize().y - screenPosition.y;

    NBRE_Vector<NBRE_String> pois;
    mPoiLayer->GetIconsAt(sp, pois);
    NBRE_Vector<NBRE_String> customPins;
    mLabelLayer->GetElementsAt(NBGM_LET_CUSTOM_PIN, sp, 0, customPins);

    for (uint32 i = 0; i < pois.size(); ++i)
    {
        NBRE_String s(pois[i]);
        ids.push_back(s);
    }

    for (uint32 i = 0; i < customPins.size(); ++i)
    {
        NBRE_String s("CUSTOM_PIN/");
        s.append(customPins[i]);
        ids.push_back(s);
    }

    for (NBRE_Map<NBRE_String, NBGM_Icon*>::iterator iter = mCreatedIcons.begin(); iter != mCreatedIcons.end(); ++iter)
    {
        if (iter->second->HitTest(sp))
        {
            ids.push_back(iter->first);
        }
    }

    return ids.size();
}

nb_boolean NBGM_MapViewImpl::GetStaticPoiInfo(const NBRE_String& id, NBGM_StaticPoiInfo& info)
{
    nb_boolean result = mLabelLayer->GetStaticPoiInfo(id, info);
    if (result)
    {
        info.bubbleOffset.y = -info.bubbleOffset.y;
    }
    return result;
}

void NBGM_MapViewImpl::SelectStaticPoi(const NBRE_String &id, nb_boolean selected)
{
    if (mLabelLayer)
    {
        mLabelLayer->SelectStaticPoi(id, selected);
    }
}

void NBGM_MapViewImpl::GetStaticPoiAt(NBRE_Vector<NBRE_String>& pois, const NBRE_Point2f& screenPosition)
{
    NBRE_Point2f sp = screenPosition;
    sp.y = mSubView->Viewport().GetRect().GetSize().y - screenPosition.y;
    mLabelLayer->GetElementsAt(NBGM_LET_STATIC_POI, sp, mPickingRadius, pois);
}

float NBGM_MapViewImpl::GetCameraHeight() const
{
    return static_cast<float>(mNBGMContext.ModelToWorld(mCamera->Position().z));
}

void NBGM_MapViewImpl::SetSkySize(float width, float height, float distance)
{
    if(mSky != NULL)
    {
        mSky->SetWallSize(width, height, distance);
    }

    if(mTransparentLayer != NULL)
    {
        mTransparentLayer->SetWallSize(width, height, distance);
    }

    if (mLabelTransparentSky != NULL) {
        mLabelTransparentSky->SetWallSize(width, height, distance);
    }
}

void NBGM_MapViewImpl::SetEndFlagLocation(const NBGM_Location64& location)
{
    if (mEndFlag == NULL)
    {
        return;
    }

    NBRE_Vector3d pos(location.position.x, location.position.y, location.position.z);
    pos.x = mNBGMContext.WorldToModel(pos.x);
    pos.y = mNBGMContext.WorldToModel(pos.y);
    pos.z = mNBGMContext.WorldToModel(pos.z);
    mEndFlag->SetPosition(pos);
}

void NBGM_MapViewImpl::SetStartFlagLocation(const NBGM_Location64& location)
{
    if (mStartFlag == NULL)
    {
        return;
    }

    NBRE_Vector3d pos(location.position.x, location.position.y, location.position.z);
    pos.x = mNBGMContext.WorldToModel(pos.x);
    pos.y = mNBGMContext.WorldToModel(pos.y);
    pos.z = mNBGMContext.WorldToModel(pos.z);
    mStartFlag->SetPosition(pos);
}

void NBGM_MapViewImpl::RefreshNavPois(const NBRE_Vector<NBGM_Poi*>& pois)
{
    if (mNavIconLayer == NULL)
    {
        return;
    }

    mNavIconLayer->Refresh(pois);
}

PAL_Error NBGM_MapViewImpl::AddNavVectorRoutePolyline(const NBGM_VectorRouteData* routeData )
{
    if (mNavVecRouteLayer == NULL)
    {
        return PAL_ErrNoInit;
    }

    if (routeData == NULL)
    {
        return PAL_ErrBadParam;
    }
    //TODO: world to model
    return mNavVecRouteLayer->AddManeuver(NBRE_Point2d(routeData->offset[0], routeData->offset[1]), routeData->data, routeData->count, routeData->maneuverID);
}

void NBGM_MapViewImpl::SetNavVectorCurrentManeuver( uint32 currentManuever )
{
    if (mNavVecRouteLayer == NULL)
    {
        return;
    }

    mNavVecRouteLayer->SetCurrentManeuver(currentManuever);
}

uint32 NBGM_MapViewImpl::NavVectorCurrentManeuver()
{
    if (mNavVecRouteLayer == NULL)
    {
        return 0;
    }

    return mNavVecRouteLayer->CurrentManeuver();
}

void NBGM_MapViewImpl::ResetNavVectorRouteLayer()
{
    if (mNavVecRouteLayer == NULL)
    {
        return;
    }

    mNavVecRouteLayer->Reset();
}

void NBGM_MapViewImpl::SetAvatarModelSizeScale(float s)
{
    if(mAvatar != NULL)
    {
        mAvatar->SetModelSale(s);
    }
}

void NBGM_MapViewImpl::SetOrthoCameraSetting(NBGM_CameraSetting setting)
{
    NBRE_Vector2i size = mSubView->Viewport().GetRect().GetSize();
    int32 screenWidth = size.x;
    int32 screenHeight = size.y;

    float cameraHeight = mNBGMContext.WorldToModel(setting.cameraHeight);
    float avatarToBottom = setting.avatarToBottom;
    float hfov = setting.hfov;

    float halfWidth = (float) (cameraHeight * tan(NBRE_Math::DegToRad(hfov * 0.5)));
    float height = halfWidth * 2 * screenHeight / screenWidth;
    float avatarHeight = height * avatarToBottom / screenHeight;

    NBRE_Vector3f eye(0.0f, 0.0f, cameraHeight);
    NBRE_Vector3f at(0.0f, 0.0f, 0.0f);
    NBRE_Vector3f up(0.0f, 1.0f, 0.0f);
    mCamera->LookAt(eye, at, up);

    NBRE_OrthoConfig orthoConfig = mCamera->GetFrustum().GetOrthoConfig();
    float distance = ViewPointDistanceWithScale();
    orthoConfig.mTop = height - avatarHeight;
    orthoConfig.mBottom = -avatarHeight;
    orthoConfig.mLeft = -halfWidth;
    orthoConfig.mRight = halfWidth;
    mCamera->GetFrustum().SetAsOrtho(orthoConfig, distance / 50, distance * 4);

    if(mSky != NULL)
    {
        mSky->SetWallSize(0, 0, halfWidth * 10);
        mLabelTransparentSky->SetWallSize(0, 0, halfWidth * 10);
    }
}

void NBGM_MapViewImpl::SetPerspectiveCameraSetting(NBGM_CameraSetting setting)
{
    NBRE_Vector2i size = mSubView->Viewport().GetRect().GetSize();
    int32 screenWidth = size.x;
    int32 screenHeight = size.y;

    float cameraHeight = mNBGMContext.WorldToModel(setting.cameraHeight);
    float cameraToAvatar = mNBGMContext.WorldToModel(setting.cameraToAvatar);
    float avatarToHorizon = mNBGMContext.WorldToModel(setting.avatarToHorizon);
    float avatarToBottom = setting.avatarToBottom;
    float hfov = setting.hfov;

    double halfVFov = nsl_atan(tan(NBRE_Math::DegToRad(hfov * 0.5)) * screenHeight / screenWidth);
    double d = screenHeight * 0.5 / nsl_tan(halfVFov);
    double angleAvatarToEye = nsl_atan((screenHeight * 0.5 - avatarToBottom) / d);
    double angleEyeToAvatar = nsl_atan(cameraToAvatar / cameraHeight);
    double angleEye = angleAvatarToEye + angleEyeToAvatar;
    double eyeToViewPoint = NBRE_Math::Abs(cameraHeight * nsl_tan(angleEye));

    float viewPointOffset = (float) (eyeToViewPoint - cameraToAvatar);
    NBRE_Vector3f eye(0.0f, -cameraToAvatar, cameraHeight);
    NBRE_Vector3f at(0.0f, viewPointOffset, 0.0f);
    NBRE_Vector3f up(0.0f, 0.0f, 1.0f);
    mCamera->LookAt(eye, at, up);

    float distance = ViewPointDistanceWithScale();

    NBRE_PerspectiveConfig perspectiveConfig = mCamera->GetFrustum().GetPerspectiveConfig();
    perspectiveConfig.mAspect = (float) screenWidth / screenHeight;
    perspectiveConfig.mFov = (float) NBRE_Math::RadToDeg(halfVFov * 2.0f);
    mCamera->GetFrustum().SetAsPerspective(perspectiveConfig, distance / 50, distance * 400);

    if(mSky != NULL)
    {
        SetHorizonDistance(mNBGMContext.ModelToWorld(avatarToHorizon));
    }
}

PAL_Error NBGM_MapViewImpl::AddNavEcmRouteSpline( const NBGM_SplineData* data )
{
    if (mNavEcmRouteLayer == NULL)
    {
        return PAL_ErrNoInit;
    }

    return mNavEcmRouteLayer->AppendSpline(data);
}

PAL_Error NBGM_MapViewImpl::SetNavEcmCurrentManeuver(uint32 currentManueverID)
{
    if (mNavEcmRouteLayer == NULL)
    {
        return PAL_ErrNoInit;
    }

    return mNavEcmRouteLayer->SetCurrentManeuver(currentManueverID);
}

void NBGM_MapViewImpl::ResetNavEcmRouteLayer()
{
    if (mNavEcmRouteLayer == NULL)
    {
        return;
    }

    mNavEcmRouteLayer->RemoveAllSplines();
}

PAL_Error NBGM_MapViewImpl::AddNavEcmManeuver(uint32 maneuverID, const NBGM_Point3d& position)
{
    if (mNavEcmRouteLayer == NULL)
    {
        return PAL_ErrNoInit;
    }

    return mNavEcmRouteLayer->AddManeuver(maneuverID, NBRE_Point3f(position.x, position.y, position.z));
}

PAL_Error
NBGM_MapViewImpl::NavEcmSnapRoute( NBGM_Location64& location, NBGM_SnapRouteInfo& snapRouteInfo ) const
{
    if (mNavEcmRouteLayer != NULL)
    {
        return mNavEcmRouteLayer->SnapRoute(location, snapRouteInfo);
    }
    return PAL_ErrBadParam;
}

NBGM_SnapRouteInfo&
NBGM_MapViewImpl::NavEcmGetAvatarSnapRouteInfo()
{
    return mAvatarSnapRouteInfo;
}

PAL_Error
NBGM_MapViewImpl::NavEcmSnapToRouteStart( NBGM_Location64& location, NBGM_SnapRouteInfo& pRouteInfo ) const
{
    if (mNavEcmRouteLayer == NULL)
    {
        return PAL_ErrBadParam;
    }

    return mNavEcmRouteLayer->SnapToRouteStart(location, pRouteInfo);
}

void
NBGM_MapViewImpl::SetIsNavMode(nb_boolean value)
{
    mNBGMContext.layoutManager->SetIsNavMode(value);
}

void
NBGM_MapViewImpl::AdjustFlagSize(float size, float minPixelSize, float maxPixelSize)
{
    size = mNBGMContext.WorldToModel(size);

    if(mStartFlag)
    {
        mStartFlag->AdjustSize(size, minPixelSize, maxPixelSize);
    }

    if(mEndFlag)
    {
        mEndFlag->AdjustSize(size, minPixelSize, maxPixelSize);
    }
}

void
NBGM_MapViewImpl::DumpProfile(NBGM_MapViewProfile& profile)
{
    mProfiler.Dump(profile);
}

nb_boolean
NBGM_MapViewImpl::GetAvatarScreenPosition(NBRE_Point2f& screenPosition)
{
    if(mAvatar)
    {
        if (mAvatar->GetScreenPos(screenPosition))
        {
            screenPosition.y = mSubView->Viewport().GetRect().GetSize().y - screenPosition.y;
            return TRUE;
        }
    }
    return FALSE;
}

nb_boolean
NBGM_MapViewImpl::IsAvatarInScreen(const NBRE_Vector3d& position)
{
    if(mAvatar)
    {
        return mAvatar->IsAvatarInScreen(position);
    }
    return FALSE;
}

void
NBGM_MapViewImpl::CheckBoundary()
{
    NBRE_Vector<NBGM_Point2d64> position;
    nb_boolean result = GetFrustumPositionInWorld(-1, position);
    if(result)
    {
        NBGM_TileMapBoundaryExtendType type = NBGM_TMBET_NONE;
        for(NBRE_Vector<NBGM_Point2d64>::iterator iter = position.begin(); iter != position.end(); ++iter)
        {
            if((*iter).x > NBRE_Math::Pi)
            {
                type = NBGM_TMBET_RIGHT;
                break;
            }
            else if((*iter).x < (-NBRE_Math::Pi))
            {
                type = NBGM_TMBET_LEFT;
                break;
            }
            else
            {
                type = NBGM_TMBET_NONE;
            }
        }
        mTileMapLayer->NotifyExtend(type);
        mLabelLayer->NotifyExtend(type);
        mPoiLayer->NotifyExtend(type);
        if(mAvatar)
        {
            mAvatar->NotifyExtend(type);
        }

        if(mStartFlag)
        {
            mStartFlag->NotifyExtend(type);
        }
        if(mEndFlag)
        {
            mEndFlag->NotifyExtend(type);
        }
        if(mNavIconLayer)
        {
            mNavIconLayer->NotifyExtend(type);
        }
        for(uint32 i=0; i<mCreatedCircles.size(); ++i)
        {
            NBGM_CircleImpl* circle = mCreatedCircles[i];
            if(circle != NULL)
            {
                circle->NotifyExtend(type);
            }
        }
        for(uint32 i=0; i<mCreatedRect2ds.size(); ++i)
        {
            NBGM_Rect2dImpl* rect = mCreatedRect2ds[i];
            if(rect != NULL)
            {
                rect->NotifyExtend(type);
            }
        }

        for (NBRE_Map<NBRE_String, NBGM_Icon*>::iterator iter = mCreatedIcons.begin(); iter != mCreatedIcons.end(); ++iter)
        {
            if (iter->second)
            {
                iter->second->NotifyExtend(type);
            }
        }
    }
}

void
NBGM_MapViewImpl::RefreshFontMaterial(const NBRE_String& materialCategoryName, const NBRE_Map<uint32, NBRE_FontMaterial>& materials)
{
    if (mLabelLayer)
    {
        mLabelLayer->RefreshFontMaterial(materialCategoryName, materials);
    }
}

float
NBGM_MapViewImpl::GetCameraFov() const
{
    const NBRE_Frustum& frustum = mCamera->GetFrustum();
    if (frustum.GetProjectionType() == NBRE_PT_PERSPECTIVE)
    {
        return frustum.GetPerspectiveConfig().mFov;
    }
    else
    {
        return 90.0f;
    }
}

float
NBGM_MapViewImpl::CalculateViewPointSkyHeight()
{
    float skyHeight = 0.f;
    float viewPointHight = static_cast<float> (mSubView->Viewport().GetRect().GetSize().y);
    if(mSky == NULL)
    {
        return skyHeight;
    }
    float cameraToHorizon = mSky->GetWallDistance();
    NBRE_Frustum& frustum = mCamera->GetFrustum();
    if(frustum.GetProjectionType() == NBRE_PT_PERSPECTIVE)
    {
        NBRE_Vector3f dir = mCamera->Direction();
        dir.Normalise();
        float tiltAngle = NBRE_Math::RadToDeg(nsl_acos(dir.DotProduct(NBRE_Vector3f(0, 0, 1))));
        float bottomAngle = tiltAngle - frustum.GetPerspectiveConfig().mFov/2.f;
        float topAngle = tiltAngle + frustum.GetPerspectiveConfig().mFov/2.f;
        float bottomDistance = static_cast<float>(nsl_tan(NBRE_Math::DegToRad(bottomAngle))*mCamera->WorldPosition().z);
        float topDistance = static_cast<float>(nsl_tan(NBRE_Math::DegToRad(topAngle))*mCamera->WorldPosition().z);
        if(bottomAngle >= 90.f)
        {
            return skyHeight;
        }
        if( (topAngle >= 90.f || cameraToHorizon < topDistance) && cameraToHorizon > bottomDistance)
        {
            float horizonAngle = NBRE_Math::RadToDeg(static_cast<float>(nsl_atan(cameraToHorizon / mCamera->WorldPosition().z)));
            float deltaAngle = horizonAngle - tiltAngle;
            float deltaLength = nsl_tan(NBRE_Math::DegToRad(deltaAngle))*(viewPointHight/2.f)/nsl_tan(NBRE_Math::DegToRad(frustum.GetPerspectiveConfig().mFov/2.f));
            skyHeight = viewPointHight/2.f - deltaLength;
            return skyHeight;
        }
    }
    return skyHeight;
}

void
NBGM_MapViewImpl::RefreshLabelLayout()
{
    mNBGMContext.layoutManager->Invalidate();
}

NBGM_TileAnimationLayer*
NBGM_MapViewImpl::CreateTileAnimationLayer()
{
    NBGM_TileAnimationLayerImpl* layer = NBRE_NEW NBGM_TileAnimationLayerImpl(this, mTileMapLayer);
    mTileAnimationLayers.push_back(layer);
    return layer;
}

void
NBGM_MapViewImpl::RemoveTileAnimationLayer(NBGM_TileAnimationLayer* layer)
{
    for (NBRE_Vector<NBGM_TileAnimationLayerImpl*>::iterator i = mTileAnimationLayers.begin(); i != mTileAnimationLayers.end(); ++i)
    {
        if (*i == layer)
        {
            (*i)->UnloadAll();
            NBRE_DELETE *i;
            mTileAnimationLayers.erase(i);
            break;
        }
    }
}

NBGM_OptionalLayer*
NBGM_MapViewImpl::CreateOptionalLayer()
{
    NBGM_OptionalLayerImpl* layer = NBRE_NEW NBGM_OptionalLayerImpl(this, mTileMapLayer);
    mOptionalLayers.push_back(layer);
    return layer;
}

void
NBGM_MapViewImpl::RemoveOptionalLayer(NBGM_OptionalLayer* layer)
{
    for (NBRE_Vector<NBGM_OptionalLayerImpl*>::iterator i = mOptionalLayers.begin(); i != mOptionalLayers.end(); ++i)
    {
        if (*i == layer)
        {
            NBRE_DELETE *i;
            mOptionalLayers.erase(i);
            break;
        }
    }
}

nb_boolean
NBGM_MapViewImpl::SelectAndTrackAvatar(float x, float y)
{
    if (!mAvatar)
    {
        return FALSE;
    }

    NBRE_Ray3d ray = mSubView->Viewport().GetRay(x, y);
    NBRE_AxisAlignedBox3d aabb = mAvatar->AxisAlignedBox();
    return NBRE_Intersectiond::HitTest(aabb, ray, NULL);
}

void
NBGM_MapViewImpl::AdjustLabelSkyDrawOrder(uint8 drawOrder)
{
    if(mLabelTransparentSky)
    {
        if(mLabelTransparentSkyDrawOrder.mainDrawOrder != drawOrder)
        {
            if(mSubView)
            {
                mOverlayManager->RemoveOverlay(mLabelTransparentSky->OverlayId());
                mLabelTransparentSkyDrawOrder.mainDrawOrder = drawOrder;
                int32 overlayId = mOverlayManager->AssignOverlayId(mLabelTransparentSkyDrawOrder);
                mOverlayManager->FindOverlay(overlayId)->SetClearFlag(NBRE_CLF_CLEAR_DEPTH);
                mLabelTransparentSky->SetOverlayId(overlayId);
            }
        }
    }
}

void
NBGM_MapViewImpl::RenderCallBack(PAL_Instance * /*pal*/, void *userData, PAL_TimerCBReason reason)
{
    NBGM_MapViewImpl* impl = (NBGM_MapViewImpl*) userData;
    if(impl!= NULL && reason == PTCBR_TimerFired)
    {
        impl->AddTask(NBRE_NEW NBGM_RenderTask(impl));
    }
}

NBGM_Context&
NBGM_MapViewImpl::GetNBGMContext()
{
    return mNBGMContext;
}

NBRE_Node*
NBGM_MapViewImpl::GetRootNode() const
{
    return mSceneManager->RootSceneNode();
}

NBRE_SurfaceSubView*
NBGM_MapViewImpl::GetSurfaceSubView() const
{
    return mSubView;
}

NBRE_DefaultOverlayManager<DrawOrderStruct>&
NBGM_MapViewImpl::GetOverlayManager()
{
    return *mOverlayManager;
}


void
NBGM_MapViewImpl::AddTask(NBGM_Task* task)
{
    mRenderThread->AddTask(task);
}

NBGM_DeviceLocation* NBGM_MapViewImpl::CreateDeviceLocation( std::string ID )
{
    NBGM_Avatar* avatar = NBRE_NEW NBGM_Avatar(mNBGMContext,
        mSceneManager,
        mSubView,
        *mOverlayManager,
        mAvatarDrawOrder,
        mGlowLayer->BuildingOverlayId(),
        NBGM_AM_MAP_FOLLOW_ME,
        mConfig.avatarSizeFactor,
        mConfig.referenceAvatarHeight,
        NBRE_Vector3f(0, 0, 0),
        ID);
    avatar->AddToSence();
    mCreatedAvatar.push_back(avatar);

    return avatar;
}

void NBGM_MapViewImpl::ReleaseDeviceLocation(NBGM_DeviceLocation* device)
{
    NBRE_Vector<NBGM_Avatar*>::iterator itor = mCreatedAvatar.begin();
    for(; itor!=mCreatedAvatar.end(); ++itor)
    {
        if((*itor)->ID() == device->ID())
        {
            mCreatedAvatar.erase(itor);
            NBRE_DELETE (NBGM_Avatar*)device;
            break;
        }
    }
}

PAL_Error
NBGM_MapViewImpl::SnapScreenshot(int x, int y, uint32 width, uint32 height, uint8* buffer) const
{
    if(!mInitialized)
    {
        return PAL_ErrNoInit;
    }
    mNBGMContext.renderContext->SwitchTo();
    mNBGMContext.renderingEngine->Context().mRenderPal->ReadPixels(x, y, width, height, buffer);
    return PAL_Ok;
}

void
NBGM_MapViewImpl::SnapScreenshot()
{
    if(mSnapScreenshot)
    {
        //perpare render
        if(mNBGMContext.renderContext)
        {
            mNBGMContext.renderContext->SwitchTo();
        }
        while(mNBGMContext.layoutManager->Update(0));

        if(mLayoutTaskCount == 0)
        {
            RenderFrame();

            if(mNBGMContext.renderContext)
            {
                mNBGMContext.renderContext->BeginRender();
            }
            if(mNBGMContext.renderContext)
            {
                mNBGMContext.renderContext->EndRender();
            }
            mWaitingLayoutFinished = FALSE;
            NBRE_Vector2i size = mSubView->Viewport().GetRect().GetSize();
            uint8* buffer = NBRE_NEW uint8[size.x*size.y*4];
            SnapScreenshot(0, 0, size.x, size.y, buffer);
            shared_ptr<NBGM_AsyncSnapScreenshotCallback> snapScreenshot = mSnapScreenshot;
//            mSnapScreenshot = NULL;
            mSnapScreenshot.reset();
            snapScreenshot->OnScreenshotFinished(size.x, size.y, buffer, size.x*size.y*4);
            NBRE_DELETE_ARRAY buffer;
        }
        else
        {
            mWaitingLayoutFinished = TRUE;
        }
    }
}

void
NBGM_MapViewImpl::GetElementsAt(const NBRE_Point2f& screenPosition, uint32 selectMask, NBRE_Set<NBGM_ElementId>& objectIds)
{
    NBRE_Point2f sp = screenPosition;

    if(sp.y > mSkyHeightOnViewport)
    {
        NBRE_Ray3d ray = mSubView->Viewport().GetRay(sp.x, sp.y);

        NBRE_Planed plane(NBRE_Vector3d(0, 0, 1), 0);

        NBRE_Vector3d wp;
        if(NBRE_Intersectiond::HitTest(plane, ray, &wp))
        {
            double ppu = 0;
            if (mTransUtil->PixelsPerUnit(NBRE_Vector3d(wp.x, wp.y, 0), ppu))
            {
                double r = mPickingRadius / ppu;
                mTileMapLayer->HitTest(NBRE_Vector2d(wp.x, wp.y), r, selectMask, objectIds);
            }
        }
    }
}

void
NBGM_MapViewImpl::EnableCollisionDetection(const NBRE_String& layerId, nb_boolean enable)
{
    if(!mInitialized || !mLabelLayer)
    {
        return;
    }

    mLabelLayer->EnableLayerLayout(layerId, enable);
    InvalidateLayout();
    RequestRenderFrame();
}

PAL_Error
NBGM_MapViewImpl::AsyncSnapScreenshot(const shared_ptr<NBGM_AsyncSnapScreenshotCallback>& snapScreenshot)
{
    if(!mInitialized)
    {
        return PAL_ErrNoInit;
    }
    if(mSnapScreenshot)
    {
        return PAL_ErrAlreadyExist;
    }
    mSnapScreenshot = snapScreenshot;

    NBGM_LoadingThreadScreenshotTask* task = NBRE_NEW NBGM_LoadingThreadScreenshotTask(mRenderThread, this);
    PAL_Error err = mConfig.resourceManager->GetLoadingTaskQueue()->AddTask(task);
    return err;
}

void
NBGM_MapViewImpl::Render()
{
    uint32 beginTime = PAL_ClockGetTimeMs();
    if(mTimerCallback && mTimerUserData)
    {
        mTimerCallback(NULL, mTimerUserData, PTCBR_TimerFired);
    }

    if(mIsInvalidate || (!mNBGMContext.layoutManager->IsNavMode() && mIsLayoutInvalidate))
    {
        nb_boolean needMoreRenderCycle = RenderFrame();
        if(needMoreRenderCycle)
        {
            RequestRenderFrame();
        }
    }

    if (!mNBGMContext.layoutManager->IsNavMode() && mIsLayoutInvalidate)
    {
        RequestRenderFrame();
    }

    uint32 cost = PAL_ClockGetTimeMs() - beginTime;
    uint32 interval = mConfig.renderInterval - (cost%mConfig.renderInterval);
    ScheduleTimer(interval, RenderCallBack, this);
}

void
NBGM_MapViewImpl::EnableRenderLog()
{
    mEnableRenderLog = TRUE;
}
void
NBGM_MapViewImpl::AddLayoutTaskRef()
{
    ++mLayoutTaskCount;
}
void
NBGM_MapViewImpl::ReleaseLayoutTaskRef()
{
    --mLayoutTaskCount;
    if(mLayoutTaskCount == 0)
    {
        if(mWaitingLayoutFinished)
        {
            SnapScreenshot();
        }
    }
}

PAL_Error
NBGM_MapViewImpl::ScheduleTimer(int32 milliseconds, PAL_TimerCallback* callback, void* userData)
{
    // @todo: do we need to check if milliseconds is zero or not?
    return PAL_TimerSetWithQueueHandle(mNBGMContext.renderingEngine->Context().mPalInstance,
                                       mRenderThreadHandle, milliseconds, callback, userData);
}

/* See description in header file. */
PAL_Error
NBGM_MapViewImpl::CancelTimer(PAL_TimerCallback* callback, void* userData)
{
    return PAL_TimerCancel(mNBGMContext.renderingEngine->Context().mPalInstance,
                           callback, userData);
}

void NBGM_MapViewImpl::AddTimerCallback(PAL_TimerCallback *callback, void *userData)
{
    mTimerCallback = callback;
    mTimerUserData = userData;
}

void
NBGM_MapViewImpl::RefreshTiles()
{
    NBRE_Vector<NBGM_VectorTile*> allTiles;
    mTileIdCache->GetAllTiles(allTiles);
    uint8 labelSkyDrawOrder = 255;
    uint8 labelDrawOrder = 0;
    nb_boolean haveLabelDrawOrder = FALSE;
    for(NBRE_Vector<NBGM_VectorTile*>::iterator iter = allTiles.begin(); iter != allTiles.end(); iter++)
    {
        NBGM_VectorTile* tile = *iter;
        labelDrawOrder = tile->LabelDrawOrder();
        if(labelDrawOrder) // the tile will be ignored if it has no label draw order
        {
            labelSkyDrawOrder = NBRE_Math::Min(labelSkyDrawOrder, static_cast<uint8>(labelDrawOrder - 1)); // make label sky wall draw order be in front of label draw order
            haveLabelDrawOrder = TRUE;
        }
    }
    if(haveLabelDrawOrder)
    {
        AdjustLabelSkyDrawOrder(labelSkyDrawOrder);
    }

    RequestRenderFrame();
    InvalidateLayout();
}

NBGM_MapViewImpl::ResourceCallBack::ResourceCallBack(NBGM_MapViewImpl* impl)
    :mMapViewImpl(impl)
{
}

NBGM_MapViewImpl::ResourceCallBack::~ResourceCallBack()
{
}

void
NBGM_MapViewImpl::ResourceCallBack::OnResponseNBMData(const NBRE_Vector<NBGM_NBMDataLoadResult>& result)
{
    for(NBRE_Vector<NBGM_NBMDataLoadResult>::const_iterator iter = result.begin(); iter != result.end(); ++iter)
    {
        NBGM_NBMDataLoadResult loadResult = *iter;
        if(loadResult.tileInfo.enableLog)
        {
            mMapViewImpl->EnableRenderLog();
        }

        NBGM_VectorTile* tile = mMapViewImpl->mTileIdCache->GetTile(loadResult.tileInfo.id);
        if(tile)
        {
            if((tile->BaseDrawOrder() == loadResult.tileInfo.baseDrawOrder) && (tile->SubDrawOrder() == loadResult.tileInfo.subDrawOrder) && (tile->LabelDrawOrder() == loadResult.tileInfo.labelDrawOrder))
            {
                mMapViewImpl->mTileIdCache->ActiveTile(loadResult.tileInfo.id);
            }
            else
            {
                mMapViewImpl->mTileIdCache->UpdateTile(loadResult.tileInfo.id, mMapViewImpl->mMapLayers, loadResult.nbmData.get(), *mMapViewImpl->mOverlayManager, loadResult.tileInfo.baseDrawOrder, loadResult.tileInfo.subDrawOrder, loadResult.tileInfo.labelDrawOrder
                , loadResult.tileInfo.enablePicking, loadResult.tileInfo.selectMask, loadResult.tileInfo.materialCategoryName);
                mMapViewImpl->mTileIdCache->ActiveTile(loadResult.tileInfo.id);
            }
        }
        else if(loadResult.nbmData != NULL)
        {
            NBRE_Vector<NBGM_VectorTileID> releasedTiles;
            mMapViewImpl->mTileIdCache->LoadTile(loadResult.tileInfo.id, mMapViewImpl->mMapLayers, loadResult.nbmData.get(), *mMapViewImpl->mOverlayManager, loadResult.tileInfo.baseDrawOrder, loadResult.tileInfo.subDrawOrder, loadResult.tileInfo.labelDrawOrder
                , loadResult.tileInfo.enablePicking, loadResult.tileInfo.selectMask, loadResult.tileInfo.materialCategoryName, releasedTiles);
            mMapViewImpl->mConfig.resourceManager->ReleaseData(releasedTiles);
        }

        if(loadResult.tileInfo.loadListener)
        {
            loadResult.tileInfo.loadListener->OnLoadingFinished(loadResult.tileInfo.id);
        }
    }

    for(NBRE_Vector<NBRE_String>::iterator iter = mUnloadTileNames.begin(); iter != mUnloadTileNames.end(); ++iter)
    {
        NBRE_String& nbmName = *iter;
        nb_boolean needRefresh = FALSE;
        if(mMapViewImpl->mTileIdCache->UnloadTile(nbmName, needRefresh))
        {
            mMapViewImpl->mConfig.resourceManager->ReleaseData(nbmName);
        }
    }

    mMapViewImpl->RefreshTiles();
    NBRE_DELETE this;
}

void
NBGM_MapViewImpl::ResourceCallBack::OnUpdateNBMData(const NBRE_Vector<NBGM_NBMDataLoadResult>& result)
{
    for(NBRE_Vector<NBGM_NBMDataLoadResult>::const_iterator iter = result.begin(); iter != result.end(); ++iter)
    {
        NBGM_NBMDataLoadResult loadResult = *iter;
        if(loadResult.tileInfo.enableLog)
        {
            mMapViewImpl->EnableRenderLog();
        }

        if(loadResult.nbmData != NULL)
        {
            mMapViewImpl->mTileIdCache->UpdateTile(loadResult.tileInfo.id, mMapViewImpl->mMapLayers, loadResult.nbmData.get(), *mMapViewImpl->mOverlayManager, loadResult.tileInfo.baseDrawOrder, loadResult.tileInfo.subDrawOrder, loadResult.tileInfo.labelDrawOrder
                , loadResult.tileInfo.enablePicking, loadResult.tileInfo.selectMask, loadResult.tileInfo.materialCategoryName);
        }

        if(loadResult.tileInfo.loadListener)
        {
            loadResult.tileInfo.loadListener->OnLoadingFinished(loadResult.tileInfo.id);
        }
    }

    mMapViewImpl->RefreshTiles();
    NBRE_DELETE this;
}

PAL_Error
NBGM_MapViewImpl::ReloadNBMTile(const NBGM_NBMDataLoadInfo& info, const std::string& needUnloadedNbmName)
{
    NBRE_Vector<NBGM_NBMDataLoadInfo> tiles;
    tiles.push_back(info);

    if(info.enableLog)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "Load Start Time = %u", PAL_ClockGetTimeMs());
    }

    ResourceCallBack* callBack = NBRE_NEW ResourceCallBack(this);
    callBack->mUnloadTileNames.push_back(needUnloadedNbmName);

    mConfig.resourceManager->RequestNBMData(tiles, callBack, mRenderThread);

    return PAL_Ok;
}


PAL_Error
NBGM_MapViewImpl::UpdateNBMTile(const NBGM_NBMDataLoadInfo& info)
{
    NBRE_Vector<NBGM_NBMDataLoadInfo> tiles;
    tiles.push_back(info);
    ResourceCallBack* callBack = NBRE_NEW ResourceCallBack(this);
    if(info.stream)
    {
        mConfig.resourceManager->RequestUpdateNBMData(tiles, callBack, mRenderThread);
    }
    else
    {
        mConfig.resourceManager->RequestNBMData(tiles, callBack, mRenderThread);
    }
    return PAL_Ok;
}

nb_boolean
NBGM_MapViewImpl::TapCompass(float screenX, float screenY)
{
    if(mCompass)
    {
        return mCompass->IsTapped(screenX, screenY);
    }
    return FALSE;
}

void
NBGM_MapViewImpl::EnableCompass(nb_boolean value)
{
    if(mCompass)
    {
        mCompass->SetEnable(value);
    }
}

void
NBGM_MapViewImpl::SetCompassPosition(float screenX, float screenY)
{
    if(mCompass)
    {
        mCompass->SetPosition(screenX, screenY);
    }
}

void
NBGM_MapViewImpl::SetCompassDayNightMode(nb_boolean isDay)
{
    if(mCompass)
    {
        mCompass->SetDayNightMode(isDay);
    }
}

void
NBGM_MapViewImpl::SetCompassIcons(const NBRE_String& dayModeIconPath, const NBRE_String& nightModeIconPath)
{
    if(mCompass)
    {
        mCompass->UpdateIcons(dayModeIconPath, nightModeIconPath);
    }
}

void
NBGM_MapViewImpl::GetCompassBoundingBox(float& leftBottomX, float& leftBottomY, float& rightTopX, float& rightTopY)const
{
    if(mCompass)
    {
        NBRE_AxisAlignedBox2f box = mCompass->GetBoundingBox();
        if (!box.IsNull())
        {
            // enlarge bounds to make it easier to click
            NBRE_Vector2f r(mPickingRadius, mPickingRadius);
            box.minExtend -= r;
            box.maxExtend += r;
        }
        leftBottomX = box.minExtend.x;
        leftBottomY = box.minExtend.y;
        rightTopX = box.maxExtend.x;
        rightTopY = box.maxExtend.y;
    }
}

NBGM_Circle*
NBGM_MapViewImpl::AddCircle(int circleId, const NBGM_CircleParameters &circlePara)
{
    NBGM_CircleImpl* circle = NBRE_NEW NBGM_CircleImpl(mNBGMContext,
        mSceneManager,
        mSubView,
        *mOverlayManager,
        mCustomObj2DDrawOrder,
        circleId,
        circlePara);

    if(circle != NULL)
    {
        circle->AddToSence();
        mCreatedCircles.push_back(circle);
    }

    return circle;
}

void
NBGM_MapViewImpl::RemoveCircle(NBGM_Circle* circle)
{
    if(circle == NULL)
    {
        return;
    }
    NBRE_Vector<NBGM_CircleImpl*>::iterator itor = mCreatedCircles.begin();
    for(; itor != mCreatedCircles.end(); ++itor)
    {
        if((*itor) == circle)
        {
            mCreatedCircles.erase(itor);
            NBRE_DELETE (NBGM_CircleImpl*)circle;
            break;
        }
    }
}

void
NBGM_MapViewImpl::RemoveAllCircles()
{
    NBRE_Vector<NBGM_CircleImpl*>::iterator itor = mCreatedCircles.begin();
    for(; itor != mCreatedCircles.end(); ++itor)
    {
        NBRE_DELETE (NBGM_CircleImpl*)(*itor);
    }
    mCreatedCircles.clear();
}

NBGM_CustomRect2d*
NBGM_MapViewImpl::AddRect2d(NBGM_Rect2dId id, const NBGM_Rect2dParameters &para)
{
    NBGM_Rect2dImpl* rect = NBRE_NEW NBGM_Rect2dImpl(mNBGMContext,
        mSceneManager,
        mSubView,
        *mOverlayManager,
        mCustomObj2DDrawOrder,
        mCustomObjectesContext,
        id,
        para);

    if(rect != NULL)
    {
        rect->AddToSence();
        mCreatedRect2ds.push_back(rect);
    }

    return rect;
}

void
NBGM_MapViewImpl::RemoveRect2d(NBGM_CustomRect2d* rect)
{
    if(rect == NULL)
    {
        return;
    }
    NBRE_Vector<NBGM_Rect2dImpl*>::iterator itor = mCreatedRect2ds.begin();
    for(; itor != mCreatedRect2ds.end(); ++itor)
    {
        if((*itor) == rect)
        {
            mCreatedRect2ds.erase(itor);
            NBRE_DELETE (NBGM_Rect2dImpl*)rect;
            break;
        }
    }
}

void
NBGM_MapViewImpl::RemoveAllRect2ds()
{
    NBRE_Vector<NBGM_Rect2dImpl*>::iterator itor = mCreatedRect2ds.begin();
    for(; itor != mCreatedRect2ds.end(); ++itor)
    {
        NBRE_DELETE (NBGM_Rect2dImpl*)(*itor);
    }
    mCreatedRect2ds.clear();
}

bool
NBGM_MapViewImpl::AddTexture(NBGM_TextureId id, const NBGM_BinaryBuffer &textureData)
{
    return mCustomObjectesContext.AddTexture(id, textureData);
}

void
NBGM_MapViewImpl::RemoveTexture(NBGM_TextureId id)
{
    mCustomObjectesContext.RemoveTexture(id);
}

void
NBGM_MapViewImpl::RemoveAllTextures()
{
    mCustomObjectesContext.RemoveAllTextures();
}

void
NBGM_MapViewImpl::SetRenderInterval(uint32 interval)
{
    mConfig.renderInterval = interval;
}

void
NBGM_MapViewImpl::AddPins(const NBRE_Vector<NBGM_PinParameters>& pinParameters)
{
    for (uint32 i = 0; i < pinParameters.size(); ++i)
    {
        NBGM_PinParameters pinParameter = pinParameters[i];

        if (mCreatedIcons.find(pinParameter.pinID) != mCreatedIcons.end())
        {
            return;
        }
        NBRE_Vector3d position(mNBGMContext.WorldToModel(pinParameter.locationX), mNBGMContext.WorldToModel(pinParameter.locationY), 0);
        NBRE_NodePtr iconNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
        GetRootNode()->AddChild(iconNode);
        NBGM_Icon* icon = NULL;

        if (VERIFY_NBM_INTERNAL_INDEX(pinParameter.pinMaterialIndex))
        {
            NBRE_Point2f imageOffset(pinParameter.unSelectedCalloutXCenterOffset/100.f, pinParameter.unSelectedCalloutYCenterOffset/100.f);
            NBRE_Point2f selectedImageOffset(pinParameter.selectedCalloutXCenterOffset/100.f, pinParameter.selectedCalloutYCenterOffset/100.f);
            NBRE_Point2f bubbleOffset(pinParameter.selectedBubbleXCenterOffset/100.f, pinParameter.selectedBubbleYCenterOffset/100.f);

            bubbleOffset -= selectedImageOffset;

            imageOffset.x = imageOffset.x - 0.5f;
            imageOffset.y = 0.5f - imageOffset.y;
            imageOffset.x *= -1.f;
            imageOffset.y *= -1.f;

            selectedImageOffset.x = selectedImageOffset.x - 0.5f;
            selectedImageOffset.y = 0.5f - selectedImageOffset.y;
            selectedImageOffset.x *= -1.f;
            selectedImageOffset.y *= -1.f;

            bubbleOffset.x *= -1.f;
            bubbleOffset.y *= -1.f;

            NBRE_Vector2f size(DP_TO_PIXEL(pinParameter.displayunSelectedWidth), DP_TO_PIXEL(pinParameter.dispalyunSelectedHeight));
            NBRE_Vector2f selectedTexSize(DP_TO_PIXEL(pinParameter.displaySelectedWidth), DP_TO_PIXEL(pinParameter.dispalySelectedHeight));

            icon = NBRE_NEW NBGM_Icon(mNBGMContext,
                                      iconNode.get(),
                                      mBillboardSet,
                                      size,
                                      selectedTexSize,
                                      position,
                                      pinParameter.pinID,
                                      TRUE,
                                      imageOffset,
                                      selectedImageOffset,
                                      bubbleOffset);

            if (icon)
            {
                icon->Initialize(pinParameter.unSelectedDataBinary,
                                 pinParameter.selectedDataBinary,
                                 NBRE_MapMaterialUtility::MakeColor(pinParameter.unSelectedCircleInteriorColor),
                                 NBRE_MapMaterialUtility::MakeColor(pinParameter.unSelectedCircleOutlineBitOnColor),
                                 NBRE_MapMaterialUtility::MakeColor(pinParameter.unSelectedCircleOutlineBitOffColor),
                                 NBRE_MapMaterialUtility::MakeColor(pinParameter.selectedCircleInteriorColor),
                                 NBRE_MapMaterialUtility::MakeColor(pinParameter.selectedCircleOutlineBitOnColor),
                                 NBRE_MapMaterialUtility::MakeColor(pinParameter.selectedCircleOutlineBitOffColor),
                                 pinParameter.circleOutlineWidth,
                                 METER_TO_MERCATOR(mNBGMContext.WorldToModel(pinParameter.radius))
                                 );
            }
        }
        else
        {
            const NBRE_MapMateriaGroup& materialGroup = mConfig.resourceManager->GetResourceContext().mapMaterialManager->GetCurrentGroup("pin");
            const NBRE_RadialPINMaterial* material = static_cast<const NBRE_RadialPINMaterial*>(&(materialGroup.GetMapMaterial(MAKE_NBM_INDEX(pinParameter.pinMaterialIndex))));

            if (!material)
            {
                continue;
            }

            NBRE_ShaderPtr normalShader = material->NormalShader();
            NBRE_ShaderPtr selectedShader = material->SelectedShader();
            NBRE_Point2f imageOffset(material->NormalXImageOffset(), material->NormalYImageOffset());
            NBRE_Point2f selectedImageOffset(material->SelectedXImageOffset(), material->SelectedYImageOffset());
            NBRE_Point2f bubbleOffset(material->SelectedXBubbleOffset(), material->SelectedYBubbleOffset());

            bubbleOffset -= selectedImageOffset;

            imageOffset.x = imageOffset.x - 0.5f;
            imageOffset.y = 0.5f - imageOffset.y;
            imageOffset.x *= -1.f;
            imageOffset.y *= -1.f;

            selectedImageOffset.x = selectedImageOffset.x - 0.5f;
            selectedImageOffset.y = 0.5f - selectedImageOffset.y;
            selectedImageOffset.x *= -1.f;
            selectedImageOffset.y *= -1.f;

            bubbleOffset.x *= -1.f;
            bubbleOffset.y *= -1.f;

            NBRE_Vector2f size(material->GetNormalWidth(), material->GetNormalHeight());
            NBRE_Vector2f selectedTexSize(material->GetSelectedWidth(), material->GetSelectedHeight());

            icon = NBRE_NEW NBGM_Icon(mNBGMContext,
                                      iconNode.get(),
                                      mBillboardSet,
                                      size,
                                      selectedTexSize,
                                      normalShader,
                                      selectedShader,
                                      position,
                                      pinParameter.pinID,
                                      TRUE,
                                      imageOffset,
                                      selectedImageOffset,
                                      bubbleOffset);
            if (icon)
            {
                icon->InitializeHalo(material->NormalHaloInteriorShader(), material->NormalHaloOutlineShader(), METER_TO_MERCATOR(mNBGMContext.WorldToModel(pinParameter.radius)));
            }
        }

        if (icon)
        {
            icon->AddToSence();
            mCreatedIcons[pinParameter.pinID] = icon;
        }
    }
}

void
NBGM_MapViewImpl::RemovePins(const NBRE_Vector<shared_ptr<NBRE_String> >& pinIDs)
{
    for (NBRE_Vector<shared_ptr<NBRE_String> >::const_iterator iter = pinIDs.begin(); iter != pinIDs.end(); ++iter)
    {
        if (mCreatedIcons.find(**iter) != mCreatedIcons.end())
        {
            NBRE_DELETE mCreatedIcons[**iter];
            mCreatedIcons.erase(**iter);
        }
    }
}

void
NBGM_MapViewImpl::RemoveAllPins()
{
    for (NBRE_Map<NBRE_String, NBGM_Icon*>::iterator iter = mCreatedIcons.begin(); iter != mCreatedIcons.end(); ++iter)
    {
        if (iter->second)
        {
            NBRE_DELETE iter->second;
        }
    }
    mCreatedIcons.clear();

}

void
NBGM_MapViewImpl::UpdatePinPosition(shared_ptr<NBRE_String> pinID, double locationX, double locationY)
{
    if (mCreatedIcons.find(*pinID) != mCreatedIcons.end())
    {
        NBRE_Vector3d position(mNBGMContext.WorldToModel(locationX), mNBGMContext.WorldToModel(locationY), 0);
        mCreatedIcons[*pinID]->SetPosition(position);
    }
}

void
NBGM_MapViewImpl::SetHBAOParameters(const NBGM_HBAOParameters& parameters)
{
    if(mTransparentLayer)
    {
        mTransparentLayer->SetHBAOParameters(parameters);
    }
}

void
NBGM_MapViewImpl::SetGlowParameters(const NBGM_GlowParameters& parameters)
{
    if(mGlowLayer)
    {
        mGlowLayer->SetGlowParameters(parameters);
    }
}

void
NBGM_MapViewImpl::SetDPI(float dpi)
{
    if (dpi == NBGM_GetConfig()->dpi)
    {
        return;
    }

    NBGM_SetDPI(dpi);
    if (mAvatar)
    {
        mAvatar->UpdateDPI();
    }

    if (mCompass)
    {
        mCompass->UpdateDPI();
    }
}

void
NBGM_MapViewImpl::UnloadSyncTiles(const NBRE_Vector<NBGM_VectorTileID>& ids)
{
    nb_boolean needRefresh = FALSE;
    for(NBRE_Vector<NBRE_String>::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
    {
        const NBRE_String& nbmName = *iter;
        nb_boolean needRefreshTile = FALSE;

        if(mTileIdCache->UnloadTile(nbmName, needRefreshTile))
        {
            mConfig.resourceManager->ReleaseData(nbmName);
        }

        if(needRefreshTile)
        {
            needRefresh = TRUE;
        }
    }
    if(needRefresh)
    {
        RefreshTiles();
    }
}

void
NBGM_MapViewImpl::UnloadSyncAllTiles()
{
    NBRE_Vector<NBGM_VectorTileID> releasedTiles;
    mTileIdCache->UnloadAllTiles(releasedTiles);
    mConfig.resourceManager->ReleaseData(releasedTiles);

    RefreshTiles();
    if (mNBGMContext.layoutManager->IsNavMode())
    {
        mNBGMContext.layoutManager->RemoveAllElements();
    }
}
