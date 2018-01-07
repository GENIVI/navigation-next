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
#include "nbred3d11texture.h"
#include "nbreitextureimage.h"
#include "nbred3d11renderpal.h"
#include "nbrelog.h"

class ImmediateImageD3D11:public NBRE_ITextureImage
{
public:
    ImmediateImageD3D11(NBRE_Image** image, uint8 faceCount, uint8 mipmapCount);
    virtual ~ImmediateImageD3D11();

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

ImmediateImageD3D11::ImmediateImageD3D11(NBRE_Image** image, uint8 faceCount, uint8 mipmapCount)
:mImages(image), mFaceCount(faceCount), mMipmapCount(mipmapCount)
{
}

ImmediateImageD3D11::~ImmediateImageD3D11()
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
ImmediateImageD3D11::Release()
{
    NBRE_DELETE this;
}

NBRE_D3D11Texture::NBRE_D3D11Texture(const NBRE_ImageInfo *info, TextureType type, nb_boolean isUsingMipmap, const NBRE_String& name, NBRE_D3D11RenderPal* pal)
    :NBRE_Texture(pal, NULL, type, info, isUsingMipmap, name), mTex(NULL), m1DTex(NULL), m2DTex(NULL), m3DTex(NULL), mShaderResourceView(NULL),mUsage(D3D11_USAGE_DEFAULT)
{
}

NBRE_D3D11Texture::NBRE_D3D11Texture(NBRE_Image** image, uint8 faceCount, uint8 numMipmaps, TextureType type, nb_boolean isUsingMipmap, const NBRE_String&name, NBRE_D3D11RenderPal* pal)
:NBRE_Texture(pal, NBRE_NEW ImmediateImageD3D11(image, faceCount, numMipmaps), type, NULL, isUsingMipmap, name), mTex(NULL), m1DTex(NULL), m2DTex(NULL), m3DTex(NULL), mShaderResourceView(NULL),mUsage(D3D11_USAGE_DEFAULT)
{
}

NBRE_D3D11Texture::NBRE_D3D11Texture(NBRE_ITextureImage *textureImage, TextureType type, nb_boolean isUsingMipmap, const NBRE_String& name, NBRE_D3D11RenderPal* pal)
:NBRE_Texture(pal, textureImage, type, NULL, isUsingMipmap, name), mTex(NULL), m1DTex(NULL), m2DTex(NULL), m3DTex(NULL), mShaderResourceView(NULL),mUsage(D3D11_USAGE_DEFAULT)
{
}

NBRE_D3D11Texture::~NBRE_D3D11Texture()
{
    Unload();
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

static DXGI_FORMAT
ToD3D11Format(NBRE_PixelFormat format)
{
    switch(format)
    {
    case NBRE_PF_R8G8B8A8:
    case NBRE_PF_R8G8B8:
    case NBRE_PF_B8G8R8:
    case NBRE_PF_A4R4G4B4:
    case NBRE_PF_A8:
    case NBRE_PF_R4G4B4A4:
    case NBRE_PF_R5G6B5:
         return DXGI_FORMAT_R8G8B8A8_UNORM;
    case NBRE_PF_NOTSUPPORT:
    default:
        nbre_assert(FALSE);
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    };
}

static uint8
GetPixelSize(NBRE_PixelFormat format)
{
    switch(format)
    {
    case NBRE_PF_R8G8B8A8:
    case NBRE_PF_B8G8R8:
    case NBRE_PF_R8G8B8:
    case NBRE_PF_A4R4G4B4:
    case NBRE_PF_A8:
    case NBRE_PF_R4G4B4A4:
    case NBRE_PF_R5G6B5:
         return 4;
    case NBRE_PF_NOTSUPPORT:
    default:
        nbre_assert(FALSE);
        return 0;
    };
}
static void
CopyMemoryToTex(ID3D11DeviceContext *context, UINT level, const D3D11_BOX* pBox, const NBRE_Image* pImage, ID3D11Resource *texture)
{
    nbre_assert(context && pImage && texture);
    uint32 size = pImage->Height()*pImage->Width()*4;
    UINT rowPitch = static_cast<UINT>(pImage->Width()*GetPixelSize(pImage->Format()));
    switch (pImage->Format())
    {
    case NBRE_PF_R8G8B8:
        {
            uint8 *data = NBRE_NEW uint8[size];
            uint8 *dest = data;
            const uint8* psrc = pImage->GetImageData();
            for (uint32 j = 0; j < size; j+=4)
            {
                *dest++ = *psrc++;
                *dest++ = *psrc++;
                *dest++ = *psrc++;
                *dest++ = 255;
            }
            context->UpdateSubresource(texture, level, pBox, (void*)data, rowPitch, 0);
            NBRE_DELETE_ARRAY data;
        }
        break;
    case NBRE_PF_R8G8B8A8:
        {
            context->UpdateSubresource(texture, level, pBox, (void*)pImage->GetImageData(), rowPitch, 0);
        }
        break;
    case NBRE_PF_A8:
        {
            uint8 *data = NBRE_NEW uint8[size];
            uint8 *dest = data;
            const uint8* psrc = pImage->GetImageData();
            for (uint32 j = 0; j < size; j+=4)
            {
                *dest++ = 255;
                *dest++ = 255;
                *dest++ = 255;
                *dest++ = *psrc++;
            }
            context->UpdateSubresource(texture, level, pBox, (void*)data, rowPitch, 0);
            NBRE_DELETE_ARRAY data;
        }
        break;
    case NBRE_PF_B8G8R8:
        {
            uint8 *data = NBRE_NEW uint8[size];
            uint8 *dest = data;
            const uint8* psrc = pImage->GetImageData();
            for (uint32 j = 0; j < size; j+=4)
            {
                *dest++ = psrc[2];
                *dest++ = psrc[1];
                *dest++ = psrc[0];
                *dest++ = 255;
                psrc += 3;
            }
            context->UpdateSubresource(texture, level, pBox, (void*)data, rowPitch, 0);
            NBRE_DELETE_ARRAY data;
        }
        break;
    case NBRE_PF_A4R4G4B4:
        {
            uint8 *data = NBRE_NEW uint8[size];
            uint8 *dest = data;
            const uint8* psrc = pImage->GetImageData();
            for (uint32 j = 0; j < size; j+=4)
            {
                *dest++ = (psrc[1] & 0x0f)<<4;
                *dest++ = (psrc[0] & 0xf0);
                *dest++ = (psrc[0] & 0x0f)<<4;
                *dest++ = (psrc[1] & 0xf0);
                psrc += 2;
            }
            context->UpdateSubresource(texture, level, pBox, (void*)data, rowPitch, 0);
            NBRE_DELETE_ARRAY data;
        }
        break;
    case NBRE_PF_R5G6B5:
        {
            uint8 *data = NBRE_NEW uint8[size];
            uint8 *dest = data;
            const uint8* pSrc = pImage->GetImageData();
            for (uint32 j = 0; j < size; j+=4)
            {
                uint16 c = *(uint16*)pSrc;
                *dest++ = (uint8)((c & 0xF800)>>8);
                *dest++ = (uint8)((c & 0x07E0)>>3);
                *dest++ = (uint8)((c & 0x001F)<<3);
                *dest++ = 255;
                pSrc += 2;
            }
            context->UpdateSubresource(texture, level, pBox, (void*)data, rowPitch, 0);
            NBRE_DELETE_ARRAY data;
        }
        break;
    case NBRE_PF_R4G4B4A4:
        {
            uint8 *data = NBRE_NEW uint8[size];
            uint8 *dest = data;
            const uint8* pSrc = pImage->GetImageData();
            for (uint32 j = 0; j < size; j+=4)
            {
                *dest++ = (pSrc[1] & 0xf0);
                *dest++ = (pSrc[1] & 0x0f)<<4;
                *dest++ = (pSrc[0] & 0xf0);
                *dest++ = (pSrc[0] & 0x0f)<<4;
                pSrc += 2;
            }
            context->UpdateSubresource(texture, level, pBox, (void*)data, rowPitch, 0);
            NBRE_DELETE_ARRAY data;
        }
        break;
    default:
        nbre_assert(FALSE);
        break;
    }
}

PAL_Error
NBRE_D3D11Texture::Create2DTexture()
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

    NBRE_D3D11RenderPal* pal = dynamic_cast<NBRE_D3D11RenderPal*>(mRenderPal);
    ID3D11Device *device = pal->GetActiveD3D11Device();
    ID3D11DeviceContext *context = pal->GetImmediateContext();

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
                autoMimmap  = mRenderPal->GetCapabilities().GetAutoMipmaps();
                usingMipmap = autoMimmap;
            }
        }
        else
        {
            count = GetLevels(NBRE_Math::Max<uint32>(mTextureInfo.mWidth, mTextureInfo.mHeight));
        }
    }

    D3D11_TEXTURE2D_DESC desc;
    desc.Width     = static_cast<UINT>(mTextureInfo.mWidth);
    desc.Height    = static_cast<UINT>(mTextureInfo.mHeight);
    desc.MipLevels = autoMimmap?0:count;
    desc.ArraySize = 1;
    desc.Format    = ToD3D11Format(mTextureInfo.mFormat);
    DXGI_SAMPLE_DESC sampleDesc;
    sampleDesc.Count    = 1;
    sampleDesc.Quality  = 0;
    desc.SampleDesc     = sampleDesc;
    desc.Usage          = D3D11_USAGE_DEFAULT;
    desc.BindFlags      = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags      = autoMimmap?D3D11_RESOURCE_MISC_GENERATE_MIPS:0;
    mUsage = desc.Usage;

    // create the texture
    HRESULT hr = device->CreateTexture2D(
        &desc,
        NULL,
        &m2DTex);
    if (FAILED(hr) || m2DTex == NULL)
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D11Texture::Create2DTexture: GenTextures failed!");
        return PAL_Failed;
    }

    hr = m2DTex->QueryInterface(__uuidof(ID3D11Resource), (void**)&mTex);
    if (FAILED(hr) || mTex == NULL)
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D11Texture::Create2DTexture: QueryInterface failed!");
        return PAL_Failed;
    }
    if (mTextureImage != NULL)
    {
        for (uint8 i = 0; i < count; ++i)
        {
            const NBRE_Image *pImage = mTextureImage->GetImage(0, i);
            if (pImage == NULL)
            {
                FreeInternalTextureImpl();
                return PAL_Failed;
            }
            CopyMemoryToTex(pal->GetImmediateContext(), i, NULL, pImage, mTex);
        }
    }
    else
    {
        uint32 size = mTextureInfo.mWidth*mTextureInfo.mHeight*GetPixelSize(mTextureInfo.mFormat);
        uint8 *buffer = NBRE_NEW uint8[size];
        nsl_memset(buffer, 0, size);
        for (uint8 i = 0; i < count; ++i)
        {
            uint32 width =  mTextureInfo.mWidth>>i;
            UINT rowPitch = (width?width:1)*(GetPixelSize(mTextureInfo.mFormat));
            context->UpdateSubresource(mTex, i, NULL, (void*)buffer, rowPitch, 0);
        }
        NBRE_DELETE_ARRAY buffer;
    }

    // set final tex
    m2DTex->GetDesc(&desc);

    ZeroMemory( &mSRVDesc, sizeof(mSRVDesc) );
    mSRVDesc.Format = desc.Format;
    mSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    mSRVDesc.Texture2D.MipLevels = desc.MipLevels;

    hr = device->CreateShaderResourceView( m2DTex, &mSRVDesc, &mShaderResourceView );
    if(autoMimmap)
    {
        context->GenerateMips(mShaderResourceView);
    }
    if (FAILED(hr))
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D11Texture::Create2DTexture: CreateShaderResourceView failed!");
        return PAL_Failed;
    }
    return PAL_Ok;
}

