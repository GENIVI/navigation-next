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
 * data_sync_places_db_reply.c: created 2006/09/25 by Mark Goddard.
 */

#include "data_sync_places_db_reply.h"
#include "vec.h"
#include "tpsdebug.h"
#include "abexp.h"

 NB_Error
data_sync_places_db_reply_init(data_util_state* pds, data_sync_places_db_reply* spdr)
{
	NB_Error err = NE_OK;

	spdr->vec_add_item = NULL;
	spdr->vec_delete_item = NULL;
	spdr->vec_modify_item = NULL;
	spdr->vec_assign_item_id = NULL;

	if (err == NE_OK) {
		spdr->vec_add_item = CSL_VectorAlloc(sizeof(data_sync_add_item));

		if (spdr->vec_add_item == NULL)
			err = NE_NOMEM;
	}

	if (err == NE_OK) {
		spdr->vec_delete_item = CSL_VectorAlloc(sizeof(data_sync_delete_item));

		if (spdr->vec_delete_item == NULL)
			err = NE_NOMEM;
	}

	if (err == NE_OK) {
		spdr->vec_modify_item = CSL_VectorAlloc(sizeof(data_sync_modify_item));

		if (spdr->vec_modify_item == NULL)
			err = NE_NOMEM;
	}

	if (err == NE_OK) {
		spdr->vec_assign_item_id = CSL_VectorAlloc(sizeof(data_sync_assign_item_id));

		if (spdr->vec_assign_item_id == NULL)
			err = NE_NOMEM;
	}

	err = err ? err : data_sync_error_init(pds, &spdr->sync_error);
	spdr->sync_error_valid = FALSE;

	err = err ? err : data_string_init(pds, &spdr->name);

	spdr->new_sync_generation = 0;
	spdr->db_id = 0;
	spdr->count = 0;

	if (err != NE_OK) {

		if (spdr->vec_add_item != NULL)
			CSL_VectorDealloc(spdr->vec_add_item);
		if (spdr->vec_delete_item != NULL)
			CSL_VectorDealloc(spdr->vec_delete_item);
		if (spdr->vec_modify_item != NULL)
			CSL_VectorDealloc(spdr->vec_modify_item);
		if (spdr->vec_assign_item_id != NULL)
			CSL_VectorDealloc(spdr->vec_assign_item_id);
	}

	return err;
}

 void		
data_sync_places_db_reply_free(data_util_state* pds, data_sync_places_db_reply* spdr)
{
	int n,l;

	if (spdr->vec_add_item) {
		l = CSL_VectorGetLength(spdr->vec_add_item);

		for (n=0;n<l;n++)
			data_sync_add_item_free(pds, (data_sync_add_item*) CSL_VectorGetPointer(spdr->vec_add_item, n));

		CSL_VectorDealloc(spdr->vec_add_item);
	}

	spdr->vec_add_item = NULL;

	if (spdr->vec_delete_item) {
		l = CSL_VectorGetLength(spdr->vec_delete_item);

		for (n=0;n<l;n++)
			data_sync_delete_item_free(pds, (data_sync_delete_item*) CSL_VectorGetPointer(spdr->vec_delete_item, n));

		CSL_VectorDealloc(spdr->vec_delete_item);
	}

	spdr->vec_delete_item = NULL;

	if (spdr->vec_modify_item) {
		l = CSL_VectorGetLength(spdr->vec_modify_item);

		for (n=0;n<l;n++)
			data_sync_modify_item_free(pds, (data_sync_modify_item*) CSL_VectorGetPointer(spdr->vec_modify_item, n));

		CSL_VectorDealloc(spdr->vec_modify_item);
	}

	spdr->vec_modify_item = NULL;

	if (spdr->vec_assign_item_id) {
		l = CSL_VectorGetLength(spdr->vec_assign_item_id);

		for (n=0;n<l;n++)
			data_sync_assign_item_id_free(pds, (data_sync_assign_item_id*) CSL_VectorGetPointer(spdr->vec_assign_item_id, n));

		CSL_VectorDealloc(spdr->vec_assign_item_id);
	}

	spdr->vec_assign_item_id = NULL;

	data_sync_error_free(pds, &spdr->sync_error);
	spdr->sync_error_valid = FALSE;

	data_string_free(pds, &spdr->name);

	spdr->new_sync_generation = 0;
	spdr->db_id = 0;
	spdr->count = 0;
}

 NB_Error	
