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

@file nbreglyphttlas.h
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
#ifndef _NBRE_GLYPHATLAS_H
#define _NBRE_GLYPHATLAS_H
#include "nbreglyph.h"
#include "nbreaxisalignedbox2.h"
#include "nbretexture.h"
#include "nbretexturemanager.h"
#include "nbrecontext.h"

class NBRE_Font;
typedef NBRE_Vector<NBRE_TexturePtr> NBRE_TextureList;
typedef NBRE_Set<wchar> NBRE_CharSet;
/** \addtogroup NBRE_Resource
*  @{
*/

/** 
Glyph slot represents a character region in glyph atlas texture.
*/
class NBRE_GlyphAtlasSlot 
{
public:
    typedef NBRE_Vector<NBRE_AxisAlignedBox2i> RectList;
public:
    NBRE_GlyphAtlasSlot(NBRE_TexturePtr texture, const RectList& slotMipmapRects);
    ~NBRE_GlyphAtlasSlot();

public:
    void SetGlyph(NBRE_GlyphPtr glyph);
    wchar GetChar() const { return mChar; }

public:
    NBRE_TexturePtr mTexture;
    RectList mSlotMipmapRects;
    wchar mChar;
};
typedef NBRE_Vector<NBRE_GlyphAtlasSlot*> NBRE_GlyphAtlasSlotList;
typedef NBRE_Map<wchar, NBRE_GlyphAtlasSlot*> NBRE_GlyphAtlasSlotMap;

/** 
Textures stores all characters to be rendered.
Characters are added or removed(LRU) on runtime.
Each character use a slot. Every slot has the same size.
*/
class NBRE_GlyphAtlas
{
    typedef NBRE_List<uint32> SlotIndexList;
public:
    NBRE_GlyphAtlas(NBRE_Context& context, const NBRE_Vector2i textureSize, const NBRE_Vector2i slotSize);
    ~NBRE_GlyphAtlas();

public:
    /// Update glyph slot, return TRUE if removedChar existed
    nb_boolean UpdateGlyph(wchar character, NBRE_GlyphPtr glyph, wchar& removedChar);
    /// Get texture
    NBRE_TexturePtr GetTexture() const { return mTexture; }
    
private:
    void AddTexture();

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_GlyphAtlas);

private:
    NBRE_Context& mContext;
    NBRE_GlyphAtlasSlotMap mGlyphSlotMap;
    NBRE_GlyphAtlasSlotList mSlots;
    SlotIndexList mActiveSlots;
    NBRE_TexturePtr mTexture;
    NBRE_Vector2i mTextureSize;
    NBRE_Vector2i mSlotSize;
};

typedef NBRE_Vector<NBRE_Image*> NBRE_GlyphAtlasImageList;
/** 
Glyph slot represents a character region in glyph atlas texture.
*/
class NBRE_AsciiGlyphAtlasSlot 
{
public:
    typedef NBRE_Vector<NBRE_AxisAlignedBox2i> RectList;
public:
    NBRE_AsciiGlyphAtlasSlot(const RectList& slotMipmapRects);
    ~NBRE_AsciiGlyphAtlasSlot();

public:
    void SetGlyph(NBRE_GlyphAtlasImageList& imgList, NBRE_GlyphPtr glyph);
    wchar GetChar() const { return mChar; }

public:
    RectList mSlotMipmapRects;
    wchar mChar;
};
typedef NBRE_Vector<NBRE_AsciiGlyphAtlasSlot*> NBRE_AsciiGlyphAtlasSlotList;
/** @} */
#endif  //_NBRE_GLYPHATLAS_H
