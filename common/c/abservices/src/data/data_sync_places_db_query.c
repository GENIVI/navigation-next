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
 * data_sync_places_db_query.c: created 2006/09/25 by Mark Goddard.
 */

#include "data_sync_places_db_query.h"
#include "vec.h"
#include "nbutilityprotected.h"
#include "abexp.h"

NB_Error
data_sync_places_db_query_init(data_util_state* pds, data_sync_places_db_query* spdq)
{
	NB_Error err = NE_OK;

	spdq->vec_add_item = NULL;
	spdq->vec_delete_item = NULL;
	spdq->vec_modify_item = NULL;

	if (err == NE_OK) {
		spdq->vec_add_item = CSL_VectorAlloc(sizeof(data_sync_add_item));

		if (spdq->vec_add_item == NULL)
			err = NE_NOMEM;
	}

	if (err == NE_OK) {
		spdq->vec_delete_item = CSL_VectorAlloc(sizeof(data_sync_delete_item));

		if (spdq->vec_delete_item == NULL)
			err = NE_NOMEM;
	}

	if (err == NE_OK) {
		spdq->vec_modify_item = CSL_VectorAlloc(sizeof(data_sync_modify_item));

		if (spdq->vec_modify_item == NULL)
			err = NE_NOMEM;
	}

	err = err ? err : data_string_init(pds, &spdq->name);

	spdq->last_sync_generation = 0;
	spdq->db_id = 0;

	if (err != NE_OK) {

		if (spdq->vec_add_item != NULL)
			CSL_VectorDealloc(spdq->vec_add_item);
		if (spdq->vec_delete_item != NULL)
			CSL_VectorDealloc(spdq->vec_delete_item);
		if (spdq->vec_modify_item != NULL)
			CSL_VectorDealloc(spdq->vec_modify_item);
	}

	return err;
}

void		
data_sync_places_db_query_free(data_util_state* pds, data_sync_places_db_query* spdq)
{
	int n,l;

	if (spdq->vec_add_item) {
		l = CSL_VectorGetLength(spdq->vec_add_item);

		for (n=0;n<l;n++)
			data_sync_add_item_free(pds, (data_sync_add_item*) CSL_VectorGetPointer(spdq->vec_add_item, n));

		CSL_VectorDealloc(spdq->vec_add_item);
	}

	spdq->vec_add_item = NULL;

	if (spdq->vec_delete_item) {
		l = CSL_VectorGetLength(spdq->vec_delete_item);

		for (n=0;n<l;n++)
			data_sync_delete_item_free(pds, (data_sync_delete_item*) CSL_VectorGetPointer(spdq->vec_delete_item, n));

		CSL_VectorDealloc(spdq->vec_delete_item);
	}

	spdq->vec_delete_item = NULL;

	if (spdq->vec_modify_item) {
		l = CSL_VectorGetLength(spdq->vec_modify_item);

		for (n=0;n<l;n++)
			data_sync_modify_item_free(pds, (data_sync_modify_item*) CSL_VectorGetPointer(spdq->vec_modify_item, n));

		CSL_VectorDealloc(spdq->vec_modify_item);
	}

	spdq->vec_modify_item = NULL;

	data_string_free(pds, &spdq->name);

	spdq->last_sync_generation = 0;
	spdq->db_id = 0;
}

