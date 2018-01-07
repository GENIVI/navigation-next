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

 @file     nbpng.cpp
 */
/*
 See description in header file.

 (C) Copyright 2011 by TeleCommunication Systems

 The information contained herein is confidential, proprietary
 to Networks In Motion, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of Networks In Motion is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
#undef PNG_NO_SETJMP
#undef PNG_SETJMP_NOT_SUPPORTED
#undef PNG_NO_SETJMP_SUPPORTED
#include "NBPng.h"
#include "png.h"
#include "pngconf.h"
#include "palstdlib.h"
#include "datastream.h"
namespace nbcommon
{
class PNG_Decoder
{
public:
    PNG_Decoder() {}
    virtual ~PNG_Decoder() {}
    
    /*Decode png from datastream*/
    virtual NB_Error Decode(nbcommon::DataStreamPtr dataStream  /*the png datastream */
                            ) = 0;
    /*Return the image data total lenght*/
    virtual size_t GetImageDataLength() =0;
    /*Return the png image width*/
    virtual uint32 GetWidth() = 0;
    /*Return the png image height*/
    virtual uint32  GetHeight() = 0;
    /*Return the png image data buffer*/
    virtual byte * GetImageData()= 0;
    /*Return the png image data with PNG_Data structure*/
    virtual PNG_Data * GetPNGData() = 0;
};

// local class
class PNG_DecoderImplement: public PNG_Decoder
{
public:
    PNG_DecoderImplement();
    virtual ~PNG_DecoderImplement();

    /*Decode png from datastream*/
    virtual NB_Error Decode(nbcommon::DataStreamPtr dataStream  /*the png datastream */
                            );
    /*Return the image data total lenght*/
    virtual size_t GetImageDataLength();
    /*Return the png image width*/
    virtual uint32 GetWidth();
    /*Return the png image height*/
    virtual uint32 GetHeight();
    /*Return the png image data buffer*/
    virtual byte * GetImageData();
    /*Return the png image data with PNG_Data structure*/
    virtual PNG_Data * GetPNGData();

    /* png read data callback*/
    static void png_read_data_callback(png_structp png_ptr, png_bytep data, png_size_t length);

protected:
    NB_Error LoadPNG32();
    size_t ReadBuffer(png_bytep data,png_size_t length);
    NB_Error GetStreamData(uint32 offset, uint32 length, byte * buf);
    NB_Error GetStreamData(uint32 length, byte * buf);

private:
    PNG_Data* m_pngData;
    nbcommon::DataStreamPtr dataStream;
    uint32 currentRead; /* The offset of stream have been readed*/
};

// Local class function

PNG_DecoderImplement::PNG_DecoderImplement()
 : m_pngData(NULL),
   dataStream(),
   currentRead(0)
{
}

PNG_DecoderImplement::~PNG_DecoderImplement()
{
    // The m_pngData free when ::DestroyPNG(PNG_Data *) be called
}
    
size_t PNG_DecoderImplement::GetImageDataLength()
{
    if (m_pngData != NULL)
    {
        return m_pngData->m_length;
    }
    else
    {
        return 0;
    }
}
    
byte*
PNG_DecoderImplement::GetImageData()
{
    if (m_pngData != NULL)
    {
        return m_pngData->m_data;
    }
    else
    {
        return NULL;
    }
}

PNG_Data*
PNG_DecoderImplement::GetPNGData()
{
    return this->m_pngData;
}

uint32
PNG_DecoderImplement::GetWidth()
{
    return this->m_pngData->m_width;
}

uint32
PNG_DecoderImplement::GetHeight()
{
    return this->m_pngData->m_height;
}

NB_Error
PNG_DecoderImplement::Decode(nbcommon::DataStreamPtr dataStream)
{
    this->dataStream = dataStream;
    this->currentRead = 0;
    return LoadPNG32();
}
    
// @todo: Explain why this warning is disabled (or fix it!)
#ifdef _WIN32
#pragma warning( disable: 4611 )
#endif
    
