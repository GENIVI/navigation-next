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

    @file abprofileparameters.c
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
#include "abprofileparameters.h"
#include "abprofileparametersprivate.h"
#include "data_profile_query.h"


struct AB_ProfileParameters
{
    NB_Context*             context;
    data_profile_query      query;
};


AB_DEF NB_Error
AB_ProfileParametersCreate(NB_Context* context, uint32 version, AB_ProfileParameters** parameters)
{
    AB_ProfileParameters* pThis = 0;
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

    err = err ? err : data_profile_query_init(dataState, &pThis->query);

	pThis->query.version = version ? version : 1;

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


AB_DEF NB_Error
AB_ProfileParametersDestroy(AB_ProfileParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_profile_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    nsl_free(pThis);

    return NE_OK;
}


AB_DEF NB_Error
AB_ProfileParametersAddGetValue(AB_ProfileParameters* pThis, const char* key)
{
    if (!pThis || !key)
    {
        return NE_INVAL;
    }
    
	return data_string_set(NB_ContextGetDataState(pThis->context), &pThis->query.getValue.key, key);
}


AB_DEF NB_Error
AB_ProfileParametersAddSetToDefaults(AB_ProfileParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

	pThis->query.isDefault = TRUE;

    return NE_OK;
}


AB_DEF NB_Error
AB_ProfileParametersAddSetStringValue(AB_ProfileParameters* pThis, const char* key, const char* value)
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


tpselt
AB_ProfileParametersToTPSQuery(AB_ProfileParameters* pThis)
{
    if (pThis)
    {
        return data_profile_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
    }

    return 0;
}

data_profile_query*
AB_ProfileParametersGetProfileQuery(AB_ProfileParameters* pThis)
{
    if (pThis)
    {
        return &pThis->query;
    }

    return 0;
}

/*! @} */
