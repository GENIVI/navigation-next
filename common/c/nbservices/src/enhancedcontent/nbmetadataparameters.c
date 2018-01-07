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

    @file nbmetadataparameters.c
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

#include "nbcontextprotected.h"
#include "nbmetadataparametersprivate.h"
#include "nbmetadatatypes.h"
#include "data_metadata_source_query.h"
#include "data_wanted_content.h"
#include "nbqalog.h"


struct NB_MetadataParameters
{
    NB_Context*                     context;
    data_metadata_source_query      query;
};


static void QaLogMetadataRequest(NB_MetadataParameters* parameters);


NB_Error
NB_MetadataParametersCreate(NB_Context* context,
                            nb_boolean wantExtendedMaps,
                            nb_boolean wantSharedMaps,
                            nb_boolean wantUnifiedMaps,
                            const char* language,
                            uint32 screenWidth,
                            uint32 screenHeight,
                            uint32 screenResolution,
                            uint64 timeStamp,
                            NB_MetadataParameters** parameters)
{
    NB_MetadataParameters* pThis = 0;
    NB_Error result = NE_OK;
    data_util_state* dataState = 0;

    if (!context || !language || !parameters)
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
    dataState = NB_ContextGetDataState(context);

    result = result ? result : data_metadata_source_query_init(dataState, &pThis->query);
    result = result ? result : data_string_set(dataState, &pThis->query.language, language);
    if (result != NE_OK)
    {
        nsl_free(pThis);
        return result;
    }

    pThis->query.want_extended_maps = wantExtendedMaps;
    pThis->query.want_shared_maps   = wantSharedMaps;
    pThis->query.want_unified_maps  = wantUnifiedMaps;
    pThis->query.screen_width       = screenWidth;
    pThis->query.screen_height      = screenHeight;
    pThis->query.screen_resolution  = screenResolution;
    pThis->query.time_stamp         = timeStamp;

    *parameters = pThis;

    return result;
}


NB_Error
NB_MetadataParametersDestroy(NB_MetadataParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_metadata_source_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    nsl_free(pThis);

    return NE_OK;
}


tpselt
NB_MetadataParametersToTPSQuery(NB_MetadataParameters* pThis)
{
    if (pThis)
    {
        QaLogMetadataRequest(pThis);

        return data_metadata_source_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
    }

    return 0;
}


