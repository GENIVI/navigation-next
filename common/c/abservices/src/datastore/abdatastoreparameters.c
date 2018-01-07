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

    @file abdatastoreparameters.c
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
#include "abdatastoreparameters.h"
#include "abdatastoreparametersprivate.h"
#include "data_datastore_query.h"

const uint32 DEFAULT_EXPIRE_TIME_MINUTES = 20;

struct AB_DataStoreParameters
{
    NB_Context*                 context;
    data_datastore_query        query;
};


AB_DEF NB_Error
AB_DataStoreParametersCreate(NB_Context* context, /* uint32 expireTimeMinutes, */ AB_DataStoreParameters** datastoreParameters)
{
    AB_DataStoreParameters* pThis = 0;
    NB_Error err = NE_OK;
    data_util_state* dataState = 0;

    if (!context || !datastoreParameters)
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

    err = err ? err : data_datastore_query_init(dataState, &pThis->query);

    // pThis->query.expire_time_min = expireTimeMinutes;
    pThis->query.expire_time_min = DEFAULT_EXPIRE_TIME_MINUTES;

    if (!err)
    {
        *datastoreParameters = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}


AB_DEF NB_Error
AB_DataStoreParametersAddStore(AB_DataStoreParameters* pThis, byte* buffer, nb_size size)
{
    if (!pThis || !buffer)
    {
        return NE_INVAL;
    }
    
    return data_datastore_query_add_action(NB_ContextGetDataState(pThis->context), &pThis->query, DATASTORE_STORE, buffer, size);
}


AB_DEF NB_Error
AB_DataStoreParametersAddRetrieve(AB_DataStoreParameters* pThis, char* retrieveId)
{
    if (!pThis || !retrieveId)
    {
        return NE_INVAL;
    }
    
    return data_datastore_query_add_action(NB_ContextGetDataState(pThis->context), &pThis->query, DATASTORE_RETRIEVE, (byte*)retrieveId, nsl_strlen(retrieveId) + 1);
}


AB_DEF NB_Error
AB_DataStoreParametersAddDelete(AB_DataStoreParameters* pThis, char* deleteId)
{
    if (!pThis || !deleteId)
    {
        return NE_INVAL;
    }
    
    return data_datastore_query_add_action(NB_ContextGetDataState(pThis->context), &pThis->query, DATASTORE_DELETE, (byte*)deleteId, nsl_strlen(deleteId) + 1);
}


AB_DEF NB_Error
AB_DataStoreParametersDestroy(AB_DataStoreParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_datastore_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    nsl_free(pThis);

    return NE_OK;
}


tpselt
AB_DataStoreParametersToTPSQuery(AB_DataStoreParameters* pThis)
{
    if (pThis)
    {
        return data_datastore_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
    }

    return 0;
}


/*! @} */
