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
#include "nbred3d9texture.h"
#include "nbreitextureimage.h"
#include "nbred3d9renderpal.h"
#include "palstdlib.h"
#include "nbrelog.h"

class ImmediateImageD3D:public NBRE_ITextureImage
{
public:
    ImmediateImageD3D(NBRE_Image** image, uint8 faceCount, uint8 mipmapCount);
    virtual ~ImmediateImageD3D();

public:
    virtual uint8 GetMipmapCount() const                        { return mMipmapCount; }
    virtual uint8 GetFaceCount() const                          { return mFaceCount; }
    virtual const NBRE_Image* GetImage(uint8 faceNumber, uint8 mipmapLevel)const
    {
        nbre_assert((faceNumber*mMipmapCount+mipmapLevel) < (mFaceCount*mMipmapCount));
        return mImages[faceNumber*mMipmapCount+mipmapLevel];
    }
    virtual void ReadInfo(NBRE_ImageInfo &fm, NBRE_ImageInfo &originalInfo)const
    {
        mImages[0]->GetInfo(originalInfo);
		fm = originalInfo;
    }
    virtual void Release();
    virtual PAL_Error Load() { return PAL_Ok; }

protected:
    NBRE_Image** mImages;
    uint8 mFaceCount;
    uint8 mMipmapCount;
};

ImmediateImageD3D::ImmediateImageD3D(NBRE_Image** image, uint8 faceCount, uint8 mipmapCount)
:mImages(image), mFaceCount(faceCount), mMipmapCount(mipmapCount)
{
}

ImmediateImageD3D::~ImmediateImageD3D()
{
    if (mImages != NULL)
    {
        for (uint8 i = 0; i < (mFaceCount * mMipmapCount); ++i)
        {
            NBRE_DELETE mImages[i];
        }
        NBRE_DELETE_ARRAY mImages;
    }
}

void
ImmediateImageD3D::Release()
{
    NBRE_DELETE this;
}

NBRE_D3D9Texture::NBRE_D3D9Texture(NBRE_IRenderPal *renderPal, const NBRE_ImageInfo *info, TextureType type, nb_boolean isUsingMipmap, const NBRE_String& name)
:NBRE_Texture(renderPal, NULL, type, info, isUsingMipmap, name), mD3D9Texture(NULL)
{
}

NBRE_D3D9Texture::NBRE_D3D9Texture(NBRE_IRenderPal *renderPal, NBRE_Image** image, uint8 faceCount, uint8 numMipmaps, TextureType type, nb_boolean isUsingMipmap, const NBRE_String&name)
:NBRE_Texture(renderPal, NBRE_NEW ImmediateImageD3D(image, faceCount, numMipmaps), type, NULL, isUsingMipmap, name), mD3D9Texture(NULL)
{
}

NBRE_D3D9Texture::NBRE_D3D9Texture(NBRE_IRenderPal *renderPal, NBRE_ITextureImage *textureImage, TextureType type, nb_boolean isUsingMipmap, const NBRE_String& name)
:NBRE_Texture(renderPal, textureImage, type, NULL, isUsingMipmap, name), mD3D9Texture(NULL)
{
}

NBRE_D3D9Texture::~NBRE_D3D9Texture()
{
    Unload();
}

