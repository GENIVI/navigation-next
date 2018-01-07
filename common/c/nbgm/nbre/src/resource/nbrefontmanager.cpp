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
#include "nbrefontmanager.h"
#include "nbretextureatlas.h"

const uint32 BASE_FONT_HEIGHT[] = { 24, 48 };
const uint32 FONT_OUTLINE_WIDTH[] = { 2, 2 };
const float FONT_OUTLINE_COLOR_SCALE[] = { 20.0f, 90.0f };
const uint32 TEXT_LAYOUT_CACHE_SIZE = 1;
const uint32 FONT_ATLAS_WIDTH[] = { 1024, 512 };
const uint32 FONT_ATLAS_HEIGHT[] = { 1024, 1024 };

NBRE_FontManager::NBRE_FontManager(NBRE_Context* context, const NBRE_String& fontFolderPath)
:mContext(context)
,mTextLayoutCache(TEXT_LAYOUT_CACHE_SIZE)
,mPalFont(NULL)
{
    mPalFont = PAL_FontCreateEx(mContext->mPalInstance, fontFolderPath.c_str());
    if(!mPalFont)
    {
        mPalFont = PAL_FontCreate(mContext->mPalInstance);
    }
    for (uint32 i = 0; i  < FONT_ATLAS_LEVELS; ++i)
    {
        mGlyphAtlas[i] = NBRE_NEW NBRE_TextureAtlas(*mContext, NBRE_Vector2i(FONT_ATLAS_WIDTH[i], FONT_ATLAS_HEIGHT[i]));
    }
}

NBRE_FontManager::~NBRE_FontManager()
{
    for (uint32 i = 0; i  < FONT_ATLAS_LEVELS; ++i)
    {
        NBRE_DELETE mGlyphAtlas[i];
    }
    PAL_FontDestroy(mPalFont);
}

NBRE_GlyphPtr
NBRE_FontManager::GetGlyph(const NBRE_FontMaterial& fontMaterial, uint32 fontId, uint32 glyphId)
{
    NBRE_GlyphPtr result;
    uint32 level = GetTextureLevel(fontMaterial);
    uint32 key = (level<<24) | (fontId<<16) | glyphId;
    GlyphCache::iterator it = mGlyphCache.find(key);
    if (it != mGlyphCache.end())
    {
        result = it->second;
    }
    else
    {
        PAL_GlyphBitmap* bitmap = NULL;
        PAL_FontGetGlyphBitmap(mPalFont, BASE_FONT_HEIGHT[level], NULL, fontId, glyphId, &bitmap);
        if (bitmap && bitmap->width > 0 && bitmap->height > 0)
        {
            uint8* dat = new uint8[bitmap->width * bitmap->height];
            for (uint32 yi = 0; yi < bitmap->height; ++yi)
            {
                nsl_memcpy(dat + yi * bitmap->width, bitmap->data + (bitmap->height - 1 - yi) * bitmap->width, bitmap->width);
            }
            NBRE_Image* img = NBRE_NEW NBRE_Image(bitmap->width, bitmap->height, NBRE_PF_A8, dat);
            uint32 slotId = (fontId << 16) | glyphId;
            const NBRE_TextureAtlasSlot* slot = mGlyphAtlas[level]->Add(slotId, img);
            if (img != NULL)
            {
                uint32 outlineWidth = FONT_OUTLINE_WIDTH[level];
                NBRE_Image* outlineImg = NBRE_NEW NBRE_Image(bitmap->width + outlineWidth * 2, bitmap->height + outlineWidth * 2, NBRE_PF_A8);
                outlineImg->Blit(img, NBRE_Vector2i(outlineWidth, outlineWidth));
                outlineImg->Blur(outlineWidth);
                outlineImg->ScaleColor(FONT_OUTLINE_COLOR_SCALE[level]);
                uint32 outlineSlotId = slotId | 0xF0000000;
                const NBRE_TextureAtlasSlot* slotBack = mGlyphAtlas[level]->Add(outlineSlotId, outlineImg);
                if (slotBack != NULL)
                {
                    NBRE_Glyph* g = NBRE_NEW NBRE_Glyph(fontId, glyphId, bitmap->xOffset, bitmap->yOffset
                        ,mGlyphAtlas[level]->GetTexture()->GetWidth(), mGlyphAtlas[level]->GetTexture()->GetHeight()
                        , slot->GetTextureRect(), slotBack->GetTextureRect(), BASE_FONT_HEIGHT[level]);
                    result = NBRE_GlyphPtr(g);
                    mGlyphCache[key] = result;
                }
                else
                {
                    NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_FontManager::GetGlyph(), out of memory, add glyph failed.");
                    nbre_assert(0);
                }
                NBRE_DELETE outlineImg;
            }
            else
            {
                NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_FontManager::GetGlyph(), out of memory, add glyph failed.");
                nbre_assert(0);
            }
            NBRE_DELETE img;
        }
        PAL_FontDestroyGlyphBitmap(mPalFont, bitmap);
    }

    return result;
}

