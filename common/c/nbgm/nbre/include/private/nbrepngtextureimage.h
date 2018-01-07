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

    @file nbrepngtextureimage.h
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
#ifndef _NBRE_PNG_TEXTURE_IMAGE_H_
#define _NBRE_PNG_TEXTURE_IMAGE_H_
#include "nbreitextureimage.h"
#include "nbrecommon.h"
#include "nbrecontext.h"
#include "nbreimage.h"

class NBRE_Image;
class NBRE_IImageCodec;
class NBRE_IOStream;

/** \addtogroup NBRE_Resource
*  @{
*/
/** \addtogroup NBRE_Shader
*  @{
*/

//! A simple png texture image.
/** Using renderpal to generate mipmaps.
*/
class NBRE_PngTextureImage: public NBRE_ITextureImage
{
public:
    NBRE_PngTextureImage(PAL_Instance& pal, NBRE_IOStream* io, uint32 offset, nb_boolean useLowQuality);
    NBRE_PngTextureImage(PAL_Instance& pal, const char* path, uint32 offset, nb_boolean useLowQuality);
    NBRE_PngTextureImage(const NBRE_ImageInfo& imageInfo);

    virtual ~NBRE_PngTextureImage();

public:
    /// Release resource
    virtual void Release();
    /// Get mipmaps count
    virtual uint8 GetMipmapCount() const { return 1; }
    /// Get count of faces
    virtual uint8 GetFaceCount() const { return 1; }
    /// Get a image of mipmap level, only level 0 is supported
    virtual const NBRE_Image* GetImage(uint8 face, uint8 mipmapLevel) const;
    /// Load image from file
    virtual PAL_Error Load();
    /// Read imageInfo
    virtual void ReadInfo(NBRE_ImageInfo &textureInfo, NBRE_ImageInfo &imageInfo)const;
    /// Get image
    NBRE_Image* GetImage() { return mImage; }
private:
    void CreateDecodc() const;

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_PngTextureImage);

protected:
    NBRE_ImageInfo mImageInfo;
    mutable NBRE_IImageCodec* mImageCodec;
    NBRE_Image *mImage;
    PAL_Instance* mPalInstance;
    NBRE_String mPath;
    uint32 mFileOffset;
    mutable NBRE_IOStream* mInStream;
    nb_boolean mUseLowQuality;
    nb_boolean mAutoResize;

};

/** @} */
/** @} */
#endif