static void
CopyMemoryToTexture(const NBRE_Image *pImage, uint8 *des, int pitch, int level)
{
    nbre_assert(pImage && des);
    uint8* pTempLine = des;
    const uint8* pSrc = pImage->GetImageData();
    uint32 height = pImage->Height();
    uint32 width  = pImage->Width();
    for (uint32 y = 0; y < height; y++)
    {
        UCHAR* pTemp = pTempLine;
        for (uint32 x = 0; x < width; x++)
        {
            switch(pImage->Format())
            {
            case NBRE_PF_R8G8B8A8:
                {
                    *pTemp++ = pSrc[2]; // Blue
                    *pTemp++ = pSrc[1]; // Green
                    *pTemp++ = pSrc[0]; // Red
                    *pTemp++ = pSrc[3]; // Alpha
                    pSrc += 4;
                }
                break;
            case NBRE_PF_B8G8R8A8:
                {
                    *pTemp++ = *pSrc++; // Blue
                    *pTemp++ = *pSrc++; // Green
                    *pTemp++ = *pSrc++; // Red
                    *pTemp++ = *pSrc++; // Alpha
                }
                break;
            case NBRE_PF_R8G8B8:
                {
                    *pTemp++ = pSrc[2]; // Blue
                    *pTemp++ = pSrc[1]; // Green
                    *pTemp++ = pSrc[0]; // Red
                    *pTemp++ = 255;     // Alpha
                    pSrc += 3;
                }
                break;
            case NBRE_PF_B8G8R8:
                {
                    *pTemp++ = *pSrc++; // Blue
                    *pTemp++ = *pSrc++; // Green
                    *pTemp++ = *pSrc++; // Red
                    *pTemp++ = 255;     // Alpha
                }
                break;
            case NBRE_PF_A8:
                {
                    *pTemp++ = 255;       // Blue
                    *pTemp++ = 255;       // Green
                    *pTemp++ = 255;       // Red
                    *pTemp++ = *pSrc++;   // Alpha
                }
                break;
            case NBRE_PF_A4R4G4B4:
                {
                    *pTemp++ = pSrc[0] & 0xf0;              // Blue
                    *pTemp++ = (pSrc[1] & 0x0f) << 4;       // Green
                    *pTemp++ = pSrc[1] & 0xf0;              // Red
                    *pTemp++ = (pSrc[0] & 0x0f) << 4;       // Alpha
                    pSrc += 2;
                }
                break;
            case NBRE_PF_R5G6B5:
                {
                    uint16 c = *(uint16*)pSrc;
                    *pTemp++ = (uint8)((c & 0x001F)<<3);    // Blue
                    *pTemp++ = (uint8)((c & 0x07E0)>>3);    // Green
                    *pTemp++ = (uint8)((c & 0xF800)>>8);    // Red
                    *pTemp++ = 255;                         // Alpha
                    pSrc += 2;
                }
                break;
            case NBRE_PF_R4G4B4A4:
                {
                    *pTemp++ = pSrc[0] & 0xf0;              // Blue
                    *pTemp++ = (pSrc[1] & 0x0f)<<4;         // Green
                    *pTemp++ = pSrc[1] & 0xf0;              // Red
                    *pTemp++ = (pSrc[0] & 0x0f)<<4;         // Alpha
                    pSrc += 2;
                }
                break;
            case NBRE_PF_NOTSUPPORT:
            default:
                nbre_assert(0);
            }
        }
        pTempLine += pitch;
    }
}

static uint8
GetLevels(uint32 val)
{
    nbre_assert(val > 0);
    uint8 level = 0;
    while(val)
    {
        val = val >> 1;
        ++level;
    }
    return level;
}

PAL_Error
NBRE_D3D9Texture::Create2DTexture()
{
    if (mTextureImage != NULL)
    {
        PAL_Error err = mTextureImage->Load();
        if (err != PAL_Ok)
        {
            return err;
        }

        const NBRE_Image* pImages = mTextureImage->GetImage(0, 0);
        if (pImages == NULL || mTextureImage->GetMipmapCount() == 0)
        {
            return PAL_Failed;
        }

        pImages->GetInfo(mTextureInfo);
    }

    if (!NBRE_Math::IsPowerOfTwo(mTextureInfo.mHeight) || !NBRE_Math::IsPowerOfTwo(mTextureInfo.mWidth) || mTextureInfo.mFormat == NBRE_PF_NOTSUPPORT)
    {
        return PAL_ErrUnsupported;
    }

    if (mRenderPal == NULL)
    {
        return PAL_ErrBadParam;
    }

    NBRE_D3D9RenderPal *pal = static_cast<NBRE_D3D9RenderPal*>(mRenderPal);

    IDirect3DDevice9 *device = pal->GetActiveD3D9Device();
    nbre_assert(device);
    uint8 count = 1;
    nb_boolean usingMipmap = IsUsingMipmap();
    nb_boolean autoMimmap = FALSE;
    if (usingMipmap)
    {
        if(mTextureImage)
        {
            count = mTextureImage->GetMipmapCount();
            if(count == 1)
            {
                autoMimmap  = pal->GetCapabilities().GetAutoMipmaps();
                usingMipmap = autoMimmap;
            }
        }
        else
        {
            count = GetLevels(NBRE_Math::Max<uint32>(mTextureInfo.mWidth, mTextureInfo.mHeight));
        }
    }
    IDirect3DTexture9 *tex = NULL;
    HRESULT hr = device->CreateTexture(mTextureInfo.mWidth,
                          mTextureInfo.mHeight,
                          autoMimmap?0:count,
                          autoMimmap?D3DUSAGE_AUTOGENMIPMAP:0,
                          D3DFMT_A8R8G8B8,
                          D3DPOOL_MANAGED,
                          &tex,
                          NULL);

    if (hr != D3D_OK || !tex)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D9Texture::Create2DTexture: GenTextures failed!");
        return PAL_Failed;
    }
    mD3D9Texture = tex;
    if (mTextureImage != NULL)
    {
        for (uint8 i = 0; i < count; ++i)
        {
            D3DLOCKED_RECT lockedRect;
            if (FAILED(tex->LockRect(i, &lockedRect, NULL, 0)))
            {
                FreeInternalTextureImpl();
                return PAL_Failed;
            }
            const NBRE_Image *pImage = mTextureImage->GetImage(0, i);
            if (pImage == NULL)
            {
                FreeInternalTextureImpl();
                return PAL_Failed;
            }

            CopyMemoryToTexture(pImage, (uint8*)lockedRect.pBits, lockedRect.Pitch, i);
            tex->UnlockRect(i);
        }
    }
    else
    {
        for (uint8 i = 0; i < count; ++i)
        {
            D3DLOCKED_RECT lockedRect;
            if (FAILED(tex->LockRect(i, &lockedRect, NULL, 0)))
            {
                FreeInternalTextureImpl();
                return PAL_Failed;
            }
            uint32 height = mTextureInfo.mHeight/(1<<i);
            nsl_memset(lockedRect.pBits, 0, lockedRect.Pitch*(height?height:1));
            tex->UnlockRect(i);
        }
    }
    return PAL_Ok;
}

