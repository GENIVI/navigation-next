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

    @file     nbtiledatasource.h

    Public structures for  tile data source.
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NB_TILE_DATA_SOURCE_H
#define NB_TILE_DATA_SOURCE_H

#include "nbexp.h"
#include "paltypes.h"
#include "nberror.h"
#include "nbtiledatasourceplaceholders.h"

/*!
    @addtogroup nbtiledatasource
    @{
*/

// Public Types ..................................................................................

/*! Projection used for the data source URL. */
typedef enum
{
    NB_TDSP_None = 0,
    NB_TDSP_Mercator       /*!< All internal (NIM) data sources use mercator projection */
} NB_TileDataSourceProjection;

/*! Data source template used by NB_TileDataSourceData.

    @see NB_TileDataSourceData
*/
typedef struct
{
    NB_TileDataSourceProjection   projection;

    /*! Specifies if it is an internal (NIM) data source or not */
    nb_boolean                    isInternalSource;

    /*! Information about tile generation on server */
    char*                         generationInfo;

    /*! The base URL and the template have to be concatenated.
        It's possible store all url in templateUrl, it will be separated
        and formatted later
    */
    char*                         baseUrl;

    /*! The template URL contains placeholders (see NB_TDS_PLACEHOLDER_* defines) which need
        to be replaced with the actual values to form a valid URL. The template has to be
        added to the base URL once formatted correctly.

        This string should be used for single URL, in this case 'baseURL' should be empty
    */
    char*                         templateUrl;

    /*! The template for the tile cache. the string can be used as a key for the cache */
    char*                         tileStoreTemplate;

    /*! Tile size for this source */
    uint32                        tileSize;

    /*! Cache-id for this source */
    uint32                        cacheId;

    /*! mime type of format used for tiles */
    char*                         mimeType;
} NB_TileDataSourceTemplate;



/*! @} */

#endif
