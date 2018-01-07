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

    @file nbmetadatainformation.c
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
#include "nbmetadatainformationprivate.h"
#include "nbmetadatatypes.h"
#include "nbenhancedcontenttypes.h"
#include "data_metadata_source_reply.h"
#include "data_content_source.h"
#include "data_url_args_template.h"
#include "data_metadata.h"
#include "nbqalog.h"
#include "data_extapp.h"

// Constants .....................................................................................


// Local types ...................................................................................

/*! The metadata information */
struct NB_MetadataInformation
{
    NB_Context*                     context;
    data_metadata_source_reply      reply;
};


// Local Functions ...............................................................................

/*! Get the metadata source replies from data_content_source

@param state An data_util_state object
@param contentSource An data_content_source object
@param reply An NB_MetadataSourceReply object set from data_content_source
@param vectorReplies vector of NB_MetadataSourceReply added by reply
@returns NB_Error
*/
static NB_Error ContentSourceGetMetadataSourceReplies(data_util_state* state, data_content_source* contentSource, NB_MetadataSourceReply* reply, CSL_Vector* vectorReplies);

/*! Get the metadata source replies from data_url_args_template

@param state An data_util_state object
@param urlArgsTemplate An data_url_args_template object
@param reply An NB_MetadataSourceReply object set from data_url_args_template
@param vectorReplies vector of NB_MetadataSourceReply added by reply
@returns NB_Error
*/
static NB_Error UrlArgsTemplateGetMetadataSourceReplies(data_util_state* state, data_url_args_template* urlArgsTemplate, NB_MetadataSourceReply* reply, CSL_Vector* vectorReplies);

/*! Get the metadata source replies from data_metadata

@param state An data_util_state object
@param metadata An data_metadata object
@param reply An NB_MetadataSourceReply object set from data_metadata
@param vectorReplies vector of NB_MetadataSourceReply added by reply
@returns NB_Error
*/
static NB_Error MetadataGetMetadataSourceReplies(data_util_state* state, data_metadata* metadata, NB_MetadataSourceReply* reply, CSL_Vector* vectorReplies);


static void QaLogMetadataResponse(NB_MetadataInformation* information);


// Public functions ..............................................................................

