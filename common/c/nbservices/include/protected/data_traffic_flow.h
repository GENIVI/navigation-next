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
* data_traffic_flow.h: created 2007/11/26 by Mark Goddard.
*/

#ifndef DATA_TRAFFIC_FLOW_H
#define DATA_TRAFFIC_FLOW_H

#include "datautil.h"
#include "vec.h"
#include "data_traffic_flow_item.h"
#include "nbexp.h"

typedef struct data_traffic_flow_ {

	/* Child Elements */
	struct CSL_Vector*			vec_traffic_flow_items;

	/* Attributes */
	uint32				age;
	uint32				type;

} data_traffic_flow;


typedef enum {

	TrafficSpeedNone = 0,
	TrafficSpeedHistoric = 1 << 0,
	TrafficSpeedRealTime = 1 << 1,
	TrafficSpeedStandard = 1 << 2,

	TrafficSpeedAll		 = TrafficSpeedHistoric | TrafficSpeedRealTime | TrafficSpeedStandard

} TrafficSpeedType;

typedef enum {

	TrafficAgeOld = 0,
	TrafficAgeNew = 1

} TrafficAgeType;

NB_Error	data_traffic_flow_init(data_util_state* pds, data_traffic_flow* ptf);
void		data_traffic_flow_free(data_util_state* pds, data_traffic_flow* ptf);

NB_Error	data_traffic_flow_from_tps(data_util_state* pds, data_traffic_flow* ptf, tpselt te);

NB_Error	data_traffic_flow_copy(data_util_state* pds, data_traffic_flow* ptf_dest, 
								   data_traffic_flow* ptf_src);

NB_Error	data_traffic_flow_get_speed(data_util_state* pds, data_traffic_flow* ptf, double minspeed,
										const char* tmcloc,	double* pspeed, char* pcolor, 
										double* pfreeflowspeed);

#endif
