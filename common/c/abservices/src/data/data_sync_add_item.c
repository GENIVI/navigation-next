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
 * data_sync_add_item.c: created 2005/01/02 by Mark Goddard.
 */

#include "data_sync_add_item.h"
#include "data_extapp_content.h"
#include "abexp.h"

NB_Error
data_sync_add_item_init(data_util_state* pds, data_sync_add_item* pe)
{
	NB_Error err = NE_OK;

	err = err ? err : data_place_init(pds, &pe->place);
	err = err ? err : data_place_message_init(pds, &pe->message);
	err = err ? err : data_string_init(pds, &pe->id);
	pe->modtime = 0;

	DATA_VEC_ALLOC(err, pe->vec_extapp_content, data_extapp_content);

	return err;
}

void		
data_sync_add_item_free(data_util_state* pds, data_sync_add_item* pe)
{
	data_place_free(pds, &pe->place);
	data_place_message_free(pds, &pe->message);
	data_string_free(pds, &pe->id);
	pe->modtime = 0;

	DATA_VEC_FREE(pds, pe->vec_extapp_content, data_extapp_content);
}

NB_Error	
data_sync_add_item_from_tps(data_util_state* pds, data_sync_add_item* pe, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt ce;
	int iter = 0;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_sync_add_item_free(pds, pe);

	err = data_sync_add_item_init(pds, pe);

	if (err != NE_OK)
		return err;

	ce = te_getchild(te, "place");

	if (ce == NULL)
		err = NE_BADDATA;
	
	err = err ? err : data_place_from_tps(pds, &pe->place, ce);
	pe->place_valid = TRUE;

	ce = te_getchild(te, "place-msg");

	if (ce != NULL) {
	
		err = err ? err : data_place_message_from_tps(pds, &pe->message, ce);
		pe->message_valid = TRUE;
	}

	while ((ce = te_nextchild(te, &iter)) != NULL)
	{
		if (nsl_strcmp(te_getname(ce), "extapp-content") == 0)
		{
			DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pe->vec_extapp_content, data_extapp_content);
		}

		if (err != NE_OK)
		{
			goto errexit;
		}
	}

	err = err ? err : data_string_from_tps_attr(pds, &pe->id, te, "id");

	pe->modtime = te_getattru(te, "modtime");

errexit:
	if (err != NE_OK)
		data_sync_add_item_free(pds, pe);
	return err;
}

tpselt		
data_sync_add_item_to_tps(data_util_state* pds, data_sync_add_item* pe)
{
	tpselt te;
	tpselt ce = NULL;

	te = te_new("sync-add-item");

	if (te == NULL)
		goto errexit;

	if (pe->place_valid) {

		if ((ce = data_place_to_tps(pds, &pe->place)) != NULL && te_attach(te, ce))
			ce = NULL;
		else
			goto errexit;
	}

	if (pe->message_valid) {

		if ((ce = data_place_message_to_tps(pds, &pe->message)) != NULL && te_attach(te, ce))
			ce = NULL;
		else
			goto errexit;
	}

	if (pe->vec_extapp_content)
	{
		int n = 0;
		int l = 0;

		l = CSL_VectorGetLength(pe->vec_extapp_content);

		for (n = 0; n < l; n++)
		{
			data_extapp_content* eac = CSL_VectorGetPointer(pe->vec_extapp_content, n);
			if ((ce = data_extapp_content_to_tps(pds, eac)) != NULL && te_attach(te, ce))
			{
				ce = NULL;
			}
			else
			{
				goto errexit;
			}
		}
	}

	if (!te_setattrc(te, "id", data_string_get(pds, &pe->id)))
		goto errexit;

	if (!te_setattru(te, "modtime", pe->modtime))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);

	return NULL;
}
