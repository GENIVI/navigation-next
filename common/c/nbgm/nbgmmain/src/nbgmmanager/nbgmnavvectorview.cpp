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
#include "nbgmnavvectorview.h"
#include "nbrelog.h"
#include "nbgmmapviewimpl.h"
#include "nbgmnaviconlayer.h"
#include "nbgmflag.h"
#include "nbgmconst.h"
#include "nbgm.h"
#include "nbgmanimation.h"

NBGM_NavVectorView::NBGM_NavVectorView(const NBGM_NavViewConfig& config, NBRE_RenderEngine* renderEngine, NBGM_ResourceManager* resourceManager):mIsNavMainView(TRUE), mCurrentManeuverIndex(0)
{
    NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM_NavVectorView::NBGM_NavVectorView, renderEngine = %p", renderEngine);
    NBGM_MapViewImplConfig c;
    c.viewConfig = config.mapViewConfig;
    c.drawNavPoi = TRUE;
    c.drawVectorNavRoute = TRUE;
    c.drawEcmNavRoute = FALSE;
    c.modelScaleFactor = VECTOR_MAP_MODEL_SCALE_FACTOR;
    c.avatarSizeFactor = METER_TO_MERCATOR(VECTOR_AVATA_SIZE_FACTOR);
    c.referenceAvatarHeight = METER_TO_MERCATOR(REFERENCE_AVATAR_BASIC_HEIGHT);
    c.useNavRoadLabel = TRUE;
    c.maxAreaBuildingLabels = 2;
    c.maxLabels = 6;
    c.renderInterval = config.mapViewConfig.renderInterval;
    c.resourceManager = resourceManager;
    mMapView = NBRE_NEW NBGM_MapViewImpl(c, renderEngine);
//	mAnimation = NBRE_NEW NBGM_Animation(renderEngine);
    mAnimation = NULL;
    mIsRight2Left = FALSE,
    mAnimationOffset = 0.0f;
    mIsAnimating = FALSE;
    mIsUseAnimation = FALSE;
    nsl_memset(&mAvatarLocation, 0, sizeof(NBGM_Location64));
}

NBGM_NavVectorView::~NBGM_NavVectorView()
{
    NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM_NavVectorView::~NBGM_NavVectorView");
    mMapView->Finalize();
//	NBRE_DELETE mAnimation;
    NBRE_DELETE mMapView;

}

void NBGM_NavVectorView::InitializeEnvironment()
{
    NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM_NavVectorView::InitializeEnvironment");
    mMapView->InitializeEnvironment();
    mMapView->SetAvatarState(NBGM_AS_NAV);
    mMapView->SetAvatarMode(NBGM_AM_NAV_CAR);
    mMapView->SetIsNavMode(TRUE);
    mMapView->AdjustFlagSize(STANDARD_FLAG_SIZE, MM_TO_PIXEL(STANDARD_FLAG_MIN_SIZE, NBGM_GetConfig()->dpi), MM_TO_PIXEL(STANDARD_FLAG_MAX_SIZE, NBGM_GetConfig()->dpi));
//	mAnimation->Initialize();
}

void NBGM_NavVectorView::RenderFrame()
{
//	if(mIsAnimating)
//	{
//		if(mIsRight2Left)
//		{
//			mAnimationOffset += 2*secondsPassed;
//			mIsAnimating = mAnimationOffset < 1;
//		}
//		else
//		{
//			mAnimationOffset -= 2*secondsPassed;
//			mIsAnimating = mAnimationOffset > 0;
//		}
//
//		if(mIsAnimating)
//		{
//			mAnimation->Move(mAnimationOffset);
//		}
//		else
//		{
//			mMapView->ActiveSurface();
//		}
//	}

    mMapView->RenderFrame();

}

void NBGM_NavVectorView::RequestRenderFrame(NBGM_FrameListener* listener)
{
    mMapView->RequestRenderFrame(listener);
}

void NBGM_NavVectorView::SetViewSize( int32 x, int32 y, uint32 width, uint32 height )
{
    NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM_NavVectorView::SetViewSize, x = %d, y = %d, width = %u, heigth = %u", x, y, width, height);
    mMapView->SetPerspective(45, static_cast<float>(width)/height);
    mMapView->OnSizeChanged(x, y, width, height);
//	mAnimation->OnSizeChanged(x, y, width, height);

}

PAL_Error NBGM_NavVectorView::LoadCommonMaterial( const NBRE_String& categoryName, const NBRE_String& materialName, shared_ptr<NBRE_IOStream> is )
{
    mMapView->LoadExternalMaterial(categoryName, materialName, is, NBRE_Vector<NBRE_String>());
    return PAL_Ok;
}

PAL_Error NBGM_NavVectorView::SetCurrentCommonMaterial( const NBRE_String& categoryName, const NBRE_String& materialName )
{
    mMapView->SetCurrentExternalMaterial(categoryName, materialName);
    NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM_NavVectorView::SetCurrentCommonMaterial, materialName = %s", materialName.c_str());
    return PAL_Ok;
}

