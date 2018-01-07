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
 * data_traffic_map_reply.c: created 2007/11/5 Eric Wang
 */

#include "data_traffic_map_reply.h"
#include "data_traffic_map_road.h"
#include "vec.h"

NB_Error	
data_traffic_map_reply_init(data_util_state* pds, data_traffic_map_reply* pp)
{
	NB_Error err = NE_OK;

	pp->roads = NULL;

	VEC_ALLOC_FSA(&pds->vec_heap, sizeof(data_traffic_map_road), pp->roads);

	if (pp->roads == NULL)
	{
		return NE_NOMEM;
	}

	return err;
}

void	
data_traffic_map_reply_free(data_util_state* pds, data_traffic_map_reply* pp)
{
	int n,l;

	if (pp->roads) {
		l = CSL_VectorGetLength(pp->roads);

		for (n=0;n<l;n++)
			data_traffic_map_road_free(pds, (data_traffic_map_road*) CSL_VectorGetPointer(pp->roads, n));

		VEC_DEALLOC_FSA(&pds->vec_heap, pp->roads);
	}

	pp->roads = NULL;
}

NB_Error
data_traffic_map_reply_from_tps(data_util_state* pds, data_traffic_map_reply* pp, tpselt te)
{
	NB_Error err = NE_OK;
	data_traffic_map_road road;
	int num_road, iter;
	tpselt ce;
	
	err = data_traffic_map_road_init(pds, &road);

	if (err != NE_OK)
		goto errexit;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_traffic_map_reply_free(pds, pp);
	err = data_traffic_map_reply_init(pds, pp);

	if (err != NE_OK)
		goto errexit;
	
	iter = num_road = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "traffic-map-road") == 0) {

			err = data_traffic_map_road_from_tps(pds, &road, ce);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(pp->roads, &road))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&road, 0, sizeof(road)); // clear out the vector_tile since we have copied it (shallow copy)
				else
					data_traffic_map_road_free(pds, &road); // free the vector_tile if it was not copied
			}

			if (err != NE_OK)
				goto errexit;
		}
	}

errexit:
	data_traffic_map_road_free(pds, &road);

	if (err != NE_OK)
		data_traffic_map_reply_free(pds, pp);

	return err;
}


NB_Error	
data_traffic_map_reply_copy(data_util_state* pds, data_traffic_map_reply* pda_dest, data_traffic_map_reply* pda_src)
{
	NB_Error err = NE_OK;


	return err;
}
