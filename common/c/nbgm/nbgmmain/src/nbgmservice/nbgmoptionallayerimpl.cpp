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

#include "nbgmoptionallayerimpl.h"
#include "nbrememorystream.h"
#include "nbgmtileanimationlayerimpl.h"

NBGM_OptionalLayerImpl::NBGM_OptionalLayerImpl(NBGM_MapViewImpl* mapViewImpl, NBGM_TileMapLayer* mTileMapLayer)
:mMapViewImpl(mapViewImpl)
,mLayer(NULL)
{
    mLayer = NBRE_NEW NBGM_TileAnimationLayerImpl(mMapViewImpl, mTileMapLayer);
    if (mLayer)
    {
        mLayer->SetCurrentFrame(0);
    }
}

NBGM_OptionalLayerImpl::~NBGM_OptionalLayerImpl()
{
    if (mLayer)
    {
        NBRE_DELETE mLayer;
    }
}

PAL_Error
NBGM_OptionalLayerImpl::LoadTiles(const std::vector<NBGM_OptionalLayerData>& tiles, uint8 baseDrawOrder, uint8 labelDrawOrder)
{
    if (mLayer == NULL)
    {
        return PAL_ErrNotFound;
    }

    std::vector<NBGM_TileAnimationData> ts;
    for (uint32 i = 0; i < tiles.size(); ++i)
    {
        const NBGM_OptionalLayerData& data = tiles[i];
        NBGM_TileAnimationData d;
        d.contentId = data.contentId;
        d.dataStream = data.dataStream;
        ts.push_back(d);
    }
    
    PAL_Error result = mLayer->LoadTiles(ts, baseDrawOrder, labelDrawOrder);
    if (result == PAL_Ok)
    {
        std::vector< shared_ptr<std::string> > ids;
        for (uint32 i = 0; i < tiles.size(); ++i)
        {
            const NBGM_OptionalLayerData& data = tiles[i];
            ids.push_back(data.contentId);
        }
        mLayer->UpdateFrame(1, ids);
    }
    return result;
}

PAL_Error 
NBGM_OptionalLayerImpl::UnloadTiles(const std::vector< shared_ptr<std::string> >& contentIds)
{
    if (mLayer == NULL)
    {
        return PAL_ErrNotFound;
    }

    return mLayer->UnloadTiles(contentIds);
}

void 
NBGM_OptionalLayerImpl::Show()
{
    if (mLayer)
    {
        mLayer->SetCurrentFrame(1);
    }
}

void 
NBGM_OptionalLayerImpl::Hide()
{
    if (mLayer)
    {
        mLayer->SetCurrentFrame(-1);
    }
}

void 
NBGM_OptionalLayerImpl::SyncData()
{
    if (mLayer)
    {
        mLayer->SyncData();
    }
}
