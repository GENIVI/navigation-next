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
#include "nbgmmapmaterialutility.h"
#include "palerror.h"
#include "nbrecommon.h"
#include "nbrepngtextureimage.h"
#include "nbrememorystream.h"
#include "nbgmconst.h"


static NBRE_String
MakeExternalTextureName(NBRE_DOM_Image* domImage)
{
    return NBRE_Utility::FormatString("ExternalTexture, path=%s", (const char *)domImage->textureName);
};

static NBRE_String
MakeInternalTextureName(const char* nbmId, NBRE_DOM_Bitmap* domBitmap)
{
    return NBRE_Utility::FormatString("InternalTexture, nbmID=%s, offset=%d, memory addr=%p", nbmId, domBitmap->offset, domBitmap->imageData);
}

NBRE_String
NBRE_MapMaterialUtility::CreateTexureName(NBRE_DOM& dom, uint16 domTextureId)
{
    PAL_Error err = PAL_Ok;

    if(VERIFY_NBM_INTERNAL_INDEX(domTextureId))
    {
        NBRE_DOM_IMGS* domIMGS = NULL;
        NBRE_DOM_Image* domImage = NULL;

        err = NBRE_DOMGetChunkById(&dom, DOM_ID_IMGS, (NBRE_DOM_Chunk**)&domIMGS);
        nbre_assert (err == PAL_Ok);

        domImage = &domIMGS->images[MAKE_NBM_INDEX(domTextureId)];
        return MakeExternalTextureName(domImage);
    }
    else
    {
        NBRE_DOM_BMAP* domBMAP = NULL;
        NBRE_DOM_Bitmap* domBitmap = NULL;

        err = NBRE_DOMGetChunkById(&dom, DOM_ID_BMAP, (NBRE_DOM_Chunk**)&domBMAP);
        nbre_assert (err == PAL_Ok);

        domBitmap = &domBMAP->bitmaps[MAKE_NBM_INDEX(domTextureId)];
        return MakeInternalTextureName(NBRE_DOMGetName(&dom), domBitmap);
    }
}

NBRE_ITextureImage*
NBRE_MapMaterialUtility::CreateTextureImage(PAL_Instance& pal, NBRE_DOM& dom, uint16 domTextureId, nb_boolean useLowQuality)
{
    NBRE_ITextureImage* result = NULL;

    //external texture
    if(VERIFY_NBM_INTERNAL_INDEX(domTextureId))
    {
        NBRE_DOM_IMGS* domIMGS = NULL;
        NBRE_DOM_Image* domImage = NULL;

        NBRE_DOMGetChunkById(&dom, DOM_ID_IMGS, (NBRE_DOM_Chunk**)&domIMGS);
        nbre_assert (domIMGS != NULL);

        domImage = &domIMGS->images[MAKE_NBM_INDEX(domTextureId)];

        result = NBRE_NEW NBRE_PngTextureImage(pal, (const char*)domImage->textureName, 0, useLowQuality);
    }
    //internal image
    else
    {
        NBRE_DOM_BMAP* domBMAP = NULL;
        NBRE_DOM_Bitmap* domBitmap = NULL;

        NBRE_DOMGetChunkById(&dom, DOM_ID_BMAP, (NBRE_DOM_Chunk**)&domBMAP);
        nbre_assert (domBMAP != NULL);

        domBitmap = &domBMAP->bitmaps[MAKE_NBM_INDEX(domTextureId)];
        NBRE_IOStream* ms = NBRE_NEW NBRE_MemoryStream(domBitmap->imageData, domBitmap->fileSize, TRUE);
        result = NBRE_NEW NBRE_PngTextureImage(pal, ms, 0, useLowQuality);
    }
    return result;
}

NBRE_Color
NBRE_MapMaterialUtility::MakeColor(uint32 value)
{
    uint8 alpha = (uint8)((value & 0x000000FF));
    uint8 red   = (uint8)((value & 0x0000FF00)>>8);
    uint8 green = (uint8)((value & 0x00FF0000)>>16);
    uint8 blue  = (uint8)((value & 0xFF000000)>>24);

    return NBRE_Color(red/255.f, green/255.f, blue/255.f, alpha/255.f);
}

NBRE_Color
NBRE_MapMaterialUtility::MakeColor(float val[])
{
    return NBRE_Color(val[0], val[1], val[2], 1.0f);
}
