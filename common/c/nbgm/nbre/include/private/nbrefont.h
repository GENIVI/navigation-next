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

@file nbrefont.h
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
#ifndef _NBRE_FONT_H
#define _NBRE_FONT_H
#include "nbretypes.h"
#include "nbreglyphatlas.h"
#include "nbreglyph.h"
#include "nbrecontext.h"

/** \addtogroup NBRE_Resource
*  @{
*/

/** 
Unicode font base class. Glyphs are loaded on runtime.
*/
class NBRE_UnicodeFont
{
public:
    NBRE_UnicodeFont(NBRE_Context& context, const NBRE_Vector2i textureSize, const NBRE_Vector2i slotSize);
    virtual ~NBRE_UnicodeFont();

public:
    /// Get glyph of char
    NBRE_GlyphPtr GetGlyph(wchar character);
    /// Get texture
    NBRE_TexturePtr GetTexture() const { return mGlyphAtlas->GetTexture(); }
    /// Convert utf-8 to wide
    static NBRE_WString ToUnicode(const char* str, uint32 langCode);

protected:
    /// Create a glyph of char
    virtual NBRE_GlyphPtr CreateGlyph(wchar character) = 0;

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_UnicodeFont);

protected:
    typedef NBRE_Map<wchar, NBRE_GlyphPtr> NBRE_GlyphMap;
    NBRE_GlyphMap mGlyphMap;
    NBRE_GlyphAtlas* mGlyphAtlas;
};

// @TODO: Unless unicode support is required, font should be compiled with ASCII version to get better performance.
//#define NBRE_Font NBRE_AsciiFont
#define NBRE_Font NBRE_UnicodeFont

typedef shared_ptr<NBRE_Font> NBRE_FontPtr;
/** 
A minimum display units of font glyph.
*/
struct NBRE_VisualGlyph
{
public:
    NBRE_VisualGlyph(uint32 fontId, uint32 glyphId):fontId(fontId), glyphId(glyphId)
    {
    }
    ~NBRE_VisualGlyph() {}

public:
    uint32 fontId;
    uint32 glyphId;
    NBRE_AxisAlignedBox2f positions;
};
/** @} */
#endif  //_NBRE_FONT_H
