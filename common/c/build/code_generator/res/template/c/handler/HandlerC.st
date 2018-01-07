HandlerC(handler) ::= <<
/*!--------------------------------------------------------------------------

    @file     nb<handler.name;format="toLower">handler.c
    @defgroup nbhandler Handler
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


#include "<FileName(handler.name, "handler", "h")>"
#include "<FileName(handler.name, "informationprivate", "h")>"
#include "<FileName(handler.name, "parametersprivate", "h")>"
#include "nbcontextprotected.h"
#include "cslnetwork.h"
<handler.requests: {request| <request.responses: {response| #include "<response.wireName;format="dashToUnderscore">.h" }>}>
#include "nbutilityprotected.h"


struct <handler.name>Handler_Query
{
	NB_NetworkQuery                     networkQuery;
	NB_<handler.name>Handler*           pThis;
};


struct NB_<handler.name>Handler
{
    NB_Context*                         context;
    NB_RequestHandlerCallback           callback;
    boolean                             inProgress;
    struct <handler.name>Handler_Query  query;
    tpselt                              tpsReply;
};


static void <handler.name>Handler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);
static void <handler.name>Handler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);


NB_DEF NB_Error
NB_<handler.name>HandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_<handler.name>Handler** handler)
{
	NB_<handler.name>Handler* pThis = 0;
    
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
    pThis->query.networkQuery.action = <handler.name>Handler_NetworkCallback;
	pThis->query.networkQuery.progress = <handler.name>Handler_NetworkProgressCallback;
	pThis->query.networkQuery.qflags = TN_QF_DEFLATE;
	pThis->query.networkQuery.failFlags = TN_FF_CLOSE_ANY;
    nsl_strcpy(pThis->query.networkQuery.target, NB_ContextGetTargetMapping(pThis->context, "reverse-geocode"));
    pThis->query.networkQuery.targetlen = -1;

    *handler = pThis;

	return NE_OK;
}


NB_DEF NB_Error
NB_<handler.name>HandlerDestroy(NB_<handler.name>Handler* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->inProgress)
    {
        NB_<handler.name>HandlerCancelRequest(pThis);
    }

    if (pThis->query.networkQuery.query)
    {
	    te_dealloc(pThis->query.networkQuery.query);
    }

	nsl_free(pThis);

    return NE_OK;
}



NB_DEF NB_Error
NB_<handler.name>HandlerStartRequest(NB_<handler.name>Handler* pThis, NB_<handler.name>Parameters* parameters)
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

    if (pThis->query.networkQuery.query)
    {
	    te_dealloc(pThis->query.networkQuery.query);
    }

    pThis->query.networkQuery.query = NB_<handler.name>ParametersToTPSQuery(parameters);
    if (!pThis->query.networkQuery.query)
    {
        return NE_NOMEM;
    }

    pThis->inProgress = TRUE;

    CSL_NetworkRequest(NB_ContextGetNetwork(pThis->context), &pThis->query.networkQuery);

    return err;
}


NB_DEF NB_Error
NB_<handler.name>HandlerCancelRequest(NB_<handler.name>Handler* pThis)
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
NB_<handler.name>HandlerIsRequestInProgress(NB_<handler.name>Handler* pThis)
{
    if (pThis)
    {
        return pThis->inProgress;
    }

    return 0;
}


<handler.requests: {request| <request.responses: {response| 
NB_DEF NB_Error
NB_<handler.name>HandlerGet<response.type.name>(NB_<handler.name>Handler* pThis, NB_<response.type.name>** information)
{
    if (!pThis || !information)
    {
        return NE_INVAL;
    \}

    if (!pThis->tpsReply)
    {
        return NE_UNEXPECTED;
    \}

    return NB_<response.type.name>Create(pThis->context, pThis->tpsReply, information);
\}
}>}>

void
<handler.name>Handler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
{
	NB_Error err = NE_OK;
	NB_<handler.name>Handler* pThis = ((struct <handler.name>Handler_Query*)query)->pThis;
	NB_NetworkRequestStatus status;
	uint32 replyerr = NE_OK;

	replyerr = GetReplyErrorCode(resp);

	if (resp->status == COMPLETE && err != NE_OK)
    {
		status = NB_NetworkRequestStatus_Failed;
	}
    else
    {
        status = TranslateNetStatus(resp->status);
    }

	pThis->inProgress = FALSE;
	query->query = NULL;

    if (pThis->callback.callback != NULL)
    {
        // Make reply available for duration of callback
        pThis->tpsReply = resp->reply;
        (pThis->callback.callback)(pThis, status, replyerr ? replyerr : err, FALSE, 100, pThis->callback.callbackData);
        pThis->tpsReply = 0;
    }
}


void
<handler.name>Handler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
	NB_<handler.name>Handler* pThis = ((struct <handler.name>Handler_Query*) query)->pThis;

    if (pThis->callback.callback)
    {
		int percent = (cur * 100) / total;
        (pThis->callback.callback)(pThis, NB_NetworkRequestStatus_Progress, NE_OK, up, percent, pThis->callback.callbackData);
	}
}


/*! @} */


>>