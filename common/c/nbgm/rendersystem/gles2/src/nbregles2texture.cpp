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
#include "nbregles2texture.h"
#include "nbreitextureimage.h"
#include "nbregles2renderpal.h"
#include "palstdlib.h"
#include "nbrelog.h"
#include "nbregles2rendertexture.h"
#include "nbregles2common.h"

class ImmediateImageES2:public NBRE_ITextureImage
{
public:
    ImmediateImageES2(NBRE_Image** image, uint8 faceCount, uint8 mipmapCount);
    virtual ~ImmediateImageES2();

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

ImmediateImageES2::ImmediateImageES2(NBRE_Image** image, uint8 faceCount, uint8 mipmapCount)
:mImages(image), mFaceCount(faceCount), mMipmapCount(mipmapCount)
{
}

ImmediateImageES2::~ImmediateImageES2()
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
ImmediateImageES2::Release()
{
    NBRE_DELETE this;
}

static uint32
GetPorperRenderSize(uint32 size)
{
    //Need to extend the size.
    uint32 ret = 0x01;
    while(ret < size)
    {
        ret <<= 1;
    }
    return ret;
}

static uint8*
ContvertB8G8R8A8ToR8G8B8A8(const NBRE_Image *pImage)
{
    nbre_assert(pImage);
    const uint8* pSrc = pImage->GetImageData();
    uint32 height = pImage->Height();
    uint32 width  = pImage->Width();
    uint8* des = NBRE_NEW uint8[height*width*4];
    uint8* pTemp = des;
    for (uint32 y = 0; y < height; y++)
    {
        for (uint32 x = 0; x < width; x++)
        {
            *pTemp++ = pSrc[2]; // Blue
            *pTemp++ = pSrc[1]; // Green
            *pTemp++ = pSrc[0]; // Red
            *pTemp++ = pSrc[3]; // Alpha
            pSrc += 4;
        }
    }
    return des;
}

static uint8*
ContvertB8G8R8ToR8G8B8(const NBRE_Image *pImage)
{
    nbre_assert(pImage);
    const uint8* pSrc = pImage->GetImageData();
    uint32 height = pImage->Height();
    uint32 width  = pImage->Width();
    uint8* des = NBRE_NEW uint8[height*width*3];
    uint8* pTemp = des;
    for (uint32 y = 0; y < height; y++)
    {
        for (uint32 x = 0; x < width; x++)
        {
            *pTemp++ = pSrc[2]; // Blue
            *pTemp++ = pSrc[1]; // Green
            *pTemp++ = pSrc[0]; // Red
            pSrc += 3;
        }
    }
    return des;
}

static void 
TexImage2D (const NBRE_Image* pImage, int32 level)
{
    switch(pImage->Format())
    {
    case NBRE_PF_B8G8R8A8:
        {
            uint8* buffer = ContvertB8G8R8A8ToR8G8B8A8(pImage);
            glTexImage2D(GL_TEXTURE_2D,
                         level,
                         GL_RGBA,
                         pImage->Width(),
                         pImage->Height(),
                         0,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         buffer);
            NBRE_DELETE_ARRAY buffer;
        }
        break;
    case NBRE_PF_R8G8B8A8:
        {
            glTexImage2D(GL_TEXTURE_2D,
                         level,
                         GL_RGBA,
                         pImage->Width(),
                         pImage->Height(),
                         0,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         pImage->GetImageData());
        }
        break;
    case NBRE_PF_B8G8R8:
        {
            uint8* buffer = ContvertB8G8R8ToR8G8B8(pImage);
            glTexImage2D(GL_TEXTURE_2D,
                         level,
                         GL_RGB,
                         pImage->Width(),
                         pImage->Height(),
                         0,
                         GL_RGB,
                         GL_UNSIGNED_BYTE,
                         buffer);
            NBRE_DELETE_ARRAY buffer;
        }
        break;
    case NBRE_PF_R8G8B8:
        {
            glTexImage2D(GL_TEXTURE_2D,
                         level,
                         GL_RGB,
                         pImage->Width(),
                         pImage->Height(),
                         0,
                         GL_RGB,
                         GL_UNSIGNED_BYTE,
                         pImage->GetImageData());
        }
        break;
    case NBRE_PF_A8:
        {
            glTexImage2D(GL_TEXTURE_2D,
                        level,
                         GL_ALPHA,
                         pImage->Width(),
                         pImage->Height(),
                         0,
                         GL_ALPHA,
                         GL_UNSIGNED_BYTE,
                         pImage->GetImageData());
            //uint8* buffer = ContvertA8ToR8G8B8A8(pImage);
            //glTexImage2D(GL_TEXTURE_2D,
            //             level,
            //             GL_RGBA,
            //             pImage->Width(),
            //             pImage->Height(),
            //             0,
            //             GL_RGBA,
            //             GL_UNSIGNED_BYTE,
            //             buffer);
            //NBRE_DELETE_ARRAY buffer;
        }
        break;
    case NBRE_PF_R4G4B4A4:
        {
            glTexImage2D(GL_TEXTURE_2D,
                         level,
                         GL_RGBA,
                         pImage->Width(),
                         pImage->Height(),
                         0,
                         GL_RGBA,
                         GL_UNSIGNED_SHORT_4_4_4_4,
                         pImage->GetImageData());
        }
        break;
    case NBRE_PF_R5G6B5:
        {
            glTexImage2D(GL_TEXTURE_2D,
                         level,
                         GL_RGB,
                         pImage->Width(),
                         pImage->Height(),
                         0,
                         GL_RGB,
                         GL_UNSIGNED_SHORT_5_6_5,
                         pImage->GetImageData());
        }
        break;
    default:
        nbre_assert(0);
    }
}

static void 
TexSubImage2D (const NBRE_Image* pImage, int32 level, int32 xoffset, int32 yoffset)
{
    switch(pImage->Format())
    {
    case NBRE_PF_B8G8R8A8:
        {
            uint8* buffer = ContvertB8G8R8A8ToR8G8B8A8(pImage);
            glTexSubImage2D(GL_TEXTURE_2D,
                         level,
                         xoffset,
                         yoffset,
                         pImage->Width(),
                         pImage->Height(),
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         buffer);
            NBRE_DELETE_ARRAY buffer;
        }
        break;
    case NBRE_PF_R8G8B8A8:
        {
            glTexSubImage2D(GL_TEXTURE_2D,
                         level,
                         xoffset,
                         yoffset,
                         pImage->Width(),
                         pImage->Height(),
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         pImage->GetImageData());
        }
        break;
    case NBRE_PF_B8G8R8:
        {
            uint8* buffer = ContvertB8G8R8ToR8G8B8(pImage);
            glTexSubImage2D(GL_TEXTURE_2D,
                         level,
                         xoffset,
                         yoffset,
                         pImage->Width(),
                         pImage->Height(),
                         GL_RGB,
                         GL_UNSIGNED_BYTE,
                         buffer);
            NBRE_DELETE_ARRAY buffer;
        }
        break;
    case NBRE_PF_R8G8B8:
        {
            glTexSubImage2D(GL_TEXTURE_2D,
                         level,
                         xoffset,
                         yoffset,
                         pImage->Width(),
                         pImage->Height(),
                         GL_RGB,
                         GL_UNSIGNED_BYTE,
                         pImage->GetImageData());
        }
        break;
    case NBRE_PF_A8:
        {
            glTexSubImage2D(GL_TEXTURE_2D,
                         level,
                         xoffset,
                         yoffset,
                         pImage->Width(),
                         pImage->Height(),
                         GL_ALPHA,
                         GL_UNSIGNED_BYTE,
                         pImage->GetImageData());
        }
        break;
    case NBRE_PF_R4G4B4A4:
        {
            glTexSubImage2D(GL_TEXTURE_2D,
                         level,
                         xoffset,
                         yoffset,
                         pImage->Width(),
                         pImage->Height(),
                         GL_RGBA,
                         GL_UNSIGNED_SHORT_4_4_4_4,
                         pImage->GetImageData());
        }
        break;
    case NBRE_PF_R5G6B5:
        {
            glTexSubImage2D(GL_TEXTURE_2D,
                         level,
                         xoffset,
                         yoffset,
                         pImage->Width(),
                         pImage->Height(),
                         GL_RGB,
                         GL_UNSIGNED_SHORT_5_6_5,
                         pImage->GetImageData());
        }
        break;
    default:
        nbre_assert(0);
    }
}

NBRE_GLES2Texture::NBRE_GLES2Texture(NBRE_IRenderPal *renderPal, const NBRE_ImageInfo *info, TextureType type, nb_boolean isUsingMipmap, const NBRE_String& name)
:NBRE_Texture(renderPal, NULL, type, info, isUsingMipmap, name), mTextureID(0), m_coordinateScaleX(1.f), m_coordinateScaleY(1.f)
{

}

NBRE_GLES2Texture::NBRE_GLES2Texture(NBRE_IRenderPal *renderPal, NBRE_Image** image, uint8 faceCount, uint8 numMipmaps, TextureType type, nb_boolean isUsingMipmap, const NBRE_String&name)
:NBRE_Texture(renderPal, NBRE_NEW ImmediateImageES2(image, faceCount, numMipmaps), type, NULL, isUsingMipmap, name), mTextureID(0), m_coordinateScaleX(1.f), m_coordinateScaleY(1.f)
{

}

NBRE_GLES2Texture::NBRE_GLES2Texture(NBRE_IRenderPal *renderPal, NBRE_ITextureImage *textureImage, TextureType type, nb_boolean isUsingMipmap, const NBRE_String& name)
:NBRE_Texture(renderPal, textureImage, type, NULL, isUsingMipmap, name), mTextureID(0), m_coordinateScaleX(1.f), m_coordinateScaleY(1.f)
{
}

NBRE_GLES2Texture::~NBRE_GLES2Texture()
{
    Unload();
}

GLenum
NBRE_GLES2Texture::GetTextureTarget(void) const
{
    switch(mType)
    {
    case TT_2D:
        return GL_TEXTURE_2D;

    case TT_2D_RENDERTARGET:
        return GL_TEXTURE_2D;

    case TT_CUBE_MAP:
        return GL_TEXTURE_CUBE_MAP;

    case TT_CUBE_MAP_RENDERTARTE:
        return GL_TEXTURE_CUBE_MAP;

    case TT_1D:
    case TT_3D:
    default:
        nbre_assert(FALSE);
        return 0;
    };
}

GLenum
NBRE_GLES2Texture::GetPixelType() const
{
    switch(mTextureInfo.mFormat)
    {
    case NBRE_PF_R8G8B8A8:
    case NBRE_PF_B8G8R8A8:
    case NBRE_PF_R8G8B8:
    case NBRE_PF_B8G8R8:
    case NBRE_PF_A8:
    case NBRE_PF_A8R8G8B8:
    case NBRE_PF_A8B8G8R8:
        return GL_UNSIGNED_BYTE;

    case NBRE_PF_A4R4G4B4:
    case NBRE_PF_R4G4B4A4:
        return GL_UNSIGNED_SHORT_4_4_4_4;

    case NBRE_PF_R5G6B5:
        return GL_UNSIGNED_SHORT_5_6_5;

    case NBRE_PF_NOTSUPPORT:
    default:
        nbre_assert(FALSE);
        return 0;
    };
}

GLenum
NBRE_GLES2Texture::GetSrcPixelFormat(NBRE_PixelFormat format) const
{
    switch(format)
    {
    case NBRE_PF_R8G8B8A8:
    case NBRE_PF_A4R4G4B4:
    case NBRE_PF_R4G4B4A4:
        return GL_RGBA;

    case NBRE_PF_R8G8B8:
    case NBRE_PF_R5G6B5:
        return GL_RGB;

    case NBRE_PF_A8:
        return GL_RGBA;

    case NBRE_PF_B8G8R8A8:
    case NBRE_PF_B8G8R8:
    case NBRE_PF_NOTSUPPORT:
    default:
        nbre_assert(FALSE);
        return 0;
    };
}

GLenum
NBRE_GLES2Texture::GetTexPixelFormat() const
{
    switch(mTextureInfo.mFormat)
    {
    case NBRE_PF_R8G8B8A8:
    case NBRE_PF_B8G8R8A8:
    case NBRE_PF_A8R8G8B8:
    case NBRE_PF_A8B8G8R8:
    case NBRE_PF_A4R4G4B4:
    case NBRE_PF_R4G4B4A4:
        return GL_RGBA;

    case NBRE_PF_R8G8B8:
    case NBRE_PF_B8G8R8:
    case NBRE_PF_R5G6B5:
        return GL_RGB;

    case NBRE_PF_A8:
        return GL_ALPHA;

    case NBRE_PF_NOTSUPPORT:
    default:
        nbre_assert(FALSE);
        return 0;
    };
}

PAL_Error
NBRE_GLES2Texture::Create2DTexture()
{
    nbre_assert(mTextureImage != NULL);
    GL_CHECK_ERROR;

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

    if (mTextureInfo.mFormat == NBRE_PF_NOTSUPPORT)
    {
        return PAL_ErrUnsupported;
    }

    GL_CHECK_ERROR;

    glGenTextures(1, &mTextureID);
    if (mTextureID == 0)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical,
                      "NBRE_GLES2Texture::Create2DTexture: glGenTextures failed! texture name=%s glError = %x",
                      mName.c_str(),
                      glGetError());
        return PAL_Failed;
    }

    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    uint8 count = 1;
    if (IsUsingMipmap())
    {
        count = mTextureImage->GetMipmapCount();
    }

    for (uint8 i = 0; i < count; ++i)
    {
        const NBRE_Image *pImage = mTextureImage->GetImage(0, i);
        if (pImage == NULL)
        {
            FreeInternalTextureImpl();
            return PAL_Failed;
        }

        NBRE_ImageInfo info;
        pImage->GetInfo(info);
        uint32 originalWidth = info.mWidth;
        uint32 originalHeight = info.mHeight;
        nb_boolean needAdjust = FALSE;
        if(!NBRE_Math::IsPowerOfTwo(originalWidth))
        {
            info.mWidth = GetPorperRenderSize(originalWidth);
            m_coordinateScaleX = (originalWidth)/(float)(info.mWidth);
            needAdjust = TRUE;
        }
        if(!NBRE_Math::IsPowerOfTwo(originalHeight))
        {
            info.mHeight= GetPorperRenderSize(originalHeight);
            m_coordinateScaleY = (originalHeight)/(float)(info.mHeight);
            needAdjust = TRUE;
        }
        if(needAdjust)
        {
            NBRE_Image *image = pImage->getExtendImage(info.mWidth, info.mHeight);
            TexImage2D(image, i);
            NBRE_DELETE image;
        }
        else
        {
            TexImage2D(pImage, i);
        }
    }

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityCritical,
                      "NBRE_GLES2Texture::Create2DTexture, glTexImage2D failed! "
                      "texture name=%s, format=0x%04x, type=0x%04x, glError = %x, line=%d",
                      mName.c_str(),
                      GetSrcPixelFormat(mTextureInfo.mFormat),
                      GetPixelType(),
                      error,
                      __LINE__);
        return PAL_Failed;
    }

    if (IsUsingMipmap() && mTextureImage->GetMipmapCount() == 1)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    error = glGetError();
    if (error == GL_NO_ERROR)
    {
        return PAL_Ok;
    }
    else
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityCritical,
                      "NBRE_GLES2Texture::Create2DTexture, glGenerateMipmap failed! texture name=%s, glError = %x, line=%d",
                      mName.c_str(),
                      error,
                      __LINE__);
        return PAL_Failed;
    }
}

