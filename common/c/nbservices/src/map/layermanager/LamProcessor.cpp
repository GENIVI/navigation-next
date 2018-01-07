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
   @file        LamProcessor.cpp
   @defgroup    nbmap

   Description: Implementation of LamProcessor.

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */


extern "C"
{
#include "palfile.h"
#include "nbcontextprotected.h"
}

#include "LamProcessor.h"
#include "datastreamimplementation.h"

using namespace nbmap;


static const uint32 BITS_PER_BYTE = 8;

/* See description in header file. */
LamProcessor::LamProcessor(NB_Context* context)
        : m_pContext(context)
{
}

/* See description in header file. */
LamProcessor::~LamProcessor()
{
}

/* See description in header file. */
PNG_Data* LamProcessor::DecodeLamFromTile(TilePtr tile)
{
    PNG_Data* decodedData = NULL;
    if (tile)
    {
        decodedData = DecodePNGFromDataStream(tile->GetData());
    }
    return decodedData;
}

/* See description in header file. */
bool LamProcessor::CheckLamIndex(PNG_Data* decodedData, int x, int y, int lamIndex)
{
    bool result = true;
    uint32 offset = decodedData->m_bitDepth *
                    (x % decodedData->m_width +
                     y % decodedData->m_height * decodedData->m_width);
    if (offset + decodedData->m_bitDepth <= decodedData->m_length) // Make sure it doesn't exceed limit.
    {
        uint32 bitmap = 0;
        // The bitmap consists of decodedData->m_bitDepth bytes, which may be 3 (RGB)
        // or 4 (RGBA). Retrieve this bit map here. But according to mobius team,
        // at present only the last Byte is useful (according to mobius team)!
        nsl_assert(decodedData->m_bitDepth <= 4);
        unsigned char* bitmapBase = (unsigned char*)decodedData->m_data + offset;
        for (uint32 i = 0; i < decodedData->m_bitDepth ; ++i)
        {
            unsigned char c = *bitmapBase;
            bitmap = (bitmap << BITS_PER_BYTE);
            bitmap |= c;
            bitmapBase++;
        }
        result = (bitmap & (1 << lamIndex)) ? true : false;
    }
    return result;
}

/* See description in header file. */
PNG_Data*  LamProcessor::DecodeLamFromFile(const string& path)
{
    NB_Error       error       = NE_OK;
    PNG_Data*      decodedData = NULL;
    unsigned char* data        = NULL;
    do
    {
        PAL_Instance* pal = NB_ContextGetPal(m_pContext);
        if (!pal || path.empty() || PAL_FileExists(pal, path.c_str()) != PAL_Ok)
        {
            error = NE_INVAL;
            break;
        }

        uint32         dataSize = 0;
        if ((PAL_FileLoadFile(pal, path.c_str(), &data, &dataSize) != PAL_Ok) || dataSize == 0)
        {
            error = NE_FSYS;
            break;
        };

        nbcommon::DataStreamPtr dataStream(new nbcommon::DataStreamImplementation());
        if (!dataStream)
        {
            error = NE_NOMEM;
            break;
        }

        error = dataStream->AppendData(data, dataSize);
        if (error)
        {
            break;
        }

        decodedData = DecodePNGFromDataStream(dataStream);

    } while (0);

    if (data)
    {
        nsl_free(data);
    }

    if (error)
    {
        decodedData = NULL;
    }
    return decodedData;
}

/* See description in header file. */
PNG_Data* LamProcessor::DecodePNGFromDataStream(nbcommon :: DataStreamPtr dataStream)
{
    PNG_Data* decodedData = NULL;
    if (dataStream && dataStream->GetDataSize())
    {
        NB_Error error = DecodePNG(dataStream, &decodedData);
        if (error)
        {
            decodedData = NULL;
        }
    }
    return decodedData;
}

/* See description in header file. */
void LamProcessor::DestroyPNGData(PNG_Data** decodedData)
{
    if (decodedData && *decodedData)
    {
        DestroyPNG(*decodedData);
        *decodedData = NULL;
    }
}

/*! @} */