PAL_Error
NBRE_D3D11Texture::GenerateInternalTextureImpl()
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
NBRE_D3D11Texture::FreeInternalTextureImpl()
{
    NBRE_SAFE_RELEASE(mShaderResourceView);
    NBRE_SAFE_RELEASE(m1DTex);
    NBRE_SAFE_RELEASE(m2DTex);
    NBRE_SAFE_RELEASE(m3DTex);
    NBRE_SAFE_RELEASE(mTex);
}

void
NBRE_D3D11Texture::UpdateRegion(int32 x, int32 y, int32 level, NBRE_Image* image)
{
    nbre_assert(image && mType == TT_2D && image->Format() == mTextureInfo.mFormat);
    D3D11_BOX box;
    box.left   = x;
    box.top    = y;
    box.bottom = y + image->Height();
    box.right  = x + image->Width();
    box.back   = 1;
    box.front  = 0;

    switch (mUsage)
    {
    case D3D11_USAGE_DEFAULT:
        {
            CopyMemoryToTex(((NBRE_D3D11RenderPal*)mRenderPal)->GetImmediateContext(), level, &box, image, mTex);
        }
        break;
    case D3D11_USAGE_IMMUTABLE:
    case D3D11_USAGE_DYNAMIC:
    case D3D11_USAGE_STAGING:
    default:
        nbre_assert(FALSE);
        break;
    }
}

