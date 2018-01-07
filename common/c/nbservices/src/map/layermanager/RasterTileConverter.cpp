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
 @file     RasterTileConverter.cpp
 @defgroup nbmap

 */
/*
 (C) Copyright 2011 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#include "RasterTileConverter.h"

#include "NBMTextureImageChunk.h"
#include "NBMTriangleGeometryChunk.h"
#include "NBMMeshChunk.h"
#include <limits.h>


using namespace nbmap;

// Local Constants ..............................................................................

static const uint32 MAX_TEXTURE_NAME_LENGTH = 30;

NB_Error RasterTileConverter::Convert(ConverterTilePtr sourceData,
                                      ConverterDataBufferPtr dataBuffer)
{
    if (sourceData == NULL || dataBuffer == NULL)
    {
        return NE_INVAL;
    }

    NBMFileHeader nbmTile;
    NB_Error result = ConvertData(&nbmTile, sourceData);
    if (result == NE_OK)
    {
        const char* pBuffer = NULL;
        unsigned int bufferSize;
        if (nbmTile.GetTileBuffer(pBuffer, bufferSize))
        {
            dataBuffer->SetData((uint8*)pBuffer, bufferSize);
        }
        else
        {
            result = NE_NOMEM;
        }
    }
    return result;
}

NB_Error RasterTileConverter::ConvertData(NBMFileHeader* nbmHeader, ConverterTilePtr tile)
{
    nbmHeader->SetIndex(tile->xPos, tile->yPos, tile->zoomLevel);
    nbmHeader->SetRefInfo((char*)NBM_FILE_REFNAME_RASTERTILE);

    char textureName[MAX_TEXTURE_NAME_LENGTH];
    nsl_sprintf(textureName, "%05u_%05u_%05u", tile->xPos, tile->yPos, tile->zoomLevel);

    return GenerateNBMData(nbmHeader, textureName, tile->buffer, tile->bufferSize);
}

NB_Error RasterTileConverter::GenerateNBMData(NBMFileHeader* nbmHeader, const char* textureName, const uint8* textureData, uint32 textureSize)
{
    NB_Error result = NE_OK;

    nbmHeader->SetMaking();

    // Mesh chunk will have 1 associated chunks: Mesh chunk -> Geometry chunk
    NBMLayerInfo layerInfo;
    layerInfo.layerType = NBM_LAYER_ID_ASSOCIATED;
    layerInfo.highScale = 0;
    layerInfo.lowScale = static_cast<float>(ULONG_MAX); // Idealy we would use max-float, but I'm not sure if we have a platform independent define for that
    // Create geometry chunk
    NBMTriangleGeometryChunk* meshGeometryChunk = (NBMTriangleGeometryChunk*)nbmHeader->CreateChunk(NBM_CHUNCK_IDENTIFIER_TRIANGLEGEOMETRY, &layerInfo);
    if (meshGeometryChunk == NULL)
    {
        return NE_NOMEM;
    }

    meshGeometryChunk->SetDataPrecision(NBMPrecision_float);

    layerInfo.layerType = NBM_LAYER_ID_RASTER;
    layerInfo.highScale = 0; //near value
    // Idealy we would use max-float, but I'm not sure if we have a platform independent define for that
    layerInfo.lowScale = static_cast<float>(ULONG_MAX);  //far value
    // Create mesh chunk
    NBMMeshChunk* meshChunk = (NBMMeshChunk*)nbmHeader->CreateChunk(NBM_CHUNCK_IDENTIFIER_MESH, &layerInfo, meshGeometryChunk);
    if (meshChunk == NULL)
    {
        return NE_NOMEM;
    }

    // Create material
    TextureBitMapBody texture;
    texture.SetName(textureName);
    texture.filesize = textureSize;
    texture.filebuf = new char[texture.filesize];
    memcpy(texture.filebuf, textureData, texture.filesize);

    if (meshChunk->SetData(texture, meshGeometryChunk) == NBM_INVALIDATE_INDEX)
    {
        result = NE_NOMEM;
    }

    return result;
}
/*! @} */
