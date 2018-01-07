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
#include "nbreglyphatlas.h"
#include "nbrefont.h"
#include "nbrerenderpal.h"

//************************************NBRE_GlyphAtlasSlot*******************************************
NBRE_GlyphAtlasSlot::NBRE_GlyphAtlasSlot(NBRE_TexturePtr texture, const RectList& slotMipmapRects)
:mTexture(texture)
,mSlotMipmapRects(slotMipmapRects)
{
}

NBRE_GlyphAtlasSlot::~NBRE_GlyphAtlasSlot()
{
}

void
NBRE_GlyphAtlasSlot::SetGlyph(NBRE_GlyphPtr glyph)
{
    for (uint32 i = 0; i < mSlotMipmapRects.size(); ++i)
    {
        NBRE_AxisAlignedBox2i& rect = mSlotMipmapRects[i];
        NBRE_Vector2i size(rect.maxExtend.x - rect.minExtend.x + 1, rect.maxExtend.y - rect.minExtend.y + 1);
        
        if (i == 0)
        {
            mTexture->UpdateRegion(rect.minExtend.x, rect.minExtend.y, i, glyph->GetBitmap(0));
        }
        else
        {
            NBRE_Image* scaledImage = glyph->GetBitmap(i)->GetResize(size.x, size.y);
            mTexture->UpdateRegion(rect.minExtend.x, rect.minExtend.y, i, scaledImage);
            NBRE_DELETE scaledImage;
        }
    }

    float tw = static_cast<float>(mTexture->GetWidth());
    float th = static_cast<float>(mTexture->GetHeight());
    NBRE_Vector2i offset = mSlotMipmapRects[0].minExtend;

    // Update foreground texcoord
    NBRE_AxisAlignedBox2i foregroundTextureRect = glyph->GetForegroundTexcoord();
    foregroundTextureRect.minExtend += offset;
    foregroundTextureRect.maxExtend += offset;
    
    float fs0 = foregroundTextureRect.minExtend.x / tw;
    float fs1 = (foregroundTextureRect.maxExtend.x + 1) / tw;
    float ft0 = foregroundTextureRect.minExtend.y / th;
    float ft1 = (foregroundTextureRect.maxExtend.y + 1) / th;
    glyph->mAtlasForegroundTexcorrd[0].x = fs0; glyph->mAtlasForegroundTexcorrd[0].y = ft0;
    glyph->mAtlasForegroundTexcorrd[1].x = fs1; glyph->mAtlasForegroundTexcorrd[1].y = ft0;
    glyph->mAtlasForegroundTexcorrd[2].x = fs1; glyph->mAtlasForegroundTexcorrd[2].y = ft1;
    glyph->mAtlasForegroundTexcorrd[3].x = fs0; glyph->mAtlasForegroundTexcorrd[3].y = ft1;

    // Update background texcoord
    NBRE_AxisAlignedBox2i backgroundTextureRect = glyph->GetBackgroundTexcoord();
    backgroundTextureRect.minExtend += offset;
    backgroundTextureRect.maxExtend += offset;

    float bs0 = backgroundTextureRect.minExtend.x / tw;
    float bs1 = (backgroundTextureRect.maxExtend.x + 1) / tw;
    float bt0 = backgroundTextureRect.minExtend.y / th;
    float bt1 = (backgroundTextureRect.maxExtend.y + 1) / th;
    glyph->mAtlasBackgroundTexcorrd[0].x = bs0; glyph->mAtlasBackgroundTexcorrd[0].y = bt0;
    glyph->mAtlasBackgroundTexcorrd[1].x = bs1; glyph->mAtlasBackgroundTexcorrd[1].y = bt0;
    glyph->mAtlasBackgroundTexcorrd[2].x = bs1; glyph->mAtlasBackgroundTexcorrd[2].y = bt1;
    glyph->mAtlasBackgroundTexcorrd[3].x = bs0; glyph->mAtlasBackgroundTexcorrd[3].y = bt1;

    mChar = glyph->GetCharacter();
}

//*************************************NBRE_GlyphAtlas******************************************
NBRE_GlyphAtlas::NBRE_GlyphAtlas(NBRE_Context& context, const NBRE_Vector2i textureSize, const NBRE_Vector2i slotSize):
    mContext(context),
    mTextureSize(textureSize),
    mSlotSize(slotSize)
{
    nbre_assert(textureSize.x > slotSize.x && textureSize.y > slotSize.y);
    AddTexture();
}

NBRE_GlyphAtlas::~NBRE_GlyphAtlas()
{
    for (NBRE_GlyphAtlasSlotList::iterator i = mSlots.begin();
        i != mSlots.end(); ++i)
    {
        NBRE_DELETE *i;
    }
}

nb_boolean 
NBRE_GlyphAtlas::UpdateGlyph(wchar character, NBRE_GlyphPtr glyph, wchar& removedChar)
{
    NBRE_GlyphAtlasSlotMap::iterator si = mGlyphSlotMap.find(character);
    if (si != mGlyphSlotMap.end())
    {
        return FALSE;
    }
    else
    {
        uint32 index = mActiveSlots.size();
        if (index < mSlots.size())
        {   
            mActiveSlots.push_back(index);
            NBRE_GlyphAtlasSlot* slot = mSlots[index];
            slot->SetGlyph(glyph);
            mGlyphSlotMap[character] = slot;
            return FALSE;
        }
        else
        {
            index = mActiveSlots.back();
            NBRE_GlyphAtlasSlot* slot = mSlots[index];
            removedChar = slot->GetChar();
            slot->SetGlyph(glyph);
            SlotIndexList::iterator it = mActiveSlots.end();
            --it;
            mActiveSlots.erase(it);
            mActiveSlots.push_front(index);
            mGlyphSlotMap.erase(removedChar);
            mGlyphSlotMap[character] = slot;
            return TRUE;
        }
    }
}

