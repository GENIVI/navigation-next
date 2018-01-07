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

@file     StaticRasterTile.h
@date     03/15/2012
@author   Victor Wang

*/
/*
(C) Copyright 2012 by Networks In Motion, Inc.                

The information contained herein is confidential, proprietary 
to Networks In Motion, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of Networks In Motion is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
/*! @{ */
#pragma once
#ifndef __STATIC_RASTER_TILE_H__
#define __STATIC_RASTER_TILE_H__

#include "NcdbTypes.h"
#include "AutoSharedPtr.h"

class CNbm3DChunk;

namespace Ncdb 
{

class Sprite;

typedef enum
{
    TILE_RESOLUTION_100 = 100,      // 100 dpi
    TILE_RESOLUTION_200 = 200,      // 200 dpi
    TILE_RESOLUTION_300 = 300,      // 300 dpi
    TILE_RESOLUTION_400 = 400,      // 400 dpi
    TILE_RESOLUTION_500 = 500,      // 500 dpi
    TILE_RESOLUTION_600 = 600,      // 600 dpi
    TILE_RESOLUTION_COUNT = 6
}RasterTileResolution;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class NCDB_API StaticRasterTile
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{
public: 
    // Construct by a map data resource path, all static PNG raster tiles 
    // chunk files exist at resource path named "rastertiles" directory.
    StaticRasterTile(const char* resourcePath);

    // Destructor
    ~StaticRasterTile(void);

    // Cleanup
    void CleanUp();

    // Initialized all static PNG raster tiles chunks
    bool Initialize();

    //! Get static tile (x, y, z) by resolution
    //! return true for success and false for failed.
    //! Need call ReleaseBuffer() to delete buffer after use it.    
    bool GetTile(AutoSharedPtr<unsigned char>& buffer, 
                 unsigned int& bufferSize,
                 int x, int y, int z, 
                 RasterTileResolution resolution);

private:
    //! Get tile(x, y, z) buffer from data chunk
    bool GetTile(AutoSharedPtr<unsigned char>& buffer, 
                 unsigned int& bufferSize,
                 int x, int y, int z,
                 CNbm3DChunk* chunk);

    //! Get chunk by resolution
    CNbm3DChunk* GetDataChunk(RasterTileResolution resolution);

private:
    typedef struct
    {
        RasterTileResolution resolution;
        CNbm3DChunk*         chunk;
    }ResolutionChunk;

    ResolutionChunk m_dataChunks[TILE_RESOLUTION_COUNT];  // Chunk datas

    bool         m_bInitialized;   // Flag indicates whether static raster tile chunks nitialized or not
    const char*  m_resourcePath;   // Map data resource path
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

};
#endif // __STATIC_RASTER_TILE_H__
/*! @} */
