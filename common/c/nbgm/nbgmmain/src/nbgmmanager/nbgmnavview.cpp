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
#include "nbgmnavviewprotected.h"
#include "nbgmnavecmview.h"
#include "nbgmnavvectorview.h"
#include "nbrefilestream.h"
#include "nbrememorystream.h"
#include "nbrelog.h"
#include "nbrerenderengine.h"
#include "nbgmanimation.h"
#include "nbgm.h"
#include "nbgmprotected.h"
#include "nbgmrendercontext.h"
#include "nbgmresourcemanager.h"

NBGM_NavViewInternal::NBGM_NavViewInternal(const NBGM_NavViewConfig& config, PAL_Instance* pal):
    mWorkMode(NBGM_NVWM_STANDARD),
    mVectorView(NULL),
    mEcmView(NULL),
    mRenderEngine(NULL),
    mAnimation(NULL),
    mIsAnimating(FALSE),
    mIsRight2Left(FALSE),
    mAnimationOffset(0.0f),
    mIsUseAnimation(FALSE),
    mUseSharedResource(FALSE),
    mResourceManager(NULL)
{
    nsl_memset(&mPalette, 0, sizeof(NBGM_Palette));

    mConfig = config;
    mPal = pal;

    mCameraState.isLandscape = FALSE;
    mCameraState.isNeedUpdate = TRUE;
    mCameraState.isAvatarBlocked = FALSE;
    mCameraState.renderState = NBGM_RS_NAV_MAINVIEW;
    mCameraState.renderContent = NBGM_RC_VECTOR;
    mCameraState.perspectiveType = NBGM_PT_PERSPECTIVE_3D;
    mCameraState.zoomLevel = NBGM_EZL_MIDDLE;
    mCameraState.tripState = NBGM_TS_ON_ROUTE;
}

NBGM_NavViewInternal::NBGM_NavViewInternal(const NBGM_NavViewConfig& config, PAL_Instance* pal, NBGM_ResourceManager* resourceManager):
    mWorkMode(NBGM_NVWM_STANDARD),
    mVectorView(NULL),
    mEcmView(NULL),
    mRenderEngine(NULL),
    mAnimation(NULL),
    mIsAnimating(FALSE),
    mIsRight2Left(FALSE),
    mAnimationOffset(0.0f),
    mIsUseAnimation(FALSE),
    mUseSharedResource(TRUE),
    mResourceManager(resourceManager)
{
    nsl_memset(&mPalette, 0, sizeof(NBGM_Palette));

    mConfig = config;
    mPal = pal;

    mCameraState.isLandscape = FALSE;
    mCameraState.isNeedUpdate = TRUE;
    mCameraState.isAvatarBlocked = FALSE;
    mCameraState.renderState = NBGM_RS_NAV_MAINVIEW;
    mCameraState.renderContent = NBGM_RC_VECTOR;
    mCameraState.perspectiveType = NBGM_PT_PERSPECTIVE_3D;
    mCameraState.zoomLevel = NBGM_EZL_MIDDLE;
    mCameraState.tripState = NBGM_TS_ON_ROUTE;
}

NBGM_NavViewInternal::~NBGM_NavViewInternal()
{
    mResourceManager->GetConfig().renderContext->SwitchTo();
    NBRE_DELETE mAnimation;
    NBRE_DELETE mEcmView;
    NBRE_DELETE mVectorView;
    if(mRenderEngine)
    {
        mRenderEngine->Deinitialize();
        NBRE_DELETE mRenderEngine;
    }
    if(!mUseSharedResource && mResourceManager != NULL)
    {
        NBGM_DestroyResourceManager(&mResourceManager);
    }
}

