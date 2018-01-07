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

    @file     abspeechstatisticsparameters.c
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
#include "abspeechstatisticsparameters.h"
#include "abspeechstatisticsparametersprivate.h"
#include "data_asr_stats_query.h"


struct AB_SpeechStatisticsParameters
{
    NB_Context*             context;
    data_asr_stats_query    query;
};


AB_DEF NB_Error
AB_SpeechStatisticsParametersCreate(NB_Context* context, const char* previousInteractionId, AB_SpeechStatisticsParameters** parameters)
{
    AB_SpeechStatisticsParameters* pThis = 0;
    NB_Error err = NE_OK;
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

    err = data_asr_stats_query_init(dataState, &pThis->query);

    if (err)
    {
        data_asr_stats_query_free(dataState, &pThis->query);
        nsl_free(pThis);
        return err;
    }

    *parameters = pThis;

    if (previousInteractionId)
    {
        data_asr_stats_set_id(NB_ContextGetDataState(pThis->context), &pThis->query, previousInteractionId);
    }

    return err;
}

AB_DEF NB_Error
AB_SpeechStatisticsParametersDestroy(AB_SpeechStatisticsParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_asr_stats_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    nsl_free(pThis);

    return NE_OK;
}


AB_DEF NB_Error
AB_SpeechStatisticsParametersAddText(AB_SpeechStatisticsParameters* pThis, const char* fieldName, const char* fieldText)
{
    return AB_SpeechStatisticsParametersAddTextFromSource(pThis, fieldName, fieldText, "text");
}


AB_DEF NB_Error
AB_SpeechStatisticsParametersAddTextFromSource(AB_SpeechStatisticsParameters* pThis, const char* fieldName, const char* fieldText, const char* fieldSource)
{
    if (!pThis || !fieldName || !fieldText)
    {
        return NE_INVAL;
    }

    return data_asr_stats_query_add_field_data(
        NB_ContextGetDataState(pThis->context),
        &pThis->query,
        fieldName,
        fieldSource,
        fieldText,
        0,
        NULL,
        NULL,
        NULL,
        0);
}


AB_DEF NB_Error
AB_SpeechStatisticsParametersAddUtterance(AB_SpeechStatisticsParameters* pThis, const char* fieldName, const char* fieldText, const char* utteranceEncoding, byte* utteranceData, uint32 utteranceSize)
{
    if (!pThis || !fieldName || !fieldText || !utteranceEncoding || !utteranceData)
    {
        return NE_INVAL;
    }

    return data_asr_stats_query_add_field_data(
        NB_ContextGetDataState(pThis->context),
        &pThis->query,
        fieldName,
        "speech",
        fieldText,
        0,
        NULL,
        utteranceEncoding,
        utteranceData,
        utteranceSize);
}


AB_DEF NB_Error
AB_SpeechStatisticsParametersAddUtteranceFromDataStore(AB_SpeechStatisticsParameters* pThis, const char* fieldName, const char* fieldText, const char* utteranceEncoding, const char* dataStoreId)
{
    if (!pThis || !fieldName || !fieldText || !utteranceEncoding || !dataStoreId)
    {
        return NE_INVAL;
    }

    return data_asr_stats_query_add_field_data(
        NB_ContextGetDataState(pThis->context),
        &pThis->query,
        fieldName,
        "speech-id",
        fieldText,
        0,
        dataStoreId,
        utteranceEncoding,
        NULL,
        0);
}


AB_DEF NB_Error
AB_SpeechStatisticsParametersClearFields(AB_SpeechStatisticsParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }
    
    return data_asr_stats_query_clear_fields(NB_ContextGetDataState(pThis->context), &pThis->query);
}


AB_DEF NB_Error
AB_SpeechStatisticsParametersSetInteractionID(AB_SpeechStatisticsParameters* pThis, const char* interactionId)
{
    if (!pThis || !interactionId)
    {
        return NE_INVAL;
    }
    
    return data_asr_stats_set_id(NB_ContextGetDataState(pThis->context), &pThis->query, interactionId);
}


AB_DEF NB_Error
AB_SpeechStatisticsParametersSetTiming(AB_SpeechStatisticsParameters* pThis, const char* timing)
{
    if (!pThis || !timing)
    {
        return NE_INVAL;
    }

    return data_asr_stats_query_set_timing(NB_ContextGetDataState(pThis->context), &pThis->query, timing);
}


tpselt
AB_SpeechStatisticsParametersToTPSQuery(AB_SpeechStatisticsParameters* pThis)
{
    if (pThis)
    {
        return data_asr_stats_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
    }

    return 0;
}


/*! @} */
