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
#include "nbgmmapmaterial.h"
#include "nbrelog.h"
#include "palstdlib.h"
#include "nbgmmapmaterialfactory.h"
#include "nbgmmapmaterialutility.h"
#include "nbgmconst.h"
#include "nbrerenderpal.h"

typedef enum NBRE_PatternDashLineType
{
    NBRE_PALT_PatternLine,
    NBRE_PALT_DashOutline,
}NBRE_PatternDashLineType;

typedef struct PatternDashRoadInfo
{
    uint32          pattern;
    NBRE_Color      bitOnColor;
    NBRE_Color      bitOffColor;
    NBRE_Color      color;
    float           outlineWidth;
}PatternDashRoadInfo;

static void
BlendColor(float scale, const NBRE_Color& bitOnColor, const NBRE_Color& bitOffColor, NBRE_Color& outputColor)
{
    if (scale > 1.0f)
    {
        scale = 1.0f;
    }
    else if (scale < 0.0f)
    {
        scale = 0.0f;
    }

    outputColor.r = (bitOnColor.r * scale + bitOffColor.r * (1.0f - scale));
    outputColor.g = (bitOnColor.g * scale + bitOffColor.g * (1.0f - scale));
    outputColor.b = (bitOnColor.b * scale + bitOffColor.b * (1.0f - scale));
    outputColor.a = (bitOnColor.a * scale + bitOffColor.a * (1.0f - scale));
}

static void
ComputeColor(uint32 pattern, uint32 width, const NBRE_Color& bitOnColor, const NBRE_Color& bitOffColor, NBRE_Color& outputColor)
{
    uint32 patternCopy = pattern;

    if (width >= 32)
    {
        if ((patternCopy & 0x00000001) == 1)
        {
            BlendColor(1.0f, bitOnColor, bitOffColor, outputColor);
        }
        else
        {
            BlendColor(0.0f, bitOnColor, bitOffColor, outputColor);
        }
    }
    else
    {
        uint32 sum = 0;
        uint32 count = 32 / width;

        for (uint32 i = 0; i < count; i++)
        {
            sum += patternCopy & 0x00000001;
            patternCopy >>= 1;
        }
        float scale = static_cast<float>(sum) / count;
        BlendColor(scale, bitOnColor, bitOffColor, outputColor);
    }
}

uint8*
CreatePatternBitMap1D(uint32 pattern, uint32 width, const NBRE_Color& bitOnColor, const NBRE_Color& bitOffColor)
{
    uint32 count = width << 2;
    uint8* pixels = NBRE_NEW uint8[count];
    if(pixels == NULL)
    {
        return NULL;
    }
    nsl_memset(pixels, 0, sizeof(uint8)*count);

    nb_boolean isExpand = FALSE;
    uint32 moveBitNum = 0;

    if(width > 32)
    {
        isExpand = TRUE;
        moveBitNum = width / 32;
    }
    else
    {
        moveBitNum = 32 / width;
    }

    NBRE_Color color;
    uint32 index = 0;
    uint32 sum = 0;

    for (uint32 i = 0; i < width; i++)
    {
        ComputeColor(pattern, width, bitOnColor, bitOffColor, color);

        pixels[index++] = static_cast<uint8>(color.r * 255);
        pixels[index++] = static_cast<uint8>(color.g * 255);
        pixels[index++] = static_cast<uint8>(color.b * 255);
        pixels[index++] = static_cast<uint8>(color.a * 255);

        if (isExpand)
        {
            if (sum == (moveBitNum - 1))
            {
                pattern >>= 1;
                sum = 0;
            }
            else
            {
                sum++;
            }
        }
        else
        {
            pattern >>= moveBitNum;
        }
    }

    return pixels;
}

