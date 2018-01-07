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

    @file abprofileinformation.c
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
#include "abprofileinformation.h"
#include "abprofileinformationprivate.h"
#include "data_profile_reply.h"


struct AB_ProfileInformation
{
    NB_Context*                 context;
	data_profile_reply          reply;
};


NB_Error
AB_ProfileInformationCreate(NB_Context* context, tpselt reply, AB_ProfileInformation** information)
{
    AB_ProfileInformation* pThis = 0;
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
    err = err ? err : data_profile_reply_from_tps(NB_ContextGetDataState(pThis->context), &pThis->reply, reply);

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
AB_ProfileInformationGetStringValue(AB_ProfileInformation* pThis, const char* key,  const char** value)
{
    if (!pThis || !key || !value)
    {
        return NE_INVAL;
    }

	if (pThis->reply.getValue.key && nsl_strcmp(pThis->reply.getValue.key, key) == 0)
	{
		*value = data_string_get(NB_ContextGetDataState(pThis->context), &pThis->reply.getValue.value);
		return NE_OK;
	}

	return NE_NOENT;
}


AB_DEF uint32
AB_ProfileInformationGetError(AB_ProfileInformation* pThis, const char** errorDescription)
{
    if (!pThis)
    {
        return 0;
    }
    
    if (errorDescription)
    {
        *errorDescription = pThis->reply.errorMsg.description;
    }
    
    //return the error code as uint32 value
    return pThis->reply.errorMsg.codeValue;
}


AB_DEF NB_Error
AB_ProfileInformationDestroy(AB_ProfileInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }
    
    data_profile_reply_free(NB_ContextGetDataState(pThis->context), &pThis->reply);

    nsl_free(pThis);        

    return NE_OK;
}



/*! @} */
