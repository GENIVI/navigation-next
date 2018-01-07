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
 * data_polyline.c: created 2005/01/06 by Mark Goddard.
 */

#include "data_polyline.h"
#include "nbspatial.h"
#include "csltypes.h"

NB_Error
data_polyline_init(data_util_state* pds, data_polyline* ppl)
{
	ppl->segments = NULL;
	ppl->numsegments = 0;

	ppl->min_lat = INVALID_LATLON;
	ppl->min_lon = INVALID_LATLON;
	ppl->max_lat = INVALID_LATLON;
	ppl->max_lon = INVALID_LATLON;

	ppl->length = 0;

	return NE_OK;
}

void
data_polyline_free(data_util_state* pds, data_polyline* ppl)
{
	nsl_free(ppl->segments);
	ppl->segments = NULL;
	ppl->numsegments = 0;
}

static void
pt_fixed_to_double(double* plat, double* plon, int32 lat, int32 lon, double factor)
{
	*plat = lat / factor;
	*plon = lon / factor;
}

static int32
decode_int32(char* data)
{
	int32 d;

	((char*)&d)[0] = data[0];
	((char*)&d)[1] = data[1];
	((char*)&d)[2] = data[2];
	((char*)&d)[3] = data[3];

	return nsl_htonl(d);
}

static int16
decode_int16(char* data)
{
	int16 d;

	((char*)&d)[0] = data[0];
	((char*)&d)[1] = data[1];

	return nsl_htons(d);
}

static NB_Error
decode_polyline_data(data_util_state* pds, char* data, size_t data_size, pl_segment** pptrip, uint32* pnumpts,
					 double* pmin_lat, double* pmin_lon, double* pmax_lat, double* pmax_lon, double* plen)
{
	NB_Error err = NE_OK;
	int16 width = 0;
	uint32 n = 0;
	double lat1, lat2, lon1, lon2;
	double len;
	boolean calcbb = FALSE;

	lat1 = lat2 = lon1 = lon2 = 0.0;

	width = data[data_size-1] >> 4;  // we should fix the format so we don't need this unnecessary shift

	if (width != 2 && width != 4)
		return NE_INVAL;

	*pnumpts = 1 /* initial point */ + (((uint32)data_size - 1 /* width */ - 8 /* initial point */) / (width*2));

	*pptrip = nsl_malloc(sizeof(pl_segment) * (*pnumpts));

	if ((*pptrip) == NULL) {
		err = NE_NOMEM;
		goto errexit;
	}

	(*pptrip)[0].lat = decode_int32(data);
	(*pptrip)[0].lon = decode_int32(data+4);
	(*pptrip)[0].len = 0;
	(*pptrip)[0].heading = POLYLINE_INVALID_HEADING;

	if (pmin_lat != NULL && pmax_lat != NULL && pmin_lon != NULL && pmax_lon != NULL && plen != NULL)
		calcbb = TRUE;

	if (calcbb) {

		pt_fixed_to_double(&lat1, &lon1, (*pptrip)[0].lat, (*pptrip)[0].lon, POLYLINE_LL_SCALE_FACTOR);

		*pmin_lat = lat1;
		*pmax_lat = lat1;
		*pmin_lon = lon1;
		*pmax_lon = lon1;
	}

	if (plen != NULL)
		*plen = 0;

	for (n = 1; n < (*pnumpts); n++) {

		if (width == 2) {

			(*pptrip)[n].lat = (*pptrip)[n-1].lat + decode_int16(data+8+(n-1)*4);
			(*pptrip)[n].lon = (*pptrip)[n-1].lon + decode_int16(data+8+(n-1)*4+2);
		}
		else if (width == 4) {

			(*pptrip)[n].lat = (*pptrip)[n-1].lat + decode_int32(data+8+(n-1)*8);
			(*pptrip)[n].lon = (*pptrip)[n-1].lon + decode_int32(data+8+(n-1)*8+4);
		}

		if (calcbb) {

			pt_fixed_to_double(&lat2, &lon2, (*pptrip)[n].lat, (*pptrip)[n].lon, POLYLINE_LL_SCALE_FACTOR);

			if (lat2 < *pmin_lat)
				*pmin_lat = lat2;
			if (lat2 > *pmax_lat)
				*pmax_lat = lat2;
			if (lon2 < *pmin_lon)
				*pmin_lon = lon2;
			if (lon2 > *pmax_lon)
				*pmax_lon = lon2;
		}

		(*pptrip)[n].len = 0;
		(*pptrip)[n].heading = POLYLINE_INVALID_HEADING;

		if (calcbb) {

			len = NB_SpatialGetLineOfSightDistance(lat1, lon1, lat2, lon2, &(*pptrip)[n-1].heading);

			if (plen != NULL)
				*plen += len;

			(*pptrip)[n-1].len = (int32) (POLYLINE_LEN_SCALE_FACTOR * len);
		}
		else
			(*pptrip)[n-1].len = -1;

		if (calcbb) {

			lat1 = lat2;
			lon1 = lon2;
		}
	}

errexit:

	if (err != NE_OK) {

		if ((*pptrip) != NULL) {

			nsl_free(*pptrip);
			*pptrip  = NULL;
			*pnumpts = 0;
		}
	}

	return err;
}