nb_boolean
NBRE_FontManager::GetTextLayout(const NBRE_FontMaterial& fontMaterial, const NBRE_WString& text, float fontHeight, nb_boolean wordWrap, PAL_TEXT_ALIGN align, NBRE_VisualGlyphList& layout)
{
    layout.clear();
    NBRE_FontManager::TextLayoutCacheEntry* entry = mTextLayoutCache.GetEntry(mPalFont, fontMaterial, text, wordWrap, align);
    if (entry != NULL && entry->glyphList.size() > 0)
    {
        layout = entry->glyphList;
        for (uint32 i = 0; i < layout.size(); ++i)
        {
            NBRE_VisualGlyph& vg = layout[i];
            vg.positions.minExtend *= fontHeight;
            vg.positions.maxExtend *= fontHeight;
        }
        return TRUE;
    }
    return FALSE;
}

float
NBRE_FontManager::GetStringWidth(const NBRE_FontMaterial& fontMaterial, float fontHeight, const NBRE_WString& text)
{
    return GetStringWidth(fontMaterial, fontHeight, text, PAL_TA_CENTER);
}

float
NBRE_FontManager::GetStringWidth(const NBRE_FontMaterial& fontMaterial, float fontHeight, const NBRE_WString& text, PAL_TEXT_ALIGN align)
{
    NBRE_FontManager::TextLayoutCacheEntry* entry = mTextLayoutCache.GetEntry(mPalFont, fontMaterial, text, FALSE, align);
    return entry ? entry->strSize.x * fontHeight : 0;
}

NBRE_Vector2f
NBRE_FontManager::GetStringSize(const NBRE_FontMaterial& fontMaterial, float fontHeight, const NBRE_WString& text, nb_boolean wordWrap)
{
    return GetStringSize(fontMaterial, fontHeight, text, wordWrap, PAL_TA_CENTER);
}

NBRE_Vector2f
NBRE_FontManager::GetStringSize(const NBRE_FontMaterial& fontMaterial, float fontHeight, const NBRE_WString& text, nb_boolean wordWrap, PAL_TEXT_ALIGN align)
{
    NBRE_Vector2f result;
    NBRE_FontManager::TextLayoutCacheEntry* entry = mTextLayoutCache.GetEntry(mPalFont, fontMaterial, text, wordWrap, align);
    if (entry)
    {
        result = entry->strSize * fontHeight;
    }
    return result;
}

