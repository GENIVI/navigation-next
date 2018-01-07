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

    @file     nbrastermapinformation.c
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

/*!
    @addtogroup nbrastermapinformation
    @{
*/

#include "nbrastermapinformation.h"
#include "nbcontextprotected.h"
#include "cslnetwork.h"
#include "nbutility.h"
#include "data_image.h"
#include "data_map_query.h"
#include "data_map_reply.h"
#include "nbspatial.h"
#include "transformutility.h"
#include "palimageblender.h"
#include "palerror.h"

struct NB_RasterMapInformation
{
    NB_Context*         context;
    data_map_query      query;
    data_map_reply      reply;
    NB_ImageOffset      pOffset;
    NB_ImageSize        poiSize;
    NB_Image*           poiImage;
    PAL_Image*          targetImage;
};

NB_Error
NB_RasterMapInformationCreate(NB_Context* context, tpselt reply, data_map_query* query, NB_RasterMapInformation** information)
{
    NB_RasterMapInformation* pThis = 0;
    NB_Error err = NE_OK;

    if (!context || !reply || !query || !information)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;
    err = err ? err : data_map_query_copy(NB_ContextGetDataState(pThis->context), &pThis->query, query);
    err = err ? err : data_map_reply_from_tps(NB_ContextGetDataState(pThis->context), &pThis->reply, reply);

    if (!err)
    {
        pThis->poiImage = NULL;
        pThis->targetImage = NULL;
        *information = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}

NB_Error
NB_RasterMapInformationPoiCreate(NB_Context* context, tpselt reply, data_map_query* query,
                                 NB_Image* poiImage, NB_ImageSize poiSize, NB_ImageOffset pOffset,NB_RasterMapInformation** information)
{
    NB_RasterMapInformation* pThis = 0;
    NB_Error err = NE_OK;

    if (!context || !reply || !query || !information)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;
    err = err ? err : data_map_query_copy(NB_ContextGetDataState(pThis->context), &pThis->query, query);
    err = err ? err : data_map_reply_from_tps(NB_ContextGetDataState(pThis->context), &pThis->reply, reply);

    if (!err)
    {
        if( poiImage )
        {
            pThis->poiImage = (NB_Image*)nsl_malloc(sizeof(NB_Image));
            pThis->poiImage->data = (uint8*)nsl_malloc(poiImage->dataLen);
            nsl_memcpy(pThis->poiImage->data, poiImage->data, poiImage->dataLen);
            pThis->poiImage->dataLen = poiImage->dataLen;
            nsl_memcpy(&pThis->pOffset, &pOffset, sizeof(NB_ImageOffset));
            nsl_memcpy(&pThis->poiSize, &poiSize, sizeof(NB_ImageSize));
        }

        *information = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}

NB_DEF NB_Error
NB_RasterMapInformationDestroy(NB_RasterMapInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_map_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    data_map_reply_free(NB_ContextGetDataState(pThis->context), &pThis->reply);

    if( pThis->poiImage )
    {
        nsl_free(pThis->poiImage->data);
        pThis->poiImage->data = NULL;
        nsl_free(pThis->poiImage);
        pThis->poiImage = NULL;
    }

    if( pThis->targetImage )
    {
        nsl_free(pThis->targetImage->imageBuffer);
        pThis->targetImage->imageBuffer = NULL;
        nsl_free(pThis->targetImage);
        pThis->targetImage = NULL;
    }

    nsl_free(pThis);

    return NE_OK;
}

NB_DEF NB_Error
NB_RasterMapInformationGetMapInformation(NB_RasterMapInformation* pThis, NB_LatitudeLongitude* latlong, double* scale, double* rotation)
{
    if (!pThis || !latlong || !scale || !rotation)
    {
        return NE_INVAL;
    }

    latlong->latitude = pThis->reply.point.lat;
    latlong->longitude = pThis->reply.point.lon;

    *scale = pThis->reply.scale;
    *rotation = pThis->reply.rotate;

    return NE_OK;
}

NB_DEF NB_Error
NB_RasterMapInformationGetImageData(NB_RasterMapInformation* pThis, NB_ImageFormat* format, uint8** data, uint32* dataLength)
{
    if (!pThis || !format || !data || !dataLength)
    {
        return NE_INVAL;
    }

    if( pThis->poiImage )
    {
        PAL_Error err = PAL_Ok;

        PAL_Image image;
        PAL_Image overlayImage;

        image.imageBuffer  = (byte*)(pThis->reply.image.data);
        image.imageBufferSize = pThis->reply.image.size;
        image.width  = pThis->query.image.width;
        image.height = pThis->query.image.height;
        image.format = PAL_IF_Unknown;

        overlayImage.imageBuffer  = (byte*)(pThis->poiImage->data);
        overlayImage.imageBufferSize = pThis->poiImage->dataLen;
        overlayImage.width  = pThis->poiSize.width;
        overlayImage.height = pThis->poiSize.height;
        overlayImage.format = PAL_IF_Unknown;

        err = PAL_ImageBlend(&image, &overlayImage, pThis->pOffset.x, pThis->pOffset.y, PAL_IF_ARGB32, &(pThis->targetImage));

        // if error happens, return base raster map image. Otherwise return with blending image.
        if (err == PAL_Ok)
        {
            *data = pThis->targetImage->imageBuffer;
            *dataLength = pThis->targetImage->imageBufferSize;
            *format = GetImageFormat(data_string_get(NB_ContextGetDataState(pThis->context), &pThis->query.image.format));
            return NE_OK;
        }
    }

    *data = pThis->reply.image.data;
    *dataLength = pThis->reply.image.size;
    *format = GetImageFormat(data_string_get(NB_ContextGetDataState(pThis->context), &pThis->query.image.format));

    return NE_OK;
}

NB_DEF NB_Error
NB_RasterMapInformationGetCenterLocation(NB_RasterMapInformation* pThis, NB_Location* location)
{
    if (!pThis || !location)
    {
        return NE_INVAL;
    }

    return SetNIMLocationFromLocMatch(location, NB_ContextGetDataState(pThis->context), &pThis->reply.locmatch);
}

NB_DEF NB_Error
NB_RasterMapInformationTransformLLToMap(NB_RasterMapInformation* pThis, NB_Rectangle* rectangle, NB_LatitudeLongitude latlong, NB_Point* point)
{
    double c_lat = 0.0;
    double c_lon = 0.0;
    double rotate = 0.0;
    double m_per_px = 0.0;

    if (!pThis || !rectangle || !point)
    {
        return NE_INVAL;
    }

    c_lat = pThis->query.map_source.point.lat;
    c_lon = pThis->query.map_source.point.lon;
    rotate = pThis->query.map_source.rotate;
    m_per_px = pThis->query.map_source.scale;

    /* We only support transforms to the same rectangle as we render to */
    if (pThis->query.image.width != rectangle->dx || pThis->query.image.height != rectangle->dy)
        return NE_INVAL;

    NB_SpatialConvertLatLongToPoint(c_lat, c_lon, m_per_px, rotate, rectangle, latlong.latitude, latlong.longitude, point);

    return NE_OK;
}

NB_DEF NB_Error
NB_RasterMapInformationTransformMapToLL(NB_RasterMapInformation* pThis, NB_Rectangle* rectangle, NB_Point* point, NB_LatitudeLongitude* latlong)
{
    double c_lat = 0.0;
    double c_lon = 0.0;
    double rotate = 0.0;
    double m_per_px = 0.0;

    if (!pThis || !rectangle || !point || !latlong)
    {
        return NE_INVAL;
    }

    c_lat = pThis->query.map_source.point.lat;
    c_lon = pThis->query.map_source.point.lon;
    rotate = pThis->query.map_source.rotate;
    m_per_px = pThis->query.map_source.scale;

    /* We only support transforms to the same rectangle as we render to */
    if (pThis->query.image.width != rectangle->dx || pThis->query.image.height != rectangle->dy)
        return NE_INVAL;

    NB_SpatialConvertPointToLatLong(c_lat, c_lon, m_per_px, rotate, rectangle, point, &latlong->latitude, &latlong->longitude);

    return NE_OK;
}


/*! @} */