void 
NBRE_GlyphAtlas::AddTexture()
{
    NBRE_ImageInfo imgInfo(mTextureSize.x, mTextureSize.y, NBRE_PF_A8);
    NBRE_TexturePtr texture(mContext.mRenderPal->CreateTexture(imgInfo, NBRE_Texture::TT_2D, TRUE, "NBRE_GlyphAtlas"));
    mTexture = texture;
    mTexture->Load();

    uint32 xCells = mTextureSize.x / mSlotSize.x;
    uint32 yCells = mTextureSize.y / mSlotSize.y;

    uint32 mipLevels = 0;
    uint32 l = NBRE_Math::Max<uint32>(imgInfo.mWidth, imgInfo.mHeight);
    while (l > 0)
    {
        ++mipLevels;
        l /= 2;
    }
    
    for (uint32 j = 0; j < yCells; ++j)
    {
        for (uint32 i = 0; i < xCells; ++i)
        {
            NBRE_GlyphAtlasSlot::RectList rects;
            uint32 w = imgInfo.mWidth;
            uint32 h = imgInfo.mHeight;
            for (uint32 level = 0; level < mipLevels; ++ level)
            {
                NBRE_AxisAlignedBox2i rc(w * i / xCells, h * j / yCells, w * (i + 1) / xCells - 1, h * (j + 1) / yCells - 1);
                if (rc.IsNull())
                {
                    rc.maxExtend = rc.minExtend;
                }
                rects.push_back(rc);
                w /= 2;
                h /= 2;
            }
            NBRE_GlyphAtlasSlot* slot = NBRE_NEW NBRE_GlyphAtlasSlot(texture, rects);
            mSlots.push_back(slot);
        }
    }
}
//************************************NBRE_AsciiGlyphAtlasSlot*******************************************
NBRE_AsciiGlyphAtlasSlot::NBRE_AsciiGlyphAtlasSlot(const RectList& slotMipmapRects)
:mSlotMipmapRects(slotMipmapRects), mChar(L'A')
{
}

NBRE_AsciiGlyphAtlasSlot::~NBRE_AsciiGlyphAtlasSlot()
{
}

void
NBRE_AsciiGlyphAtlasSlot::SetGlyph(NBRE_GlyphAtlasImageList& imgList, NBRE_GlyphPtr glyph)
{
    for (uint32 i = 0; i < mSlotMipmapRects.size(); ++i)
    {
        NBRE_AxisAlignedBox2i& rect = mSlotMipmapRects[i];
        NBRE_Vector2i size(rect.maxExtend.x - rect.minExtend.x + 1, rect.maxExtend.y - rect.minExtend.y + 1);
        NBRE_Image* img = imgList[i];
        if (i == 0)
        {
            img->Blit(glyph->GetBitmap(0), rect.minExtend);
        }
        else
        {
            NBRE_Image* scaledImage = glyph->GetBitmap(i)->GetResize(size.x, size.y);
            img->Blit(scaledImage, rect.minExtend);
            NBRE_DELETE scaledImage;
        }
    }

    float tw = static_cast<float>(imgList[0]->Width());
    float th = static_cast<float>(imgList[0]->Height());
    NBRE_Vector2i offset = mSlotMipmapRects[0].minExtend;

    // Update foreground texcoord
    NBRE_AxisAlignedBox2i foregroundTextureRect = glyph->GetForegroundTexcoord();
    foregroundTextureRect.minExtend += offset;
    foregroundTextureRect.maxExtend += offset;
    
    float fs0 = foregroundTextureRect.minExtend.x / tw;
    float fs1 = (foregroundTextureRect.maxExtend.x + 1) / tw;
    float ft0 = foregroundTextureRect.minExtend.y / th;
    float ft1 = (foregroundTextureRect.maxExtend.y + 1) / th;
    glyph->mAtlasForegroundTexcorrd[0].x = fs0; glyph->mAtlasForegroundTexcorrd[0].y = ft0;
    glyph->mAtlasForegroundTexcorrd[1].x = fs1; glyph->mAtlasForegroundTexcorrd[1].y = ft0;
    glyph->mAtlasForegroundTexcorrd[2].x = fs1; glyph->mAtlasForegroundTexcorrd[2].y = ft1;
    glyph->mAtlasForegroundTexcorrd[3].x = fs0; glyph->mAtlasForegroundTexcorrd[3].y = ft1;

    // Update background texcoord
    NBRE_AxisAlignedBox2i backgroundTextureRect = glyph->GetBackgroundTexcoord();
    backgroundTextureRect.minExtend += offset;
    backgroundTextureRect.maxExtend += offset;

    float bs0 = backgroundTextureRect.minExtend.x / tw;
    float bs1 = (backgroundTextureRect.maxExtend.x + 1) / tw;
    float bt0 = backgroundTextureRect.minExtend.y / th;
    float bt1 = (backgroundTextureRect.maxExtend.y + 1) / th;
    glyph->mAtlasBackgroundTexcorrd[0].x = bs0; glyph->mAtlasBackgroundTexcorrd[0].y = bt0;
    glyph->mAtlasBackgroundTexcorrd[1].x = bs1; glyph->mAtlasBackgroundTexcorrd[1].y = bt0;
    glyph->mAtlasBackgroundTexcorrd[2].x = bs1; glyph->mAtlasBackgroundTexcorrd[2].y = bt1;
    glyph->mAtlasBackgroundTexcorrd[3].x = bs0; glyph->mAtlasBackgroundTexcorrd[3].y = bt1;

    mChar = glyph->GetCharacter();
}
