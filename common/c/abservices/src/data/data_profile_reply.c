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

#include "data_profile_reply.h"

static NB_Error getNamedPairElement(data_util_state* pds, data_pair* pair, tpselt te);

NB_Error
data_profile_reply_init(data_util_state* pds, data_profile_reply* ppr)
{
	NB_Error err = NE_OK;

	err = err ? err : data_pair_init(pds, &ppr->getValue);
	err = err ? err : data_pair_init(pds, &ppr->setValue);
	err = err ? err : data_error_msg_init(pds, &ppr->errorMsg);

	return err;
}

void
data_profile_reply_free(data_util_state* pds, data_profile_reply* ppr)
{
	data_pair_free(pds, &ppr->getValue);
	data_pair_free(pds, &ppr->setValue);
	data_error_msg_free(pds, &ppr->errorMsg);
}

NB_Error
data_profile_reply_from_tps(data_util_state* pds, data_profile_reply* ppr, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt ce;
	int iter;

	if (te == NULL)
	{
		err = NE_INVAL;
		goto errexit;
	}

	data_profile_reply_free(pds, ppr);
	err = data_profile_reply_init(pds, ppr);

	iter = 0;
	while (!err && (ce = te_nextchild(te, &iter)) != NULL)
	{
	    const char *elmentName = te_getname(ce);
        if (nsl_strcmp(elmentName, "get-value") == 0)
		{
			err = getNamedPairElement(pds, &ppr->getValue, ce);
		}
		else if (nsl_strcmp(elmentName, "set-value") == 0)
		{
			err = getNamedPairElement(pds, &ppr->setValue, ce);
		}
		else if (nsl_strcmp(elmentName, "error_msg"))
		{
		    err = data_error_msg_from_tps(pds, &ppr->errorMsg, ce);
		}
		else
		{
			err = NE_INVAL;
		}
	}
	
errexit:
	if (err != NE_OK)
	{
		data_profile_reply_free(pds, ppr);
	}

	return err;
}

boolean
data_profile_reply_equal(data_util_state* pds, data_profile_reply* ppr1, data_profile_reply* ppr2)
{
	return (boolean)( data_pair_equal(pds, &ppr1->getValue, &ppr2->getValue) &&
                      data_pair_equal(pds, &ppr1->setValue, &ppr2->setValue) &&
                      data_error_msg_equal(pds, &ppr1->errorMsg, &ppr2->errorMsg) );
}

NB_Error
data_profile_reply_copy(data_util_state* pds, data_profile_reply* ppr_dest, data_profile_reply* ppr_src)
{
	NB_Error err = NE_OK;

	data_profile_reply_free(pds, ppr_dest);
	err = data_profile_reply_init(pds, ppr_dest);

	err = err ? err : data_pair_copy(pds, &ppr_dest->getValue, &ppr_src->getValue);
	err = err ? err : data_pair_copy(pds, &ppr_dest->setValue, &ppr_src->setValue);
	err = err ? err : data_error_msg_copy(pds, &ppr_dest->errorMsg, &ppr_src->errorMsg);

	return err;
}

static NB_Error
getNamedPairElement(data_util_state* pds, data_pair* pair, tpselt te)
{
	tpselt pairElement;

	pairElement = te_getchild(te, "pair");
	if (!pairElement) {
		return NE_INVAL;
	}

	return data_pair_from_tps(pds, pair, pairElement);
}
