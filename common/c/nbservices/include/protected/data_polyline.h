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
/* (C) Copyright 2005 by Networks In Motion, Inc.                */
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
 * data_polyline.h: created 2005/01/06 by Mark Goddard.
 */

#ifndef DATA_POLYLINE_H
#define DATA_POLYLINE_H

#include "datautil.h"
#include "dynbuf.h"
#include "nbexp.h"

#define POLYLINE_LL_SCALE_FACTOR	186413.51
#define POLYLINE_LEN_SCALE_FACTOR	10.0
#define POLYLINE_INVALID_HEADING	-999.0

typedef struct {

	int32	lat;		/* latitude in fixed point format (multiplied by POLYLINE_LL_SCALE_FACTOR */
	int32	lon;		/* longitude in fixed point format (multiplied by POLYLINE_LL_SCALE_FACTOR */
	int32	len;		/* length of this segment in fixed point format (multiplied by POLYLINE_LEN_SCALE_FACTOR) */
	double	heading;

} pl_segment;

typedef struct data_polyline_ {
	
	pl_segment*		segments;
	uint32			numsegments;

	double			min_lat;
	double			min_lon;
	double			max_lat;
	double			max_lon;

	double			length;

} data_polyline;

typedef void (*data_polyline_enum_cb)(void * pUser, double p1_lat, double p1_lon, double p2_lat, double p2_lon);
typedef void (*data_polyline_enum_mercator_cb)(void * pUser, double p1_mx, double p1_my, double p2_mx, double p2_my);

NB_Error	data_polyline_init(data_util_state* pds, data_polyline* ppl);
void		data_polyline_free(data_util_state* pds, data_polyline* ppl);

NB_Error	data_polyline_from_tps_attr(data_util_state* pds, data_polyline* ppl, tpselt te, const char* name, boolean calcbounds);

NB_Error	data_polyline_copy(data_util_state* pds, data_polyline* ppl_dest, data_polyline* ppl_src);

uint32		data_polyline_len(data_util_state* pds, data_polyline* ppl);
double		data_polyline_dist(data_util_state* pds, data_polyline* ppl);
NB_Error	data_polyline_get(data_util_state* pds, data_polyline* ppl, uint32 index, double* plat, double* plon, double* plen, double* pheading);
NB_Error	data_polyline_enum(data_util_state* pds, data_polyline* ppl, data_polyline_enum_cb cb, void* cbuser);
NB_Error	data_polyline_enum_rev(data_util_state* pds, data_polyline* ppl, data_polyline_enum_cb cb, void* cbuser);

NB_Error	data_polyline_enum_mercator(data_util_state* pds, data_polyline* ppl, data_polyline_enum_cb cb, void* cbuser);

boolean		data_polyline_in_bounds(data_util_state* pds, data_polyline* ppl, double lat, double lon, double buffer);
boolean		data_polyline_in_rect(data_util_state* pds, data_polyline* ppl, 
					double lat1, double lon1, double lat2, double lon2, double buffer);

#endif

