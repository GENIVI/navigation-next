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

    @file nbgmmapviewdatacache.h
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

#ifndef _NBGM_MAP_VIEW_DATA_CACHE_H_
#define _NBGM_MAP_VIEW_DATA_CACHE_H_
#include "nbgmvectortile.h"
#include "nbgmvectortiledata.h"

class TileCacheStratege;
struct NBGM_Context;

/*! \addtogroup NBGM_Manager
*  @{
*/

/*! NBGM_MapViewData class
 This class responsilble for caching map data(such as tiles, common materials...) for the rendering thread.
 LoadTile/UnloadTile should work in tile-loading thead.
 SyncData should be called in rendering thread and be protected.
 */

class NBGM_MapViewDataCache
{
public:
    NBGM_MapViewDataCache(int32 tileCapability);
    ~NBGM_MapViewDataCache();

public:
    void LoadTile(const NBGM_VectorTileID& tileId, NBGM_MapLayerArray& layers, NBGM_NBMData* nbmData, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, nb_boolean enablePicking, uint32 selectMask, const NBRE_String& materialCategoryName, NBRE_Vector<NBGM_VectorTileID>& releasedTiles);
    nb_boolean ActiveTile(const NBGM_VectorTileID& tileId);
    nb_boolean UnloadTile(const NBGM_VectorTileID& tileId, nb_boolean& needRefresh);
    void UnloadAllTiles(NBRE_Vector<NBGM_VectorTileID>& releasedTiles);
    void GetAllTiles(NBRE_Vector<NBGM_VectorTile*>& allTiles);
    void UpdateTile(const NBGM_VectorTileID& tileId, NBGM_MapLayerArray& layers, NBGM_NBMData* nbmData, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, nb_boolean enablePicking, uint32 selectMask, const NBRE_String& materialCategoryName);
    NBGM_VectorTile* GetTile(const NBGM_VectorTileID& tileId);

    uint32 GetActivedTileNum() const;
    void LogActivedTile() const;

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_MapViewDataCache);

private:
    typedef NBRE_Cache<NBGM_VectorTileID, NBGM_VectorTile*> TileIDCache;

private:
    TileCacheStratege* mTileCacheStratege;
    TileIDCache mTileIDs;
};

/*! @} */

#endif


