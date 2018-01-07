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

    @file     data_datastore_query.c
*/
/*
    See file description in header file.

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

#include "data_datastore_query.h"


NB_Error	
data_datastore_query_init(data_util_state* pds, data_datastore_query* pdsq)
{
	pdsq->vec_datastore_elements = CSL_VectorAlloc(sizeof(data_datastore));

	if (!pdsq->vec_datastore_elements)
	{
	    return NE_NOMEM;
	}

    pdsq->saved_data_action = DATASTORE_NONE;
	pdsq->expire_time_min = 20;
	
	return NE_OK;
}

void		
data_datastore_query_free(data_util_state* pds, data_datastore_query* pdsq)
{
	int l = 0;
	int n = 0;
	
	if (pdsq->vec_datastore_elements)
	{
		l = CSL_VectorGetLength(pdsq->vec_datastore_elements);

		for (n = 0; n < l; n++)
		{
		    data_datastore_free(pds, (data_datastore*) CSL_VectorGetPointer(pdsq->vec_datastore_elements, n));
		}
		CSL_VectorDealloc(pdsq->vec_datastore_elements);
		pdsq->vec_datastore_elements = 0;
	}
}

tpselt		
data_datastore_query_to_tps(data_util_state* pds, data_datastore_query* pdsq)
{
	int l = 0;
	int n = 0;
	
	tpselt te = 0;
	tpselt ce = 0;

    const char* action = 0;
    
	te = te_new("datastore-query");

	l = CSL_VectorGetLength(pdsq->vec_datastore_elements);
	for (n = 0; n < l; n++)
	{
		data_datastore* pdatastore = NULL;
		pdatastore = (data_datastore*)CSL_VectorGetPointer(pdsq->vec_datastore_elements, n);
		if (pdatastore)
		{
		    ce = data_datastore_to_tps(pds, pdatastore, pdsq->saved_data_action);
			if (!ce)
			{
				goto errexit;
			}

			if (!te_attach(te, ce))
			{
				goto errexit;
			}
		}
	}

    action = data_datastore_get_action_string(pdsq->saved_data_action);
    if (action && !te_setattrc(te, "saved-data-action", action))
    {
        goto errexit;
    }
        
	if (!te_setattru(te, "expire-time-min", pdsq->expire_time_min))
    {
		goto errexit;
    }

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);
	return 0;
}


NB_Error
data_datastore_query_add_action(data_util_state* pds, data_datastore_query* pdsq, datastore_action action, byte* data, uint32 dataSize)
{
    NB_Error err = NE_OK;

    data_datastore datastore;

    if (!pds || !pdsq || action == DATASTORE_NONE)
    {
        return NE_INVAL;
    }

    data_datastore_init(pds, &datastore);
    
    if (pdsq->saved_data_action == DATASTORE_NONE)
    {
        pdsq->saved_data_action = action;
    }

    datastore.action = action;

    if (action == DATASTORE_STORE)
    {
        err = data_blob_set(pds, &datastore.data, data, dataSize);
    }
    else
    {
        err = data_string_set(pds, &datastore.id, (char*)data);
    }

    if (err == NE_OK)
    {
        if (!CSL_VectorAppend(pdsq->vec_datastore_elements, &datastore))
        {
            data_datastore_free(pds, &datastore);
            err = NE_NOMEM;
        }
    }
    
    return err;
}   
