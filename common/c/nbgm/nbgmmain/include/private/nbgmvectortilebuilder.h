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

    @file nbgmvectortilebuilder.h
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

#ifndef _NBGM_VECTOR_TILE_BUILDER_H_
#define _NBGM_VECTOR_TILE_BUILDER_H_
#include "nbgmvectortiledata.h"
#include "nbgmcontext.h"
#include "nbgmbuildmodelcontext.h"

/*! \addtogroup NBRE_Service
*  @{
*/

/** Defines an instance manage all layer in one vector tile.
*/

class NBRE_Model;
class NBRE_SubMode;

typedef NBRE_Map<uint16, NBRE_Model*> NBGM_DrawOrderModelMap;
typedef NBRE_Map<uint16, NBRE_Set<NBRE_SubModel*>*> NBGM_DrawOrderSubModelMap;

class NBGM_VectorTileBuilder
{
public:
    static PAL_Error PolylinePack2Tristripe(NBRE_Array* polyline,
                                            float width,
                                            NBGM_FloatBuffer& vertexBuffer,
                                            NBGM_FloatBuffer& normalBuffer,
                                            NBGM_FloatBuffer& texcoordBuffer,
                                            NBGM_ShortBuffer& indexBuffer);

    static PAL_Error CreateVectorRoadModel(NBGM_ResourceContext& resourceContext,
                                           NBRE_Layer* dataTile,
                                           NBGM_DrawOrderModelMap* foreground,
                                           NBGM_DrawOrderModelMap* background,
                                           NBGM_DrawOrderSubModelMap& textureAnimatedSubModels,
                                           const NBRE_String& materialCategoryName);

    static PAL_Error CreateVectorRoadModel(NBGM_ResourceContext& resourceContext,
                                           NBRE_Layer* dataTile,
                                           NBRE_Model* foreground,
                                           NBRE_Model* background,
                                           NBRE_Set<NBRE_SubModel*>& textureAnimatedSubModels,
                                           const NBRE_String& materialCategoryName);

    //static NBRE_Entity* CreateVectorBackgroundRoadEntity(NBGM_Context& nbgmContext, NBRE_Layer* dataTile);
    //static NBRE_Entity* CreateVectorForegroundRoadEntity(NBGM_Context& nbgmContext, NBRE_ITextureAnimation& texAnimation, NBRE_Layer* dataTile);
    static NBRE_Model* CreateVectorAreaModel(NBGM_ResourceContext& resourceContext,
                                             NBRE_Layer* dataTile,
                                             const NBRE_Point3d& tileCenter,
                                             const NBRE_String& materialCategoryName);

    static NBRE_Model* CreateVectorAreaWireFrameModel(NBGM_ResourceContext& resourceContext,
                                                      NBRE_Layer* dataTile,
                                                      const NBRE_Point3d& tileCenter,
                                                      const NBRE_String& materialCategoryName);
};

#endif

