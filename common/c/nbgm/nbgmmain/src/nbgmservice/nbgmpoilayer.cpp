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
#include "nbgmpoilayer.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmpointlabel.h"
#include "nbgmicon.h"
#include "nbgmconst.h"
#include "nbrelog.h"
#include "nbgm.h"

NBGM_PoiLayer::NBGM_PoiLayer(NBGM_Context& nbgmContext, NBRE_Node& parentNode, NBRE_BillboardSet* billboardSet)
    :mPriority(0)
    ,mParentNode(parentNode)
    ,mTileCreator(nbgmContext, billboardSet, &mPriority)
{
}

NBGM_PoiLayer::~NBGM_PoiLayer()
{
    DataTileMap::iterator pTileMapEnd = mTileMap.end();
    DataTileMap::iterator iter;
    for (iter = mTileMap.begin(); iter != pTileMapEnd; ++iter)
    {
        if(iter->second->Node())
        {
            mParentNode.RemoveChild(iter->second->Node().get());
        }
        NBRE_DELETE iter->second;
    }
}

NBGM_IMapLayerTileCreator&
NBGM_PoiLayer::GetTileCreator()
{
    return mTileCreator;
}

void
NBGM_PoiLayer::OnAddTile(const NBGM_VectorTileID& tileId, NBGM_IMapLayerTile* tile)
{
    if(tile != NULL)
    {
        nbre_assert(mTileMap.find(tileId) == mTileMap.end());
        NBGM_PoiLayerTile* tileData = static_cast<NBGM_PoiLayerTile*> (tile);
        tileData->AddToScene();
        mParentNode.AddChild(tileData->Node());
        mTileMap[tileId] = tileData;
        mTileOrder.push_back(tileId);
    }
}

void
NBGM_PoiLayer::OnRemoveTile(const NBGM_VectorTileID& tileId)
{
    DataTileMap::iterator i = mTileMap.find(tileId);
    if (i != mTileMap.end())
    {
        if(i->second->Node())
        {
            mParentNode.RemoveChild(i->second->Node().get());
        }
        NBRE_DELETE i->second;
        mTileMap.erase(i);

        NBRE_Vector<NBGM_VectorTileID>::iterator it = find(mTileOrder.begin(), mTileOrder.end(), tileId);
        if (it != mTileOrder.end())
        {
            mTileOrder.erase(it);
        }
    }
}

void
NBGM_PoiLayer::OnActiveTile(const NBGM_VectorTileID& tileId)
{
    DataTileMap::iterator i = mTileMap.find(tileId);
    if (i != mTileMap.end())
    {
        i->second->OnActive();
    }
}

void
NBGM_PoiLayer::OnDeactiveTile(const NBGM_VectorTileID& tileId)
{
    DataTileMap::iterator i = mTileMap.find(tileId);
    if (i != mTileMap.end())
    {
        i->second->OnDeactive();
    }
}

nb_boolean
NBGM_PoiLayer::SelectIcon(const NBRE_String& id, nb_boolean isSelected)
{
    for (DataTileMap::iterator im = mTileMap.begin(); im != mTileMap.end(); ++im)
    {
        NBGM_PoiLayerTile* tileData = im->second;
        if (tileData && tileData->SelectIcon(id, isSelected))
        {
            return TRUE;
        }
    }
    return FALSE;
}

void
NBGM_PoiLayer::UnselectAll()
{
    for (DataTileMap::iterator im = mTileMap.begin(); im != mTileMap.end(); ++im)
    {
        NBGM_PoiLayerTile* tileData = im->second;
        if(tileData)
        {
            tileData->UnselectAll();
        }
    }
}

void
NBGM_PoiLayer::Update()
{
    for (DataTileMap::iterator im = mTileMap.begin(); im != mTileMap.end(); ++im)
    {
        NBGM_PoiLayerTile* tileData = im->second;
        if(tileData)
        {
           tileData->Update();
        }
    }
}

nb_boolean
NBGM_PoiLayer::GetIconPosition(const NBRE_String& id, NBRE_Point2f& pos)
{
    for (DataTileMap::iterator im = mTileMap.begin(); im != mTileMap.end(); ++im)
    {
        NBGM_PoiLayerTile* tileData = im->second;
        if (tileData && tileData->GetIconPosition(id, pos))
        {
            return TRUE;
        }
    }
    return FALSE;
}

nb_boolean
NBGM_PoiLayer::GetIconBubblePosition(const NBRE_String& id, NBRE_Point2f& pos)
{
    for (DataTileMap::iterator im = mTileMap.begin(); im != mTileMap.end(); ++im)
    {
        NBGM_PoiLayerTile* tileData = im->second;
        if (tileData && tileData->GetIconBubblePosition(id, pos))
        {
            return TRUE;
        }
    }
    return FALSE;

}

void
NBGM_PoiLayer::Refresh(float /*skyHeight*/)
{
    // This function will cause an issue with pin.
    // Comment out these line for now, because the sky will be disabled.
    //for (DataTileMap::iterator im = mTileMap.begin(); im != mTileMap.end(); ++im)
    //{
    //   NBGM_PoiLayerTile* tileData = im->second;
    //   if (tileData)
    //   {
    //       tileData->Refresh(skyHeight);
    //    }
    //}
}

void
NBGM_PoiLayer::GetIconsAt(const NBRE_Point2f& screenPosition, NBRE_Vector<NBRE_String>& icons)
{
    NBRE_Point2f sp(screenPosition.x, screenPosition.y);
    for (DataTileMap::iterator im = mTileMap.begin(); im != mTileMap.end(); ++im)
    {
        NBGM_PoiLayerTile* tileData = im->second;
        if(tileData)
        {
            tileData->GetIconsAt(sp, icons);
        }
    }
}

void
NBGM_PoiLayer::NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType)
{
    /// To keep pin's draw order absolutely same as insert order
    /// We have to record the insert order and create mirror nodes with this order
    /// because std::map doesn't access objects by insert order
    for (NBRE_Vector<NBGM_VectorTileID>::iterator it = mTileOrder.begin(); it != mTileOrder.end(); ++it)
    {
        DataTileMap::iterator im = mTileMap.find(*it);
        if (im != mTileMap.end())
        {
            NBGM_PoiLayerTile* tileData = im->second;
            if(tileData)
            {
                tileData->NotifyExtend(extendType);
            }
        }
    }
}
