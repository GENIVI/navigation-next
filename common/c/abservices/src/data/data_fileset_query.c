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

    @file     data_fileset_query.c
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

#include "data_fileset_query.h"
#include "data_fileset.h"
#include "data_blob.h"
#include "vec.h"

NB_Error
data_fileset_query_init(data_util_state* pds, data_fileset_query* fsq)
{
	NB_Error err = NE_OK;

	fsq->vec_fileset_names = CSL_VectorAlloc(sizeof(data_blob));

	if (fsq->vec_fileset_names == NULL)
	{
		return NE_NOMEM;
	}
	
	return err;
}

void		
data_fileset_query_free(data_util_state* pds, data_fileset_query* fsq)
{
	int n = 0;
	int l = 0;
	
	if (fsq->vec_fileset_names)
	{
		l = CSL_VectorGetLength(fsq->vec_fileset_names);

		for (n = 0; n < l;n++)
		{
			data_blob_free(pds, (data_blob*)CSL_VectorGetPointer(fsq->vec_fileset_names, n));
		}

		CSL_VectorDealloc(fsq->vec_fileset_names);
	}

	fsq->vec_fileset_names = NULL;
}

tpselt		
data_fileset_query_to_tps(data_util_state* pds, data_fileset_query* fsq)
{
	tpselt te;
	tpselt ce = NULL;
	int n,l;

	te = te_new("fileset-download-query");

	if (te == NULL)
		goto errexit;

	if (fsq->vec_fileset_names)
	{
		l = CSL_VectorGetLength(fsq->vec_fileset_names);

		for (n = 0; n < l; n++)
		{
			data_blob* pn = CSL_VectorGetPointer(fsq->vec_fileset_names, n);

			ce = te_new("fileset-request");

			if (ce != NULL)
			{
				if (te_setattr(ce, "name", (const char*)pn->data, pn->size) && te_attach(te, ce))
				{
					ce = NULL;
				}
				else
				{
					goto errexit;
				}
			}
			else 
			{
				goto errexit;
			}
		}
	}

	return te;

errexit:
    te_dealloc(te);
	te_dealloc(ce);
	return NULL;
}

NB_Error
data_fileset_query_add_fileset_name(data_util_state* pds, data_fileset_query* fq, const char* fileSetName, size_t fileSetNameLength)
{
    NB_Error err = NE_OK;
    data_blob name;
    
    err = err ? err : data_blob_init(pds, &name);
    err = err ? err : data_blob_set(pds, &name, (const byte*)fileSetName, fileSetNameLength);
    
    if (err == NE_OK)
    {
        if (!CSL_VectorAppend(fq->vec_fileset_names, &name))
        {
            data_blob_free(pds, &name);
            err = NE_NOMEM;
        }
    }

    return err;
}

/*! @} */
