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

    @file     nbrastermapparameters.h
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

#ifndef NBRASTERMAPPARAMETERS_H
#define NBRASTERMAPPARAMETERS_H

#include "nbcontext.h"
#include "nbimage.h"
#include "nbrouteinformation.h"
#include "paltypes.h"

/*!
    @addtogroup nbrastermapparameters
    @{
*/


/*! @struct NB_RasterMapParameters
Opaque data structure that defines the parameters of a raster map render and download operation
*/
typedef struct NB_RasterMapParameters NB_RasterMapParameters;


typedef enum
{
    NB_RMLS_None,       /*!< No legend */
    NB_RMLS_Imperial,   /*!< A scale bar measured in imperial english units (i.e. feet/miles) */
    NB_RMLS_Metric,     /*!< A scale bar measured in metric units (i.e. meters/kilometers) */
    NB_RMLS_Both        /*!< A set of scale bars measured in both imperial english and metric units */
} NB_RasterMapLegendStyle;


/*! @struct NB_RasterMapConfiguration
Application-specific raster map configuration
*/
typedef struct NB_RasterMapConfiguration
{
    nb_boolean              wantPlaces;             /*!< Whether or not places (points of interest) should be included on map */
    nb_boolean              wantTraffic;            /*!< Whether or not traffic incidents should be included on map.  Only valid for maps that display a route */
    nb_boolean              wantCenterRevGeocoded;  /*!< Whether or not to reverse geocode the center of the map and return the address */
    NB_RasterMapLegendStyle legend;                 /*!< The type of legend to include on the map */
    uint16                  dpi;                    /*!< DPI value for the display to be considered when rendering the map map image.  Specify 0 for default */
} NB_RasterMapConfiguration;


/*! Create raster map parameters for a regular map

@param center The latitude and longitude on which to center the map
@param format The image format that the raster map should be rendered into
@param size The size of the map, in pixels
@param scale The scale of the map, in meters per pixel
@param rotation The heading of "up" in degrees.  The map is rotated counter-clockwise by the number of degrees specified
@param configuration Application-specific map rendering configuration settings
@param parameters On success, a newly created NB_RasterMapParameters object; NULL otherwise.  A valid object must be destroyed using NB_RasterMapParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_RasterMapParametersCreate(NB_Context* context, NB_LatitudeLongitude* center, NB_ImageFormat format, NB_ImageSize* size, double scale, double rotation, NB_RasterMapConfiguration* configuration, NB_RasterMapParameters** parameters);


/*! Create raster map parameters for a map showing a route

@param center The latitude and longitude on which to center the map
@param format The image format that the raster map should be rendered into
@param size The size of the map, in pixels
@param scale The scale of the map, in meters per pixel
@param rotation The heading of "up" in degrees.  The map is rotated counter-clockwise by the number of degrees specified
@param configuration Application-specific map rendering configuration settings
@param route The route to display on the map
@param parameters On success, a newly created NB_RasterMapParameters object; NULL otherwise.  A valid object must be destroyed using NB_RasterMapParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_RasterMapParametersCreateRoute(NB_Context* context, NB_LatitudeLongitude* center, NB_ImageFormat format, NB_ImageSize* size, double scale, double rotation, NB_RasterMapConfiguration* configuration, NB_RouteInformation* route, NB_RasterMapParameters** parameters);


/*! Create raster map parameters for a map showing a maneuver on a route

@param center The latitude and longitude on which to center the map
@param format The image format that the raster map should be rendered into
@param size The size of the map, in pixels
@param scale The scale of the map, in meters per pixel
@param rotation The heading of "up" in degrees.  The map is rotated counter-clockwise by the number of degrees specified
@param configuration Application-specific map rendering configuration settings
@param route The route containing the maneuver
@param index The zero-based index of the maneuver on the route to display on the map
@param parameters On success, a newly created NB_RasterMapParameters object; NULL otherwise.  A valid object must be destroyed using NB_RasterMapParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_RasterMapParametersCreateManeuver(NB_Context* context, NB_LatitudeLongitude* center, NB_ImageFormat format, NB_ImageSize* size, double scale, double rotation, NB_RasterMapConfiguration* configuration, NB_RouteInformation* route, uint32 index, NB_RasterMapParameters** parameters);


/*! Create raster map parameters for a map showing a detour

@param center The latitude and longitude on which to center the map
@param format The image format that the raster map should be rendered into
@param size The size of the map, in pixels
@param scale The scale of the map, in meters per pixel
@param rotation The heading of "up" in degrees.  The map is rotated counter-clockwise by the number of degrees specified
@param configuration Application-specific map rendering configuration settings
@param route The original route to display on the map
@param detour The detour route to display on the map
@param parameters On success, a newly created NB_RasterMapParameters object; NULL otherwise.  A valid object must be destroyed using NB_RasterMapParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_RasterMapParametersCreateDetour(NB_Context* context, NB_LatitudeLongitude* center, NB_ImageFormat format, NB_ImageSize* size, double scale, double rotation, NB_RasterMapConfiguration* configuration, NB_RouteInformation* route, NB_RouteInformation* detour, NB_RasterMapParameters** parameters);


/*! Destroy a previously created ReverseRasterMapParameters object

@param parameters A NB_RasterMapParameters object created with a call to one of the NB_RasterMapParametersCreate functions
@returns NB_Error
*/
NB_DEC NB_Error NB_RasterMapParametersDestroy(NB_RasterMapParameters* parameters);

/*! Create raster map parameters for a regular map with POI image

@param center The latitude and longitude of POI
@param format The image format that the raster map should be rendered into
@param size The size of the map, in pixels
@param zoom level
@param configuration Application-specific map rendering configuration settings
@param poi image data for overlay
@param poi image size
@param offset from poi location to raster map center
@param offset from poi location to poi drawing position
@param parameters On success, a newly created NB_RasterMapParameters object; NULL otherwise.  A valid object must be destroyed using NB_RasterMapParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_RasterMapParametersCreateWithPoi(NB_Context* context, NB_LatitudeLongitude* poiLocation,
                                             NB_ImageFormat format, NB_ImageSize* size, uint8 zoomLevel, NB_RasterMapConfiguration* configuration,
                                             NB_Image* poiImage, NB_ImageSize* poiSize, NB_ImageOffset* mapCenterOffset, NB_ImageOffset* poiOffset,
                                             NB_RasterMapParameters** parameters);

/*! @} */

#endif
