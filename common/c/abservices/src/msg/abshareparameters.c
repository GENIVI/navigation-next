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

    @file     abshareparameters.c
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


#include "abshareparameters.h"
#include "abshareparametersprivate.h"
#include "nbcontextprotected.h"
#include "data_send_message_query.h"
#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE
#include "data_send_place_message_query.h"
#endif

struct AB_ShareParameters
{
    NB_Context*                     context;
    data_send_message_query         query;
#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE
    data_send_place_message_query   queryOld;
#endif
};

static NB_Error CreateThis(NB_Context* context, data_msg_content_type type, const char* fromMobileDirectoryNumber, const char* fromName, const char* toEmailOrPhone, const char* message, const NB_Place* place, const char* bannerId, AB_ShareParameters** parameters);


AB_DEF NB_Error
AB_ShareParametersCreateMessage(NB_Context* context, const char* fromMobileDirectoryNumber, const char* fromName, const char* toEmailOrPhone, const char* message, const NB_Place* place, AB_ShareParameters** parameters)
{
#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE
    AB_ShareParameters* pThis = 0;
    data_util_state* dataState;
    NB_Error err = NE_OK;

    if (!place || !parameters)
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

    dataState = NB_ContextGetDataState(pThis->context);

    pThis->query.content.type = MSG_CONTENT_UNDEFINED;
            
    err = err ? err : data_send_place_message_query_init(dataState, &pThis->queryOld);
    err = err ? err : data_string_set(dataState, &pThis->queryOld.from, fromMobileDirectoryNumber);
    err = err ? err : data_string_set(dataState, &pThis->queryOld.from_name, fromName);
    err = err ? err : AB_ShareParametersAddAdditionalRecipient(pThis, toEmailOrPhone);
    err = err ? err : data_string_set(dataState, &pThis->queryOld.message, message);
    err = err ? err : data_place_from_nimplace(dataState, &pThis->queryOld.place, place);

    if (!err)
    {
        *parameters = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
#else
    return NE_NOSUPPORT;
#endif
}


AB_DEF NB_Error AB_ShareParametersCreatePlaceMessage(NB_Context* context, const char* fromMobileDirectoryNumber, const char* fromName, const char* toEmailOrPhone, const char* message, const NB_Place* place, AB_ShareParameters** shareParameters)
{
    return CreateThis(context, MSG_CONTENT_PLACE, fromMobileDirectoryNumber, fromName, toEmailOrPhone, message, place, NULL, shareParameters);
}


AB_DEF NB_Error AB_ShareParametersCreatePlaceMessageWithBannerId(NB_Context* context, const char* fromMobileDirectoryNumber, const char* fromName, const char* toEmailOrPhone, const char* message,  const NB_Place* place, const char* bannerId, AB_ShareParameters** shareParameters)
{
    return CreateThis(context, MSG_CONTENT_PLACE, fromMobileDirectoryNumber, fromName, toEmailOrPhone, message, place, bannerId, shareParameters);
}


AB_DEF NB_Error AB_ShareParametersCreateShareMessage(NB_Context* context, const char* fromMobileDirectoryNumber, const char* fromName, const char* toEmailOrPhone, const char* message, const NB_Place* place, AB_ShareParameters** shareParameters)
{
    return CreateThis(context, MSG_CONTENT_SHARE, fromMobileDirectoryNumber, fromName, toEmailOrPhone, message, place, NULL, shareParameters);
}


AB_DEF NB_Error AB_ShareParametersCreateTextMessage(NB_Context* context, const char* fromMobileDirectoryNumber, const char* fromName, const char* toEmailOrPhone, const char* message, AB_ShareParameters** shareParameters)
{
    return CreateThis(context, MSG_CONTENT_TEXT, fromMobileDirectoryNumber, fromName, toEmailOrPhone, message, NULL, NULL, shareParameters);
}


AB_DEF NB_Error
AB_ShareParametersAddAdditionalRecipient(AB_ShareParameters* pThis, const char* toEmailOrPhone)
{
    NB_Error err = NE_OK;

    if (!pThis || !toEmailOrPhone || !nsl_strlen(toEmailOrPhone))
    {
        return NE_INVAL;
    }

#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE
    if (pThis->query.content.type == MSG_CONTENT_UNDEFINED)
    {
        err = err ? err : data_send_place_message_query_add_recipient(NB_ContextGetDataState(pThis->context), &pThis->queryOld, toEmailOrPhone);
    }
    else
    {
        err = err ? err : data_send_message_query_add_recipient(NB_ContextGetDataState(pThis->context), &pThis->query, toEmailOrPhone);
    }
#else
    err = err ? err : data_send_message_query_add_recipient(NB_ContextGetDataState(pThis->context), &pThis->query, toEmailOrPhone);
#endif
    return err;
}

AB_DEF NB_Error
AB_ShareParametersAddSetStringValue(AB_ShareParameters* pThis, const char* key, const char* value)
{
    NB_Error err = NE_OK;
    
    if (!pThis || !key || !value)
    {
        return NE_INVAL;
    }
    
	err = data_string_set(NB_ContextGetDataState(pThis->context), &pThis->query.setValue.key, key);
	err = err ? err : data_string_set(NB_ContextGetDataState(pThis->context), &pThis->query.setValue.value, value);
    
    return err;
}

AB_DEF NB_Error
AB_ShareParametersDestroy(AB_ShareParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE
    if (pThis->query.content.type == MSG_CONTENT_UNDEFINED)
    {
        data_send_place_message_query_free(NB_ContextGetDataState(pThis->context), &pThis->queryOld);
    }
    else
    {
        data_send_message_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    }
#else
    data_send_message_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
#endif

    nsl_free(pThis);

    return NE_OK;
}

tpselt
AB_ShareParametersToTPSQuery(AB_ShareParameters* pThis)
{
    if (pThis)
    {
#ifdef INCLUDE_DEPRECATED_SENDPLACEMESSAGE    
        if (pThis->query.content.type == MSG_CONTENT_UNDEFINED)
        {
            return data_send_place_message_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->queryOld);
        }
        else
        {
            return data_send_message_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
        }
#else
        return data_send_message_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
#endif
    }

    return 0;
}


