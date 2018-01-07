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

    @file     data_raster_tile_data_source_reply.c
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

/*! @{ */


#include "data_raster_tile_data_source_reply.h"


// Local Constants ...............................................................................

// Main elements
#define ELEMENT_MAPTILE                 "maptile-source"

// Subelements of map-tile element
#define ELEMENT_INTERNAL_SOURCE         "internal-source"
#define ELEMENT_BASE_URL                "url"
#define ELEMENT_TEMPLATE                "url-args-template"

// Attributes of map-tile element
#define ATTRIBUTE_MAP_GENERATION        "gen"
#define ATTRIBUTE_PROJECTION            "projection"
#define ATTRIBUTE_VALUE_MERCATOR        "mercator"

// Attributes of template element
#define ATTRIBUTE_TEMPLATE_TYPE         "type"
#define ATTRIBUTE_TEMPLATE_STRING       "template"
#define ATTRIBUTE_VALUE_TYPE_MAP        "map"
#define ATTRIBUTE_VALUE_TYPE_TRAFFIC    "traffic"
#define ATTRIBUTE_VALUE_TYPE_ROUTE      "route"
#define ATTRIBUTE_VALUE_TYPE_PT_ROUTE   "ptroute"       //public transit route
#define ATTRIBUTE_VALUE_TYPE_SATELLITE  "satellite"
#define ATTRIBUTE_VALUE_TYPE_HYBRID     "hybrid"
#define ATTRIBUTE_VALUE_TYPE_ROAD       "road"
#define ATTRIBUTE_VALUE_TYPE_ROUTESELECTOR1       "routeselector1"
#define ATTRIBUTE_VALUE_TYPE_ROUTESELECTOR2       "routeselector2"
#define ATTRIBUTE_VALUE_TYPE_ROUTESELECTOR3       "routeselector3"

// Attributes of base-url element
#define ATTRIBUTE_BASE_URL_VALUE    "value"

// Default value for number in data source array
const uint8 DEFAULT_DATA_COUNT = 3;


// Local functions ...............................................................................

static NB_Error GetMapTileElement(data_util_state* state, tpselt mapTileElement, NB_RasterTileDataSourceData* data);

static void FillTemplateData(NB_RasterTileDataSourceTemplate* template,
                             boolean dataValid,
                             NB_RasterTileDataSourceProjection projection,
                             boolean isInternalSource,
                             const char* generationInfo,
                             const char* baseUrl,
                             const char* tileStoreTemplate,
                             uint32 tileSize,
                             uint32 cacheId,
                             uint32 minZoom,
                             uint32 maxZoom);


// Public functions ..............................................................................

/* See description in header file */
void
data_raster_tile_data_source_reply_init(data_util_state* state, data_raster_tile_data_source_reply* reply)
{
    nsl_memset(reply, 0, sizeof(*reply));
}

/* See description in header file */
void
data_raster_tile_data_source_reply_free(data_util_state* state, data_raster_tile_data_source_reply* reply)
{
    // Nothing to free
}

/* See description in header file */
NB_Error
data_raster_tile_data_source_reply_from_tps(data_util_state* state, data_raster_tile_data_source_reply* reply, tpselt tpsElement)
{
    NB_Error result         = NE_OK;
    tpselt mapTileElement   = NULL;
    int iterator            = 0;
    boolean valid           = FALSE;

    if (tpsElement == NULL)
    {
        return NE_INVAL;
    }

    data_raster_tile_data_source_reply_free(state, reply);
    data_raster_tile_data_source_reply_init(state, reply);

    while ((mapTileElement = te_nextchild(tpsElement, &iterator)) != NULL)
    {
        if (mapTileElement && (nsl_stricmp(te_getname(mapTileElement), ELEMENT_MAPTILE) == 0))
        {
            // Each maptile-element can contain one or more templates. We extact each template
            // that is present.
            result = GetMapTileElement(state, mapTileElement, &reply->data);
            if (result == NE_OK)
            {
                valid = TRUE;
            }
        }
    }

    // Did we receive at least one element
    if (valid && (result == NE_OK))
    {
        return NE_OK;
    }

    // Failure
    data_raster_tile_data_source_reply_free(state, reply);
    return NE_INVAL;
}


