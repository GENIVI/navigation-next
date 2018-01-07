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
/* The information contained herein is confidential and propmdetary  */
/* to Networks In Motion, Inc., and considered a trade secret as     */
/* defined in section 499C of the penal code of the State of         */
/* California.                                                       */
/*                                                                   */
/* This intellectual property is for internal use only by            */
/* Networks In Motion, Inc.  This source code contains propmdetary   */
/* information of Networks In Motion, Inc. and shall not be used or  */
/* copied by or disclosed to anyone other than a Networks In Motion, */
/* Inc. certified employee that has obtained written authorization   */
/* to view or modify said source code.                               */
/*                                                                   */
/*********************************************************************/

/*
 * data_map_descriptor.h: created 2005/06/03 by Mark Goddard.
 */

#ifndef DISABLE_VECTOR_TURN_MAPS

#include "data_map_descriptor.h"

NB_Error
data_map_descriptor_init(data_util_state* pds, data_map_descriptor* pmd)
{
	NB_Error err = NE_OK;

	err = err ? err : data_polyline_init(pds, &pmd->bounds);

	return err;
}

void		
data_map_descriptor_free(data_util_state* pds, data_map_descriptor* pmd)
{
	data_polyline_free(pds, &pmd->bounds);
}

NB_Error	
data_map_descriptor_from_tps(data_util_state* pds, data_map_descriptor* pmd, tpselt te)
{
	NB_Error err = NE_OK;
	
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_map_descriptor_free(pds, pmd);

	err = data_map_descriptor_init(pds, pmd);

	if (err != NE_OK)
		return err;
/*	
	err = err ? err : data_polyline_from_tps_attr(pds, &pmd->bounds, te, "bounds");
	
	pmd->minPpriority = te_getattru(te, "min-priority");
	pmd->maxPriority = te_getattru(te, "max-priority");
	
	if (!te_getattrf(te, "min-scale", &pmd->minScale))
		err = NE_BADDATA;
*/
errexit:
	if (err != NE_OK)
		data_map_descriptor_free(pds, pmd);
	return err;
}

NB_Error	
data_map_descriptor_copy(data_util_state* pds, data_map_descriptor* pmd_dest, data_map_descriptor* pmd_src)
{
	NB_Error err = NE_OK;

	data_map_descriptor_free(pds, pmd_dest);
	err = data_map_descriptor_init(pds, pmd_dest);

	err = err ? err : data_polyline_copy(pds, &pmd_dest->bounds, &pmd_src->bounds);

	pmd_dest->minPpriority = pmd_src->minPpriority;
	pmd_dest->maxPriority = pmd_src->maxPriority;
	pmd_dest->minScale = pmd_src->minScale;

	return err;
}

#endif