NB_Error
NB_MetadataParametersAddWantedContent(NB_MetadataParameters* parameters,
                                      const char* country,
                                      const char* datasetId,
                                      NB_EnhancedContentDataType type)
{
    NB_Error result = NE_OK;

    if (!parameters || !country || !datasetId)
    {
        return NE_INVAL;
    }

    if (type == NB_ECDT_All && nsl_strlen(datasetId) != 0)
    {
        return data_metadata_source_query_add_wanted_content(NB_ContextGetDataState(parameters->context), &parameters->query, country, datasetId, "");
    }

    if (type & NB_ECDT_CityModel)
    {
        result = result ? result : data_metadata_source_query_add_wanted_content(NB_ContextGetDataState(parameters->context), &parameters->query, country, datasetId, DATASET_TYPE_CITY_MODEL);
    }

    if (type & NB_ECDT_MotorwayJunctionObject)
    {
        result = result ? result : data_metadata_source_query_add_wanted_content(NB_ContextGetDataState(parameters->context), &parameters->query, country, datasetId, DATASET_TYPE_MOTORWAY_JUNCTION_OBJECT);
    }

    if (type & NB_ECDT_CommonTexture)
    {
        result = result ? result : data_metadata_source_query_add_wanted_content(NB_ContextGetDataState(parameters->context), &parameters->query, country, datasetId, DATASET_TYPE_COMMON_TEXTURE);
    }
    /* @todo: to be uncommented when server support this. */
    /*
    {
        result = result ? result : data_metadata_source_query_add_wanted_content(NB_ContextGetDataState(parameters->context), &parameters->query, country, datasetId, DATASET_TYPE_SPECIFIC_TEXTURE);
    }
    */

    if (type & NB_ECDT_RealisticSign)
    {
        result = result ? result : data_metadata_source_query_add_wanted_content(NB_ContextGetDataState(parameters->context), &parameters->query, country, datasetId, DATASET_TYPE_REALISTIC_SIGN);
    }


    /* @todo: to be uncommented when server support this. */
    /* if (type & NB_ECDT_RasterMaps) */
    /* { */
    /*     result = result ? result : data_metadata_source_query_add_wanted_content(NB_ContextGetDataState(parameters->context), &parameters->query, country, datasetId, DATASET_TYPE_RASTER_MAPS); */
    /* } */

    if (type & NB_ECDT_UnifiedBinaryModel)
    {
        result = result ? result : data_metadata_source_query_add_wanted_content(NB_ContextGetDataState(parameters->context), &parameters->query, country, datasetId, DATASET_TYPE_UNIFIED_BINARY_MODEL);
    }

    if (type & NB_ECDT_SpeedLimitsSign)
    {
        result = result ? result : data_metadata_source_query_add_wanted_content(NB_ContextGetDataState(parameters->context), &parameters->query, country, datasetId, DATASET_TYPE_SPEED_SIGN);
    }

    if(type & NB_ECDT_MapRegionsCommon)
    {
        result = result ? result : data_metadata_source_query_add_wanted_content(NB_ContextGetDataState(parameters->context), &parameters->query, country, datasetId, DATASET_TYPE_MAP_REGIONS_COMMON);
    }

    if(type & NB_ECDT_MapRegions)
    {
        result = result ? result : data_metadata_source_query_add_wanted_content(NB_ContextGetDataState(parameters->context), &parameters->query, country, datasetId, DATASET_TYPE_MAP_REGIONS);
    }
    return result;
}

NB_Error
NB_MetadataParametersAddWantExtAppContents(NB_MetadataParameters* parameters, int32 appMask)
{
    NB_Error err = NE_OK;

    if (!parameters)
    {
        return NE_BADDATA;
    }

    if (appMask & NB_TPA_Aisle411)
    {
        err = err ? err : data_metadata_source_query_add_extapp_content(NB_ContextGetDataState(parameters->context), &parameters->query, "ASAPP");
    }

    if (appMask & NB_TPA_HopStop)
    {
        err = err ? err : data_metadata_source_query_add_extapp_content(NB_ContextGetDataState(parameters->context), &parameters->query, "HSAPP");
    }

    if (appMask & NB_TPA_Fandango)
    {
        err = err ? err : data_metadata_source_query_add_extapp_content(NB_ContextGetDataState(parameters->context), &parameters->query, "FDAPP");
    }

    if (appMask & NB_TPA_OpenTable)
    {
        err = err ? err : data_metadata_source_query_add_extapp_content(NB_ContextGetDataState(parameters->context), &parameters->query, "OTAPP");
    }

    return err;
}

static void
QaLogMetadataRequest(NB_MetadataParameters* parameters)
{
    if (NB_ContextGetQaLog(parameters->context))
    {
        data_util_state* dataState = NB_ContextGetDataState(parameters->context);
        uint32 wantedContentCount = CSL_VectorGetLength(parameters->query.vec_wanted_content);
        uint32 index = 0;

        NB_QaLogMetadataSourceRequest(
            parameters->context,
            data_string_get(dataState, &parameters->query.language),
            wantedContentCount,
            parameters->query.want_extended_maps,
            parameters->query.screen_width,
            parameters->query.screen_height,
            parameters->query.screen_resolution
        );

        for (index = 0; index < wantedContentCount; index++)
        {
            data_wanted_content* wantedContent = (data_wanted_content*)CSL_VectorGetPointer(parameters->query.vec_wanted_content, index);

            if (wantedContent)
            {
                NB_QaLogWantedContent(
                                      parameters->context,
                                      NB_QLWCT_MetadataSource,
                                      data_string_get(dataState, &wantedContent->type),
                                      data_string_get(dataState, &wantedContent->country)
                                      );
            }
        }
    }
}

/*! @} */
