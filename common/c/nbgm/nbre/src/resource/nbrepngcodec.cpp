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
#include "nbrepngcodec.h"
#include "palstdlib.h"
#include "png.h"
#include "nbreimage.h"
#include "nbrecommon.h"
#include "nbrefilestream.h"
#include "nbrelog.h"

#pragma warning(disable: 4611)


static void
png_read_data_callback(png_structp png_ptr, png_bytep data, png_size_t length)
{
    uint32 bytesRead;
    
    if (png_ptr == NULL)
    {
        return;
    }

    NBRE_IOStream* is = static_cast<NBRE_IOStream*>(png_get_io_ptr(png_ptr));
    PAL_Error err = is->Read(data, (uint32)length, &bytesRead);

    if (err != PAL_Ok || bytesRead != length)
    {
        png_error(png_ptr, "Read Error");
    }
}

static nb_boolean
ReadPNGInfo(NBRE_IOStream& infile, uint32& w, uint32& h, NBRE_PixelFormat& format)
{
    png_structp         png_ptr;
    png_infop           info_ptr;
    unsigned char       sig[8] = {0};
    int                 bit_depth;
    int                 color_type;
    png_uint_32         width;
    png_uint_32         height;
    uint32              bytesRead;

    PAL_Error err = infile.Read(sig, 8, &bytesRead);
    if (err != PAL_Ok)
    {
        return FALSE;
    }

    if ((bytesRead != 8) || (png_sig_cmp(sig, 0, 8)))
    {
       return FALSE;
    }
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
       return FALSE;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
       png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
       return FALSE; /* out of memory */
    }
    //comment to disable compile warning
    if (setjmp(png_jmpbuf(png_ptr)))
    {
       png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
       w = 1;
       h = 1;
       format = NBRE_PF_R8G8B8A8;
       return TRUE;
    }
    png_set_read_fn(png_ptr, &infile, png_read_data_callback);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
         &color_type, NULL, NULL, NULL);
    /* Set up some transforms. */
    if (bit_depth > 8)
    {
       png_set_strip_16(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
       png_set_gray_to_rgb(png_ptr);
    }
    png_set_expand(png_ptr);
    /* Update the png info struct.*/
    png_read_update_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
        &color_type, NULL, NULL, NULL);
    if (bit_depth != 8)
    {
        // All color should be converted to 8 bits grayscale/RGB/RGBA now
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return FALSE;
    }
    switch(color_type)
    {
    case PNG_COLOR_TYPE_RGBA:
        format = NBRE_PF_R8G8B8A8;
        break;
    case PNG_COLOR_TYPE_GRAY:
        format = NBRE_PF_A8;
        break;
    case PNG_COLOR_TYPE_RGB:
    default:
        format = NBRE_PF_R8G8B8;
        break;
    }
    w = width;
    h = height;
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return TRUE;
}


