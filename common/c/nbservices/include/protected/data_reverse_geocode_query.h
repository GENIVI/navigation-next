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
 * data_reverse_geocode_query.h: created 2004/12/21 by Mark Goddard.
 */

#ifndef DATA_REVERSE_GEOCODE_QUERY_H
#define DATA_REVERSE_GEOCODE_QUERY_H

#include "datautil.h"
#include "data_position.h"
#include "nbexp.h"

typedef struct data_reverse_geocode_query_ {

	/* Child Elements */
	data_position		position;

	/* Attributes */
	boolean				routeable;
	double				scale;
    boolean             avoidLocalRoad;

} data_reverse_geocode_query;

NB_Error	data_reverse_geocode_query_init(data_util_state* pds, data_reverse_geocode_query* prgq);
void		data_reverse_geocode_query_free(data_util_state* pds, data_reverse_geocode_query* prgq);

tpselt		data_reverse_geocode_query_to_tps(data_util_state* pds, data_reverse_geocode_query* prgq);

boolean		data_reverse_geocode_query_equal(data_util_state* pds, data_reverse_geocode_query* prgq1, data_reverse_geocode_query* prgq2);
NB_Error	data_reverse_geocode_query_copy(data_util_state* pds, data_reverse_geocode_query* prgq_dest, data_reverse_geocode_query* prgq_src);

#endif