PAL_Error
NBRE_D3D11Texture::CreateCubeTexture()
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
        return  PAL_ErrUnsupported;
    }
    NBRE_D3D11RenderPal* pal = dynamic_cast<NBRE_D3D11RenderPal*>(mRenderPal);
    ID3D11Device *device = pal->GetActiveD3D11Device();
    ID3D11DeviceContext *context = pal->GetImmediateContext();

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
                autoMimmap  = mRenderPal->GetCapabilities().GetAutoMipmaps();
                usingMipmap = autoMimmap;
            }
        }
        else
        {
            count = GetLevels(NBRE_Math::Max<uint32>(mTextureInfo.mWidth, mTextureInfo.mHeight));
        }
    }

    D3D11_TEXTURE2D_DESC desc;
    desc.Width            = static_cast<UINT>(mTextureInfo.mWidth);
    desc.Height            = static_cast<UINT>(mTextureInfo.mHeight);
    desc.MipLevels        = autoMimmap?0:count;
    desc.ArraySize        = 6;
    desc.Format            = ToD3D11Format(mTextureInfo.mFormat);
    DXGI_SAMPLE_DESC sampleDesc;
    sampleDesc.Count = 1;
    sampleDesc.Quality = 0;
    desc.SampleDesc        = sampleDesc;
    desc.Usage            = D3D11_USAGE_DEFAULT;
    desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags        = D3D11_RESOURCE_MISC_TEXTURECUBE;
    mUsage = desc.Usage;
    if(autoMimmap)
    {
        desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }

    // create the texture
    HRESULT hr = device->CreateTexture2D(
        &desc,
        NULL,
        &m2DTex);
    if (FAILED(hr) || m2DTex == NULL)
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D11Texture::CreateCubeTexture: GenTextures failed!");
        return PAL_Failed;
    }

    hr = m2DTex->QueryInterface(__uuidof(ID3D11Resource), (void**)&mTex);
    if (FAILED(hr) || mTex == NULL)
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D11Texture::CreateCubeTexture: QueryInterface failed!");
        return PAL_Failed;
    }
    if (mTextureImage != NULL)
    {
        for(uint8 j = 0; j < 6; ++j)
        {
            for (uint8 i = 0; i < count; ++i)
            {
                const NBRE_Image *pImage = mTextureImage->GetImage(j, i);
                if (pImage == NULL)
                {
                    FreeInternalTextureImpl();
                    return PAL_Failed;
                }
                CopyMemoryToTex(pal->GetImmediateContext(), D3D11CalcSubresource(i, j, count), NULL, pImage, mTex);
            }
        }
    }

    // set final tex
    m2DTex->GetDesc(&desc);

    ZeroMemory( &mSRVDesc, sizeof(mSRVDesc) );
    mSRVDesc.Format = desc.Format;
    mSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    mSRVDesc.Texture2D.MipLevels = desc.MipLevels;
    mSRVDesc.Texture2D.MostDetailedMip = 0;

    hr = device->CreateShaderResourceView( m2DTex, &mSRVDesc, &mShaderResourceView );
    if(autoMimmap)
    {
        context->GenerateMips(mShaderResourceView);
    }
    if (FAILED(hr))
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D11Texture::CreateCubeTexture: CreateShaderResourceView failed!");
        return PAL_Failed;
    }
    return PAL_Ok;
}

