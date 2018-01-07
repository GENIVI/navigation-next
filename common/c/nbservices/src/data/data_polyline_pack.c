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
 * data_polyline_pack.c: created 2007/10/09 by Michael Gilbert.
 */

#include "data_polyline_pack.h"

NB_Error
data_polyline_pack_init(data_util_state* pds, data_polyline_pack* ppp)
{
	ppp->segments = NULL;
	ppp->numsegments = 0;

	return NE_OK;
}

void
data_polyline_pack_free(data_util_state* pds, data_polyline_pack* ppp)
{
	nsl_free(ppp->segments);
	ppp->segments = NULL;
	ppp->numsegments = 0;
}

static void
decode_packed_coordinate_pair(byte* data, double* px, double* py, uint32 passes)
{
	uint32 p, n;
	uint32 mask = 32768;
	uint32 v = 0;

	for (n = 0; n < ((passes*2+7)/8); n++) {
		v <<= 8;
		v |= data[n];
	}

    *px = *py = 0.0;

    p = 1<<(passes*2 - 1);

    while (p > 0)
	{
        if (p & v)
            *px += mask;
        p >>= 1;
        if (p & v)
            *py += mask;
        p >>= 1;
        mask >>= 1;

	}

	// Adjust for border edge and size
    *px = (*px * 1.25 / 65536.0) - .125;
    *py = (*py * 1.25 / 65536.0) - .125;
}

static NB_Error 
decode_polyline_pack_data(data_util_state* pds, byte* data, size_t data_size, uint32 passes, plp_segment** ppstrip, uint32* pnumpts)
{
	NB_Error err = NE_OK;
	// Width = passes*2 bits.  Round up to the next whole byte value.
	uint32 width = (passes * 2 + (8 - 1)) / 8;
	uint32 n;

	*pnumpts = (uint32)(data_size / width);

	*ppstrip = nsl_malloc(sizeof(plp_segment) * (*pnumpts));

	if ((*ppstrip) == NULL) {
		err = NE_NOMEM;
		goto errexit;
	}


	for (n = 0; n < (*pnumpts); n++) {
	
		decode_packed_coordinate_pair(data+n*width, &(*ppstrip)[n].x, &(*ppstrip)[n].y, passes);
	}

errexit:

	if (err != NE_OK) {

		if ((*ppstrip) != NULL) {

			nsl_free(*ppstrip);
			*ppstrip  = NULL;
			*pnumpts = 0;
		}
	}

	return err;
}

NB_Error
data_polyline_pack_from_tps_attr(data_util_state* pds, data_polyline_pack* ppp, tpselt te, const char* name, uint32 passes)
{
	char*	data;
	size_t	size;

	if (te == NULL || name == NULL)
		return NE_INVAL;

	if (!te_getattr(te, name, &data, &size))
		return NE_INVAL;

	ppp->numpasses = passes;

	return decode_polyline_pack_data(pds, (byte*)data, size, ppp->numpasses, &ppp->segments, &ppp->numsegments);
}

NB_Error
data_polyline_pack_copy(data_util_state* pds, data_polyline_pack* ppp_dest, data_polyline_pack* ppp_src)
{
	NB_Error err = NE_OK;

	if (ppp_src->numsegments == 0 && ppp_src->segments == NULL) {

		ppp_dest->numsegments = 0;
		ppp_dest->segments = NULL;
	}
	else {

		ppp_dest->numsegments = ppp_src->numsegments;

		ppp_dest->segments = nsl_malloc(ppp_dest->numsegments * sizeof(plp_segment));

		if (ppp_dest->segments == NULL) {

			ppp_dest->numsegments = 0;
			return NE_NOMEM;
		}

		nsl_memcpy(ppp_dest->segments, ppp_src->segments, ppp_dest->numsegments * sizeof(plp_segment));
	}
	return err;
}

uint32		
data_polyline_pack_len(data_util_state* pds, data_polyline_pack* ppp)
{
	return ppp->numsegments;
}

NB_Error
data_polyline_pack_enum(data_util_state* pds, data_polyline_pack* ppp, data_polyline_pack_enum_cb cb, void* cbuser)
{
	NB_Error err = NE_OK;
	uint32 n;

	for (n=ppp->numsegments-1;n>=1;n--) {

		if (cb != NULL)
			cb(cbuser,ppp->segments[n].x,ppp->segments[n].y,ppp->segments[n-1].x,ppp->segments[n-1].y);
	}

	return err;
}

NB_Error
data_polyline_pack_enum_rev(data_util_state* pds, data_polyline_pack* ppp, data_polyline_pack_enum_cb cb, void* cbuser)
{
	NB_Error err = NE_OK;
	uint32 n;

	for (n=1;n<ppp->numsegments;n++) {

		if (cb != NULL)
			cb(cbuser,ppp->segments[n-1].x,ppp->segments[n-1].y,ppp->segments[n].x,ppp->segments[n].y);
	}

	return err;
}

