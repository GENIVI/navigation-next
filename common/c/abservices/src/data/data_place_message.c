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
 * data_place_message.c: created 2006/10/18 by Mark Goddard.
 */

#include "data_place_message.h"
#include "absynchronizationtypes.h"
#include "abexp.h"

NB_Error
data_place_message_init(data_util_state* pds, data_place_message* ppm)
{
	NB_Error err = NE_OK;

	err = err ? err : data_string_init(pds, &ppm->from);
	err = err ? err : data_string_init(pds, &ppm->from_name);
	err = err ? err : data_string_init(pds, &ppm->to);
	err = err ? err : data_string_init(pds, &ppm->message);
	err = err ? err : data_string_init(pds, &ppm->signature);
	ppm->stime = 0;
	ppm->flags = AB_PLACE_MESSAGE_FLAGS_NONE;

	err = err ? err : data_place_msg_banner_init(pds, &ppm->place_msg_banner);

	return err;
}

void		
data_place_message_free(data_util_state* pds, data_place_message* ppm)
{
	data_string_free(pds, &ppm->from);
	data_string_free(pds, &ppm->from_name);
	data_string_free(pds, &ppm->to);
	data_string_free(pds, &ppm->message);
	data_string_free(pds, &ppm->signature);
	ppm->stime = 0;
	ppm->flags = AB_PLACE_MESSAGE_FLAGS_NONE;
	data_place_msg_banner_free(pds, &ppm->place_msg_banner);
}

NB_Error	
data_place_message_from_tps(data_util_state* pds, data_place_message* ppm, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt ce;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_place_message_free(pds, ppm);

	err = data_place_message_init(pds, ppm);

	if (err != NE_OK)
		return err;
	
	err = err ? err : data_string_from_tps_attr(pds, &ppm->from, te, "from");
	err = err ? err : data_string_from_tps_attr(pds, &ppm->from_name, te, "from-name");
	err = err ? err : data_string_from_tps_attr(pds, &ppm->to, te, "to");
	err = err ? err : data_string_from_tps_attr(pds, &ppm->message, te, "message");
	err = err ? err : data_string_from_tps_attr(pds, &ppm->signature, te, "signature");
	ppm->stime = te_getattru(te, "stime");
	ppm->flags = te_getattru(te, "flags");

	ce = te_getchild(te, "place-msg-banner");

	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	err = err ? err : data_place_msg_banner_from_tps(pds, &ppm->place_msg_banner, ce);

errexit:
	if (err != NE_OK)
		data_place_message_free(pds, ppm);
	return err;
}

tpselt		
data_place_message_to_tps(data_util_state* pds, data_place_message* ppm)
{
	tpselt te;
	tpselt ce = NULL;

	te = te_new("place-msg");

	if (te == NULL)
		goto errexit;

	if (!te_setattrc(te, "from", data_string_get(pds, &ppm->from)))
		goto errexit;

	if (!te_setattrc(te, "from-name", data_string_get(pds, &ppm->from_name)))
		goto errexit;

	if (!te_setattrc(te, "to", data_string_get(pds, &ppm->to)))
		goto errexit;

	if (!te_setattrc(te, "message", data_string_get(pds, &ppm->message)))
		goto errexit;

	if (!te_setattrc(te, "signature", data_string_get(pds, &ppm->signature)))
		goto errexit;

	if (!te_setattru(te, "stime", (uint32) ppm->stime))
		goto errexit;

	if (!te_setattru(te, "flags", (uint32) ppm->flags))
		goto errexit;

	if ((ce = data_place_msg_banner_to_tps(pds, &ppm->place_msg_banner)) != NULL && te_attach(te, ce))
		ce = NULL;
	else
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);
	return NULL;
}


NB_Error	
data_place_message_from_abplacemessage(data_util_state* pds, data_place_message* ppm, AB_PlaceMessage* pPlaceMessage)
{
    NB_Error err = NE_OK;

    data_place_message_free(pds, ppm);

    err = data_place_message_init(pds, ppm);

    err = err ? err : data_string_set(pds, &ppm->from, pPlaceMessage->from);
    err = err ? err : data_string_set(pds, &ppm->from_name, pPlaceMessage->from_name);
    err = err ? err : data_string_set(pds, &ppm->to, pPlaceMessage->to);
    err = err ? err : data_string_set(pds, &ppm->message, pPlaceMessage->message);
    err = err ? err : data_string_set(pds, &ppm->signature, pPlaceMessage->signature);
    ppm->stime = pPlaceMessage->stime;
    ppm->flags = pPlaceMessage->flags;

    if (err != NE_OK)
        data_place_message_free(pds, ppm);

    return err;
}
