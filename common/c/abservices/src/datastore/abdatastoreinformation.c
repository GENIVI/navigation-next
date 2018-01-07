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

    @file abdatastoreinformation.c
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
#include "abdatastoreinformation.h"
#include "abdatastoreinformationprivate.h"
#include "data_datastore_reply.h"


struct AB_DataStoreInformation
{
    NB_Context*                 context;
	data_datastore_reply        reply;
};


NB_Error
AB_DataStoreInformationCreate(NB_Context* context, tpselt reply, AB_DataStoreInformation** information)
{
    AB_DataStoreInformation* pThis = 0;
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
    err = err ? err : data_datastore_reply_from_tps(NB_ContextGetDataState(pThis->context), &pThis->reply, reply);

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
AB_DataStoreInformationDestroy(AB_DataStoreInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }
    
    data_datastore_reply_free(NB_ContextGetDataState(pThis->context), &pThis->reply);

    nsl_free(pThis);        

    return NE_OK;
}


AB_DEF NB_Error
AB_DataStoreInformationGetResultCount(AB_DataStoreInformation* pThis, uint32* operationCount, uint32* completionCode)
{
    NB_Error err = NE_OK;
    
    *operationCount = CSL_VectorGetLength(pThis->reply.vec_datastore_elements);
    *completionCode = pThis->reply.completion_code;
    
    return err;
}


AB_DEF NB_Error
AB_DataStoreInformationGetResult(AB_DataStoreInformation* pThis, uint32 index, byte** data, nb_size* dataSize)
{
    NB_Error err = NE_OK;
    
    if (!pThis || !data || !dataSize)
    {
        return NE_INVAL;
    }

    *data = 0;
    *dataSize = 0;
    
    err = NE_NOENT;

    if (index < (uint32)CSL_VectorGetLength(pThis->reply.vec_datastore_elements))
    {
        data_datastore* pDataStoreElement = (data_datastore*)CSL_VectorGetPointer(pThis->reply.vec_datastore_elements, index);
        if (pDataStoreElement)
        {
            if (pDataStoreElement->id)
            {
                const char* id = data_string_get(NB_ContextGetDataState(pThis->context), &pDataStoreElement->id);
                int size = nsl_strlen(id);                

                *data = nsl_malloc(size + 1);
                if (*data)
                {
                    nsl_memcpy(*data, id, size);
                    (*data)[size] = 0;
                    *dataSize = size + 1;
                    err = NE_OK;
                }
                else
                {
                    err = NE_NOMEM;
                }
            }
            else if (pDataStoreElement->data.size)
            {
                *data = nsl_malloc(pDataStoreElement->data.size);
                if (*data)
                {
                    nsl_memcpy(*data, pDataStoreElement->data.data, pDataStoreElement->data.size);
                    *dataSize = pDataStoreElement->data.size;
                    err = NE_OK;
                }
                else
                {
                    err = NE_NOMEM;
                }
            }
        }
    }
    
    return err;
}


/*! @} */
