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

    @file       palfont.cpp

    See header file for description.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "palfont.h"
#include "palfreetypeengine.h"
typedef FreetypeEngine FontEngine;

struct PAL_Font
{
    FontEngine* ctl;
};
/*! @{ */

// Functions ....................................................................................

PAL_DEF PAL_Font*
PAL_FontCreate(PAL_Instance* pal)
{
    return NULL;
}

PAL_DEF PAL_Font*
PAL_FontCreateEx(PAL_Instance* pal, const char* fontFolder)
{
    PAL_Font* inst = new PAL_Font;
    inst->ctl = new FontEngine(pal, fontFolder);
    return inst;
}

PAL_DEF void
PAL_FontDestroy(PAL_Font* instance)
{
    if (instance != NULL)
    {
        delete instance->ctl;
        delete instance;
    }
}

PAL_DEF PAL_Error
PAL_FontGetComplexTextLayout(PAL_Font* instance, uint32 pixelFontHeight, const char* style, const char* utf8String, uint32 lineWidth, PAL_TEXT_ALIGN align, PAL_GlyphInfo** glyphInfos, uint32* glyphCount, PAL_GlyphBoundingBox* boundingBox)
{
    if (instance == NULL)
    {
        return PAL_ErrNoMem;
    }
    FontEngine* ctl = instance->ctl;

    return ctl->GetComplexTextLayout(pixelFontHeight, style, utf8String, lineWidth, align, glyphInfos, glyphCount, boundingBox);
}

PAL_DEF PAL_Error
PAL_FontGetGlyphBitmap(PAL_Font* instance, uint32 pixelFontHeight, const char* style, uint32 fontId, uint32 glyphID, PAL_GlyphBitmap** result)
{
    PAL_Error err = PAL_Ok;
    if (instance == NULL)
    {
        return PAL_ErrNoMem;
    }

    FontEngine* ctl = instance->ctl;
    PAL_GlyphBitmap* gb = new PAL_GlyphBitmap;
    nsl_memset(gb, 0, sizeof(PAL_GlyphBitmap));
    ctl->GetGlyphBitmap(pixelFontHeight, fontId, glyphID, &gb->data, &gb->width, &gb->height, &gb->xOffset, &gb->yOffset);
    if (gb->data == NULL)
    {

        delete gb;
        *result = NULL;
        return PAL_ErrNotFound;
    }
    *result = gb;

    return err;
}

PAL_DEF PAL_Error
PAL_FontDestroyGlyphLayout(PAL_Font* instance, PAL_GlyphInfo* glyphInfos)
{
    PAL_Error err = PAL_Ok;
    delete[] glyphInfos;
    return err;
}

PAL_DEF PAL_Error
PAL_FontDestroyGlyphBitmap(PAL_Font* instance, PAL_GlyphBitmap* glyphBitmap)
{
    PAL_Error err = PAL_Ok;
    if (glyphBitmap)
    {
        delete[] glyphBitmap->data;
        delete glyphBitmap;
    }
    return err;
}

/*! @} */
