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
#include "nbgmnavecmview.h"
#include "nbgmmapviewimpl.h"
#include "nbrelog.h"
#include "nbgmmapviewimpl.h"
#include "nbgmconst.h"

NBGM_NavEcmView::NBGM_NavEcmView(const NBGM_MapViewConfig& config, NBRE_RenderEngine* renderEngine, NBGM_ResourceManager* resourceManager)
{
    NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM_NavEcmView::NBGM_NavEcmView, renderEngine = %p", renderEngine);
    NBGM_MapViewImplConfig ecmConfig;
    ecmConfig.viewConfig = config;
    ecmConfig.drawNavPoi = FALSE;
    ecmConfig.drawVectorNavRoute = FALSE;
    ecmConfig.drawEcmNavRoute = TRUE;
    ecmConfig.modelScaleFactor = ECM_MAP_MODEL_SCALE_FACTOR;
    ecmConfig.avatarSizeFactor = ECM_AVATA_SIZE_FACTOR;
    ecmConfig.referenceAvatarHeight = REFERENCE_AVATAR_BASIC_HEIGHT;
    ecmConfig.useNavRoadLabel = TRUE;
    ecmConfig.maxLabels = 6;
    ecmConfig.maxAreaBuildingLabels = 2;
    ecmConfig.renderInterval = config.renderInterval;
    ecmConfig.resourceManager = resourceManager;
    mMapView = NBRE_NEW NBGM_MapViewImpl(ecmConfig, renderEngine);
}

NBGM_NavEcmView::~NBGM_NavEcmView()
{
    NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM_NavEcmView::~NBGM_NavEcmView");
    mMapView->Finalize();
    NBRE_DELETE mMapView;
}

void NBGM_NavEcmView::InitializeEnvironment()
{
    NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM_NavEcmView::InitializeEnvironment");
    mMapView->InitializeEnvironment();
    mMapView->SetAvatarState(NBGM_AS_NAV);
    mMapView->SetAvatarMode(NBGM_AM_NAV_CAR);
    mMapView->SetIsNavMode(TRUE);
}

void NBGM_NavEcmView::RenderFrame()
{
    mMapView->RenderFrame();
}

void NBGM_NavEcmView::RequestRenderFrame(NBGM_FrameListener* listener)
{
    mMapView->RequestRenderFrame(listener);
}

void NBGM_NavEcmView::SetViewSize( int32 /*x*/, int32 /*y*/, uint32 /*width*/, uint32 /*height*/ )
{

}

PAL_Error NBGM_NavEcmView::SetPalette( const NBGM_Palette& /*palette*/ )
{
    return PAL_ErrUnsupported;
}


PAL_Error NBGM_NavEcmView::UpdateNavData( const NBGM_NavData& data, uint32 modifyFlag )
{
    if(modifyFlag & NBGM_NDF_LOCATION)
    {
        NBGM_Location64 loc;
        loc.position.x = data.ecmNavData.avatarLocation.position.x;
        loc.position.y = data.ecmNavData.avatarLocation.position.y;
        loc.position.z = data.ecmNavData.avatarLocation.position.z;
        loc.heading = data.ecmNavData.avatarLocation.heading;

        NBGM_SnapRouteInfo& avatarSnapInfo = mMapView->NavEcmGetAvatarSnapRouteInfo();
        PAL_Error err = mMapView->NavEcmSnapRoute(loc, avatarSnapInfo);
        if (err != PAL_Ok)
        {
            NBGM_SnapRouteInfo snapRouteInfo;
            err = mMapView->NavEcmSnapRoute(loc, snapRouteInfo);
            if(err == PAL_Ok)
            {
                avatarSnapInfo = snapRouteInfo;
            }
            else
            {
                if(err == PAL_ErrNotFound && !avatarSnapInfo.IsValid())
                {
                    err = mMapView->NavEcmSnapToRouteStart(loc, avatarSnapInfo);
                }
            }
        }

        if (err == PAL_Ok)
        {
            mMapView->SetAvatarLocation(loc);
            mMapView->SetViewCenter(loc.position.x, loc.position.y);
            mMapView->SetRotateAngle(static_cast<float>(data.ecmNavData.viewHeading));
        }
        else
        {
            return err;
        }
    }

    if(modifyFlag & NBGM_NDF_ECM_ROUTE)
    {
        mMapView->ResetNavEcmRouteLayer();

        std::vector<NBGM_SplineData*>::const_iterator pSpline = data.ecmNavData.splines.begin();
        std::vector<NBGM_SplineData*>::const_iterator pSplineEnd = data.ecmNavData.splines.end();

        for(; pSpline != pSplineEnd; ++pSpline)
        {
            mMapView->AddNavEcmRouteSpline(*pSpline);
        }

        std::vector<NBGM_EcmManeuver>::const_iterator pMan = data.ecmNavData.maneuvers.begin();
        std::vector<NBGM_EcmManeuver>::const_iterator pManEnd = data.ecmNavData.maneuvers.end();

        for(; pMan != pManEnd; ++pMan)
        {
            mMapView->AddNavEcmManeuver(pMan->id, pMan->position);
        }
    }

    if (modifyFlag & NBGM_NDF_CURRENT_MANEUVER)
    {
        mMapView->SetNavEcmCurrentManeuver(data.currentManeuver);
    }
    return PAL_Ok;
}

PAL_Error NBGM_NavEcmView::SetCameraSetting(NBGM_NavCameraSetting setting)
{
    NBGM_CameraSetting nbgmSetting;
    nbgmSetting.avatarToBottom = setting.avatarToBottom;
    nbgmSetting.avatarToHorizon = setting.avatarToHorizon;
    nbgmSetting.cameraHeight = setting.cameraHeight;
    nbgmSetting.cameraToAvatar = setting.cameraToAvatar;
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

void NBGM_NavEcmView::OnSizeChanged(int32 x, int32 y, uint32 w, uint32 h)
{
     NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM_NavEcmView::OnSizeChanged x = %d, y = %d, width = %u, height = %u", x, y, w, h);
     mMapView->OnSizeChanged(x, y, w, h);
}

void NBGM_NavEcmView::ActiveSurface()
{
    mMapView->ActiveSurface();
}

void NBGM_NavEcmView::SetSkyDayNight(nb_boolean isDay)
{
    mMapView->SetSkyDayNight(isDay);
}