PAL_Error
NBRE_GLES2Texture::GenerateInternalTextureImpl()
{
    GLenum error = GL_NO_ERROR;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical,
                      "NBRE_GLES2Texture::Create2DTexture, Found gl errors! texture name=%s, glError = %x, line=%d",
                      mName.c_str(),
                      error,
                      __LINE__);
    }

    PAL_Error err = PAL_Ok;
    switch(mType)
    {
    case TT_2D:
        {
            err = Create2DTexture();
        }
        break;

    case TT_2D_RENDERTARGET:
        {
            err = Create2DRenderTexture();
        }
        break;

    case TT_CUBE_MAP:
        {
            err = CreateCubeTexture();
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

PAL_Error
NBRE_GLES2Texture::CreateCubeTexture()
{
    if (mTextureImage == NULL)
    {
        return PAL_Failed;
    }

    if(!mRenderPal->GetCapabilities().GetSupportCubeMap())
    {
        NBRE_DebugLog(PAL_LogSeverityCritical,
                      "NBRE_GLES2Texture::CreateCubeTexture, current render system doesn't support cube maping!");
        return PAL_ErrUnsupported;
    }

    PAL_Error err = mTextureImage->Load();
    if (err != PAL_Ok)
    {
        return err;
    }

    if (mTextureImage->GetFaceCount() != 6)
    {
        return PAL_Failed;
    }

    glGenTextures(1, &mTextureID);
    if (mTextureID == 0)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_GLES2Texture::CreateCubeTexture: GenTextures failed! glError = %x, line=%u", glGetError(), __LINE__);
        return PAL_Failed;
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

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
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+j, i, GetTexPixelFormat(), pImage->Width(), pImage->Height(), 0, GetSrcPixelFormat(mTextureInfo.mFormat), GetPixelType(), pImage->GetImageData());
        }
    }

    if (IsUsingMipmap() && mTextureImage->GetMipmapCount() == 1)
    {
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    GLenum error = glGetError();
    if (error == GL_NO_ERROR)
    {
        return PAL_Ok;
    }
    else
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_GLES2Texture::CreateCubeTexture failed! glError = %x, line=%u", error, __LINE__);
        return PAL_Failed;
    }
}

