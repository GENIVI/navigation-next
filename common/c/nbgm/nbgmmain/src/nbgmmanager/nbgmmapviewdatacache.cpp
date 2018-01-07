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

#include "nbgmmapviewdatacache.h"
#include "nbrecache.h"
#include "nbgmmapmaterialmanager.h"
#include "nbreshadermanager.h"
#include "nbrelog.h"
#include "nbgmbinloader.h"
#include "nbrerenderengine.h"
#include "nbgm.h"

class TileCacheStratege : public NBRE_ICacheStrategy<NBGM_VectorTileID, NBGM_VectorTile*>
{
public:
    TileCacheStratege(int32 capability): mCapability(capability){};

public:
    uint32 GetActivedTileNum() const;
    void LogActivedTile() const;

private:
    virtual PAL_Error GetRemovedKeys (NBRE_Vector<NBGM_VectorTileID>& removedKeys)const;
    virtual void OnPut(const NBGM_VectorTileID& key, NBGM_VectorTile*const& val);
    virtual void OnGet (const NBGM_VectorTileID& key);
    virtual void OnTouch (const NBGM_VectorTileID& key);
    virtual nb_boolean OnRealse(const NBGM_VectorTileID& key);
    virtual void OnRemoveAll();
    virtual void Destroy(){ NBRE_DELETE this; }
    virtual void OnLock (const NBGM_VectorTileID& /*key*/){};
    virtual void OnUnlock (const NBGM_VectorTileID& /*key*/){};
    virtual void OnRemove(const NBGM_VectorTileID& key);
    virtual void OnDestroy(NBGM_VectorTile*& val);
    virtual void OnUpdate(const NBGM_VectorTileID& key, NBGM_VectorTile*const& val, NBGM_VectorTile*& oldVal);

private:
    typedef NBRE_Set<NBGM_VectorTileID> VectorTileSet;
    typedef NBRE_List<NBGM_VectorTileID> VectorTileList;

private:
    int32 mCapability;
    VectorTileSet mUsingTileList;
    VectorTileList mUnusingTileList;
};

PAL_Error TileCacheStratege::GetRemovedKeys(NBRE_Vector<NBGM_VectorTileID>& removedKeys)const
{
    removedKeys.clear();
    int32 removeCount = (mUnusingTileList.size()+mUsingTileList.size()) - mCapability;
    for (VectorTileList::const_iterator it = mUnusingTileList.begin(); (it != mUnusingTileList.end())&&(removeCount>0); ++it, --removeCount)
    {
        removedKeys.push_back(*it);
    }
    return PAL_Ok;
}

void TileCacheStratege::OnPut(const NBGM_VectorTileID& key, NBGM_VectorTile*const& /*val*/)
{
    mUsingTileList.insert(key);
}

void TileCacheStratege::OnGet(const NBGM_VectorTileID& /*key*/)
{
}

void TileCacheStratege::OnTouch (const NBGM_VectorTileID& key)
{
    if(mUsingTileList.find(key) == mUsingTileList.end())
    {
        for (VectorTileList::iterator it = mUnusingTileList.begin(); it != mUnusingTileList.end(); ++it)
        {
            if(key == *it)
            {
                mUnusingTileList.erase(it);
                mUsingTileList.insert(key);
                break;
            }
        }
    }
}

nb_boolean TileCacheStratege::OnRealse(const NBGM_VectorTileID& key)
{
    VectorTileSet::iterator iter = mUsingTileList.find(key);
    if(iter == mUsingTileList.end())
    {
        return FALSE;
    }
    mUsingTileList.erase(iter);
    mUnusingTileList.push_back(key);
    return static_cast<int32>(mUnusingTileList.size()+mUsingTileList.size()) > mCapability;;
}

void TileCacheStratege::OnRemoveAll()
{
    mUsingTileList.clear();
    mUnusingTileList.clear();
}

void TileCacheStratege::OnRemove(const NBGM_VectorTileID& key)
{
    for (VectorTileList::iterator it = mUnusingTileList.begin(); it != mUnusingTileList.end(); ++it)
    {
        if(key == *it)
        {
            mUnusingTileList.erase(it);
            break;
        }
    }
}

void TileCacheStratege::OnDestroy(NBGM_VectorTile*& val)
{
    NBRE_DELETE val;
}

void TileCacheStratege::OnUpdate(const NBGM_VectorTileID& /*key*/, NBGM_VectorTile*const& /*val*/, NBGM_VectorTile*& oldVal)
{
    NBRE_DELETE oldVal;
}

uint32 TileCacheStratege::GetActivedTileNum() const
{
    return mUsingTileList.size();
}

void TileCacheStratege::LogActivedTile() const
{
    VectorTileSet::const_iterator iter = mUsingTileList.begin();
    VectorTileSet::const_iterator end =  mUsingTileList.end();
    NBRE_DebugLog(PAL_LogSeverityInfo, "=========================NBGM Current actived tiles, size=%lu=========================", mUsingTileList.size());
    for(; iter!=end; ++iter)
    {
        NBRE_String id = *iter;
        NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM Current actived tiles:%s", id.c_str());
    }
}

