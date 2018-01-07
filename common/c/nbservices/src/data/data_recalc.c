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
/* The information contained herein is confidential and proprcetary  */
/* to Networks In Motion, Inc., and considered a trade secret as     */
/* defined in section 499C of the penal code of the State of         */
/* California.                                                       */
/*                                                                   */
/* This intellectual property is for internal use only by            */
/* Networks In Motion, Inc.  This source code contains proprcetary   */
/* information of Networks In Motion, Inc. and shall not be used or  */
/* copied by or disclosed to anyone other than a Networks In Motion, */
/* Inc. certified employee that has obtained written authorization   */
/* to view or modify said source code.                               */
/*                                                                   */
/*********************************************************************/

/*
 * data_recalc.h: created 2005/03/24 by Mark Goddard.
 */

#include "data_recalc.h"

NB_Error
data_recalc_init(data_util_state* pds, data_recalc* prc)
{
	NB_Error err = NE_OK;

	err = err ? err : data_blob_init(pds, &prc->route_id);
	err = err ? err : data_string_init(pds, &prc->why);

	return err;
}

void		
data_recalc_free(data_util_state* pds, data_recalc* prc)
{
	data_blob_free(pds, &prc->route_id);
	data_string_free(pds, &prc->why);
}

NB_Error  data_recalc_copy(data_util_state* pds, data_recalc* dst, data_recalc* src)
{
    NB_Error err = NE_OK;

    err = err ? err : data_blob_copy(pds, &dst->route_id, &src->route_id);
    err = err ? err : data_string_copy(pds, &dst->why, &src->why);

    return err;
}

tpselt		
data_recalc_to_tps(data_util_state* pds, data_recalc* prc)
{
	tpselt te;

	te = te_new("recalc");

	if (te == NULL)
		goto errexit;

	if (!te_setattr(te, "route-id", (const char*) prc->route_id.data, prc->route_id.size))
		goto errexit;

	if (!te_setattrc(te, "why", data_string_get(pds, &prc->why)))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	return NULL;
}