static nb_boolean
LoadPNG(NBRE_IOStream& infile, NBRE_Image** image)
{
    png_structp         png_ptr;                /* internally used by libpng */ 
    png_infop           info_ptr;               /* user requested transforms */ 
    uint8*              image_data = NULL;      /* raw png image data */ 
    unsigned char       sig[8] = {0};           /* PNG signature array */
    int                 bit_depth; 
    int                 color_type;
    unsigned int        rowbytes;                 /* raw bytes at row n in image */ 
    uint32              pixelSize;
    uint32              i;
    uint32              bytesRead;

    NBRE_ImageInfo imageInfo(1, 1, NBRE_PF_R8G8B8A8);
    NBRE_Image* result = NULL;

    PAL_Error err = infile.Read(sig, 8, &bytesRead);  
    if ((err != PAL_Ok) || (bytesRead != 8) || (png_sig_cmp(sig, 0, 8)))
    {
       return FALSE; 
    } 

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL); 
    if (!png_ptr) 
    { 
       return FALSE;    /* out of memory */ 
    } 
   
    info_ptr = png_create_info_struct(png_ptr); 
    if (!info_ptr) 
    { 
       png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL); 
       return FALSE;    /* out of memory */ 
    } 

    //comment to disable compile warning
    if (setjmp(png_jmpbuf(png_ptr))) 
    { 
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        if(image_data != NULL)
        {
            NBRE_DELETE_ARRAY image_data;
        }
        if(result == NULL)
        {
            imageInfo.mWidth = imageInfo.mWidth>1?imageInfo.mWidth:1;
            imageInfo.mHeight = imageInfo.mHeight>1?imageInfo.mHeight:1;
            result = NBRE_NEW NBRE_Image(imageInfo);
        }
        *image = result;
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE PNG decode failed, use default image instead!");
        return TRUE;
    }

    png_set_read_fn(png_ptr, &infile, png_read_data_callback);
    png_set_sig_bytes(png_ptr, 8); 
    png_read_info(png_ptr, info_ptr); 
  
    png_get_IHDR(png_ptr, info_ptr, (png_uint_32*)&imageInfo.mWidth, (png_uint_32*)&imageInfo.mHeight, &bit_depth,
         &color_type, NULL, NULL, NULL); 
  
     
    /* Set up some transforms. */ 
    if (bit_depth > 8) 
    { 
       png_set_strip_16(png_ptr); 
    } 
    if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) 
    { 
       png_set_gray_to_rgb(png_ptr);
    }

    png_set_expand(png_ptr);
     
    /* Update the png info struct.*/ 
    png_read_update_info(png_ptr, info_ptr);

    png_get_IHDR(png_ptr, info_ptr, (png_uint_32*)&imageInfo.mWidth, (png_uint_32*)&imageInfo.mHeight, &bit_depth,
        &color_type, NULL, NULL, NULL); 
  
    if (bit_depth != 8)
    {
        // All color should be converted to 8 bits grayscale/RGB/RGBA now
        // If not, return error
        return FALSE;
    }

    switch(color_type)
    {
    case PNG_COLOR_TYPE_RGBA:
        pixelSize = 4;
        imageInfo.mFormat = NBRE_PF_R8G8B8A8;
        break;
    case PNG_COLOR_TYPE_GRAY:
        pixelSize = 1;
        imageInfo.mFormat = NBRE_PF_A8;
        break;
    case PNG_COLOR_TYPE_RGB:
    default:
        pixelSize = 3;
        imageInfo.mFormat = NBRE_PF_R8G8B8;
        break;
    }
    
    result = NBRE_NEW NBRE_Image(imageInfo);
    image_data = result->GetImageData();
    if (image_data == NULL) 
    { 
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        NBRE_DELETE_ARRAY image_data;
        return FALSE;
    }

     /* now we can go ahead and just read the whole image */ 
    rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    png_bytep row_pointer = NBRE_NEW uint8[rowbytes];
    if (row_pointer == NULL)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        NBRE_DELETE_ARRAY(image_data);
        return FALSE;
    }

    for (i = 0;  i < imageInfo.mHeight;  ++i)
    {
        png_read_row(png_ptr, row_pointer, NULL);
        nsl_memcpy(image_data + (imageInfo.mHeight - 1 - i) * imageInfo.mWidth * pixelSize, row_pointer, imageInfo.mWidth * pixelSize);
    }

    NBRE_DELETE_ARRAY row_pointer;
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL); 


    *image = result;
    return TRUE;
}

NBRE_PngCodec::NBRE_PngCodec()
{
}

NBRE_PngCodec::~NBRE_PngCodec()
{
}

NBRE_Image*
NBRE_PngCodec::Image(NBRE_IOStream& is)
{
    NBRE_Image* result = NULL;
    
    LoadPNG(is, &result);
    NBRE_DebugLog(PAL_LogSeverityInfo, "NBRE_PngCodec::LoadPNG, From memory, result=%p", result);

    return result;
}

void NBRE_PngCodec::ReadInfo(NBRE_IOStream& is, NBRE_ImageInfo &fm, NBRE_ImageInfo &originalInfo)const
{
    nb_boolean loaded = FALSE;
    originalInfo.mWidth = 0;
    originalInfo.mHeight = 0;
    originalInfo.mFormat = NBRE_PF_NOTSUPPORT;

    uint32 pos = is.GetPos();
    loaded = ReadPNGInfo(is, originalInfo.mWidth, originalInfo.mHeight, originalInfo.mFormat);
    is.Seek(PFSO_Start, pos);
    fm = originalInfo;
}
