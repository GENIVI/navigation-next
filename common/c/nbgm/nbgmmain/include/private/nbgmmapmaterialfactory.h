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

    @file nbgmmapmaterialfactory.h

*/
/*
(C) Copyright 2011 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_MAP_MATERIAL_FACTORY_H_
#define _NBRE_MAP_MATERIAL_FACTORY_H_

#include "nbretypes.h"
#include "nbgmdom.h"
#include "nbrecontext.h"
#include "nbrepacktextureimage.h"
#include "nbreshader.h"
#include "nbreaxisalignedbox2.h"

class NBRE_MapMaterial;
class NBRE_TextureManager;
class NBRE_MapMateriaGroup;

/*! \addtogroup NBRE_Mapdata
*  @{
*/
struct NBRE_MapMaterialCreateContext
{
public:
    NBRE_MapMaterialCreateContext();
    ~NBRE_MapMaterialCreateContext();

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_MapMaterialCreateContext);

public:
    NBRE_Image* paletteImage;
    int32 palettePixelOffset;
};

/*! \addtogroup NBRE_MapMaterial
*  @{
*/
class NBRE_MapMaterialFactory
{
public:
    static PAL_Error CreateMapMaterialFromDom(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, uint16 mtrlID, nb_boolean isCommomMaterial, NBRE_MapMaterial** material);
    static PAL_Error CreateMapMaterialFromDom(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, uint16 mtrlID, NBRE_MapMaterialCreateContext* materialContext, nb_boolean isCommomMaterial, const NBRE_MapMateriaGroup* dependsGroup, NBRE_MapMaterial** material);
    static PAL_Error CreateMapMaterialFromDom(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, uint16 mtrlID, NBRE_MapMaterialCreateContext* materialContext, nb_boolean isCommomMaterial, const NBRE_MapMateriaGroup* dependsGroup, NBRE_DOM_MTRL* domMtrlChunk, NBRE_MapMaterial** material);

private:
    static PAL_Error CreateMapMaterial(NBRE_DOM_MaterialType matType, uint16 matId, NBRE_MapMaterialCreateContext* materialContext, const NBRE_MapMateriaGroup* dependsGroup, NBRE_MapMaterial** material);
};
/** @} */
/** @} */
#endif
