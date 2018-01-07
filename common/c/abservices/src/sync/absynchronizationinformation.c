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

    @file absynchronizationinformation.c
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
#include "absynchronizationinformation.h"
#include "absynchronizationinformationprivate.h"
#include "nbutilityprotected.h"
#include "data_sync_places_query.h"
#include "data_sync_places_reply.h"
#include "abutil.h"
#include "nbutilityprotected.h"


struct AB_SynchronizationInformation
{
    NB_Context*             context;
    data_sync_places_reply  reply;
};


static void ClearSynchronizationOperation(AB_SynchronizationOperation* pPSO);


NB_Error
AB_SynchronizationInformationCreate(NB_Context* context, tpselt reply, AB_SynchronizationInformation** information)
{
    AB_SynchronizationInformation* pThis = 0;
    NB_Error err = NE_OK;
    data_util_state* dataState = 0;

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
    dataState = NB_ContextGetDataState(pThis->context);

    err = err ? err : data_sync_places_reply_from_tps(dataState, &pThis->reply, reply);

    if (!err)
    {
        *information = pThis;
    }
    else
    {
        AB_SynchronizationInformationDestroy(pThis);
    }

    return err;
}


AB_DEF NB_Error
AB_SynchronizationInformationGetServerOperationCount(AB_SynchronizationInformation* pThis, const char* databaseName, AB_SynchronizationOperationType opType, uint32* operationCount)
{
    data_sync_places_db_reply* dbr = 0;
    
    if (!pThis || !databaseName || !operationCount)
    {
        return NE_INVAL;
    }

    *operationCount = 0;
    
    dbr = data_sync_places_reply_get_db_reply(NB_ContextGetDataState(pThis->context), &pThis->reply, databaseName);

    if (!dbr)
    {
        return NE_INVAL;
    }

    switch (opType)
    {
        case ASOT_Add:
            *operationCount = CSL_VectorGetLength(dbr->vec_add_item);
            break;
        case ASOT_AssignId:
            *operationCount = CSL_VectorGetLength(dbr->vec_assign_item_id);
            break;
        case ASOT_Delete:
            *operationCount = CSL_VectorGetLength(dbr->vec_delete_item);
            break;
        case ASOT_Modify:
            *operationCount = CSL_VectorGetLength(dbr->vec_modify_item);
            break;
        case ASOT_ModifyTimeStamp:
            *operationCount = 0;
            break;
        case ASOT_All:
            *operationCount = CSL_VectorGetLength(dbr->vec_add_item) +
                              CSL_VectorGetLength(dbr->vec_assign_item_id) +
                              CSL_VectorGetLength(dbr->vec_delete_item) +
                              CSL_VectorGetLength(dbr->vec_modify_item);
        default:
            break;
    }

    return NE_OK;
}


AB_DEF NB_Error
AB_SynchronizationInformationGetServerOperation(AB_SynchronizationInformation* pThis, const char* databaseName, AB_SynchronizationOperationType opType, uint32 index, AB_SynchronizationOperation* operation)
{
    NB_Error err = NE_OK;
    data_sync_places_db_reply* dbr = 0;
    data_util_state* dataState = 0;

    if (!pThis || !databaseName || !operation)
    {
        return NE_INVAL;
    }
    
    dataState = NB_ContextGetDataState(pThis->context);
    if (!dataState)
    {
        return NE_INVAL;
    }

    dbr = data_sync_places_reply_get_db_reply(dataState, &pThis->reply, databaseName);
    if (!dbr)
    {
        return NE_INVAL;
    }

    switch (opType)
    {
        case ASOT_Add:
        {
            data_sync_add_item*	pai = CSL_VectorGetPointer(dbr->vec_add_item, index);

            ClearSynchronizationOperation(operation);
            
            operation->operationType = opType;
            nsl_strlcpy(operation->databaseName, databaseName, sizeof(operation->databaseName));
            operation->serverId = nsl_atoi(data_string_get(dataState, &pai->id));
            operation->modifiedTime = pai->modtime;

            if (pai->place_valid)
            {
                err = err ? err : SetNIMPlaceFromPlace(&operation->place, dataState, &pai->place);
            }
            operation->placeValid = pai->place_valid;

            if (pai->message_valid)
            {
                err = err ? err : SetPlaceMessageFromMessage(&operation->message, dataState, &pai->message);
            }

            operation->messageValid = pai->message_valid;

            err = err ? err : SetNIMExtAppContentFromExtAppContentVector(&operation->extAppContent, dataState, pai->vec_extapp_content);

            return err;
        }
        case ASOT_AssignId:
        {
            data_sync_assign_item_id* paai = CSL_VectorGetPointer(dbr->vec_assign_item_id, index);

            ClearSynchronizationOperation(operation);

            operation->operationType = opType;
            nsl_strcpy(operation->databaseName, databaseName);
            operation->localId = nsl_atoi(data_string_get(dataState, &paai->local_id));
            operation->serverId = nsl_atoi(data_string_get(dataState, &paai->server_id));
            operation->modifiedTime = 0;

            return NE_OK;
        }
        case ASOT_Delete:
        {
            data_sync_delete_item* pdi = CSL_VectorGetPointer(dbr->vec_delete_item, index);

            ClearSynchronizationOperation(operation);

            operation->operationType = opType;
            nsl_strcpy(operation->databaseName, databaseName);
            operation->localId = 0;
            operation->serverId = pdi->id;
            operation->modifiedTime = 0;

            return NE_OK;
        }
        case ASOT_Modify:
        {
            data_sync_modify_item*	pmi = CSL_VectorGetPointer(dbr->vec_modify_item, index);

            ClearSynchronizationOperation(operation);

            operation->operationType = opType;
            nsl_strcpy(operation->databaseName, databaseName);
            operation->serverId = pmi->id;
            operation->modifiedTime = pmi->modtime;

            if (pmi->place_valid)
            {
                err = err ? err : SetNIMPlaceFromPlace(&operation->place, dataState, &pmi->place);
            }
            operation->placeValid = pmi->place_valid;

            if (pmi->message_valid)
            {
                err = err ? err : SetPlaceMessageFromMessage(&operation->message, dataState, &pmi->message);
            }
            operation->messageValid = pmi->message_valid;

            err = err ? err : SetNIMExtAppContentFromExtAppContentVector(&operation->extAppContent, dataState, pmi->vec_extapp_content);

            return err;
        }
        case ASOT_ModifyTimeStamp:
            return NE_INVAL;

        default:
            break;
    }

    return NE_INVAL;
}


