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
 * data_pLocation->c: created 2004/12/09 by Mark Goddard.
 */

#include "data_location.h"
#if 0
#include "nblocation.h"
#endif

NB_DEF NB_Error
data_location_init(data_util_state* pds, data_location* plm)
{
	NB_Error err = NE_OK;

	err = data_address_init(pds, &plm->address);
	err = err ? err : data_point_init(pds, &plm->point);
	err = err ? err : data_string_init(pds, &plm->name);
    plm->box = NULL;

	return err;
}

NB_DEF void
data_location_free(data_util_state* pds, data_location* plm)
{
	data_address_free(pds, &plm->address);
	data_point_free(pds, &plm->point);
	data_string_free(pds, &plm->name);

    if ( plm->box != NULL )
    {
        data_box_free(pds, plm->box);
        nsl_free(plm->box);
        plm->box = NULL;
    }
}

NB_DEF tpselt
data_location_to_tps(data_util_state* pds, data_location* plm)
{
	tpselt te;
	tpselt ce;

	te = te_new("location");

	if (te == NULL)
		goto errexit;

	if ((ce = data_address_to_tps(pds, &plm->address)) != NULL && te_attach(te, ce))
		ce = NULL;
	else
		goto errexit;

	if ((ce = data_point_to_tps(pds, &plm->point)) != NULL && te_attach(te, ce))
		ce = NULL;
	else
		goto errexit;

	if (!te_setattrc(te, "name", data_string_get(pds, &plm->name)))
		goto errexit;

    if ( plm->box != NULL )
    {
        if ( ((ce = data_box_to_tps(pds, plm->box)) != NULL) && te_attach(te, ce) )
            ce = NULL;
        else
            goto errexit;
    }

	return te;

errexit:

	te_dealloc(te);
	return NULL;
}

NB_DEF NB_Error
data_location_from_tps(data_util_state* pds, data_location* plm, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_location_free(pds, plm);

	err = data_location_init(pds, plm);

	if (err != NE_OK)
		return err;

	ce = te_getchild(te, "address");

	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	err = err ? err : data_address_from_tps(pds, &plm->address, ce);

	ce = te_getchild(te, "point");

	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	err = err ? err : data_point_from_tps(pds, &plm->point, ce);

    ce = te_getchild(te, "box");
    if ( ce != NULL )
    {
        plm->box = nsl_malloc(sizeof(data_box));
        if ( plm->box == NULL )
        {
            err = NE_INVAL;
            goto errexit;
        }

        err = err ? err : data_box_init(pds, plm->box);
        err = err ? err : data_box_from_tps(pds, plm->box, ce);
    }

    err = err ? err : data_string_from_tps_attr(pds, &plm->name, te, "name");

errexit:

    if (err != NE_OK) {

        DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_location (error_code: %d)", err));
		data_location_free(pds, plm);
    }

	return err;
}

NB_DEF boolean
data_location_equal(data_util_state* pds, data_location* plm1, data_location* plm2)
{
    boolean ret = data_address_equal(pds, &plm1->address, &plm2->address) &&
                data_point_equal(pds, &plm1->point, &plm2->point) &&
                data_string_equal(pds, &plm1->name, &plm2->name);

    if ( (plm1 == NULL) && (plm2 == NULL) )
        return ret;
    else if ( (plm1 != NULL) && (plm2 != NULL) )
        return (ret && data_box_equal(pds, plm1->box, plm2->box));
    else
        return FALSE;
}

NB_DEF NB_Error
data_location_copy(data_util_state* pds, data_location* plm_dest, data_location* plm_src)
{
	NB_Error err = NE_OK;

	data_location_free(pds, plm_dest);
	data_location_init(pds, plm_dest);

	err = data_address_copy(pds, &plm_dest->address, &plm_src->address);
	err = err ? err : data_point_copy(pds, &plm_dest->point, &plm_src->point);
	err = err ? err : data_string_copy(pds, &plm_dest->name, &plm_src->name);

    if ( plm_src->box != NULL )
    {
        if ( plm_dest->box == NULL )
        {
            plm_dest->box = nsl_malloc(sizeof(data_box));
            if ( plm_dest->box == NULL )
            {
                err = NE_INVAL;
            }
        }

        err = err ? err : data_box_copy(pds, plm_dest->box, plm_src->box);
    }

	return err;
}

NB_DEF NB_Error
data_location_from_nimlocation(data_util_state* pds, data_location* pl, const NB_Location* pLocation)
{
	NB_Error err = NE_OK;

	data_location_free(pds, pl);

	err = data_location_init(pds, pl);

	err = err ? err : data_string_set(pds, &pl->name, pLocation->areaname);
	err = err ? err : data_address_from_nimlocation(pds, &pl->address, pLocation);
	err = err ? err : data_point_from_latlon(pds, &pl->point, pLocation->latitude, pLocation->longitude);

	if (err != NE_OK)
		data_location_free(pds, pl);

	return err;
}

NB_Error
data_location_to_nimlocation(data_util_state* state, data_location* dataLocation, NB_Location* nbLocation)
{
    if ((!state) || (!dataLocation) || (!nbLocation))
    {
        return NE_INVAL;
    }

    nbLocation->latitude = dataLocation->point.lat;
    nbLocation->longitude = dataLocation->point.lon;
    data_string_get_copy(state, &(dataLocation->name), nbLocation->areaname, sizeof(nbLocation->areaname));

    return data_address_to_nimlocation(state, (&dataLocation->address), nbLocation);
}

NB_DEF uint32
data_location_get_tps_size(data_util_state* pds, data_location* pl)
{
    uint32 size = 0;

    size += data_address_get_tps_size(pds, &pl->address);
    size += data_point_get_tps_size(pds, &pl->point);
    size += data_string_get_tps_size(pds, &pl->name);

    if ( pl->box != NULL )
        size += data_box_get_tps_size(pds, pl->box);

    return size;
}

NB_DEF void
data_location_to_buf(data_util_state* pds, data_location* pl, struct dynbuf* pdb)
{
    data_address_to_buf(pds, &pl->address, pdb);
    data_point_to_buf(pds, &pl->point, pdb);
    data_string_to_buf(pds, &pl->name, pdb);
    data_box_to_buf(pds, pl->box, pdb);
}

NB_DEF NB_Error
data_location_from_binary(data_util_state* pds, data_location* pl, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_address_from_binary(pds, &pl->address, pdata, pdatalen);
    err = err ? err : data_point_from_binary(pds, &pl->point, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pl->name, pdata, pdatalen);

    if ( pl->box != NULL )
        err = err ? err : data_box_from_binary(pds, pl->box, pdata, pdatalen);

    return err;
}

