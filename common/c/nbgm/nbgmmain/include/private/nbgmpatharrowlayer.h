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

    @file nbgmpatharrowlayer.h
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
#ifndef _NBGM_PATH_ARROW_LAYER_H_
#define _NBGM_PATH_ARROW_LAYER_H_

#include "nbretypes.h"
#include "nbrescenemanager.h"
#include "nbrerenderengine.h"
#include "nbreentity.h"
#include "nbgmcontext.h"
#include "nbgmmaplayer.h"
#include "nbgmvectortiledata.h"
#include "nbgmarray.h"
#include "nbrecamera.h"
#include "nbgmbuildmodelcontext.h"
#include "nbgmpatharrowlayertile.h"
#include "nbgmmapviewprofiler.h"
#include "nbgmdefaultoverlaymanager.h"

/*! \addtogroup NBRE_Service
*  @{
*/

class NBGM_PathArrowLayer : public NBGM_IMapLayer
{
public:
    NBGM_PathArrowLayer(NBGM_Context& nbgmContext, NBRE_Node& parentNode, NBRE_SurfaceSubView* subView, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, NBGM_MapViewProfiler* profiler);
    virtual ~NBGM_PathArrowLayer();

public:
    ///Derive From NBGM_IMapLayer
    virtual NBGM_IMapLayerTileCreator& GetTileCreator();
    virtual void OnAddTile(const NBGM_VectorTileID& tileId, NBGM_IMapLayerTile* tile);
    virtual void OnRemoveTile(const NBGM_VectorTileID& tileId);
    virtual void OnActiveTile(const NBGM_VectorTileID& tileId);
    virtual void OnDeactiveTile(const NBGM_VectorTileID& tileId);

    void UpdatePathArrowVisibility(const NBRE_CameraPtr& mCamera);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_PathArrowLayer);

    void AddOverlay(const NBGM_PathArrowLayerTile& tile);
    void RemoveOverlay(const NBGM_PathArrowLayerTile& tile);

private:
    typedef NBRE_Map<NBGM_VectorTileID, NBGM_PathArrowLayerTile*> TileMap;
    typedef NBRE_Map<uint32, uint32> OverlayCountTable;

    NBRE_Node& mParentNode;
    TileMap mTileMap;
    NBGM_PathArrowLayerTileCreator mTileDataCreator;
    NBRE_SurfaceSubView* mSubView;
    NBRE_DefaultOverlayManager<DrawOrderStruct>& mOverlayManager;
    NBGM_MapViewProfiler* mProfiler;
    OverlayCountTable mOverlayCountTable;
};

/*! @} */

#endif
