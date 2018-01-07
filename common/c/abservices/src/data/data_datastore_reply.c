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

#include "data_datastore_reply.h"
#include "vec.h"
#include "abexp.h"

NB_Error
data_datastore_reply_init(data_util_state* pds, data_datastore_reply* pdsr)
{
	NB_Error err = NE_OK;

	pdsr->vec_datastore_elements = CSL_VectorAlloc(sizeof(data_datastore));

	if (pdsr->vec_datastore_elements == NULL)
	{
		err = NE_NOMEM;
	}

	pdsr->completion_code = 0;

	if (err)
	{
		data_datastore_reply_free(pds, pdsr);
	}

	return err;
}

void		
data_datastore_reply_free(data_util_state* pds, data_datastore_reply* pdsr)
{
	int l = 0;
	int n = 0;
	
	if (pdsr->vec_datastore_elements)
	{
		l = CSL_VectorGetLength(pdsr->vec_datastore_elements);

		for (n = 0; n < l; n++)
		{
			data_datastore_free(pds, (data_datastore*) CSL_VectorGetPointer(pdsr->vec_datastore_elements, n));
		}
		CSL_VectorDealloc(pdsr->vec_datastore_elements);
		pdsr->vec_datastore_elements = 0;
	}
}

NB_Error	
data_datastore_reply_from_tps(data_util_state* pds, data_datastore_reply* pdsr, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt ce = 0;
	int iter = 0;
	
	if (te == NULL)
	{
		err = NE_INVAL;
		goto errexit;
	}

	data_datastore_reply_free(pds, pdsr);

	err = data_datastore_reply_init(pds, pdsr);

	if (err != NE_OK)
	{
		return err;
	}

	while ((ce = te_nextchild(te, &iter)) != NULL)
	{
	    data_datastore datastore;
    	data_datastore_init(pds, &datastore);

		if (nsl_strcmp(te_getname(ce), "data-elem") == 0)
		{
		    err = data_datastore_from_tps(pds, &datastore, ce);

			if (err == NE_OK)
			{
				if (!CSL_VectorAppend(pdsr->vec_datastore_elements, &datastore))
				{
					err = NE_NOMEM;
                }
			}
		}

		if (err != NE_OK)
		{
			goto errexit;
		}
	}

	pdsr->completion_code = te_getattru(te, "completion-code");

errexit:
	if (err != NE_OK)
	{
		data_datastore_reply_free(pds, pdsr);
	}
	return err;
}
