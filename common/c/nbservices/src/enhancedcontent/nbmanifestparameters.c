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

    @file nbmanifestparameters.c
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
#include "nbmanifestparameters.h"
#include "data_metadata_manifest_query.h"
#include "data_wanted_content.h"
#include "nbqalog.h"

struct NB_ManifestParameters
{
    NB_Context*                     context;
    data_metadata_manifest_query    query;
};


static void QaLogManifestQuery(NB_ManifestParameters* parameters);


NB_Error
NB_ManifestParametersCreate(NB_Context* context, NB_ManifestParameters** parameters)
{
    NB_ManifestParameters* pThis = 0;
    NB_Error result = NE_OK;
    data_util_state* dataState = 0;

    if (!context || !parameters)
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

    result = result ? result : data_metadata_manifest_query_init(dataState, &pThis->query);

    if (!result)
    {
        *parameters = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return result;
}


NB_Error
NB_ManifestParametersDestroy(NB_ManifestParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_metadata_manifest_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    nsl_free(pThis);

    return NE_OK;
}


tpselt
NB_ManifestParametersToTPSQuery(NB_ManifestParameters* pThis)
{
    if (pThis)
    {
        QaLogManifestQuery(pThis);

        return data_metadata_manifest_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
    }

    return 0;
}


NB_Error
NB_ManifestParametersAddWantedContent(NB_ManifestParameters* parameters,
                                      const char* country,
                                      const char* datasetId,
                                      NB_EnhancedContentDataType type)
{
    NB_Error result = NE_OK;

    if (!parameters || !country || !datasetId)
    {
        return NE_INVAL;
    }

    if (type == NB_ECDT_All)
    {
        return data_metadata_manifest_query_add_wanted_content(NB_ContextGetDataState(parameters->context),
                                                               &parameters->query,
                                                               country,
                                                               datasetId,
                                                               "");
    }

    if (type & NB_ECDT_CityModel)
    {
        result = result ? result : data_metadata_manifest_query_add_wanted_content(NB_ContextGetDataState(parameters->context),
                                                                                   &parameters->query,
                                                                                   country,
                                                                                   datasetId,
                                                                                   DATASET_TYPE_CITY_MODEL);
    }

    if (type & NB_ECDT_MotorwayJunctionObject)
    {
        result = result ? result : data_metadata_manifest_query_add_wanted_content(NB_ContextGetDataState(parameters->context),
                                                                                   &parameters->query,
                                                                                   country,
                                                                                   datasetId,
                                                                                   DATASET_TYPE_MOTORWAY_JUNCTION_OBJECT);
    }

    if (type & NB_ECDT_CommonTexture)
    {
        result = result ? result : data_metadata_manifest_query_add_wanted_content(NB_ContextGetDataState(parameters->context),
                                                                                   &parameters->query,
                                                                                   country,
                                                                                   datasetId,
                                                                                   DATASET_TYPE_COMMON_TEXTURE);
    }

    if (type & NB_ECDT_SpecificTexture)
    {
        result = result ? result : data_metadata_manifest_query_add_wanted_content(NB_ContextGetDataState(parameters->context),
                                                                                   &parameters->query,
                                                                                   country,
                                                                                   datasetId,
                                                                                   DATASET_TYPE_SPECIFIC_TEXTURE);
    }

    if (type & NB_ECDT_RealisticSign)
    {
        result = result ? result : data_metadata_manifest_query_add_wanted_content(NB_ContextGetDataState(parameters->context),
                                                                                   &parameters->query,
                                                                                   country,
                                                                                   datasetId,
                                                                                   DATASET_TYPE_REALISTIC_SIGN);
    }

    if (type & NB_ECDT_MapRegions)
    {
        result = result ? result : data_metadata_manifest_query_add_wanted_content(NB_ContextGetDataState(parameters->context),
                                                                                   &parameters->query,
                                                                                   country,
                                                                                   datasetId,
                                                                                   DATASET_TYPE_MAP_REGIONS);
    }
    //@todo: add wantted-content for SpeedLimitsImges.
    return result;
}


static void
QaLogManifestQuery(NB_ManifestParameters* parameters)
{
    if (NB_ContextGetQaLog(parameters->context))
    {
        data_util_state* dataState = NB_ContextGetDataState(parameters->context);

        uint32 wantedContentCount = CSL_VectorGetLength(parameters->query.vec_wanted_content);
        uint32 index = 0;

        NB_QaLogManifestRequest(parameters->context, wantedContentCount);
        
        for (index = 0; index < wantedContentCount; index++)
        {
            data_wanted_content* wantedContent = (data_wanted_content*)CSL_VectorGetPointer(parameters->query.vec_wanted_content, index);

            if (wantedContent)
            {
                NB_QaLogWantedContent(
                    parameters->context,
                    NB_QLWCT_Manifest,
                    data_string_get(dataState, &wantedContent->type),
                    data_string_get(dataState, &wantedContent->country)
                );
            }
        }
    }
}

/*! @} */