void NBGM_NavViewInternal::InitializeEnvironment()
{
    if(!mUseSharedResource)
    {
        NBGM_CreateInternalResourceManager(&mConfig.mapViewConfig, mPal, &mResourceManager);
    }
    mRenderEngine = NBGM_CreateRenderEingne(mResourceManager->GetResourceContext().renderPal, mPal);

    mVectorView = NBRE_NEW NBGM_NavVectorView(mConfig, mRenderEngine, mResourceManager);
    if(mConfig.isUseECM && mEcmView == NULL)
    {
        mEcmView = NBRE_NEW NBGM_NavEcmView(mConfig.mapViewConfig, mRenderEngine, mResourceManager);
    }

//	mAnimation = NBRE_NEW NBGM_Animation(mRenderEngine);
    mIsAnimating = FALSE;
    mIsUseAnimation = mConfig.isUseAnimation;

    mVectorView->InitializeEnvironment();
    if(mEcmView)
    {
        mEcmView->InitializeEnvironment();
    }
//	mAnimation->Initialize();
}

void NBGM_NavViewInternal::Invalidate()
{
    if(mCameraState.isNeedUpdate)
    {
        UpdateCamera();
        mCameraState.isNeedUpdate = FALSE;
    }


    switch(mWorkMode)
    {
    case NBGM_NVWM_STANDARD:
        mVectorView->RequestRenderFrame(NULL);
        break;
    case NBGM_NVWM_ENHANCED:
         if(mEcmView)
         {
             mEcmView->RequestRenderFrame(NULL);
         }
        break;
    default:
        break;
    }
}

void NBGM_NavViewInternal::SetViewSize( int32 x, int32 y, uint32 width, uint32 height )
{
     if(mEcmView)
     {
        mEcmView->SetViewSize(x, y, width, height);
     }
    mVectorView->SetViewSize(x, y, width, height);
//	mAnimation->OnSizeChanged(x, y, width, height);
    mCameraState.isNeedUpdate = TRUE;
}

PAL_Error NBGM_NavViewInternal::LoadCommonMaterial( const std::string& categoryName, const std::string& materialName, const std::string& filePath )
{
    if(materialName.empty())
    {
        return PAL_ErrBadParam;
    }
    shared_ptr<NBRE_FileStream> fs(NBRE_NEW NBRE_FileStream(NBGM_GetConfig()->pal, filePath.c_str(), 2048));
    return mVectorView->LoadCommonMaterial(categoryName, materialName, fs);
}

PAL_Error NBGM_NavViewInternal::LoadCommonMaterialFromBuffer( const std::string& categoryName, const std::string& materialName, const uint8* buffer, uint32 bufferSize )
{
    PAL_Error err = PAL_Ok;

    if(buffer == NULL || bufferSize == 0)
    {
        return PAL_ErrBadParam;
    }
    shared_ptr<NBRE_MemoryStream> ms(NBRE_NEW NBRE_MemoryStream(buffer, bufferSize, TRUE));
    err = mVectorView->LoadCommonMaterial(categoryName, materialName, ms);
    NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM_NavViewInternal::LoadCommonMaterialFromBuffer, materialName = %s, buffer = %p, bufferSize = %u, err = 0x%08x", materialName.c_str(), buffer, bufferSize, err);
    return err;
}

PAL_Error NBGM_NavViewInternal::SetCurrentCommonMaterial( const std::string& categoryName, const std::string& materialName )
{
    return mVectorView->SetCurrentCommonMaterial(categoryName, materialName);
}

PAL_Error NBGM_NavViewInternal::LoadNBMTile( const std::string& nbmName, const std::string& categoryName, uint8 baseDrawOrder, uint8 labelDrawOrder, const std::string& filePath )
{
    //check input
    PAL_Error err = PAL_Ok;
    if(nbmName.empty())
    {
        return PAL_ErrBadParam;
    }

    //load tile
    //it will be deleted in loading tile task queue
    NBRE_FileStream* fs = NBRE_NEW NBRE_FileStream(NBGM_GetConfig()->pal, filePath.c_str(), 1024);
    err = mVectorView->LoadNBMTile(nbmName, categoryName, baseDrawOrder, labelDrawOrder, fs);

    //log result and return
    NBRE_DebugLog(PAL_LogSeverityInfo,
        "NBGM_NavViewInternal::LoadNBMTile, nbmName=%s, baseDrawOrder=%u, labelDrawOrder=%u, filePath=%s, err=0x%08x",
        nbmName.c_str(), baseDrawOrder, labelDrawOrder, filePath.c_str(), err);
    return err;
}

