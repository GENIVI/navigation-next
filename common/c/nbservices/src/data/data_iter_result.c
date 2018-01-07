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
 * data_iter_result.c: created 2005/01/02 by Mark Goddard.
 */

#include "data_iter_result.h"

static void
data_iter_result_clear(data_util_state* pds, data_iter_result* pir)
{
	pir->exhausted = FALSE;
}

NB_Error	
data_iter_result_init(data_util_state* pds, data_iter_result* pir)
{
	NB_Error err = NE_OK;

	err = err ? err : data_blob_init(pds, &pir->state);

	data_iter_result_clear(pds, pir);

	if (err)
		data_iter_result_free(pds, pir);

	return err;
}

void		
data_iter_result_free(data_util_state* pds, data_iter_result* pir)
{
	data_blob_free(pds, &pir->state);

	data_iter_result_init(pds, pir);
}

NB_Error	
data_iter_result_from_tps(data_util_state* pds, data_iter_result* pir, tpselt te)
{
	NB_Error err = NE_OK;
	char*	data;
	size_t	size;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_iter_result_free(pds, pir);

	err = data_iter_result_init(pds, pir);

	if (err != NE_OK)
		return err;
	
	pir->exhausted = (te_getchild(te, "exhausted") != NULL) ? TRUE : FALSE;

	if (te_getattr(te, "state", &data, &size)) {

		err = err ? err : data_blob_set(pds, &pir->state, (byte*) data, size);
	}
	else {
		err = NE_INVAL;
		goto errexit;
	}

errexit:
	if (err != NE_OK)
		data_iter_result_free(pds, pir);
	return err;
}

boolean		
data_iter_result_equal(data_util_state* pds, data_iter_result* pir1, data_iter_result* pir2)
{
	return	(boolean) (data_blob_equal(pds, &pir1->state, &pir2->state) &&
					   pir1->exhausted == pir2->exhausted);
}

NB_Error	
data_iter_result_copy(data_util_state* pds, data_iter_result* pir_dest, data_iter_result* pir_src)
{
	NB_Error err = NE_OK;

	data_iter_result_free(pds, pir_dest);

	err = err ? err : data_iter_result_init(pds, pir_dest);

	err = err ? err : data_blob_copy(pds, &pir_dest->state, &pir_src->state);

	pir_dest->exhausted = pir_src->exhausted;

	return err;
}

