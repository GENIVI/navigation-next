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
 * data_sync_delete_item.c: created 2006/09/25 by Mark Goddard.
 */

#include "data_sync_delete_item.h"
#include "abexp.h"

NB_Error
data_sync_delete_item_init(data_util_state* pds, data_sync_delete_item* pe)
{
	NB_Error err = NE_OK;

	pe->id = 0;

	return err;
}

void		
data_sync_delete_item_free(data_util_state* pds, data_sync_delete_item* pe)
{
	pe->id = 0;
}

NB_Error	
data_sync_delete_item_from_tps(data_util_state* pds, data_sync_delete_item* pe, tpselt te)
{
	NB_Error err = NE_OK;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_sync_delete_item_free(pds, pe);

	err = data_sync_delete_item_init(pds, pe);

	if (err != NE_OK)
		return err;

	if (err == NE_OK && !te_getattru64(te, "id", &pe->id))
		err = NE_BADDATA;

errexit:
	if (err != NE_OK)
		data_sync_delete_item_free(pds, pe);
	return err;
}

tpselt
data_sync_delete_item_to_tps(data_util_state* pds, data_sync_delete_item* pe)
{
	tpselt te;

	te = te_new("sync-delete-item");

	if (te == NULL)
		goto errexit;

	if (!te_setattru64(te, "id", pe->id))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	return NULL;
}
