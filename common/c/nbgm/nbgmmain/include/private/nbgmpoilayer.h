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

    @file nbgmpoilayer.h
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

#ifndef _NBGM_POI_LAYER_H_
#define _NBGM_POI_LAYER_H_
#include "nbretypes.h"
#include "nbrescenemanager.h"
#include "nbrerenderengine.h"
#include "nbrebillboardnode.h"
#include "nbgmvectortile.h"
#include "nbgmvectortiledata.h"
#include "nbgmlayoutelement.h"
#include "nbgmmaplayer.h"
#include "nbgmcontext.h"
#include "nbgmpoilayertile.h"

/*! \addtogroup NBRE_Service
*  @{
*/

/** Poi layer for poi & pin
*/
class NBGM_PoiLayer: public NBGM_IMapLayer
{
private:
    typedef NBRE_Map<NBGM_VectorTileID, NBGM_PoiLayerTile*> DataTileMap;

public:
    NBGM_PoiLayer(NBGM_Context& nbgmContext, NBRE_Node& parentNode, NBRE_BillboardSet* billboardSet);
    virtual ~NBGM_PoiLayer();

public:
    ///Derive From NBGM_IMapLayer
    NBGM_IMapLayerTileCreator& GetTileCreator();
    void OnAddTile(const NBGM_VectorTileID& tileId, NBGM_IMapLayerTile* tile);
    void OnRemoveTile(const NBGM_VectorTileID& tileId);
    void OnActiveTile(const NBGM_VectorTileID& tileId);
    void OnDeactiveTile(const NBGM_VectorTileID& tileId);
    /// Set priority
    void SetPriority(int32 priority) { mPriority = priority; }
    /// Select/unselect
    nb_boolean SelectIcon(const NBRE_String& id, nb_boolean isSelected);
    /// Unselect all icons;
    void UnselectAll();
    /// Get icon position
    nb_boolean GetIconPosition(const NBRE_String& id, NBRE_Point2f& pos);
    nb_boolean GetIconBubblePosition(const NBRE_String& id, NBRE_Point2f& pos);
    /// Get icon at screen position
    void GetIconsAt(const NBRE_Point2f& screenPosition, NBRE_Vector<NBRE_String>& icons);
    /// Update icons
    void Update();
    /// Notify icons extend
    void NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType);

    void Refresh(float skyHeight);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_PoiLayer);

private:
    DataTileMap mTileMap;
    int32 mPriority;
    NBRE_Node& mParentNode;
    NBGM_PoiLayerTileCreator mTileCreator;
    NBRE_Vector<NBGM_VectorTileID> mTileOrder;
};

/*! @} */

#endif


