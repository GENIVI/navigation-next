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
 * data_sync_places_status_reply.c: created 2006/09/25 by Mark Goddard.
 */

#include "data_sync_places_status_reply.h"
#include "vec.h"
#include "abexp.h"

 NB_Error
data_sync_places_status_reply_init(data_util_state* pds, data_sync_places_status_reply* spq)
{
	NB_Error err = NE_OK;

	spq->vec_dbstatusreply = CSL_VectorAlloc(sizeof(data_sync_places_db_status_reply));

	if (spq->vec_dbstatusreply == NULL)
		return NE_NOMEM;

	return err;
}

 void		
data_sync_places_status_reply_free(data_util_state* pds, data_sync_places_status_reply* spq)
{
	int n,l;

	if (spq->vec_dbstatusreply) {
		l = CSL_VectorGetLength(spq->vec_dbstatusreply);

		for (n=0;n<l;n++)
			data_sync_places_db_status_reply_free(pds, (data_sync_places_db_status_reply*) CSL_VectorGetPointer(spq->vec_dbstatusreply, n));

		CSL_VectorDealloc(spq->vec_dbstatusreply);
	}

	spq->vec_dbstatusreply = NULL;
}

 NB_Error	
data_sync_places_status_reply_from_tps(data_util_state* pds, data_sync_places_status_reply* spq, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;
	int iter;
	data_sync_places_db_status_reply	dbreply;
	
	err = data_sync_places_db_status_reply_init(pds, &dbreply);

	if (err != NE_OK)
		return err;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_sync_places_status_reply_free(pds, spq);

	err = data_sync_places_status_reply_init(pds, spq);

	if (err != NE_OK)
		return err;
	
	iter = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "sync-places-db-status-reply") == 0) {

			err = data_sync_places_db_status_reply_from_tps(pds, &dbreply, ce);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(spq->vec_dbstatusreply, &dbreply))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&dbreply, 0, sizeof(dbreply)); // clear out the dbreply since we have copied it (shallow copy)
				else
					data_sync_places_db_status_reply_free(pds, &dbreply); // free the dbreply if it was not copied
			}

			if (err != NE_OK)
				goto errexit;
		}
	}
	
errexit:
	data_sync_places_db_status_reply_free(pds, &dbreply);
	if (err != NE_OK)
		data_sync_places_status_reply_free(pds, spq);
	return err;
}

 NB_Error	
data_sync_places_status_reply_copy(data_util_state* pds, data_sync_places_status_reply* spq_dest, data_sync_places_status_reply* spq_src)
{
	NB_Error err = NE_OK;
	int n,l;
	data_sync_places_db_status_reply	dbreply;

	data_sync_places_db_status_reply_init(pds, &dbreply);

	data_sync_places_status_reply_free(pds, spq_dest);
	err = err ? err : data_sync_places_status_reply_init(pds, spq_dest);
	
	l = CSL_VectorGetLength(spq_src->vec_dbstatusreply);

	for (n=0;n<l && err == NE_OK;n++) {

		err = err ? err : data_sync_places_db_status_reply_copy(pds, &dbreply, CSL_VectorGetPointer(spq_src->vec_dbstatusreply, n));
		err = err ? err : CSL_VectorAppend(spq_dest->vec_dbstatusreply, &dbreply) ? NE_OK : NE_NOMEM;

		if (err)
			data_sync_places_db_status_reply_free(pds, &dbreply);
		else
			data_sync_places_db_status_reply_init(pds, &dbreply);
	}

	return err;
}

static data_sync_places_db_status_reply*
data_sync_places_status_reply_find_db_status_reply(data_util_state* pds, data_sync_places_status_reply* spr, const char* dbname)
{
	data_sync_places_db_status_reply* dbr;
	int n,l;

	dbr = NULL;
	l = CSL_VectorGetLength(spr->vec_dbstatusreply);

	for (n=0;n<l;n++) {

		dbr = CSL_VectorGetPointer(spr->vec_dbstatusreply, n);

		if (nsl_strcmp(dbr->name, dbname) == 0)
			break;

		dbr = NULL;
	}

	return dbr;
}

 data_sync_places_db_status_reply*
data_sync_places_status_reply_get_db_status_reply(data_util_state* pds, data_sync_places_status_reply* spr, const char* dbname)
{
	NB_Error err = NE_OK;
	data_sync_places_db_status_reply* dbr;

	dbr = data_sync_places_status_reply_find_db_status_reply(pds, spr, dbname);

	if (dbr == NULL) {

		data_sync_places_db_status_reply n_dbr;

		err = data_sync_places_db_status_reply_init(pds, &n_dbr);

		err = err ? err : data_string_set(pds, &n_dbr.name, dbname);

		if (err == NE_OK) {
			if (!CSL_VectorAppend(spr->vec_dbstatusreply, &n_dbr))
				err = NE_NOMEM;

			if (err == NE_OK) {

				nsl_memset(&n_dbr, 0, sizeof(n_dbr)); // clear out the data_sync_places_db_status_reply since we have copied it (shallow copy)

				dbr = data_sync_places_status_reply_find_db_status_reply(pds, spr, dbname);
			}
			else
				data_sync_places_db_status_reply_free(pds, &n_dbr); // free the data_sync_places_db_status_reply if it was not copied
		}

		if (dbr == NULL)
			err = NE_NOMEM;

		if (err != NE_OK)
			dbr = NULL;			
	}

	return dbr;
}