uint32
NBRE_FontManager::GetTextureLevel(const NBRE_FontMaterial& fontMaterial)
{
    if (fontMaterial.foreColor.a < 1.0f && fontMaterial.maxFontHeightPixels > BASE_FONT_HEIGHT[0])
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

NBRE_FontManager::TextLayoutCache::TextLayoutCache(uint32 maxEntryCount)
:mMaxEntryCount(maxEntryCount)
{
}

NBRE_FontManager::TextLayoutCache::~TextLayoutCache()
{
    for (uint32 i = 0; i < mEntries.size(); ++i)
    {
        NBRE_DELETE mEntries[i];
        mEntries[i] = NULL;
    }
}

NBRE_FontManager::TextLayoutCacheEntry*
NBRE_FontManager::TextLayoutCache::GetEntry(PAL_Font* palFont, const NBRE_FontMaterial& fontMaterial, const NBRE_WString& text, nb_boolean wordWrap, PAL_TEXT_ALIGN align)
{
    uint32 level = NBRE_FontManager::GetTextureLevel(fontMaterial);
    // Try in cache
    int32 num = (int32)mEntries.size();
    for (int32 i = 0; i < num; ++i)
    {
        NBRE_FontManager::TextLayoutCacheEntry* entry = mEntries[i];
        if (entry->style == fontMaterial.font && entry->text == text && entry->wordWrap == wordWrap && level == entry->level && entry->align == align)
        {
            // Found, move to front
            if (i > 0)
            {
                for (int32 j = i; j > 0; --j)
                {
                    mEntries[j] = mEntries[j - 1];
                }
                mEntries[0] = entry;
            }
            return entry;
        }
    }

    // Not in cache, get result from nbpal
    NBRE_FontManager::TextLayoutCacheEntry* result = NBRE_NEW NBRE_FontManager::TextLayoutCacheEntry(fontMaterial.font, text, wordWrap, level, align);
    PAL_GlyphInfo* gs = NULL;
    uint32 cnt = 0;
    PAL_GlyphBoundingBox aabb = {1, 1, -1, -1};

    if (PAL_FontGetComplexTextLayout(palFont, BASE_FONT_HEIGHT[level], fontMaterial.font.c_str(), text.c_str(), wordWrap ? BASE_FONT_HEIGHT[level] * 8: 0xFFFF, align, &gs, &cnt, &aabb) == PAL_Ok)
    {
        if (cnt == 0)
        {
            PAL_FontDestroyGlyphLayout(palFont, gs);
            return FALSE;
        }

        float factor = 1.0f / BASE_FONT_HEIGHT[level];
        float baseLine = (aabb.yMax + 1 - aabb.yMin) * 0.5f;
        result->strSize.x = (aabb.xMax + 1) * factor - aabb.xMin * factor;
        result->strSize.y = (aabb.yMax + 1) * factor - aabb.yMin * factor;

        for (uint32 i = 0; i < cnt; ++i)
        {
            const PAL_GlyphInfo& gi = gs[i];
            NBRE_VisualGlyph subGlyph(gi.fontId, gi.glyphId);
            const PAL_GlyphBoundingBox& box = gi.boundingBox;

            subGlyph.positions.minExtend.x = (float)(box.xMin - aabb.xMin);
            subGlyph.positions.maxExtend.x = (float)(box.xMax - aabb.xMin + 1);
            subGlyph.positions.minExtend.y = (float)(aabb.yMax - box.yMax) - baseLine;
            subGlyph.positions.maxExtend.y = (float)(aabb.yMax - box.yMin + 1) - baseLine;

            subGlyph.positions.minExtend *= factor;
            subGlyph.positions.maxExtend *= factor;

            result->glyphList.push_back(subGlyph);
        }
        PAL_FontDestroyGlyphLayout(palFont, gs);
    }

    // Add to cache
    if (num < (int32)mMaxEntryCount)
    {
        // Not full
        mEntries.push_back(result);
        for (int32 j = num; j > 0; --j)
        {
            mEntries[j] = mEntries[j - 1];
        }
        mEntries[0] = result;
    }
    else
    {
        // Cache full, remove last entry
        NBRE_DELETE mEntries.back();
        for (int32 j = mMaxEntryCount - 1; j > 0; --j)
        {
            mEntries[j] = mEntries[j - 1];
        }
        mEntries[0] = result;
    }

    return result;
}