static NBRE_Image*
CreatePatternDashLineBmp2D(uint32 width, uint32 height, const PatternDashRoadInfo& info, NBRE_PatternDashLineType type)
{
    NBRE_Image* result = NBRE_NEW NBRE_Image(width, height, NBRE_PF_R8G8B8A8);

    uint32 count1D = width << 2;
    uint32 outlineWidthIntegerPart = static_cast<uint32>(info.outlineWidth * (height >> 1));
    float  outlineWidthDecimalPart = info.outlineWidth * (height >> 1) - outlineWidthIntegerPart;
    const uint32 NO_PATTERN  = 0xffffffff;

    uint8* pixels2D = result->GetImageData();

    uint8* patternPixels1D = CreatePatternBitMap1D(info.pattern, width, info.bitOnColor, info.bitOffColor);
    if (patternPixels1D == NULL)
    {
        NBRE_DELETE result;
        return NULL;
    }

    uint8* noPatternPixels1D = CreatePatternBitMap1D(NO_PATTERN, width, info.color, info.color);
    if (noPatternPixels1D == NULL)
    {
        NBRE_DELETE result;
        NBRE_DELETE_ARRAY patternPixels1D;
        return NULL;
    }

    uint8* transparentPixels1D = NBRE_NEW uint8[count1D];
    if (transparentPixels1D == NULL)
    {
        NBRE_DELETE result;
        NBRE_DELETE_ARRAY patternPixels1D;
        NBRE_DELETE_ARRAY noPatternPixels1D;
        return NULL;
    }

    uint32 i = 0;
    uint32 transparentRow = 2;
    uint32 transitionRow = 0;
    uint32 halfHeight = height >> 1;

    switch(type)
    {
    case NBRE_PALT_PatternLine:

        // create transparent row
        nsl_memcpy(transparentPixels1D, noPatternPixels1D, count1D);

        for (i = 3; i < count1D; i += 4)
        {
            transparentPixels1D[i] = 0x00;
        }

        for(i = 0; i < transparentRow; i++)
        {
            nsl_memcpy(pixels2D + count1D * i, transparentPixels1D, count1D);
        }

        // create outline row
        for(i = transparentRow; i < outlineWidthIntegerPart + transparentRow; i++)
        {
            nsl_memcpy(pixels2D + count1D * i, noPatternPixels1D, count1D);
        }

        // create transition row
        for (i = 0; i < width; i++)
        {
            uint32 index = i << 2;

            noPatternPixels1D[index]     = static_cast<uint8>(noPatternPixels1D[index]     * outlineWidthDecimalPart + patternPixels1D[index]     * (1 - outlineWidthDecimalPart));
            noPatternPixels1D[index + 1] = static_cast<uint8>(noPatternPixels1D[index + 1] * outlineWidthDecimalPart + patternPixels1D[index + 1] * (1 - outlineWidthDecimalPart));
            noPatternPixels1D[index + 2] = static_cast<uint8>(noPatternPixels1D[index + 2] * outlineWidthDecimalPart + patternPixels1D[index + 2] * (1 - outlineWidthDecimalPart));
            noPatternPixels1D[index + 3] = static_cast<uint8>(noPatternPixels1D[index + 3] * outlineWidthDecimalPart + patternPixels1D[index + 3] * (1 - outlineWidthDecimalPart));
        }
        for (i = outlineWidthIntegerPart + transparentRow; i < outlineWidthIntegerPart + transparentRow + transitionRow; i++)
        {
            nsl_memcpy(pixels2D + count1D * i, noPatternPixels1D, count1D);
        }

        // create pattern row
        for (i = outlineWidthIntegerPart + transparentRow + transitionRow; i < halfHeight; i++)
        {
            nsl_memcpy(pixels2D + count1D * i, patternPixels1D, count1D);
        }
        break;

    case NBRE_PALT_DashOutline:

        // create transparent row
        nsl_memcpy(transparentPixels1D, patternPixels1D, count1D);

        for (i = 3; i < count1D; i += 4)
        {
            transparentPixels1D[i] = 0x00;
        }

        for(i = 0; i < transparentRow; i++)
        {
            nsl_memcpy(pixels2D + count1D * i, transparentPixels1D, count1D);
        }

        // create pattern line row
        outlineWidthIntegerPart += 1;
        for(i = transparentRow; i < outlineWidthIntegerPart + transparentRow; i++)
        {
            nsl_memcpy(pixels2D + count1D * i, patternPixels1D, count1D);
        }

        // create pattern row
        for (i = outlineWidthIntegerPart + transparentRow; i < halfHeight; i++)
        {
            nsl_memcpy(pixels2D + count1D * i, noPatternPixels1D, count1D);
        }
        break;

    default:
        NBRE_DebugLog(PAL_LogSeverityCritical, "CreatePatternDashLineBmp2D failed, type=%d", type);
        nbre_assert(0);
    }

    // create symmetrical part
    for (i = halfHeight; i < height; i++)
    {
        nsl_memcpy(pixels2D + count1D * i, pixels2D + count1D * (height - i - 1), count1D);
    }

    NBRE_DELETE_ARRAY patternPixels1D;
    NBRE_DELETE_ARRAY noPatternPixels1D;
    NBRE_DELETE_ARRAY transparentPixels1D;

    return result;
}