PAL_Error NBGM_NavViewInternal::LoadNBMTileFromBuffer( const std::string& nbmName, const std::string& categoryName, uint8 baseDrawOrder, uint8 labelDrawOrder, uint8* buffer, uint32 bufferSize )
{
    //check input
    PAL_Error err = PAL_Ok;
    if(buffer == NULL || bufferSize == 0)
    {
        return PAL_ErrBadParam;
    }

    //load tile
    //it will be deleted in loading tile task queue
    NBRE_MemoryStream* ms = NBRE_NEW NBRE_MemoryStream(buffer, bufferSize, TRUE);
    err = mVectorView->LoadNBMTile(nbmName, categoryName, baseDrawOrder, labelDrawOrder, ms);

    //log result and return
    NBRE_DebugLog(PAL_LogSeverityInfo,
        "NBGM_NavViewInternal::LoadNBMTileFromBuffer, nbmName=%s, baseDrawOrder=%u, labelDrawOrder=%u, buffer =%p bufferSize=%u, err=0x%08x",
        nbmName.c_str(), baseDrawOrder, labelDrawOrder, buffer, bufferSize, err);
    return err;
}

void NBGM_NavViewInternal::UnLoadTile( const std::string& nbmName )
{
    mVectorView->UnLoadTile(nbmName);
}

void NBGM_NavViewInternal::EnableLayer( uint8 /*drawOrder*/, bool /*enable*/ )
{
    //mVectorView->EnableLayer(layerId, enable);
}

void NBGM_NavViewInternal::EnableProfiling( uint32 /*flags*/ )
{
    NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_NavViewInternal::EnableProfiling not support yet");
    nbre_assert(FALSE);
}

PAL_Error NBGM_NavViewInternal::SwitchToWorkMode( NBGM_NavViewWorkMode workMode )
{
    mWorkMode = workMode;

    switch(mWorkMode)
    {
    case NBGM_NVWM_STANDARD:
        mVectorView->ActiveSurface();
        break;
    case NBGM_NVWM_ENHANCED:
        if(mEcmView)
        {
            mEcmView->ActiveSurface();
        }
        break;
    default:
        return PAL_ErrUnsupported;
    }

    return PAL_Ok;
}

PAL_Error NBGM_NavViewInternal::SetPalette( const NBGM_Palette& palette )
{
    mPalette = palette;
    NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_NavViewInternal::SetPalette not support yet");
    nbre_assert(FALSE);
    return PAL_Ok;
}

PAL_Error NBGM_NavViewInternal::UpdateExcludeRect( std::vector<NBGM_Rect2d*> rects )
{
    return mVectorView->UpdateExcludeRect(rects);
}

NBGM_NavViewWorkMode NBGM_NavViewInternal::GetWorkMode() const
{
    return mWorkMode;
}

PAL_Error NBGM_NavViewInternal::UpdateNavData( NBGM_NavData& data, uint32 modifyFlag )
{
    switch(mWorkMode)
    {
    case NBGM_NVWM_STANDARD:
        return mVectorView->UpdateNavData(data, modifyFlag);

    case NBGM_NVWM_ENHANCED:
        if(mEcmView)
        {
            return mEcmView->UpdateNavData(data, modifyFlag);
        }
        return PAL_ErrNoInit;

    default:
        return PAL_ErrUnsupported;
    }
}

void NBGM_NavViewInternal::ShowNavMainView()
{

    if(mIsUseAnimation)
    {
        mIsAnimating = (mCameraState.renderState == NBGM_RS_MANUEVER_LOOKAHEAD);
    }


    if(mIsAnimating)
    {
//		mAnimation->ShootPicture2(mVectorView->Surface());
        mVectorView->RenderFrame();
    }

    mCameraState.renderState = NBGM_RS_NAV_MAINVIEW;
    mCameraState.isNeedUpdate = TRUE;
    if(mCameraState.isNeedUpdate)
    {
        UpdateCamera();
        mCameraState.isNeedUpdate = FALSE;
    }
    mVectorView->ShowNavMainView();

    if(mIsAnimating)
    {
//		mAnimation->ShootPicture1(mVectorView->Surface());
        mVectorView->RenderFrame();
//		mAnimation->Begin();
        mAnimationOffset = 1;
        mIsRight2Left = FALSE;
    }

}

