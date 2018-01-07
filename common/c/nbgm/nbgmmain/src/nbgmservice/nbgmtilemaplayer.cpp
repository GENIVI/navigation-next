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
#include "nbgmtilemaplayer.h"
#include "nbgmvectortilebuilder.h"
#include "nbrerenderengine.h"
#include "nbreplane.h"
#include "nbretypeconvert.h"
#include "nbrelog.h"
#include "nbgmconst.h"
#include "nbgm.h"
#include "nbretransformation.h"
#include "nbremath.h"
#include "nbgmbuildutility.h"

NBGM_TileMapLayer::PatternTextureAnimation::PatternTextureAnimation(): mScale(1.0f), mDirtyFrameNumber(0),mZoomLevel(NBGM_GetConfig()->relativeZoomLevel)
{
}

void
NBGM_TileMapLayer::PatternTextureAnimation::SetCurrentZoomLevel(int8 zoomlevel)
{
    mZoomLevel = zoomlevel;
    ++mDirtyFrameNumber;
}

void
NBGM_TileMapLayer::PatternTextureAnimation::ModifyPass(NBRE_Pass* pass) const
{
    if(pass == NULL)
    {
        return;
    }

    mScale = 1.0f/GetPolylineScaleFactor(mZoomLevel, TRUE);
    NBRE_Matrix4x4f scaleMat = NBRE_Transformationf::BuildScaleMatrix(mScale, 1.0f, 1.0f);

    NBRE_TextureUnitList& ul = pass->GetTextureUnits();
    NBRE_TextureUnitList::iterator u = ul.begin();
    NBRE_TextureUnitList::iterator end = ul.end();
    for(; u != end; ++u)
    {
        u->SetTransform(scaleMat);
    }
}

NBGM_TileMapLayer::NBGM_TileMapLayer(NBGM_Context& nbgmContext, NBRE_Node& parentNode, NBRE_SurfaceSubView* subView, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, NBGM_MapViewProfiler* profiler, NBGM_TransparentLayer* transparentLayer, NBGM_GlowLayer* glowLayer, NBGM_NavVectorRouteLayer* navVecRouteLayer, int32 layerStride, int32 transparentRoadBgOverlay, int32 buildingOverlay, int32 glowOverlay):
    mNBGMContext(nbgmContext),
    mLayerStride(layerStride),
    mParentNode(parentNode),
    mZoomLevel(NBGM_GetConfig()->relativeZoomLevel),
    mTransparentRoadBgOverlay(transparentRoadBgOverlay),
    mBuildingOverlay(buildingOverlay),
    mGlowOverlay(glowOverlay),
    mRenderRoadToFrameBuffer(FALSE),
    mTileDataCreator(NULL),
    mSubView(subView),
    mOverlayManager(overlayManager),
    mProfiler(profiler),
    mTransparentLayer(transparentLayer),
    mGlowLayer(glowLayer),
    mNavVecRouteLayer(navVecRouteLayer)
{
    mTileDataCreator = NBRE_NEW NBGM_TileMapLayerTileCreator(nbgmContext, &mTexcoorAnimation, &mZoomLevel, &mTransparentRoadBgOverlay, &mBuildingOverlay, &mGlowOverlay);
}

NBGM_TileMapLayer::~NBGM_TileMapLayer()
{
    for (VectorMapTileTable::iterator iter = mMapTileTable.begin(); iter != mMapTileTable.end(); ++iter)
    {
        if(iter->second->Node())
        {
            mParentNode.RemoveChild(iter->second->Node().get());
        }
        NBRE_DELETE iter->second;
    }
    NBRE_DELETE mTileDataCreator;
}

void
NBGM_TileMapLayer::OnAddTile(const NBGM_VectorTileID& tileId, NBGM_IMapLayerTile* tile)
{
    if(tile != NULL)
    {
        nbre_assert(mMapTileTable.find(tileId) == mMapTileTable.end());
        NBGM_TileMapLayerTile* tileData = static_cast<NBGM_TileMapLayerTile*> (tile);
        tileData->SwitchRoadOverlays(mRenderRoadToFrameBuffer, mTransparentRoadBgOverlay, mBuildingOverlay);
        AddOverlay(*tileData);
        mParentNode.AddChild(tileData->Node());
        mMapTileTable[tileId] = tileData;
    }
}

NBGM_IMapLayerTileCreator&
NBGM_TileMapLayer::GetTileCreator()
{
    return *mTileDataCreator;
}

void
NBGM_TileMapLayer::OnRemoveTile(const NBGM_VectorTileID& tileId)
{
    VectorMapTileTable::iterator iter = mMapTileTable.find(tileId);
    if(iter != mMapTileTable.end())
    {
        RemoveOverlay(*iter->second);
        if(iter->second->Node())
        {
            mParentNode.RemoveChild(iter->second->Node().get());
        }
        NBRE_DELETE iter->second;
        mMapTileTable.erase(iter);
    }
}

void
NBGM_TileMapLayer::OnActiveTile(const NBGM_VectorTileID& tileId)
{
    VectorMapTileTable::iterator iter = mMapTileTable.find(tileId);
    if(iter != mMapTileTable.end())
    {
        iter->second->Active();
    }
}

void
NBGM_TileMapLayer::OnDeactiveTile(const NBGM_VectorTileID& tileId)
{
    VectorMapTileTable::iterator iter = mMapTileTable.find(tileId);
    if(iter != mMapTileTable.end())
    {
        iter->second->Deactive();
    }
}