PAL_Error
NBRE_D3D9Texture::GenerateInternalTextureImpl()
{
    PAL_Error err = PAL_Ok;
    switch(mType)
    {
    case TT_2D:
        {
            err = Create2DTexture();
        }
        break;
    case TT_CUBE_MAP:
        {
            err = CreateCubeTexture();
        }
        break;
    case TT_2D_RENDERTARGET:
        {
            err = Create2DRenderTexture();
        }
        break;
    case TT_CUBE_MAP_RENDERTARTE:
        {
            err = CreateCubeRenderTexture();
        }
        break;
    case TT_1D:
    case TT_3D:
    default:
        err = PAL_ErrUnsupported;
    }

    return err;
}

void
NBRE_D3D9Texture::FreeInternalTextureImpl()
{
    if (mD3D9Texture)
    {
        mD3D9Texture->Release();
        mD3D9Texture = NULL;
    }
}

void
NBRE_D3D9Texture::UpdateRegion(int32 x, int32 y, int32 level, NBRE_Image* image)
{
    nbre_assert(image && mType == TT_2D);
    IDirect3DTexture9* tex2D = (IDirect3DTexture9*)mD3D9Texture;
    D3DLOCKED_RECT lockedRect;
    RECT rect;
    rect.left   = x;
    rect.top    = y;
    rect.bottom = y + image->Height();
    rect.right  = x + image->Width();
    HRESULT t = (tex2D->LockRect(level, &lockedRect, &rect, 0));
    if (SUCCEEDED(t))
    {
        CopyMemoryToTexture(image, (uint8*)lockedRect.pBits, lockedRect.Pitch, level);
        tex2D->UnlockRect(level);
    }
}

