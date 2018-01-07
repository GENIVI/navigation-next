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
#include "nbreimage.h"
#include "nbremath.h"
#include "nbreaxisalignedbox2.h"
#include "nbrecolor.h"
#include "nbrelog.h"

static void
ResizeGreyscale(const uint8* image, int32 width, int32 height, uint8* imageNew, int32 widthNew, int32 heightNew)
{
    int32 i = 0;
    int32 j = 0;
    int32 k = 0;
    
    nbre_assert(widthNew > 0);
    nbre_assert(height > 0);
    
    uint8* temp = NBRE_NEW uint8[widthNew * height];
    for (j = 0; j < height; ++j)
    {
        for (i = 0; i < widthNew; ++i)
        {
            if (widthNew >= width)
            {
                k = NBRE_Math::Clamp<int32>((int32)((i + 0.5f) * width / widthNew), 0, width - 1);
                uint8* pDst = temp + (j * widthNew + i);
                const uint8* pSrc = image + (width * j + k);
                pDst[0] = pSrc[0];
            }
            else
            {
                uint32 total = 0;
                uint32 value = 0;
                for (k = NBRE_Math::Max<int32>(0, (int32)((i - 0.5f) * width / widthNew)); k <= NBRE_Math::Min<int32>(width - 1,(int32)((i + 0.5f) * width / widthNew)); ++k)
                {
                    value += image[width * j + k];
                    ++total;
                }

                temp[j * widthNew + i] = (uint8)(total == 0 ? 0 : value / total);
            }
        }
    }

    for (i = 0; i < widthNew; ++i)
    {
        for(j = 0; j < heightNew; ++j)
        {
            if (heightNew >= height)
            {
                k = NBRE_Math::Clamp<int32>((int32)((j + 0.5f) * height / heightNew), 0, height - 1);
                uint8* pDst = imageNew + (j * widthNew + i);
                const uint8* pSrc = temp + (k * widthNew + i);

                pDst[0] = pSrc[0];
            }
            else
            {
                uint32 total = 0;
                uint32 value = 0;
                for (k = NBRE_Math::Max<int32>(0, (int32)((j - 0.5f) * height / heightNew)); k < NBRE_Math::Min<int32>(height - 1, (int32)((j + 0.5f) * height / heightNew)); ++k)
                {
                    value += temp[k * widthNew + i];
                    ++total;
                }
                imageNew[j * widthNew + i] = (uint8)(total == 0 ? 0 : value / total);
            }
        }
    }
    NBRE_DELETE_ARRAY temp;
}

