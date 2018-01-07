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

    @file nbmanifestinformation.c
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
#include "nbmanifestinformation.h"
#include "nbmetadatatypes.h"
#include "data_metadata_manifest_reply.h"
#include "data_dataset.h"
#include "data_content_details.h"
#include "nbqalog.h"

// Constants .....................................................................................


// Local types ...................................................................................

/*! The manifest information */
struct NB_ManifestInformation
{
    NB_Context*                     context;
    data_metadata_manifest_reply    reply;
};


// Local Functions ...............................................................................

/*! Get the metadata manifest replies from data_dataset

@param state An data_util_state object
@param dataset An data_dataset object
@param reply An NB_MetadataManifestReply object set from data_dataset
@param vectorReplies vector of NB_MetadataManifestReply added by reply
@returns NB_Error
*/
static NB_Error DatasetGetMetadataManifestReplies(data_util_state* state, data_dataset* dataset, NB_MetadataManifestReply* reply, CSL_Vector* vectorReplies);

/*! Get the metadata manifest replies from data_content_details

@param state An data_util_state object
@param metadata An data_content_details object
@param reply An NB_MetadataManifestReply object set from data_content_details
@param vectorReplies vector of NB_MetadataManifestReply added by reply
@returns NB_Error
*/
static NB_Error ContentDetailsGetMetadataManifestReplies(data_util_state* state, data_content_details* contentDetails, NB_MetadataManifestReply* reply, CSL_Vector* vectorReplies);

static void QaLogManifestResponse(NB_ManifestInformation* manifest);


// Public functions ..............................................................................

