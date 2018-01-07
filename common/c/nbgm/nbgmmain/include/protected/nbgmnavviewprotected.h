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

    @file nbgmnavviewprotected.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBGM_NAV_VIEW_PROTECTED_H_
#define _NBGM_NAV_VIEW_PROTECTED_H_
#include "nbrecommon.h"
#include "nbgmnavview.h"
#include "paltypes.h"
#include "nbgmnavviewconfig.h"
#include "nbgmcamerastate.h"

class NBGM_NavVectorView;
class NBGM_NavEcmView;
class NBRE_RenderEngine;
class NBRE_IRenderPal;
class NBGM_Animation;
class NBGM_ResourceManager;

/*! \addtogroup NBGM_Manager
*  @{
*/
class NBGM_NavViewInternal : public NBGM_NavView
{
public:
    NBGM_NavViewInternal(const NBGM_NavViewConfig& config, PAL_Instance* pal);
    NBGM_NavViewInternal(const NBGM_NavViewConfig& config, PAL_Instance* pal, NBGM_ResourceManager* resourceManager);
    ~NBGM_NavViewInternal();

private:
    virtual void InitializeEnvironment();
    virtual void Invalidate();
    virtual void LoadFirstECMTile();
    virtual void SetViewSize( int32 x, int32 y, uint32 width, uint32 height );
    virtual PAL_Error LoadCommonMaterial( const std::string& categoryName, const std::string& materialName, const std::string& filePath );
    virtual PAL_Error LoadCommonMaterialFromBuffer( const std::string& categoryName, const std::string& materialName, const uint8* buffer, uint32 bufferSize );
    virtual PAL_Error SetCurrentCommonMaterial( const std::string& categoryName, const std::string& materialName );
    virtual void SetSkyDayNight(nb_boolean isDay);
    virtual PAL_Error LoadNBMTile( const std::string& nbmName, const std::string& categoryName, uint8 baseDrawOrder, uint8 labelDrawOrder, const std::string& filePath );
    virtual PAL_Error LoadNBMTileFromBuffer( const std::string& nbmName, const std::string& categoryName, uint8 baseDrawOrder, uint8 labelDrawOrder, uint8* buffer, uint32 bufferSize );
    virtual void UnLoadTile( const std::string& nbmName );
    virtual void EnableLayer( uint8 drawOrder, bool enable );
    virtual void EnableProfiling( uint32 flags );
    virtual PAL_Error SwitchToWorkMode( NBGM_NavViewWorkMode workMode );
    virtual PAL_Error SetPalette( const NBGM_Palette& palette );
    virtual PAL_Error UpdateExcludeRect( std::vector<NBGM_Rect2d*> rects );
    virtual PAL_Error SetCameraSetting( const std::map<NBGM_NavCameraUsage, NBGM_NavCameraSetting>& settings );
    virtual NBGM_NavViewWorkMode GetWorkMode() const;
    virtual PAL_Error UpdateNavData( NBGM_NavData& data, uint32 modifyFlag );
    virtual void ShowNavMainView();
    virtual void ShowManeuver( int32 maneuverID );    
    virtual void SetNavMode( NBGM_NavViewMode mode );
    virtual void RefreshLabelLayout();	
	virtual void SetLandscape(bool is_Landscape);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_NavViewInternal);
    void UpdateCamera();
    NBGM_NavCameraUsage GetEcmCameraUsage();

    typedef NBRE_Map<NBGM_NavCameraUsage, NBGM_NavCameraSetting> CameraSettings;

private:
    NBGM_NavViewConfig      mConfig;
    PAL_Instance*           mPal;
    NBGM_Palette            mPalette;
    NBGM_NavViewWorkMode    mWorkMode;
    NBGM_NavVectorView*     mVectorView;
    NBGM_NavEcmView*        mEcmView;
    CameraSettings          mCameraSettings;
    NBGM_CameraState        mCameraState;
    NBRE_RenderEngine*      mRenderEngine;

	NBGM_Animation* mAnimation;
	nb_boolean mIsAnimating;
	nb_boolean mIsRight2Left;
	float mAnimationOffset;
	nb_boolean mIsUseAnimation;
    nb_boolean mUseSharedResource;
    NBGM_ResourceManager* mResourceManager;
};

/*! @} */
#endif