static void
ResizeRGBA(const uint8* image, int32 width, int32 height, uint8* imageNew, int32 widthNew, int32 heightNew)
{
    int32 i = 0;
    int32 j = 0;
    int32 k = 0;
    
    nbre_assert(widthNew > 0);
    nbre_assert(height > 0);
    
    uint8* temp = NBRE_NEW uint8[widthNew * height * 4];
    for (j = 0; j < height; ++j)
    {
        for (i = 0; i < widthNew; ++i)
        {
            if (widthNew >= width)
            {
                k = NBRE_Math::Clamp<int32>((int32)((i + 0.5f) * width / widthNew), 0, width - 1);
                uint8* pDst = temp + (j * widthNew + i) * 4;
                const uint8* pSrc = image + (width * j + k) * 4;

                pDst[0] = pSrc[0];
                pDst[1] = pSrc[1];
                pDst[2] = pSrc[2];
                pDst[3] = pSrc[3];
            }
            else
            {
                uint32 total = 0;
                uint32 value[4] = {0, 0, 0, 0};

                for (k = NBRE_Math::Max<int32>(0, (int32)((i - 0.5f) * width / widthNew)); k <= NBRE_Math::Min<int32>(width - 1,(int32)((i + 0.5f) * width / widthNew)); ++k)
                {
                    const uint8* pImage = image + (width * j + k) * 4;
                    value[0] += pImage[0];
                    value[1] += pImage[1];
                    value[2] += pImage[2];
                    value[3] += pImage[3];
                    ++total;
                }

                uint8* pTemp = temp + (j * widthNew + i) * 4;
                if (total == 0)
                {
                    pTemp[0] = 0;
                    pTemp[1] = 0;
                    pTemp[2] = 0;
                    pTemp[3] = 0;
                }
                else
                {
                    pTemp[0] = static_cast<uint8>(value[0] / total);
                    pTemp[1] = static_cast<uint8>(value[1] / total);
                    pTemp[2] = static_cast<uint8>(value[2] / total);
                    pTemp[3] = static_cast<uint8>(value[3] / total);
                }
            }
        }
    }

    for (i = 0; i < widthNew; ++i)
    {
        for(j = 0; j < heightNew; ++j)
        {
            if (heightNew >= height)
            {
                k = NBRE_Math::Clamp<int32>((int32)((j + 0.5f) * height / heightNew), 0, height - 1);
                uint8* pDst = imageNew + (j * widthNew + i) * 4;
                const uint8* pSrc = temp + (k * widthNew + i) * 4;

                pDst[0] = pSrc[0];
                pDst[1] = pSrc[1];
                pDst[2] = pSrc[2];
                pDst[3] = pSrc[3];
            }
            else
            {
                uint32 total = 0;
                uint32 value[4] = {0, 0, 0, 0};

                for (k = NBRE_Math::Max<int32>(0, (int32)((j - 0.5f) * height / heightNew)); k < NBRE_Math::Min<int32>(height - 1, (int32)((j + 0.5f) * height / heightNew)); ++k)
                {
                    uint8* pImage = temp + (k * widthNew + i) * 4;
                    value[0] += pImage[0];
                    value[1] += pImage[1];
                    value[2] += pImage[2];
                    value[3] += pImage[3];
                    ++total;
                }

                uint8* pTemp = imageNew + (j * widthNew + i) * 4;
                if (total == 0)
                {
                    pTemp[0] = 0;
                    pTemp[1] = 0;
                    pTemp[2] = 0;
                    pTemp[3] = 0;
                }
                else
                {
                    pTemp[0] = static_cast<uint8>(value[0] / total);
                    pTemp[1] = static_cast<uint8>(value[1] / total);
                    pTemp[2] = static_cast<uint8>(value[2] / total);
                    pTemp[3] = static_cast<uint8>(value[3] / total);
                }
            }
        }
    }
    NBRE_DELETE_ARRAY temp;
}