/* See header file for description. */
NB_Error
NB_ManifestInformationCreate(NB_Context* context, tpselt reply, NB_ManifestInformation** information)
{
    NB_ManifestInformation* pThis = 0;
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
    result = result ? result : data_metadata_manifest_reply_from_tps(NB_ContextGetDataState(pThis->context), &pThis->reply, reply);

    if (!result)
    {
        QaLogManifestResponse(pThis);

        *information = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return result;
}

/* See header file for description. */
NB_Error
NB_ManifestInformationDestroy(NB_ManifestInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_metadata_manifest_reply_free(NB_ContextGetDataState(pThis->context), &pThis->reply);

    nsl_free(pThis);

    return NE_OK;
}

/* See header file for description. */
NB_Error
NB_ManifestInformationGetMetadataManifestReplies(NB_ManifestInformation* information, int64 metadataId, CSL_Vector** vectorReplies)
{
    NB_Error result = NE_OK;
    uint32 length = 0;
    uint32 n = 0;
    CSL_Vector* replies = NULL;
    data_dataset* dataset = NULL;
    NB_MetadataManifestReply reply = {0};

    if (!information || !vectorReplies)
    {
        return NE_INVAL;
    }

    // Alloc vector of metadata manifest replies.
    replies = CSL_VectorAlloc(sizeof(NB_MetadataManifestReply));
    if (!replies)
    {
        return NE_NOMEM;
    }

    reply.metadataId = metadataId;

    // Set from data_dataset.
    length = CSL_VectorGetLength(information->reply.vec_dataset);
    for (n = 0; n < length; ++n)
    {
        dataset = (data_dataset*)CSL_VectorGetPointer(information->reply.vec_dataset, n);
        result = DatasetGetMetadataManifestReplies(NB_ContextGetDataState(information->context), dataset, &reply, replies);
        if (result != NE_OK)
        {
            CSL_VectorDealloc(replies);
            return result;
        }
    }

    *vectorReplies = replies;

    return NE_OK;
}


// Local Functions ...............................................................................

/* See declaration for description */
static NB_Error
DatasetGetMetadataManifestReplies(data_util_state* state, data_dataset* dataset, NB_MetadataManifestReply* reply, CSL_Vector* vectorReplies)
{
    NB_Error result = NE_OK;
    uint32 length = 0;
    uint32 n = 0;
    data_content_details* contentDetails = NULL;

    if (!state || !dataset || !reply || !vectorReplies)
    {
        return NE_INVAL;
    }

    // Set from data_url_args_template.
    length = CSL_VectorGetLength(dataset->vec_content_details);
    for (n = 0; n < length; ++n)
    {
        contentDetails = (data_content_details*)CSL_VectorGetPointer(dataset->vec_content_details, n);
        result = ContentDetailsGetMetadataManifestReplies(state, contentDetails, reply, vectorReplies);
        if (result != NE_OK)
        {
            break;
        }
    }

    return result;
}

/* See declaration for description */
static NB_Error
ContentDetailsGetMetadataManifestReplies(data_util_state* state, data_content_details* contentDetails, NB_MetadataManifestReply* reply, CSL_Vector* vectorReplies)
{
    const char* tmp = NULL;

    if (!state || !contentDetails || !reply || !vectorReplies)
    {
        return NE_INVAL;
    }

    // Set bounding box.
    reply->boundingLatitude1 = contentDetails->box.topLeft.lat;
    reply->boundingLongitude1 = contentDetails->box.topLeft.lon;
    reply->boundingLatitude2 = contentDetails->box.bottomRight.lat;
    reply->boundingLongitude2 = contentDetails->box.bottomRight.lon;

    // Set fileId.
    tmp = data_string_get(state, &contentDetails->file_id);
    if (!tmp)
    {
        return NE_NOENT;
    }
    nsl_memset(reply->fileId, 0, sizeof(reply->fileId));
    nsl_strlcpy(reply->fileId, tmp, sizeof(reply->fileId));

    // Set fileVersion.
    tmp = data_string_get(state, &contentDetails->version);
    if (!tmp)
    {
        return NE_NOENT;
    }
    nsl_memset(reply->fileVersion, 0, sizeof(reply->fileVersion));
    nsl_strlcpy(reply->fileVersion, tmp, sizeof(reply->fileVersion));

    // Set fileSize.
    reply->fileSize = contentDetails->size;

    if (!CSL_VectorAppend(vectorReplies, reply))
    {
        return NE_NOMEM;
    }

    return NE_OK;
}

static void
QaLogManifestResponse(NB_ManifestInformation* information)
{
    if (NB_ContextGetQaLog(information->context))
    {
        data_util_state* dataState = NB_ContextGetDataState(information->context);

        uint32 datasetCount = CSL_VectorGetLength(information->reply.vec_dataset);
        uint32 datasetIndex = 0;
        
        for (datasetIndex = 0; datasetIndex < datasetCount; datasetIndex++)
        {
            data_dataset* dataset = (data_dataset*)CSL_VectorGetPointer(information->reply.vec_dataset, datasetIndex);
            
            if (dataset)
            {
                uint32 contentDetailsCount = CSL_VectorGetLength(dataset->vec_content_details);
                uint32 contentDetailsIndex = 0;

                data_content_details* firstContentDetails = (data_content_details*)CSL_VectorGetPointer(dataset->vec_content_details, 0);
                data_content_details* lastContentDetails = (data_content_details*)CSL_VectorGetPointer(dataset->vec_content_details, contentDetailsCount - 1);

                if (firstContentDetails && lastContentDetails)
                {
                    NB_QaLogManifestResponse(
                        information->context,
                        data_string_get(dataState, &dataset->dataset_id),
                        contentDetailsCount,
                        data_string_get(dataState, &firstContentDetails->file_id),
                        firstContentDetails->box.topLeft.lat, firstContentDetails->box.topLeft.lon,
                        firstContentDetails->box.bottomRight.lat, firstContentDetails->box.bottomRight.lon,
                        data_string_get(dataState, &lastContentDetails->file_id),
                        lastContentDetails->box.topLeft.lat, lastContentDetails->box.topLeft.lon,
                        lastContentDetails->box.bottomRight.lat, lastContentDetails->box.bottomRight.lon
                    );

                    if (CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(information->context)))
                    {
                        for (contentDetailsIndex = 0; contentDetailsIndex < contentDetailsCount; contentDetailsIndex++)
                        {
                            data_content_details* contentDetails = (data_content_details*)CSL_VectorGetPointer(dataset->vec_content_details, contentDetailsIndex);
                            
                            if (contentDetails)
                            {
                                NB_QaLogManifestResponseContent(
                                    information->context,
                                    data_string_get(dataState, &contentDetails->file_id),
                                    data_string_get(dataState, &contentDetails->version),
                                    contentDetails->box.topLeft.lat, contentDetails->box.topLeft.lon,
                                    contentDetails->box.bottomRight.lat, contentDetails->box.bottomRight.lon
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
