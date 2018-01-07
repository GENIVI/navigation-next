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

    @file nbgmtileanimationlayerimpl.h.h
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
#ifndef _NBGM_TILE_ANIMATION_LAYER_IMPL_H_
#define _NBGM_TILE_ANIMATION_LAYER_IMPL_H_
#include "nbretypes.h"
#include "nbrecommon.h"
#include "nbgmtileanimationlayer.h"
#include "nbgmmapviewimpl.h"
#include "nbgmtilemaplayer.h"

/*! \addtogroup NBGM_Service
*  @{
*/
class NBGM_TileAnimationLayerImpl:
    public NBGM_TileAnimationLayer
{
public:
    NBGM_TileAnimationLayerImpl(NBGM_MapViewImpl* mapViewImpl, NBGM_TileMapLayer* tileMapLayer);
    virtual ~NBGM_TileAnimationLayerImpl();

    virtual PAL_Error LoadTiles(const std::vector<NBGM_TileAnimationData>& tiles, uint8 baseDrawOrder, uint8 labelDrawOrder);
    virtual PAL_Error UpdateFrame(int frameIndex, const std::vector< shared_ptr<std::string> >& contentIds);
    virtual PAL_Error SetCurrentFrame(int frameIndex);
    virtual PAL_Error UnloadTiles(const std::vector< shared_ptr<std::string> >& contentIds);
    virtual PAL_Error SetOpacity(uint8 opacity);

    void SyncData();
    void UnloadAll();

private:
    struct AnimationTile
    {
    public:
        AnimationTile(const NBRE_String& id, int frameIndex):id(id), frameIndex(frameIndex) {}
    public:
        NBRE_String id;
        int frameIndex;
    };
    typedef NBRE_Vector<AnimationTile*> AnimationTileList;
    typedef NBRE_Map<NBRE_String, int> TileFrameMap;
    NBGM_MapViewImpl* mMapViewImpl;
    NBGM_TileMapLayer* mTileMapLayer;
    AnimationTileList mFrames;
    int mCurrentFrameIndex;
    TileFrameMap mTileFrameMap;
    float mAlpha;
};

/*! @} */

#endif
