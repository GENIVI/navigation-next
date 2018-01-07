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

    @file nbreimage.h
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
#ifndef _NBRE_IMAGE_H_
#define _NBRE_IMAGE_H_
#include "nbretypes.h"
#include "nbrecommon.h"
#include "nbrevector2.h"

class NBRE_Color;
/** \addtogroup NBRE_Resource
*  @{
*/
/** \addtogroup NBRE_Image
*  @{
*/

enum NBRE_PixelFormat
{
    /// 8-bit pixel format, all bits alpha.
    NBRE_PF_A8 ,
    /// 16-bit pixel format, 5 bits red, 6 bits green, 5 bits blue.
    NBRE_PF_R5G6B5,
    /// 16-bit pixel format, 5 bits red, 6 bits green, 5 bits blue.
    NBRE_PF_B5G6R5,
    /// 16-bit pixel format, 4 bits for alpha, red, green and blue.
    NBRE_PF_A4R4G4B4,
    /// 16-bit pixel format, 5 bits for blue, green, red and 1 for alpha.
    NBRE_PF_A1R5G5B5,
    /// 24-bit pixel format, 8 bits for red, green and blue.
    NBRE_PF_R8G8B8,
    /// 24-bit pixel format, 8 bits for blue, green and red.
    NBRE_PF_B8G8R8,
    /// 32-bit pixel format, 8 bits for alpha, red, green and blue.
    NBRE_PF_A8R8G8B8,
    /// 32-bit pixel format, 8 bits for blue, green, red and alpha.
    NBRE_PF_A8B8G8R8,
    /// 32-bit pixel format, 8 bits for blue, green, red and alpha.
    NBRE_PF_B8G8R8A8,
    /// 32-bit pixel format, 8 bits for red, green, blue and alpha.
    NBRE_PF_R8G8B8A8,
    /// 16-bit pixel format, 4 bits for red, green and blue, alpha.
    NBRE_PF_R4G4B4A4,
    /// 16-bit pixel format, 5 bits for blue, green, red and 1 for alpha.
    NBRE_PF_R5G5B5A1,
    /// Not support format
    NBRE_PF_NOTSUPPORT
};

struct NBRE_ImageInfo
{
    NBRE_ImageInfo()
        :mWidth(1), mHeight(1), mFormat(NBRE_PF_A8)
    {
    }
    NBRE_ImageInfo(uint32 width, uint32 height, NBRE_PixelFormat format)
        :mWidth(width), mHeight(height), mFormat(format)
    {
    }
    /// The width of the image in pixels
    uint32 mWidth;
    /// The height of the image in pixels
    uint32 mHeight;
    /// The pixel format of the image
    NBRE_PixelFormat mFormat;
};

class NBRE_Image
{
public:
    NBRE_Image(const NBRE_ImageInfo& info);
    NBRE_Image(uint32 width, uint32 height, NBRE_PixelFormat format);
    NBRE_Image(uint32 width, uint32 height, NBRE_PixelFormat format, uint8* imageData);
    ~NBRE_Image();

public:
    void GetInfo(NBRE_ImageInfo & info)const { info = mInfo; }
    uint32 Width() const                     { return mInfo.mWidth; }
    uint32 Height() const                    { return mInfo.mHeight; }
    NBRE_PixelFormat Format() const          { return mInfo.mFormat; }
    uint8* GetImageData()                    { return mImageData; }
    const uint8* GetImageData() const        { return mImageData; }
    NBRE_Image* GetResize(uint32 widthNew, uint32 heightNew) const;
    void Blit(const NBRE_Image* img, const NBRE_Vector2i& offset);
    void SetPixel(int32 x, int32 y, const NBRE_Color& c);
    void SetPixel(int32 offset, const NBRE_Color& c);
    NBRE_Image* ConvertToFormat(NBRE_PixelFormat newFormat) const;
    void Resize(uint32 widthNew, uint32 heightNew);
    void FlipY();
    void Extend(uint32 widthNew, uint32 heightNew);
    NBRE_Image* getExtendImage(uint32 widthNew, uint32 heightNew)const;
    void Blur(uint32 radius);
    void ScaleColor(float scale);

    static uint32 GetImageDataSize(const NBRE_ImageInfo& imageInfo);
    static uint32 GetPixelSize(const NBRE_ImageInfo& imageInfo);

private:
    void CreateData();

private:
    /// Image Info
    NBRE_ImageInfo mInfo;
    /// Raw image data
    uint8* mImageData;
};

typedef NBRE_Vector<NBRE_Image*> NBRE_ImageList;

/** @} */
/** @} */
#endif
