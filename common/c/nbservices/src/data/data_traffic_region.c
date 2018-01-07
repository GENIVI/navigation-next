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
* data_traffic_region.c: created 2007/11/26 by Mark Goddard.
*/

#include "data_traffic_region.h"

static void
data_traffic_region_clear(data_util_state* pds, data_traffic_region* ptr)
{
	ptr->start = -1;
	ptr->length = -1;
}

NB_Error	
data_traffic_region_init(data_util_state* pds, data_traffic_region* ptr)
{
	NB_Error err = NE_OK;

	DATA_MEM_ZERO(ptr, data_traffic_region);

	DATA_INIT(pds, err, &ptr->location, data_string);

	data_traffic_region_clear(pds, ptr);

	return err;
}

void	
data_traffic_region_free(data_util_state* pds, data_traffic_region* ptr)
{
	DATA_FREE(pds, &ptr->location, data_string);

	data_traffic_region_clear(pds, ptr);
}

NB_Error
data_traffic_region_from_tps(data_util_state* pds, data_traffic_region* ptr, tpselt te)
{
	NB_Error err = NE_OK;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	DATA_REINIT(pds, err, ptr, data_traffic_region);

	if (err != NE_OK)
		return err;

	if (!err && !te_getattrd(te, "start", &ptr->start))
		err = NE_INVAL;

	if (!err && !te_getattrd(te, "length", &ptr->length))
		err = NE_INVAL;

	err = err ? err : data_string_from_tps_attr(pds, &ptr->location, te, "location");

errexit:
	if (err != NE_OK) {

		DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_traffic_region: (error_code: %d)", err));
		DATA_FREE(pds, ptr, data_traffic_region);
	}

	return err;
}

boolean
data_traffic_region_equal(data_util_state* pds, data_traffic_region* ptr1, data_traffic_region* ptr2)
{
	return (boolean) (data_string_equal(pds, &ptr1->location, &ptr2->location) && 
					  ptr1->start == ptr2->start && ptr1->length == ptr2->length);
}

NB_Error
data_traffic_region_copy(data_util_state* pds, data_traffic_region* ptr_dest, data_traffic_region* ptr_src)
{
	NB_Error err = NE_OK;

	DATA_REINIT(pds, err, ptr_dest, data_traffic_region);

	DATA_COPY(pds, err, &ptr_dest->location, &ptr_src->location, data_string);

	ptr_dest->start = ptr_src->start;
	ptr_dest->length = ptr_src->length;

	return err;
}