static NBRE_Image*
CreateDashOrPatternTexureImage(PatternDashRoadInfo& info, NBRE_PatternDashLineType type)
{
    uint32 height = 0;
    uint32 width = 0;

    switch(type)
    {
    case NBRE_PALT_PatternLine:
        height = PACK_TEXTURE_IMAGE_WIDTH >> 3;
        width = PACK_TEXTURE_IMAGE_WIDTH;
        break;

    case NBRE_PALT_DashOutline:
        height = PACK_TEXTURE_IMAGE_WIDTH >> 3;
        width = PACK_TEXTURE_IMAGE_WIDTH;
        break;

    default:
        NBRE_DebugLog(PAL_LogSeverityCritical, "CreateDashOrPatternTexureImage failed, type=%d", type);
        nbre_assert(0);
    }


    NBRE_Image* image = CreatePatternDashLineBmp2D(width, height, info, type);
    if(image == NULL)
    {
        return NULL;
    }

    NBRE_Image* newImage = image->ConvertToFormat(NBRE_PF_R4G4B4A4);
    NBRE_DELETE image;

    return newImage;
}

void
NBRE_MapPatternLineMaterial::LoadImpl(PAL_Instance* /*palInstance*/, NBRE_IRenderPal& renderPal, NBRE_TextureManager& /*textureManager*/, NBRE_DOM& /*dom*/, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    PatternDashRoadInfo info;
    info.bitOnColor = NBRE_MapMaterialUtility::MakeColor(domMaterial.lpm->bitOnColor);
    info.bitOffColor = NBRE_MapMaterialUtility::MakeColor(domMaterial.lpm->bitOffColor);
    info.color = NBRE_MapMaterialUtility::MakeColor(domMaterial.lpm->outlineColor);
    info.pattern = domMaterial.lpm->linePattern;
    info.outlineWidth = domMaterial.lpm->outlineWidth / 100.0f;

    NBRE_Image* image = CreateDashOrPatternTexureImage(info, NBRE_PALT_PatternLine);
    NBRE_Image** images = NBRE_NEW NBRE_Image*[1];
    images[0] = image;

    NBRE_Pass* pass = NBRE_NEW NBRE_Pass();
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);
    pass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
    pass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);

    NBRE_TextureUnit texUnit;
    texUnit.SetTexture(NBRE_TexturePtr(renderPal.CreateTexture(images, 1, 1, TRUE, NBRE_Texture::TT_2D, "NBRE_MapPatternLineMaterial")));
    texUnit.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_REPEAT, NBRE_TAM_CLAMP, FALSE));
    pass->GetTextureUnits().push_back(texUnit);

    NBRE_Shader* shader = NBRE_NEW NBRE_Shader();
    shader->AddPass(NBRE_PassPtr(pass));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MapPatternLineMaterial, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MapPatternLineMaterial, Internal, id=%d", mtrlID));
    }
    mShaders.push_back(NBRE_ShaderPtr(shader));
}

void
NBRE_MapDashLineMaterial::LoadImpl(PAL_Instance* /*palInstance*/, NBRE_IRenderPal& renderPal, NBRE_TextureManager& /*textureManager*/, NBRE_DOM& /*dom*/, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    PatternDashRoadInfo info;
    info.bitOnColor = NBRE_MapMaterialUtility::MakeColor(domMaterial.dom->bitOnColor);
    info.bitOffColor = NBRE_MapMaterialUtility::MakeColor(domMaterial.dom->bitOffColor);
    info.color = NBRE_MapMaterialUtility::MakeColor(domMaterial.dom->interiorColor);
    info.pattern = domMaterial.dom->pattern;
    info.outlineWidth = domMaterial.dom->outlineWidth / 100.0f;

    NBRE_Image* image = CreateDashOrPatternTexureImage(info, NBRE_PALT_DashOutline);
    NBRE_Image** images = NBRE_NEW NBRE_Image*[1];
    images[0] = image;

    NBRE_Pass* pass = NBRE_NEW NBRE_Pass();
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);
    pass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
    pass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);

    NBRE_TextureUnit texUnit;
    texUnit.SetTexture(NBRE_TexturePtr(renderPal.CreateTexture(images, 1, 1, FALSE, NBRE_Texture::TT_2D, "NBRE_MapDashLineMaterial")));
    texUnit.SetState(NBRE_TextureState(NBRE_TFT_LINEAR, NBRE_TFT_LINEAR, NBRE_TAM_REPEAT, NBRE_TAM_CLAMP, FALSE));
    pass->GetTextureUnits().push_back(texUnit);

    NBRE_Shader* shader = NBRE_NEW NBRE_Shader();
    shader->AddPass(NBRE_PassPtr(pass));

    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MapDashLineMaterial, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MapDashLineMaterial, Internal, id=%d", mtrlID));
    }
    mShaders.push_back(NBRE_ShaderPtr(shader));
}