NB_Error
data_polyline_from_tps_attr(data_util_state* pds, data_polyline* ps, tpselt te, const char* name, boolean calcbounds)
{
	char*	data;
	size_t	size;

	if (te == NULL || name == NULL)
		return NE_INVAL;

	if (!te_getattr(te, name, &data, &size))
		return NE_INVAL;

	return decode_polyline_data(pds, data, size, &ps->segments, &ps->numsegments,
		calcbounds ? &ps->min_lat : NULL, calcbounds ? &ps->min_lon : NULL,
		calcbounds ? &ps->max_lat : NULL, calcbounds ? &ps->max_lon : NULL,
		calcbounds ? &ps->length : NULL);
}

NB_Error
data_polyline_copy(data_util_state* pds, data_polyline* ppl_dest, data_polyline* ppl_src)
{
	data_polyline_free(pds, ppl_dest);
	data_polyline_init(pds, ppl_dest);

	if (ppl_src->numsegments == 0 && ppl_src->segments == NULL) {

		ppl_dest->numsegments = 0;
		ppl_dest->segments = NULL;
	}
	else {

		ppl_dest->numsegments = ppl_src->numsegments;

		ppl_dest->segments = nsl_malloc(ppl_dest->numsegments * sizeof(pl_segment));

		if (ppl_dest->segments == NULL) {

			ppl_dest->numsegments = 0;
			return NE_NOMEM;
		}

		nsl_memcpy(ppl_dest->segments, ppl_src->segments, ppl_dest->numsegments * sizeof(pl_segment));
	}

	ppl_dest->min_lat = ppl_src->min_lat;
	ppl_dest->min_lon = ppl_src->min_lon;
	ppl_dest->max_lat = ppl_src->max_lat;
	ppl_dest->max_lon = ppl_src->max_lon;
	ppl_dest->length  = ppl_src->length;

	return NE_OK;
}

uint32
data_polyline_len(data_util_state* pds, data_polyline* ppl)
{
    if (!ppl)
        return 0;
    return ppl->numsegments;
}

double
data_polyline_dist(data_util_state* pds, data_polyline* ppl)
{
	double len = 0.0;
	uint32 n;

	for (n=0;n<ppl->numsegments;n++) {

		len += ppl->segments[n].len / POLYLINE_LEN_SCALE_FACTOR;
	}

	return len;
}

NB_Error
data_polyline_get(data_util_state* pds, data_polyline* ppl, uint32 index, double* plat, double* plon, double* plen, double* pheading)
{
	if (index >= ppl->numsegments) {

		if (plon)
			*plon = INVALID_LATLON;
		if (plat)
			*plat = INVALID_LATLON;
		if (plen)
			*plen = 0;
		if (pheading)
			*pheading = POLYLINE_INVALID_HEADING;

		return NE_INVAL;
	}

	if (plat != NULL && plon != NULL)
		pt_fixed_to_double(plat, plon, ppl->segments[index].lat, ppl->segments[index].lon, POLYLINE_LL_SCALE_FACTOR);

	if (plen)
		*plen = ppl->segments[index].len / POLYLINE_LEN_SCALE_FACTOR;

	if (pheading)
		*pheading = ppl->segments[index].heading;

	return NE_OK;
}

