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
#include "nbregltexture.h"
#include "nbreitextureimage.h"
#include "nbreglrenderpal.h"
#include "palstdlib.h"
#include "nbrelog.h"
#include "nbreglrendertexture.h"

class ImmediateImage:public NBRE_ITextureImage
{
public:
    ImmediateImage(NBRE_Image** image, uint8 faceCount, uint8 mipmapCount);
    virtual ~ImmediateImage();

public:
    virtual uint8 GetMipmapCount() const                        { return mMipmapCount; }
    virtual uint8 GetFaceCount() const                          { return mFaceCount; }
    virtual const NBRE_Image* GetImage(uint8 faceNumber, uint8 mipmapLevel)const  
    { 
        nbre_assert((faceNumber*mMipmapCount+mipmapLevel) < (mFaceCount*mMipmapCount)); 
        return mImages[faceNumber*mMipmapCount+mipmapLevel]; 
    }
    virtual void Release();
    virtual PAL_Error Load() { return PAL_Ok; }
    virtual void ReadInfo(NBRE_ImageInfo &fm, NBRE_ImageInfo &originalInfo)const
    {
        mImages[0]->GetInfo(originalInfo);
		fm = originalInfo;
    }
protected:
    NBRE_Image** mImages;
    uint8 mFaceCount;
    uint8 mMipmapCount;
};

ImmediateImage::ImmediateImage(NBRE_Image** image, uint8 faceCount, uint8 mipmapCount)
:mImages(image), mFaceCount(faceCount), mMipmapCount(mipmapCount)
{
}

ImmediateImage::~ImmediateImage()
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
ImmediateImage::Release()
{
    NBRE_DELETE this;
}

NBRE_GLTexture::NBRE_GLTexture(NBRE_IRenderPal *renderPal, const NBRE_ImageInfo *info, TextureType type, nb_boolean isUsingMipmap, const NBRE_String& name)
:NBRE_Texture(renderPal, NULL, type, info, isUsingMipmap, name), mTextureID(0)
{
}

NBRE_GLTexture::NBRE_GLTexture(NBRE_IRenderPal *renderPal, NBRE_Image** image, uint8 faceCount, uint8 numMipmaps, TextureType type, nb_boolean isUsingMipmap, const NBRE_String&name)
:NBRE_Texture(renderPal, NBRE_NEW ImmediateImage(image, faceCount, numMipmaps), type, NULL, isUsingMipmap, name), mTextureID(0)
{
}

NBRE_GLTexture::NBRE_GLTexture(NBRE_IRenderPal *renderPal, NBRE_ITextureImage *textureImage, TextureType type, nb_boolean isUsingMipmap, const NBRE_String& name)
:NBRE_Texture(renderPal, textureImage, type, NULL, isUsingMipmap, name), mTextureID(0)
{
}

NBRE_GLTexture::~NBRE_GLTexture()
{
    Unload();
}

GLenum
NBRE_GLTexture::GetTextureTarget(void) const
{
    switch(mType)
    {
    case TT_1D:
        return GL_TEXTURE_1D;
    case TT_2D:
        return GL_TEXTURE_2D;
    case TT_CUBE_MAP:
        return GL_TEXTURE_CUBE_MAP;
    case TT_2D_RENDERTARGET:
        return GL_TEXTURE_2D;
    case TT_CUBE_MAP_RENDERTARTE:
        return GL_TEXTURE_CUBE_MAP;
    case TT_3D:
    default:
        nbre_assert(FALSE);
        return 0;
    };
}

GLenum
NBRE_GLTexture::GetPixelType() const
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
NBRE_GLTexture::GetSrcPixelFormat() const
{
    switch(mTextureInfo.mFormat)
    {
    case NBRE_PF_R8G8B8A8:
    case NBRE_PF_A4R4G4B4:
    case NBRE_PF_R4G4B4A4:
        return GL_RGBA;
    case NBRE_PF_B8G8R8A8:
        return GL_BGRA;
    case NBRE_PF_R8G8B8:
    case NBRE_PF_R5G6B5:
        return GL_RGB;
    case NBRE_PF_B8G8R8:
        return GL_BGR;
    case NBRE_PF_A8:
        return GL_ALPHA;
    case NBRE_PF_NOTSUPPORT:
    default:
    //    nbre_assert(FALSE);
    //    return 0;
    //};
        nbre_assert(FALSE);
    return 0;
    };
}

GLenum
NBRE_GLTexture::GetTexPixelFormat() const
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

static uint8
GetPixelSize(NBRE_PixelFormat format)
{
    switch(format)
    {
    case NBRE_PF_R8G8B8A8:
         return 4;
    case NBRE_PF_R8G8B8:
         return 3;
    case NBRE_PF_A4R4G4B4:
         return 2;
    case NBRE_PF_A8:
         return 1;
    case NBRE_PF_NOTSUPPORT:
    default:
        nbre_assert(FALSE);
        return 0;
    };
}

