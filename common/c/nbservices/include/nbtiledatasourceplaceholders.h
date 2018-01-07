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

    @file     nbtiledatasourceplaceholders.h

    Placeholders for the URL template
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

#ifndef NB_TILE_DATA_SOURCE_PLACE_HOLDERS_H
#define NB_TILE_DATA_SOURCE_PLACE_HOLDERS_H

/*!
    @addtogroup nbtiledatasource
    @{
*/

// Public Constants ..............................................................................

/*! Placeholders for the URL template.

    Each placeholder has to be replaced with the appropriate value in order to make the HTTP
    request. See template URL in NB_TileDataSourceData.

    See SDS "Faster Rendering of Maps, Section 6.4.6.2 "URL-ARGS-Template" for allowed values.
    See SDS "Satellite Maps" for additional information.
*/

// to be moved to separate header
#define NB_TDS_PLACEHOLDER_PREFIX          '$'     // All placeholders start with the prefix
#define NB_TDS_PLACEHOLDER_VERSION         "v"
#define NB_TDS_PLACEHOLDER_LOCALE          "loc"
#define NB_TDS_PLACEHOLDER_FORMAT          "fmt"
#define NB_TDS_PLACEHOLDER_X_COORDINATE    "x"     // Either "x/y/z" OR "q" is valid. Never both.
#define NB_TDS_PLACEHOLDER_Y_COORDINATE    "y"
#define NB_TDS_PLACEHOLDER_ZOOM            "z"
#define NB_TDS_PLACEHOLDER_QUADKEY         "q"
#define NB_TDS_PLACEHOLDER_TILE_SIZE       "sz"
#define NB_TDS_PLACEHOLDER_USER_PARAMETERS "user"

#define NB_MIME_TYPE_PNG                   "image/png"
#define NB_MIME_TYPE_JPEG                  "image/jpeg"
#define NB_MIME_TYPE_GIF                   "image/gif"

/*! The handling for the route parameters is a little different. We append either one or two route overlay parameters
    if they are needed. These are the route parameters */
#define NB_TDS_PARAMETER_ROUTE_ID1         "&rid1="
#define NB_TDS_PARAMETER_ROUTE_COLOR1      "&rc1="
#define NB_TDS_PARAMETER_ROUTE_ID2         "&rid2="
#define NB_TDS_PARAMETER_ROUTE_COLOR2      "&rc2="

/*! Format for color for route request. Format is RGBA. */
#define NB_TDS_COLOR_FORMAT                "%02X%02X%02X%02X"

/*! @} */

#endif