static void
ResizeRGB(const uint8* image, int32 width, int32 height, uint8* imageNew, int32 widthNew, int32 heightNew)
{
    int32 i = 0;
    int32 j = 0;
    int32 k = 0;
    
    nbre_assert(widthNew > 0);
    nbre_assert(height > 0);
    
    uint8* temp = NBRE_NEW uint8[widthNew * height * 3];
    for (j = 0; j < height; ++j)
    {
        for (i = 0; i < widthNew; ++i)
        {
            if (widthNew >= width)
            {
                k = NBRE_Math::Clamp<int32>((int32)((i + 0.5f) * width / widthNew), 0, width - 1);
                uint8* pDst = temp + (j * widthNew + i) * 3;
                const uint8* pSrc = image + (width * j + k) * 3;

                pDst[0] = pSrc[0];
                pDst[1] = pSrc[1];
                pDst[2] = pSrc[2];
            }
            else
            {
                uint32 total = 0;
                uint32 value[] = {0, 0, 0};

                for (k = NBRE_Math::Max<int32>(0, (int32)((i - 0.5f) * width / widthNew)); k <= NBRE_Math::Min<int32>(width - 1,(int32)((i + 0.5f) * width / widthNew)); ++k)
                {
                    const uint8* pImage = image + (width * j + k) * 3;
                    value[0] += pImage[0];
                    value[1] += pImage[1];
                    value[2] += pImage[2];
                    ++total;
                }

                uint8* pTemp = temp + (j * widthNew + i) * 3;
                if (total == 0)
                {
                    pTemp[0] = 0;
                    pTemp[1] = 0;
                    pTemp[2] = 0;
                }
                else
                {
                    pTemp[0] = static_cast<uint8>(value[0] / total);
                    pTemp[1] = static_cast<uint8>(value[1] / total);
                    pTemp[2] = static_cast<uint8>(value[2] / total);
                }
            }
        }
    }

    for (i = 0; i < widthNew; ++i)
    {
        for(j = 0; j < heightNew; ++j)
        {
            if (heightNew >= height)
            {
                k = NBRE_Math::Clamp<int32>((int32)((j + 0.5f) * height / heightNew), 0, height - 1);
                uint8* pDst = imageNew + (j * widthNew + i) * 3;
                const uint8* pSrc = temp + (k * widthNew + i) * 3;

                pDst[0] = pSrc[0];
                pDst[1] = pSrc[1];
                pDst[2] = pSrc[2];
            }
            else
            {
                uint32 total = 0;
                uint32 value[] = {0, 0, 0};

                for (k = NBRE_Math::Max<int32>(0, (int32)((j - 0.5f) * height / heightNew)); k < NBRE_Math::Min<int32>(height - 1, (int32)((j + 0.5f) * height / heightNew)); ++k)
                {
                    uint8* pImage = temp + (k * widthNew + i) * 3;
                    value[0] += pImage[0];
                    value[1] += pImage[1];
                    value[2] += pImage[2];
                    ++total;
                }

                uint8* pTemp = imageNew + (j * widthNew + i) * 3;
                if (total == 0)
                {
                    pTemp[0] = 0;
                    pTemp[1] = 0;
                    pTemp[2] = 0;
                }
                else
                {
                    pTemp[0] = static_cast<uint8>(value[0] / total);
                    pTemp[1] = static_cast<uint8>(value[1] / total);
                    pTemp[2] = static_cast<uint8>(value[2] / total);
                }
            }
        }
    }
    NBRE_DELETE_ARRAY temp;
}

static NBRE_Image* 
ConvertRGBA8ToRGBA4(const NBRE_Image* img)
{
    uint32 w = img->Width();
    uint32 h = img->Height();
    uint32 numPixels = w * h;

    nbre_assert(numPixels > 0);
    
    NBRE_Image* result = NBRE_NEW NBRE_Image(w, h, NBRE_PF_R4G4B4A4);
    if(result == NULL)
    {
        return NULL;
    }
    uint8* bits = result->GetImageData();
    const uint8* data = img->GetImageData();

    const uint8* pSrc = data;
    uint8* pDst = bits;

    for (uint32 i = 0; i < numPixels; ++i)
    {
        *pDst++ = (pSrc[3]>>4) | (pSrc[2] & 0xF0);
        *pDst++ = (pSrc[1]>>4) | (pSrc[0] & 0xF0);
        pSrc += 4;
    }

    return result;
}

static NBRE_Image* 
ConvertRGB8ToR5G6B5(const NBRE_Image* img)
{
    uint32 w = img->Width();
    uint32 h = img->Height();
    uint32 numPixels = w * h;
    
    nbre_assert(numPixels > 0);
    
    NBRE_Image* result = NBRE_NEW NBRE_Image(w, h, NBRE_PF_R5G6B5);
    if(result == NULL)
    {
        return NULL;
    }
    uint8* bits = result->GetImageData();
    const uint8* data = img->GetImageData();

    const uint8* pSrc = data;
    uint16* pDst = (uint16*)bits;

    for (uint32 i = 0; i < numPixels; ++i)
    {
        uint16 r = (pSrc[0] + 4) * 31 / 255;
        uint16 g = (pSrc[1] + 2) * 63 / 255;
        uint16 b = (pSrc[2] + 4) * 31 / 255;
        *pDst++ = (r << 11) | (g << 5) |(b);
        pSrc += 3;
    }

    return result;
}


