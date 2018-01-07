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
 * data_route_map.c: created 2007/10/15 by Michael Gilbert
 */

#include "data_route_map.h"

NB_Error
data_route_map_init(data_util_state* pds, data_route_map* prm)
{
	NB_Error err = NE_OK;

	prm->vec_vector_tiles = NULL;

	VEC_ALLOC_FSA(&pds->vec_heap, sizeof(data_vector_tile), prm->vec_vector_tiles);

	if (prm->vec_vector_tiles == NULL) {

		err = NE_NOMEM;
		goto errexit;
	}

	return NE_OK;

errexit:

	data_route_map_free(pds, prm);
	return err;
}

void
data_route_map_free(data_util_state* pds, data_route_map* prm)
{
	int n,l;

	if (prm->vec_vector_tiles) {
		l = CSL_VectorGetLength(prm->vec_vector_tiles);

		for (n=0;n<l;n++)
			data_vector_tile_free(pds, (data_vector_tile*) CSL_VectorGetPointer(prm->vec_vector_tiles, n));

		VEC_DEALLOC_FSA(&pds->vec_heap, prm->vec_vector_tiles);
	}

	prm->vec_vector_tiles = NULL;
}

NB_Error
data_route_map_from_tps(data_util_state* pds, data_route_map* prm, tpselt te, uint32 polyline_pack_passes)
{
	NB_Error err = NE_OK;
	data_vector_tile vector_tile;
	int iter;
	uint32 num_vector_tiles;
	tpselt ce;
	
	err = data_vector_tile_init(pds, &vector_tile);

	if (err != NE_OK)
		goto errexit;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_route_map_free(pds, prm);

	err = data_route_map_init(pds, prm);

	if (err != NE_OK)
		goto errexit;
	
	iter = num_vector_tiles = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (nsl_strcmp(te_getname(ce), "vector-tile") == 0) {

			err = data_vector_tile_from_tps(pds, &vector_tile, ce, polyline_pack_passes);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(prm->vec_vector_tiles, &vector_tile))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&vector_tile, 0, sizeof(vector_tile)); // clear out the vector_tile since we have copied it (shallow copy)
				else
					data_vector_tile_free(pds, &vector_tile); // free the vector_tile if it was not copied
			}

			if (err != NE_OK)
				goto errexit;
		}
	}

errexit:
	data_vector_tile_free(pds, &vector_tile);

	if (err != NE_OK)
		data_route_map_free(pds, prm);

	return err;
}

NB_Error	
data_route_map_copy(data_util_state* pds, data_route_map* prm_dest, data_route_map* prm_src)
{
	NB_Error err = NE_OK;
	int n,l;
	data_vector_tile	vector_tile;

	data_vector_tile_init(pds, &vector_tile);

	data_route_map_free(pds, prm_dest);
	err = err ? err : data_route_map_init(pds, prm_dest);
	
	l = CSL_VectorGetLength(prm_src->vec_vector_tiles);

	for (n=0;n<l && err == NE_OK;n++) {

		err = err ? err : data_vector_tile_copy(pds, &vector_tile, CSL_VectorGetPointer(prm_src->vec_vector_tiles, n));
		err = err ? err : CSL_VectorAppend(prm_dest->vec_vector_tiles, &vector_tile) ? NE_OK : NE_NOMEM;

		if (err)
			data_vector_tile_free(pds, &vector_tile);
		else
			err = err ? err : data_vector_tile_init(pds, &vector_tile);
	}

	// free scratch vector_tile
	data_vector_tile_free(pds, &vector_tile);

	return err;
}

