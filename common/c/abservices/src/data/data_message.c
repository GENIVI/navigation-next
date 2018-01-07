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
 * data_message.c: created 2006/09/29 by Chetan Nagaraj.
 */

#include "data_message.h"
#include "abexp.h"
#include "palclock.h"

NB_Error
data_message_init(data_util_state* pds, data_message* pm)
{

	NB_Error err = NE_OK;

	err = err ? err : data_formatted_text_init(pds, &pm->formatted_text);
	err = err ? err : data_message_nag_init(pds, &pm->message_nag);
	err = err ? err : data_url_init(pds, &pm->url);	
	err = err ? err : data_string_init(pds, &pm->id);
	err = err ? err : data_string_init(pds, &pm->title);
	err = err ? err : data_string_init(pds, &pm->type);
	err = err ? err : data_string_init(pds, &pm->language);
	err = err ? err : data_string_init(pds, &pm->accept_text);
	err = err ? err : data_string_init(pds, &pm->center_text);
	err = err ? err : data_string_init(pds, &pm->decline_text);

	pm->confirm			= FALSE;
	pm->exit_on_decline	= FALSE;

	pm->time = PAL_ClockGetUnixTime();

	return err;

}

void		
data_message_free(data_util_state* pds, data_message* pm)
{
	
	data_formatted_text_free(pds, &pm->formatted_text);
	data_message_nag_free(pds, &pm->message_nag);
	data_url_free(pds, &pm->url);
	data_string_free(pds, &pm->id);
	data_string_free(pds, &pm->title);
	data_string_free(pds, &pm->type);
	data_string_free(pds, &pm->language);
	data_string_free(pds, &pm->accept_text);
	data_string_free(pds, &pm->center_text);
	data_string_free(pds, &pm->decline_text);

	pm->time = 0;
}

NB_Error	
data_message_from_tps(data_util_state* pds, data_message* pm, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt ce;

	data_message_free(pds, pm);

	err = data_message_init(pds, pm);

	if (err != NE_OK)
		return err;

	ce = te_getchild(te, "formatted-text");
	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}
	err = err ? err : data_formatted_text_from_tps(pds, &pm->formatted_text, ce);
	
	ce = te_getchild(te, "message-nag");
	if (ce != NULL) {
		err = err ? err : data_message_nag_from_tps(pds, &pm->message_nag, ce);
	}
	
	ce = te_getchild(te, "url");
	if (ce != NULL)
		err = err ? err : data_url_from_tps(pds, &pm->url, ce);
	
	pm->confirm = te_getchild(te, "confirm") ? TRUE : FALSE;
	pm->exit_on_decline = te_getchild(te, "exit-on-decline") ? TRUE : FALSE;

	err = err ? err : data_string_from_tps_attr(pds, &pm->id, te, "id");
	err = err ? err : data_string_from_tps_attr(pds, &pm->title, te, "title");
	err = err ? err : data_string_from_tps_attr(pds, &pm->type, te, "type");
	err = err ? err : data_string_from_tps_attr(pds, &pm->language, te, "language");
	err = err ? err : data_string_from_tps_attr(pds, &pm->accept_text, te, "accept-text");
	err = err ? err : data_string_from_tps_attr(pds, &pm->center_text, te, "center-text");
	err = err ? err : data_string_from_tps_attr(pds, &pm->decline_text, te, "decline-text");

	pm->time = PAL_ClockGetUnixTime();

errexit:
	if (err != NE_OK)
		data_message_free(pds, pm);
	return err;

}

boolean		
data_message_equal(data_util_state* pds, data_message* pm1, data_message* pm2)
{

	return	(boolean) (data_formatted_text_equal(pds, &pm1->formatted_text, &pm2->formatted_text) &&
						data_message_nag_equal(pds, &pm1->message_nag, &pm2->message_nag) &&
						data_url_equal(pds, &pm1->url, &pm2->url) &&
						data_string_equal(pds, &pm1->id, &pm2->id) && 
						data_string_equal(pds, &pm1->title, &pm2->title) && 
						data_string_equal(pds, &pm1->type, &pm2->type) &&
						data_string_equal(pds, &pm1->language, &pm2->language) &&
						data_string_equal(pds, &pm1->accept_text, &pm2->accept_text) &&
						data_string_equal(pds, &pm1->center_text, &pm2->center_text) &&
						data_string_equal(pds, &pm1->decline_text, &pm2->decline_text) &&
						pm1->confirm == pm2->confirm &&
						pm1->exit_on_decline == pm2->exit_on_decline);
}

