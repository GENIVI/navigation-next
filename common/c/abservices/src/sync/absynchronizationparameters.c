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

    @file absynchronizationparameters.c
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
#include "absynchronizationparameters.h"
#include "absynchronizationparametersprivate.h"
#include "data_sync_places_query.h"
#include "data_sync_places_db_query.h"


struct AB_SynchronizationParameters
{
    NB_Context*             context;
    data_sync_places_query  query;
};


AB_DEF NB_Error
AB_SynchronizationParametersCreate(NB_Context* context, AB_SynchronizationParameters** parameters)
{
    AB_SynchronizationParameters* pThis = 0;
    NB_Error err = NE_OK;

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

    err = err ? err : data_sync_places_query_init(NB_ContextGetDataState(context), &pThis->query);

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
AB_SynchronizationParametersDestroy(AB_SynchronizationParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_sync_places_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    nsl_free(pThis);

    return NE_OK;
}


AB_DEF NB_Error
AB_SynchronizationParamatersSetDatabaseData(AB_SynchronizationParameters* pThis, const char* databaseName, uint32 generationId, uint32 databaseId)
{
    data_sync_places_db_query* dbq = 0;

    if (!pThis || !databaseName)
    {
        return NE_INVAL;
    }
    
    dbq = data_sync_places_query_get_db_query(NB_ContextGetDataState(pThis->context), &pThis->query, databaseName);

    if (!dbq)
    {
        return NE_INVAL;
    }

    dbq->last_sync_generation = generationId;	
    dbq->db_id = databaseId;	

    return NE_OK;
}


AB_DEF NB_Error
AB_SynchronizationParametersAddLocalOperation(AB_SynchronizationParameters* pThis, AB_SynchronizationOperation* operation)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = 0;
    data_sync_places_db_query* dbq = 0;

    if (!pThis || !operation)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(pThis->context);

    dbq = data_sync_places_query_get_db_query(dataState, &pThis->query, operation->databaseName);

    if (!dbq)
    {
        return NE_INVAL;
    }

    switch (operation->operationType)
    {
        case ASOT_Add:
            err = data_sync_places_db_query_add_add_item_with_ext_app_content(dataState, dbq, operation->localId, operation->modifiedTime, operation->placeValid ? &operation->place : NULL, operation->messageValid ? &operation->message : NULL, &operation->extAppContent);
            break;
        case ASOT_Modify:
            err = data_sync_places_db_query_add_modify_item_with_ext_app_content(dataState, dbq, operation->serverId, operation->modifiedTime, operation->placeValid ? &operation->place : NULL, operation->messageValid ? &operation->message : NULL, operation->placeMessageFlags, &operation->extAppContent);
            break;
        case ASOT_ModifyTimeStamp:
            err = data_sync_places_db_query_add_modify_item_with_ext_app_content(dataState, dbq, operation->serverId, operation->modifiedTime, operation->placeValid ? &operation->place : NULL, operation->messageValid ? &operation->message : NULL, operation->placeMessageFlags, &operation->extAppContent);
            break;
        case ASOT_Delete:
            err = data_sync_places_db_query_add_delete_item(dataState, dbq, operation->serverId);
            break;
        case ASOT_AssignId:
            err = NE_INVAL;
            break;
        case ASOT_None:
            // no further action required.  This simply ensures that the sync_places_db_query is added.
            break;
        default:
            err = NE_INVAL;
    }

    return err;
}

AB_DEF NB_Error
AB_SynchronizationParametersGetLocalOperationCount(AB_SynchronizationParameters* pThis,
        const char* databaseName, AB_SynchronizationOperationType opType, uint32* localOperationCount)
{
    data_util_state* dataState = NULL;
    data_sync_places_db_query* dbq = NULL;

    if (!pThis || !localOperationCount)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(pThis->context);

    dbq = data_sync_places_query_get_db_query(dataState, &pThis->query, databaseName);
    if (!dbq)
    {
        return NE_INVAL;
    }

    switch (opType)
    {
        case ASOT_Add:
            *localOperationCount = CSL_VectorGetLength(dbq->vec_add_item);
            break;
        case ASOT_Modify:
            *localOperationCount = CSL_VectorGetLength(dbq->vec_modify_item);
            break;
        case ASOT_ModifyTimeStamp:
            *localOperationCount = 0;
            break;
        case ASOT_Delete:
            *localOperationCount = CSL_VectorGetLength(dbq->vec_delete_item);
            break;
        case ASOT_AssignId:
            *localOperationCount = 0;
            break;
        case ASOT_None:
            *localOperationCount = 0;
            break;
        case ASOT_All:
            *localOperationCount = CSL_VectorGetLength(dbq->vec_add_item) +
                                   CSL_VectorGetLength(dbq->vec_delete_item) +
                                   CSL_VectorGetLength(dbq->vec_modify_item);
            break;
        default:
            *localOperationCount = 0;
            break;
    }

    return NE_OK;
}

tpselt
AB_SynchronizationParametersToTPSQuery(AB_SynchronizationParameters* pThis)
{
    if (pThis)
    {
        return data_sync_places_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
    }

    return 0;
}


/*! @} */
