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

    @file nbgmglowoverlay.h
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
#ifndef _NBGM_GLOW_OVERLAY_H_
#define _NBGM_GLOW_OVERLAY_H_
#include "nbretypes.h"
#include "nbrecommon.h"
#include "nbrescenemanager.h"
#include "nbgmdefaultoverlaymanager.h"
#include "nbgmdraworder.h"


struct NBGM_Context;
class NBRE_SurfaceSubView;
class NBRE_Overlay;
class NBRE_Entity;
class NBRE_RenderSurface;
class NBGM_MapViewProfiler;
class NBGM_Sky;

/*! \addtogroup NBRE_Service
*  @{
*/

/** This class is used to show glowing objects.
*/

class NBGM_GlowLayer: public NBRE_IVisitable, public NBRE_IRenderOperationProvider
{

public:
    enum MapState
    {
        MT_CommonMap,
        MT_SatelliteMap
    };

public:
    NBGM_GlowLayer(NBGM_Context& nbgmContext, const DrawOrderStruct& buildingDrawOrder, const DrawOrderStruct& transparentOverlayDrawOrder, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager);
    ~NBGM_GlowLayer();

public:
    void Initialize(const NBRE_SceneManagerPtr& sceneManager, const NBRE_CameraPtr& camera, NBRE_SurfaceSubView* subView, NBGM_MapViewProfiler& profiler, nb_boolean drawSky);
     void OnSizeChanged(int32 x, int32 y, uint32 w, uint32 h);

    // NBRE_IVisitable members
    virtual void Accept(NBRE_Visitor& visitor);

    ///From NBRE_IRenderQueueVisitable
    virtual void NotifyCamera(const NBRE_Camera& camera);
    virtual void UpdateRenderQueue(NBRE_Overlay& overlay);
    virtual int32 OverlayId() const;
    virtual nb_boolean Visible() const;    
    virtual void UpdateSurfaceList();
    virtual uint32 GetPriority() const{return 0;}

    void SetDrawOrder(uint8 value);
    void SetBuildingDrawOrder(uint8 value);
    int32 BuildingOverlayId();
    void SetAlpha(float alpha);
    void SetGlowParameters(const NBGM_GlowParameters& parameters);
private:
    DISABLE_COPY_AND_ASSIGN(NBGM_GlowLayer);

private:
    void CreateRectVertex();
    void SetBlurSamples(int s);

private:
    /// render context
    NBGM_Context& mNBGMContext;
    DrawOrderStruct mBuildingDrawOrder;
    DrawOrderStruct mTransparentOverlayDrawOrder;
    int32 mBuildingOverlayId;
    uint8 mB3dDrawOrder;

    NBRE_RenderSurface* mTransparentSurface; 
    NBRE_SurfaceSubView* mTransparentSubView;
    NBRE_DefaultOverlayManager<DrawOrderStruct>& mOverlayManager;
    NBRE_DefaultOverlayManager<DrawOrderStruct>* mTransparentOverlayManager;
    NBRE_Overlay* mTransparentOverlay;

    NBRE_VertexData* mShardVertex;
    NBRE_IndexData* mIndexData;
    float mAlpha;

    NBRE_Pass* mCommonPass;
    NBRE_Matrix4x4d mVertexTransform;

    NBRE_SurfaceSubView* mSubView;
    NBRE_CameraPtr mScreenCamera;
    nb_boolean mIsDrawOrderInitialized;
    NBRE_TexturePtr mEmptyTexture;
    NBRE_RenderTarget* mCommonTarget;
    nb_boolean mEnabled;
    NBRE_Color mClearColor;
    NBRE_Color mScaleColor;
    float mRadius;
};

/*! @} */

#endif
