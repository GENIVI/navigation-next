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
#include "nbrepacktextureimage.h"
#include "nbrepngcodec.h"
#include "nbreimage.h"
#include "nbrecommon.h"
#include "nbrepackingtree.h"

NBRE_PackTextureImage::NBRE_PackTextureImage(uint32 width, uint32 height, int32 padding)
:mImage(NULL)
,mPackingTree(NULL)
,mSize(static_cast<int32>(width), static_cast<int32>(height))
{
    NBRE_ImageInfo imageInfo(width, height, NBRE_PF_R8G8B8A8);
    mImage = NBRE_NEW NBRE_Image(imageInfo);
    if(mImage == NULL)
    {
        return;
    }

    uint32 size = NBRE_Image::GetImageDataSize(imageInfo);
    uint8* data = mImage->GetImageData();
	nsl_memset(data, 0xFF, sizeof(size));
    for (uint32 i = 0; i < size; i += 4)
    {
		data[i+3] = 0;
    }

    mPackingTree = NBRE_NEW NBRE_PackingTree(mSize, padding);
}

NBRE_PackTextureImage::~NBRE_PackTextureImage()
{
    NBRE_DELETE mPackingTree;
    NBRE_DELETE mImage;
}

const NBRE_Image*
NBRE_PackTextureImage::GetImage(uint8 face, uint8 mipmapLevel) const
{
    nbre_assert((face == 0) && (mipmapLevel == 0));
    return mImage;
}

void
NBRE_PackTextureImage::Release()
{
    NBRE_DELETE this;
}

PAL_Error
NBRE_PackTextureImage::Load()
{
    return PAL_Ok;
}

nb_boolean 
NBRE_PackTextureImage::AddImage(const NBRE_Image* img, NBRE_AxisAlignedBox2f& texcoord)
{
    nbre_assert(img != NULL);
    NBRE_Vector2i size(static_cast<int32>(img->Width()), static_cast<int32>(img->Height()));
    NBRE_AxisAlignedBox2i resultRc;

    if (mPackingTree->Insert(size, resultRc))
    {
        mImage->Blit(img, resultRc.minExtend);
        texcoord.minExtend.x = static_cast<float>(resultRc.minExtend.x + 0.5f) / mSize.x;
        texcoord.minExtend.y = static_cast<float>(resultRc.minExtend.y + 0.5f) / mSize.y;
        texcoord.maxExtend.x = static_cast<float>(resultRc.maxExtend.x + 0.5f) / mSize.x;
        texcoord.maxExtend.y = static_cast<float>(resultRc.maxExtend.y + 0.5f) / mSize.y;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void
NBRE_PackTextureImage::ReadInfo(NBRE_ImageInfo &fm, NBRE_ImageInfo &originalInfo)const
{
    mImage->GetInfo(originalInfo);
    fm = originalInfo;
}

