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
   @file        TileInternal.h
   @defgroup    nbmap

   Description: This header file is a simple wrapper of Tile.h. It included
   Tile.h, and provides several structors and macros used by CCC internally.
   Files in CCC should include this file instead of including "Tile.h".
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

#ifndef _TILEWRAPPER_H_
#define _TILEWRAPPER_H_

#include "Tile.h"
#include "Cache.h"

// Keys of the additional data of the tile
#define TILE_ADDITIONAL_KEY_DATASET_ID       "ds-gen-id"
#define TILE_ADDITIONAL_KEY_ERROR            "error"
#define TILE_ADDITIONAL_KEY_ETAG             "etag"
#define TILE_ADDITIONAL_KEY_NAME             "name"
#define TILE_ADDITIONAL_KEY_OPTIONAL         "optional"
#define TILE_ADDITIONAL_KEY_PARENT_LAYER     "parent-layer"
#define TILE_ADDITIONAL_KEY_REFRESH_INTERVAL "refresh-interval"
#define TILE_ADDITIONAL_KEY_TOD_MODE         "tod-mode"

// Defined layer deactivating.
#define LAYER_DEACTIVATING_IDENTIFIER  "deactivating-layer"

#define LAYER_LEVEL_IDENTIFIER  "level"
#define LAYER_LEVEL_BACKGROUND  "background"

// Defined layer categories.
#define LAYER_CATEGORY_IDENTIFIER "category"
#define LAYER_CATEGORY_ROUTE      "route"
#define LAYER_CATEGORY_TRAFFIC    "traffic"
#define LAYER_CATEGORY_WEATHER    "weather"
#define LAYER_CATEGORY_BASEMAP    "base-map"
#define LAYER_CATEGORY_SATELLITE  "satellite"
#define LAYER_CATEGORY_MATERIAL   "material"

#define MATERIAL_TYPE   "type"
#define MATERIAL_BASE_TYPE   "material-type"
#define MATERIAL_THEME_TYPE  "theme"

const string CONTENTID_PARAMETER_ROUTE_ID1("$rid1");
const string CONTENTID_PARAMETER_ROUTE_COLOR1("$rc1");
const string CONTENTID_PARAMETER_ROUTE_ID2("$rid2");
const string CONTENTID_PARAMETER_ROUTE_COLOR2("$rc2");

const string CONTENTID_PARAMETER_ROUTE_ID("$rid");
const string CONTENTID_PARAMETER_ROUTE_COLOR("$rc");

const string OPTIONAL_DEFAULT_ON("default-on");
const string OPTIONAL_DEFAULT_OFF("default-off");

namespace nbmap
{

// Invalid tile zoom level
#define INVALID_TILE_ZOOM_LEVEL  0
#define INVALID_LABLE_DRAW_ORDER 255


/*! Information of Tile, shared by all tiles of the same layer.

  This data structor packs information shared by all tiles of the same layer together,
  this will reduce parameters passed down from OffboardLayerProvider to other objects,
  such as TileManagers, TileRequests, or Tiles. Without this data structor, there'll be
  too many parameters to pass.

  @todo: Hide this structor from interface.
*/
struct TileLayerInfo
{
    TileLayerInfo()
            : layerID(0),
              minZoom(0),
              maxZoom(0),
              drawOrder(0),
              subDrawOrder(0),
              labelDrawOrder(INVALID_LABLE_DRAW_ORDER),
              refTileGridLevel(INVALID_TILE_ZOOM_LEVEL)
    {
    }

    uint32       layerID;
    uint32       minZoom;
    uint32       maxZoom;
    uint32       drawOrder;
    uint32       subDrawOrder;
    uint32       labelDrawOrder;
    uint32       refTileGridLevel;
    shared_ptr<std::string>  materialType;
    shared_ptr<std::string>  tileDataType;
    shared_ptr<const string> materialCategory;
    shared_ptr<std::string>  materialBaseType;
    shared_ptr<std::string>  materialThemeType;
    nbcommon::CachePtr cache;
};

typedef shared_ptr<struct TileLayerInfo> TileLayerInfoPtr;

// Several strings defined by DTS.

#define TILE_TYPE_HYBRID                    "hybrid"
#define TILE_TYPE_ROUTE                     "route"
#define TILE_TYPE_TRAFFIC                   "traffic"
#define TILE_TYPE_DVRT                      "DVRT"
#define TILE_TYPE_GVRT                      "GVRT"
#define TILE_TYPE_LAM                       "LAM"
#define TILE_TYPE_BR                        "BR"
#define TILE_TYPE_COMMON_MATERIALS          "MAT"
#define TILE_TYPE_LAYER_AVAILABILITY_MATRIX "LAM"
#define TILE_TYPE_DATA_AVAILABILITY_MATRIX  "DAM"
#define TILE_TYPE_2D_BUILDING_FOOTPRINTS    "B2D"
#define TILE_TYPE_SATELLITE                 "satellite"

/* @todo: Remove both definitions until there is another flag of metadata used to
          identify if the layer is a frame list layer, an animation layer or other
          layers.
*/
#define TILE_TYPE_WEATHER_FRAME_LIST        "DWRFL"
#define TILE_TYPE_WEATHER                   "DWR"

#define PARAMETER_KEY_TILE_X     "$x"
#define PARAMETER_KEY_TILE_Y     "$y"
#define PARAMETER_KEY_ZOOM_LEVEL "$z"
#define PARAMETER_KEY_FRAME_ID   "$frame"
#define PARAMETER_KEY_TIME_STAMP "$ts"
#define PARAMETER_KEY_QUAD       "$q"


#define HTTP_HEADER_TILE_REFRESH  "Refresh"


// Material type strings, defined in protocol.
#define MATERIAL_DAY_TIME "day"
#define MATERIAL_NIGHT_TIME "night"

}

#endif /* _TILEWRAPPER_H_ */
/*! @} */