static float 
ImageBlurFactor(int32 i, float radius)
{
    if (i == 0)
    {
        return 1.0f;
    }
    else
    {
        float low = i - 0.5f;
        if (low > radius + 0.5f)
        {
            return 0;
        }
        else
        {
            float high = MIN(i + 0.5f, radius + 0.5f);
            return ((1.0f - (low - 0.5f) / radius) + (1.0f - (high - 0.5f) / radius)) * 0.5f * (high - low);
        }
    }
}

static void
ImageBlur(uint8* image, int32 width, int32 height, float radius)
{
    int32 i;
    int32 j;
    int32 r;
    float total;
    float component;
    float factor;
    uint8* result = NBRE_NEW uint8[width * height];

    // Do x axis blur on image first, save result to result
    for (i = 0; i < width; ++i)
    {
        for (j = 0; j < height; ++j)
        {
            factor = ImageBlurFactor(0, radius);
            total = (float)image[i + j * width] * factor;
            component = factor;

            for (r = 1; r <= (int32)ceilf(radius); ++r)
            {
                factor = ImageBlurFactor(r, radius);
                if (i - r >= 0)
                {                    
                    total += image[i - r + j * width] * factor;
                    component += factor;
                }

                if (i + r < width)
                {
                    total += image[i + r + j * width] * factor;
                    component += factor;
                }
            }

            result[i + j * width] = (uint8)(total / component);
        }
    }

    // Do y axis blur on result, save result to image
    for (i = 0; i < width; ++i)
    {
        for (j = 0; j < height; ++j)
        {
            factor = ImageBlurFactor(0, radius);
            total = result[i + j * width] * factor;
            component = factor;

            for (r = 1; r <= (int32)ceilf(radius); ++r)
            {
                factor = ImageBlurFactor(r, radius);
                if (j - r >= 0)
                {
                    total += result[i + (j - r) * width] * factor;
                    component += factor;
                }

                if (j + r < height)
                {
                    total += result[i + (j + r) * width] * factor;
                    component += factor;
                }
            }

            image[i + j * width] = (uint8)(total / component);
        }
    }

    NBRE_DELETE_ARRAY result;
}


NBRE_Image::NBRE_Image(uint32 width, uint32 height, NBRE_PixelFormat format)
    :mInfo(width, height, format),
    mImageData(NULL)
{
    CreateData();
}

NBRE_Image::NBRE_Image(const NBRE_ImageInfo& info)
    :mInfo(info),
    mImageData(NULL)
{
    CreateData();
}

NBRE_Image::NBRE_Image(uint32 width, uint32 height, NBRE_PixelFormat format, uint8* imageData)
    :mInfo(width, height, format),
    mImageData(imageData)
{
    nbre_assert(mImageData != NULL);
}

NBRE_Image::~NBRE_Image()
{
    if (mImageData != NULL)
    {
        NBRE_DELETE_ARRAY mImageData;
    }
}

void
NBRE_Image::CreateData()
{
    uint32 dataSize = GetImageDataSize(mInfo);
    nbre_assert(dataSize!=0);
    mImageData = NBRE_NEW uint8[dataSize];
    if(mImageData!= 0)
    {
        nsl_memset(mImageData, 0, dataSize);
    }
}


void
NBRE_Image::FlipY()
{
#define ROW(line) (uint8*)(mImageData + (line) * columnPixelSize)
    uint32 columnPixelSize = NBRE_Image::GetPixelSize(mInfo) * mInfo.mWidth;
    uint8* rowData = NBRE_NEW uint8[columnPixelSize]; 
    for (uint32 i = 0;  i < mInfo.mHeight/2;  ++i)
    {
        nsl_memcpy(rowData,                    ROW(i),                      columnPixelSize);
        nsl_memcpy(ROW(i),                     ROW(mInfo.mHeight - i -1),   columnPixelSize);
        nsl_memcpy(ROW(mInfo.mHeight - i -1),  rowData,                     columnPixelSize);
    }
    NBRE_DELETE[] rowData;
#undef ROW
}

