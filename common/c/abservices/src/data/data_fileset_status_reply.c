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

    @file     data_fileset_status_reply.c
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

#include "data_fileset_status_reply.h"
#include "data_fileset.h"
#include "vec.h"


NB_Error
data_fileset_status_reply_init(data_util_state* pds, data_fileset_status_reply* fsr)
{
	fsr->vec_fileset_statuses = CSL_VectorAlloc(sizeof(data_fileset));

	if (!fsr->vec_fileset_statuses)
	{
		return NE_NOMEM;
	}

	return NE_OK;
}

void		
data_fileset_status_reply_free(data_util_state* pds, data_fileset_status_reply* fsr)
{
	int n = 0;
	int l = 0;

	if (fsr->vec_fileset_statuses)
	{
		l = CSL_VectorGetLength(fsr->vec_fileset_statuses);

		for (n = 0; n < l; n++)
		{
			data_fileset_free(pds, (data_fileset*) CSL_VectorGetPointer(fsr->vec_fileset_statuses, n));
		}

		CSL_VectorDealloc(fsr->vec_fileset_statuses);
	}

	fsr->vec_fileset_statuses = 0;
}

NB_Error	
data_fileset_status_reply_from_tps(data_util_state* pds, data_fileset_status_reply* fsr, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt ce = 0;
	int iter = 0;
	
	data_fileset fileset;
	
	err = data_fileset_init(pds, &fileset);

	if (err != NE_OK)
	{
		return err;
	}

	if (te == NULL)
	{
		err = NE_INVAL;
		goto errexit;
	}

	data_fileset_status_reply_free(pds, fsr);

	err = data_fileset_status_reply_init(pds, fsr);

	if (err != NE_OK)
	{
		return err;
	}
	
	iter = 0;

	while ((ce = te_nextchild(te, &iter)) != 0)
	{
		if (nsl_strcmp(te_getname(ce), "fileset") == 0)
		{
			err = data_fileset_from_tps(pds, &fileset, ce);

			if (err == NE_OK)
			{
				if (!CSL_VectorAppend(fsr->vec_fileset_statuses, &fileset))
				{
					err = NE_NOMEM;
				}

				if (err == NE_OK)
				{
					nsl_memset(&fileset, 0, sizeof(fileset)); // clear out the fileset since we have copied it (shallow copy)
				}
				else
				{
					data_fileset_free(pds, &fileset); // free the fileset if it was not copied
				}
			}

			if (err != NE_OK)
			{
				goto errexit;
			}
		}
	}
	
errexit:
	data_fileset_free(pds, &fileset);
	if (err != NE_OK)
	{
		data_fileset_status_reply_free(pds, fsr);
	}
	return err;
}

uint32
data_fileset_status_reply_num_filesets(data_fileset_status_reply* fsr)
{
	return CSL_VectorGetLength(fsr->vec_fileset_statuses);
}

data_fileset*
data_fileset_status_reply_get_fileset(data_fileset_status_reply* fsr, int n)
{
	data_fileset* pResult = 0;
	int l = CSL_VectorGetLength(fsr->vec_fileset_statuses);
	
	if (n >= 0 && n < l)
	{
        pResult = CSL_VectorGetPointer(fsr->vec_fileset_statuses, n);
    }

	return pResult;
}


/*! @} */
