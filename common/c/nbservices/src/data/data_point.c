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
 * data_point.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_point.h"

NB_Error	
data_point_init(data_util_state* pds, data_point* pdp)
{
	pdp->lat = -999.0;
	pdp->lon = -999.0;

	return NE_OK;
}

void	
data_point_free(data_util_state* pds, data_point* pdp)
{
	data_point_init(pds, pdp);
}

tpselt		
data_point_to_tps(data_util_state* pds, data_point* pdp)
{
	tpselt te;

	te = te_new("point");
	
	if (te == NULL)
		goto errexit;

	if (!te_setattrd(te, "lat", pdp->lat))
		goto errexit;

	if (!te_setattrd(te, "lon", pdp->lon))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);

	return NULL;
}

NB_Error	
data_point_from_tps(data_util_state* pds, data_point* pdp, tpselt te)
{

	if (!te_getattrd(te, "lat", &pdp->lat) || !te_getattrd(te, "lon", &pdp->lon))
		return NE_INVAL;

	return NE_OK;
}

boolean		
data_point_equal(data_util_state* pds, data_point* pdp1, data_point* pdp2)
{
	return (boolean)(pdp1->lat == pdp2->lat && pdp1->lon == pdp2->lon);
}

NB_Error	
data_point_copy(data_util_state* pds, data_point* pdp_dest, data_point* pdp_src)
{

	data_point_free(pds, pdp_dest);
	data_point_init(pds, pdp_dest);

	pdp_dest->lat = pdp_src->lat;
	pdp_dest->lon = pdp_src->lon;

	return NE_OK;
}

NB_Error
data_point_from_latlon(data_util_state* pds, data_point* pdp, double lat, double lon)
{
	pdp->lat = lat;
	pdp->lon = lon;

	return NE_OK;
}

void
data_point_to_buf(data_util_state* pds, data_point* pdp, struct dynbuf *dbp)
{
    if (pdp != NULL)
    {
        dbufcat(dbp, (const byte*) &pdp->lat, sizeof(pdp->lat));
        dbufcat(dbp, (const byte*) &pdp->lon, sizeof(pdp->lon));
    }
}

NB_Error
data_point_from_binary(data_util_state* pds, data_point* pdp, byte** ppdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_double_from_binary(pds, &pdp->lat, ppdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pdp->lon, ppdata, pdatalen);

    return err;
}

uint32
data_point_get_tps_size(data_util_state* pds, data_point* pdp)
{
    uint32 size = 0;

    if (pdp)
    {
        size += sizeof(pdp->lat);
        size += sizeof(pdp->lon);
    }
    
    return size;
}
