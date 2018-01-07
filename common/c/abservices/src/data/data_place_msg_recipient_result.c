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
/* defined in section 499C of the ppmrnal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scoppmr and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * data_place_msg_recipient_result.c: created 2006/10/24 by Mark Goddard.
 */

#include "data_place_msg_recipient_result.h"
#include "data_place_msg_error.h"
#include "data_place_msg_id.h"
#include "abexp.h"

NB_Error
data_place_msg_recipient_result_init(data_util_state* pds, data_place_msg_recipient_result* ppmr)
{
	NB_Error err = NE_OK;

	err = err ? err : data_string_init(pds, &ppmr->to);
	err = err ? err : data_place_msg_error_init(pds, &ppmr->place_msg_error);
	err = err ? err : data_place_msg_id_init(pds, &ppmr->place_msg_id);

	return err;
}

void		
data_place_msg_recipient_result_free(data_util_state* pds, data_place_msg_recipient_result* ppmr)
{
	data_string_free(pds, &ppmr->to);
	data_place_msg_error_free(pds, &ppmr->place_msg_error);
	data_place_msg_id_free(pds, &ppmr->place_msg_id);
}

NB_Error		
data_place_msg_recipient_result_from_tps(data_util_state* pds, data_place_msg_recipient_result* ppmr, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt ce = NULL;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_place_msg_recipient_result_free(pds, ppmr);

	err = data_place_msg_recipient_result_init(pds, ppmr);

	if (err != NE_OK)
		return err;
	
	err = err ? err : data_string_from_tps_attr(pds, &ppmr->to, te, "to");

	ce = te_getchild(te, "place-msg-error");

	if (ce != NULL)
		err = err ? err : data_place_msg_error_from_tps(pds, &ppmr->place_msg_error, ce);

	ce = te_getchild(te, "place-msg-id");

	if (ce != NULL)
		err = err ? err : data_place_msg_id_from_tps(pds, &ppmr->place_msg_id, ce);

errexit:
	if (err != NE_OK)
		data_place_msg_recipient_result_free(pds, ppmr);
	return err;
}

