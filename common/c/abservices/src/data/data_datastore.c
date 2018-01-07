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

    @file     data_datastore.c
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

#include "data_datastore.h"


NB_Error
data_datastore_init(data_util_state* pds, data_datastore* pdatastore)
{
	NB_Error err = NE_OK;

	pdatastore->action = DATASTORE_NONE;

	err = err ? err : data_blob_init(pds, &pdatastore->data);
	err = err ? err : data_string_init(pds, &pdatastore->id);
	
	return err;
}

void		
data_datastore_free(data_util_state* pds, data_datastore* pdatastore)
{
	data_blob_free(pds, &pdatastore->data);
	data_string_free(pds, &pdatastore->id);
}

tpselt
data_datastore_to_tps(data_util_state* pds, data_datastore* pdatastore, datastore_action default_action)
{
	tpselt te = 0;

	te = te_new("data-elem");
	if (!te)
	{
		goto errexit;
	}

    switch (pdatastore->action)
    {
        case DATASTORE_STORE:
			if (!te_setattr(te, "data", (const char*)pdatastore->data.data, pdatastore->data.size)
			    || !te_setattru(te, "size", pdatastore->data.size))
			{
				goto errexit;
			}
            break;
        
        case DATASTORE_RETRIEVE:
        case DATASTORE_DELETE:
			if (!te_setattrc(te, "id", data_string_get(pds, &pdatastore->id)))
			{
				goto errexit;
			}
            break;
            
        default:
            goto errexit;
    }
    
    if (pdatastore->action != default_action && !te_setattrc(te, "override-action", data_datastore_get_action_string(pdatastore->action)))
    {
        goto errexit;
    }

	return te;

errexit:
    te_dealloc(te);
	return 0;
}

NB_Error
data_datastore_from_tps(data_util_state* pds, data_datastore* pdatastore, tpselt te)
{
	NB_Error err = NE_OK;
	const char* action = 0;

	data_datastore_free(pds, pdatastore);

	err = data_datastore_init(pds, pdatastore);
	if (err)
	{
	    return NE_NOMEM;
	}

    pdatastore->action = DATASTORE_NONE;
    action = te_getattrc(te, "override-action");
    if (action)
    {
        if (nsl_strcmp(action, "store") == 0)
        {
            pdatastore->action = DATASTORE_STORE;
        }
        else if (nsl_strcmp(action, "retrieve") == 0)
        {
            pdatastore->action = DATASTORE_RETRIEVE;
        }
        else if (nsl_strcmp(action, "delete") == 0)
        {
            pdatastore->action = DATASTORE_DELETE;
        }
    }
    
    if (te_getattrc(te, "id"))
    {
	    err = data_string_from_tps_attr(pds, &pdatastore->id, te, "id");
    }
    else
    {
	    err = data_blob_from_tps_attr(pds, &pdatastore->data, te, "data");
    }

	if (err != NE_OK)
	{
		data_datastore_free(pds, pdatastore);
	}
	
	return err;
}

const char*
data_datastore_get_action_string(datastore_action action)
{
    switch (action)
    {
        case DATASTORE_STORE:
            return "store";
        case DATASTORE_RETRIEVE:
            return "retrieve";
        case DATASTORE_DELETE:
            return "delete";
        default:
            break;
    }
    return 0;
}

/*! @} */
