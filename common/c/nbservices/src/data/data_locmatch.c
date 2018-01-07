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
 * data_locmatch.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_locmatch.h"
#include "data_extapp_content.h"

static void
data_locmatch_clear(data_util_state* pds, data_locmatch* plm)
{
	plm->incomplete = FALSE;
    plm->accuracy = 0;
}

NB_DEF NB_Error
data_locmatch_init(data_util_state* pds, data_locmatch* plm)
{
	NB_Error err = NE_OK;

	err = data_location_init(pds, &plm->location);

    DATA_VEC_ALLOC(err, plm->vec_extapp_content, data_extapp_content);

	data_locmatch_clear(pds, plm);

	return err;
}

NB_DEF void	
data_locmatch_free(data_util_state* pds, data_locmatch* plm)
{
	data_location_free(pds, &plm->location);

    DATA_VEC_FREE(pds, plm->vec_extapp_content, data_extapp_content);

	data_locmatch_clear(pds, plm);
}

NB_DEF tpselt		
data_locmatch_to_tps(data_util_state* pds, data_locmatch* plm)
{
	tpselt te;
	tpselt ce;

	te = te_new("locmatch");
	
	if (te == NULL)
		goto errexit;

	if ((ce = data_location_to_tps(pds, &plm->location)) != NULL && te_attach(te, ce))
		ce = NULL;
	else
		goto errexit;
	
	if (plm->incomplete) {

		if ((ce = te_new("incomplete")) != NULL && te_attach(te, ce))
			ce = NULL;
		else
			goto errexit;
	}

    if (plm->vec_extapp_content)
    {
        int n = 0;
        int l = 0;

        l = CSL_VectorGetLength(plm->vec_extapp_content);

        for (n = 0; n < l; n++)
        {
            data_extapp_content* eac = CSL_VectorGetPointer(plm->vec_extapp_content, n);

            DATA_TO_TPS(pds, errexit, te, eac, data_extapp_content);
        }
    }

    if (!te_setattru(te, "accuracy", plm->accuracy))
    {
        goto errexit;
    }

	return te;

errexit:

	te_dealloc(te);
	return NULL;
}

NB_DEF NB_Error	
data_locmatch_from_tps(data_util_state* pds, data_locmatch* plm, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;
    int iter = 0;

    if (te == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_locmatch_free(pds, plm);

    err = data_locmatch_init(pds, plm);
    if (err != NE_OK)
    {
        goto errexit;
    }

    ce = te_getchild(te, "location");
    if (ce == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    while ((ce = te_nextchild(te, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "location") == 0)
        {
            err = data_location_from_tps(pds, &plm->location, ce);
        }
        else if (nsl_strcmp(te_getname(ce), "incomplete") == 0)
        {
            plm->incomplete = TRUE;
        }
        else if (nsl_strcmp(te_getname(ce), "extapp-content") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, plm->vec_extapp_content, data_extapp_content);
        }

        if (err != NE_OK)
        {
            goto errexit;
        }
    }

    plm->accuracy = te_getattru(te, "accuracy");

errexit:
    if (err != NE_OK)
    {
        data_locmatch_free(pds, plm);
    }

    return err;
}

NB_DEF boolean		
data_locmatch_equal(data_util_state* pds, data_locmatch* plm1, data_locmatch* plm2)
{
    int ret = TRUE;

    DATA_EQUAL(pds, ret, &plm1->location, &plm2->location, data_location);

    if (ret)
    {
        ret = (plm1->incomplete == plm2->incomplete);
    }

    DATA_VEC_EQUAL(pds, ret, plm1->vec_extapp_content, plm2->vec_extapp_content, data_extapp_content);

    return (boolean)ret;
}

NB_DEF NB_Error	
data_locmatch_copy(data_util_state* pds, data_locmatch* plm_dest, data_locmatch* plm_src)
{
	NB_Error err = NE_OK;

	data_locmatch_free(pds, plm_dest);
	data_locmatch_init(pds, plm_dest);

	err = data_location_copy(pds, &plm_dest->location, &plm_src->location);
	plm_dest->incomplete = plm_src->incomplete;

    DATA_VEC_COPY(pds, err, plm_dest->vec_extapp_content, plm_src->vec_extapp_content, data_extapp_content);

	return err;
}
