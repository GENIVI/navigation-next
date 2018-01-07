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
 * data_route_polyline.c: created 2007/09/12 by Michael Gilbert.
 */

#include "data_route_polyline.h"

NB_Error
data_route_polyline_init(data_util_state* pds, data_route_polyline* prp)
{
	NB_Error err = NE_OK;

	err = err ? err : data_polyline_init(pds, &prp->polyline);

	return err;
}

void		
data_route_polyline_free(data_util_state* pds, data_route_polyline* prp)
{
	data_polyline_free(pds, &prp->polyline);
}

NB_Error	
data_route_polyline_from_tps(data_util_state* pds, data_route_polyline* prp, tpselt te)
{
	NB_Error err = NE_OK;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_route_polyline_free(pds, prp);

	err = data_route_polyline_init(pds, prp);

	if (err != NE_OK)
		return err;
	
	if (data_polyline_from_tps_attr(pds, &prp->polyline, te, "polyline", FALSE) != NE_OK)
		data_polyline_init(pds, &prp->polyline);

errexit:
	if (err != NE_OK)
		data_route_polyline_free(pds, prp);
	return err;
}

NB_Error	
data_route_polyline_copy(data_util_state* pds, data_route_polyline* prp_dest, data_route_polyline* prp_src)
{
	NB_Error err = NE_OK;

	data_route_polyline_free(pds, prp_dest);
	data_route_polyline_init(pds, prp_dest);

	err = err ? err : data_polyline_copy(pds, &prp_dest->polyline, &prp_src->polyline);

	return err;
}
