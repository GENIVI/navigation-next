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

    @file nbimage.c
    @defgroup nbimage image
*/
/*
(C) Copyright 2011 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "nbimage.h"

/*! @{ */

/* definitions for PNG format */

typedef enum
{
    PNG_BLACK_AND_WHITE = 0,
    PNG_FULL_COLOR = 2,
    PNG_INDEXED_COLOR = 3,
    PNG_B_AND_W_WITH_ALPHA = 4,
    PNG_FULL_COLOR_WITH_ALPHA = 6
}PNG_COLOR_TYPES;

#define SIGANUTE_LENGHT     8
#define CHUNK_NAME_LENGHT   4
#define IHDR_CHUNK_LENGHT  13
#define CHUNK_TRASH_LENGHT   4

typedef struct
{
    uint32 Width;
    uint32 Height;
    byte BitDepth;
    byte ColorType;
    byte Compression;
    byte Filter;
    byte Interlace;
}IHDR_Chunk;

const char PNG_SIGNATURE[] = {137, 80, 78, 71, 13, 10, 26, 10};
const char IHDR_NAME[] = {'I', 'H', 'D', 'R'};


//  Local functions
NB_Error
IsPNGTransparent(void* data, int dataLen, nb_boolean *result);

/* See header file for description. */
NB_DEF NB_Error
NB_ImageHasTransparency(void* data,
                        int dataLen,
                        NB_ImageFormat format,
                        nb_boolean *result
                        )
{
    if ( NULL == data || NULL == result || dataLen <= 0 )
    {
        return NE_INVAL;
    }

    switch( format )
    {
        case NB_IF_PNG:
            return IsPNGTransparent(data, dataLen, result);

        case NB_IF_JPG:
            *result = FALSE;
            return NE_OK;

        case NB_IF_GIF:
            //TODO
            return NE_NOSUPPORT;

        case NB_IF_BMP:
            *result = FALSE;
            return NE_OK;

        case NB_IF_BMP_Deflate:
            *result = FALSE;
            return NE_OK;

        default:
            return NE_INVAL;
    }
}

/*! Check - have PNG image transparency.

    The function check PNG file, allocated in 'data' on transparence pallete.
    Out parameter - 'boolean result', return 'TRUE' if file transparenced, 'FALSE' otherwise

    @return NB_Error code
*/
NB_Error
IsPNGTransparent(void* data,
                 int dataLen,
                 nb_boolean *result)
{
    byte colorType = 0;
    byte *pLocalData = (byte*)data;

    /*
    PNG file have following header's structure:
    [signature - 8 bytes][chunk header - 8 bytes][ IHDR  chunk  -  13 bytes ]
    Where:
    chunk header: [trash - 4 byte | name - 4 bytes]

    First chunk SHOULD be IHDR - chunk with file info.

    So, firstly we read signature and check it - is file really PNG
    Then we check first chunk type - if it is not IHDR - something wrong.
    Otherwise we read IHDR chunk and parse it.
    */

    if ( dataLen < SIGANUTE_LENGHT + CHUNK_TRASH_LENGHT + CHUNK_NAME_LENGHT + IHDR_CHUNK_LENGHT )
    {
        return NE_INVAL;
    }

    //Check - is file really PNG
    if ( 0 != nsl_memcmp((void*)pLocalData, PNG_SIGNATURE, SIGANUTE_LENGHT) )
    {
        return NE_INVAL;
    }

    pLocalData += (SIGANUTE_LENGHT + CHUNK_TRASH_LENGHT);

    //Check - is first chunk really IHDR
    if ( 0 != nsl_memcmp((void*)pLocalData, IHDR_NAME, CHUNK_NAME_LENGHT) )
    {
        return NE_INVAL;
    }

    pLocalData += CHUNK_NAME_LENGHT;

    colorType = ((IHDR_Chunk*)pLocalData)->ColorType;

    if (PNG_B_AND_W_WITH_ALPHA == colorType || PNG_FULL_COLOR_WITH_ALPHA == colorType)
    {
        *result = TRUE;
        return NE_OK;
    }

    *result = FALSE;
    return NE_OK;
}

/*! @} */
