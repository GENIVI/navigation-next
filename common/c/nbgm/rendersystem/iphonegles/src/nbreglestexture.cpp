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
#include "nbreglestexture.h"
#include "nbreitextureimage.h"
#include "nbreglesrenderpal.h"
#include "palstdlib.h"
#include "nbrelog.h"
#include "nbreglesrendertexture.h"
#include "nbreglescommon.h"

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
    case TT_2D:
        return GL_TEXTURE_2D;
    case TT_CUBE_MAP:
        return GL_TEXTURE_CUBE_MAP;
    case TT_2D_RENDERTARGET:
        return GL_TEXTURE_2D;
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
NBRE_GLTexture::GetSrcPixelFormat(NBRE_PixelFormat format) const
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
        return GL_ALPHA;
    case NBRE_PF_B8G8R8A8:
    case NBRE_PF_B8G8R8:
    case NBRE_PF_NOTSUPPORT:
    default:
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

    if (!NBRE_Math::IsPowerOfTwo(mTextureInfo.mHeight) || !NBRE_Math::IsPowerOfTwo(mTextureInfo.mWidth) || mTextureInfo.mFormat == NBRE_PF_NOTSUPPORT)
    {
        return PAL_ErrUnsupported;
    }
    while(glGetError());

    glGenTextures(1, &mTextureID);
    if (mTextureID == 0)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor,
                      "NBRE_GLTexture::Create2DTexture: glGenTextures failed! texture name=%s glError = %d",
                      mName.c_str(),
                      glGetError());
        return PAL_Failed;
    }
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    GL_CHECK_ERROR;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GL_CHECK_ERROR;

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
            glTexImage2D(GL_TEXTURE_2D,
                         i,
                         GetTexPixelFormat(),
                         pImage->Width(),
                         pImage->Height(),
                         0,
                         GetSrcPixelFormat(mTextureInfo.mFormat),
                         GetPixelType(),
                         pImage->GetImageData());
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
            uint32 fac = 1<<i;
            uint32 width =  mTextureInfo.mWidth/fac;
            uint32 height =  mTextureInfo.mHeight/fac;
            glTexImage2D(GL_TEXTURE_2D,
                         i,
                         GetTexPixelFormat(),
                         width?width:1,
                         height?height:1,
                         0,
                         GetSrcPixelFormat(mTextureInfo.mFormat),
                         GetPixelType(),
                         buffer);
        }
        NBRE_DELETE_ARRAY buffer;
    }

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        FreeInternalTextureImpl();
        NBRE_DebugLog(PAL_LogSeverityMajor,
                      "NBRE_GLTexture::Create2DTexture, glTexImage2D failed! texture name=%s, format=0x%04x, type=0x%04x, glError = %d, line=%d",
                      mName.c_str(),
                      GetSrcPixelFormat(mTextureInfo.mFormat),
                      GetPixelType(),
                      error,
                      __LINE__);
        if (error == GL_INVALID_OPERATION)
        {
            if(GetSrcPixelFormat(mTextureInfo.mFormat) != GetTexPixelFormat())
            {
                NBRE_DebugLog(PAL_LogSeverityMajor,
                              "NBRE_GLTexture::Create2DTexture, internalformat and format are not the same");
            }
            
            if( GetPixelType()==GL_UNSIGNED_SHORT_5_6_5 && GetSrcPixelFormat(mTextureInfo.mFormat)!=GL_RGB)
            {
                
                NBRE_DebugLog(PAL_LogSeverityMajor,
                              "NBRE_GLTexture::Create2DTexture, type is GL_UNSIGNED_SHORT_5_6_5 and format is not GL_RGB");
            }
            if((GetPixelType()==GL_UNSIGNED_SHORT_4_4_4_4 || GetPixelType()==GL_UNSIGNED_SHORT_5_5_5_1 )
               && GetSrcPixelFormat(mTextureInfo.mFormat)!=GL_RGBA)
            {
                NBRE_DebugLog(PAL_LogSeverityMajor,
                              "NBRE_GLTexture::Create2DTexture, type is one of GL_UNSIGNED_SHORT_4_4_4_4, or GL_UNSIGNED_SHORT_5_5_5_1 and format is not GL_RGBA");
            }
        }
        
        return PAL_Failed;
    }

    if (IsUsingMipmap() && mTextureImage && mTextureImage->GetMipmapCount() == 1)
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
        NBRE_DebugLog(PAL_LogSeverityMajor,
                      "NBRE_GLTexture::Create2DTexture, glGenerateMipmap failed! texture name=%s, glError = %d, line=%d",
                      mName.c_str(),
                      error,
                      __LINE__);
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
    
    if(!mRenderPal->GetCapabilities().GetSupportCubeMap())
    {
        NBRE_DebugLog(PAL_LogSeverityMajor,
                      "NBRE_GLTexture::CreateCubeTexture, current render system doesn't support cube maping!");
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
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLTexture::CreateCubeTexture: GenTextures failed! glError = %d, line=%u", glGetError(), __LINE__);
        return PAL_Failed;
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID);
    GL_CHECK_ERROR;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GL_CHECK_ERROR;

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
            GL_CHECK_ERROR;
        }

    }

    GL_CHECK_ERROR;
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
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLTexture::CreateCubeTexture failed! glError = %d, line=%u", error, __LINE__);
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
    GL_CHECK_ERROR;
    if (mTextureID == 0)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLTexture::Create2DRenderTexture: GenTextures failed! glError = %d", glGetError());
        return PAL_Failed;
    }
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    GL_CHECK_ERROR;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GL_CHECK_ERROR;

    if (IsUsingMipmap())
    {
        return PAL_ErrUnsupported;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GetTexPixelFormat(), mTextureInfo.mWidth, mTextureInfo.mHeight, 0, GetSrcPixelFormat(mTextureInfo.mFormat), GetPixelType(), NULL);

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
    if(mTextureID != 0)
    {
        glDeleteTextures(1, &mTextureID);
        GL_CHECK_ERROR;
    }
    mTextureID  = 0;
}

void
NBRE_GLTexture::UpdateRegion(int32 x, int32 y, int32 level, NBRE_Image* image)
{
    nbre_assert(image != NULL);
    nbre_assert(mTextureID!=0);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    GL_CHECK_ERROR;
    GLenum imgFormat = GetSrcPixelFormat(image->Format());
    glTexSubImage2D(GL_TEXTURE_2D, level, x, y, image->Width(),
        image->Height(), imgFormat, GL_UNSIGNED_BYTE, image->GetImageData());

	GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor,
                      "NBRE_GLTexture::UpdateRegion, glTexSubImage2D failed");
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    GL_CHECK_ERROR;
}

