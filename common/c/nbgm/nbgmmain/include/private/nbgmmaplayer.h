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

    @file nbgmmaplayer.h
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

#ifndef _NBGM_MAP_LAYER_H_
#define _NBGM_MAP_LAYER_H_
#include "nbretypes.h"
#include "nbgmnbmdata.h"
#include "nbgmdefaultoverlaymanager.h"
#include "nbgmdraworder.h"

/*! \addtogroup NBGM_Service
*  @{
*/
enum NBGM_TileMapBoundaryExtendType
{
    NBGM_TMBET_NONE = 0,
    NBGM_TMBET_LEFT,
    NBGM_TMBET_RIGHT,
};

typedef NBRE_String NBGM_VectorTileID;

class NBGM_IMapLayerTile
{
protected:
    NBGM_IMapLayerTile() {};
public:
    virtual ~NBGM_IMapLayerTile() {};
};


class NBGM_IMapLayerTileCreator
{
public:
    virtual NBGM_IMapLayerTile* CreateMapLayerTile(const NBGM_VectorTileID& tileId, NBGM_NBMData* nbmData, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, nb_boolean enablePicking, uint32 selectMask, const NBRE_String& materialCategoryName) = 0;
};


class NBGM_IMapLayer
{
public: 
    /// Add tile, running in rendering thread
    virtual void OnAddTile(const NBGM_VectorTileID& tileId, NBGM_IMapLayerTile* tile) = 0;
    /// Remove tile, running in rendering thread
    virtual void OnRemoveTile(const NBGM_VectorTileID& tileId) = 0;
    /// Show tile, running in rendering thread
    virtual void OnActiveTile(const NBGM_VectorTileID& tileId) = 0;
    /// Hide tile, running in rendering thread
    virtual void OnDeactiveTile(const NBGM_VectorTileID& tileId) = 0;
    /// Get the data creator to generate the map layer data
    virtual NBGM_IMapLayerTileCreator& GetTileCreator() = 0;
};

typedef NBRE_List<NBGM_IMapLayer*> NBGM_MapLayerArray;
typedef NBRE_Map<NBGM_IMapLayer*, NBGM_IMapLayerTile*> NBGM_MapLayerTileMap;

/*! @} */

#endif


