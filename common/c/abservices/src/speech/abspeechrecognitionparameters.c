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

    @file     abspeechrecognitionparameters.c
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
#include "abspeechrecognitionparameters.h"
#include "abspeechrecognitionparametersprivate.h"
#include "data_asr_reco_query.h"
#include "abqalog.h"


struct AB_SpeechRecognitionParameters
{
    NB_Context*         context;
    data_asr_reco_query query;
};


AB_DEF NB_Error
AB_SpeechRecognitionParametersCreate(NB_Context* context, NB_GpsLocation* gpsFix, const char* previousInteractionId, AB_SpeechRecognitionParameters** parameters)
{
    AB_SpeechRecognitionParameters* pThis = 0;
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

    err = data_asr_reco_query_init(dataState, &pThis->query);

    if (err)
    {
        data_asr_reco_query_free(dataState, &pThis->query);
        nsl_free(pThis);
        return err;
    }

    *parameters = pThis;

    if (gpsFix)
    {
        data_asr_reco_query_set_gps_fix(NB_ContextGetDataState(pThis->context), &pThis->query, *gpsFix);
    }

    if (previousInteractionId)
    {
        data_asr_reco_set_id(NB_ContextGetDataState(pThis->context), &pThis->query, previousInteractionId);
    }

    pThis->query.want_accuracy = 0;
    pThis->query.want_extended_codes = 0;

    return err;
}


AB_DEF NB_Error
AB_SpeechRecognitionParametersDestroy(AB_SpeechRecognitionParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_asr_reco_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    nsl_free(pThis);

    return NE_OK;
}


AB_DEF NB_Error
AB_SpeechRecognitionParametersAddText(AB_SpeechRecognitionParameters* pThis, const char* fieldName, const char* fieldText)
{
    return AB_SpeechRecognitionParametersAddTextFromSource(pThis, fieldName, fieldText, "text");
}


AB_DEF NB_Error
AB_SpeechRecognitionParametersAddTextFromSource(AB_SpeechRecognitionParameters* pThis, const char* fieldName, const char* fieldText, const char* fieldSource)
{
    if (!pThis || !fieldName || !fieldText)
    {
        return NE_INVAL;
    }

    return data_asr_reco_query_add_field_data(
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
AB_SpeechRecognitionParametersAddUtterance(AB_SpeechRecognitionParameters* pThis, const char* fieldName, const char* fieldText, const char* utteranceEncoding, byte* utteranceData, uint32 utteranceSize)
{
    if (!pThis || !fieldName || !fieldText || !utteranceEncoding || !utteranceData)
    {
        return NE_INVAL;
    }

    return data_asr_reco_query_add_field_data(
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
AB_SpeechRecognitionParametersAddUtteranceFromDataStore(AB_SpeechRecognitionParameters* pThis, const char* fieldName, const char* fieldText, const char* utteranceEncoding, const char* dataStoreId)
{
    if (!pThis || !fieldName || !fieldText || !utteranceEncoding || !dataStoreId)
    {
        return NE_INVAL;
    }

    return data_asr_reco_query_add_field_data(
        NB_ContextGetDataState(pThis->context),
        &pThis->query,
        fieldName,
        "speech",
        fieldText,
        0,
        dataStoreId,
        utteranceEncoding,
        NULL,
        0);
}


AB_DEF NB_Error
AB_SpeechRecognitionParametersClearFields(AB_SpeechRecognitionParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }
    
    return data_asr_reco_query_clear_fields(NB_ContextGetDataState(pThis->context), &pThis->query);
}


AB_DEF NB_Error
AB_SpeechRecognitionParametersSetInteractionID(AB_SpeechRecognitionParameters* pThis, const char* interactionId)
{
    if (!pThis || !interactionId)
    {
        return NE_INVAL;
    }
    
    return data_asr_reco_set_id(NB_ContextGetDataState(pThis->context), &pThis->query, interactionId);
}


AB_DEF NB_Error
AB_SpeechRecognitionParametersSetConfigurationOptions(AB_SpeechRecognitionParameters* pThis, uint32 options)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    pThis->query.want_accuracy = (boolean)(options & AB_ASR_WantAccuracy);
    pThis->query.want_extended_codes = (boolean)(options & AB_ASR_WantExtendedCodes);
    
    return NE_OK;
}


tpselt
AB_SpeechRecognitionParametersToTPSQuery(AB_SpeechRecognitionParameters* pThis)
{
    if (pThis)
    {
        int l = 0;
        int i = 0;

        if (pThis->query.vec_field_data != NULL)
        {
            data_util_state* pDataState = NB_ContextGetDataState(pThis->context);
            l = CSL_VectorGetLength(pThis->query.vec_field_data);
            for (i = 0; i < l; i++)
            {
                data_asr_field_data* pfd = CSL_VectorGetPointer(pThis->query.vec_field_data, i);
                AB_QaLogSpeechRecognitionRequest(
                    pThis->context,
                    data_string_get(pDataState, &pfd->field_name),
                    data_string_get(pDataState, &pfd->field_text),
                    data_string_get(pDataState, &pfd->utterance_encoding),
                    data_string_get(pDataState, &pfd->utterance_id),
                    pfd->utterance_data.size,
                    pThis->query.asr_id
                );
            }
        }

        return data_asr_reco_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
    }

    return 0;
}


/*! @} */
