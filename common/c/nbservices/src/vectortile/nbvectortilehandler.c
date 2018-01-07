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

@file     vectortilehandler.c
@defgroup vectortilehandler vector tile handler implementation


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

#include "palclock.h"
#include "nbcontextprotected.h"
#include "nbvectortilehandler.h"
#include "nbvectortileparameters.h"
#include "nbvectortileparametersprivate.h"
#include "nbvectortileinformation.h"
#include "nbvectortileinformationprivate.h"

#include "cslnetwork.h"
#include "nbutility.h"
#include "data_vector_tile_query.h"

/*! @{ */

// Private functions .............................................................................

static void VectorTileHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);
static void VectorTileHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);


// Local types ...................................................................................


struct VectorTileHandler_Query
{
    NB_NetworkQuery          networkQuery;
    NB_VectorTileHandler*    pThis;
};


struct _NB_VectorTileHandler
{
    NB_Context*					    context;
    NB_RequestHandlerCallback       callback;
    nb_boolean                      inProgress;
    struct VectorTileHandler_Query  query;
    tpselt                          tpsReply;
    uint32                          passes;
    uint32                          zoomLevel;

};




// Public functions ..............................................................................

/* See header file for description. */
NB_DEF NB_Error
NB_VectorTileHandlerCreate(
    NB_Context* context,
    NB_RequestHandlerCallback* callback,
    NB_VectorTileHandler** handler
    )
{
    NB_VectorTileHandler* pThis = NULL;

    if (!context || !callback || !handler)
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

    if (callback)
    {
        pThis->callback.callback = callback->callback;
        pThis->callback.callbackData = callback->callbackData;
    }

    pThis->query.pThis = pThis;
    pThis->query.networkQuery.action = VectorTileHandler_NetworkCallback;
    pThis->query.networkQuery.progress = VectorTileHandler_NetworkProgressCallback;
    pThis->query.networkQuery.qflags = TN_QF_DEFLATE;
    pThis->query.networkQuery.failFlags = TN_FF_CLOSE_ANY;
    nsl_strlcpy(pThis->query.networkQuery.target, NB_ContextGetTargetMapping(pThis->context, "vector-tile"), sizeof(pThis->query.networkQuery.target));
    pThis->query.networkQuery.targetlen = -1;

    *handler = pThis;
    return NE_OK;
}    


/* See header file for description. */
NB_DEF NB_Error
NB_VectorTileHandlerDestroy(
    NB_VectorTileHandler* pThis
    )
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->inProgress)
    {
        NB_VectorTileHandlerCancelRequest(pThis);
    }

    te_dealloc(pThis->query.networkQuery.query);
	nsl_free(pThis);

    return NE_OK;
}    


/* See header file for description. */
NB_DEF NB_Error 
NB_VectorTileHandlerStartRequest(
    NB_VectorTileHandler* pThis,
    NB_VectorTileParameters* parameters
    )
{
    NB_Error err = NE_OK;
	NB_VectorTileParametersConfiguration* config = NULL;
	data_vector_tile_query vq = { 0 };

    if (!pThis || !parameters)
    {
        return NE_INVAL;
    }

    if (pThis->inProgress)
    {
        return NE_BUSY;
    }

    // Needed in tile information create
    NB_VectorTileParametersGetConfigurationParameters(parameters, &config);
    pThis->passes = config->passes;
    pThis->zoomLevel = config->zoomLevel;
    
    te_dealloc(pThis->query.networkQuery.query);
    
    pThis->query.networkQuery.query = NB_VectorTileParametersToTPSQuery(parameters, &vq);
    if (!pThis->query.networkQuery.query)
    {
        return NE_NOMEM;
    }
    pThis->inProgress = TRUE;

    CSL_NetworkRequest(NB_ContextGetNetwork(pThis->context), &pThis->query.networkQuery);

	data_vector_tile_query_free(NB_ContextGetDataState(pThis->context), &vq);

    return err;
    
}    



/* See header file for description. */
NB_DEF NB_Error 
NB_VectorTileHandlerCancelRequest(
    NB_VectorTileHandler* pThis
    )
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


/* See header file for description. */
NB_DEF nb_boolean 
NB_VectorTileHandlerIsRequestInProgress(
    NB_VectorTileHandler* pThis
    )
{
    if (pThis)
    {
        return pThis->inProgress;
    }

    return 0;
}    


/* See header file for description. */
NB_DEF NB_Error
NB_VectorTileHandlerGetTileInformation(
    NB_VectorTileHandler* pThis,
    NB_VectorTileInformation** information
    )
{
	if (!pThis || !information)
	{
		return NE_INVAL;
	}

    if (!pThis->tpsReply)
    {
        return NE_UNEXPECTED;
    }

    return NB_VectorTileInformationCreate(pThis->context, pThis->zoomLevel, pThis->passes, pThis->tpsReply, information);
}    


// Local functions ...............................................................................    

void 
VectorTileHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
    if (!up)
    {
        NB_VectorTileHandler* pThis = ((struct VectorTileHandler_Query*) query)->pThis;

        if (pThis->callback.callback)
        {
            int percent = (cur * 100) / total;
            (pThis->callback.callback)(pThis, NB_NetworkRequestStatus_Progress, NE_OK, 0, percent, pThis->callback.callbackData);
        }
    }
}

void 
VectorTileHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
{
    NB_VectorTileHandler* pThis = ((struct VectorTileHandler_Query*)query)->pThis;
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



/*! @} */