void
NBGM_TileMapLayer::SetCurrentZoomLevel(int8 zoomlevel)
{
    mZoomLevel = zoomlevel;
    VectorMapTileTable::iterator iter = mMapTileTable.begin();
    while(iter != mMapTileTable.end())
    {
        iter->second->SetCurrentZoomLevel(zoomlevel);
        ++iter;
    }

    mTexcoorAnimation.SetCurrentZoomLevel(zoomlevel);
}

void
NBGM_TileMapLayer::UpdateEntityVisibility(const NBRE_CameraPtr& mCamera)
{
    NBRE_Planef cameraPlane(-mCamera->WorldDirection(), NBRE_TypeConvertf::Convert(mCamera->WorldPosition()));

    VectorMapTileTable::iterator pVectorMapTile = mMapTileTable.begin();
    VectorMapTileTable::iterator pVectorMapTileEnd = mMapTileTable.end();

    for (; pVectorMapTile != pVectorMapTileEnd; ++pVectorMapTile)
    {
        pVectorMapTile->second->UpdateEntityVisibility(cameraPlane);
    }
}

void
NBGM_TileMapLayer::NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType)
{
    for(VectorMapTileTable::iterator iter = mMapTileTable.begin(); iter != mMapTileTable.end(); ++iter)
    {
        iter->second->NotifyExtend(extendType);
    }
}

void
NBGM_TileMapLayer::SetTileOpacity(const NBGM_VectorTileID& tileId, float opacity)
{
    VectorMapTileTable::iterator it = mMapTileTable.find(tileId);
    if (it != mMapTileTable.end())
    {
        it->second->SetOpacity(opacity);
    }
}

void
NBGM_TileMapLayer::HitTest(const NBRE_Vector2d& center, double radius, uint32 selectMask, NBRE_Set<NBGM_ElementId>& objectIds)
{
    for (VectorMapTileTable::iterator it = mMapTileTable.begin(); it != mMapTileTable.end(); ++it)
    {
        it->second->HitTest(center, radius, selectMask, objectIds);
    }
}

void
NBGM_TileMapLayer::AddOverlay(const NBGM_TileMapLayerTile& tile)
{
    uint8 drawOrder = tile.GetDrawOrder();
    if(tile.IsBuildingTile())
    {
    	if(mTransparentLayer)
    	{
    	    mTransparentLayer->SetBuildingDrawOrder(drawOrder);

    	}
        return;
    }
    if(tile.IsRoadTile())
    {
        if(mTransparentLayer)
        {
            mTransparentLayer->SetRoadDrawOrder(drawOrder);
        }
    }
    if(tile.IsPolylineTile())
    {
        if(mGlowLayer)
        {
            mGlowLayer->SetDrawOrder(drawOrder);
        }
    }

    const NBRE_Set<int32>& overlayOrders = tile.GetOverlayIds();
    for(NBRE_Set<int32>::const_iterator iter = overlayOrders.begin(); iter != overlayOrders.end(); ++iter)
    {
        if(mOverlayCountTable[*iter] == 0)
        {
            mProfiler->AddOverlay(*iter, *mOverlayManager.FindOverlay(*iter));
        }
        ++mOverlayCountTable[*iter];
    }
}

void
NBGM_TileMapLayer::RemoveOverlay(const NBGM_TileMapLayerTile& tile)
{
    if(tile.IsBuildingTile())
    {
        return;
    }

    const NBRE_Set<int32>& overlayIds = tile.GetOverlayIds();
    for(NBRE_Set<int32>::const_iterator iter = overlayIds.begin(); iter != overlayIds.end(); ++iter)
    {
        int32 overlayId = *iter;
        nbre_assert(overlayId > 0);

        --mOverlayCountTable[overlayId];
        if(mOverlayCountTable[overlayId] == 0)
        {
            mProfiler->RemoveRenderQueue(*mOverlayManager.FindOverlay(overlayId));
            mOverlayManager.RemoveOverlay(overlayId);
        }
    }
}

nb_boolean
NBGM_TileMapLayer::CheckIfHasLM3D()
{
    VectorMapTileTable::iterator pVectorMapTile = mMapTileTable.begin();
    VectorMapTileTable::iterator pVectorMapTileEnd = mMapTileTable.end();

    for (; pVectorMapTile != pVectorMapTileEnd; ++pVectorMapTile)
    {
        if(pVectorMapTile->second->CheckIfHasLM3D())
        {
            return TRUE;
        }
    }
    return FALSE;
}

void
NBGM_TileMapLayer::DisableUnTexturedLandMarks()
{
    VectorMapTileTable::iterator pVectorMapTile = mMapTileTable.begin();
    VectorMapTileTable::iterator pVectorMapTileEnd = mMapTileTable.end();

    for (; pVectorMapTile != pVectorMapTileEnd; ++pVectorMapTile)
    {
        pVectorMapTile->second->DisableUnTexturedLandMarks();
    }
}

void
NBGM_TileMapLayer::SetRenderRoadToFrameBuffer(nb_boolean value)
{
    if (mRenderRoadToFrameBuffer == value)
    {
        return;
    }

    mRenderRoadToFrameBuffer = value;

    VectorMapTileTable::iterator pVectorMapTile = mMapTileTable.begin();
    VectorMapTileTable::iterator pVectorMapTileEnd = mMapTileTable.end();

    for (; pVectorMapTile != pVectorMapTileEnd; ++pVectorMapTile)
    {
        pVectorMapTile->second->SwitchRoadOverlays(mRenderRoadToFrameBuffer, mTransparentRoadBgOverlay, mBuildingOverlay);
    }
}
