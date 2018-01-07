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
 * data_geocode_reply.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_geocode_reply.h"
#include "vec.h"

NB_Error
data_geocode_reply_init(data_util_state* pds, data_geocode_reply* pgr)
{
	NB_Error err = NE_OK;

	err = err ? err : data_sliceres_init(pds, &pgr->sliceres);

	pgr->vec_locmatch = CSL_VectorAlloc(sizeof(data_locmatch));

	if (pgr->vec_locmatch == NULL)
		return NE_NOMEM;

	return err;
}

void		
data_geocode_reply_free(data_util_state* pds, data_geocode_reply* pgr)
{
	int n,l;

	if (pgr->vec_locmatch) {
		l = CSL_VectorGetLength(pgr->vec_locmatch);

		for (n=0;n<l;n++)
			data_locmatch_free(pds, (data_locmatch*) CSL_VectorGetPointer(pgr->vec_locmatch, n));

		CSL_VectorDealloc(pgr->vec_locmatch);
	}

	data_sliceres_free(pds, &pgr->sliceres);

	pgr->vec_locmatch = NULL;
}

NB_Error	
data_geocode_reply_from_tps(data_util_state* pds, data_geocode_reply* pgr, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;
	int iter;
	data_locmatch	locmatch;
	
	err = data_locmatch_init(pds, &locmatch);

	if (err != NE_OK)
		return err;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_geocode_reply_free(pds, pgr);

	err = data_geocode_reply_init(pds, pgr);

	if (err != NE_OK)
		return err;
	
	ce = te_getchild(te, "sliceres");

	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	err = data_sliceres_from_tps(pds, &pgr->sliceres, ce);

	if (err != NE_OK)
		return err;

	iter = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "locmatch") == 0) {

			err = data_locmatch_from_tps(pds, &locmatch, ce);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(pgr->vec_locmatch, &locmatch))
					err = NE_NOMEM;

				if (err == NE_OK)
					data_locmatch_init(pds, &locmatch); // clear out the locmatch since we have copied it (shallow copy)
				else
					data_locmatch_free(pds, &locmatch); // free the locmatch if it was not copied
			}

			if (err != NE_OK)
				goto errexit;
		}
	}
	
errexit:
	data_locmatch_free(pds, &locmatch);
	if (err != NE_OK)
		data_geocode_reply_free(pds, pgr);
	return err;
}

boolean		
data_geocode_reply_equal(data_util_state* pds, data_geocode_reply* pgr1, data_geocode_reply* pgr2)
{
	int ret;
	int n,l;

	ret = ((l = CSL_VectorGetLength(pgr1->vec_locmatch)) == CSL_VectorGetLength(pgr2->vec_locmatch));
	
	for (n = 0; n < l && ret; n++) {
		
		ret = ret && data_locmatch_equal(pds, (data_locmatch*) CSL_VectorGetPointer(pgr1->vec_locmatch, n), (data_locmatch*) CSL_VectorGetPointer(pgr2->vec_locmatch, n));
	}

	ret = ret && data_sliceres_equal(pds, &pgr1->sliceres, &pgr2->sliceres);

	return (boolean) ret;
}

NB_Error	
data_geocode_reply_copy(data_util_state* pds, data_geocode_reply* pgr_dest, data_geocode_reply* pgr_src)
{
	NB_Error err = NE_OK;
	int n,l;
	data_locmatch	locmatch;

	data_locmatch_init(pds, &locmatch);

	data_geocode_reply_free(pds, pgr_dest);
	err = err ? err : data_geocode_reply_init(pds, pgr_dest);
	
	l = CSL_VectorGetLength(pgr_src->vec_locmatch);

	for (n=0;n<l && err == NE_OK;n++) {

		err = err ? err : data_locmatch_copy(pds, &locmatch, CSL_VectorGetPointer(pgr_src->vec_locmatch, n));
		err = err ? err : CSL_VectorAppend(pgr_dest->vec_locmatch, &locmatch) ? NE_OK : NE_NOMEM;

		if (err)
			data_locmatch_free(pds, &locmatch);
		else
			data_locmatch_init(pds, &locmatch);
	}

	err = err ? err : data_sliceres_copy(pds, &pgr_dest->sliceres, &pgr_src->sliceres);

	return err;
}

