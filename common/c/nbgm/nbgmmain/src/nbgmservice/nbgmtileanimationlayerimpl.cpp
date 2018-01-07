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

#include "nbgmtileanimationlayerimpl.h"
#include "nbrememorystream.h"

NBGM_TileAnimationLayerImpl::NBGM_TileAnimationLayerImpl(NBGM_MapViewImpl* mapViewImpl, NBGM_TileMapLayer* tileMapLayer)
:mMapViewImpl(mapViewImpl)
,mTileMapLayer(tileMapLayer)
,mCurrentFrameIndex(-1)
,mAlpha(1.0f)
{

}

NBGM_TileAnimationLayerImpl::~NBGM_TileAnimationLayerImpl()
{
    for (uint32 j = 0; j < mFrames.size(); ++j)
    {
        AnimationTile* at = mFrames[j];
        NBRE_DELETE at;
    }
}

PAL_Error
NBGM_TileAnimationLayerImpl::LoadTiles(const std::vector<NBGM_TileAnimationData>& tiles, uint8 baseDrawOrder, uint8 labelDrawOrder)
{
    PAL_Error err = PAL_Ok;
    for (uint32 i = 0; i < tiles.size(); ++i)
    {
        const NBGM_TileAnimationData& tile = tiles[i];
        NBRE_MemoryStream* ms = NBRE_NEW NBRE_MemoryStream(tile.dataStream.addr, tile.dataStream.size, TRUE);
        NBRE_String id = *(tile.contentId.get());
        NBGM_NBMDataLoadInfo info;
        info.id = id;
        info.materialCategoryName = *tile.materialCategory;
        info.baseDrawOrder = baseDrawOrder;
        info.labelDrawOrder = labelDrawOrder;
        info.stream = shared_ptr<NBRE_IOStream>(ms);
        info.enableLog = FALSE;
        mMapViewImpl->LoadNbmData(info);
        
        nb_boolean found = FALSE;
        for (uint32 j = 0; j < mFrames.size(); ++j)
        {
            AnimationTile* at = mFrames[j];
            if (at->id == id)
            {
                found = TRUE;
                break;
            }
        }
        if (!found)
        {
            int frameIndex = -1;
            TileFrameMap::iterator it = mTileFrameMap.find(id);
            if (it != mTileFrameMap.end())
            {
                // tile frame index is set before loaded
                frameIndex = it->second;
                mTileFrameMap.erase(it);
            }
            mFrames.push_back(NBRE_NEW AnimationTile(id, frameIndex));
        }
    }
    
    return err;
}

PAL_Error 
NBGM_TileAnimationLayerImpl::UpdateFrame(int frameIndex, const std::vector< shared_ptr<std::string> >& contentIds)
{
    PAL_Error err = PAL_Ok;
    for (uint32 i = 0; i < contentIds.size(); ++i)
    {
        std::string* id = contentIds[i].get();
        nb_boolean found = FALSE;
        for (uint32 j = 0; j < mFrames.size(); ++j)
        {
            AnimationTile* at = mFrames[j];
            if (at->id == *id)
            {
                at->frameIndex = frameIndex;
                found = TRUE;
                break;
            }
        }
        if (!found)
        {
            // tile has not been loaded yet
            // record its frame, and set it when loading
            mTileFrameMap[*id] = frameIndex;
        }
    }
    return err;
}

PAL_Error 
NBGM_TileAnimationLayerImpl::SetCurrentFrame(int frameIndex)
{
    PAL_Error err = PAL_Ok;
    mCurrentFrameIndex = frameIndex;
    return err;
}

PAL_Error 
NBGM_TileAnimationLayerImpl::UnloadTiles(const std::vector< shared_ptr<std::string> >& contentIds)
{
    PAL_Error err = PAL_Ok;
    for (uint32 i = 0; i < contentIds.size(); ++i)
    {
        std::string* id = contentIds[i].get();
        mMapViewImpl->UnLoadTile(*id);
        for (uint32 j = 0; j < mFrames.size(); ++j)
        {
            AnimationTile* at = mFrames[j];
            if (at->id == *id)
            {
                NBRE_DELETE at;
                mFrames.erase(mFrames.begin() + j);
                --j;
            }
        }
    }
    return err;
}

PAL_Error 
NBGM_TileAnimationLayerImpl::SetOpacity(uint8 opacity)
{
    PAL_Error err = PAL_Ok;
    mAlpha = NBRE_Math::Clamp<float>(opacity * 0.01f, 0.0f, 1.0f);
    return err;
}

void NBGM_TileAnimationLayerImpl::SyncData()
{
    for (uint32 j = 0; j < mFrames.size(); ++j)
    {
        AnimationTile* at = mFrames[j];
        mTileMapLayer->SetTileOpacity(at->id, mAlpha);
        if (at->frameIndex != -1 && at->frameIndex == mCurrentFrameIndex)
        {   
            mTileMapLayer->OnActiveTile(at->id);
        }
        else
        {
            mTileMapLayer->OnDeactiveTile(at->id);
        }
    }
}

void NBGM_TileAnimationLayerImpl::UnloadAll()
{
    for (uint32 j = 0; j < mFrames.size(); ++j)
    {
        AnimationTile* at = mFrames[j];
        mMapViewImpl->UnLoadTile(at->id);
        NBRE_DELETE at;
    }
    mFrames.clear();
}
