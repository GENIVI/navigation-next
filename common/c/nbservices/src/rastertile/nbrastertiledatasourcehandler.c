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

    @file     nbrastertiledatasourcehandler.c
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


#include "nbrastertiledatasourcehandler.h"
#include "nbrastertiledatasourceinformationprivate.h"
#include "cslnetwork.h"
#include "data_raster_tile_data_source_query.h"
#include "nbcontextprotected.h"
#include "nbutility.h"
#include "nbqalog.h"
#include "paldisplay.h"


// Local Constants ...............................................................................

// Server servlet used for data source 
#define SERVER_SERVLET "maptile-source"


// Local Types ...................................................................................

typedef struct 
{
	NB_NetworkQuery                     networkQuery;
	NB_RasterTileDataSourceHandler*     pThis;

} NetworkQuery;


struct NB_RasterTileDataSourceHandler
{
    NB_Context*                         context;
    NB_RequestHandlerCallback           callback;
    boolean                             inProgress;
    NetworkQuery                        query;
    tpselt                              tpsReply;
};


// Local functions ...............................................................................

static void NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* response);
static void NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);


// Public functions ..............................................................................

/* See header file for description */
NB_DEF NB_Error 
NB_RasterTileDataSourceHandlerCreate(NB_Context* context,
                                     NB_RequestHandlerCallback* callback,
                                     NB_RasterTileDataSourceHandler** handler)
{
	NB_RasterTileDataSourceHandler* pThis = NULL;
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
    pThis->query.networkQuery.action = &NetworkCallback;
	pThis->query.networkQuery.progress = &NetworkProgressCallback;
	pThis->query.networkQuery.qflags = TN_QF_DEFLATE;
	pThis->query.networkQuery.failFlags = TN_FF_CLOSE_ANY;
    nsl_strlcpy(pThis->query.networkQuery.target, SERVER_SERVLET, sizeof(pThis->query.networkQuery.target));
    pThis->query.networkQuery.targetlen = -1;

    *handler = pThis;
	return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error 
NB_RasterTileDataSourceHandlerDestroy(NB_RasterTileDataSourceHandler* handler)
{
    if (!handler)
    {
        return NE_INVAL;
    }

    if (handler->inProgress)
    {
        NB_RasterTileDataSourceHandlerCancelRequest(handler);
    }

    te_dealloc(handler->query.networkQuery.query);
	nsl_free(handler);
    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error 
NB_RasterTileDataSourceHandlerStartRequest(NB_RasterTileDataSourceHandler* handler,
                                           NB_RasterTileDataSourceParameters* parameters)
{
    uint32 screenWidth = 0;
    uint32 screenHeight = 0;
    uint32 screenResolution = 0;

    if (!handler || !parameters)
    {
        return NE_INVAL;
    }

    if (handler->inProgress)
    {
        return NE_BUSY;
    }

    te_dealloc(handler->query.networkQuery.query);

    // Get display info
    PAL_DisplayGetInfo(PD_Main, &screenWidth, &screenHeight, &screenResolution);

    // Create tps query 
    handler->query.networkQuery.query = data_raster_tile_data_source_query_create_tps(screenWidth, screenHeight, screenResolution);
    if (!handler->query.networkQuery.query)
    {
        return NE_NOMEM;
    }

    // Start network query. Network callbacks will be called asynchronously.
    handler->inProgress = TRUE;
    CSL_NetworkRequest(NB_ContextGetNetwork(handler->context), &handler->query.networkQuery);

    NB_QaLogDTSRequest(handler->context);

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error 
NB_RasterTileDataSourceHandlerCancelRequest(NB_RasterTileDataSourceHandler* handler)
{
    if (!handler)
    {
        return NE_INVAL;
    }

    if (!handler->inProgress)
    {
        return NE_UNEXPECTED;
    }

	CSL_NetworkCancel(NB_ContextGetNetwork(handler->context), &handler->query.networkQuery);

    return NE_OK;
}

/* See header file for description */
NB_DEF nb_boolean 
NB_RasterTileDataSourceHandlerIsRequestInProgress(NB_RasterTileDataSourceHandler* handler)
{
    if (handler)
    {
        return handler->inProgress;
    }

    return 0;
}

/* See header file for description */
NB_DEF NB_Error 
NB_RasterTileDataSourceHandlerGetInformation(NB_RasterTileDataSourceHandler* handler,
                                             NB_RasterTileDataSourceInformation** information)
{
    if (!handler || !information)
    {
        return NE_INVAL;
    }

    if (!handler->tpsReply)
    {
        return NE_UNEXPECTED;
    }

    // Create a information object based on the server tps reply and return it.
    return NB_RasterTileDataSourceInformationCreate(handler->context, handler->tpsReply, information);
}

/*! Network callback.

    Gets called once the download is completed.

    @return None
*/
void
NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* response)
{
	NB_RasterTileDataSourceHandler* pThis = ((NetworkQuery*)query)->pThis;
	NB_NetworkRequestStatus status = NB_NetworkRequestStatus_Failed;
	uint32 replyerr = NE_OK;

	replyerr = GetReplyErrorCode(response);

    status = TranslateNetStatus(response->status);

	pThis->inProgress = FALSE;
	query->query = NULL;

    if (pThis->callback.callback != NULL)
    {
        // Make reply available for duration of callback
        pThis->tpsReply = response->reply;
        (pThis->callback.callback)(pThis, status, replyerr, FALSE, 100, pThis->callback.callbackData);
        pThis->tpsReply = 0;
    }
}

/*! Progress callback for network call.

    Not all server calls have a progress.

    @return None
*/
void
NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
	NB_RasterTileDataSourceHandler* pThis = ((NetworkQuery*) query)->pThis;

    if (pThis->callback.callback)
    {
        // Forward the progress to the user callback
		int percent = (cur * 100) / total;
        (pThis->callback.callback)(pThis, NB_NetworkRequestStatus_Progress, NE_OK, up, percent, pThis->callback.callbackData);
	}
}



/*! @} */