NB_Error	
data_message_copy(data_util_state* pds, data_message* pm_dest, data_message* pm_src)
{

	NB_Error err = NE_OK;

	err = err ? err : data_formatted_text_copy(pds, &pm_dest->formatted_text, &pm_src->formatted_text);
	err = err ? err : data_message_nag_copy(pds, &pm_dest->message_nag, &pm_src->message_nag);
	err = err ? err : data_url_copy(pds, &pm_dest->url, &pm_src->url);
	err = err ? err : data_string_copy(pds, &pm_dest->id, &pm_src->id);
	err = err ? err : data_string_copy(pds, &pm_dest->title, &pm_src->title);
	err = err ? err : data_string_copy(pds, &pm_dest->type, &pm_src->type);
	err = err ? err : data_string_copy(pds, &pm_dest->language, &pm_dest->language);
	err = err ? err : data_string_copy(pds, &pm_dest->accept_text, &pm_src->accept_text);
	err = err ? err : data_string_copy(pds, &pm_dest->center_text, &pm_src->center_text);
	err = err ? err : data_string_copy(pds, &pm_dest->decline_text, &pm_src->decline_text);

	pm_dest->confirm = pm_src->confirm;
	pm_dest->exit_on_decline = pm_src->exit_on_decline;
	pm_dest->time = pm_src->time;
	
	return err;

}

void
data_message_to_buf(data_util_state* pds, data_message* pm, struct dynbuf* pdb)
{
	uint32 i = 0;

	data_formatted_text_to_buf(pds, &pm->formatted_text, pdb);
	data_message_nag_to_buf(pds, &pm->message_nag, pdb);

	if (!nsl_strempty(pm->url.value))
		i = 1;
	dbufcat(pdb, (const byte*)&i, sizeof(uint32));
	if (i == 1)
		data_url_to_buf(pds, &pm->url, pdb);

	i = pm->confirm ? 1 : 0;
	dbufcat(pdb, (const byte*)&i, sizeof(uint32));

	i = pm->exit_on_decline ? 1 : 0;
	dbufcat(pdb, (const byte*)&i, sizeof(uint32));

	data_string_to_buf(pds, &pm->id, pdb);
	data_string_to_buf(pds, &pm->title, pdb);
	data_string_to_buf(pds, &pm->type, pdb);
	data_string_to_buf(pds, &pm->language, pdb);
	data_string_to_buf(pds, &pm->accept_text, pdb);
	data_string_to_buf(pds, &pm->center_text, pdb);
	data_string_to_buf(pds, &pm->decline_text, pdb);
}

NB_Error
data_message_from_binary(data_util_state* pds, data_message* pm, byte** pdata, size_t* pdatalen)
{
	
	NB_Error err = NE_OK;
	uint32 i = 0;
	
	err = err ? err : data_formatted_text_from_binary(pds, &pm->formatted_text, pdata, pdatalen);
	err = err ? err : data_message_nag_from_binary(pds, &pm->message_nag, pdata, pdatalen);

	err = err ? err : data_uint32_from_binary(pds, &i, pdata, pdatalen);
	if (i == 1)
		err = err ? err : data_url_from_binary(pds, &pm->url, pdata, pdatalen);

	err = err ? err : data_uint32_from_binary(pds, &i, pdata, pdatalen);
	if (i == 0)
		pm->confirm = FALSE;
	else
		pm->confirm = TRUE;

	err = err ? err : data_uint32_from_binary(pds, &i, pdata, pdatalen);
	if (i == 0)
		pm->exit_on_decline = FALSE;
	else
		pm->exit_on_decline = TRUE;

	err = err ? err : data_string_from_binary(pds, &pm->id, pdata, pdatalen);
	err = err ? err : data_string_from_binary(pds, &pm->title, pdata, pdatalen);
	err = err ? err : data_string_from_binary(pds, &pm->type, pdata, pdatalen);
	err = err ? err : data_string_from_binary(pds, &pm->language, pdata, pdatalen);
	err = err ? err : data_string_from_binary(pds, &pm->accept_text, pdata, pdatalen);
	err = err ? err : data_string_from_binary(pds, &pm->center_text, pdata, pdatalen);
	err = err ? err : data_string_from_binary(pds, &pm->decline_text, pdata, pdatalen);

	pm->time = PAL_ClockGetUnixTime();

	return err;

}

#define MESSAGE_TYPE_EULA		"eula"
#define MESSAGE_TYPE_UPGRADE	"upgrade"
#define MESSAGE_TYPE_MOTD		"motd"

static int
data_message_priority(const data_message* pm)
{
	//1. type == upgrade and exit_on_decline
	//2. type == eula
	//3. type == upgrade and !exit_on_decline
	//4. type == motd

	if (nsl_strcmp(pm->type, MESSAGE_TYPE_UPGRADE) == 0) {
		if (pm->exit_on_decline)
			return 1;
		else
			return 3;
	}

	if (nsl_strcmp(pm->type, MESSAGE_TYPE_EULA) == 0)
		return 2;

	return 4;
}

int
data_message_compare(const data_message* pA, const data_message* pB)
{
	if (pA == NULL && pB == NULL)
		return 0;

	if (pA != NULL && pB == NULL)
		return 1;
	
	if (pA == NULL && pB != NULL)
		return -1;

	if (data_message_priority(pA) < data_message_priority(pB))
		return -1;

	if (data_message_priority(pA) > data_message_priority(pB))
		return 1;

	if (pA->time < pB->time)
		return -1;

	if (pA->time > pB->time)
		return 1;
	
	return 0;
}