void
NBRE_Image::Resize(uint32 widthNew, uint32 heightNew)
{
    nbre_assert(widthNew > 0 && heightNew > 0);
    uint8* newData = NULL;

    switch(mInfo.mFormat)
    {
    case NBRE_PF_A8:
        {
            newData = NBRE_NEW uint8[widthNew * heightNew];
            ResizeGreyscale(mImageData, mInfo.mWidth, mInfo.mHeight, newData, widthNew, heightNew);
        }
        break;
    case NBRE_PF_R8G8B8A8:
        {
            newData = NBRE_NEW uint8[widthNew * heightNew * 4];
            ResizeRGBA(mImageData, mInfo.mWidth, mInfo.mHeight, newData, widthNew, heightNew);
        }
        break;
    case NBRE_PF_R8G8B8:
        {
            newData = NBRE_NEW uint8[widthNew * heightNew * 3];
            ResizeRGB(mImageData, mInfo.mWidth, mInfo.mHeight, newData, widthNew, heightNew);
        }
        break;
    //@TODO: add other format here
    default:
        // Not support format
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_Image::Resize failed, mFormat=%d", mInfo.mFormat);
        nsl_assert(0);
        break;
    }

    NBRE_DELETE_ARRAY mImageData;
    mImageData = newData;
    mInfo.mWidth = widthNew;
    mInfo.mHeight = heightNew;
}


NBRE_Image* 
NBRE_Image::GetResize(uint32 widthNew, uint32 heightNew) const
{
    nbre_assert(widthNew > 0 && heightNew > 0);
    switch(mInfo.mFormat)
    {
    case NBRE_PF_A8:
        {
            NBRE_Image* result = NBRE_NEW NBRE_Image(widthNew, heightNew, NBRE_PF_A8);
            if(result == NULL)
            {
                return NULL;
            }
            ResizeGreyscale(mImageData, mInfo.mWidth, mInfo.mHeight, result->GetImageData(), widthNew, heightNew);
            return result;
        }
        break;
    case NBRE_PF_R8G8B8A8:
        {
            NBRE_Image* result = NBRE_NEW NBRE_Image(widthNew, heightNew, NBRE_PF_R8G8B8A8);
            if(result == NULL)
            {
                return NULL;
            }
            ResizeRGBA(mImageData, mInfo.mWidth, mInfo.mHeight, result->GetImageData(), widthNew, heightNew);
            return result;
        }
        break;
    case NBRE_PF_R8G8B8:
        {
            NBRE_Image* result = NBRE_NEW NBRE_Image(widthNew, heightNew, NBRE_PF_R8G8B8);
            if(result == NULL)
            {
                return NULL;
            }
            ResizeRGB(mImageData, mInfo.mWidth, mInfo.mHeight, result->GetImageData(), widthNew, heightNew);
            return result;
        }
        break;
    //@TODO: add other format here
    default:
        // Not support format
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_Image::GetResize failed, mFormat=%d", mInfo.mFormat);
        nbre_assert(0);
        break;
    }
    return NULL;
}

