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

    @file nbred3d9texture.h
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
#ifndef _NBRE_D3D9_TEXTURE_H_
#define _NBRE_D3D9_TEXTURE_H_
#include "nbretexture.h"
#include <d3d9.h>

/*! \addtogroup NBRE_RenderPal
*  @{
*/
/*! \addtogroup NBRE_D3D9RenderPal
*  @{
*/
class NBRE_D3D9Texture: public NBRE_Texture
{
public:
    NBRE_D3D9Texture(NBRE_IRenderPal *renderPal, const NBRE_ImageInfo *info, TextureType type, nb_boolean isUsingMipmap, const NBRE_String& name);
    NBRE_D3D9Texture(NBRE_IRenderPal *renderPal, NBRE_Image** image, uint8 faceCount, uint8 numMipmaps, TextureType type, nb_boolean isUsingMipmap, const NBRE_String&name);
    NBRE_D3D9Texture(NBRE_IRenderPal *renderPal, NBRE_ITextureImage *textureImage, TextureType type, nb_boolean isUsingMipmap, const NBRE_String&name);
    virtual ~NBRE_D3D9Texture();

public:
    IDirect3DBaseTexture9* GetD3DTexture() const { return mD3D9Texture; }
    virtual void UpdateRegion(int32 x, int32 y, int32 level, NBRE_Image* image);

private:
    PAL_Error Create2DTexture(void);
    PAL_Error CreateCubeTexture(void);
    PAL_Error Create2DRenderTexture(void);
    PAL_Error CreateCubeRenderTexture(void);
    virtual PAL_Error GenerateInternalTextureImpl(void);
    virtual void FreeInternalTextureImpl(void);

public:
    IDirect3DBaseTexture9* mD3D9Texture;
};

/** @} */
/** @} */
#endif
