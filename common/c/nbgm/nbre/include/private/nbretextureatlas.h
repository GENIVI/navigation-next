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

@file nbretextureatlas.h
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
#ifndef _NBRE_TEXTURE_ATLAS_H
#define _NBRE_TEXTURE_ATLAS_H
#include "nbreaxisalignedbox2.h"
#include "nbretexture.h"
#include "nbretexturemanager.h"
#include "nbrecontext.h"
#include "nbrepackingtree.h"

class NBRE_TextureAtlasSlot
{
    friend class NBRE_TextureAtlas;
public:
    NBRE_TextureAtlasSlot(NBRE_PackingTreeNode* node, const NBRE_AxisAlignedBox2i& textureRect)
        :mNode(node),mTextureRect(textureRect) {}
    ~NBRE_TextureAtlasSlot() {}

    const NBRE_AxisAlignedBox2i& GetTextureRect() const { return mTextureRect; }
private:
    NBRE_PackingTreeNode* mNode;
    NBRE_AxisAlignedBox2i mTextureRect;
};

typedef std::map<uint32, NBRE_TextureAtlasSlot*> TextureAtlasSlotList;
/** 
Textures stores all characters to be rendered.
Characters are added or removed(LRU) on runtime.
Each character use a slot. Every slot has the same size.
*/
class NBRE_TextureAtlas
{
public:
    NBRE_TextureAtlas(NBRE_Context& context, const NBRE_Vector2i textureSize);
    ~NBRE_TextureAtlas();

public:
    /// Add image
    const NBRE_TextureAtlasSlot* Add(uint32 index, NBRE_Image* image);
    /// Add image with mipmaps
    const NBRE_TextureAtlasSlot* Add(uint32 index, NBRE_Image** images, uint32 levels);
    /// Remove image
    void Remove(uint32 index);
    /// Get texture
    NBRE_TexturePtr GetTexture() const { return mTexture; }
    /// Get slot texcoord
    const NBRE_AxisAlignedBox2i* GetTexcoord(uint32 index);

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_TextureAtlas);

private:
    NBRE_Context& mContext;
    NBRE_PackingTree mPackingTree;
    NBRE_TexturePtr mTexture;
    TextureAtlasSlotList mSlots;
};
/** @} */
#endif  //_NBRE_TEXTURE_ATLAS_H
