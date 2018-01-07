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
* data_traffic_incident.h: created 2007/11/27 by Huey Ly
*/

#ifndef DATA_TRAFFIC_INCIDENT_H
#define DATA_TRAFFIC_INCIDENT_H

#include "datautil.h"
#include "data_string.h"
#include "nbexp.h"

typedef struct data_traffic_incident_ {

	/* Attributes */
	uint32			type;
	uint32			criticality;
	uint32			entry_time;
	uint32			start_time;
	uint32			end_time;
	int32           utc_offset;
	data_string		road;
	data_string		description;

	/* Extra Calculated Data */
	double			distance_from_start;
	uint32			nman_closest;
	uint32			nseg_closest;
	boolean			new_item;

} data_traffic_incident;

NB_Error	data_traffic_incident_init(data_util_state* pds, data_traffic_incident* pp);
void		data_traffic_incident_free(data_util_state* pds, data_traffic_incident* pp);

NB_Error	data_traffic_incident_from_tps(data_util_state* pds, data_traffic_incident* pp, tpselt te);

boolean		data_traffic_incident_equal(data_util_state* pds, data_traffic_incident* pp1, data_traffic_incident* pp2);
NB_Error	data_traffic_incident_copy(data_util_state* pds, data_traffic_incident* pp_dest, data_traffic_incident* pp_src);

void        data_traffic_incident_to_buf(data_util_state* pds, data_traffic_incident* pp, struct dynbuf *pdb);
NB_Error    data_traffic_incident_from_binary(data_util_state* pds, data_traffic_incident* pp, byte** ppdata, size_t* pdatalen);
uint32      data_traffic_incident_get_tps_size(data_util_state* pds, data_traffic_incident* pp);

#endif