void 
NBRE_Image::Blit(const NBRE_Image* img, const NBRE_Vector2i& offset)
{
    nbre_assert(img && img->mInfo.mFormat == mInfo.mFormat);

    int32 w = (int32)mInfo.mWidth;
    int32 h = (int32)mInfo.mHeight;
    NBRE_AxisAlignedBox2i rc(
        NBRE_Math::Max<int32>(0, offset.x), 
        NBRE_Math::Max<int32>(0, offset.y),
        NBRE_Math::Min<int32>(w - 1, offset.x + img->Width() - 1),
        NBRE_Math::Min<int32>(h - 1, offset.y + img->Height() - 1)
        );

    int32 pixelSize = 4;
    switch(mInfo.mFormat)
    {
    case NBRE_PF_A8:
        pixelSize = 1;
        break;
    case NBRE_PF_R8G8B8:
        pixelSize = 3;
        break;
    default:
        // @TODO: add other format support here
        break;
    }

    if (!rc.IsNull())
    {
        for (int32 y = rc.minExtend.y; y <= rc.maxExtend.y; ++y)
        {
            uint8* src = img->mImageData + pixelSize * ((y - offset.y) * img->mInfo.mWidth + rc.minExtend.x - offset.x);
            uint8* dst = mImageData + pixelSize * (y * w + rc.minExtend.x);
            nsl_memcpy(dst, src, pixelSize * (rc.maxExtend.x - rc.minExtend.x + 1));
        }
    }
}

void
NBRE_Image::Extend(uint32 widthNew, uint32 heightNew)
{
    if (widthNew == 0 || heightNew == 0)
    {
        return;
    }

    if (widthNew == mInfo.mWidth && heightNew == mInfo.mHeight)
    {
        return;
    }

    NBRE_ImageInfo infoNew(widthNew, heightNew, mInfo.mFormat);
    uint32 pixelSize = GetPixelSize(infoNew);
    uint8* dataNew = NBRE_NEW uint8[pixelSize * widthNew * heightNew];
    nsl_memset(dataNew, 0, pixelSize * widthNew * heightNew);

    uint32 rowSize = pixelSize * (widthNew < mInfo.mWidth ? widthNew : mInfo.mWidth);
    uint32 h = heightNew < mInfo.mHeight ? heightNew : mInfo.mHeight;
    for (uint32 y = 0; y < h; ++y)
    {
        nsl_memcpy(dataNew + pixelSize * widthNew * y, mImageData + pixelSize * mInfo.mWidth * y, rowSize);
    }

    NBRE_DELETE mImageData;
    mInfo = infoNew;
    mImageData = dataNew;
}

void 
NBRE_Image::SetPixel(int32 x, int32 y, const NBRE_Color& c)
{
    switch(mInfo.mFormat)
    {
    case NBRE_PF_R8G8B8:
        {
            uint8 r = static_cast<uint8>(c.r*255);
            uint8 g = static_cast<uint8>(c.g*255);
            uint8 b = static_cast<uint8>(c.b*255);

            int32 index = (y*mInfo.mWidth + x)*3;
            mImageData[index] = r;
            mImageData[index+1] = g;
            mImageData[index+2] = b;
        }
        break;
    case NBRE_PF_R8G8B8A8:
        {
            uint8 r = static_cast<uint8>(c.r*255);
            uint8 g = static_cast<uint8>(c.g*255);
            uint8 b = static_cast<uint8>(c.b*255);
            uint8 a = static_cast<uint8>(c.a*255);

            int32 index = (y*mInfo.mWidth + x)*4;
            mImageData[index] = r;
            mImageData[index+1] = g;
            mImageData[index+2] = b;
            mImageData[index+3] = a;
        }
        break;
    default:
        // @TODO: add other format support here
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_Image::SetPixel failed, x=%d, y=%d, mInfo.mFormat=%d", x, y, mInfo.mFormat);
        nbre_assert(FALSE);
        break;
    }
}

void 
NBRE_Image::SetPixel(int32 offset, const NBRE_Color& c)
{
    SetPixel(offset%mInfo.mWidth, offset/mInfo.mWidth, c);
}

