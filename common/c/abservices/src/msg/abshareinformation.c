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

    @file     abshareinfo.c
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


#include "abshareinformation.h"
#include "nbcontextprotected.h"
#include "cslnetwork.h"
#include "data_send_message_reply.h"
#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE
#include "data_send_place_message_reply.h"                      
#endif

struct AB_ShareInformation
{
    NB_Context*                     context;
    data_send_message_reply         reply;
#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE
    boolean                         deprecated;
	data_send_place_message_reply   replyDeprecated; 
#endif
};

NB_Error
AB_ShareInformationCreate(NB_Context* context, tpselt reply, AB_ShareInformation** information)
{
    AB_ShareInformation* pThis = 0;
    NB_Error err = NE_OK;

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
    

#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE
    if (!nsl_strcmp(te_getname(reply), "send-place-message-reply"))
    {
        err = data_send_place_message_reply_from_tps(NB_ContextGetDataState(context), &pThis->replyDeprecated, reply);
        pThis->deprecated = TRUE;
    }
    else
    {
        err = data_send_message_reply_from_tps(NB_ContextGetDataState(context), &pThis->reply, reply);
        pThis->deprecated = FALSE;
    }
#else
    err = data_send_message_reply_from_tps(NB_ContextGetDataState(context), &pThis->reply, reply);
#endif

    if (!err)
    {
        *information = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}

AB_DEF NB_Error
AB_ShareInformationGetResultCount(AB_ShareInformation* pThis, uint32* resultCount)
{
    if (!pThis || !resultCount)
    {
        return NE_INVAL;
    }

#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE
    if (pThis->deprecated)
    {
        *resultCount = data_send_place_message_reply_num_results(NB_ContextGetDataState(pThis->context), &pThis->replyDeprecated);
    }
    else
    {
        *resultCount = data_send_message_reply_num_results(NB_ContextGetDataState(pThis->context), &pThis->reply);
    }
#else
    *resultCount = data_send_message_reply_num_results(NB_ContextGetDataState(pThis->context), &pThis->reply);
#endif    
    
    return NE_OK;
}

AB_DEF NB_Error
AB_ShareInformationGetResult(AB_ShareInformation* pThis, uint32 index, AB_SharePlaceMessageStatus* status, char* to, nb_size toSize, char* id, nb_size idSize)
{
#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE
    data_util_state* dataState = 0;
    data_place_msg_recipient_result* result = 0;

    if (!pThis || !status)
    {
        return NE_INVAL;
    }
    
    if (to)
    {
        *to = 0;
    }
    if (id)
    {
        *id = 0;
    }

    dataState = NB_ContextGetDataState(pThis->context);
    result = data_send_place_message_reply_get_result(dataState, &pThis->replyDeprecated, index);

    if (!result)
    {
       *status = ASPMS_UnknownError;
       return NE_NOENT;
    }

    if (str_cmpxi(data_string_get(dataState, &result->place_msg_error.code), "") == 0)
    {
        *status = ASPMS_Success;
    }
    else if (str_cmpxi(data_string_get(dataState, &result->place_msg_error.code), "F") == 0)
    {
        *status = ASPMS_InboxFull;
    }
    else if (str_cmpxi(data_string_get(dataState, &result->place_msg_error.code), "I") == 0)
    {
        *status = ASPMS_InvalidNumber;
    }
    else
    {
        *status = ASPMS_UnknownError;
    }

    if (to && toSize)
    {
        nsl_strlcpy(to, data_string_get(dataState, &result->to), toSize);
    }

    if (id && idSize)
    {
        nsl_strlcpy(id, data_string_get(dataState, &result->place_msg_id.id), idSize);
    }
    return NE_OK;
#else
    return NE_NOSUPPORT;
#endif
}


AB_DEF NB_Error
AB_ShareInformationGetResultEx(AB_ShareInformation* pThis, uint32 index, uint32* errorCode, char* errorMessage, nb_size errorMessageSize, char* to, nb_size toSize, char* id, nb_size idSize)
{
    data_util_state* dataState = 0;
    data_send_msg_result* result = 0;

    if (!pThis)
    {
        return NE_INVAL;
    }

    if (errorCode)
    {
        *errorCode = 0;
    }
    if (errorMessage && errorMessageSize)
    {
        *errorMessage = 0;
    }
    if (to && toSize)
    {
        *to = 0;
    }
    if (id && idSize)
    {
        *id = 0;
    }

    dataState = NB_ContextGetDataState(pThis->context);
    result = data_send_message_reply_get_result(dataState, &pThis->reply, index);

    if (!result)
    {
        return NE_NOENT;
    }

    if (errorCode)
    {
        *errorCode = result->error_msg.codeValue;
    }
    
    if (errorMessage && errorMessageSize)
    {
        nsl_strlcpy(errorMessage, data_string_get(dataState, &result->error_msg.description), errorMessageSize);
    }
    
    if (to && toSize)
    {
        nsl_strlcpy(to, data_string_get(dataState, &result->to), toSize);
    }

    if (id && idSize)
    {
        nsl_strlcpy(id, data_string_get(dataState, &result->message_id.id), idSize);
    }
    return NE_OK;
}


AB_DEF NB_Error
AB_ShareInformationDestroy(AB_ShareInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }
 
#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE   
    if (pThis->deprecated)
    {
        data_send_place_message_reply_free(NB_ContextGetDataState(pThis->context), &pThis->replyDeprecated);
    }
    else
    {
        data_send_message_reply_free(NB_ContextGetDataState(pThis->context), &pThis->reply);
    }
#else
    data_send_message_reply_free(NB_ContextGetDataState(pThis->context), &pThis->reply);
#endif

    nsl_free(pThis);        

    return NE_OK;
}


/*! @} */