data_msg_content_type
AB_ShareParametersGetType(AB_ShareParameters* pThis)
{
    return pThis->query.content.type;
}


NB_Error
CreateThis(NB_Context* context, data_msg_content_type type, const char* fromMobileDirectoryNumber, const char* fromName, const char* toEmailOrPhone, const char* message, const NB_Place* place, const char* bannerId, AB_ShareParameters** parameters)
{
    AB_ShareParameters* pThis = 0;
    data_util_state* dataState;
    NB_Error err = NE_OK;

    if (!parameters)
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

    dataState = NB_ContextGetDataState(pThis->context);

    pThis->query.content.type = type;

    err = err ? err : data_send_message_query_init(dataState, &pThis->query);
    err = err ? err : data_string_set(dataState, &pThis->query.from, fromMobileDirectoryNumber);
    err = err ? err : data_string_set(dataState, &pThis->query.from_name, fromName);

    err = err ? err : AB_ShareParametersAddAdditionalRecipient(pThis, toEmailOrPhone);

    switch (pThis->query.content.type)
    {
        case MSG_CONTENT_PLACE:
            data_string_set(dataState, &pThis->query.content.place_msg_content.message_text.text, message);
            data_place_from_nimplace(dataState, &pThis->query.content.place_msg_content.place, place);
            if (bannerId && *bannerId)
            {
                data_string_set(dataState, &pThis->query.content.place_msg_content.place_msg_banner.id, bannerId);
            }
            break;
        case MSG_CONTENT_SHARE:
            data_string_set(dataState, &pThis->query.content.share_content.message_text.text, message);
            data_place_from_nimplace(dataState, &pThis->query.content.share_content.place, place);
            break;
        case MSG_CONTENT_TEXT:
            data_string_set(dataState, &pThis->query.content.text_msg_content.message_text.text, message);
            break;
        default:
            err = NE_INVAL;
            break;
    }
    
    if (!err)
    {
        *parameters = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}

/*! @} */
