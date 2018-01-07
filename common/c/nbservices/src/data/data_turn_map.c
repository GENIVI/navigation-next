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
/* The information contained herein is confidential and proptmetary  */
/* to Networks In Motion, Inc., and considered a trade secret as     */
/* defined in section 499C of the penal code of the State of         */
/* California.                                                       */
/*                                                                   */
/* This intellectual property is for internal use only by            */
/* Networks In Motion, Inc.  This source code contains proptmetary   */
/* information of Networks In Motion, Inc. and shall not be used or  */
/* copied by or disclosed to anyone other than a Networks In Motion, */
/* Inc. certified employee that has obtained written authorization   */
/* to view or modify said source code.                               */
/*                                                                   */
/*********************************************************************/

/*
 * data_turn_map.h: created 2005/06/03 by Mark Goddard.
 */

#include "data_turn_map.h"

#if 0

NB_Error
data_turn_map_init(data_util_state* pds, data_turn_map* ptm)
{
	NB_Error err = NE_OK;

	err = err ? err : data_point_init(pds, &ptm->center);
	err = err ? err : data_vector_map_init(pds, &ptm->vector_map);

	ptm->scale = 0.0;
	ptm->rotation = 0.0;

	return err;
}

void		
data_turn_map_free(data_util_state* pds, data_turn_map* ptm)
{
	data_point_free(pds, &ptm->center);
	data_vector_map_free(pds, &ptm->vector_map);
}

NB_Error	
data_turn_map_from_tps(data_util_state* pds, data_turn_map* ptm, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt ce;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_turn_map_free(pds, ptm);

	err = data_turn_map_init(pds, ptm);

	if (err != NE_OK)
		return err;
	
	ce = te_getchild(te, "point");

	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	err = err ? err : data_point_from_tps(pds, &ptm->center, ce);

	ce = te_getchild(te, "vector-map");

	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	err = err ? err : data_vector_map_from_tps(pds, &ptm->vector_map, ce);

	if (err == NE_OK) {

		if (!te_getattrf(te, "scale", &ptm->scale) || !te_getattrf(te, "rotation", &ptm->rotation))
			err = NE_INVAL;
	}

errexit:
	if (err != NE_OK)
		data_turn_map_free(pds, ptm);
	return err;
}

NB_Error	
data_turn_map_copy(data_util_state* pds, data_turn_map* ptm_dest, data_turn_map* ptm_src)
{
	NB_Error err = NE_OK;

	data_turn_map_free(pds, ptm_dest);
	data_turn_map_init(pds, ptm_dest);

	err = err ? err : data_point_copy(pds, &ptm_dest->center, &ptm_src->center);
	err = err ? err : data_vector_map_copy(pds, &ptm_dest->vector_map, &ptm_src->vector_map);

	ptm_dest->scale = ptm_src->scale;
	ptm_dest->rotation = ptm_src->rotation;

	return err;
}

#endif