/* See header file for description */
NB_Error
NB_MetadataInformationCreate(NB_Context* context, tpselt reply, NB_MetadataInformation** information)
{
    NB_MetadataInformation* pThis = 0;
    NB_Error result = NE_OK;

    if (!context || !reply || !information)
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
    result = result ? result : data_metadata_source_reply_from_tps(NB_ContextGetDataState(pThis->context), &pThis->reply, reply);

    if (!result)
    {
        QaLogMetadataResponse(pThis);

        *information = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return result;
}

/* See header file for description */
NB_Error
NB_MetadataInformationDestroy(NB_MetadataInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_metadata_source_reply_free(NB_ContextGetDataState(pThis->context), &pThis->reply);

    nsl_free(pThis);

    return NE_OK;
}

/* See header file for description */
NB_Error
NB_MetadataInformationGetMetadataSourceReplies(NB_MetadataInformation* information, CSL_Vector** vectorReplies)
{
    NB_Error result = NE_OK;
    uint32 length = 0;
    uint32 n = 0;
    CSL_Vector* replies = NULL;
    data_content_source* contentSource = NULL;

    if (!information || !vectorReplies)
    {
        return NE_INVAL;
    }

    // Alloc vector of metadata source replies.
    replies = CSL_VectorAlloc(sizeof(NB_MetadataSourceReply));
    if (!replies)
    {
        return NE_NOMEM;
    }

    // Set from data_content_source.
    length = CSL_VectorGetLength(information->reply.vec_content_source);
    for (n = 0; n < length; ++n)
    {
        NB_MetadataSourceReply reply = {0};

        contentSource = (data_content_source*)CSL_VectorGetPointer(information->reply.vec_content_source, n);
        result = ContentSourceGetMetadataSourceReplies(NB_ContextGetDataState(information->context), contentSource, &reply, replies);
        if (result != NE_OK)
        {
            CSL_VectorDealloc(replies);
            return result;
        }
    }

    *vectorReplies = replies;

    return NE_OK;
}

/* See header file for description */
NB_Error
NB_MetadataInformationGetCitySummaryDisplay(NB_MetadataInformation* information, const char** citySummaryDisplay)
{
    const char* tmp = NULL;

    if (!information || !citySummaryDisplay)
    {
        return NE_INVAL;
    }

    tmp = data_string_get(NB_ContextGetDataState(information->context), &information->reply.city_summary_display);

    if (!tmp)
    {
        return NE_NOENT;
    }

    *citySummaryDisplay = tmp;

    return NE_OK;
}

NB_Error
NB_MetadataInformationGetExtAppTemplatesContentCount(NB_MetadataInformation* information, uint32* count)
{
    NB_Error err = NE_OK;

    if (!information || !count)
    {
        return NE_BADDATA;
    }

    *count = 0;

    if (information->reply.vec_extapp)
    {
        *count = CSL_VectorGetLength(information->reply.vec_extapp);
    }

    return err;
}

NB_Error
NB_MetadataInformationGetExtAppTemplatesContent(NB_MetadataInformation* information, NB_ExtAppTemplatesContent* extAppTemplatesContent, uint32 index)
{
    data_extapp* pea = NULL;

    if (!information || !extAppTemplatesContent)
    {
        return NE_BADDATA;
    }

    if (!information->reply.vec_extapp || index >= (uint32)CSL_VectorGetLength(information->reply.vec_extapp))
    {
        return NE_RANGE;
    }

    nsl_memset(extAppTemplatesContent, 0, sizeof(NB_ExtAppTemplatesContent));

    pea = (data_extapp*)CSL_VectorGetPointer(information->reply.vec_extapp, index);
    if (pea)
    {
        const char* appName = data_string_get(NB_ContextGetDataState(information->context), &(pea->name));

        if (appName)
        {
            const char* iconName = data_string_get(NB_ContextGetDataState(information->context), &(pea->icon_location.name));
            const char* iconDescription = data_string_get(NB_ContextGetDataState(information->context), &(pea->icon_location.description));

            if (nsl_strcmp(appName, "ASAPP") == 0)
            {
                extAppTemplatesContent->app = NB_TPA_Aisle411;
            }
            else if (nsl_strcmp(appName, "HSAPP") == 0)
            {
                extAppTemplatesContent->app = NB_TPA_HopStop;
            }
            else if (nsl_strcmp(appName, "FDAPP") == 0)
            {
                extAppTemplatesContent->app = NB_TPA_Fandango;
            }
            else if (nsl_strcmp(appName, "OTAPP") == 0)
            {
                extAppTemplatesContent->app = NB_TPA_OpenTable;
            }

            if (iconName)
            {
                nsl_strncpy(extAppTemplatesContent->iconName, iconName, sizeof(extAppTemplatesContent->iconName));
            }

            if (iconDescription)
            {
                nsl_strncpy(extAppTemplatesContent->iconDescription, iconDescription, sizeof(extAppTemplatesContent->iconDescription));
            }
        }
    }

    return NE_OK;
}

// Local Functions ...............................................................................

/* See declaration for description */
static NB_Error
ContentSourceGetMetadataSourceReplies(data_util_state* state, data_content_source* contentSource, NB_MetadataSourceReply* reply, CSL_Vector* vectorReplies)
{
    NB_Error result = NE_OK;
    uint32 length = 0;
    uint32 n = 0;
    const char* tmp = NULL;
    data_url_args_template* urlArgsTemplate = NULL;

    if (!state || !contentSource || !reply || !vectorReplies)
    {
        return NE_INVAL;
    }

    // Set urlBase.
    tmp = data_string_get(state, &contentSource->url.value);
    if (!tmp)
    {
        return NE_NOENT;
    }
    nsl_memset(reply->urlBase, 0, sizeof(reply->urlBase));
    nsl_strlcpy(reply->urlBase, tmp, sizeof(reply->urlBase));

    // Set projectionType.
    tmp = data_string_get(state, &contentSource->projection);
    if (!tmp)
    {
        return NE_NOENT;
    }
    if (nsl_strcmp(tmp, PROJECTION_TYPE_MERCATOR) == 0)
    {
        reply->projectionType = NB_ECPT_Mercator;
    }
    else if (nsl_strcmp(tmp, PROJECTION_TYPE_TRANSVERSE_MERCATOR) == 0)
    {
        reply->projectionType = NB_ECPT_TransverseMercator;
    }
    else if (nsl_strcmp(tmp, PROJECTION_TYPE_UNIVERSAL_TRANSVERSE_MERCATOR) == 0)
    {
        reply->projectionType = NB_ECPT_UniversalTransverseMercator;
    }
    else
    {
        reply->projectionType = NB_ECPT_Invalid;
    }

    // Set country.
    tmp = data_string_get(state, &contentSource->country);
    if (!tmp)
    {
        return NE_NOENT;
    }
    nsl_memset(reply->country, 0, sizeof(reply->country));
    nsl_strlcpy(reply->country, tmp, MAX_ENHANCED_CONTENT_CONFIG_ITEM_LENGTH + 1);

    // Set from data_url_args_template.
    length = CSL_VectorGetLength(contentSource->vec_url_args_template);
    for (n = 0; n < length; ++n)
    {
        urlArgsTemplate = (data_url_args_template*)CSL_VectorGetPointer(contentSource->vec_url_args_template, n);
        result = UrlArgsTemplateGetMetadataSourceReplies(state, urlArgsTemplate, reply, vectorReplies);
        if (result != NE_OK)
        {
            break;
        }
    }

    return result;
}

/* See declaration for description */
static NB_Error
UrlArgsTemplateGetMetadataSourceReplies(data_util_state* state, data_url_args_template* urlArgsTemplate, NB_MetadataSourceReply* reply, CSL_Vector* vectorReplies)
{
    NB_Error result = NE_OK;
    uint32 length = 0;
    uint32 n = 0;
    const char* tmp = NULL;
    data_metadata* metadata = NULL;

    if (!state || !urlArgsTemplate || !reply || !vectorReplies)
    {
        return NE_INVAL;
    }

    // Set contentType.
    tmp = data_string_get(state, &urlArgsTemplate->type);
    if (!tmp)
    {
        return NE_NOENT;
    }
    if (nsl_strcmp(tmp, DATASET_TYPE_CITY_MODEL) == 0)
    {
        reply->contentType = NB_ECDT_CityModel;
    }
    else if (nsl_strcmp(tmp, DATASET_TYPE_MOTORWAY_JUNCTION_OBJECT) == 0)
    {
        reply->contentType = NB_ECDT_MotorwayJunctionObject;
    }
    else if (nsl_strcmp(tmp, DATASET_TYPE_COMMON_TEXTURE) == 0)
    {
        reply->contentType = NB_ECDT_CommonTexture;
    }
    else if (nsl_strcmp(tmp, DATASET_TYPE_SPECIFIC_TEXTURE) == 0)
    {
        reply->contentType = NB_ECDT_SpecificTexture;
    }
    else if (nsl_strcmp(tmp, DATASET_TYPE_REALISTIC_SIGN) == 0)
    {
        reply->contentType = NB_ECDT_RealisticSign;
    }
    else if (nsl_strcmp(tmp, DATASET_TYPE_SPEED_SIGN) == 0)
    {
        reply->contentType = NB_ECDT_SpeedLimitsSign;
    }
    else if (nsl_strcmp(tmp, DATASET_TYPE_MAP_REGIONS_COMMON) == 0)
    {
        reply->contentType = NB_ECDT_MapRegionsCommon;
    }
    else if (nsl_strcmp(tmp, DATASET_TYPE_MAP_REGIONS) == 0)
    {
        reply->contentType = NB_ECDT_MapRegions;
    }
    else
    {
        reply->contentType = NB_ECDT_None;
    }

    // Set urlArgsTemplate.
    tmp = data_string_get(state, &urlArgsTemplate->template);
    if (!tmp)
    {
        return NE_NOENT;
    }
    nsl_memset(reply->urlArgsTemplate, 0, sizeof(reply->urlArgsTemplate));
    nsl_strlcpy(reply->urlArgsTemplate, tmp, MAX_ENHANCED_CONTENT_PATH_LENGTH + 1);

    // Set from data_metadata.
    length = CSL_VectorGetLength(urlArgsTemplate->vec_metadata);
    for (n = 0; n < length; ++n)
    {
        metadata = (data_metadata*)CSL_VectorGetPointer(urlArgsTemplate->vec_metadata, n);
        result = MetadataGetMetadataSourceReplies(state, metadata, reply, vectorReplies);
        if (result != NE_OK)
        {
            break;
        }
    }

    return result;
}

/* See declaration for description */
static NB_Error
MetadataGetMetadataSourceReplies(data_util_state* state, data_metadata* metadata, NB_MetadataSourceReply* reply, CSL_Vector* vectorReplies)
{
    const char* tmp = NULL;

    if (!state || !metadata || !reply || !vectorReplies)
    {
        return NE_INVAL;
    }

    // Set bounding box.
    reply->boundingLatitude1 = metadata->box.topLeft.lat;
    reply->boundingLongitude1 = metadata->box.topLeft.lon;
    reply->boundingLatitude2 = metadata->box.bottomRight.lat;
    reply->boundingLongitude2 = metadata->box.bottomRight.lon;

    // Set projection parameters.
    tmp = data_string_get(state, &metadata->projection_parameters.datum);
    if (!tmp)
    {
        return NE_NOENT;
    }
    if (nsl_strcmp(tmp, PROJECTION_DATUM_WGS84) == 0)
    {
        reply->datum = NB_ECPD_WGS84;
    }
    else if (nsl_strcmp(tmp, PROJECTION_DATUM_D_NORTH_AMERICAN_1983) == 0)
    {
        reply->datum = NB_ECPD_DNorthAmerican1983;
    }
    else
    {
        reply->datum = NB_ECPD_Invalid;
    }

    reply->originLatitude = metadata->projection_parameters.origin_latitude;
    reply->originLongitude = metadata->projection_parameters.origin_longitude;
    reply->scaleFactor = metadata->projection_parameters.scale_factor;
    reply->falseEasting = metadata->projection_parameters.false_easting;
    reply->falseNorthing = metadata->projection_parameters.false_northing;
    reply->zOffset = metadata->projection_parameters.z_offset;

    // Set status for force delete.
    reply->deleted = metadata->force_delete;

    // Set datasetId.
    tmp = data_string_get(state, &metadata->id);
    if (!tmp)
    {
        return NE_NOENT;
    }
    nsl_memset(reply->datasetId, 0, sizeof(reply->datasetId));
    nsl_strlcpy(reply->datasetId, tmp, MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1);

    // Set datasetSize.
    reply->datasetSize = metadata->size;

    // Set displayName.
    tmp = data_string_get(state, &metadata->display_name);
    if (!tmp)
    {
        return NE_NOENT;
    }
    nsl_memset(reply->displayName, 0, sizeof(reply->displayName));
    nsl_strcpy(reply->displayName, tmp);

    // Set manifestVersion.
    tmp = data_string_get(state, &metadata->manifest_version);
    if (!tmp)
    {
        return NE_NOENT;
    }
    nsl_memset(reply->manifestVersion, 0, sizeof(reply->manifestVersion));
    nsl_strcpy(reply->manifestVersion, tmp);

    if (!CSL_VectorAppend(vectorReplies, reply))
    {
        return NE_NOMEM;
    }

    return NE_OK;
}

static void
QaLogMetadataResponse(NB_MetadataInformation* information)
{
    if (NB_ContextGetQaLog(information->context))
    {
        data_util_state* dataState = NB_ContextGetDataState(information->context);
        
        uint32 contentSourceCount = CSL_VectorGetLength(information->reply.vec_content_source);
        uint32 contentSourceIndex = 0;
        
        for (contentSourceIndex = 0; contentSourceIndex < contentSourceCount; contentSourceIndex++)
        {
            data_content_source* contentSource = (data_content_source*)CSL_VectorGetPointer(information->reply.vec_content_source, contentSourceIndex);

            if (contentSource)
            {
                uint32 urlTemplateCount = CSL_VectorGetLength(contentSource->vec_url_args_template);
                uint32 urlTemplateIndex = 0;

                NB_QaLogMetadataSourceResponse(
                    information->context,
                    data_string_get(dataState, &information->reply.city_summary_display),
                    data_string_get(dataState, &contentSource->url.value),
                    data_string_get(dataState, &contentSource->projection),
                    urlTemplateCount
                );

                for (urlTemplateIndex = 0; urlTemplateIndex < urlTemplateCount; urlTemplateIndex++)
                {
                    data_url_args_template* urlTemplate = (data_url_args_template*)CSL_VectorGetPointer(contentSource->vec_url_args_template, urlTemplateIndex);

                    if (urlTemplate)
                    {
                        uint32 metadataCount = CSL_VectorGetLength(urlTemplate->vec_metadata);
                        uint32 metadataIndex = 0;

                        for (metadataIndex = 0; metadataIndex < metadataCount; metadataIndex++)
                        {
                            data_metadata* metadata  = (data_metadata*)CSL_VectorGetPointer(urlTemplate->vec_metadata, metadataIndex);

                            if (metadata)
                            {
                                NB_QaLogMetadataSourceTemplate(
                                    information->context,
                                    data_string_get(dataState, &urlTemplate->type),
                                    data_string_get(dataState, &urlTemplate->template),
                                    urlTemplate->cache_id,
                                    data_string_get(dataState, &metadata->id),
                                    data_string_get(dataState, &metadata->manifest_version),
                                    metadata->size,
                                    data_string_get(dataState, &metadata->display_name),
                                    metadata->box.topLeft.lat,
                                    metadata->box.topLeft.lon,
                                    metadata->box.bottomRight.lat,
                                    metadata->box.bottomRight.lon,
                                    data_string_get(dataState, &metadata->projection_parameters.datum),
                                    metadata->projection_parameters.false_easting,
                                    metadata->projection_parameters.false_northing,
                                    metadata->projection_parameters.origin_latitude,
                                    metadata->projection_parameters.origin_longitude,
                                    metadata->projection_parameters.scale_factor,
                                    metadata->projection_parameters.z_offset
                                );
                            }
                        }
                    }
                }
            }
        }
    }
}

/*! @} */
