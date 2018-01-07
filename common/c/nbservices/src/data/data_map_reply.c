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
 * data_map_reply.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_map_reply.h"
#include "data_string.h"
#include "data_blob.h"
#include "vec.h"
#include "data_traffic_map_road.h"
#include "datautil.h"
#include "data_proxmatch.h"

static void
data_map_reply_clear(data_util_state* pds, data_map_reply* pmr)
{
	pmr->scale = 0.0;
	pmr->rotate = 0.0;
}

NB_Error
data_map_reply_init(data_util_state* pds, data_map_reply* pmr)
{
	NB_Error err = NE_OK;

	pmr->roads = NULL;
	pmr->hastraffic = FALSE;

	//VEC_ALLOC_FSA(&pds->vec_heap, sizeof(data_traffic_map_road), pmr->roads);
	pmr->roads = CSL_VectorAllocPlace(sizeof(data_traffic_map_road), fsaheap_alloc(&pds->vec_heap));

	if (pmr->roads == NULL) {
		err = NE_NOMEM;
		goto errexit;
	}

	err = err ? err : data_point_init(pds, &pmr->point);
	err = err ? err : data_locmatch_init(pds, &pmr->locmatch);
	err = err ? err : data_blob_init(pds, &pmr->image);

    DATA_VEC_ALLOC(err, pmr->proxmatches, data_proxmatch);

	data_map_reply_clear(pds, pmr);

	return err;

errexit:
	data_map_reply_free(pds, pmr);
	return err;
}

void		
data_map_reply_free(data_util_state* pds, data_map_reply* pmr)
{
	int n,l;
	data_point_free(pds, &pmr->point);
	data_locmatch_free(pds, &pmr->locmatch);
	data_blob_free(pds, &pmr->image);

	if (pmr->roads) {
		l = CSL_VectorGetLength(pmr->roads);
		for (n=0;n<l;n++)
			data_traffic_map_road_free(pds, (data_traffic_map_road*) CSL_VectorGetPointer(pmr->roads, n));

		VEC_DEALLOC_FSA(&pds->vec_heap, pmr->roads);
	}

	pmr->roads = NULL;
	pmr->hastraffic = FALSE;

    DATA_VEC_FREE(pds, pmr->proxmatches, data_proxmatch);

	data_map_reply_clear(pds, pmr);


}

NB_Error	
data_map_reply_from_tps(data_util_state* pds, data_map_reply* pmr, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;
	char*	data;
	size_t	size;
	int iter, num_roads;
	data_traffic_map_road road;
	
	err = data_traffic_map_road_init(pds, &road);

	if (err != NE_OK)
		goto errexit;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_map_reply_free(pds, pmr);

	err = data_map_reply_init(pds, pmr);

	if (err != NE_OK)
		return err;
	
	ce = te_getchild(te, "map-has-traffic");

	if (ce == NULL) 
		pmr->hastraffic = FALSE;
	else
		pmr->hastraffic = TRUE;

	ce = te_getchild(te, "point");

	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}


	err = err ? err : data_point_from_tps(pds, &pmr->point, ce);

	ce = te_getchild(te, "locmatch");

	if (ce != NULL) {

		err = err ? err : data_locmatch_from_tps(pds, &pmr->locmatch, ce);
	}

	if (te_getattr(te, "image", &data, &size)) {

		err = err ? err : data_blob_set(pds, &pmr->image, (byte*) data, size);
	}
	else {
		err = NE_INVAL;
		goto errexit;
	}

	if (!te_getattrf(te, "scale", &pmr->scale) || !te_getattrf(te, "rotate", &pmr->rotate))
		err = NE_INVAL;

	iter = num_roads = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "traffic-map-road") == 0) {
		
			err = data_traffic_map_road_from_tps(pds, &road, ce);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(pmr->roads, &road))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&road, 0, sizeof(road)); // clear out the vector_tile since we have copied it (shallow copy)
				else
					data_traffic_map_road_free(pds, &road); // free the vector_tile if it was not copied
			}


			if (err != NE_OK)
				goto errexit;
		}
		if (nsl_strcmp(te_getname(ce), "proxmatch") == 0) {
			DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pmr->proxmatches, data_proxmatch);
		}
	}
errexit:
	if (err != NE_OK)
		data_map_reply_free(pds, pmr);
	return err;
}

