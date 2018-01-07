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
 * data_road.c: created 2005/01/02 by Mark Goddard.
 */

#include "data_road.h"
#include "nbspatial.h"
#include "spatialvector.h"

NB_Error
data_road_init(data_util_state* pds, data_road* prd)
{
	prd->vec_polylines = NULL;

	prd->vec_route_number_info = NULL;

	prd->vec_polyline_pack = NULL;

	prd->vec_tristrips = NULL;

	prd->priority = 0;
	prd->lanes = 0;

	data_string_init(pds, &prd->label);

	return NE_OK;
}

void
data_road_free(data_util_state* pds, data_road* prd)
{
	int n,l;
	struct tristrip* pts;

	if (prd->vec_polylines) {
		l = CSL_VectorGetLength(prd->vec_polylines);

		for (n=0;n<l;n++)
			data_polyline_free(pds, (data_polyline*) CSL_VectorGetPointer(prd->vec_polylines, n));

		VEC_DEALLOC_FSA(&pds->vec_heap, prd->vec_polylines);
	}

	if (prd->vec_polyline_pack) {
		l = CSL_VectorGetLength(prd->vec_polyline_pack);

		for (n=0;n<l;n++)
			data_polyline_pack_free(pds, (data_polyline_pack*) CSL_VectorGetPointer(prd->vec_polyline_pack, n));

		VEC_DEALLOC_FSA(&pds->vec_heap, prd->vec_polyline_pack);
	}

	if (prd->vec_route_number_info) {
		l = CSL_VectorGetLength(prd->vec_route_number_info);

		for (n=0;n<l;n++)
			data_route_number_info_free(pds, (data_route_number_info*) CSL_VectorGetPointer(prd->vec_route_number_info, n));

		VEC_DEALLOC_FSA(&pds->vec_heap, prd->vec_route_number_info);
	}

	prd->vec_route_number_info = NULL;

	data_string_free(pds, &prd->label);

	if (prd->vec_tristrips)
	{
		l = CSL_VectorGetLength(prd->vec_tristrips);
		for (n=0;n<l;n++) {

			pts = CSL_VectorGetPointer(prd->vec_tristrips, n);

			nsl_free(pts->points);
		}

		VEC_DEALLOC_FSA(&pds->vec_heap, prd->vec_tristrips);
		prd->vec_tristrips = NULL;
	}
}

