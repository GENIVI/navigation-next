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
/* (C) Copyright 2007 by Networks In Motion, Inc.                */
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
 * data_event_detail.c: created 2007/06/12 by Michael Gilbert.
 */

#include "data_event_detail.h"

NB_Error
data_event_detail_init(data_util_state* pds, data_event_detail* ped)
{
	NB_Error err = NE_OK;

	err = err ? err : data_string_init(pds, &ped->id);
	err = err ? err : data_string_init(pds, &ped->type);
	err = err ? err : data_phone_init(pds, &ped->phone);

	return err;
}

void
data_event_detail_free(data_util_state* pds, data_event_detail* ped)
{
	data_string_free(pds, &ped->id);
	data_string_free(pds, &ped->type);
	data_phone_free(pds, &ped->phone);
}

tpselt
data_event_detail_to_tps(data_util_state* pds, data_event_detail* ped)
{
	tpselt te;
	tpselt ce = NULL;

	te = te_new("event-detail");
	
	if (te == NULL)
		goto errexit;


	if ((ce = data_phone_to_tps(pds, &ped->phone)) != NULL && te_attach(te, ce))
		ce = NULL;
	else
		goto errexit;

	if (nsl_strlen(data_string_get(pds, &ped->id)) > 0)
		if (!te_setattrc(te, "id", data_string_get(pds, &ped->id)))
			goto errexit;

	if (!te_setattrc(te, "type", data_string_get(pds, &ped->type)))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);

	return NULL;
}

NB_Error
data_event_detail_copy(data_util_state* pds, data_event_detail* ped_dest, data_event_detail* ped_src)
{
	NB_Error err = NE_OK;

	data_event_detail_free(pds, ped_dest);

	err = err ? err : data_event_detail_init(pds, ped_dest);

	err = err ? err : data_phone_copy(pds, &ped_dest->phone, &ped_src->phone);
	err = err ? err : data_string_copy(pds, &ped_dest->id, &ped_src->id);
	err = err ? err : data_string_copy(pds, &ped_dest->type, &ped_src->type);

	return err;
}

