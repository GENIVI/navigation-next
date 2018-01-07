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

    @file     nbrastermaphandler.c
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
    @addtogroup nbrastermaphandler
    @{
*/

#include "nbrastermaphandler.h"
#include "nbrastermapparametersprivate.h"
#include "nbrastermapinformationprivate.h"
#include "nbsearchinformationprivate.h"
#include "nbcontextprotected.h"
#include "cslnetwork.h"
#include "data_map_query.h"
#include "data_map_reply.h"
#include "nbutilityprotected.h"


struct RasterMapHandler_Query
{
    NB_NetworkQuery                 networkQuery;
    NB_RasterMapHandler*            pThis;
};


struct NB_RasterMapHandler
{
    NB_Context*                     context;
    NB_RequestHandlerCallback       callback;
    boolean                         inProgress;
    struct RasterMapHandler_Query   query;
    tpselt                          tpsReply;

    data_map_query                  lastQuery;
    NB_ImageOffset                  pOffset;
    NB_ImageSize                    poiSize;
    NB_Image*                       poiImage;
};


static void RasterMapHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);
static void RasterMapHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);


NB_DEF NB_Error
NB_RasterMapHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_RasterMapHandler** handler)
{
    NB_RasterMapHandler* pThis = 0;

    if (!context || !handler)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (pThis == NULL)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;
    if (callback)
    {
        pThis->callback.callback = callback->callback;
        pThis->callback.callbackData = callback->callbackData;
    }

    pThis->query.pThis = pThis;
    pThis->query.networkQuery.action = RasterMapHandler_NetworkCallback;
    pThis->query.networkQuery.progress = RasterMapHandler_NetworkProgressCallback;
    pThis->query.networkQuery.qflags = TN_QF_DEFLATE;
    pThis->query.networkQuery.failFlags = TN_FF_CLOSE_ANY;
    nsl_strlcpy(pThis->query.networkQuery.target, NB_ContextGetTargetMapping(pThis->context, "map"), sizeof(pThis->query.networkQuery.target));
    pThis->query.networkQuery.targetlen = -1;

    pThis->poiImage = NULL;

    *handler = pThis;

    return NE_OK;
}


NB_DEF NB_Error
NB_RasterMapHandlerDestroy(NB_RasterMapHandler* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->inProgress)
    {
        NB_RasterMapHandlerCancelRequest(pThis);
    }

    te_dealloc(pThis->query.networkQuery.query);
    data_map_query_free(NB_ContextGetDataState(pThis->context), &pThis->lastQuery);

    if( pThis->poiImage )
    
    data_map_query_free(NB_ContextGetDataState(pThis->context), &pThis->lastQuery);

    nsl_free(pThis);

    return NE_OK;
}


NB_DEF NB_Error
NB_RasterMapHandlerStartRequest(NB_RasterMapHandler* pThis, NB_RasterMapParameters* parameters)
{
    NB_Error err = NE_OK;

    if (!pThis || !parameters)
    {
        return NE_INVAL;
    }

    if (pThis->inProgress)
    {
        return NE_BUSY;
    }

    err = NB_RasterMapParametersCopyQuery(parameters, &pThis->lastQuery);

    if (err != NE_OK)
        return err;

    te_dealloc(pThis->query.networkQuery.query);

    pThis->query.networkQuery.query = NB_RasterMapParametersToTPSQuery(parameters);
    if (!pThis->query.networkQuery.query)
    {
        return NE_NOMEM;
    }
    pThis->inProgress = TRUE;

    if( parameters->poiImage )
    {
        pThis->poiImage = (NB_Image*)nsl_malloc(sizeof(NB_Image));
        pThis->poiImage->data = (uint8*)nsl_malloc(parameters->poiImage->dataLen);
        nsl_memcpy(pThis->poiImage->data, parameters->poiImage->data, parameters->poiImage->dataLen);
        pThis->poiImage->dataLen = parameters->poiImage->dataLen;
        nsl_memcpy(&pThis->pOffset, &parameters->pOffset, sizeof(NB_ImageOffset));
        nsl_memcpy(&pThis->poiSize, &parameters->poiSize, sizeof(NB_ImageSize));
    }

    CSL_NetworkRequest(NB_ContextGetNetwork(pThis->context), &pThis->query.networkQuery);

    return err;
}


NB_DEF NB_Error
NB_RasterMapHandlerCancelRequest(NB_RasterMapHandler* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (!pThis->inProgress)
    {
        return NE_UNEXPECTED;
    }

    CSL_NetworkCancel(NB_ContextGetNetwork(pThis->context), &pThis->query.networkQuery);

    return NE_OK;
}


NB_DEF nb_boolean
NB_RasterMapHandlerIsRequestInProgress(NB_RasterMapHandler* pThis)
{
    if (pThis)
    {
        return pThis->inProgress;
    }

    return 0;
}


NB_DEF NB_Error
NB_RasterMapHandlerGetMapInformation(NB_RasterMapHandler* pThis, NB_RasterMapInformation** information)
{
    NB_Error err = NE_OK;

    if (!pThis || !information)
    {
        return NE_INVAL;
    }

    if (!pThis->tpsReply)
    {
        return NE_UNEXPECTED;
    }

    err = NB_RasterMapInformationPoiCreate(pThis->context, pThis->tpsReply, &pThis->lastQuery,
                                           pThis->poiImage, pThis->poiSize, pThis->pOffset, information);

    return err;
}

NB_DEF NB_Error
NB_RasterMapHandlerGetSearchInformation(NB_RasterMapHandler* pThis, nb_boolean* hasTrafficIncidents, NB_SearchInformation** searchInformation)
{
    NB_Error err = NE_OK;
    data_map_reply reply = {{0}};

    if (!pThis || !searchInformation)
    {
        return NE_INVAL;
    }

    if (!pThis->tpsReply)
    {
        return NE_UNEXPECTED;
    }

    err = err ? err : data_map_reply_from_tps(NB_ContextGetDataState(pThis->context), &reply, pThis->tpsReply);
    *hasTrafficIncidents = reply.hastraffic;
    data_map_reply_free(NB_ContextGetDataState(pThis->context), &reply);

    err = NB_SearchInformationCreateFromTPSReply(pThis->tpsReply, 0, pThis->context, searchInformation);

    return err;
}

void
RasterMapHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
{
    NB_RasterMapHandler* pThis = ((struct RasterMapHandler_Query*)query)->pThis;
    NB_NetworkRequestStatus status;

    uint32 replyerr = NE_OK;

    replyerr = GetReplyErrorCode(resp);

    status = TranslateNetStatus(resp->status);

    pThis->inProgress = 0;
    query->query = NULL;

    if (pThis->callback.callback != NULL)
    {
        // Make reply available for duration of callback
        pThis->tpsReply = resp->reply;
        (pThis->callback.callback)(pThis, status, replyerr, 0, 100, pThis->callback.callbackData);
        pThis->tpsReply = 0;
    }
}


void
RasterMapHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
    if (!up)
    {
        NB_RasterMapHandler* pThis = ((struct RasterMapHandler_Query*) query)->pThis;

        if (pThis->callback.callback)
        {
            int percent = (cur * 100) / total;
            (pThis->callback.callback)(pThis, NB_NetworkRequestStatus_Progress, NE_OK, 0, percent, pThis->callback.callbackData);
        }
    }
}

/*! @} */