// Local functions ...............................................................................

/*! Fill the data source element based on the given tps map tile element.

    @return NB_Error
*/

typedef enum {
    TtNone = -1,
    TtMap = 0,
    TtHybrid,
    TtSatellite,
    TtTraffic,
    TtRoute,
    TtPTRoute,  //public transit route
    TtRouteSelector1,
    TtRouteSelector2,
    TtRouteSelector3,
    TtEND
} TileType;

NB_Error
GetMapTileElement(data_util_state* state,
                  tpselt mapTileElement,                /*!< Source tps-element */
                  NB_RasterTileDataSourceData* data     /*!< Data element to fill with the content */
                  )
{
    tpselt      childElement        = NULL;
    int         iterator            = 0;
    const char* projectionString    = NULL;

    NB_RasterTileDataSourceProjection projection    = NB_RTDSP_None;
    boolean isInternalSource                        = FALSE;
    const char* generationInfo                      = NULL;
    const char* baseUrl                             = NULL;

    // Flags to see which templates are contained in this tps element
    boolean mapValid       = FALSE;
    boolean trafficValid   = FALSE;
    boolean routeValid     = FALSE;
    boolean satelliteValid = FALSE;
    boolean hybridValid    = FALSE;
    boolean ptrouteValid   = FALSE;
    boolean routeSelector1Valid     = FALSE;
    boolean routeSelector2Valid     = FALSE;
    boolean routeSelector3Valid     = FALSE;

    const char* tileStoreTemplate[TtEND] = {0};
    uint32      tileSize[TtEND] = {0};
    uint32      minZoom[TtEND] = {0};
    uint32      maxZoom[TtEND] = {0};
    uint32      cacheId[TtEND] = {0};
    int         element;


    // For all child elements
    while ((childElement = te_nextchild(mapTileElement, &iterator)) != NULL)
    {
        const char* elementName = te_getname(childElement);
        if (elementName)
        {
            // The internal source flag is optional
            if (nsl_stricmp(elementName, ELEMENT_INTERNAL_SOURCE) == 0)
            {
                isInternalSource = TRUE;
            }
            // Is it the base url
            else if (nsl_stricmp(elementName, ELEMENT_BASE_URL) == 0)
            {
                // Get base URL
                baseUrl = te_getattrc(childElement, ATTRIBUTE_BASE_URL_VALUE);
                if (!baseUrl)
                {
                    return NE_INVAL;
                }
            }
            // Is it a template. There could be (should be) multiple template elements
            else if (nsl_stricmp(elementName, ELEMENT_TEMPLATE) == 0)
            {
                const char* templateString = NULL;
                const char* templateType = NULL;

                // Get template string
                templateString = te_getattrc(childElement, ATTRIBUTE_TEMPLATE_STRING);
                if (!templateString)
                {
                    return NE_INVAL;
                }

                // Get template type
                templateType = te_getattrc(childElement, ATTRIBUTE_TEMPLATE_TYPE);
                if (!templateType)
                {
                    return NE_INVAL;
                }

                // Get type of map
                element = TtNone;
                // For road map, the template type attribute may be 'map' or 'road'. Handle both cases
                if (nsl_stricmp(templateType, ATTRIBUTE_VALUE_TYPE_MAP) == 0
                    || nsl_stricmp(templateType, ATTRIBUTE_VALUE_TYPE_ROAD) == 0)
                {
                    nsl_strlcpy(data->templateMap.templateUrl, templateString, NB_RTDS_TEMPLATE_LENGTH);
                    mapValid = TRUE;
                    element = TtMap;
                }
                else if(nsl_stricmp(templateType, ATTRIBUTE_VALUE_TYPE_TRAFFIC) == 0)
                {
                    nsl_strlcpy(data->templateTraffic.templateUrl, templateString, NB_RTDS_TEMPLATE_LENGTH);
                    trafficValid = TRUE;
                    element = TtTraffic;
                }
                else if(nsl_stricmp(templateType, ATTRIBUTE_VALUE_TYPE_ROUTE) == 0)
                {
                    nsl_strlcpy(data->templateRoute.templateUrl, templateString, NB_RTDS_TEMPLATE_LENGTH);
                    routeValid = TRUE;
                    element = TtRoute;
                }
                else if(nsl_stricmp(templateType, ATTRIBUTE_VALUE_TYPE_SATELLITE) == 0)
                {
                    nsl_strlcpy(data->templateSatellite.templateUrl, templateString, NB_RTDS_TEMPLATE_LENGTH);
                    satelliteValid = TRUE;
                    element = TtSatellite;
                }
                else if(nsl_stricmp(templateType, ATTRIBUTE_VALUE_TYPE_HYBRID) == 0)
                {
                    nsl_strlcpy(data->templateHybrid.templateUrl, templateString, NB_RTDS_TEMPLATE_LENGTH);
                    hybridValid = TRUE;
                    element = TtHybrid;
                }
                else if (nsl_stricmp(templateType, ATTRIBUTE_VALUE_TYPE_PT_ROUTE) == 0)
                {
                    nsl_strlcpy(data->templatePTRoute.templateUrl, templateString, NB_RTDS_TEMPLATE_LENGTH);
                    ptrouteValid = TRUE;
                    element = TtPTRoute;
                }
                else if(nsl_stricmp(templateType, ATTRIBUTE_VALUE_TYPE_ROUTESELECTOR1) == 0)
                {
                    nsl_strlcpy(data->templateRouteSelector1.templateUrl, templateString, NB_RTDS_TEMPLATE_LENGTH);
                    routeSelector1Valid = TRUE;
                    element = TtRouteSelector1;
                }
                else if(nsl_stricmp(templateType, ATTRIBUTE_VALUE_TYPE_ROUTESELECTOR2) == 0)
                {
                    nsl_strlcpy(data->templateRouteSelector2.templateUrl, templateString, NB_RTDS_TEMPLATE_LENGTH);
                    routeSelector2Valid = TRUE;
                    element = TtRouteSelector2;
                }
                else if(nsl_stricmp(templateType, ATTRIBUTE_VALUE_TYPE_ROUTESELECTOR3) == 0)
                {
                    nsl_strlcpy(data->templateRouteSelector3.templateUrl, templateString, NB_RTDS_TEMPLATE_LENGTH);
                    routeSelector3Valid = TRUE;
                    element = TtRouteSelector3;
                }

                // Get settings
                if( element != TtNone )
                {
                    tileStoreTemplate[element] = te_getattrc(childElement, "tile-store-template");
                    tileSize[element] = te_getattru(childElement, "tile-size");
                    minZoom[element] = te_getattru(childElement, "min-zoom");
                    maxZoom[element] = te_getattru(childElement, "max-zoom");
                    cacheId[element] = te_getattru(childElement, "cache-id");
                }
            }
        }
    }

    // Get tile map generation information
    generationInfo = te_getattrc(mapTileElement, ATTRIBUTE_MAP_GENERATION);

    // Get projection attribute.
    projectionString = te_getattrc(mapTileElement, ATTRIBUTE_PROJECTION);
    if (projectionString)
    {
        // We currently only have mercator projection.
        if (nsl_stricmp(projectionString, ATTRIBUTE_VALUE_MERCATOR) == 0)
        {
            projection = NB_RTDSP_Mercator;
        }
    }

    /*
        Check which templates were contained in this tps-element and set the projection, source,
        generation info and base Url accordingly.
    */
    FillTemplateData(&data->templateMap,
                      mapValid,
                      projection,
                      isInternalSource,
                      generationInfo,
                      baseUrl,
                      tileStoreTemplate[TtMap],
                      tileSize[TtMap],
                      cacheId[TtMap],
                      minZoom[TtMap],
                      maxZoom[TtMap]);

    FillTemplateData(&data->templateTraffic,
                      trafficValid,
                      projection,
                      isInternalSource,
                      generationInfo,
                      baseUrl,
                      tileStoreTemplate[TtTraffic],
                      tileSize[TtTraffic],
                      cacheId[TtTraffic],
                      minZoom[TtTraffic],
                      maxZoom[TtTraffic]);

    FillTemplateData(&data->templateRoute,
                      routeValid,
                      projection,
                      isInternalSource,
                      generationInfo,
                      baseUrl,
                      tileStoreTemplate[TtRoute],
                      tileSize[TtRoute],
                      cacheId[TtRoute],
                      minZoom[TtRoute],
                      maxZoom[TtRoute]);

    FillTemplateData(&data->templateSatellite,
                      satelliteValid,
                      projection,
                      isInternalSource,
                      generationInfo,
                      baseUrl,
                      tileStoreTemplate[TtSatellite],
                      tileSize[TtSatellite],
                      cacheId[TtSatellite],
                      minZoom[TtSatellite],
                      maxZoom[TtSatellite]);

    FillTemplateData(&data->templateHybrid,
                      hybridValid,
                      projection,
                      isInternalSource,
                      generationInfo,
                      baseUrl,
                      tileStoreTemplate[TtHybrid],
                      tileSize[TtHybrid],
                      cacheId[TtHybrid],
                      minZoom[TtHybrid],
                      maxZoom[TtHybrid]);

    FillTemplateData(&data->templatePTRoute,
                      ptrouteValid,
                      projection,
                      isInternalSource,
                      generationInfo,
                      baseUrl,
                      tileStoreTemplate[TtPTRoute],
                      tileSize[TtPTRoute],
                      cacheId[TtPTRoute],
                      minZoom[TtPTRoute],
                      maxZoom[TtPTRoute]);

    FillTemplateData(&data->templateRouteSelector1,
                      routeSelector1Valid,
                      projection,
                      isInternalSource,
                      generationInfo,
                      baseUrl,
                      tileStoreTemplate[TtRouteSelector1],
                      tileSize[TtRouteSelector1],
                      cacheId[TtRouteSelector1],
                      minZoom[TtRouteSelector1],
                      maxZoom[TtRouteSelector1]);

    FillTemplateData(&data->templateRouteSelector2,
                      routeSelector2Valid,
                      projection,
                      isInternalSource,
                      generationInfo,
                      baseUrl,
                      tileStoreTemplate[TtRouteSelector2],
                      tileSize[TtRouteSelector2],
                      cacheId[TtRouteSelector2],
                      minZoom[TtRouteSelector2],
                      maxZoom[TtRouteSelector2]);

    FillTemplateData(&data->templateRouteSelector3,
                      routeSelector3Valid,
                      projection,
                      isInternalSource,
                      generationInfo,
                      baseUrl,
                      tileStoreTemplate[TtRouteSelector3],
                      tileSize[TtRouteSelector3],
                      cacheId[TtRouteSelector3],
                      minZoom[TtRouteSelector3],
                      maxZoom[TtRouteSelector3]);

    return NE_OK;
}

