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

    @file nbgmpoilayertile.h
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

#ifndef _NBGM_POI_LAYER_TILE_H_
#define _NBGM_POI_LAYER_TILE_H_
#include "nbgmmaplayer.h"
#include "nbrebillboardnode.h"
#include "nbgmlayoutelement.h"

/*! \addtogroup NBRE_Service
*  @{
*/

class NBGM_PoiLayerTile : public NBGM_IMapLayerTile
{
public:
    NBGM_PoiLayerTile();
    NBGM_PoiLayerTile(NBGM_Context* nbgmContext, const NBRE_String& id, NBRE_BillboardSet* billboardSet, int32 priority);
    ~NBGM_PoiLayerTile();

public:
    void CreateTileItems(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter);
    void OnActive();
    void OnDeactive();
    void UnselectAll();
    nb_boolean SelectIcon(const NBRE_String& id, nb_boolean isSelected);
    nb_boolean GetIconPosition(const NBRE_String& id, NBRE_Point2f& pos);
    nb_boolean GetIconBubblePosition(const NBRE_String& id, NBRE_Point2f& pos);
    void GetIconsAt(const NBRE_Point2f& screenPosition, NBRE_Vector<NBRE_String>& icons);
    void AddToScene();
    NBRE_NodePtr Node();
    void Update();
    void Refresh(float skyHeight);
    void NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType);

private:
    void CreateTilePoi(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter);
    void CreateTileGPin(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter);

private:
    NBGM_Context*    mNBGMContext;
    NBRE_BillboardSet* mBillboardSet;
    NBRE_String mId;
    NBRE_NodePtr mNode;
    NBRE_NodePtr mSubNode;
    NBGM_LayoutElementList mElements;
    NBRE_Point2f mBubbleOffset;
    int32 mPriority;
    nb_boolean mIsActive;
};

class NBGM_PoiLayerTileCreator : public NBGM_IMapLayerTileCreator
{
public:
    NBGM_PoiLayerTileCreator(NBGM_Context& nbgmContext, NBRE_BillboardSet* billboardSet, int32* priority);
    ~NBGM_PoiLayerTileCreator();

public:
    NBGM_IMapLayerTile* CreateMapLayerTile(const NBGM_VectorTileID& tileId, NBGM_NBMData* nbmData, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, nb_boolean enablePicking, uint32 selectMask, const NBRE_String& materialCategoryName);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_PoiLayerTileCreator);

private:
    NBGM_Context&       mNBGMContext;
    NBRE_BillboardSet*  mBillboardSet;
    int32*              mPriority;
};

/*! @} */

#endif