data_sync_places_db_reply_from_tps(data_util_state* pds, data_sync_places_db_reply* spdr, tpselt te)
{
	NB_Error err = NE_OK;
	data_sync_add_item			add_item;
	data_sync_delete_item		delete_item;
	data_sync_modify_item		modify_item;
	data_sync_assign_item_id	assign_item_id;
	tpselt	ce;
	int iter;
	
	if (te == NULL)
		return NE_INVAL;

	nsl_memset(&add_item, 0, sizeof(add_item));
	nsl_memset(&delete_item, 0, sizeof(delete_item));
	nsl_memset(&modify_item, 0, sizeof(modify_item));
	nsl_memset(&assign_item_id, 0, sizeof(assign_item_id));

	err = data_sync_add_item_init(pds, &add_item);

	if (err != NE_OK)
		goto errexit;

	err = data_sync_delete_item_init(pds, &delete_item);

	if (err != NE_OK)
		goto errexit;

	err = data_sync_modify_item_init(pds, &modify_item);

	if (err != NE_OK)
		goto errexit;

	err = data_sync_assign_item_id_init(pds, &assign_item_id);

	if (err != NE_OK)
		goto errexit;

	data_sync_places_db_reply_free(pds, spdr);

	err = data_sync_places_db_reply_init(pds, spdr);

	if (err != NE_OK)
		goto errexit;

	err = data_string_from_tps_attr(pds, &spdr->name, te, "name");

	if (err != NE_OK)
		goto errexit;

	spdr->new_sync_generation = te_getattru(te, "new-sync-generation");
	spdr->db_id = te_getattru(te, "db-id");
	spdr->count = te_getattru(te, "count");
	
	iter = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "sync-add-item") == 0) {

			err = data_sync_add_item_from_tps(pds, &add_item, ce);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(spdr->vec_add_item, &add_item))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&add_item, 0, sizeof(add_item)); // clear out the add_item since we have copied it (shallow copy)
				else
					data_sync_add_item_free(pds, &add_item); // free the add_item if it was not copied
			}

			if (err != NE_OK){
#ifndef NDEBUG
				DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("Place_sync error sync-add-item"));
				dumpelt(ce, 0);
#endif
				goto errexit;
			}
		}
		else if (nsl_strcmp(te_getname(ce), "sync-delete-item") == 0) {

			err = data_sync_delete_item_from_tps(pds, &delete_item, ce);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(spdr->vec_delete_item, &delete_item))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&delete_item, 0, sizeof(delete_item)); // clear out the delete_item since we have copied it (shallow copy)
				else
					data_sync_delete_item_free(pds, &delete_item); // free the delete_item if it was not copied
			}

			if (err != NE_OK){
#ifndef NDEBUG
				DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("Place_sync error sync-delete-item"));
				dumpelt(ce, 0);
#endif
				goto errexit;
			}
		}
		else if (nsl_strcmp(te_getname(ce), "sync-modify-item") == 0) {

			err = data_sync_modify_item_from_tps(pds, &modify_item, ce);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(spdr->vec_modify_item, &modify_item))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&modify_item, 0, sizeof(modify_item)); // clear out the modify_item since we have copied it (shallow copy)
				else
					data_sync_modify_item_free(pds, &modify_item); // free the modify_item if it was not copied
			}

			if (err != NE_OK){
#ifndef NDEBUG
				DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("Place_sync error sync-modify-item"));
				dumpelt(ce, 0);
#endif
				goto errexit;
			}
		}
		else if (nsl_strcmp(te_getname(ce), "sync-assign-item-id") == 0) {

			err = data_sync_assign_item_id_from_tps(pds, &assign_item_id, ce);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(spdr->vec_assign_item_id, &assign_item_id))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&assign_item_id, 0, sizeof(assign_item_id)); // clear out the assign_item_id since we have copied it (shallow copy)
				else
					data_sync_assign_item_id_free(pds, &assign_item_id); // free the assign_item_id if it was not copied
			}

			if (err != NE_OK){
#ifndef NDEBUG
				DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("Place_sync error sync-assign-item"));
				dumpelt(ce, 0);
#endif
				goto errexit;
			}
		}
		else if (nsl_strcmp(te_getname(ce), "sync-error") == 0) {

			err = data_sync_error_from_tps(pds, &spdr->sync_error, ce);

			if (err == NE_OK) {

				spdr->sync_error_valid = TRUE;
			}
		}
	}
	
errexit:

	data_sync_add_item_free(pds, &add_item);
	data_sync_delete_item_free(pds, &delete_item);
	data_sync_modify_item_free(pds, &modify_item);
	data_sync_assign_item_id_free(pds, &assign_item_id);

	if (err != NE_OK)
		data_sync_places_db_reply_free(pds, spdr);

	return err;
}
