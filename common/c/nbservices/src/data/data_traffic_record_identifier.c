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
 * data_traffic_record_identifier.c: created 2007/11/26 by Mark Goddard.
 */

#include "data_traffic_record_identifier.h"
#include "vec.h"

NB_Error
data_traffic_record_identifier_init(data_util_state* pds, data_traffic_record_identifier* ptri)
{
	NB_Error err = NE_OK;

	DATA_MEM_ZERO(ptri, data_traffic_record_identifier);

	DATA_INIT(pds, err, &ptri->value, data_string);
	DATA_INIT(pds, err, &ptri->new_realtime_traffic_flow, data_traffic_flow);
	DATA_INIT(pds, err, &ptri->old_realtime_traffic_flow, data_traffic_flow);
	DATA_INIT(pds, err, &ptri->new_traffic_incidents, data_traffic_incidents);
	DATA_INIT(pds, err, &ptri->old_traffic_incidents, data_traffic_incidents);
	DATA_INIT(pds, err, &ptri->nav_progress, data_nav_progress);

	return err;
}

void		
data_traffic_record_identifier_free(data_util_state* pds, data_traffic_record_identifier* ptri)
{
	DATA_FREE(pds, &ptri->value, data_string);
	DATA_FREE(pds, &ptri->new_realtime_traffic_flow, data_traffic_flow);
	DATA_FREE(pds, &ptri->old_realtime_traffic_flow, data_traffic_flow);
	DATA_FREE(pds, &ptri->new_traffic_incidents, data_traffic_incidents);
	DATA_FREE(pds, &ptri->old_traffic_incidents, data_traffic_incidents);
	DATA_FREE(pds, &ptri->nav_progress, data_nav_progress);
}

tpselt
data_traffic_record_identifier_to_tps(data_util_state* pds, data_traffic_record_identifier* ptri)
{
	tpselt te = NULL;
	tpselt ce = NULL;

	te = te_new("traffic-record-identifier");

	if (te == NULL)
		goto errexit;

	if (!te_setattrc(te, "value", data_string_get(pds, &ptri->value)))
		goto errexit;

	if ((ce = data_nav_progress_to_tps(pds, &ptri->nav_progress)) != NULL && te_attach(te, ce))
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
data_traffic_record_identifier_from_tps(data_util_state* pds, data_traffic_record_identifier* ptri, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;
	int iter;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_traffic_record_identifier_free(pds, ptri);

	err = data_traffic_record_identifier_init(pds, ptri);

	if (err != NE_OK)
		return err;
	
	err = data_string_from_tps_attr(pds, &ptri->value, te, "value");

	if (err != NE_OK)
		return err;

	iter = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "traffic-flow") == 0) {

			uint32 type = te_getattru(ce, "type");
			uint32 age  = te_getattru(ce, "age");

			if (age == 0 && type == 1)
				err = err ? err : data_traffic_flow_from_tps(pds, &ptri->old_realtime_traffic_flow, ce);
			else if (age == 1 && type == 1)
				err = err ? err : data_traffic_flow_from_tps(pds, &ptri->new_realtime_traffic_flow, ce);
		}
		else if (nsl_strcmp(te_getname(ce), "traffic-incidents") == 0) {

			uint32 age  = te_getattru(ce, "age");

			if (age == 0)
				err = err ? err : data_traffic_incidents_from_tps(pds, &ptri->old_traffic_incidents, ce);
			else if (age == 1)
				err = err ? err : data_traffic_incidents_from_tps(pds, &ptri->new_traffic_incidents, ce);
		}
	}

errexit:
	if (err != NE_OK)
		data_traffic_record_identifier_free(pds, ptri);
	return err;
}

NB_Error	
data_traffic_record_identifier_copy(data_util_state* pds, data_traffic_record_identifier* ptri_dest, data_traffic_record_identifier* ptri_src)
{
	NB_Error err = NE_OK;

	DATA_REINIT(pds, err, ptri_dest, data_traffic_record_identifier);

	DATA_COPY(pds, err, &ptri_dest->value, &ptri_src->value, data_string);

	DATA_COPY(pds, err, &ptri_dest->new_realtime_traffic_flow, &ptri_src->new_realtime_traffic_flow, data_traffic_flow);
	DATA_COPY(pds, err, &ptri_dest->old_realtime_traffic_flow, &ptri_src->old_realtime_traffic_flow, data_traffic_flow);
	DATA_COPY(pds, err, &ptri_dest->new_traffic_incidents, &ptri_src->new_traffic_incidents, data_traffic_incidents);
	DATA_COPY(pds, err, &ptri_dest->old_traffic_incidents, &ptri_src->old_traffic_incidents, data_traffic_incidents);

	DATA_COPY(pds, err, &ptri_dest->nav_progress, &ptri_src->nav_progress, data_nav_progress);

	return err;
}

