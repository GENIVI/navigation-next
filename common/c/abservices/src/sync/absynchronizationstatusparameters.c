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

    @file absynchronizationstatusparameters.c
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
#include "absynchronizationstatusparameters.h"
#include "absynchronizationstatusparametersprivate.h"

#include "data_sync_places_status_query.h"


struct AB_SynchronizationStatusParameters
{
    NB_Context*                     context;
    data_sync_places_status_query   query;
};


AB_DEF NB_Error
AB_SynchronizationStatusParametersCreate(NB_Context* context, AB_SynchronizationStatusParameters** parameters)
{
    AB_SynchronizationStatusParameters* pThis = 0;
    NB_Error err = NE_OK;
    
    data_util_state* dataState = 0;
    
    if (!context || !parameters)
    {
        return NE_INVAL;
    }

    *parameters = 0;

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    dataState = NB_ContextGetDataState(pThis->context);

    pThis->context = context;

    err = data_sync_places_status_query_init(dataState, &pThis->query);

    if (!err)
    {
        *parameters = pThis;
    }
    else
    {
        data_sync_places_status_query_free(dataState, &pThis->query);
        nsl_free(pThis);
    }

    return err;
}

AB_DEF NB_Error
AB_SynchronizationStatusParametersAddDatabase(AB_SynchronizationStatusParameters* pThis, const char* databaseName)
{
    NB_Error err = NE_OK;
    data_sync_places_db_status_query* dbsq = 0;
    
    if (!pThis)
    {
        return NE_INVAL;
    }

   	dbsq = data_sync_places_status_query_get_db_status_query(NB_ContextGetDataState(pThis->context), &pThis->query, databaseName);
    if (!dbsq)
    {
        err = NE_INVAL;
    }

    return err;
}

AB_DEF NB_Error
AB_SynchronizationStatusParametersDestroy(AB_SynchronizationStatusParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_sync_places_status_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    nsl_free(pThis);

    return NE_OK;
}


tpselt
AB_SynchronizationStatusParametersToTPSQuery(AB_SynchronizationStatusParameters* pThis)
{
    if (pThis)
    {
        return data_sync_places_status_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
    }

    return 0;
}


/*! @} */
