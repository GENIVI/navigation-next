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
#include "nbgmmapmaterialfactory.h"
#include "nbgmmapmaterialutility.h"
#include "nbrerenderpal.h"
#include "nbgmbuildutility.h"
#include "nbgmconst.h"

void
NBRE_MapOutlinedColorsMaterial::LoadImpl(PAL_Instance* /*palInstance*/, NBRE_IRenderPal& renderPal, NBRE_TextureManager& /*textureManager*/, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial)
{
    mOutLineWidth = domMaterial.ocm->outlineWidth/100.f;
    if(isCommomMaterial)
    {
        NBRE_ImageInfo imageInfo;
        mContext->paletteImage->GetInfo(imageInfo);
        int32 x, y;
        NBRE_Color c;

        c.SetAsBGRA(domMaterial.ocm->interiorColor);
        x = mContext->palettePixelOffset%imageInfo.mWidth;
        y = mContext->palettePixelOffset/imageInfo.mWidth;
        ++mContext->palettePixelOffset;
        mContext->paletteImage->SetPixel(x, y, c);
        mInteriorTexcoord.x = (x + 0.5f)/imageInfo.mWidth;
        mInteriorTexcoord.y = (y + 0.5f)/imageInfo.mHeight;

        c.SetAsBGRA(domMaterial.ocm->outlineColor);
        x = mContext->palettePixelOffset%imageInfo.mWidth;
        y = mContext->palettePixelOffset/imageInfo.mWidth;
        ++mContext->palettePixelOffset;
        mContext->paletteImage->SetPixel(x, y, c);
        mOutlineTexcoord.x = (x + 0.5f)/imageInfo.mWidth;
        mOutlineTexcoord.y = (y + 0.5f)/imageInfo.mHeight;
        mShaderNames.push_back(VECTOR_OUTLINE_POLYLINE_TEXTURE_NAME);
    }
    else
    {
        NBRE_Image *image = NBRE_NEW NBRE_Image(2, 1, NBRE_PF_R8G8B8A8);
        uint8 *buffer = image->GetImageData();
        NBRE_Color c;
        c.SetAsBGRA(domMaterial.ocm->interiorColor);
        buffer[0] = static_cast<uint8>(c.r*255);
        buffer[1] = static_cast<uint8>(c.g*255);
        buffer[2] = static_cast<uint8>(c.b*255);
        buffer[3] = static_cast<uint8>(c.a*255);
        c.SetAsBGRA(domMaterial.ocm->outlineColor);
        buffer[4] = static_cast<uint8>(c.r*255);
        buffer[5] = static_cast<uint8>(c.g*255);
        buffer[6] = static_cast<uint8>(c.b*255);
        buffer[7] = static_cast<uint8>(c.a*255);
        mInteriorTexcoord.x = 0.25f;
        mInteriorTexcoord.y = 0.5f;
        mOutlineTexcoord.x = 0.75f;
        mOutlineTexcoord.y = 0.5f;
        NBRE_ShaderPtr shader = NBGM_BuildShaderUtility::CreatePolylineShader(renderPal, image);
        mShaders.push_back(shader);
        mShaderNames.push_back(NBRE_Utility::FormatString("NBRE_MapOutlinedColorsMaterial, internal, dom=%s, id=%d",
            NBRE_DOMGetName(&dom), mtrlID));
    }
}
