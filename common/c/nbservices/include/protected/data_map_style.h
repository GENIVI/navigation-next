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

/*-
 * data_map_style.h: created 2004/12/09 by Mark Goddard.
 */

#ifndef DATA_MAP_STYLE_
#define DATA_MAP_STYLE_

#include "datautil.h"
#include "data_blob.h"
#include "data_string.h"
#include "nbexp.h"

#define LEGEND_NONE		"none"
#define LEGEND_IMPERIAL	"imperial"
#define LEGEND_METRIC	"metric"
#define LEGEND_BOTH		"both"

typedef struct data_map_style_ {
	/* Attributes */
	data_string			scheme;
	data_string			legend;
	data_blob			route_id;
	data_blob			alt_route_id;
	uint32				maneuver;
	boolean				maneuver_valid;
	uint32	traffic;

} data_map_style;

NB_Error	data_map_style_init(data_util_state* pds, data_map_style* pms);
void		data_map_style_free(data_util_state* pds, data_map_style* pms);

tpselt		data_map_style_to_tps(data_util_state* pds, data_map_style* pms);

boolean		data_map_style_equal(data_util_state* pds, data_map_style* pms1, data_map_style* pms2);
NB_Error	data_map_style_copy(data_util_state* pds, data_map_style* pms_dest, data_map_style* pms_src);

#endif

