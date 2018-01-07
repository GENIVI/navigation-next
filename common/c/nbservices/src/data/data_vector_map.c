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

/*********************************************************************/
/*                                                                   */
/* (C) Copyright 2000 - 2005 by Networks In Motion, Inc.             */
/*                                                                   */
/* The information contained herein is confidential and propvmetary  */
/* to Networks In Motion, Inc., and considered a trade secret as     */
/* defined in section 499C of the penal code of the State of         */
/* California.                                                       */
/*                                                                   */
/* This intellectual property is for internal use only by            */
/* Networks In Motion, Inc.  This source code contains propvmetary   */
/* information of Networks In Motion, Inc. and shall not be used or  */
/* copied by or disclosed to anyone other than a Networks In Motion, */
/* Inc. certified employee that has obtained written authorization   */
/* to view or modify said source code.                               */
/*                                                                   */
/*********************************************************************/

/*
 * data_vector_map.h: created 2005/06/03 by Mark Goddard.
 */

#include "data_vector_map.h"

#if 0

NB_Error
data_vector_map_init(data_util_state* pds, data_vector_map* pvm)
{
	NB_Error err = NE_OK;

	VEC_ALLOC_FSA(&pds->vec_heap, sizeof(data_road), pvm->vec_roads);

	if (pvm->vec_roads == NULL)
		return NE_NOMEM;

	return err;
}

void		
data_vector_map_free(data_util_state* pds, data_vector_map* pvm)
{
	int n,l;

	if (pvm->vec_roads) {
		l = CSL_VectorGetLength(pvm->vec_roads);

		for (n=0;n<l;n++)
			data_road_free(pds, (data_road*) CSL_VectorGetPointer(pvm->vec_roads, n));

		VEC_DEALLOC_FSA(&pds->vec_heap, pvm->vec_roads);
	}

	pvm->vec_roads = NULL;
}

NB_Error	
data_vector_map_from_tps(data_util_state* pds, data_vector_map* pvm, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt	ce;
	int iter;
	int num_road;
	data_road road;
	
	err = data_road_init(pds, &road);

	if (err != NE_OK)
		return err;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_vector_map_free(pds, pvm);

	err = data_vector_map_init(pds, pvm);

	if (err != NE_OK)
		return err;
	
	iter = 0;
	num_road = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (STRNCMP(te_getname(ce), "road-", 5) == 0)
			num_road++;
	}

	if (!CSL_VectorSetAlloc(pvm->vec_roads, num_road)) {
		err = NE_NOMEM;
		goto errexit;
	}
	
	iter = 0;

	while ((ce = te_nextchild(te, &iter)) != NULL) {

		if (STRNCMP(te_getname(ce), "road-", 5) == 0) {

			err = data_road_from_tps(pds, &road, ce);

			if (err == NE_OK) {
				if (!CSL_VectorAppend(pvm->vec_roads, &road))
					err = NE_NOMEM;

				if (err == NE_OK)
					nsl_memset(&road, 0, sizeof(road)); // clear out the road since we have copied it (shallow copy)
				else
					data_road_free(pds, &road); // free the road if it was not copied
			}

			if (err != NE_OK)
				goto errexit;
		}
	}

errexit:
	data_road_free(pds, &road);

	// sort the roads to be order by priority
	CSL_VectorSort2(pvm->vec_roads, data_road_compare, &road);

	if (err != NE_OK)
		data_vector_map_free(pds, pvm);
	return err;
}

NB_Error	
data_vector_map_copy(data_util_state* pds, data_vector_map* pvm_dest, data_vector_map* pvm_src)
{
	NB_Error err = NE_OK;
	int n,l;
	data_road	road;

	data_road_init(pds, &road);

	data_vector_map_free(pds, pvm_dest);
	err = err ? err : data_vector_map_init(pds, pvm_dest);
	
	l = CSL_VectorGetLength(pvm_src->vec_roads);

	for (n=0;n<l && err == NE_OK;n++) {

		err = err ? err : data_road_copy(pds, &road, CSL_VectorGetPointer(pvm_src->vec_roads, n));
		err = err ? err : CSL_VectorAppend(pvm_dest->vec_roads, &road) ? NE_OK : NE_NOMEM;

		if (err)
			data_road_free(pds, &road);
		else
			data_road_init(pds, &road);
	}

	// free scratch road
	data_road_free(pds, &road);

	return err;
}

#endif

