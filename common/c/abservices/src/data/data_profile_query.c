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
/* (C) Copyright 2008 by Networks In Motion, Inc.                */
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

#include "data_profile_query.h"

static boolean attachNamedPairElement(data_util_state* pds, tpselt te, const char* name, data_pair* pair);

NB_Error
data_profile_query_init(data_util_state* pds, data_profile_query* ppq)
{
	NB_Error err = NE_OK;

	ppq->isDefault = FALSE;
	ppq->version = 0;
	err = data_pair_init(pds, &ppq->getValue);
	err = err ? err : data_pair_init(pds, &ppq->setValue);

	return err;
}

void
data_profile_query_free(data_util_state* pds, data_profile_query* ppq)
{
	data_pair_free(pds, &ppq->getValue);
	data_pair_free(pds, &ppq->setValue);
}

tpselt
data_profile_query_to_tps(data_util_state* pds, data_profile_query* ppq)
{
	tpselt te = NULL;
	tpselt ce = NULL;
	
	te = te_new("profile-query");
	if (!te) {
		goto errexit;
	}

	if (!te_setattru(te, "version", ppq->version)) {
		goto errexit;
	}

	if (ppq->isDefault) {
		ce = te_new("default");
		if (!ce || !te_attach(te, ce)) {
			goto errexit;
		}
	}

	if (!attachNamedPairElement(pds, te, "get-value", &ppq->getValue)) {
		goto errexit;
	}

	if (!attachNamedPairElement(pds, te, "set-value", &ppq->setValue)) {
		goto errexit;
	}

	return te;

errexit:
    te_dealloc(te);
	te_dealloc(ce);
	return NULL;
}

boolean
data_profile_query_equal(data_util_state* pds, data_profile_query* ppq1, data_profile_query* ppq2)
{
	int ret = ppq1->isDefault == ppq2->isDefault && ppq1->version == ppq2->version;
	ret = ret ? ret : data_pair_equal(pds, &ppq1->getValue, &ppq2->getValue);
	ret = ret ? ret : data_pair_equal(pds, &ppq1->setValue, &ppq2->setValue);

	return (boolean)ret;
}

NB_Error
data_profile_query_copy(data_util_state* pds, data_profile_query* ppq_dest, data_profile_query* ppq_src)
{
	NB_Error err = NE_OK;

	data_profile_query_free(pds, ppq_dest);
	err = data_profile_query_init(pds, ppq_dest);

	ppq_dest->isDefault = ppq_src->isDefault;
	ppq_dest->version = ppq_src->version;

	err = err ? err : data_pair_copy(pds, &ppq_dest->getValue, &ppq_src->getValue);
	err = err ? err : data_pair_copy(pds, &ppq_dest->setValue, &ppq_src->setValue);

	return err;
}

static boolean
attachNamedPairElement(data_util_state* pds, tpselt te, const char* name, data_pair* pair)
{
	tpselt namedElement = NULL;
	tpselt pairElement = NULL;

	if (nsl_strlen(data_string_get(pds, &pair->key)) > 0) {
		namedElement = te_new(name);
		if (!namedElement) {
			goto errexit;
		}

		if ((pairElement = data_pair_to_tps(pds, pair)) == NULL || !te_attach(namedElement, pairElement)) {
			goto errexit;
		}

		if (!te_attach(te, namedElement)) {
			goto errexit;
		}
	}

	return TRUE;

errexit:
	te_dealloc(namedElement);
	te_dealloc(pairElement);
	return FALSE;
}
