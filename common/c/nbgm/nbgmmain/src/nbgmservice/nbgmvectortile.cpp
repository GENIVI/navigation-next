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
#include "nbgmvectortile.h"
#include "nbgmvectortiledata.h"
#include "nbgmvectortilebuilder.h"
#include "nbgmlabellayer.h"
#include "nbrelog.h"

NBGM_VectorTile::NBGM_VectorTile(const NBGM_VectorTileID& tileId, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder): 
    mTileId(tileId), mBaseDrawOrder(baseDrawOrder), mSubDrawOrder(subDrawOrder), mLabelDrawOrder(labelDrawOrder), mActived(TRUE)
{
}

NBGM_VectorTile::~NBGM_VectorTile()
{
    if(mTileDataTable)
    {
        for(NBGM_MapLayerTileMap::iterator iter = mTileDataTable->begin(); iter != mTileDataTable->end(); ++iter)
        {
            iter->first->OnRemoveTile(mTileId);
        }
    }
}

const NBGM_VectorTileID& NBGM_VectorTile::Id() const
{
    return mTileId;
}

NBGM_VectorTileMapPtr NBGM_VectorTile::LoadMapdata(const NBGM_VectorTileID& tileId, NBGM_MapLayerArray& mapLayers, NBGM_NBMData* nbmData, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, nb_boolean enablePicking, uint32 selectMask, const NBRE_String& materialCategoryName)
{
    NBGM_VectorTileMapPtr tileDataTablePtr(NBRE_NEW NBGM_MapLayerTileMap());
    for(NBGM_MapLayerArray::iterator iter = mapLayers.begin(); iter != mapLayers.end(); ++iter)
    {
        NBGM_IMapLayerTile* tileData = (*iter)->GetTileCreator().CreateMapLayerTile(tileId, nbmData, overlayManager, drawOrder, subDrawOrder, labelDrawOrder, enablePicking, selectMask, materialCategoryName);
        if(tileData != NULL)
        {
            (*tileDataTablePtr)[(*iter)] = tileData;
        }
    }
    return tileDataTablePtr;
}

void NBGM_VectorTile::BindMapData(const NBGM_VectorTileMapPtr& tileDataTable)
{
    mTileDataTable = tileDataTable;
}

void NBGM_VectorTile::AddToScene()
{
    for(NBGM_MapLayerTileMap::iterator iter = mTileDataTable->begin(); iter != mTileDataTable->end(); ++iter)
    {
        iter->first->OnAddTile(mTileId, iter->second);
    }
}

void NBGM_VectorTile::Deactive()
{
    if(!mActived)
    {
        return;
    }
    for(NBGM_MapLayerTileMap::iterator iter = mTileDataTable->begin(); iter != mTileDataTable->end(); ++iter)
    {
        iter->first->OnDeactiveTile(mTileId);
    }
    mActived = FALSE;
}

void NBGM_VectorTile::Active()
{
    if(mActived)
    {
        return;
    }
    for(NBGM_MapLayerTileMap::iterator iter = mTileDataTable->begin(); iter != mTileDataTable->end(); ++iter)
    {
        iter->first->OnActiveTile(mTileId);
    }
    mActived = TRUE;
}

uint8 NBGM_VectorTile::BaseDrawOrder()
{
    return mBaseDrawOrder;
}

uint8 NBGM_VectorTile::LabelDrawOrder()
{
    return mLabelDrawOrder;
}

int32 NBGM_VectorTile::SubDrawOrder()
{
    return mSubDrawOrder;
}
