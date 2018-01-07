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

@file nbreglyph.h
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
#ifndef _NBRE_GLYPH_H
#define _NBRE_GLYPH_H
#include "nbretypes.h"
#include "nbreaxisalignedbox2.h"
#include "nbreimage.h"
#include "nbresharedptr.h"

/** \addtogroup NBRE_Resource
*  @{
*/
/** 
Glyph is the bitmap cache of characters.
It also stores texture coordinates of the character;
*/
class NBRE_Glyph 
{
    friend class NBRE_GlyphAtlasSlot;
    friend class NBRE_AsciiGlyphAtlasSlot;
public:
    NBRE_Glyph(wchar character, const NBRE_AxisAlignedBox2i& foreTexcoord, const NBRE_AxisAlignedBox2i& backTexcoord, const NBRE_ImageList& bitmaps, int32 baseFontHeight);
    
    NBRE_Glyph(uint32 fontId, uint32 glyphId, int32 xOffset, int32 yOffset, uint32 atlasWidth, uint32 atlasHeight
        , const NBRE_AxisAlignedBox2i& foreTexcoord, const NBRE_AxisAlignedBox2i& backTexcoord, int32 baseFontHeight);
    ~NBRE_Glyph();

public:
    const NBRE_AxisAlignedBox2i& GetForegroundTexcoord() const { return mLocalForegroundTexcoord; }
    const NBRE_AxisAlignedBox2i& GetBackgroundTexcoord() const { return mLocalBackgroundTexcoord; }
    NBRE_Image* GetBitmap(uint32 level) const;
    NBRE_Vector2f GetForegroundSize(float fontHeight) const { return mForegroundBaseSize * fontHeight; }
    NBRE_Vector2f GetBackgroundSize(float fontHeight) const { return mBackgroundBaseSize * fontHeight; }
    NBRE_Vector2i GetOffset() const { return mOffset; }
    const NBRE_Vector2f* GetAtlasForegroundTexcoord() const { return mAtlasForegroundTexcorrd; }
    const NBRE_Vector2f* GetAtlasBackgroundTexcoord() const { return mAtlasBackgroundTexcorrd; }
    wchar GetCharacter() const { return mCharacter; }

private:
    wchar mCharacter;
    NBRE_AxisAlignedBox2i mLocalForegroundTexcoord;
    NBRE_AxisAlignedBox2i mLocalBackgroundTexcoord;
    NBRE_ImageList mBitmaps;
    int32 mBaseFontHeight;
    NBRE_Vector2f mForegroundBaseSize;
    NBRE_Vector2f mBackgroundBaseSize;
    NBRE_Vector2f mAtlasForegroundTexcorrd[4];
    NBRE_Vector2f mAtlasBackgroundTexcorrd[4];

    uint32 mGlyphId;
    uint32 mFontId;
    NBRE_Vector2i mOffset;
};

typedef shared_ptr<NBRE_Glyph> NBRE_GlyphPtr;
/** @} */
#endif
