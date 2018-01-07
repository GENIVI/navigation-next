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

    @file     nbrastermapparameters.c
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*!
    @addtogroup nbrastermapparameters
    @{
*/

#include "nbrastermapparameters.h"
#include "nbcontextprotected.h"
#include "data_map_query.h"
#include "nbrouteinformationprivate.h"
#include "nbutility.h"
#include "nbrastermapparametersprivate.h"
#include "nbspatial.h"
#include "transformutility.h"
#include "palmath.h"

static NB_Error CreateThis(NB_Context* context, NB_LatitudeLongitude* center, NB_ImageFormat format, NB_ImageSize* size, double scale, double rotation, NB_RasterMapConfiguration* configuration, NB_RasterMapParameters** parameters);


NB_DEF NB_Error
NB_RasterMapParametersCreate(NB_Context* context, NB_LatitudeLongitude* center, NB_ImageFormat format, NB_ImageSize* size, double scale, double rotation, NB_RasterMapConfiguration* configuration, NB_RasterMapParameters** parameters)
{
    NB_RasterMapParameters* pThis = 0;
    NB_Error err = NE_OK;

    if (!context || !center || !size || !configuration || !parameters)
    {
        return NE_INVAL;
    }

    *parameters = 0;

    err = CreateThis(context, center, format, size, scale, rotation, configuration, &pThis);
    if( pThis )
    {
        pThis->poiImage = NULL;
    }

    if (!err)
    {
        *parameters = pThis;
    }
    else if (pThis)
    {
        nsl_free(pThis);
    }

    return err;
}

NB_DEC NB_Error NB_RasterMapParametersCreateWithPoi(NB_Context* context, NB_LatitudeLongitude* poiLocation,
                                                    NB_ImageFormat format, NB_ImageSize* size, uint8 zoomLevel,
                                                    NB_RasterMapConfiguration* configuration,
                                                    NB_Image* poiImage, NB_ImageSize* poiSize, NB_ImageOffset* mapCenterOffset, NB_ImageOffset* poiOffset,
                                                    NB_RasterMapParameters** parameters)
{
    NB_RasterMapParameters* pThis = NULL;
    NB_Error err = NE_OK;
    double rotation =0.0;
    double scale = 0.0;
    NB_LatitudeLongitude  center;
    double  mpx, mpy;
    double  mx, my;
    double  deltaX, deltaY;
    double  cosLatRadius;

    if( !poiLocation || !poiImage ||
        !size || !size->height || !size->width ||
        !mapCenterOffset || !poiOffset ||
        !poiSize || !poiSize->height || !poiSize->width ||
        zoomLevel > 19 )
    {
        return NE_INVAL;
    }


    if( mapCenterOffset->x > poiOffset->x )
    {
        deltaX = mapCenterOffset->x - poiOffset->x;
    }
    else
    {
        deltaX = poiOffset->x - mapCenterOffset->x;
    }

    if( mapCenterOffset->y > poiOffset->y )
    {
        deltaY = mapCenterOffset->y - poiOffset->y;
    }
    else
    {
        deltaY = poiOffset->y - mapCenterOffset->y;
    }

    // poi position out of range
    if( deltaX > (double)(size->width) || deltaY > (double)(size->height) )
    {
        return NE_INVAL;
    }

    // convert zoom level to scale
    cosLatRadius = RADIUS_EARTH_METERS*nsl_cos(TO_RAD(poiLocation->latitude));
    scale = cosLatRadius/256.0/nsl_pow(2, zoomLevel);

    mx = NB_SpatialMetersToMercator((double)mapCenterOffset->x*scale, cosLatRadius);
    my = NB_SpatialMetersToMercator((double)mapCenterOffset->y*scale, cosLatRadius);
    NB_SpatialConvertLatLongToMercator(poiLocation->latitude, poiLocation->longitude, &mpx, &mpy);
    NB_SpatialConvertMercatorToLatLong(mpx+mx, mpy-my, &(center.latitude), &(center.longitude));
    err = NB_RasterMapParametersCreate(context, &center, format, size, scale, rotation, configuration, &pThis);
    if( err == NE_OK && poiImage && poiLocation )
    {
        pThis->poiImage = (NB_Image*)nsl_malloc(sizeof(NB_Image));
        pThis->poiImage->data = (uint8*)nsl_malloc(poiImage->dataLen);
        nsl_memcpy(pThis->poiImage->data, poiImage->data, poiImage->dataLen);
        pThis->poiImage->dataLen = poiImage->dataLen;
        pThis->pOffset.x = (double)(size->width)/2.0-(mapCenterOffset->x - poiOffset->x);
        pThis->pOffset.y = (double)(size->height)/2.0-(mapCenterOffset->y - poiOffset->y);
        nsl_memcpy(&pThis->poiSize, poiSize, sizeof(NB_ImageSize));
    }

    *parameters = pThis;

    return err;
}

NB_DEF NB_Error
NB_RasterMapParametersCreateRoute(NB_Context* context, NB_LatitudeLongitude* center, NB_ImageFormat format, NB_ImageSize* size, double scale, double rotation, NB_RasterMapConfiguration* configuration, NB_RouteInformation* route, NB_RasterMapParameters** parameters)
{
    NB_RasterMapParameters* pThis = 0;
    NB_Error err = NE_OK;
    data_blob* routeID = 0;

    if (!context || !center || !size || !configuration || !route || !parameters)
    {
        return NE_INVAL;
    }

    *parameters = 0;

    err = CreateThis(context, center, format, size, scale, rotation, configuration, &pThis);
    if( pThis )
    {
        pThis->poiImage = NULL;
    }
    if (!err && pThis)
    {
        err = err ? err : NB_RouteInformationGetID(route, &routeID);
        err = err ? err : data_blob_copy(NB_ContextGetDataState(context), &pThis->query.map_style.route_id, routeID);
    }

    if (!err)
    {
        *parameters = pThis;
    }
    else if (pThis)
    {
        nsl_free(pThis);
    }

    return err;
}

NB_DEF NB_Error
NB_RasterMapParametersCreateManeuver(NB_Context* context, NB_LatitudeLongitude* center, NB_ImageFormat format, NB_ImageSize* size, double scale, double rotation, NB_RasterMapConfiguration* configuration, NB_RouteInformation* route, uint32 index, NB_RasterMapParameters** parameters)
{
    NB_RasterMapParameters* pThis = 0;
    NB_Error err = NE_OK;
    data_blob* routeID = 0;

    if (!context || !center || !size || !configuration || !route || !parameters)
    {
        return NE_INVAL;
    }

    if (index >= NB_RouteInformationGetManeuverCount(route))
    {
        return NE_RANGE;
    }

    *parameters = 0;

    err = CreateThis(context, center, format, size, scale, rotation, configuration, &pThis);
    if( pThis )
    {
        pThis->poiImage = NULL;
    }
    else
    {
        return NE_NOMEM;
    }

    if (!err)
    {
        pThis->query.map_style.maneuver_valid = TRUE;
        pThis->query.map_style.maneuver = index;

        err = err ? err : NB_RouteInformationGetID(route, &routeID);
        err = err ? err : data_blob_copy(NB_ContextGetDataState(context), &pThis->query.map_style.route_id, routeID);
    }

    if (!err)
    {
        *parameters = pThis;
    }
    else if (pThis)
    {
        nsl_free(pThis);
    }

    return err;
}

NB_DEF NB_Error
NB_RasterMapParametersCreateDetour(NB_Context* context, NB_LatitudeLongitude* center, NB_ImageFormat format, NB_ImageSize* size, double scale, double rotation, NB_RasterMapConfiguration* configuration, NB_RouteInformation* route, NB_RouteInformation* detour, NB_RasterMapParameters** parameters)
{
    NB_RasterMapParameters* pThis = 0;
    NB_Error err = NE_OK;
    data_blob* routeID = 0;

    if (!context || !center || !size || !configuration || !route || !detour || !parameters)
    {
        return NE_INVAL;
    }

    *parameters = 0;

    err = CreateThis(context, center, format, size, scale, rotation, configuration, &pThis);
    if( pThis )
    {
        pThis->poiImage = NULL;
    }

    if (!err)
    {
        err = err ? err : NB_RouteInformationGetID(route, &routeID);
        err = err ? err : data_blob_copy(NB_ContextGetDataState(context), &pThis->query.map_style.route_id, routeID);

        err = err ? err : NB_RouteInformationGetID(route, &routeID);
        err = err ? err : data_blob_copy(NB_ContextGetDataState(context), &pThis->query.map_style.alt_route_id, routeID);
    }

    if (!err)
    {
        *parameters = pThis;
    }
    else if (pThis)
    {
        nsl_free(pThis);
    }

    return err;
}

NB_DEF NB_Error
NB_RasterMapParametersDestroy(NB_RasterMapParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_map_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    if( pThis->poiImage )
    {
        nsl_free(pThis->poiImage->data);
        pThis->poiImage->data = NULL;
        nsl_free(pThis->poiImage);
        pThis->poiImage = NULL;
    }

    nsl_free(pThis);

    return NE_OK;
}

tpselt
NB_RasterMapParametersToTPSQuery(NB_RasterMapParameters* pThis)
{
    if (pThis)
    {
        return data_map_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
    }

    return 0;
}

NB_Error
NB_RasterMapParametersCopyQuery(NB_RasterMapParameters* parameters, data_map_query* destQuery)
{
    return data_map_query_copy(NB_ContextGetDataState(parameters->context), destQuery, &parameters->query);
}

NB_Error
CreateThis(NB_Context* context, NB_LatitudeLongitude* center, NB_ImageFormat format, NB_ImageSize* size, double scale, double rotation, NB_RasterMapConfiguration* configuration, NB_RasterMapParameters** parameters)
{
    NB_RasterMapParameters* pThis = *parameters;
    data_util_state* dataState = 0;
    NB_Error err = NE_OK;

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;
    dataState = NB_ContextGetDataState(pThis->context);

    err = data_map_query_init(dataState, &pThis->query);
    if (!err)
    {
        pThis->query.identify_center = configuration->wantCenterRevGeocoded;
        pThis->query.want_traffic = configuration->wantTraffic;
        pThis->query.want_pois = configuration->wantPlaces;
        // pThis->query.want_compass = configuration->wantCompass;

        err = data_string_set(dataState, &pThis->query.image.format, GetImageFormatString(format));

        pThis->query.image.width = (uint16)size->width;
        pThis->query.image.height = (uint16)size->height;
        pThis->query.image.dpi = configuration->dpi;

        pThis->query.map_source.point.lat = center->latitude;
        pThis->query.map_source.point.lon = center->longitude;

        pThis->query.map_source.scale = scale;
        pThis->query.map_source.rotate = rotation;
        err = data_string_set(dataState, &pThis->query.map_source.variant, "point");

        pThis->query.map_style.traffic = configuration->wantTraffic;

        pThis->query.map_style.maneuver_valid = FALSE;
        pThis->query.map_style.maneuver = NAV_MANEUVER_NONE;

        switch (configuration->legend)
        {
            case NB_RMLS_None:
                err = data_string_set(dataState, &pThis->query.map_style.legend, LEGEND_NONE);
                break;
            case NB_RMLS_Imperial:
                err = data_string_set(dataState, &pThis->query.map_style.legend, LEGEND_IMPERIAL);
                break;
            case NB_RMLS_Metric:
                err = data_string_set(dataState, &pThis->query.map_style.legend, LEGEND_METRIC);
                break;
            case NB_RMLS_Both:
                err = data_string_set(dataState, &pThis->query.map_style.legend, LEGEND_BOTH);
                break;
            default:
                err = NE_INVAL;
        }

        *parameters = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return NE_OK;
}


/*! @} */