NB_Error PNG_DecoderImplement::LoadPNG32()
{
    png_structp     png_ptr;            /* internally used by libpng */
    png_infop       info_ptr;           /* user requested transforms */
    png_bytepp       row_pointers = NULL;

    char            *image_data = NULL; /* raw png image data */
    unsigned char   sig[8];             /* PNG signature array */

    int             bit_depth;
    int             color_type;

    png_uint_32     width;              /* PNG image width in pixels */
    png_uint_32     height;             /* PNG image height in pixels */
    unsigned int    rowbytes;           /* raw bytes at row n in image */
    unsigned long   i;
    uint32          length;

    if(GetStreamData(sizeof(sig), sig)!= NE_OK)
    {
        return NE_INVAL;
    }

    if (png_sig_cmp(sig, 0, 8))
    {
        return NE_INVAL;
    }

    /*
     * Set up the PNG structs
     */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        return NE_NOMEM;    /* out of memory */
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
        return NE_NOMEM;    /* out of memory */
    }

    /*
     * block to handle libpng errors,
     * then check whether the PNG file had a bKGD chunk
     */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NE_INVAL;
    }

    /*
     * takes our file stream pointer (infile) and
     * stores it in the png_ptr struct for later use.
     */

    png_set_read_fn(png_ptr, this, png_read_data_callback);

    /*
     * lets libpng know that we already checked the 8
     * signature bytes, so it should not expect to find
     * them at the current file pointer location
     */
    png_set_sig_bytes(png_ptr, 8);

    /* read all the info up to the image data  */
    png_read_info(png_ptr, info_ptr);

    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
                 &color_type, NULL, NULL, NULL);


    /* Set up some transforms. */
    if (color_type & PNG_COLOR_MASK_ALPHA)
    {
        png_set_strip_alpha(png_ptr);
    }
    if (bit_depth > 8)
    {
        png_set_strip_16(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        png_set_gray_to_rgb(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(png_ptr);
    }

    /* Update the png info struct.*/
    png_read_update_info(png_ptr, info_ptr);

    /* Rowsize in bytes. */
    rowbytes = (unsigned int)png_get_rowbytes(png_ptr, info_ptr);
    bit_depth = rowbytes/width;

    length = rowbytes * height;

     /* Allocate the image_data buffer. */
    m_pngData = (PNG_Data *) nsl_malloc(sizeof(PNG_Data) + length);
    if (m_pngData == NULL)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NE_NOMEM;
    }

    m_pngData->m_length = length / sizeof(m_pngData->m_data[0]);
    m_pngData->m_width =  width;
    m_pngData->m_height = height;
    m_pngData->m_bitDepth = bit_depth;

    image_data = (char *) m_pngData->m_data;

    if ((row_pointers = (png_bytepp)nsl_malloc(height*sizeof(png_bytep))) == NULL)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        nsl_free(m_pngData);
        m_pngData = NULL;
        return NE_NOMEM;
    }

    /* set the individual row_pointers to point at the correct offsets */
    for (i = 0;  i < height;  ++i)
    {
        row_pointers[i] = (png_byte*)image_data + i*rowbytes;
    }

    /* now we can go ahead and just read the whole image */
    png_read_image(png_ptr, row_pointers);

    /* Clean up. */
    nsl_free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    return NE_OK;
}

// @todo: Explain why this warning is disabled (or fix it!)
#ifdef _WIN32
#pragma warning( default: 4200 )
#endif
    
NB_Error
PNG_DecoderImplement::GetStreamData(uint32 length, byte * buf)
{
    NB_Error error = NE_OK;
    error = GetStreamData(this->currentRead, length, buf);
    if (error == NE_OK)
    {
        currentRead += length;           /* Record offset of stream have been read*/
    }
    return error;
}
    
NB_Error
PNG_DecoderImplement::GetStreamData(uint32 offset, uint32 length, byte * buf)
{
    NB_Error error;
    size_t readlength = 0;
    if (!this->dataStream)
    {
        return NE_NOTINIT;
    }

    while (readlength < length)
    {
        readlength += this->dataStream->GetData(buf, readlength + offset, length-readlength);
    }

    error = (readlength == length ? NE_OK : NE_INVAL);
    return error;

}

size_t
PNG_DecoderImplement::ReadBuffer(png_bytep data, png_size_t length)
{
    if (!this->dataStream)
    {
        return 0;
    }

    if (GetStreamData(length, data) != NE_OK)
    {
        return 0;
    }

    return length;
}

void
PNG_DecoderImplement::png_read_data_callback(png_structp png_ptr,
                                             png_bytep data,
                                             png_size_t length)
{
    PNG_DecoderImplement * pThis = NULL;
    png_size_t check;

    if (png_ptr == NULL)
    {
        return;
    }

    pThis = (PNG_DecoderImplement *)png_get_io_ptr(png_ptr);

    if (pThis == NULL)
    {
        return;
    }

    check = pThis->ReadBuffer(data, length);

    if (check != length)
    {
        png_error(png_ptr, "Read Error");
    }
}

}; // Namespace nbcommon


// public fucntion
using namespace nbcommon;
NB_Error DecodePNG(nbcommon::DataStreamPtr dataStream, PNG_Data** pngData)
{
    PNG_DecoderImplement decoder;

    NB_Error error = decoder.Decode(dataStream);
    if (error != NE_OK)
    {
        *pngData = NULL;
        return  error;
    }

    /* We use the decode.m_pngData directly
       so can not free the m_pngData in PNG_Decode::~PNG_Decoder
     */
    *pngData = decoder.GetPNGData();
    return NE_OK;
}

void
DestroyPNG(PNG_Data * pngData)
{
    nsl_free(pngData);
}