PAL_Error
NBRE_GLTexture::Create2DTexture()
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

    // Do not check PO2 texture size here for GL renderPal.
    if (mTextureInfo.mFormat == NBRE_PF_NOTSUPPORT)
    {
        return PAL_ErrUnsupported;
    }

    glGenTextures(1, &mTextureID);
    if (mTextureID == 0)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLTexture::Create2DTexture: GenTextures failed! glError = %d", glGetError());
        return PAL_Failed;
    }
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    uint8 count = 1;
    if(mTextureImage != NULL)
    {
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
            glTexImage2D(GL_TEXTURE_2D, i, GetTexPixelFormat(), pImage->Width(), pImage->Height(), 0, GetSrcPixelFormat(), GetPixelType(), pImage->GetImageData());
        }
    }
    else
    {
        if (IsUsingMipmap())
        {
            count = GetLevels(NBRE_Math::Max<uint32>(mTextureInfo.mWidth, mTextureInfo.mHeight));
        }
        uint32 size = mTextureInfo.mWidth*mTextureInfo.mHeight*GetPixelSize(mTextureInfo.mFormat);
        uint8 *buffer = NBRE_NEW uint8[size];
        nsl_memset(buffer, 0, size);
        for (uint8 i = 0; i < count; ++i)
        {
            uint32 width =  mTextureInfo.mWidth>>i;
            uint32 height =  mTextureInfo.mHeight>>i;
            width = (width==0)?1:width;
            height = (height==0)?1:height;
            glTexImage2D(GL_TEXTURE_2D, i, GetTexPixelFormat(), width?width:1, height?height:1, 0, GetSrcPixelFormat(), GetPixelType(), buffer);
        }
        NBRE_DELETE_ARRAY buffer;
    }
    if (IsUsingMipmap() && mTextureImage && mTextureImage->GetMipmapCount() == 1)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    GLenum error = glGetError();
    if (error == GL_NO_ERROR)
    {
        return PAL_Ok;
    }
    else
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLTexture::Create2DTexture failed! glError = %d", error);
        return PAL_Failed;
    }
}

PAL_Error
NBRE_GLTexture::GenerateInternalTextureImpl()
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

PAL_Error
NBRE_GLTexture::CreateCubeTexture()
{
    if (mTextureImage == NULL)
    {
        return PAL_Failed;
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
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLTexture::CreateCubeTexture: GenTextures failed! glError = %d", glGetError());
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
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+j, i, GetTexPixelFormat(), pImage->Width(), pImage->Height(), 0, GetSrcPixelFormat(), GetPixelType(), pImage->GetImageData());
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
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLTexture::Create2DTexture failed! glError = %d", error);
        return PAL_Failed;
    }
}

PAL_Error
NBRE_GLTexture::Create2DRenderTexture()
{
    if (!NBRE_Math::IsPowerOfTwo(mTextureInfo.mHeight) || !NBRE_Math::IsPowerOfTwo(mTextureInfo.mWidth))
    {
        return PAL_ErrUnsupported;
    }

    glGenTextures(1, &mTextureID);
    if (mTextureID == 0)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLTexture::Create2DRenderTexture: GenTextures failed! glError = %d", glGetError());
        return PAL_Failed;
    }
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (IsUsingMipmap())
    {
        return PAL_ErrUnsupported;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GetTexPixelFormat(), mTextureInfo.mWidth, mTextureInfo.mHeight, 0, GetSrcPixelFormat(), GetPixelType(), NULL);

    GLenum error = glGetError();
    if (error == GL_NO_ERROR)
    {
        return PAL_Ok;
    }
    else
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLTexture::Create2DRenderTexture failed! glError = %d", error);
        return PAL_Failed;
    }
}

PAL_Error
NBRE_GLTexture::CreateCubeRenderTexture()
{
    return PAL_ErrUnsupported;
}

void
NBRE_GLTexture::FreeInternalTextureImpl()
{
    glDeleteTextures(1, &mTextureID);
    mTextureID  = 0;
}

void
NBRE_GLTexture::UpdateRegion(int32 x, int32 y, int32 level, NBRE_Image* image)
{
    nbre_assert(image != NULL);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    GLenum imgFormat = GL_ALPHA;
    switch(image->Format())
    {
    case NBRE_PF_R8G8B8A8:
        imgFormat = GL_RGBA;
        break;
    case NBRE_PF_R8G8B8:
        imgFormat = GL_RGB;
        break;
    case NBRE_PF_A8:
        imgFormat = GL_ALPHA;
        break;
    default:
        nbre_assert(0);
        break;
    }
    glTexSubImage2D(GL_TEXTURE_2D, level, x, y, image->Width(),
        image->Height(), imgFormat, GL_UNSIGNED_BYTE, image->GetImageData());
    glBindTexture(GL_TEXTURE_2D, 0);
}