PAL_Error
NBRE_GLES2Texture::Create2DRenderTexture()
{
    if (!NBRE_Math::IsPowerOfTwo(mTextureInfo.mHeight) || !NBRE_Math::IsPowerOfTwo(mTextureInfo.mWidth))
    {
        return PAL_ErrUnsupported;
    }

    glGenTextures(1, &mTextureID);
    if (mTextureID == 0)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_GLES2Texture::Create2DRenderTexture: GenTextures failed! glError = %x", glGetError());
        return PAL_Failed;
    }
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (IsUsingMipmap())
    {
        return PAL_ErrUnsupported;
    }
    // Ensure the texture is completed!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GetTexPixelFormat(), mTextureInfo.mWidth, mTextureInfo.mHeight, 0, GetSrcPixelFormat(mTextureInfo.mFormat), GetPixelType(), NULL);

    GLenum error = glGetError();
    if (error == GL_NO_ERROR)
    {
        return PAL_Ok;
    }
    else
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_GLES2Texture::Create2DRenderTexture failed! glError = %x", error);
        return PAL_Failed;
    }
}

PAL_Error
NBRE_GLES2Texture::CreateCubeRenderTexture()
{
    return PAL_ErrUnsupported;
}

void
NBRE_GLES2Texture::FreeInternalTextureImpl()
{
    glDeleteTextures(1, &mTextureID);
    mTextureID  = 0;
}

void
NBRE_GLES2Texture::UpdateRegion(int32 x, int32 y, int32 level, NBRE_Image* image)
{
    nbre_assert(image != NULL);
    nbre_assert(mTextureID!=0);
    glBindTexture(GL_TEXTURE_2D, mTextureID);

    GL_CHECK_ERROR
    TexSubImage2D(image, level, x, y);
    GL_CHECK_ERROR

}

