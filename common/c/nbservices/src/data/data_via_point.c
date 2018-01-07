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

#include "data_via_point.h"
#if 0
#include "nblocation.h"
#endif

NB_DEF NB_Error
data_via_point_init(data_util_state* pds, data_via_point* plm)
{
	NB_Error err = NE_OK;

	err = data_address_init(pds, &plm->address);
	err = err ? err : data_point_init(pds, &plm->point);
    plm->passed = FALSE;
    plm->isStopPoint = FALSE;

	return err;
}

NB_DEF void
data_via_point_free(data_util_state* pds, data_via_point* plm)
{
	data_address_free(pds, &plm->address);
	data_point_free(pds, &plm->point);
}

NB_DEF tpselt
data_via_point_to_tps(data_util_state* pds, data_via_point* point)
{
	tpselt te;
	tpselt tchild;

    te = te_new("waypoint");
    if(te == NULL)
    {
        return NULL;
    }
    tchild = data_point_to_tps(pds, &point->point);
    if(tchild)
    {
        te_attach(te, tchild);
    }
    tchild = data_address_to_tps(pds, &point->address);
    if(tchild)
    {
        te_attach(te, tchild);
    }
	return te;

errexit:

	te_dealloc(te);
	return NULL;
}

NB_DEF NB_Error
data_via_point_from_tps(data_util_state* pds, data_via_point* point, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_via_point_free(pds, point);

	err = data_via_point_init(pds, point);

	if (err != NE_OK)
		return err;

	ce = te_getchild(te, "address");

	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	err = err ? err : data_address_from_tps(pds, &point->address, ce);

	ce = te_getchild(te, "point");

	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	err = err ? err : data_point_from_tps(pds, &point->point, ce);

errexit:

    if (err != NE_OK) {

        DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_via_point (error_code: %d)", err));
		data_via_point_free(pds, point);
    }

	return err;
}

NB_DEF NB_Error
data_via_point_from_nimlocation(data_util_state* pds, data_via_point* pl, const NB_Location* pLocation)
{
	NB_Error err = NE_OK;

	data_via_point_free(pds, pl);

	err = data_via_point_init(pds, pl);

	err = err ? err : data_address_from_nimlocation(pds, &pl->address, pLocation);
	err = err ? err : data_point_from_latlon(pds, &pl->point, pLocation->latitude, pLocation->longitude);

	if (err != NE_OK)
		data_via_point_free(pds, pl);

	return err;
}

NB_DEF NB_Error
data_via_point_to_nimlocation(data_util_state* state, data_via_point* dataLocation, NB_Location* nbLocation)
{
    if ((!state) || (!dataLocation) || (!nbLocation))
    {
        return NE_INVAL;
    }

    nbLocation->latitude = dataLocation->point.lat;
    nbLocation->longitude = dataLocation->point.lon;

    return data_address_to_nimlocation(state, (&dataLocation->address), nbLocation);
}

NB_DEF NB_Error
data_via_point_copy(data_util_state* pds, data_via_point* via_dest, data_via_point* via_src)
{
    NB_Error err = NE_OK;
    
    data_via_point_free(pds, via_dest);
    data_via_point_init(pds, via_dest);
    
    err = data_address_copy(pds, &via_dest->address, &via_src->address);
    err = err ? err : data_point_copy(pds, &via_dest->point, &via_src->point);
    via_dest->passed = via_src->passed;
    via_dest->isStopPoint = via_src->isStopPoint;
    
    return err;
}

