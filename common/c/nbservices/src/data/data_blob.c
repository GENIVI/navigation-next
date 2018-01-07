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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * data_blob.c: created 2004/12/12 by Mark Goddard.
 */

#include "data_blob.h"
#include "paldebuglog.h"

NB_DEF NB_Error
data_blob_init(data_util_state* pds, data_blob* pb)
{
	pb->data = NULL;
	pb->size = 0;

	return NE_OK;
}

NB_DEF void
data_blob_free(data_util_state* pds, data_blob* pb)
{
    nsl_free(pb->data);
    data_blob_init(pds, pb);
}

NB_DEF NB_Error
data_blob_set(data_util_state* pds, data_blob* pb, const byte* data, size_t size)
{
    NB_Error err = NE_OK;

    data_blob_free(pds, pb);

    if (!data || !size)
    {
        return err;
    }

    err = data_blob_alloc(pds, pb, size);
    if (err == NE_OK)
    {
        nsl_memcpy(pb->data, data, size);
    }

    nsl_assert((pb->size == size));  // data_blob_set [6k]: copy failed

    return err;
}

NB_DEF NB_Error
data_blob_set_cstr(data_util_state* pds, data_blob* ps, const char* data)
{
	return data_blob_set(pds, ps, (const byte*) data, nsl_strlen(data));
}

NB_DEF NB_Error	
data_blob_from_tps_attr(data_util_state* pds, data_blob* pb, tpselt te, const char* name)
{
	char*	data;
	size_t	size;

	if (te == NULL || name == NULL)
		return NE_INVAL;

	if (!te_getattr(te, name, &data, &size))
		return NE_INVAL;

	return data_blob_set(pds, pb, (byte*) data, size);
}

NB_DEF NB_Error	
data_blob_alloc(data_util_state* pds, data_blob* pb, size_t size)
{
	pb->data = nsl_malloc(size);
	pb->size = size;

	if (pb->data == NULL) {
		pb->size = 0;
		return NE_NOMEM;
	}
	
	return NE_OK;
}

NB_DEF boolean		
data_blob_equal(data_util_state* pds, data_blob* pb1, data_blob* pb2)
{
	return (boolean) (pb1->size == pb2->size && nsl_memcmp(pb1->data,pb2->data, pb1->size) == 0);
}

NB_DEF NB_Error	
data_blob_copy(data_util_state* pds, data_blob* pb_dest, data_blob* pb_src)
{
	data_blob_free(pds, pb_dest);

	return data_blob_set(pds, pb_dest, pb_src->data, pb_src->size);
}

NB_DEF void
data_blob_to_buf(data_util_state* pds, data_blob* ps, struct dynbuf *dbp)
{
	dbufcat(dbp, (const byte*) &ps->size, sizeof(ps->size));
	if (ps->size > 0)
		dbufcat(dbp, ps->data, ps->size);
}

NB_DEF NB_Error	
data_blob_from_binary(data_util_state* pds, data_blob* ps, byte** ppdata, size_t* pdatalen)
{
	NB_Error err = NE_OK;
	size_t size;

	if (*pdatalen < sizeof(size))
		return NE_BADDATA;
	
	nsl_memcpy(&size, *ppdata, sizeof(size));

	*ppdata += sizeof(size);
	*pdatalen -= sizeof(size);

	err = data_blob_set(pds, ps, *ppdata, size);

	if (err == NE_OK) {

		*ppdata += size;
		*pdatalen -= size;
	}
	
	return err;
}
