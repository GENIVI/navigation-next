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

/*--------------------------------------------------------------------------

(C) Copyright 2012 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "nbgmmapmaterialfactory.h"
#include "nbgmmapmaterial.h"
#include "nbrerenderpal.h"
#include "nbgmcommon.h"
#include "nbrelog.h"

static const uint32 PaletteImageSize = 8;

NBRE_MapMaterialCreateContext::NBRE_MapMaterialCreateContext()
{
    paletteImage = NBRE_NEW NBRE_Image(PaletteImageSize, PaletteImageSize, NBRE_PF_R8G8B8A8);
    palettePixelOffset = 0;
}

NBRE_MapMaterialCreateContext::~NBRE_MapMaterialCreateContext()
{
    NBRE_DELETE paletteImage;
}

PAL_Error
NBRE_MapMaterialFactory::CreateMapMaterial( NBRE_DOM_MaterialType matType, uint16 matId, NBRE_MapMaterialCreateContext* materialContext, const NBRE_MapMateriaGroup* dependsGroup, NBRE_MapMaterial** material)
{
    if (material == NULL)
    {
        return PAL_ErrBadParam;
    }

    switch(matType)
    {
    case NBRE_DMT_StandardTexturedMaterial:
        *material = NBRE_NEW NBRE_MapStandardTexturedMaterial(matId);
        break;

    case NBRE_DMT_StandardColorMaterial:
        *material = NBRE_NEW NBRE_MapStandardColorMaterial(matId);
        break;

    case NBRE_DMT_OutlinedColorMaterial:
        *material = NBRE_NEW NBRE_MapOutlinedColorsMaterial(matId, materialContext);
        break;

    case NBRE_DMT_LightedMaterial:
        *material = NBRE_NEW NBRE_MapLightedMaterial(matId);
        break;

    case NBRE_DMT_FontMaterial:
        *material = NBRE_NEW NBRE_MapFontMaterial(matId);
        break;

    case NBRE_DMT_LinePatternMaterial:
        *material = NBRE_NEW NBRE_MapPatternLineMaterial(matId, materialContext);
        break;

    case NBRE_DMT_BackgroundMaterial:
        *material = NBRE_NEW NBRE_MapBackgroundColorMaterial(matId, materialContext);
        break;

    case NBRE_DMT_ShieldMaterial:
        *material = NBRE_NEW NBRE_MapShieldMaterial(matId);
        break;

    case NBRE_DMT_DashOutlineMaterial:
        *material = NBRE_NEW NBRE_MapDashLineMaterial(matId, materialContext);
        break;

    case NBRE_DMT_PathArrowMaterial:
        *material = NBRE_NEW NBRE_MapPathArrowMaterial(matId);
        break;

    case NBRE_DMT_HorizontalLightSourceMaterial:
        *material = NBRE_NEW NBRE_MapHorizontalLightSourceMaterial(matId);
        break;

    case NBRE_DMT_PinMaterial:
        *material = NBRE_NEW NBRE_PinMaterial(matId);
        break;

    case NBRE_DMT_OutlinedComplexColorFillMaterial:
        *material = NBRE_NEW NBRE_OutlinedComplexColorFillMaterial(matId);
        break;

    case NBRE_DMT_OutlinedSimpleColorFillMaterial:
        *material = NBRE_NEW NBRE_OutlinedSimpleColorFillMaterial(matId);
        break;

    case NBRE_DMT_ShieldWithYGapMaterial:
        *material = NBRE_NEW NBRE_MapShieldMaterial(matId);
        break;

    case NBRE_DMT_RadialPinMaterial:
        *material = NBRE_NEW NBRE_RadialPINMaterial(matId);
        break;

    case NBRE_DMT_StaticPOIMaterial:
        *material = NBRE_NEW NBRE_StaticPOIMaterial(matId);
        break;

    case NBRE_DMT_PolylineCapMaterial:
        *material = NBRE_NEW NBRE_PolylineCapMaterial(matId, materialContext);
        break;

    case NBRE_DMT_BitMapPatternMaterial:
        *material = NBRE_NEW NBRE_BitMapPatternMaterial(matId, materialContext);
        break;
    case NBRE_DMT_OutlinedHoleyMaterial:
        *material = NBRE_NEW NBRE_OutlinedHoleyMaterial(matId);
        break;
    default:
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_MapMaterialFactory::CreateMapMaterial Unsupported Material!, matType=%d", matType);
        return PAL_ErrUnsupported;
    }

    if (*material == NULL)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_MapMaterialFactory::CreateMapMaterial Material == NULL, matType=%d", matType);
        return PAL_ErrNoMem;
    }
    else
    {
        (*material)->mDependGroup = dependsGroup;
    }

    return PAL_Ok;
}

PAL_Error
NBRE_MapMaterialFactory::CreateMapMaterialFromDom(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, uint16 mtrlID, NBRE_MapMaterialCreateContext* materialContext
                                                  , nb_boolean isCommomMaterial, const NBRE_MapMateriaGroup* dependsGroup, NBRE_DOM_MTRL* domMtrlChunk, NBRE_MapMaterial** material)
{
    uint32 counter = 0;
    uint16 currID = mtrlID;
    PAL_Error err = PAL_ErrUnsupported;

    while (err == PAL_ErrUnsupported && (counter++ < domMtrlChunk->count))
    {
        if(currID >= domMtrlChunk->count)
        {
            NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_MapMaterialFactory::CreateMapMaterialFromDom material backup index out of range! material index = %d, backup level = %d, backup index=%d", mtrlID, counter-1, currID);
            return PAL_ErrBadParam;
        }

        NBRE_DOM_Material& domMaterial = domMtrlChunk->materials[currID];
        currID = domMaterial.backupIndex;

        err = NBRE_MapMaterialFactory::CreateMapMaterial( static_cast<NBRE_DOM_MaterialType>(domMaterial.type), mtrlID, materialContext, dependsGroup, material);
        if (err == PAL_Ok)
        {
            (*material)->Load(palInstance, renderPal, textureManager, dom, domMaterial, mtrlID, isCommomMaterial);
            break;
        }
    }

    return err;
}

PAL_Error
NBRE_MapMaterialFactory::CreateMapMaterialFromDom(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, uint16 mtrlID, nb_boolean isCommomMaterial, NBRE_MapMaterial** material)
{
    return CreateMapMaterialFromDom(palInstance, renderPal, textureManager, dom, mtrlID, NULL, isCommomMaterial, NULL, material);
}

PAL_Error
NBRE_MapMaterialFactory::CreateMapMaterialFromDom(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, uint16 mtrlID, NBRE_MapMaterialCreateContext* materialContext
    , nb_boolean isCommomMaterial, const NBRE_MapMateriaGroup* dependsGroup, NBRE_MapMaterial** material)
{
    NBRE_DOM_MTRL* domMtrlChunk = NULL;
    PAL_Error err = NBRE_DOMGetChunkById(&dom, DOM_ID_MTRL, (NBRE_DOM_Chunk**)&domMtrlChunk);
    if (err != PAL_Ok)
    {
        return err;
    }

    if (domMtrlChunk->count == 0)
    {
        return PAL_ErrBadParam;
    }

    return CreateMapMaterialFromDom(palInstance, renderPal, textureManager, dom, mtrlID, materialContext, isCommomMaterial, dependsGroup, domMtrlChunk, material);
}
