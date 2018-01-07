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
#include "nbreglyph.h"

NBRE_Glyph::NBRE_Glyph(wchar character, const NBRE_AxisAlignedBox2i& foreTexcoord, const NBRE_AxisAlignedBox2i& backTexcoord, const NBRE_ImageList& bitmaps, int32 baseFontHeight)
:mCharacter(character)
,mLocalForegroundTexcoord(foreTexcoord)
,mLocalBackgroundTexcoord(backTexcoord)
,mBitmaps(bitmaps)
,mBaseFontHeight(baseFontHeight)
{
    mForegroundBaseSize.y = static_cast<float>(mLocalForegroundTexcoord.maxExtend.y + 1 - mLocalForegroundTexcoord.minExtend.y) / baseFontHeight;
    mForegroundBaseSize.x = mForegroundBaseSize.y * static_cast<float>(mLocalForegroundTexcoord.maxExtend.x + 1 - mLocalForegroundTexcoord.minExtend.x)
        / (mLocalForegroundTexcoord.maxExtend.y + 1 - mLocalForegroundTexcoord.minExtend.y);

    mBackgroundBaseSize.y = static_cast<float>(mLocalBackgroundTexcoord.maxExtend.y + 1 - mLocalBackgroundTexcoord.minExtend.y) / baseFontHeight;
    mBackgroundBaseSize.x = mBackgroundBaseSize.y * static_cast<float>(mLocalBackgroundTexcoord.maxExtend.x + 1 - mLocalBackgroundTexcoord.minExtend.x)
        / (mLocalBackgroundTexcoord.maxExtend.y + 1 - mLocalBackgroundTexcoord.minExtend.y);
}

NBRE_Glyph::NBRE_Glyph(uint32 fontId, uint32 glyphId, int32 xOffset, int32 yOffset, uint32 atlasWidth, uint32 atlasHeight
                       , const NBRE_AxisAlignedBox2i& foreTexcoord, const NBRE_AxisAlignedBox2i& backTexcoord, int32 baseFontHeight)
:mCharacter(0)
,mLocalForegroundTexcoord(foreTexcoord)
,mLocalBackgroundTexcoord(backTexcoord)
,mBaseFontHeight(baseFontHeight)
,mGlyphId(glyphId)
,mFontId(fontId)
,mOffset(xOffset, yOffset)
{
    mForegroundBaseSize.y = static_cast<float>(mLocalForegroundTexcoord.maxExtend.y + 1 - mLocalForegroundTexcoord.minExtend.y) / baseFontHeight;
    mForegroundBaseSize.x = mForegroundBaseSize.y * static_cast<float>(mLocalForegroundTexcoord.maxExtend.x + 1 - mLocalForegroundTexcoord.minExtend.x)
        / (mLocalForegroundTexcoord.maxExtend.y + 1 - mLocalForegroundTexcoord.minExtend.y);

    mBackgroundBaseSize.y = static_cast<float>(mLocalBackgroundTexcoord.maxExtend.y + 1 - mLocalBackgroundTexcoord.minExtend.y) / baseFontHeight;
    mBackgroundBaseSize.x = mBackgroundBaseSize.y * static_cast<float>(mLocalBackgroundTexcoord.maxExtend.x + 1 - mLocalBackgroundTexcoord.minExtend.x)
        / (mLocalBackgroundTexcoord.maxExtend.y + 1 - mLocalBackgroundTexcoord.minExtend.y);

    float tw = (float)atlasWidth;
    float th = (float)atlasHeight;

    // Update foreground texcoord
    const NBRE_AxisAlignedBox2i& foregroundTextureRect = mLocalForegroundTexcoord;
    float fs0 = foregroundTextureRect.minExtend.x / tw;
    float fs1 = (foregroundTextureRect.maxExtend.x + 1) / tw;
    float ft0 = foregroundTextureRect.minExtend.y / th;
    float ft1 = (foregroundTextureRect.maxExtend.y + 1) / th;
    mAtlasForegroundTexcorrd[0].x = fs0; mAtlasForegroundTexcorrd[0].y = ft0;
    mAtlasForegroundTexcorrd[1].x = fs1; mAtlasForegroundTexcorrd[1].y = ft0;
    mAtlasForegroundTexcorrd[2].x = fs1; mAtlasForegroundTexcorrd[2].y = ft1;
    mAtlasForegroundTexcorrd[3].x = fs0; mAtlasForegroundTexcorrd[3].y = ft1;

    // Update background texcoord
    const NBRE_AxisAlignedBox2i& backgroundTextureRect = mLocalBackgroundTexcoord;
    float bs0 = backgroundTextureRect.minExtend.x / tw;
    float bs1 = (backgroundTextureRect.maxExtend.x + 1) / tw;
    float bt0 = backgroundTextureRect.minExtend.y / th;
    float bt1 = (backgroundTextureRect.maxExtend.y + 1) / th;
    mAtlasBackgroundTexcorrd[0].x = bs0; mAtlasBackgroundTexcorrd[0].y = bt0;
    mAtlasBackgroundTexcorrd[1].x = bs1; mAtlasBackgroundTexcorrd[1].y = bt0;
    mAtlasBackgroundTexcorrd[2].x = bs1; mAtlasBackgroundTexcorrd[2].y = bt1;
    mAtlasBackgroundTexcorrd[3].x = bs0; mAtlasBackgroundTexcorrd[3].y = bt1;
}


NBRE_Glyph::~NBRE_Glyph()
{
    for (NBRE_ImageList::iterator i = mBitmaps.begin(); i != mBitmaps.end(); ++i)
    {
        NBRE_DELETE *i;
    }
}

NBRE_Image* 
NBRE_Glyph::GetBitmap(uint32 level) const
{
    if (level >= mBitmaps.size())
    {
        return mBitmaps.back();
    }
    return mBitmaps[level];
}