tpselt		
data_sync_places_db_query_to_tps(data_util_state* pds, data_sync_places_db_query* spdq)
{
	tpselt te;
	tpselt ce = NULL;
	int n,l;

	te = te_new("sync-places-db-query");

	if (te == NULL)
		goto errexit;

	if (!te_setattrc(te, "name", data_string_get(pds, &spdq->name)))
		goto errexit;

	if (!te_setattru(te, "last-sync-generation", spdq->last_sync_generation))
		goto errexit;

	if (!te_setattru(te, "db-id", spdq->db_id))
		goto errexit;

	if (spdq->vec_delete_item) {
		l = CSL_VectorGetLength(spdq->vec_delete_item);

		for (n=0;n<l;n++) {
			data_sync_delete_item* di = CSL_VectorGetPointer(spdq->vec_delete_item, n);

			if ((ce = data_sync_delete_item_to_tps(pds, di)) != NULL && te_attach(te, ce))
				ce = NULL;
			else
				goto errexit;
		}
	}	

	if (spdq->vec_modify_item) {
		l = CSL_VectorGetLength(spdq->vec_modify_item);

		for (n=0;n<l;n++) {
			data_sync_modify_item* mi = CSL_VectorGetPointer(spdq->vec_modify_item, n);

			if ((ce = data_sync_modify_item_to_tps(pds, mi)) != NULL && te_attach(te, ce))
				ce = NULL;
			else
				goto errexit;
		}
	}	

	if (spdq->vec_add_item) {
		l = CSL_VectorGetLength(spdq->vec_add_item);

		for (n=0;n<l;n++) {
			data_sync_add_item* ai = CSL_VectorGetPointer(spdq->vec_add_item, n);

			if ((ce = data_sync_add_item_to_tps(pds, ai)) != NULL && te_attach(te, ce))
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
data_sync_places_db_query_add_add_item(data_util_state* pds, data_sync_places_db_query* spdq, int64 local_id, nb_unixTime modtime, NB_Place* pPlace, AB_PlaceMessage* pPlaceMessage)
{
    NB_Error err = NE_OK;

    err = data_sync_places_db_query_add_add_item_with_ext_app_content(pds, spdq, local_id, modtime, pPlace, pPlaceMessage, NULL);

    return err;
}

NB_Error
data_sync_places_db_query_add_add_item_with_ext_app_content(data_util_state* pds, data_sync_places_db_query* spdq, int64 local_id, nb_unixTime modtime, NB_Place* pPlace, AB_PlaceMessage* pPlaceMessage, NB_ExtAppContent* pExtAppContent)
{
    NB_Error err = NE_OK;
    data_sync_add_item ai;

    err = data_sync_add_item_init(pds, &ai);

    ai.modtime = (uint32) modtime;

    err = data_string_set_i64(pds, &ai.id, local_id);

    if (pPlace != NULL) {

        err = err ? err : data_place_from_nimplace(pds, &ai.place, pPlace);
        ai.place_valid = TRUE;
    }
    else {

        ai.place_valid = FALSE;
    }

    if (pPlaceMessage != NULL) {

        err = err ? err : data_place_message_from_abplacemessage(pds, &ai.message, pPlaceMessage);
        ai.message_valid = TRUE;
    }
    else {

        ai.message_valid = FALSE;
    }

    if (pExtAppContent)
    {
        err = SetExtAppContentVectorFromNIMExtAppContent(ai.vec_extapp_content, pds, pExtAppContent);
    }

    if (err == NE_OK && !CSL_VectorAppend(spdq->vec_add_item, &ai))
        err = NE_NOMEM;

    return err;
}

NB_Error
data_sync_places_db_query_add_modify_item(data_util_state* pds, data_sync_places_db_query* spdq, uint64 server_id, nb_unixTime modtime, NB_Place* pPlace, AB_PlaceMessage* pPlaceMessage, AB_PlaceMessageFlags placeMessageFlags)
{
    NB_Error err = NE_OK;

    err = data_sync_places_db_query_add_modify_item_with_ext_app_content(pds, spdq, server_id, modtime, pPlace, pPlaceMessage, placeMessageFlags, NULL);

    return err;
}

NB_Error
data_sync_places_db_query_add_modify_item_with_ext_app_content(data_util_state* pds, data_sync_places_db_query* spdq, uint64 server_id, nb_unixTime modtime, NB_Place* pPlace, AB_PlaceMessage* pPlaceMessage, AB_PlaceMessageFlags placeMessageFlags, NB_ExtAppContent* pExtAppContent)
{
    NB_Error err = NE_OK;
    data_sync_modify_item mi;

    err = data_sync_modify_item_init(pds, &mi);

    mi.id = server_id;
    mi.modtime = (uint32) modtime;

    if (pPlace != NULL) {

        err = err ? err : data_place_from_nimplace(pds, &mi.place, pPlace);
        mi.place_valid = TRUE;
    }
    else {

        mi.place_valid = FALSE;
    }

    if (placeMessageFlags != AB_PLACE_MESSAGE_FLAGS_NONE) {

        mi.place_msg_flag_valid = TRUE;
        mi.place_msg_flags.flags = placeMessageFlags;
    }

    if (pExtAppContent)
    {
        err = SetExtAppContentVectorFromNIMExtAppContent(mi.vec_extapp_content, pds, pExtAppContent);
    }

    if (err == NE_OK && !CSL_VectorAppend(spdq->vec_modify_item, &mi))
        err = NE_NOMEM;

    return err;
}

NB_Error	
data_sync_places_db_query_add_delete_item(data_util_state* pds, data_sync_places_db_query* spdq, uint64 id)
{
	NB_Error err = NE_OK;
	data_sync_delete_item di;

	err = data_sync_delete_item_init(pds, &di);

	di.id = id;

	if (!CSL_VectorAppend(spdq->vec_delete_item, &di))
		err = NE_NOMEM;

	return err;
}