NB_Error
data_polyline_enum(data_util_state* pds, data_polyline* ppl, data_polyline_enum_cb cb, void* cbuser)
{
	uint32 n;
	double lat1;
	double lon1;
	double lat2;
	double lon2;

	for (n=1;n<ppl->numsegments;n++) {

		pt_fixed_to_double(&lat1, &lon1, ppl->segments[n-1].lat, ppl->segments[n-1].lon, POLYLINE_LL_SCALE_FACTOR);
		pt_fixed_to_double(&lat2, &lon2, ppl->segments[n].lat, ppl->segments[n].lon, POLYLINE_LL_SCALE_FACTOR);

		if (cb != NULL)
			cb(cbuser,lat1,lon1,lat2,lon2);
	}

	return NE_OK;
}

NB_Error
data_polyline_enum_rev(data_util_state* pds, data_polyline* ppl, data_polyline_enum_cb cb, void* cbuser)
{
	uint32 n;
	double lat1;
	double lon1;
	double lat2;
	double lon2;

	for (n=ppl->numsegments-1;n>=1;n--) {

		pt_fixed_to_double(&lat1, &lon1, ppl->segments[n].lat, ppl->segments[n].lon, POLYLINE_LL_SCALE_FACTOR);
		pt_fixed_to_double(&lat2, &lon2, ppl->segments[n-1].lat, ppl->segments[n-1].lon, POLYLINE_LL_SCALE_FACTOR);

		if (cb != NULL)
			cb(cbuser,lat1,lon1,lat2,lon2);
	}

	return NE_OK;
}

NB_Error
data_polyline_enum_mercator(data_util_state* pds, data_polyline* ppl, data_polyline_enum_mercator_cb cb, void* cbuser)
{
	uint32 n;
	double lat;
	double lon;

	double mx1;
	double my1;
	double mx2;
	double my2;

	pt_fixed_to_double(&lat, &lon, ppl->segments[0].lat, ppl->segments[0].lon, POLYLINE_LL_SCALE_FACTOR);

	NB_SpatialConvertLatLongToMercator(lat, lon, &mx1, &my1);

	for (n=1;n<ppl->numsegments;n++) {

		pt_fixed_to_double(&lat, &lon, ppl->segments[n].lat, ppl->segments[n].lon, POLYLINE_LL_SCALE_FACTOR);
		NB_SpatialConvertLatLongToMercator(lat, lon, &mx2, &my2);

		if (cb != NULL)
			cb(cbuser, mx1, my1, mx2, my2);

		mx1 = mx2;
		my1 = my2;
	}

	return NE_OK;
}

boolean
data_polyline_in_bounds(data_util_state* pds, data_polyline* ppl, double lat, double lon, double buffer)
{
	double minlatb = ppl->min_lat - buffer;
	double minlonb = ppl->min_lon - buffer;
	double maxlatb = ppl->max_lat + buffer;
	double maxlonb = ppl->max_lon + buffer;

	return (boolean) (lat > minlatb && lat < maxlatb && lon > minlonb && lon < maxlonb);
}

boolean
data_polyline_in_rect(data_util_state* pds, data_polyline* ppl,
	double lat1, double lon1, double lat2, double lon2, double buffer)
{
	double minlatb = ppl->min_lat - buffer;
	double minlonb = ppl->min_lon - buffer;
	double maxlatb = ppl->max_lat + buffer;
	double maxlonb = ppl->max_lon + buffer;

	double minlat;
	double maxlat;
	double minlon;
	double maxlon;

	if (lat1 < lat2) {

		minlat = lat1 - buffer;
		maxlat = lat2 + buffer;
	}
	else {

		minlat = lat2 - buffer;
		maxlat = lat1 + buffer;
	}

	if (lon1 < lon2) {

		minlon = lon1 - buffer;
		maxlon = lon2 + buffer;
	}
	else {

		minlon = lon2 - buffer;
		maxlon = lon1 + buffer;
	}

	return (boolean) (minlat < maxlatb && minlatb < maxlat && minlon < maxlonb && minlonb < maxlon);
}
