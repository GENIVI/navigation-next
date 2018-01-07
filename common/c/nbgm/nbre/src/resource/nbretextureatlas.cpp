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
#include "nbretextureatlas.h"
#include "nbrerenderpal.h"

NBRE_TextureAtlas::NBRE_TextureAtlas(NBRE_Context& context, const NBRE_Vector2i textureSize)
:mContext(context)
,mPackingTree(textureSize, 1)
{
    NBRE_ImageInfo imgInfo;
    imgInfo.mWidth = (uint32)textureSize.x;
    imgInfo.mHeight = (uint32)textureSize.y;
    imgInfo.mFormat = NBRE_PF_A8;
    mTexture = NBRE_TexturePtr(mContext.mRenderPal->CreateTexture(imgInfo, NBRE_Texture::TT_2D, FALSE, "NBRE_GlyphAtlas"));
    mTexture->Load();
}

NBRE_TextureAtlas::~NBRE_TextureAtlas()
{
    for (TextureAtlasSlotList::iterator it = mSlots.begin(); it != mSlots.end(); ++it)
    {
        NBRE_DELETE it->second;
    }
}

const NBRE_TextureAtlasSlot* 
NBRE_TextureAtlas::Add(uint32 index, NBRE_Image* image)
{
    TextureAtlasSlotList::iterator it = mSlots.find(index);
    if (it != mSlots.end())
    {
        return it->second;
    }

    NBRE_Vector2i size(image->Width(), image->Height());
    NBRE_AxisAlignedBox2i resultRect;

    // try to find an empty slot
    NBRE_PackingTreeNode* node = mPackingTree.Insert(size, resultRect);
    if (node)
    {
        NBRE_TextureAtlasSlot* slot = new NBRE_TextureAtlasSlot(node, resultRect);
        mSlots[index] = slot;
        int32 x = resultRect.minExtend.x;
        int32 y = resultRect.minExtend.y;
        uint32 i = 0;
        mTexture->UpdateRegion(x, y, i, image);
        return slot;
    }
    return NULL;
}

const NBRE_TextureAtlasSlot* 
NBRE_TextureAtlas::Add(uint32 index, NBRE_Image** images, uint32 levels)
{
    TextureAtlasSlotList::iterator it = mSlots.find(index);
    if (it != mSlots.end())
    {
        return it->second;
    }

    NBRE_Vector2i size(images[0]->Width(), images[0]->Height());
    NBRE_AxisAlignedBox2i resultRect;

    // try to find an empty slot
    NBRE_PackingTreeNode* node = mPackingTree.Insert(size, resultRect);
    if (node)
    {
        NBRE_TextureAtlasSlot* slot = new NBRE_TextureAtlasSlot(node, resultRect);
        mSlots[index] = slot;
        int dx = resultRect.minExtend.x;
        int dy = resultRect.minExtend.y;
        for (uint32 i = 0;i < levels; ++i)
        {
            mTexture->UpdateRegion(dx, dy, i, images[i]);
            dx /= 2;
            dy /= 2;
        }
        return slot;
    }
    return NULL;
}

void
NBRE_TextureAtlas::Remove(uint32 index)
{
    TextureAtlasSlotList::iterator it = mSlots.find(index);
    if (it != mSlots.end())
    {
        NBRE_TextureAtlasSlot* s = it->second;
        mPackingTree.Remove(s->mNode);
        NBRE_DELETE s;
        // remove slot
        mSlots.erase(it);
    }
}

const NBRE_AxisAlignedBox2i*
NBRE_TextureAtlas::GetTexcoord(uint32 index)
{
    TextureAtlasSlotList::iterator it = mSlots.find(index);
    if (it != mSlots.end())
    {
        return &it->second->mTextureRect;
    }
    return NULL;
}
