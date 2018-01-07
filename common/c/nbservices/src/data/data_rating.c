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
 * data_rating.c: created 2005/11/07 by Mark Goddard.
 */

#include "data_rating.h"

NB_Error	
data_rating_init(data_util_state* pds, data_rating* dat)
{
	NB_Error err = NE_OK;

    dat->star = 0;
    
    DATA_INIT(pds, err, &dat->mpaa, data_string);

	return err;
}

void	
data_rating_free(data_util_state* pds, data_rating* dat)
{

    DATA_FREE(pds, &dat->mpaa, data_string);
}

NB_Error	
data_rating_from_tps(data_util_state* pds, data_rating* dat, tpselt te)
{
	NB_Error err = NE_OK;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(pds, err, dat, data_rating);

    err = err ? err : data_string_from_tps_attr(pds, &dat->mpaa, te, "mpaa");

    dat->star = te_getattru(te, "star");

errexit:
	if (err != NE_OK)
		data_rating_free(pds, dat);
	return err;
}

boolean		
data_rating_equal(data_util_state* pds, data_rating* lhs, data_rating* rhs)
{
   int ret = TRUE;

   DATA_EQUAL(pds, ret, &lhs->mpaa, &rhs->mpaa, data_string);

   ret = ret && lhs->star == rhs->star;

   return (boolean) ret;

}

NB_Error	
data_rating_copy(data_util_state* pds, data_rating* dst, data_rating* src)
{
	NB_Error err = NE_OK;

    DATA_REINIT(pds, err, dst, data_rating);

    DATA_COPY(pds, err, &dst->mpaa, &src->mpaa, data_string);

    dst->star = src->star;

    return err;
}

uint32   data_rating_get_tps_size(data_util_state* pds, data_rating* pdat)
{
    uint32 size = 0;

    size += sizeof(pdat->star);
    size += sizeof(pdat->mpaa);

    return size;
}

void     data_rating_to_buf(data_util_state* pds, data_rating* pdat, struct dynbuf* pdb)
{
    dbufcat(pdb, (byte*)&pdat->star, sizeof(pdat->star));
    data_string_to_buf(pds, &pdat->mpaa, pdb);
}

NB_Error data_rating_from_binary(data_util_state* pds, data_rating* pdat, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_uint32_from_binary(pds, &pdat->star, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pdat->mpaa, pdata, pdatalen);

    return err;
}