void NBGM_NavViewInternal::ShowManeuver(int32 maneuverID)
{
    SwitchToWorkMode(NBGM_NVWM_STANDARD);

    if (!mVectorView->FindManeuver(maneuverID))
    {
        return;
    }

    if(mIsUseAnimation)
    {
        mIsAnimating = (mCameraState.renderState == NBGM_RS_NAV_MAINVIEW) || (mVectorView->NavVectorCurrentManeuver() != static_cast<uint32>(maneuverID));
    }

    if(mIsAnimating)
    {
        mIsRight2Left = (mCameraState.renderState == NBGM_RS_NAV_MAINVIEW) || mVectorView->NavVectorCurrentManeuver() < static_cast<uint32>(maneuverID);
        if(mIsRight2Left)
        {
//			mAnimation->ShootPicture1(mVectorView->Surface());
        }
        else
        {
//			mAnimation->ShootPicture2(mVectorView->Surface());
        }
        mVectorView->RenderFrame();
    }

    mCameraState.renderState = NBGM_RS_MANUEVER_LOOKAHEAD;
    mCameraState.isNeedUpdate = TRUE;

    if(mCameraState.isNeedUpdate)
    {
        UpdateCamera();
        mCameraState.isNeedUpdate = FALSE;
    }
    mVectorView->ShowManaeuver(maneuverID);

    if(mIsAnimating)
    {
        if(mIsRight2Left)
        {
            mAnimationOffset = 0;
//			mAnimation->ShootPicture2(mVectorView->Surface());
        }
        else
        {
            mAnimationOffset = 1;
//			mAnimation->ShootPicture1(mVectorView->Surface());
        }
        mVectorView->RenderFrame();
//		mAnimation->Begin();
    }

}
void NBGM_NavViewInternal::SetNavMode( NBGM_NavViewMode /*mode*/ )
{

}

void NBGM_NavViewInternal::LoadFirstECMTile()
{
    NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_NavViewInternal::LoadFirstECMTile not support yet");
    nbre_assert(FALSE);
}

PAL_Error NBGM_NavViewInternal::SetCameraSetting(const std::map<NBGM_NavCameraUsage, NBGM_NavCameraSetting>& settings)
{
    mCameraSettings = settings;
    return PAL_Ok;
}

