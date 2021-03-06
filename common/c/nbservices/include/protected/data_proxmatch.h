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
 * data_proxmatch.h: created 2004/12/09 by Mark Goddard.
 */

#ifndef DATA_PROXMATCH_H
#define DATA_PROXMATCH_H

#include "datautil.h"
#include "data_place.h"
#include "data_search_filter.h"
#include "data_poi_content.h"
#include "nbexp.h"
#include "data_transit_poi.h"
#include "data_data_availability.h"

typedef struct data_proxmatch_ {

	/* Child Elements */
	data_place			place;
    data_search_filter  search_filter;
    boolean             enhanced_poi;
    boolean             premium_placement;
    boolean             unmappable;
    boolean             has_poi_content;
    data_poi_content    poi_content;
    struct CSL_Vector*  vec_transit_poi;
    struct CSL_Vector*  vec_extapp_content;
    data_data_availability* data_availability;

	/* Attributes */
	double				distance;
	uint32              accuracy;

} data_proxmatch;

NB_DEC NB_Error	data_proxmatch_init(data_util_state* pds, data_proxmatch* ppm);
NB_DEC void		data_proxmatch_free(data_util_state* pds, data_proxmatch* ppm);

NB_DEC NB_Error	data_proxmatch_from_tps(data_util_state* pds, data_proxmatch* ppm, tpselt te);

NB_DEC boolean		data_proxmatch_equal(data_util_state* pds, data_proxmatch* ppm1, data_proxmatch* ppm2);
NB_DEC NB_Error	data_proxmatch_copy(data_util_state* pds, data_proxmatch* ppm_dest, data_proxmatch* ppm_src);

#endif
