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
* data_traffic_incidents.c: created 2007/11/27 by Mark Goddard.
*/

#include "data_traffic_incidents.h"
#include "vec.h"

NB_Error	
data_traffic_incidents_init(data_util_state* pds, data_traffic_incidents* pti)
{
	NB_Error err = NE_OK;

	DATA_MEM_ZERO(pti, data_traffic_incidents);

	DATA_VEC_ALLOC(err, pti->vec_place, data_place);

	return err;
}

void
data_traffic_incidents_free(data_util_state* pds, data_traffic_incidents* pti)
{
	DATA_VEC_FREE(pds, pti->vec_place, data_place);
}

NB_Error	
data_traffic_incidents_from_tps(data_util_state* pds, data_traffic_incidents* pti, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;
	int iter;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	DATA_REINIT(pds, err, pti, data_traffic_incidents);

	if (err != NE_OK)
		goto errexit;

	pti->age	=	te_getattru(te, "age");

	iter = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "place") == 0) {

			DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pti->vec_place, data_place);
		}

		if (err != NE_OK)
			goto errexit;
	}

errexit:

	if (err != NE_OK) {

		DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_traffic_incidents failed (error_code: %d)", err));
		DATA_FREE(pds, pti, data_traffic_incidents);
	}

	return err;
}

NB_Error
data_traffic_incidents_copy(data_util_state* pds, data_traffic_incidents* pti_dest, data_traffic_incidents* pti_src)
{
	NB_Error err = NE_OK;

	DATA_REINIT(pds, err, pti_dest, data_traffic_incidents);

	DATA_VEC_COPY(pds, err, pti_dest->vec_place, pti_src->vec_place, data_place);

	pti_dest->age = pti_src->age;

	return err;
}
