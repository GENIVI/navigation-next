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
#include "nbrepngtextureimage.h"
#include "nbrepngcodec.h"
#include "nbrejpegcodec.h"
#include "nbreimage.h"
#include "nbrecommon.h"
#include "nbrefilestream.h"

NBRE_PngTextureImage::NBRE_PngTextureImage(PAL_Instance& pal, const char* path, uint32 offset, nb_boolean useLowQuality)
    :mImageCodec(NULL)
    ,mImage(NULL)
    ,mPalInstance(&pal)
    ,mPath(path)
    ,mFileOffset(offset)
    ,mInStream(NULL)
    ,mUseLowQuality(useLowQuality)
{
}

NBRE_PngTextureImage::NBRE_PngTextureImage(PAL_Instance& /*pal*/, NBRE_IOStream* io, uint32 offset, nb_boolean useLowQuality)
    :mImageCodec(NULL)
    ,mImage(NULL)
    ,mPalInstance(NULL)
    ,mFileOffset(offset)
    ,mInStream(io)
    ,mUseLowQuality(useLowQuality)
{
}

NBRE_PngTextureImage::NBRE_PngTextureImage(const NBRE_ImageInfo& imageInfo)
    :mImageCodec(NULL)
    ,mImage(NULL)
    ,mPalInstance(NULL)
    ,mFileOffset(0)
    ,mInStream(NULL)
    ,mUseLowQuality(FALSE)
{
    mImageInfo = imageInfo;
}

NBRE_PngTextureImage::~NBRE_PngTextureImage()
{
    NBRE_DELETE mImageCodec;
    NBRE_DELETE mImage;
    NBRE_DELETE mInStream;
}

const NBRE_Image*
NBRE_PngTextureImage::GetImage(uint8 face, uint8 mipmapLevel)const
{
    nbre_assert((face == 0) && (mipmapLevel == 0));
    return mImage;
}

void
NBRE_PngTextureImage::Release()
{
    NBRE_DELETE this;
}

void
NBRE_PngTextureImage::CreateDecodc() const
{
    // Open the file stream if neccesary.
    if(mInStream == NULL && !mPath.empty())
    {
        mInStream = NBRE_NEW NBRE_FileStream(mPalInstance, mPath.c_str(), 2048);
        mInStream->Seek(PFSO_Start, static_cast<int32>(mFileOffset));
    }

    if(mImageCodec != NULL)
    {
        return;
    }

    if(mInStream != NULL)
    {
        uint8 fileHeader[4];
        uint32 readBytes;
        mInStream->Read(fileHeader, 4, &readBytes);
        if(fileHeader[0] == 0x89 && fileHeader[1] == 0x50 && fileHeader[2] == 0x4E && fileHeader[3] == 0x47)
        {
            mImageCodec = NBRE_NEW NBRE_PngCodec();
        }
        else if(fileHeader[0] == 0xFF && fileHeader[1] == 0xD8 && fileHeader[2] == 0xFF)
        {
            mImageCodec = NBRE_NEW NBRE_JpegCodec();
        }
        mInStream->Seek(PFSO_Start, static_cast<int32>(mFileOffset));
    }
}

PAL_Error
NBRE_PngTextureImage::Load()
{
    if (mImage == NULL)
    {
        CreateDecodc();
        if(mImageCodec != NULL)
        {
            mImage = mImageCodec->Image(*mInStream);
            // Need to close the file stream immediately after operation done.
            if(mInStream != NULL && !mPath.empty())
            {
                NBRE_DELETE mInStream;
                mInStream = NULL;
            }
        }
        else
        {
            //Create a empty image if there is no image codec
            mImage = NBRE_NEW NBRE_Image(mImageInfo);
        }
        if (mImage == NULL)
        {
            return PAL_Failed;
        }
    }

    NBRE_ImageInfo info;
    mImage->GetInfo(info);

    if (mUseLowQuality)
    {
        if (info.mFormat == NBRE_PF_R8G8B8A8)
        {
           NBRE_Image* temp = mImage;
           mImage = temp->ConvertToFormat(NBRE_PF_R4G4B4A4);
           NBRE_DELETE temp;
        }
        else if (info.mFormat == NBRE_PF_R8G8B8)
        {
            NBRE_Image* temp = mImage;
            mImage = temp->ConvertToFormat(NBRE_PF_R5G6B5);
            NBRE_DELETE temp;
        }
    }

    return PAL_Ok;
}

void
NBRE_PngTextureImage::ReadInfo(NBRE_ImageInfo &fm,  NBRE_ImageInfo &originalInfo)const
{
    CreateDecodc();
    if(mImageCodec != NULL)
    {
        mImageCodec->ReadInfo(*mInStream, fm, originalInfo);
        // Need to close the file stream immediately after operation done.
        if(mInStream != NULL && !mPath.empty())
        {
            NBRE_DELETE mInStream;
            mInStream = NULL;
        }
    }

    if (mUseLowQuality)
    {
        switch(fm.mFormat)
        {
        case NBRE_PF_R8G8B8A8:
            fm.mFormat = NBRE_PF_R4G4B4A4;
            break;
        case NBRE_PF_R8G8B8:
            fm.mFormat = NBRE_PF_R5G6B5;
            break;
        default:
            break;
        }
    }
}