AB_DEF NB_Error
AB_SynchronizationInformationGetDatabaseData(AB_SynchronizationInformation* pThis, const char* databaseName, uint32* generationId, uint32* databaseId)
{
    data_sync_places_db_reply* dbr = 0;

    if (!pThis || !databaseName || !generationId || !databaseId)
    {
        return NE_INVAL;
    }
    
    *generationId = 0;
    *databaseId = 0;

    dbr = data_sync_places_reply_get_db_reply(NB_ContextGetDataState(pThis->context), &pThis->reply, databaseName);

    if (!dbr)
    {
        return NE_INVAL;
    }
    
    *generationId = dbr->new_sync_generation;
    *databaseId = dbr->db_id;
    
    return NE_OK;
}


AB_DEF NB_Error
AB_SynchronizationInformationGetServerCount(AB_SynchronizationInformation* pThis, const char* databaseName, uint32* serverCount)
{
    data_sync_places_db_reply* dbr = 0;

    if (!pThis || !databaseName || !serverCount)
    {
        return NE_INVAL;
    }
    
    dbr = data_sync_places_reply_get_db_reply(NB_ContextGetDataState(pThis->context), &pThis->reply, databaseName);

    if (!dbr)
    {
        return NE_INVAL;
    }

    *serverCount = dbr->count;
    return NE_OK;
}


AB_DEF NB_Error
AB_SynchronizationInformationGetServerError(AB_SynchronizationInformation* pThis, const char* databaseName, nb_boolean* errorValid, AB_SynchronizationError* errorCode, uint32* errorValue)
{
    data_sync_places_db_reply* dbr = 0;
    const char* code = NULL;

    if (!pThis || !databaseName || !errorValid)
    {
        return NE_INVAL;
    }

    *errorValid = FALSE;
    
    dbr = data_sync_places_reply_get_db_reply(NB_ContextGetDataState(pThis->context), &pThis->reply, databaseName);

    if (!dbr)
    {
        return FALSE;
    }

    if (errorValid)
    {
        *errorValid = dbr->sync_error_valid;
    }
    
    if (errorCode)
    {
        code = data_string_get(NB_ContextGetDataState(pThis->context), &dbr->sync_error.code);

        if (str_cmpx("F", code) == 0)
        {
            *errorCode = ASE_Full;
        }
        else if (str_cmpx("I", code) == 0)
        {
            *errorCode = ASE_InconsistentDatabase;
        }
        else
        {
            *errorCode = ASE_None;
        }
    }

    if (errorValue)
    {
        *errorValue = dbr->sync_error.num;
    }

    return NE_OK;
}


AB_DEF NB_Error
AB_SynchronizationInformationDestroy(AB_SynchronizationInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }
    
    data_sync_places_reply_free(NB_ContextGetDataState(pThis->context), &pThis->reply);

    nsl_free(pThis);        

    return NE_OK;
}


void
ClearSynchronizationOperation(AB_SynchronizationOperation* pSO)
{
    nsl_memset(pSO, 0, sizeof(*pSO));

    ClearNIMPlace(&pSO->place);
    ClearPlaceMessage(&pSO->message);
}


/*! @} */
