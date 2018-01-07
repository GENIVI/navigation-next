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
#include "nbreshader.h"
#include "nbreshadermanager.h"
#include "nbretexturemanager.h"
#include "nbrefilestream.h"
#include "nbrememorystream.h"
#include "nbrepngtextureimage.h"
#include "nbrerenderpal.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmmapmaterialutility.h"

static NBRE_TexturePtr
CreateRepeatImage(NBRE_IRenderPal& renderPal, int32 width, uint8* sideColors, uint32 numSideColors, uint8* topColor, int32 repeat)
{
    NBRE_Image** images = NBRE_NEW NBRE_Image*[1];

    images[0] = NBRE_NEW NBRE_Image(width, 2, NBRE_PF_R8G8B8);
    uint8* pixels =images[0]->GetImageData();
    nsl_memset(pixels, 0, width * 2 * 3);
    if(pixels != NULL)
    {
        int32 i;
        uint32 range = width / numSideColors - 1;

        for(i = 0; i < width; ++i)
        {
            uint8 color[3];

            uint32 section = numSideColors * i / width;
            uint32 t = i % (width / numSideColors);
            for (int j = 0; j < 3; ++j)
            {
                uint32 lerpColor = (sideColors[section * 3 + j] * (range - t) + 
                    sideColors[((section + 1) % numSideColors) * 3 + j] * t) / range;
                color[j] = (uint8)lerpColor;
            }

            pixels[i*3] = color[0];
            pixels[i*3+1] = color[1];
            pixels[i*3+2] = color[2];
        }

        for(i = width; i < width*2; ++i)
        {
            pixels[i*3] = topColor[0];
            pixels[i*3+1] = topColor[1];
            pixels[i*3+2] = topColor[2];

        }
    }


    return NBRE_TexturePtr(renderPal.CreateTexture(images, 1, 1, FALSE, NBRE_Texture::TT_2D, "NBRE_MapHorizontalLightSourceMaterial"));
}

void ConvertColor(uint32 value, uint8* output)
{
    uint8 red   = (uint8)((value & 0x0000FF00)>>8);
    uint8 green = (uint8)((value & 0x00FF0000)>>16);
    uint8 blue  = (uint8)((value & 0xFF000000)>>24);
    output[0] = red;
    output[1] = green;
    output[2] = blue;
}

void
NBRE_MapHorizontalLightSourceMaterial::LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    NBRE_TextureUnit texUnit;

    uint8 sideColors[] = {255,0,0,  0,255,0,  0,0, 255,  255,255,0};
    uint8 topColor[] = {255, 255, 255};

    if (domMaterial.hlsm)
    {
        ConvertColor(domMaterial.hlsm->degree0Color, sideColors + 0);
        ConvertColor(domMaterial.hlsm->degree90Color, sideColors + 3);
        ConvertColor(domMaterial.hlsm->degree180Color, sideColors + 6);
        ConvertColor(domMaterial.hlsm->degree270Color, sideColors + 9);
        ConvertColor(domMaterial.hlsm->topColor, topColor);
    }
    

    texUnit.SetTexture(CreateRepeatImage(renderPal, 128, sideColors, 4, topColor, 1));

    NBRE_Pass* pass = NBRE_NEW NBRE_Pass();
    pass->GetTextureUnits().push_back(texUnit);
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(TRUE);

    NBRE_Shader* shader = NBRE_NEW NBRE_Shader();
    shader->AddPass(NBRE_PassPtr(pass));

    mShaders.push_back(NBRE_ShaderPtr(shader));
    if (isCommomMaterial)
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MapHorizontalLightSourceMaterial, External, id=%d", mtrlID));
    }
    else
    {
        mShaderNames.push_back( NBRE_Utility::FormatString("NBRE_MapHorizontalLightSourceMaterial, Internal, id=%d", mtrlID));
    }
}