NBRE_Image* 
NBRE_Image::ConvertToFormat(NBRE_PixelFormat newFormat) const
{
    NBRE_Image* result = NULL;
    switch (mInfo.mFormat)
    {
    case NBRE_PF_R8G8B8A8:
        switch (newFormat)
        {
        case NBRE_PF_R4G4B4A4:
            result = ConvertRGBA8ToRGBA4(this);
            break;
        default:
            // @TODO: add other format support here
            NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_Image::ConvertToFormat failed, newFormat=%d", newFormat);
            nbre_assert(0);
            break;
        }
        break;
    case NBRE_PF_R8G8B8:
        switch (newFormat)
        {
        case NBRE_PF_R5G6B5:
            result = ConvertRGB8ToR5G6B5(this);
            break;
        default:
            // @TODO: add other format support here
            NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_Image::ConvertToFormat failed, newFormat=%d", newFormat);
            nbre_assert(0);
            break;
        }
        break;
    default:
        // @TODO: add other format support here
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_Image::ConvertToFormat failed, mFormat=%d", mInfo.mFormat);
        nbre_assert(0);
        break;
    }
    return result;
}

uint32
NBRE_Image::GetPixelSize(const NBRE_ImageInfo& imageInfo)
{
    uint32 pixelSize = 4;
    
    switch (imageInfo.mFormat)
    {
        case NBRE_PF_A8:
            pixelSize = 1;
            break;
        case NBRE_PF_R5G6B5:
        case NBRE_PF_B5G6R5:
        case NBRE_PF_A4R4G4B4:
        case NBRE_PF_A1R5G5B5:
        case NBRE_PF_R4G4B4A4:
        case NBRE_PF_R5G5B5A1:
            pixelSize = 2;
            break;
        case NBRE_PF_R8G8B8:
        case NBRE_PF_B8G8R8:
            pixelSize = 3;
            break;
        case NBRE_PF_A8R8G8B8:
        case NBRE_PF_A8B8G8R8:
        case NBRE_PF_B8G8R8A8:
        case NBRE_PF_R8G8B8A8:
            pixelSize = 4;
            break;
        default:
            nbre_assert(FALSE);
            break;
    }
    return pixelSize;
}

uint32
NBRE_Image::GetImageDataSize(const NBRE_ImageInfo& imageInfo)
{
    return GetPixelSize(imageInfo) * imageInfo.mWidth * imageInfo.mHeight;
}

void
NBRE_Image::Blur(uint32 radius)
{
    if (radius == 0)
    {
        return;
    }

    switch(mInfo.mFormat)
    {
    case NBRE_PF_A8:
        ImageBlur(mImageData, (int32)mInfo.mWidth, (int32)mInfo.mHeight, (float)radius);
        break;
    default:
        nbre_assert(0);
        break;
    }
}

void
NBRE_Image::ScaleColor(float scale)
{
    if (scale < 0)
    {
        return;
    }

    switch(mInfo.mFormat)
    {
    case NBRE_PF_A8:
        {
            uint32 width = mInfo.mWidth;
            uint32 height = mInfo.mHeight;

            for (uint32 i = 0; i < width * height; ++i)
            {
                int32 v = (int32)(mImageData[i] * scale);
                mImageData[i] = v >= 0xFF ? 0xFF : (uint8)v;
            }
        }
        break;
    default:
        nbre_assert(0);
        break;
    }
}

NBRE_Image*
NBRE_Image::getExtendImage(uint32 widthNew, uint32 heightNew)const
{
    if (widthNew == 0 || heightNew == 0)
    {
        return NULL;
    }

    NBRE_ImageInfo newInfo = mInfo;
    newInfo.mWidth = widthNew;
    newInfo.mHeight = heightNew;
    NBRE_Image *image = NBRE_NEW NBRE_Image(newInfo);
    if(image == NULL)
    {
        return NULL;
    }

    uint32 pixelSize = GetPixelSize(newInfo);
    uint32 rowSize = pixelSize * (widthNew < mInfo.mWidth ? widthNew : mInfo.mWidth);
    uint32 h = heightNew < mInfo.mHeight ? heightNew : mInfo.mHeight;
    for (uint32 y = 0; y < h; ++y)
    {
        nsl_memcpy(image->GetImageData() + pixelSize * widthNew * y, mImageData + pixelSize * mInfo.mWidth * y, rowSize);
    }
    return image;
}
