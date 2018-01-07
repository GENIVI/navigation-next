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

    @file     nbrastertiledatasourcepublic.h

    Public structures for raster tile data source.
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NB_RASTER_TILE_DATA_SOURCE_PUBLIC_H
#define NB_RASTER_TILE_DATA_SOURCE_PUBLIC_H

#include "nbexp.h"
#include "paltypes.h"
#include "nberror.h"

/*!
    @addtogroup nbrastertiledatasource
    @{
*/

// Public Constants ..............................................................................

#define NB_RTDS_GENERATION_LENGTH           256
#define NB_RTDS_BASE_URL_LENGTH             256
#define NB_RTDS_TEMPLATE_LENGTH             256
#define NB_RTDS_CONTENT_TEMPLATE_LENGTH     256

/*! Placeholders for the URL template.

    Each placeholder has to be replaced with the appropriate value in order to make the HTTP
    request. See template URL in NB_RasterTileDataSourceData.

    See SDS "Faster Rendering of Maps, Section 6.4.6.2 "URL-ARGS-Template" for allowed values.
    See SDS "Satellite Maps" for additional information.
*/
#define NB_RTDS_PLACEHOLDER_PREFIX          '$'     // All placeholders start with the prefix
#define NB_RTDS_PLACEHOLDER_VERSION         "v"
#define NB_RTDS_PLACEHOLDER_LOCALE          "loc"
#define NB_RTDS_PLACEHOLDER_FORMAT          "fmt"
#define NB_RTDS_PLACEHOLDER_X_COORDINATE    "x"     // Either "x/y/z" OR "q" is valid. Never both.
#define NB_RTDS_PLACEHOLDER_Y_COORDINATE    "y"
#define NB_RTDS_PLACEHOLDER_ZOOM            "z"
#define NB_RTDS_PLACEHOLDER_QUADKEY         "q"
#define NB_RTDS_PLACEHOLDER_TILE_SIZE       "sz"
#define NB_RTDS_PLACEHOLDER_RESOLUTION      "res"

/*! The handling for the route parameters is a little different. We append either one or two route overlay parameters
    if they are needed. These are the route parameters */
#define NB_RTDS_PARAMETER_ROUTE_ID1         "&rid1="
#define NB_RTDS_PARAMETER_ROUTE_COLOR1      "&rc1="
#define NB_RTDS_PARAMETER_ROUTE_ID2         "&rid2="
#define NB_RTDS_PARAMETER_ROUTE_COLOR2      "&rc2="
#define NB_RTDS_PARAMETER_ROUTE_ID3         "&rid3="

/*! Used to overwrite the tile size for hybrid/satellite templates in order to be consistent with the current interface */
#define NB_RTDS_PARAMETER_HYBRID_SIZE       "&size="

/*! Format for color for route request. Format is RGBA. */
#define NB_RTDS_COLOR_FORMAT                "%02X%02X%02X%02X"


// Public Types ..................................................................................

/*! Projection used for the data source URL. */
typedef enum
{
    NB_RTDSP_None = 0,

    NB_RTDSP_Mercator       /*!< All internal (NIM) data sources use mercator projection */

} NB_RasterTileDataSourceProjection;

/*! Data source template used by NB_RasterTileDataSourceData.

    @see NB_RasterTileDataSourceData
*/
typedef struct
{
    NB_RasterTileDataSourceProjection   projection;

    /*! Specifies if it is an internal (NIM) data source or not */
    nb_boolean  isInternalSource;

    /*! Information about tile generation on server */
    char        generationInfo[NB_RTDS_GENERATION_LENGTH];

    /*! The base URL and the template have to be concatenated. */
    char        baseUrl[NB_RTDS_BASE_URL_LENGTH];

    /*! The template URL contains placeholders (see NB_RTDS_PLACEHOLDER_* defines) which need
        to be replaced with the actual values to form a valid URL. The template has to be
        added to the base URL once formatted correctly.
    */
    char        templateUrl[NB_RTDS_TEMPLATE_LENGTH];

    /*! The template for the tile cache.  */
    char        tileStoreTemplate[NB_RTDS_CONTENT_TEMPLATE_LENGTH];

    /*! Tile size for this source */
    uint32      tileSize;

    /*! Cache-id for this source */
    uint32      cacheId;

    /*! Minimum zoom level for this source */
    uint32      minZoom;

    /*! Maximum zoom level for this source */
    uint32      maxZoom;

} NB_RasterTileDataSourceTemplate;

/*! Data source data returned by the callback.

    @see NB_RasterTileDataSourceCallbackFunction
*/
typedef struct
{
    /*! Templates for all the currently supported map tiles */
    NB_RasterTileDataSourceTemplate templateMap;
    NB_RasterTileDataSourceTemplate templateTraffic;
    NB_RasterTileDataSourceTemplate templateRoute;
    NB_RasterTileDataSourceTemplate templateSatellite;
    NB_RasterTileDataSourceTemplate templateHybrid;
    NB_RasterTileDataSourceTemplate templatePTRoute;
    NB_RasterTileDataSourceTemplate templateRouteSelector1;
    NB_RasterTileDataSourceTemplate templateRouteSelector2;
    NB_RasterTileDataSourceTemplate templateRouteSelector3;
} NB_RasterTileDataSourceData;


/*! @} */

#endif