void NBGM_NavViewInternal::UpdateCamera()
{
    NBGM_NavCameraUsage usage = NBGM_NCU_VECTOR_3D_PORTRAIT;
    NBGM_RendererType type = NBGM_RT_VECTOR;
    if(mCameraState.renderState == NBGM_RS_MANUEVER_LOOKAHEAD)
    {
        usage = mCameraState.isLandscape ? NBGM_NCU_LOOK_AHEAD_LANDSCAPE : NBGM_NCU_LOOK_AHEAD_PORTRAIT;
        type = NBGM_RT_VECTOR;
    }
    else if(mCameraState.renderState == NBGM_RS_NAV_MAINVIEW)
    {
        switch(mCameraState.tripState)
        {
        case NBGM_TS_ON_ROUTE:
            if(mCameraState.perspectiveType == NBGM_PT_PERSPECTIVE_3D)
            {
                switch(mCameraState.renderContent)
                {
                case NBGM_RC_MJO:
                    if(mCameraState.isAvatarBlocked)
                    {
                        usage = mCameraState.isLandscape ? NBGM_NCU_ECM_MIN_LANDSCAPE : NBGM_NCU_ECM_MIN_PORTRAIT;
                    }
                    else
                    {
                        usage = mCameraState.isLandscape ? NBGM_NCU_MJO_LANDSCAPE : NBGM_NCU_MJO_PORTRAIT;
                    }
                    type = NBGM_RT_ECM;
                    break;
                case NBGM_RC_VECTOR:
                    usage = mCameraState.isLandscape ? NBGM_NCU_VECTOR_3D_LANDSCAPE : NBGM_NCU_VECTOR_3D_PORTRAIT;
                    type = NBGM_RT_VECTOR;
                    break;
                case NBGM_RC_ECM:
                    if(mCameraState.isAvatarBlocked)
                    {
                        usage = mCameraState.isLandscape ? NBGM_NCU_ECM_MIN_LANDSCAPE : NBGM_NCU_ECM_MIN_PORTRAIT;
                    }
                    else
                    {
                        usage = GetEcmCameraUsage();
                    }
                    type = NBGM_RT_ECM;
                    break;
                }
            }
            else if(mCameraState.perspectiveType == NBGM_PT_PERSPECTIVE_2D)
            {
                usage = mCameraState.isLandscape ? NBGM_NCU_VECTOR_2D_LANDSCAPE : NBGM_NCU_VECTOR_2D_PORTRAIT;
                type = NBGM_RT_VECTOR;
            }
            break;
        case NBGM_TS_START_UP:
            usage = mCameraState.isLandscape ? NBGM_NCU_VECTOR_3D_LANDSCAPE : NBGM_NCU_VECTOR_3D_PORTRAIT;
            type = NBGM_RT_VECTOR;
            break;
        case NBGM_TS_OFF_ROUTE:
            if(mCameraState.perspectiveType == NBGM_PT_PERSPECTIVE_3D)
            {
                usage = mCameraState.isLandscape ? NBGM_NCU_VECTOR_3D_LANDSCAPE : NBGM_NCU_VECTOR_3D_PORTRAIT;
            }
            else if(mCameraState.perspectiveType == NBGM_PT_PERSPECTIVE_2D)
            {
                usage = mCameraState.isLandscape ? NBGM_NCU_VECTOR_2D_LANDSCAPE : NBGM_NCU_VECTOR_2D_PORTRAIT;
            }
            type = NBGM_RT_VECTOR;
            break;
        }
    }
    CameraSettings::iterator iter = mCameraSettings.find(usage);
    if(iter != mCameraSettings.end())
    {
        NBGM_NavCameraSetting setting = mCameraSettings[usage];
        if(type == NBGM_RT_VECTOR)
        {
            mVectorView->SetCameraSetting(setting);
        }
        else if(type == NBGM_RT_ECM)
        {
            if(mEcmView)
            {
                mEcmView->SetCameraSetting(setting);
            }
        }
    }
}

NBGM_NavCameraUsage NBGM_NavViewInternal::GetEcmCameraUsage()
{
    NBGM_NavCameraUsage usage = NBGM_NCU_ECM_MIN_LANDSCAPE;
    switch(mCameraState.zoomLevel)
    {
    case NBGM_EZL_MAX:
        usage = mCameraState.isLandscape ? NBGM_NCU_ECM_MAX_LANDSCAPE : NBGM_NCU_ECM_MAX_PORTRAIT;
        break;
    case NBGM_EZL_MIDDLE:
        usage = mCameraState.isLandscape ? NBGM_NCU_ECM_MIDDLE_LANDSCAPE : NBGM_NCU_ECM_MIDDLE_PORTRAIT;
        break;
    case MBGM_EZL_MIN:
        usage = mCameraState.isLandscape ? NBGM_NCU_ECM_MIN_LANDSCAPE : NBGM_NCU_ECM_MIN_PORTRAIT;
        break;
    }
    return usage;
}

void
NBGM_NavViewInternal::SetSkyDayNight(nb_boolean isDay)
{
    mVectorView->SetSkyDayNight(isDay);
    if(mEcmView)
    {
        mEcmView->SetSkyDayNight(isDay);
    }
}

void
NBGM_NavViewInternal::RefreshLabelLayout()
{
    mVectorView->RefreshLabelLayout();
}

void
NBGM_NavViewInternal::SetLandscape(bool is_Landscape)
{
    mCameraState.isLandscape = is_Landscape;
    mCameraState.isNeedUpdate = TRUE;
}
