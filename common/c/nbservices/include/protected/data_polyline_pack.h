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
 * data_polyline_pack.h: created 2007/10/09 by Michael Gilbert.
 */

#ifndef DATA_POLYLINE_PACK_H
#define DATA_POLYLINE_PACK_H

#include "datautil.h"
#include "dynbuf.h"
#include "nbexp.h"

typedef struct {

	double x;
	double y;

} plp_segment;

typedef struct data_polyline_pack_ {
	
	plp_segment*	segments;
	uint32			numsegments;
	uint32			numpasses;

} data_polyline_pack;

typedef void (*data_polyline_pack_enum_cb)(void * pUser, double x1, double y1, double x2, double y2);

NB_Error	data_polyline_pack_init(data_util_state* pds, data_polyline_pack* ppp);
void		data_polyline_pack_free(data_util_state* pds, data_polyline_pack* ppp);

NB_Error	data_polyline_pack_from_tps_attr(data_util_state* pds, data_polyline_pack* ppp, tpselt te, const char* name, uint32 passes);

NB_Error	data_polyline_pack_copy(data_util_state* pds, data_polyline_pack* ppl_dest, data_polyline_pack* ppl_src);

uint32		data_polyline_pack_len(data_util_state* pds, data_polyline_pack* ppp);
NB_Error	data_polyline_pack_enum(data_util_state* pds, data_polyline_pack* ppp, data_polyline_pack_enum_cb cb, void* cbuser);
NB_Error	data_polyline_pack_enum_rev(data_util_state* pds, data_polyline_pack* ppp, data_polyline_pack_enum_cb cb, void* cbuser);

#endif