NB_Error
data_road_from_tps(data_util_state* pds, data_road* prd, tpselt te, uint32 polyline_pack_passes)
{
	NB_Error err = NE_OK;
	tpselt	ce;
	int iter;
	int num_polyline;
	int num_polyline_pack;
	int num_route_number_info;
	data_polyline polyline;
	data_polyline_pack polyline_pack;
	data_route_number_info route_number_info;
	const char* name;
	
	err = data_polyline_init(pds, &polyline);
	err = data_polyline_pack_init(pds, &polyline_pack);
	err = data_route_number_info_init(pds, &route_number_info);

	if (err != NE_OK)
		goto errexit;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_road_free(pds, prd);

	err = data_road_init(pds, prd);

	if (err != NE_OK)
		return err;

	name = te_getname(te);

	if (nsl_strcmp(name, "road-limited-access") == 0)
		prd->type = road_limited_access;
	else if (nsl_strcmp(name, "road-arterial") == 0)
		prd->type = road_arterial;
	else if (nsl_strcmp(name, "road-local") == 0)
		prd->type = road_local;
	else if (nsl_strcmp(name, "road-terminal") == 0)
		prd->type = road_terminal;
	else if (nsl_strcmp(name, "road-rotary") == 0)
		prd->type = road_rotary;
	else if (nsl_strcmp(name, "road-ramp") == 0)
		prd->type = road_ramp;
	else if (nsl_strcmp(name, "road-bridge") == 0)
		prd->type = road_bridge;
	else if (nsl_strcmp(name, "road-tunnel") == 0)
		prd->type = road_tunnel;
	else if (nsl_strcmp(name, "road-skyway") == 0)
		prd->type = road_skyway;
	else if (nsl_strcmp(name, "road-ferry") == 0)
		prd->type = road_ferry;
	else
		prd->type = road_none;

	iter = 0;
	num_polyline = 0;
	num_polyline_pack = 0;
	num_route_number_info = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "polyline") == 0)
			num_polyline++;
		if (nsl_strcmp(te_getname(ce), "polyline-pack") == 0)
			num_polyline_pack++;
		if (nsl_strcmp(te_getname(ce), "route-number-info") == 0)
			num_route_number_info++;
	}

	if (num_polyline > 0)
	{
		VEC_ALLOC_FSA(&pds->vec_heap, sizeof(data_polyline), prd->vec_polylines);

		if (prd->vec_polylines == NULL) {

			err = NE_NOMEM;
			goto errexit;
		}

		if (!CSL_VectorSetAlloc(prd->vec_polylines, num_polyline)) {
			err = NE_NOMEM;
			goto errexit;
		}
	}

	if (num_polyline_pack > 0)
	{
		VEC_ALLOC_FSA(&pds->vec_heap, sizeof(data_polyline_pack), prd->vec_polyline_pack);

		if (prd->vec_polyline_pack == NULL) {

			err = NE_NOMEM;
			goto errexit;
		}

		if (!CSL_VectorSetAlloc(prd->vec_polyline_pack, num_polyline_pack)) {
			err = NE_NOMEM;
			goto errexit;
		}
	}

	if (num_route_number_info > 0)
	{
		VEC_ALLOC_FSA(&pds->vec_heap, sizeof(data_route_number_info), prd->vec_route_number_info);

		if (prd->vec_route_number_info == NULL) {

			err = NE_NOMEM;
			goto errexit;
		}

		if (!CSL_VectorSetAlloc(prd->vec_route_number_info, num_route_number_info)) {
			err = NE_NOMEM;
			goto errexit;
		}
	}

	iter = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "polyline") == 0) {

			err = data_polyline_from_tps_attr(pds, &polyline, ce, "value", FALSE);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(prd->vec_polylines, &polyline))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&polyline, 0, sizeof(polyline)); // clear out the polyline since we have copied it (shallow copy)
				else
					data_polyline_free(pds, &polyline); // free the polyline if it was not copied
			}

			if (err != NE_OK)
				goto errexit;
		}

		if (nsl_strcmp(te_getname(ce), "polyline-pack") == 0) {

			err = data_polyline_pack_from_tps_attr(pds, &polyline_pack, ce, "value", polyline_pack_passes);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(prd->vec_polyline_pack, &polyline_pack))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&polyline_pack, 0, sizeof(polyline_pack)); // clear out the polyline since we have copied it (shallow copy)
				else
					data_polyline_pack_free(pds, &polyline_pack); // free the polyline if it was not copied
			}

			if (err != NE_OK)
				goto errexit;
		}

		if (nsl_strcmp(te_getname(ce), "route-number-info") == 0) {

			err = data_route_number_info_from_tps(pds, &route_number_info, ce);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(prd->vec_route_number_info, &route_number_info))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&route_number_info, 0, sizeof(route_number_info)); // clear out the item since we have copied it (shallow copy)
				else
					data_route_number_info_free(pds, &route_number_info); // free the item if it was not copied
			}

			if (err != NE_OK)
				goto errexit;
		}
	}

	prd->priority = te_getattru(te, "priority");
	prd->lanes = te_getattru(te, "lanes");

	err = err ? err : data_string_from_tps_attr(pds, &prd->label, te, "label"); // Okay if it does not exist

	if (err == NE_INVAL)
		err = NE_OK;
	
errexit:
	data_polyline_free(pds, &polyline);
	data_polyline_pack_free(pds, &polyline_pack);
	data_route_number_info_free(pds, &route_number_info);

	if (err != NE_OK)
		data_road_free(pds, prd);
	return err;
}

