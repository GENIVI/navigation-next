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
* data_traffic_record_identifier.h: created 2007/11/26 by Mark Goddard.
*/

#ifndef DATA_TRAFFIC_RECORD_IDENTIFIER_H
#define DATA_TRAFFIC_RECORD_IDENTIFIER_H

#include "datautil.h"
#include "data_string.h"
#include "data_nav_progress.h"
#include "data_traffic_flow.h"
#include "data_traffic_incidents.h"
#include "data_place.h"
#include "nbexp.h"

typedef struct data_traffic_record_identifier_ {

	/* Child Elements */
	data_traffic_flow		new_realtime_traffic_flow;
	data_traffic_flow		old_realtime_traffic_flow;

	data_traffic_incidents	new_traffic_incidents;
	data_traffic_incidents	old_traffic_incidents;

	data_nav_progress	nav_progress;

	/* Attributes */
	data_string		value;
	boolean			reply_not_ready;

} data_traffic_record_identifier;

NB_Error	data_traffic_record_identifier_init(data_util_state* pds, data_traffic_record_identifier* ptri);
void		data_traffic_record_identifier_free(data_util_state* pds, data_traffic_record_identifier* ptri);

tpselt		data_traffic_record_identifier_to_tps(data_util_state* pds, data_traffic_record_identifier* ptri);
NB_Error	data_traffic_record_identifier_from_tps(data_util_state* pds, data_traffic_record_identifier* ptri, tpselt te);

NB_Error	data_traffic_record_identifier_copy(data_util_state* pds, data_traffic_record_identifier* ptri_dest, data_traffic_record_identifier* ptri_src);

#endif

