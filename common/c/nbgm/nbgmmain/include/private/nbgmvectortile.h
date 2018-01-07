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

    @file nbgmvectortile.h
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

#ifndef _NBGM_VECTOR_TILE_H_
#define _NBGM_VECTOR_TILE_H_
#include "nbretypes.h"
#include "nbrevector3.h"
#include "nbrenode.h"
#include "nbgmmaplayer.h"

struct NBRE_Mapdata;

/*! \addtogroup NBRE_Service
*  @{
*/

typedef shared_ptr<NBGM_MapLayerTileMap> NBGM_VectorTileMapPtr;

/** Defines an instance manage all layer in one vector tile.
*/
class NBGM_VectorTile 
{

public:
    explicit NBGM_VectorTile(const NBGM_VectorTileID& tileId, uint8 baseDrawOrder, int32 subDrawOrder, uint8 labelDrawOrder);
    ~NBGM_VectorTile();

public:
    static NBGM_VectorTileMapPtr LoadMapdata(const NBGM_VectorTileID& tileId, NBGM_MapLayerArray& mapLayers, NBGM_NBMData* nbmData, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, nb_boolean enablePicking, uint32 selectMask, const NBRE_String& materialCategoryName);

public:
    const NBGM_VectorTileID& Id() const;
    void BindMapData(const NBGM_VectorTileMapPtr& tileDataTable);
    void AddToScene();
    void Active();
    void Deactive();
    uint8 BaseDrawOrder();
    uint8 LabelDrawOrder();
    int32 SubDrawOrder();

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_VectorTile);

private:
    NBGM_VectorTileID mTileId;
    NBGM_VectorTileMapPtr mTileDataTable;
    uint8 mBaseDrawOrder;
    int32 mSubDrawOrder;
    uint8 mLabelDrawOrder;
    nb_boolean mActived;
};

/*! @} */

#endif

