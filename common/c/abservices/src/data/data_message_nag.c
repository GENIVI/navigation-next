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
 * data_message_nag.c: created 2006/10/02 by Chetan Nagaraj.
 */

#include "data_message_nag.h"
#include "abexp.h"

NB_Error
data_message_nag_init(data_util_state* pds, data_message_nag* pmn)
{
	
	NB_Error err = NE_OK;

	err = err ? err : data_string_init(pds, &pmn->freq_unit);
	pmn->freq_count		= 0;
	err = err ? err : data_string_init(pds, &pmn->expire_unit);
	pmn->expire_count	= 0;
	pmn->active_date	= 0;
	pmn->expire_date	= 0;

	return err;

}

void 
data_message_nag_free(data_util_state* pds, data_message_nag* pmn)
{

	data_string_free(pds, &pmn->freq_unit);
	data_string_free(pds, &pmn->expire_unit);
	pmn->freq_count		= 0;
	pmn->active_date	= 0;
	pmn->expire_count	= 0;
	pmn->expire_date	= 0;

}

NB_Error	
data_message_nag_from_tps(data_util_state* pds, data_message_nag* pmn, tpselt te)
{

	NB_Error err = NE_OK;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_message_nag_free(pds, pmn);

	err = data_message_nag_init(pds, pmn);

	if (err != NE_OK)
		goto errexit;

	err = err ? err : data_string_from_tps_attr(pds, &pmn->freq_unit, te, "freq-units");
	pmn->freq_count		= te_getattru(te, "freq-count");
	err = err ? err : data_string_from_tps_attr(pds, &pmn->expire_unit, te, "expire-units");
	pmn->expire_count	= te_getattru(te, "expire-count");
	pmn->active_date	= te_getattru(te, "active-date");
	pmn->expire_date	= te_getattru(te, "expire-date");

errexit:
	if (err != NE_OK)
		data_message_nag_free(pds, pmn);
	return err;

}

boolean		
data_message_nag_equal(data_util_state* pds, data_message_nag* pmn1, data_message_nag* pmn2)
{

	return (boolean) (data_string_equal(pds, &pmn1->freq_unit, &pmn2->freq_unit) &&
						pmn1->freq_count == pmn2->freq_count &&
						data_string_equal(pds, &pmn1->expire_unit, &pmn2->expire_unit) &&
						pmn1->expire_count == pmn2->expire_count &&
						pmn1->active_date == pmn2->active_date &&
						pmn1->expire_date == pmn2->expire_date);

}

NB_Error	
data_message_nag_copy(data_util_state* pds, data_message_nag* pmn_dest, data_message_nag* pmn_src)
{

	NB_Error err = NE_OK;

	data_message_nag_free(pds, pmn_dest);

	err = err ? err : data_message_nag_init(pds, pmn_dest);
	
	err = err ? err : data_string_copy(pds, &pmn_dest->freq_unit, &pmn_src->freq_unit);
	pmn_dest->freq_count	= pmn_src->freq_count; 
	err = err ? err : data_string_copy(pds, &pmn_dest->freq_unit, &pmn_src->expire_unit);
	pmn_dest->expire_count	= pmn_src->expire_count;
	pmn_dest->active_date	= pmn_src->active_date;
	pmn_dest->expire_date	= pmn_src->expire_date;

	return err;

}

void
data_message_nag_to_buf(data_util_state* pds, data_message_nag* pmn, struct dynbuf* pdb)
{

	data_string_to_buf(pds, &pmn->freq_unit, pdb);
	dbufcat(pdb, (const byte*)&pmn->freq_count, sizeof(uint32));
	data_string_to_buf(pds, &pmn->expire_unit, pdb);
	dbufcat(pdb, (const byte*)&pmn->expire_count, sizeof(uint32));
	dbufcat(pdb, (const byte*)&pmn->active_date, sizeof(uint32));
	dbufcat(pdb, (const byte*)&pmn->expire_date, sizeof(uint32));

}

NB_Error
data_message_nag_from_binary(data_util_state* pds, data_message_nag* pmn, byte** pdata, size_t* pdatalen)
{

	NB_Error err = NE_OK;

	err = err ? err : data_string_from_binary(pds, &pmn->freq_unit, pdata, pdatalen);
	err = err ? err : data_uint32_from_binary(pds, &pmn->freq_count, pdata, pdatalen);
	err = err ? err : data_string_from_binary(pds, &pmn->expire_unit, pdata, pdatalen);
	err = err ? err : data_uint32_from_binary(pds, &pmn->expire_count, pdata, pdatalen);
	err = err ? err : data_uint32_from_binary(pds, &pmn->active_date, pdata, pdatalen);
	err = err ? err : data_uint32_from_binary(pds, &pmn->expire_date, pdata, pdatalen);

	return err;

}
