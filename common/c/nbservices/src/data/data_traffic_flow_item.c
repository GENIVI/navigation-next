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

/*-
* data_traffic_flow_items.c: created 2007/11/26 by Mark Goddard.
*/

#include "data_traffic_flow_item.h"

static void
data_traffic_flow_item_clear(data_util_state* pds, data_traffic_flow_item* ptfi)
{
	ptfi->speed = -1;
	ptfi->free_flow_speed = -1;
}

NB_Error	
data_traffic_flow_item_init(data_util_state* pds, data_traffic_flow_item* ptfi)
{
	NB_Error err = NE_OK;

	DATA_MEM_ZERO(ptfi, data_traffic_flow_item);

	DATA_INIT(pds, err, &ptfi->location, data_string);
	DATA_INIT(pds, err, &ptfi->color, data_string);

	data_traffic_flow_item_clear(pds, ptfi);

	return err;
}

void	
data_traffic_flow_item_free(data_util_state* pds, data_traffic_flow_item* ptfi)
{
	DATA_FREE(pds, &ptfi->location, data_string);
	DATA_FREE(pds, &ptfi->color, data_string);

	data_traffic_flow_item_clear(pds, ptfi);
}

NB_Error
data_traffic_flow_item_from_tps(data_util_state* pds, data_traffic_flow_item* ptfi, tpselt te)
{
	NB_Error err = NE_OK;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	DATA_REINIT(pds, err, ptfi, data_traffic_flow_item);

	if (err != NE_OK)
		return err;

	if (!err && !te_getattrd(te, "speed", &ptfi->speed)) {

		err = NE_INVAL;
	}

	err = err ? err : data_string_from_tps_attr(pds, &ptfi->location, te, "location");
	err = err ? err : data_string_from_tps_attr(pds, &ptfi->color, te, "color");

	if (!err && !te_getattrd(te, "free-flow-speed", &ptfi->free_flow_speed)) {

		ptfi->free_flow_speed = -1;
	}

errexit:
	if (err != NE_OK) {

		DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_traffic_flow_item: (error_code: %d)", err));
		DATA_FREE(pds, ptfi, data_traffic_flow_item);
	}

	return err;
}

boolean
data_traffic_flow_item_equal(data_util_state* pds, data_traffic_flow_item* ptfi1, data_traffic_flow_item* ptfi2)
{
	return (boolean) (data_string_equal(pds, &ptfi1->location, &ptfi2->location) && 
					  data_string_equal(pds, &ptfi1->color, &ptfi2->color) &&
					  ptfi1->speed == ptfi2->speed);
}

NB_Error
data_traffic_flow_item_copy(data_util_state* pds, data_traffic_flow_item* ptfi_dest, data_traffic_flow_item* ptfi_src)
{
	NB_Error err = NE_OK;

	DATA_REINIT(pds, err, ptfi_dest, data_traffic_flow_item);

	DATA_COPY(pds, err, &ptfi_dest->location, &ptfi_src->location, data_string);
	DATA_COPY(pds, err, &ptfi_dest->color, &ptfi_src->color, data_string);

	ptfi_dest->speed			= ptfi_src->speed;
	ptfi_dest->free_flow_speed	= ptfi_src->free_flow_speed;
	ptfi_dest->new_item			= ptfi_src->new_item; 

	return err;
}