PAL_Error NBGM_NavVectorView::SetPalette( const NBGM_Palette& /*palette*/ )
{
    return PAL_ErrUnsupported;
}

PAL_Error NBGM_NavVectorView::UpdateExcludeRect( NBRE_Vector<NBGM_Rect2d*> /*rects*/ )
{
    return PAL_ErrUnsupported;
}


PAL_Error NBGM_NavVectorView::AddVectorRoutePolyline(const NBGM_VectorRouteData* routeData )
{
    PAL_Error err = mMapView->AddNavVectorRoutePolyline(routeData);
    return err;
}

PAL_Error NBGM_NavVectorView::LoadNBMTile( const std::string& nbmName, const NBRE_String& categoryName, uint8 baseDrawOrder, uint8 labelDrawOrder, NBRE_IOStream* is)
{
    NBGM_NBMDataLoadInfo info;
    info.id = nbmName;
    info.materialCategoryName = categoryName;
    info.baseDrawOrder = baseDrawOrder;
    info.labelDrawOrder = labelDrawOrder;
    info.stream = shared_ptr<NBRE_IOStream>(is);
    mMapView->AddTask(NBRE_NEW NBGM_LoadNBMTileTask(mMapView, info));

    return PAL_Ok;
}

void NBGM_NavVectorView::UnLoadTile( const std::string& nbmName )
{
    NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM_NavVectorView::UnLoadTile, nbmName = %s", nbmName.c_str());
    mMapView->AddTask(NBRE_NEW NBGM_UnloadTileTask(mMapView, nbmName));
}

static float GetSimpleTurnmapHeading(const NBGM_VectorRouteData* routeData)
{
    int32 numSegments = routeData->count - 4; // if first point, skip

    if(numSegments < 0)
    {
        return 0;
    }
    // length measuring
    float lastMercPtX = routeData->data[numSegments+0];
    float lastMercPtY = routeData->data[numSegments+1];

    float mercPtX = routeData->data[numSegments+2];
    float mercPtY = routeData->data[numSegments+3];

    return 90-NBRE_Math::DirectionToHeading(mercPtX-lastMercPtX, mercPtY-lastMercPtY);
}

PAL_Error NBGM_NavVectorView::UpdateNavData(const NBGM_NavData& data, uint32 modifyFlag)
{
    if(modifyFlag & NBGM_NDF_LOCATION)
    {
        mAvatarLocation = data.vectorNavData.avatarLocation;
        mMapView->SetAvatarLocation(data.vectorNavData.avatarLocation);
        if(mIsNavMainView)
        {
            mMapView->SetViewCenter(data.vectorNavData.avatarLocation.position.x, data.vectorNavData.avatarLocation.position.y);
            mMapView->SetRotateAngle(static_cast<float>(-data.vectorNavData.viewHeading));
        }
    }

    if (modifyFlag & (NBGM_NDF_RESET_VECTOR_ROUTE|NBGM_NDF_VECTOR_ROUTE))
    {
        ResetVectorRouteLayer();
    }

    //refresh label after recalc
    if (modifyFlag & (NBGM_NDF_RESET_VECTOR_ROUTE|NBGM_NDF_VECTOR_ROUTE))
    {
        mMapView->RefreshLabelLayout();
    }

    if(modifyFlag & NBGM_NDF_VECTOR_ROUTE)
    {
        mVecManeuvers.clear();
        for(std::vector<NBGM_VectorRouteData*>::const_iterator i = data.vectorNavData.routePolyline.begin(); i != data.vectorNavData.routePolyline.end(); ++i)
        {
            mVecManeuvers[(*i)->maneuverID].position.x = (*i)->offset[0];
            mVecManeuvers[(*i)->maneuverID].position.y = (*i)->offset[1];
            mVecManeuvers[(*i)->maneuverID].heading = GetSimpleTurnmapHeading(*i);
            AddVectorRoutePolyline(*i);
        }
    }

    if (modifyFlag & NBGM_NDF_START_FLAG)
    {
        mMapView->SetStartFlagLocation(data.vectorNavData.startFlagLoc);
    }

    if (modifyFlag & NBGM_NDF_END_FLAG)
    {
        mMapView->SetEndFlagLocation(data.vectorNavData.endFlagLoc);
    }

    if (modifyFlag & NBGM_NDF_POI)
    {
        mMapView->RefreshNavPois(data.poiList);
    }

    if (modifyFlag & NBGM_NDF_CURRENT_MANEUVER)
    {
        SetVectorCurrentManeuver(data.currentManeuver);
    }
    if(modifyFlag & NBGM_NDF_AVATAR_MODE_FLAG)
    {
        mMapView->SetAvatarMode(data.avatarMode);
    }
    return PAL_Ok;
}

void NBGM_NavVectorView::OnSizeChanged( int32 x, int32 y, uint32 w, uint32 h )
{
    NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM_NavEcmView::OnSizeChanged x = %d, y = %d, width = %u, height = %u", x, y, w, h);
    mMapView->OnSizeChanged(x, y, w, h);
//	mAnimation->OnSizeChanged(x, y, w, h);
}

