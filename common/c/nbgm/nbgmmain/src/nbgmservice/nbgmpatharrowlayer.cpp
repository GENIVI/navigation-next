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
#include "nbgmpatharrowlayer.h"
#include "nbgmmapmaterialmanager.h"
#include "nbrelog.h"
#include "nbgmconst.h"
#include "nbremath.h"
#include "nbgmbuildutility.h"
#include "nbgm.h"
#include "nbretypeconvert.h"

NBGM_PathArrowLayer::NBGM_PathArrowLayer( NBGM_Context& nbgmContext, NBRE_Node& parentNode, NBRE_SurfaceSubView* subView, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, NBGM_MapViewProfiler* profiler)
    :mParentNode(parentNode),
    mTileDataCreator(nbgmContext),
    mSubView(subView),
    mOverlayManager(overlayManager), 
    mProfiler(profiler)
{
}

NBGM_PathArrowLayer::~NBGM_PathArrowLayer()
{
    TileMap::iterator pTile = mTileMap.begin();
    TileMap::iterator pTileEnd = mTileMap.end();

    for (; pTile != pTileEnd; ++pTile)
    {
        if(pTile->second->Node())
        {
            mParentNode.RemoveChild(pTile->second->Node().get());
        }
        NBRE_DELETE pTile->second;
    }
}

NBGM_IMapLayerTileCreator&
NBGM_PathArrowLayer::GetTileCreator()
{
    return mTileDataCreator;
}

void
NBGM_PathArrowLayer::OnAddTile( const NBGM_VectorTileID& tileId, NBGM_IMapLayerTile* tile)
{
    if(tile != NULL)
    {
        nbre_assert(mTileMap.find(tileId) == mTileMap.end());
        NBGM_PathArrowLayerTile* tileData = static_cast<NBGM_PathArrowLayerTile*> (tile);
        mTileMap[tileId] = tileData;
        mParentNode.AddChild(tileData->Node());
        AddOverlay(*tileData);
    }
}

void
NBGM_PathArrowLayer::OnRemoveTile( const NBGM_VectorTileID& tileId )
{
    TileMap::iterator iter = mTileMap.find(tileId);
    if(iter != mTileMap.end())
    {
        RemoveOverlay(*iter->second);
        if(iter->second->Node())
        {
            mParentNode.RemoveChild(iter->second->Node().get());
        }
        NBRE_DELETE iter->second;
        mTileMap.erase(iter);
    }
}

void
NBGM_PathArrowLayer::OnActiveTile( const NBGM_VectorTileID& tileId )
{
    TileMap::iterator iter = mTileMap.find(tileId);
    if(iter != mTileMap.end())
    {
        iter->second->OnActive();
    }
}

void
NBGM_PathArrowLayer::OnDeactiveTile( const NBGM_VectorTileID& tileId )
{
    TileMap::iterator iter = mTileMap.find(tileId);
    if(iter != mTileMap.end())
    {
        iter->second->OnDeactive();
    }
}

void
NBGM_PathArrowLayer::UpdatePathArrowVisibility( const NBRE_CameraPtr& mCamera )
{
    NBRE_Planef cameraPlane(-mCamera->WorldDirection(), NBRE_TypeConvertf::Convert(mCamera->WorldPosition()));

    TileMap::iterator pMapTile = mTileMap.begin();
    TileMap::iterator pMapTileEnd = mTileMap.end();

    for (; pMapTile != pMapTileEnd; ++pMapTile)
    {
        pMapTile->second->UpdateEntitiesVisibility(cameraPlane);
    }
}

void
NBGM_PathArrowLayer::AddOverlay(const NBGM_PathArrowLayerTile& tile)
{
    int32 overlayId = tile.GetOverlayId();
    if(mOverlayCountTable[overlayId] == 0)
    {
        mProfiler->AddOverlay(overlayId, *mOverlayManager.FindOverlay(overlayId));
    }
    ++mOverlayCountTable[overlayId];
}

void
NBGM_PathArrowLayer::RemoveOverlay(const NBGM_PathArrowLayerTile& tile)
{
    int32 overlayId = tile.GetOverlayId();
    nbre_assert(mOverlayCountTable[overlayId] > 0);
    --mOverlayCountTable[overlayId];
    if(mOverlayCountTable[overlayId] == 0)
    {
        mProfiler->RemoveRenderQueue(*mOverlayManager.FindOverlay(overlayId));
        mOverlayManager.RemoveOverlay(overlayId);
    }
}