PAL_Error
NBRE_D3D9Texture::CreateCubeTexture()
{
    if (mTextureImage != NULL)
    {
        PAL_Error err = mTextureImage->Load();
        if (err != PAL_Ok)
        {
            return err;
        }

        if (mTextureImage->GetFaceCount() != 6)
        {
            return PAL_Failed;
        }

        const NBRE_Image* pImages = mTextureImage->GetImage(0, 0);
        if (pImages == NULL || mTextureImage->GetMipmapCount() == 0)
        {
            return PAL_Failed;
        }

        pImages->GetInfo(mTextureInfo);
    }

    if (!NBRE_Math::IsPowerOfTwo(mTextureInfo.mHeight) || !NBRE_Math::IsPowerOfTwo(mTextureInfo.mWidth) || mTextureInfo.mFormat == NBRE_PF_NOTSUPPORT)
    {
        return PAL_ErrUnsupported;
    }

    if (mRenderPal == NULL)
    {
        return PAL_ErrBadParam;
    }

    NBRE_D3D9RenderPal *pal = static_cast<NBRE_D3D9RenderPal*>(mRenderPal);

    IDirect3DDevice9 *device = pal->GetActiveD3D9Device();
    nbre_assert(device);
    IDirect3DCubeTexture9 *cube = NULL;

    uint8 count = 1;
    nb_boolean usingMipmap = IsUsingMipmap();
    nb_boolean autoMimmap = FALSE;
    if (mTextureImage != NULL && usingMipmap && mTextureImage->GetMipmapCount() == 1)
    {
        autoMimmap  = pal->GetCapabilities().GetAutoMipmaps();
        usingMipmap = autoMimmap;
    }
    if (mTextureImage != NULL && usingMipmap)
    {
        count = mTextureImage->GetMipmapCount();
    }

    HRESULT hr = device->CreateCubeTexture(mTextureInfo.mWidth,
        autoMimmap?0:count,
        autoMimmap?D3DUSAGE_AUTOGENMIPMAP:0,
        D3DFMT_A8R8G8B8,
        D3DPOOL_MANAGED,
        &cube,
        NULL);

    if (hr != D3D_OK || !cube)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D9Texture::CreateCubeTexture: GenTextures failed!");
        return PAL_Failed;
    }
    mD3D9Texture = cube;
    if (mTextureImage != NULL)
    {
        for(uint8 j = 0; j < 6; ++j)
        {
            const NBRE_Image* pImages = mTextureImage->GetImage(j, 0);
            if (pImages == NULL || mTextureImage->GetMipmapCount() == 0)
            {
                return PAL_Failed;
            }

            pImages->GetInfo(mTextureInfo);
            if (!NBRE_Math::IsPowerOfTwo(mTextureInfo.mHeight) || !NBRE_Math::IsPowerOfTwo(mTextureInfo.mWidth))
            {
                return PAL_ErrUnsupported;
            }

            uint8 count = 1;
            if (IsUsingMipmap())
            {
                count = mTextureImage->GetMipmapCount();
            }
            for (uint8 i = 0; i < count; ++i)
            {
                const NBRE_Image *pImage = mTextureImage->GetImage(j, i);
                if (pImage == NULL)
                {
                    FreeInternalTextureImpl();
                    return PAL_Failed;
                }
                D3DLOCKED_RECT lockedRect;
                if (FAILED(cube->LockRect((D3DCUBEMAP_FACES)(D3DCUBEMAP_FACE_POSITIVE_X+j), i, &lockedRect,NULL,0)))
                {
                    FreeInternalTextureImpl();
                    return PAL_Failed;
                }

                CopyMemoryToTexture(pImage, (uint8*)lockedRect.pBits, lockedRect.Pitch, i);
                cube->UnlockRect((D3DCUBEMAP_FACES)(D3DCUBEMAP_FACE_POSITIVE_X+j), i);
            }
        }
    }
    return PAL_Ok;
}

PAL_Error
NBRE_D3D9Texture::Create2DRenderTexture()
{
    if (!NBRE_Math::IsPowerOfTwo(mTextureInfo.mHeight) || !NBRE_Math::IsPowerOfTwo(mTextureInfo.mWidth) || IsUsingMipmap())
    {
        return PAL_ErrUnsupported;
    }

    if (mRenderPal == NULL)
    {
        return PAL_ErrBadParam;
    }

    NBRE_D3D9RenderPal *pal = static_cast<NBRE_D3D9RenderPal*>(mRenderPal);

    IDirect3DDevice9 *device = pal->GetActiveD3D9Device();
    nbre_assert(device);
    IDirect3DTexture9 *tex = NULL;
    if (mD3D9Texture)
    {
        mD3D9Texture->Release();
    }
    HRESULT hr = device->CreateTexture(mTextureInfo.mWidth,
        mTextureInfo.mHeight,
        1,
        D3DUSAGE_RENDERTARGET,
        D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT,
        &tex,
        NULL);

    if (hr != D3D_OK)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D9Texture::Create2DRenderTexture: GenTextures failed!");
        return PAL_Failed;
    }
    mD3D9Texture = tex;
    return PAL_Ok;
}

PAL_Error
NBRE_D3D9Texture::CreateCubeRenderTexture()
{
    return PAL_ErrUnsupported;
}
