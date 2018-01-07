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

@file nbrefontmanager.h
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
#ifndef _NBRE_FONTMANAGER_H
#define _NBRE_FONTMANAGER_H
#include "nbretypes.h"
#include "nbrecontext.h"
#include "nbrefont.h"
#include "nbretextureatlas.h"
#include "nbrefontmaterial.h"
#include "palfont.h"

/** \addtogroup NBRE_Resource
*  @{
*/
#define FONT_ATLAS_LEVELS 2
typedef NBRE_Vector<NBRE_VisualGlyph> NBRE_VisualGlyphList;

/** 
Font manager class.
Use this class to get fonts.
*/
class NBRE_FontManager
{
    struct TextLayoutCacheEntry
    {
        TextLayoutCacheEntry(NBRE_String style, NBRE_WString text, nb_boolean wordWrap, uint32 level, PAL_TEXT_ALIGN align)
            :style(style), text(text), wordWrap(wordWrap), level(level), align(align)
        {
        }

        NBRE_String style;
        NBRE_WString text;
        nb_boolean wordWrap;
        uint32 level;
        PAL_TEXT_ALIGN align;
        NBRE_Vector2f strSize;
        NBRE_VisualGlyphList glyphList;
    };

    class TextLayoutCache
    {
    public:
        TextLayoutCache(uint32 maxEntryCount);
        ~TextLayoutCache();
        TextLayoutCacheEntry* GetEntry(PAL_Font* palFont, const NBRE_FontMaterial& fontMaterial, const NBRE_WString& text, nb_boolean wordWrap, PAL_TEXT_ALIGN align);

    private:
        uint32 mMaxEntryCount;
        NBRE_Vector<TextLayoutCacheEntry*> mEntries;
    };

public:
    NBRE_FontManager(NBRE_Context* context, const NBRE_String& fontFolderPath);
    virtual ~NBRE_FontManager();

public:
    NBRE_GlyphPtr GetGlyph(const NBRE_FontMaterial& fontMaterial, uint32 fontId, uint32 glyphId);
    nb_boolean GetTextLayout(const NBRE_FontMaterial& fontMaterial, const NBRE_WString& text, float fontHeight, nb_boolean wordWrap, PAL_TEXT_ALIGN align, NBRE_VisualGlyphList& layout);
    NBRE_TexturePtr GetTexture(const NBRE_FontMaterial& fontMaterial) const { return mGlyphAtlas[GetTextureLevel(fontMaterial)]->GetTexture(); }
    float GetStringWidth(const NBRE_FontMaterial& fontMaterial, float fontHeight, const NBRE_WString& text);
    NBRE_Vector2f GetStringSize(const NBRE_FontMaterial& fontMaterial, float fontHeight, const NBRE_WString& text, nb_boolean wordWrap);
    float GetStringWidth(const NBRE_FontMaterial& fontMaterial, float fontHeight, const NBRE_WString& text, PAL_TEXT_ALIGN align);
    NBRE_Vector2f GetStringSize(const NBRE_FontMaterial& fontMaterial, float fontHeight, const NBRE_WString& text, nb_boolean wordWrap, PAL_TEXT_ALIGN align);
    static uint32 GetTextureLevel(const NBRE_FontMaterial& fontMaterial);

private:
    NBRE_Context* mContext;
    NBRE_TextureAtlas* mGlyphAtlas[FONT_ATLAS_LEVELS];
    typedef NBRE_Map<uint32, NBRE_GlyphPtr> GlyphCache;
    GlyphCache mGlyphCache;
    TextLayoutCache mTextLayoutCache;
    PAL_Font* mPalFont;
};

#endif  //_NBRE_FONTMANAGER_H
