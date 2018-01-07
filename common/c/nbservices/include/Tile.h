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
    @file     Tile.h
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

#ifndef __TILE__
#define __TILE__

#include "smartpointer.h"
#include "datastream.h"
#include "TileKey.h"
#include <string>
#include <map>

namespace nbmap
{

#define TDT_DETAILED_VECTOR_AREAS      "DVA"
#define TDT_DETAILED_VECTOR_ROADS      "DVR"
#define TDT_UNTEXTURED_3D_BUILDINGS    "B3D"
#define TDT_LANDMARKS_3D               "LM3D"
#define TDT_MAPTILE_ROAD               "road"
#define TDT_MAPTILE_TRAFFIC            "traffic"
#define TDT_MAPTILE_ROUTE              "route"
#define TDT_MAPTILE_SATELLITE          "satellite"
#define TDT_MAPTILE_HYBRID             "hybrid"
#define TDT_MAPTILE_MAP                "map"
#define TDT_INVALID                    ""


/*! Draw orders */
struct DrawOrder
{
    uint8  baseDrawOrder;
    uint8  labelDrawOrder;
    uint32 subDrawOrder;
};

/*! Tile provides methods that allow the client to obtain the binary data that represents
    the tile.
*/
class Tile
{
public:
    virtual ~Tile() {}

    /*! Get the tile data as a buffer.

        Provides a streaming interface to the data.

        @return A shared pointer to a DataStream object
    */
    virtual nbcommon::DataStreamPtr GetData() = 0;

    /*! Get Tile Data Size

        @return Data size of tile
    */
    virtual uint32 GetSize() = 0;

    /*! Get the data type of the tile

        @todo: this is still used by NBGMViewController, can be removed when not needed.
        @return A string of data type
    */
    virtual shared_ptr<std::string> GetDataType() = 0;

    /*! GetContentID, it is the identifier for the tile

        @return A string of content ID
    */
    virtual shared_ptr<std::string> GetContentID() = 0;

    /*! Get tile key of data

        @return Real tile key of data
    */
    virtual TileKeyPtr GetTileKey() = 0;

    /*! Get the full path of the caching tile

        The returned path is used by NBGM library to read the data of the caching tile.

        @todo: Remove this from interface once not needed.
        @return Full path of the caching tile
    */
    virtual shared_ptr<std::string> GetCachingPath() = 0;

    /*! Get the additional data of key and value

        The keys of the additional data are TILE_ADDITIONAL_KEY_*.

        @return Additional data of key and value
    */
    virtual shared_ptr<std::map<std::string, shared_ptr<std::string> > >
    GetAdditionalData() = 0;

    /*! Get Draw order for the layer

        Return the order in which the available layers should
        be drawn. Each layer has a particular draw order associated with it.
        Clients should display layers based on this draw order.

        @note: deprecated, use Tile::GetDrawOrders() instead.

        @return draw order.
    */
    virtual uint32
    GetDrawOrder() = 0;

    /*! Get label draw order for the layer

      @return label draw order.
    */
    virtual uint32
    GetLabelDrawOrder() = 0;

    /*! Get the layer ID for this tile.

        @note: deprecated, use Tile::GetDrawOrders() instead.

        @return The ID for this tile. Tiles belonging to the same layer have same id.
    */
    virtual uint32
    GetLayerID() = 0;

    /*! Returns associated time stamp

        Time stamp value of 0 means that this tile is not part of
        animation layer. If time stamp does have a value then the
        tile belongs to animation layer. time stamp can be used to
        identify where in the animation sequence should this tile be
        displayed.

        @return time stamp for the tile
     */

    virtual uint32  GetTimeStamp() const = 0;

    /*! Get category of corresponding material.

        @return shared pointer of string, it returns NULL to indicate using default materials.
    */
    virtual shared_ptr<const string> GetMaterialCategory()  const = 0;

    /*! Get base type of corresponding material.

        @return shared pointer of string
    */
    virtual shared_ptr<const string> GetMaterialBaseType() const = 0;
    
    /*! Get theme type of corresponding material.

        @return shared pointer of string
    */
    virtual shared_ptr<const string> GetMaterialThemeType() const = 0;

    virtual DrawOrder GetDrawOrders() const = 0;
};

typedef shared_ptr<Tile> TilePtr;

}
#endif
/*! @} */