NB_Error
data_road_copy(data_util_state* pds, data_road* prd_dest, data_road* prd_src)
{
    NB_Error err = NE_OK;
    int n,l;
    data_polyline_pack polyline_pack;
    data_route_number_info route_number_info;

    data_route_number_info_init(pds, &route_number_info);
    data_polyline_pack_init(pds, &polyline_pack);

    data_road_free(pds, prd_dest);

    err = data_road_init(pds, prd_dest);

    if (err == NE_OK && prd_src->vec_polylines)
    {
        l = CSL_VectorGetLength(prd_src->vec_polylines);

        VEC_ALLOC_FSA(&pds->vec_heap, sizeof(data_polyline), prd_dest->vec_polylines);

        if (prd_dest->vec_polylines == NULL)
        {
            return NE_NOMEM;
        }

        if (!CSL_VectorSetAlloc(prd_dest->vec_polylines, l))
        {
            return NE_NOMEM;
        }

        DATA_VEC_COPY(pds, err, prd_dest->vec_polylines, prd_src->vec_polylines, data_polyline);
    }

    if (err == NE_OK && prd_src->vec_polyline_pack)
    {
        l = CSL_VectorGetLength(prd_src->vec_polyline_pack);

        VEC_ALLOC_FSA(&pds->vec_heap, sizeof(data_polyline_pack), prd_dest->vec_polyline_pack);

        if (prd_dest->vec_polyline_pack == NULL)
        {
            return NE_NOMEM;
        }

        if (!CSL_VectorSetAlloc(prd_dest->vec_polyline_pack, l))
        {
            return NE_NOMEM;
        }

        for (n=0;n<l && err == NE_OK;n++)
        {
            err = err ? err : data_polyline_pack_copy(pds, &polyline_pack, CSL_VectorGetPointer(prd_src->vec_polyline_pack, n));
            err = err ? err : CSL_VectorAppend(prd_dest->vec_polyline_pack, &polyline_pack) ? NE_OK : NE_NOMEM;

            if (err)
                data_polyline_pack_free(pds, &polyline_pack);
            else
                data_polyline_pack_init(pds, &polyline_pack);
        }
    }

    if (err == NE_OK && prd_src->vec_route_number_info)
    {
        l = CSL_VectorGetLength(prd_src->vec_route_number_info);

        VEC_ALLOC_FSA(&pds->vec_heap, sizeof(data_route_number_info), prd_dest->vec_route_number_info);

        if (prd_dest->vec_route_number_info == NULL)
        {
            return NE_NOMEM;
        }

        if (!CSL_VectorSetAlloc(prd_dest->vec_route_number_info, l))
        {
            return NE_NOMEM;
        }

        for (n=0;n<l && err == NE_OK;n++)
        {
            err = err ? err : data_route_number_info_copy(pds, &route_number_info, CSL_VectorGetPointer(prd_src->vec_route_number_info, n));
            err = err ? err : CSL_VectorAppend(prd_dest->vec_route_number_info, &route_number_info) ? NE_OK : NE_NOMEM;

            if (err)
                data_route_number_info_free(pds, &route_number_info);
            else
                data_route_number_info_init(pds, &route_number_info);
        }
    }

    prd_dest->priority = prd_src->priority;

    prd_dest->type = prd_src->type;

    prd_dest->lanes = prd_src->lanes;

    err = err ? err : data_string_copy(pds, &prd_dest->label, &prd_src->label);

    // free scratch route number info
    data_route_number_info_free(pds, &route_number_info);

    return err;
}

int
data_road_compare(const void* r1, const void* r2)
{
	const data_road* rd1 = r1;
	const data_road* rd2 = r2;

	if (rd1->priority < rd2->priority)
		return -1;
	else if (rd1->priority == rd2->priority)
		return 0;
	else
		return 1;
}

void
data_road_free_polyline_pack(data_util_state* pds, data_road* prd)
{
	int n,l;

	if (prd->vec_polyline_pack) {
		l = CSL_VectorGetLength(prd->vec_polyline_pack);

		for (n=0;n<l;n++)
			data_polyline_pack_free(pds, (data_polyline_pack*) CSL_VectorGetPointer(prd->vec_polyline_pack, n));

		VEC_DEALLOC_FSA(&pds->vec_heap, prd->vec_polyline_pack);
	}
}

