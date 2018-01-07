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

/*
 * data_message_confirm.c: created 2006/09/30 by Chetan Nagaraj.
 */

#include "data_message_confirm.h"
#include "abexp.h"

NB_Error
data_message_confirm_init(data_util_state* pds, data_message_confirm* pmc)
{
	NB_Error err = NE_OK;

	err = err ? err : data_string_init(pds, &pmc->id);
	err = err ? err : data_string_init(pds, &pmc->confirm_action);

	return err;
}

void		
data_message_confirm_free(data_util_state* pds, data_message_confirm* pmc)
{
	data_string_free(pds, &pmc->id);
	data_string_free(pds, &pmc->confirm_action);
}

tpselt		
data_message_confirm_to_tps(data_util_state* pds, data_message_confirm* pmc)
{
	tpselt te;

	te = te_new("message-confirm");
	
	if (te == NULL)
		goto errexit;

	if (!te_setattrc(te, "id", pmc->id))
		goto errexit;

	if (!te_setattrc(te, "confirm-action", pmc->confirm_action))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	return NULL;
}

NB_Error	
data_message_confirm_from_tps(data_util_state* pds, data_message_confirm* pmc, tpselt te)
{
	NB_Error err = NE_OK;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_message_confirm_free(pds, pmc);

	err = data_message_confirm_init(pds, pmc);

	if (err != NE_OK)
		return err;
	
	err = err ? err : data_string_from_tps_attr(pds, &pmc->id, te, "id");
	err = err ? err : data_string_from_tps_attr(pds, &pmc->confirm_action, te, "confirm-action");

errexit:
	if (err != NE_OK)
		data_message_confirm_free(pds, pmc);
	return err;
}

boolean		
data_message_confirm_equal(data_util_state* pds, data_message_confirm* pmc1, data_message_confirm* pmc2)
{
	return (boolean) (data_string_equal(pds, &pmc1->id, &pmc2->id) && 
					  data_string_equal(pds, &pmc1->confirm_action, &pmc2->confirm_action));
}

NB_Error	
data_message_confirm_copy(data_util_state* pds, data_message_confirm* pmc_dest, data_message_confirm* pmc_src)
{
	NB_Error err = NE_OK;

	data_message_confirm_free(pds, pmc_dest);

	err = err ? err : data_message_confirm_init(pds, pmc_dest);

	err = err ? err : data_string_copy(pds, &pmc_dest->id, &pmc_src->id);
	err = err ? err : data_string_copy(pds, &pmc_dest->confirm_action, &pmc_src->confirm_action);

	return err;
}
