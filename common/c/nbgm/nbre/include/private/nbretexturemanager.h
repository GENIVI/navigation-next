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

    @file nbretexturemanager.h
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
#ifndef _NBRE_TEXTURE_MANAGER_H_
#define _NBRE_TEXTURE_MANAGER_H_

#include "nbretexture.h"
#include "nbrecache.h"
#include "nbretextureptr.h"
#include "nbrecontext.h"

class TextureCacheStratge;
/** \addtogroup NBRE_Resource
*  @{
*/
/** \addtogroup NBRE_Shader
*  @{
*/
/** Class for loading & managing textures.
 */


class NBRE_TextureManager
{
public:
    NBRE_TextureManager(NBRE_IRenderPal* renderPal);
    ~NBRE_TextureManager();

public:
    /// Create common textures, should be called after render context prepared
    void Initialize();

    /// Create a general texture by images, if failed or existed return nullptr
    NBRE_TexturePtr CreateTexture(const NBRE_String &name, NBRE_Image** image, uint8 faceCount, uint8 numMipmaps, nb_boolean isUsingMipmap, NBRE_Texture::TextureType texType);

    /// Create a general texture by a texture image, if failed or existed return nullptr
    NBRE_TexturePtr CreateTexture(const NBRE_String &name, NBRE_ITextureImage* textureImage, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap);

    /// Create a texture for render target without name
    NBRE_TexturePtr CreateRenderTexture(const NBRE_ImageInfo& info, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap);

    /// Create a texture for render target, if failed or existed return nullptr
    NBRE_TexturePtr CreateRenderTexture(const NBRE_String &name, const NBRE_ImageInfo& info, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap);

    /// Find a texture by name, if not exist return nullptr
    NBRE_TexturePtr GetTexture(const NBRE_String &name);
    /// Lock the texture in order to avoid be released when refreshing
    PAL_Error LockTexture(const NBRE_String &name);
    /// Unlock the texture
    PAL_Error UnlockTexture(const NBRE_String &name);
    /// Remove all textures
    void RemoveAllTextures();
    /// Get the count of textures
    uint32 GetTextureCount()const { return mTextures.GetSize(); }
    /// Get a default texture
    NBRE_TexturePtr GetDefaultTexture();
    /// Merge textures, release the resource of removed texture
    void Merge(NBRE_TextureManager* manager);

private:
    void CreateDefaultTexture();

private:
    typedef NBRE_Cache<NBRE_String, NBRE_TexturePtr> TexturePtrCache;

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_TextureManager);
    
private:
    NBRE_IRenderPal* mRenderPal;
    TextureCacheStratge* mTextureCacheStratege;
    TexturePtrCache mTextures;
};

/** @} */
/** @} */
#endif