void NBGM_NavVectorView::SetVectorCurrentManeuver( uint32 maneuverIndex )
{
    mCurrentManeuverIndex = maneuverIndex;
    if(mIsNavMainView)
    {
        mMapView->SetNavVectorCurrentManeuver(mCurrentManeuverIndex);
    }
}

void NBGM_NavVectorView::ResetVectorRouteLayer()
{
    mMapView->ResetNavVectorRouteLayer();
}

PAL_Error NBGM_NavVectorView::SetCameraSetting(const NBGM_NavCameraSetting& setting)
{
    NBGM_CameraSetting nbgmSetting;
    nbgmSetting.cameraHeight = METER_TO_MERCATOR(setting.cameraHeight);
    nbgmSetting.cameraToAvatar = METER_TO_MERCATOR(setting.cameraToAvatar);
    nbgmSetting.avatarToHorizon = METER_TO_MERCATOR(setting.avatarToHorizon);
    nbgmSetting.avatarToBottom = setting.avatarToBottom;
    nbgmSetting.hfov = setting.hFov;
    NBGM_CameraProjectionType type = setting.projectionType;
    if(type == NBGM_CPT_PERSPECTIVE)
    {
         mMapView->SetPerspectiveCameraSetting(nbgmSetting);
    }
    else if(type == NBGM_CPT_ORTHO)
    {
        mMapView->SetOrthoCameraSetting(nbgmSetting);
    }
    mMapView->SetAvatarScale(setting.avatarScale);
    mMapView->SetCurrentZoomLevel(NAV_VIEW_ZOOM_LEVEL);
    return PAL_Ok;
}

void NBGM_NavVectorView::SetAvatarState(NBGM_AvatarState state)
{
    mMapView->SetAvatarState(state);
}

void NBGM_NavVectorView::ActiveSurface()
{
    mMapView->ActiveSurface();
}

void NBGM_NavVectorView::ShowNavMainView()
{
//    if(mIsUseAnimation)
//    {
//    	mIsAnimating = !mIsNavMainView;
//    }
//
//	if(mIsAnimating)
//	{
//		mAnimation->ShootPicture2(mMapView->Surface());
//	}

    mIsNavMainView = TRUE;
    mMapView->SetViewCenter(mAvatarLocation.position.x, mAvatarLocation.position.y);
    mMapView->SetRotateAngle(static_cast<float>(mAvatarLocation.heading));
    mMapView->SetNavVectorCurrentManeuver(mCurrentManeuverIndex);

//	if(mIsAnimating)
//	{
//		mAnimation->ShootPicture1(mMapView->Surface());
//		mAnimation->Begin();
//		mAnimationOffset = 1;
//		mIsRight2Left = FALSE;
//	}
}

void NBGM_NavVectorView::ShowManaeuver(int32 maneuverID)
{
    NBRE_Map<int32, VecManeuver>::iterator iter = mVecManeuvers.find(maneuverID);
    if(iter == mVecManeuvers.end())
    {
        return;
    }
//
//    if(mIsUseAnimation)
//    {
//    	mIsAnimating = mIsNavMainView || (mMapView->NavVectorCurrentManeuver() != static_cast<uint32>(maneuverID));
//    }
//
//	if(mIsAnimating)
//	{
//		mIsRight2Left = mIsNavMainView || mMapView->NavVectorCurrentManeuver() < static_cast<uint32>(maneuverID);
//		if(mIsRight2Left)
//		{
//			mAnimation->ShootPicture1(mMapView->Surface());
//		}
//		else
//		{
//			mAnimation->ShootPicture2(mMapView->Surface());
//		}
//	}

    mIsNavMainView = FALSE;
    mMapView->SetViewCenter(iter->second.position.x, iter->second.position.y);
    mMapView->SetNavVectorCurrentManeuver(maneuverID);
    mMapView->SetRotateAngle(iter->second.heading);

//	if(mIsAnimating)
//	{
//		if(mIsRight2Left)
//		{
//			mAnimationOffset = 0;
//			mAnimation->ShootPicture2(mMapView->Surface());
//		}
//		else
//		{
//			mAnimationOffset = 1;
//			mAnimation->ShootPicture1(mMapView->Surface());
//		}
//		mAnimation->Begin();
//	}

}

nb_boolean NBGM_NavVectorView::FindManeuver(int32 maneuverID)
{
    NBRE_Map<int32, VecManeuver>::iterator iter = mVecManeuvers.find(maneuverID);
    if(iter == mVecManeuvers.end())
    {
        return FALSE;
    }

    return TRUE;
}

uint32 NBGM_NavVectorView::NavVectorCurrentManeuver()
{
    return mMapView->NavVectorCurrentManeuver();

}

NBRE_RenderSurface* NBGM_NavVectorView::Surface()
{
    return mMapView->Surface();
}

void NBGM_NavVectorView::SetSkyDayNight(nb_boolean isDay)
{
    mMapView->SetSkyDayNight(isDay);
}

void NBGM_NavVectorView::RefreshLabelLayout()
{
    mMapView->RefreshLabelLayout();
}
