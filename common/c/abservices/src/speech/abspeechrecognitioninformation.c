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

    @file abspeechrecognitioninformation.c
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
#include "abspeechrecognitioninformation.h"
#include "abspeechrecognitioninformationprivate.h"
#include "data_asr_reco_reply.h"
#include "nbutilityprotected.h"


struct AB_SpeechRecognitionInformation
{
    NB_Context*             context;
	data_asr_reco_reply     reply;
};


NB_Error
AB_SpeechRecognitionInformationCreate(NB_Context* context, tpselt reply, AB_SpeechRecognitionInformation** information)
{
    AB_SpeechRecognitionInformation* pThis = 0;
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
    err = err ? err : data_asr_reco_reply_from_tps(NB_ContextGetDataState(pThis->context), &pThis->reply, reply);

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
AB_SpeechRecognitionInformationGetRecognizeResults(AB_SpeechRecognitionInformation* pThis, AB_SpeechRecognitionResults* resultInfo)
{
    if (!pThis || !resultInfo)
    {
        return NE_INVAL;
    }
    
    nsl_memset(resultInfo, 0, sizeof(AB_SpeechRecognitionResults));
    
    resultInfo->completionCode = pThis->reply.completion_code;

    if (pThis->reply.asr_id)
    {
        resultInfo->interactionID = data_string_get(NB_ContextGetDataState(pThis->context), &pThis->reply.asr_id);
    }
    
    if (pThis->reply.ambiguous_interaction)
    {
        resultInfo->lastFieldName = data_string_get(NB_ContextGetDataState(pThis->context), &pThis->reply.ambiguous_interaction);
    }

    resultInfo->locationMatchCount = CSL_VectorGetLength(pThis->reply.vec_locmatch);
    resultInfo->placeMatchCount = CSL_VectorGetLength(pThis->reply.vec_proxmatch);
    resultInfo->dataIdCount = CSL_VectorGetLength(pThis->reply.vec_data_elem_id);
    
    return NE_OK;
}


AB_DEF NB_Error
AB_SpeechRecognitionInformationGetRecognizedPlace(AB_SpeechRecognitionInformation* pThis, uint32 poiIndex, NB_Place* place, double* distance)
{
    NB_Error err = NE_NOENT;
    
    if (!pThis || !place)
    {
        return NE_INVAL;
    }
    
    nsl_memset(place, 0, sizeof(NB_Place));
    if (distance)
    {
        *distance = 0.0;
    }
    
    if (poiIndex < (uint32)CSL_VectorGetLength(pThis->reply.vec_proxmatch))
    {
        data_proxmatch* pProxMatch = (data_proxmatch*)CSL_VectorGetPointer(pThis->reply.vec_proxmatch, poiIndex);
        if (pProxMatch)
        {
            err = SetNIMPlaceFromPlace(place, NB_ContextGetDataState(pThis->context), &pProxMatch->place);
            if (distance)
            {
                *distance = pProxMatch->distance;
            }
        }
    }

    return err;
}


AB_DEF NB_Error
AB_SpeechRecognitionInformationGetRecognizedLocation(AB_SpeechRecognitionInformation* pThis, uint32 locationIndex, NB_Location* location)
{
    NB_Error err = NE_NOENT;
    
    if (!pThis || !location)
    {
        return NE_INVAL;
    }
    
    nsl_memset(location, 0, sizeof(NB_Location));

	if (locationIndex < (uint32)CSL_VectorGetLength(pThis->reply.vec_locmatch))
	{
	    data_locmatch* pLocMatch = (data_locmatch*)CSL_VectorGetPointer(pThis->reply.vec_locmatch, locationIndex);
	    if (pLocMatch)
	    {
    		err = SetNIMLocationFromLocMatch(location, NB_ContextGetDataState(pThis->context), pLocMatch);
	    }
    }
      
    return err;
}


AB_DEF NB_Error
AB_SpeechRecognitionInformationGetSavedDataId(AB_SpeechRecognitionInformation* pThis, uint32 savedDataIdIndex, const char** savedDataId)
{
    NB_Error err = NE_OK;

    if (!pThis || !savedDataId)
    {
        return NE_INVAL;
    }
    
    *savedDataId = NULL;
    
    if (savedDataIdIndex < (uint32)CSL_VectorGetLength(pThis->reply.vec_data_elem_id))
    {
        data_string* pSavedDataId = (data_string*)CSL_VectorGetPointer(pThis->reply.vec_data_elem_id, savedDataIdIndex);
        if (pSavedDataId)
        {
            *savedDataId = data_string_get(NB_ContextGetDataState(pThis->context), pSavedDataId);
            err = *savedDataId ? NE_OK : NE_INVAL;
        }
    }
    else
    {
        err = NE_NOENT;
    }


    return err;
}


AB_DEF NB_Error
AB_SpeechRecognitionInformationDestroy(AB_SpeechRecognitionInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }
    
    data_asr_reco_reply_free(NB_ContextGetDataState(pThis->context), &pThis->reply);

    nsl_free(pThis);        

    return NE_OK;
}

/*! @} */
