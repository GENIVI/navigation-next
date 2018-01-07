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

    @file nbgmpatharrowlayertile.h
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
#ifndef _NBGM_PATH_ARROW_LAYER_TILE_H_
#define _NBGM_PATH_ARROW_LAYER_TILE_H_

#include "nbgmmaplayer.h"
#include "nbgmbuildmodelcontext.h"
#include "nbrerenderengine.h"
#include "nbgmbuildmodelcontext.h"

#define  DEBUG_PATH_ARROW_TRACK 0

/*! \addtogroup NBRE_Service
*  @{
*/

class NBGM_PathArrowLayerTile : public NBGM_IMapLayerTile
{
public:
    NBGM_PathArrowLayerTile(NBGM_Context& nbgmContext, const NBRE_String& id, const NBRE_Point3d& tileCenter, int32 overlayId);
    ~NBGM_PathArrowLayerTile();

public:
    void CreatePathArrow(NBGM_NBMLayerData* layer);

#if DEBUG_PATH_ARROW_TRACK
    void CreateDebugPathArrowTrack(NBRE_Layer* layer, const NBRE_ShaderPtr& shader);
#endif

    NBRE_NodePtr& Node(){ return mNode;}

    void OnActive();
    void OnDeactive();
    void UpdateEntitiesVisibility(const NBRE_Planef& cameraPlane);

    int32 GetOverlayId() const { return mOverlayId; }

private:
    struct EntityAssistInfo
    {
        EntityAssistInfo(float nearVisibility, float farVisibility, NBRE_Entity* entity)
            :mNearVisibility(nearVisibility), mFarVisibility(farVisibility), mEntity(entity){}
        ~EntityAssistInfo(){}

        float        mNearVisibility;
        float        mFarVisibility;
        NBRE_Entity* mEntity;
    };
    typedef NBRE_Vector<EntityAssistInfo> EntityAssistInfoList;

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_PathArrowLayerTile);

private:
    NBGM_Context&             mNBGMContext;
    NBRE_String               mId;
    NBRE_NodePtr              mNode;
    int32                     mOverlayId;
    nb_boolean                mIsActive;
    EntityAssistInfoList      mEntityAssistInfoList;
};

class NBGM_PathArrowLayerTileCreator : public NBGM_IMapLayerTileCreator
{
public:
    NBGM_PathArrowLayerTileCreator(NBGM_Context& nbgmContext);
    ~NBGM_PathArrowLayerTileCreator();

public:
    NBGM_IMapLayerTile* CreateMapLayerTile(const NBGM_VectorTileID& tileId, NBGM_NBMData* nbmData, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, nb_boolean enablePicking, uint32 selectMask, const NBRE_String& materialCategoryName);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_PathArrowLayerTileCreator);

private:
    NBGM_Context&   mNBGMContext;
#if DEBUG_PATH_ARROW_TRACK
    NBRE_ShaderPtr  mArrowTrackShader;
#endif
};

/*! @} */

#endif