/*! Helper function to fill template data. */
void
FillTemplateData(NB_RasterTileDataSourceTemplate* template,         /*!< Template to fill */
                 boolean dataValid,                                 /*!< Only fill data if this flag is set to TRUE */
                 NB_RasterTileDataSourceProjection projection,      /*!< Data to set to template */
                 boolean isInternalSource,                          /*!< Data to set to template */
                 const char* generationInfo,                        /*!< Data to set to template */
                 const char* baseUrl,                               /*!< Data to set to template */
                 const char* tileStoreTemplate,                     /*!< Data to set to template for the tile cache.  */
                 uint32 tileSize,                                   /*!< TileSize for template */
                 uint32 cacheId,                                    /*!< CacheId for template */
                 uint32 minZoom,                                    /*!< Minimum zoom level for template */
                 uint32 maxZoom                                     /*!< Maximum zoom level for template */
                )
{
    if (!dataValid)
    {
        return;
    }

    template->projection = projection;
    template->isInternalSource = isInternalSource;

    if (generationInfo)
    {
        nsl_strlcpy(template->generationInfo, generationInfo, sizeof(template->generationInfo));
    }

    if (baseUrl)
    {
        nsl_strlcpy(template->baseUrl, baseUrl, sizeof(template->baseUrl));
    }

    if (tileStoreTemplate)
    {
        nsl_strlcpy(template->tileStoreTemplate, tileStoreTemplate, sizeof(template->tileStoreTemplate));
    }

    template->tileSize = tileSize;
    template->cacheId = cacheId;

    template->minZoom = minZoom;
    template->maxZoom = maxZoom;
}

/*! @} */

