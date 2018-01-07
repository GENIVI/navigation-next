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
 * data_vector_tile.c: created 2007/10/08 by Michael Gilbert
 */

#include "data_vector_tile.h"

NB_Error
data_vector_tile_init(data_util_state* pds, data_vector_tile* pvt)
{
	NB_Error err = NE_OK;

	pvt->vec_roads = NULL;
	pvt->vec_lines = NULL;
	pvt->vec_areas = NULL;

	VEC_ALLOC_FSA(&pds->vec_heap, sizeof(data_road), pvt->vec_roads);

	if (pvt->vec_roads == NULL) {

		err = NE_NOMEM;
		goto errexit;
	}

	VEC_ALLOC_FSA(&pds->vec_heap, sizeof(data_line), pvt->vec_lines);

	if (pvt->vec_lines == NULL) {

		err = NE_NOMEM;
		goto errexit;
	}

	VEC_ALLOC_FSA(&pds->vec_heap, sizeof(data_road), pvt->vec_areas);

	if (pvt->vec_areas == NULL) {

		err = NE_NOMEM;
		goto errexit;
	}

	pvt->tx = 0;
	pvt->ty = 0;

	data_vector_tile_descriptor_init(pds, &pvt->vector_tile_descriptor);

	return NE_OK;

errexit:

	data_vector_tile_free(pds, pvt);
	return err;
}

void
data_vector_tile_free(data_util_state* pds, data_vector_tile* pvt)
{
	int n,l;

	if (pvt->vec_roads) {
		l = CSL_VectorGetLength(pvt->vec_roads);

		for (n=0;n<l;n++)
			data_road_free(pds, (data_road*) CSL_VectorGetPointer(pvt->vec_roads, n));

		VEC_DEALLOC_FSA(&pds->vec_heap, pvt->vec_roads);
	}

	if (pvt->vec_lines) {
		l = CSL_VectorGetLength(pvt->vec_lines);

		for (n=0;n<l;n++)
			data_line_free(pds, (data_line*) CSL_VectorGetPointer(pvt->vec_lines, n));

		VEC_DEALLOC_FSA(&pds->vec_heap, pvt->vec_lines);
	}
	
	if (pvt->vec_areas) {
		l = CSL_VectorGetLength(pvt->vec_areas);

		for (n=0;n<l;n++)
			data_area_free(pds, (data_area*) CSL_VectorGetPointer(pvt->vec_areas, n));

		VEC_DEALLOC_FSA(&pds->vec_heap, pvt->vec_areas);
	}

	pvt->vec_roads = NULL;
	pvt->vec_lines = NULL;
	pvt->vec_areas = NULL;
	data_vector_tile_descriptor_free(pds, &pvt->vector_tile_descriptor);
}

NB_Error
data_vector_tile_from_tps(data_util_state* pds, data_vector_tile* pvt, tpselt te, uint32 polyline_pack_passes)
{
	NB_Error err = NE_OK;
	data_road road;
	data_line line;
	data_area area;
	int num_roads, num_lines, num_areas, iter;
	tpselt ce;
	
	err = data_road_init(pds, &road);
	err = data_line_init(pds, &line);
	err = data_area_init(pds, &area);

	if (err != NE_OK)
		goto errexit;


	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_vector_tile_free(pds, pvt);

	err = data_vector_tile_init(pds, pvt);

	if (err != NE_OK)
		goto errexit;
	
	iter = num_roads = num_lines = num_areas = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {
		// @todo: (BUG 55808) support data_point
		if (nsl_strncmp(te_getname(ce), "road-", 5) == 0) {

			err = data_road_from_tps(pds, &road, ce, polyline_pack_passes);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(pvt->vec_roads, &road))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&road, 0, sizeof(road)); // clear out the road since we have copied it (shallow copy)
				else
					data_road_free(pds, &road); // free the road if it was not copied
			}

			if (err != NE_OK)
				goto errexit;
		}

		if (nsl_strncmp(te_getname(ce), "line-", 5) == 0) {

			err = data_line_from_tps(pds, &line, ce, polyline_pack_passes);

			if (err == NE_OK) {

				if (!CSL_VectorAppend(pvt->vec_lines, &line))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&line, 0, sizeof(line)); // clear out the line since we have copied it (shallow copy)
				else
					data_line_free(pds, &line); // free the line if it was not copied
			}

			if (err != NE_OK)
				goto errexit;
		}
		if (nsl_strncmp(te_getname(ce), "area-", 5) == 0) {

			err = data_area_from_tps(pds, &area, ce, polyline_pack_passes);

			if (err == NE_OK) {

				if (!CSL_VectorAppend(pvt->vec_areas, &area))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&area, 0, sizeof(area)); // clear out the area since we have copied it (shallow copy)
				else
					data_area_free(pds, &area); // free the area if it was not copied
			}

			if (err != NE_OK)
				goto errexit;
		}
	}

	pvt->tx = te_getattru(te, "tx");
	pvt->ty = te_getattru(te, "ty");

	if ((ce = te_getchild(te, "vector-tile-descriptor")) != NULL)
		err = data_vector_tile_descriptor_from_tps(pds, &pvt->vector_tile_descriptor, ce);
	else
	{
		data_vector_tile_descriptor_free(pds, &pvt->vector_tile_descriptor);
	}
	
errexit:
	data_road_free(pds, &road);
	data_line_free(pds, &line);
	data_area_free(pds, &area);

	// sort the roads/lines to be ordered by priority
	CSL_VectorSort2(pvt->vec_roads, data_road_compare, &road);
	CSL_VectorSort2(pvt->vec_lines, data_line_compare, &line);
	CSL_VectorSort2(pvt->vec_areas, data_area_compare, &area);

	if (err != NE_OK)
		data_vector_tile_free(pds, pvt);

	return err;
}

NB_Error	
data_vector_tile_copy(data_util_state* pds, data_vector_tile* pvt_dest, data_vector_tile* pvt_src)
{
	NB_Error err = NE_OK;
	int n,l;
	data_road	road;
	data_line	line;

	data_road_init(pds, &road);
	data_line_init(pds, &line);

	data_vector_tile_free(pds, pvt_dest);
	err = err ? err : data_vector_tile_init(pds, pvt_dest);
	
	l = CSL_VectorGetLength(pvt_src->vec_roads);

	for (n=0;n<l && err == NE_OK;n++) {

		err = err ? err : data_road_copy(pds, &road, CSL_VectorGetPointer(pvt_src->vec_roads, n));
		err = err ? err : CSL_VectorAppend(pvt_dest->vec_roads, &road) ? NE_OK : NE_NOMEM;

		if (err)
			data_road_free(pds, &road);
		else
			err = err ? err : data_road_init(pds, &road);
	}

	l = CSL_VectorGetLength(pvt_src->vec_lines);

	for (n=0;n<l && err == NE_OK;n++) {

		err = err ? err : data_line_copy(pds, &line, CSL_VectorGetPointer(pvt_src->vec_lines, n));
		err = err ? err : CSL_VectorAppend(pvt_dest->vec_lines, &line) ? NE_OK : NE_NOMEM;

		if (err)
			data_line_free(pds, &line);
		else
			err = err ? err : data_line_init(pds, &line);
	}

	err = err ? err : data_vector_tile_descriptor_copy(pds, &pvt_dest->vector_tile_descriptor, &pvt_src->vector_tile_descriptor);

	pvt_dest->tx = pvt_src->tx;
	pvt_dest->ty = pvt_src->ty;

	// free scratch road
	data_road_free(pds, &road);

	return err;
}
