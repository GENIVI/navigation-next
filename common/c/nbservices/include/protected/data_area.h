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
 * data_area.h: created 2007/11/09 by Michael Gilbert.
 */

#ifndef DATA_AREA_H
#define DATA_AREA_H

#include "datautil.h"
#include "data_polyline_pack.h"
#include "data_string.h"
#include "nbexp.h"

typedef enum area_type_ {

	area_none = 0,
	area_building,
	area_interior,
	area_water,
	area_campus,
	area_parks,
	area_geopolitical

} area_type;

typedef struct data_area_ {

	/* Child Elements */

	/* Attributes */
	data_polyline_pack	polyline_pack;
	uint32				priority;
	data_string			label;

	/* Type */
	area_type			type;

} data_area;

NB_Error	data_area_init(data_util_state* pds, data_area* pda);
void		data_area_free(data_util_state* pds, data_area* pda);

NB_Error	data_area_from_tps(data_util_state* pds, data_area* pda, tpselt te, uint32 polyline_pack_passes);
NB_Error	data_area_copy(data_util_state* pds, data_area* pda_dest, data_area* pda_src);

int			data_area_compare(const void* r1, const void* r2);

#endif
