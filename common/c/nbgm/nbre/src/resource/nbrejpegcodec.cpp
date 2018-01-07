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
#include "nbrejpegcodec.h"
#include "palstdlib.h"
#include "turbojpeg.h"
#include "nbreimage.h"
#include "nbrecommon.h"
#include "nbrefilestream.h"
#include "nbrelog.h"

static nb_boolean
ReadJpegInfo(NBRE_IOStream& infile, NBRE_ImageInfo& imageInfo)
{
    uint32 bufferSize = infile.GetSize();
    uint8* buffer = (uint8*)nsl_malloc(bufferSize);
    uint32 readBytes = 0;
    int jpegSubsamp = 0;
    infile.Read(buffer, bufferSize, &readBytes);

    tjhandle chandle=tjInitDecompress();
    tjDecompressHeader2(chandle, buffer, bufferSize, (int*)&imageInfo.mWidth, (int*)&imageInfo.mHeight, (int*)&jpegSubsamp);
    tjDestroy(chandle);
    nsl_free(buffer);
    return TRUE;
}

static nb_boolean
LoadJpeg(NBRE_IOStream& infile, NBRE_Image** image)
{
    NBRE_Image* result = NULL;
    NBRE_ImageInfo imageInfo(1, 1, NBRE_PF_R8G8B8);
    
    uint32 bufferSize = infile.GetSize();
    uint8* buffer = (uint8*)nsl_malloc(bufferSize);
    uint32 readBytes = 0;
     int jpegSubsamp = 0;
    infile.Read(buffer, bufferSize, &readBytes);

    tjhandle chandle = tjInitDecompress();
    tjDecompressHeader2(chandle, buffer, bufferSize, (int*)&imageInfo.mWidth, (int*)&imageInfo.mHeight, (int*)&jpegSubsamp);

    result = NBRE_NEW NBRE_Image(imageInfo);

    tjDecompress2(chandle, buffer, NBRE_Image::GetImageDataSize(imageInfo), result->GetImageData(), imageInfo.mWidth, 0, imageInfo.mHeight, TJPF_RGB, 0);
    tjDestroy(chandle);

    result->FlipY();

    *image = result;
    nsl_free(buffer);
    return TRUE;
}

NBRE_JpegCodec::NBRE_JpegCodec()
{
}

NBRE_JpegCodec::~NBRE_JpegCodec()
{
}

NBRE_Image*
NBRE_JpegCodec::Image(NBRE_IOStream& is)
{
    NBRE_Image* result = NULL;
    LoadJpeg(is, &result);
    return result;
}

void 
NBRE_JpegCodec::ReadInfo(NBRE_IOStream& is, NBRE_ImageInfo &fm, NBRE_ImageInfo &originalInfo) const
{
    originalInfo.mWidth = 1;
    originalInfo.mHeight = 1;
    originalInfo.mFormat = NBRE_PF_R8G8B8A8;

    uint32 pos = is.GetPos();
    ReadJpegInfo(is, originalInfo);
    is.Seek(PFSO_Start, pos);

    fm = originalInfo;
}
