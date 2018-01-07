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

    @file abservermessageparameters.c
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

/*! @{ */


#include "nbcontextprotected.h"
#include "abservermessageparameters.h"
#include "abservermessageparametersprivate.h"
#include "data_message_confirm_query.h"

#define MAXIMUM_LANGUAGE_LENGTH     7

struct AB_ServerMessageParameters
{
    NB_Context*                 context;

    AB_ServerMessageQueryType   queryType;
    char                        language[MAXIMUM_LANGUAGE_LENGTH + 1];
    uint64                      timeStamp;

    data_message_confirm_query  confirmQuery;
    AB_MessageParameterConfiguration config;
};


static NB_Error CreateThis(NB_Context* context, AB_ServerMessageQueryType queryType, const char* language, uint64 timestamp, AB_MessageParameterConfiguration config, AB_ServerMessageParameters** parameters);


AB_DEF NB_Error
AB_ServerMessageParametersCreateStatus(NB_Context* context, const char* language, uint64 timestamp, AB_ServerMessageParameters** parameters)
{
    return CreateThis(context, ASMQT_MessageStatus, language, timestamp, AB_None, parameters);
}


AB_DEF NB_Error
AB_ServerMessageParametersCreateQuery(NB_Context* context, const char* language, uint64 timestamp, AB_ServerMessageParameters** parameters)
{
    return CreateThis(context, ASMQT_MessageQuery, language, timestamp, AB_None, parameters);
}
AB_DEF NB_Error
AB_ServerMessageParametersNotificationCreateQuery(NB_Context* context, const char* language, uint64 timestamp, AB_MessageParameterConfiguration config, AB_ServerMessageParameters** parameters)
{
    return CreateThis(context, ASMQT_MessageQuery, language, timestamp, config, parameters);
}

AB_DEF NB_Error
AB_ServerMessageParametersCreateConfirm(NB_Context* context, const char* messageId, const char* action, AB_ServerMessageParameters** parameters)
{
    AB_ServerMessageParameters* pThis = 0;
    NB_Error err = NE_OK;
    data_util_state* dataState = 0;
    data_message_confirm mc = { 0 };

    if (!context || !messageId || !action || !parameters)
    {
        return NE_INVAL;
    }

    err = CreateThis(context, ASMQT_MessageConfirm, "", 0, AB_None, &pThis);
    if (err)
    {
        return err;
    }

    dataState = NB_ContextGetDataState(context);
    err = data_message_confirm_init(dataState, &mc);
    err = err ? err : data_string_set(dataState, &mc.id, messageId);
    err = err ? err : data_string_set(dataState, &mc.confirm_action, action);
    err = err ? err : data_message_confirm_query_add_message_confirm(dataState, &pThis->confirmQuery, &mc);

    if (!err)
    {
        *parameters = pThis;
    }
    else
    {
        AB_ServerMessageParametersDestroy(pThis);
    }

    return err;
}


AB_DEF NB_Error
AB_ServerMessageParametersCreateReview(NB_Context* context, const char* messageType, AB_ServerMessageParameters** parameters)
{
    AB_ServerMessageParameters* pThis = 0;
    NB_Error err = NE_OK;
    data_util_state* dataState = 0;
    data_retrieve_confirmed rc = { 0 };

    if (!context || !messageType || !parameters)
    {
        return NE_INVAL;
    }

    err = CreateThis(context, ASMQT_MessageConfirm, "", 0, AB_None, &pThis);
    if (!err)
    {
        *parameters = pThis;
    }
    else
    {
        return err;
    }

    dataState = NB_ContextGetDataState(context);
    err = data_retrieve_confirmed_init(dataState, &rc);
    err = err ? err : data_string_set(dataState, &rc.type, messageType);
    err = err ? err : data_message_confirm_query_add_retrieve_confirmed(dataState, &pThis->confirmQuery, &rc);

    if (err)
    {
        AB_ServerMessageParametersDestroy(pThis);
    }

    return err;
}


AB_DEF NB_Error
AB_ServerMessageParametersDestroy(AB_ServerMessageParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_message_confirm_query_free(NB_ContextGetDataState(pThis->context), &pThis->confirmQuery);

    nsl_free(pThis);

    return NE_OK;
}


AB_ServerMessageQueryType
AB_ServerMessageParametersGetQueryType(AB_ServerMessageParameters* pThis)
{
    return pThis->queryType;
}


tpselt
AB_ServerMessageParametersToTPSQuery(AB_ServerMessageParameters* pThis)
{
    tpselt te = 0;
    data_util_state* dataState = 0;

    if (!pThis)
    {
        return 0;
    }

    dataState = NB_ContextGetDataState(pThis->context);
    
    switch (pThis->queryType)
    {
        case ASMQT_MessageQuery:
            te = te_new("message-query");

            if (te != NULL)
            {
                if (!te_setattrc(te, "language", pThis->language))
                {
                    goto errexit;
                }

                if(!te_setattru64(te, "ts", pThis->timeStamp))
                {
                    goto errexit;
                }
                if(pThis->config & AB_WantNotification)
                {
                    //Add a child Element
                    tpselt childElement = 0;
                    childElement = te_new("want-message-notification");
                    
                    if(childElement != NULL)
                    {
                        if (te_attach(te, childElement))
                        {
                            childElement = NULL;
                        }
                        else
                        {
                            goto errexit;
                        }
                        
                    }
                }
            }
            break;

        case ASMQT_MessageConfirm:
            te = data_message_confirm_query_to_tps(dataState, &pThis->confirmQuery);
            break;
            
        case ASMQT_MessageStatus:
            te = te_new("message-status-query");

            if (te != NULL)
            {
                if (!te_setattrc(te, "language", pThis->language))
                {
                    goto errexit;
                }
                if(!te_setattru64(te, "ts", pThis->timeStamp))
                {
                    goto errexit;
                }
            }
            break;
    }

    return te;

errexit:
    te_dealloc(te);
    return 0;
}


NB_Error
CreateThis(NB_Context* context, AB_ServerMessageQueryType queryType, const char* language, uint64 timestamp, AB_MessageParameterConfiguration config, AB_ServerMessageParameters** parameters)
{
    AB_ServerMessageParameters* pThis = 0;
    NB_Error err = NE_OK;
    data_util_state* dataState = 0;

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;
    pThis->queryType = queryType;
    pThis->timeStamp = timestamp;
    pThis->config = config;
    nsl_strlcpy(pThis->language, language, MAXIMUM_LANGUAGE_LENGTH);

    dataState = NB_ContextGetDataState(context);
    err = err ? err : data_message_confirm_query_init(dataState, &pThis->confirmQuery);

    if (!err)
    {
        *parameters = pThis;
    }
    else
    {
        AB_ServerMessageParametersDestroy(pThis);
    }

    return err;
}


/*! @} */
