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
 * data_road.h: created 2005/01/02 by Mark Goddard.
 */

#ifndef DATA_ROAD_H
#define DATA_ROAD_H

#include "datautil.h"
#include "vec.h"
#include "data_polyline.h"
#include "data_polyline_pack.h"
#include "data_string.h"
#include "data_route_number_info.h"
#include "nbexp.h"

typedef enum road_type_ {

	road_none = 0,
	road_limited_access,
	road_arterial,
	road_local,
	road_terminal,
	road_rotary,
	road_ramp,
	road_bridge,
	road_tunnel,
	road_skyway,
	road_ferry

} road_type;

typedef struct data_road_ {

	/* Child Elements */
	struct CSL_Vector*			vec_polylines;
	struct CSL_Vector*			vec_route_number_info;
	struct CSL_Vector*			vec_polyline_pack;

	/* Attributes */
	uint32			priority;
	data_string		label;
	uint32			lanes;

	/* Type */
	road_type		type;

	/* User Data stored here for convenience */
	struct CSL_Vector*			vec_tristrips;
	uint32				polyline_pack_passes;

} data_road;

NB_Error	data_road_init(data_util_state* pds, data_road* prd);
void		data_road_free(data_util_state* pds, data_road* prd);

NB_Error	data_road_from_tps(data_util_state* pds, data_road* prd, tpselt te, uint32 polyline_pack_passes);
NB_Error	data_road_copy(data_util_state* pds, data_road* prd_dest, data_road* prd_src);

int			data_road_compare(const void* r1, const void* r2);
void		data_road_free_polyline_pack(data_util_state* pds, data_road* prd);
#endif