NBGM_MapViewDataCache::NBGM_MapViewDataCache(int32 tileCapability):mTileCacheStratege(NBRE_NEW TileCacheStratege(tileCapability)),
    mTileIDs(mTileCacheStratege)
{
}

NBGM_MapViewDataCache::~NBGM_MapViewDataCache()
{
}

uint32 NBGM_MapViewDataCache::GetActivedTileNum() const
{
    return mTileCacheStratege->GetActivedTileNum();
}

void NBGM_MapViewDataCache::LogActivedTile() const
{
    mTileCacheStratege->LogActivedTile();
}

void NBGM_MapViewDataCache::LoadTile(const NBGM_VectorTileID& tileId, NBGM_MapLayerArray& layers, NBGM_NBMData* nbmData, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, nb_boolean enablePicking, uint32 selectMask, const NBRE_String& materialCategoryName, NBRE_Vector<NBGM_VectorTileID>& releasedTiles)
{
    PAL_Error err = PAL_Ok;
    releasedTiles.clear();
    NBGM_VectorTile* tile = NULL;
    err = mTileIDs.Get(tileId, tile);

    if(err != PAL_Ok)
    {
        tile = NBRE_NEW NBGM_VectorTile(tileId, drawOrder, subDrawOrder, labelDrawOrder);
        mTileIDs.Put(tileId, tile, releasedTiles);

        NBGM_VectorTileMapPtr data = NBGM_VectorTile::LoadMapdata(tileId, layers, nbmData, overlayManager, drawOrder, subDrawOrder, labelDrawOrder, enablePicking, selectMask, materialCategoryName);
        tile->BindMapData(data);
        tile->AddToScene();
    }


    NBRE_DebugLog(PAL_LogSeverityDebug,
                  "NBGM_MapViewDataCache::LoadTile( tileId=%s ) succeed, End",
                  tileId.c_str());
}

nb_boolean NBGM_MapViewDataCache::ActiveTile(const NBGM_VectorTileID& tileId)
{
    NBGM_VectorTile* tile = NULL;
    PAL_Error err = mTileIDs.Get(tileId, tile);
    if(err == PAL_Ok)
    {
        mTileIDs.Touch(tileId);
        tile->Active();
        return TRUE;
    }

    return FALSE;
}

nb_boolean NBGM_MapViewDataCache::UnloadTile(const NBGM_VectorTileID& tileId, nb_boolean& needRefresh)
{
    nb_boolean hasRemove = FALSE;
    needRefresh = FALSE;
    if(mTileIDs.Contains(tileId))
    {
        needRefresh = TRUE;
        if(mTileIDs.Release(tileId))
        {
            hasRemove = TRUE;
        }
        else
        {
            NBGM_VectorTile* tile = NULL;
            PAL_Error err = mTileIDs.Get(tileId, tile);
            if(err == PAL_Ok)
            {
                tile->Deactive();
            }
        }
    }
    return hasRemove;
}

void NBGM_MapViewDataCache::UnloadAllTiles(NBRE_Vector<NBGM_VectorTileID>& releasedTiles)
{
    releasedTiles.clear();
    mTileIDs.GetAllKeys(releasedTiles);
    mTileIDs.RemoveAll();
}

void NBGM_MapViewDataCache::GetAllTiles(NBRE_Vector<NBGM_VectorTile*>& allTiles)
{
    allTiles.clear();
    NBRE_Vector<NBGM_VectorTileID> allKeys;
    mTileIDs.GetAllKeys(allKeys);
    for(NBRE_Vector<NBGM_VectorTileID>::iterator iter = allKeys.begin(); iter != allKeys.end(); ++iter)
    {
        NBGM_VectorTile* tile = NULL;
        PAL_Error err = mTileIDs.Get(*iter, tile);
        if(err == PAL_Ok)
        {
            allTiles.push_back(tile);
        }
    }
}

NBGM_VectorTile* NBGM_MapViewDataCache::GetTile(const NBGM_VectorTileID& tileId)
{
    NBGM_VectorTile* tile = NULL;
    PAL_Error err = mTileIDs.Get(tileId, tile);
    if(err == PAL_Ok)
    {
        return tile;
    }

    return NULL;
}

void NBGM_MapViewDataCache::UpdateTile(const NBGM_VectorTileID& tileId, NBGM_MapLayerArray& layers, NBGM_NBMData* nbmData, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, nb_boolean enablePicking, uint32 selectMask, const NBRE_String& materialCategoryName)
{
    if(mTileIDs.Contains(tileId))
    {
        NBGM_VectorTile* tile = NBRE_NEW NBGM_VectorTile(tileId, drawOrder, subDrawOrder, labelDrawOrder);
        mTileIDs.Update(tileId, tile);
        NBGM_VectorTileMapPtr data = NBGM_VectorTile::LoadMapdata(tileId, layers, nbmData, overlayManager, drawOrder, subDrawOrder, labelDrawOrder, enablePicking, selectMask, materialCategoryName);
        tile->BindMapData(data);
        tile->AddToScene();
    }
}