PAL_Error
NBRE_D3D11Texture::Create2DRenderTexture()
{
    if (!NBRE_Math::IsPowerOfTwo(mTextureInfo.mHeight) || !NBRE_Math::IsPowerOfTwo(mTextureInfo.mWidth) || mTextureInfo.mFormat == NBRE_PF_NOTSUPPORT)
    {
        return PAL_ErrUnsupported;
    }

    NBRE_D3D11RenderPal* pal = dynamic_cast<NBRE_D3D11RenderPal*>(mRenderPal);
    ID3D11Device *device = pal->GetActiveD3D11Device();

    D3D11_TEXTURE2D_DESC desc;
    desc.Width     = static_cast<UINT>(mTextureInfo.mWidth);
    desc.Height    = static_cast<UINT>(mTextureInfo.mHeight);
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format    = ToD3D11Format(mTextureInfo.mFormat);
    DXGI_SAMPLE_DESC sampleDesc;
    sampleDesc.Count    = 1;
    sampleDesc.Quality  = 0;
    desc.SampleDesc     = sampleDesc;
    desc.Usage          = D3D11_USAGE_DEFAULT;
    desc.BindFlags      = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags      = 0;
    mUsage = desc.Usage;

    // create the texture
    HRESULT hr = device->CreateTexture2D(
        &desc,
        NULL,
        &m2DTex);
    if (FAILED(hr) || m2DTex == NULL)
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D11Texture::Create2DRenderTexture: GenTextures failed!");
        return PAL_Failed;
    }

    hr = m2DTex->QueryInterface(__uuidof(ID3D11Resource), (void**)&mTex);
    if (FAILED(hr) || mTex == NULL)
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D11Texture::Create2DRenderTexture: QueryInterface failed!");
        return PAL_Failed;
    }

      // set final tex
    m2DTex->GetDesc(&desc);

    ZeroMemory( &mSRVDesc, sizeof(mSRVDesc) );
    mSRVDesc.Format = desc.Format;
    mSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    mSRVDesc.Texture2D.MipLevels = desc.MipLevels;
    mSRVDesc.Texture2D.MostDetailedMip = 0;

    hr = device->CreateShaderResourceView( m2DTex, &mSRVDesc, &mShaderResourceView );
    if (FAILED(hr))
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D11Texture::Create2DRenderTexture: CreateShaderResourceView failed!");
        return PAL_Failed;
    }
    return PAL_Ok;
}

PAL_Error
NBRE_D3D11Texture::CreateCubeRenderTexture()
{
    return PAL_ErrUnsupported;
}
