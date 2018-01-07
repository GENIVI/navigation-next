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
/* (C) Copyright 2006 by Networks In Motion, Inc.                */
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
 * data_sync_places_status_query.c: created 2006/09/25 by Mark Goddard.
 */

#include "data_sync_places_status_query.h"
#include "vec.h"
#include "abexp.h"

 NB_Error
data_sync_places_status_query_init(data_util_state* pds, data_sync_places_status_query* spq)
{
	NB_Error err = NE_OK;

	spq->vec_dbstatusquery = CSL_VectorAlloc(sizeof(data_sync_places_db_status_query));

	if (spq->vec_dbstatusquery == NULL)
		return NE_NOMEM;

	return err;
}

 void		
data_sync_places_status_query_free(data_util_state* pds, data_sync_places_status_query* spq)
{
	int n,l;

	if (spq->vec_dbstatusquery) {
		l = CSL_VectorGetLength(spq->vec_dbstatusquery);

		for (n=0;n<l;n++)
			data_sync_places_db_status_query_free(pds, (data_sync_places_db_status_query*) CSL_VectorGetPointer(spq->vec_dbstatusquery, n));

		CSL_VectorDealloc(spq->vec_dbstatusquery);
	}

	spq->vec_dbstatusquery = NULL;
}

 tpselt		
data_sync_places_status_query_to_tps(data_util_state* pds, data_sync_places_status_query* spq)
{
	tpselt te;
	tpselt ce = NULL;
	int n,l;

	te = te_new("sync-places-status-query");

	if (te == NULL)
		goto errexit;

	if (spq->vec_dbstatusquery) {
		l = CSL_VectorGetLength(spq->vec_dbstatusquery);

		for (n=0;n<l;n++) {
			data_sync_places_db_status_query* dbq = CSL_VectorGetPointer(spq->vec_dbstatusquery, n);

			if ((ce = data_sync_places_db_status_query_to_tps(pds, dbq)) != NULL && te_attach(te, ce))
				ce = NULL;
			else
				goto errexit;
		}
	}	

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);

	return NULL;
}

 NB_Error	
data_sync_places_status_query_copy(data_util_state* pds, data_sync_places_status_query* spq_dest, data_sync_places_status_query* spq_src)
{
	NB_Error err = NE_OK;
	int n,l;
	data_sync_places_db_status_query	dbquery;

	data_sync_places_db_status_query_init(pds, &dbquery);

	data_sync_places_status_query_free(pds, spq_dest);
	err = err ? err : data_sync_places_status_query_init(pds, spq_dest);
	
	l = CSL_VectorGetLength(spq_src->vec_dbstatusquery);

	for (n=0;n<l && err == NE_OK;n++) {

		err = err ? err : data_sync_places_db_status_query_copy(pds, &dbquery, CSL_VectorGetPointer(spq_src->vec_dbstatusquery, n));
		err = err ? err : CSL_VectorAppend(spq_dest->vec_dbstatusquery, &dbquery) ? NE_OK : NE_NOMEM;

		if (err)
			data_sync_places_db_status_query_free(pds, &dbquery);
		else
			data_sync_places_db_status_query_init(pds, &dbquery);
	}

	return err;
}

static data_sync_places_db_status_query*
data_sync_places_status_query_find_db_status_query(data_util_state* pds, data_sync_places_status_query* spq, const char* dbname)
{
	data_sync_places_db_status_query* dbq;
	int n,l;

	dbq = NULL;
	l = CSL_VectorGetLength(spq->vec_dbstatusquery);

	for (n=0;n<l;n++) {

		dbq = CSL_VectorGetPointer(spq->vec_dbstatusquery, n);

		if (nsl_strcmp(dbq->name, dbname) == 0)
			break;

		dbq = NULL;
	}

	return dbq;
}

 data_sync_places_db_status_query*
data_sync_places_status_query_get_db_status_query(data_util_state* pds, data_sync_places_status_query* spq, const char* dbname)
{
	NB_Error err = NE_OK;
	data_sync_places_db_status_query* dbq;

	dbq = data_sync_places_status_query_find_db_status_query(pds, spq, dbname);

	if (dbq == NULL) {

		data_sync_places_db_status_query n_dbq;

		err = data_sync_places_db_status_query_init(pds, &n_dbq);

		err = err ? err : data_string_set(pds, &n_dbq.name, dbname);

		if (err == NE_OK) {
			if (!CSL_VectorAppend(spq->vec_dbstatusquery, &n_dbq))
				err = NE_NOMEM;

			if (err == NE_OK) {

				nsl_memset(&n_dbq, 0, sizeof(n_dbq)); // clear out the data_sync_places_db_status_query since we have copied it (shallow copy)

				dbq = data_sync_places_status_query_find_db_status_query(pds, spq, dbname);
			}
			else
				data_sync_places_db_status_query_free(pds, &n_dbq); // free the data_sync_places_db_status_query if it was not copied
		}

		if (dbq == NULL)
			err = NE_NOMEM;

		if (err != NE_OK)
			dbq = NULL;			
	}

	return dbq;
}
