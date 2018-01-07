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

    @file nbgmtilemaplayer.h
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

#ifndef _NBGM_TILE_MAP_LAYER_H_
#define _NBGM_TILE_MAP_LAYER_H_

#include "nbretypes.h"
#include "nbgmvectortiledata.h"
#include "nbgmvectortile.h"
#include "nbgmmaplayer.h"
#include "nbgmcontext.h"
#include "nbgmvectorvertexscale.h"
#include "nbgmtilemaplayertile.h"
#include "nbgmmapviewprofiler.h"
#include "nbgmtransparentoverlay.h"
#include "nbgmglowoverlay.h"
#include "nbgmnavvectorroutelayer.h"


///define how many layer have in one draw order
static const int32 NBGM_RoadBackgroundLayer = 0;
static const int32 NBGM_RoadForegroundLayer = 1;

/*! \addtogroup NBRE_Service
*  @{
*/

class NBGM_TileMapLayer: public NBGM_IMapLayer
{
    class PatternTextureAnimation: public NBRE_ITextureAnimation
    {
    public:
        PatternTextureAnimation();
        ~PatternTextureAnimation(){};

    public:
        void SetCurrentZoomLevel(int8 zoomlevel);
        virtual void ModifyPass(NBRE_Pass* pass) const;
        virtual uint32 GetDirtyFrameNumber() const {return mDirtyFrameNumber;}

    private:
        DISABLE_COPY_AND_ASSIGN(PatternTextureAnimation);

    private:
        mutable float mScale;
        uint32 mDirtyFrameNumber;
        int8 mZoomLevel;
    };

public: 
    NBGM_TileMapLayer(NBGM_Context& nbgmContext, NBRE_Node& parentNode, NBRE_SurfaceSubView* subView, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, NBGM_MapViewProfiler* mProfiler, NBGM_TransparentLayer* transparentLayer, NBGM_GlowLayer* glowLayer, NBGM_NavVectorRouteLayer* navVecRouteLayer, int32 layerStride, int32 transparentRoadBgOverlay, int32 buildingOverlay, int32 glowOverlay);
    virtual ~NBGM_TileMapLayer();

    void SetCurrentZoomLevel(int8 zoomlevel);

    void LoadBinTile(float scale);
    void NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType);
    void SetTileOpacity(const NBGM_VectorTileID& tileId, float opacity);
    void HitTest(const NBRE_Vector2d& center, double radius, uint32 selectMask, NBRE_Set<NBGM_ElementId>& objectIds);

    ///Derive From NBGM_IMapLayer
    void OnAddTile(const NBGM_VectorTileID& tileId, NBGM_IMapLayerTile* tile);
    void OnRemoveTile(const NBGM_VectorTileID& tileId);
    void OnActiveTile(const NBGM_VectorTileID& tileId);
    void OnDeactiveTile(const NBGM_VectorTileID& tileId);
    NBGM_IMapLayerTileCreator& GetTileCreator();
    void UpdateEntityVisibility(const NBRE_CameraPtr& mCamera);
    nb_boolean CheckIfHasLM3D();
    void DisableUnTexturedLandMarks();
    void SetRenderRoadToFrameBuffer(nb_boolean value);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_TileMapLayer);

private:
    typedef NBRE_Map<NBGM_VectorTileID, NBGM_TileMapLayerTile*> VectorMapTileTable;
    typedef NBRE_Map<uint32, uint32> OverlayCountTable;

private:
    void AddOverlay(const NBGM_TileMapLayerTile& tile);
    void RemoveOverlay(const NBGM_TileMapLayerTile& tile);

private:

    NBGM_Context& mNBGMContext;
    int32 mLayerStride;
    VectorMapTileTable mMapTileTable;
    NBRE_Node& mParentNode;

    int8 mZoomLevel;
    PatternTextureAnimation mTexcoorAnimation;

    int32 mTransparentRoadBgOverlay;
    int32 mBuildingOverlay;
    int32 mGlowOverlay;
    nb_boolean mRenderRoadToFrameBuffer;

    NBGM_TileMapLayerTileCreator* mTileDataCreator;
    NBRE_SurfaceSubView*          mSubView;
    NBRE_DefaultOverlayManager<DrawOrderStruct>& mOverlayManager;
    NBGM_MapViewProfiler*         mProfiler;
    NBGM_TransparentLayer*        mTransparentLayer;
    NBGM_GlowLayer*               mGlowLayer;
    NBGM_NavVectorRouteLayer*     mNavVecRouteLayer;
    OverlayCountTable             mOverlayCountTable;
};
/*! @} */

#endif


