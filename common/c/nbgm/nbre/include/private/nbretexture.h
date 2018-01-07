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

    @file nbretexture.h
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
#ifndef _NBRE_TEXTURE_H_
#define _NBRE_TEXTURE_H_
#include "nbreimage.h"
#include "palerror.h"

class NBRE_ITextureImage;
class NBRE_RenderTarget;
class NBRE_IRenderPal;

/** \addtogroup NBRE_Resource
*  @{
*/
/** \addtogroup NBRE_Shader
*  @{
*/
class NBRE_Texture;

class NBRE_ITextureEventListener
{
public:
    virtual void OnDestroyed(NBRE_Texture* texture) = 0;
};

class NBRE_Texture
{
public:

    enum TextureType
    {
        /// 1D texture
        TT_1D = 0,
        /// 2D texture
        TT_2D,
        /// 3D volume texture
        TT_3D,
        /// 3D cube map
        TT_CUBE_MAP,
        /// 2D render texture
        TT_2D_RENDERTARGET,
        /// cube map render texture
        TT_CUBE_MAP_RENDERTARTE
    };

    enum TextureState
    {
        /// Not loaded
        TS_UNLOADED = 0,
        /// Loaded
        TS_LOADED
    };

public:
    NBRE_Texture(NBRE_IRenderPal *renderPal, NBRE_ITextureImage *texImage, TextureType type, const NBRE_ImageInfo *info, nb_boolean isUsingMipmap, const NBRE_String& name);
    virtual ~NBRE_Texture();

public:
    /// Load into video memory
    PAL_Error Load();
    /// Unload from video memory
    void Unload();
    const NBRE_String& GetName()const           { return mName; }
    TextureState GetState(void) const           { return mLoadingState; }
    TextureType GetType()const                  { return mType; }
    nb_boolean IsUsingMipmap()const             { return mUsingMipmap; }
    const NBRE_ImageInfo& GetTextureInfo()const { return mTextureInfo; }
    uint32 GetWidth()const                      { return mTextureInfo.mWidth; }
    uint32 GetHeight()const                     { return mTextureInfo.mHeight; }
    uint32 GetOriginalWidth()const              { return mOriginalInfo.mWidth; }
    uint32 GetOriginalHeight()const             { return mOriginalInfo.mHeight; }

    void SetEventListener(NBRE_ITextureEventListener* listener);

    virtual void UpdateRegion(int32 x, int32 y, int32 level, NBRE_Image* image) = 0;
protected:
    virtual PAL_Error GenerateInternalTextureImpl(void) = 0;
    virtual void FreeInternalTextureImpl(void) = 0;

protected:
    /// RenderPal
    NBRE_IRenderPal* mRenderPal;
    /// Texture image
    NBRE_ITextureImage* mTextureImage;
    /// Texture type
    TextureType mType;
    /// Texture usage
    int32 mUsage;
    /// Texture loading state
    TextureState mLoadingState;
    /// Desired image information
    NBRE_ImageInfo mTextureInfo;
    /// Original image information
    NBRE_ImageInfo mOriginalInfo;
    /// if using mipmap
    nb_boolean mUsingMipmap;
    /// Optional name of the texture
    NBRE_String mName;

    NBRE_ITextureEventListener* mTextureListener;
};


/** @} */
/** @} */
#endif
