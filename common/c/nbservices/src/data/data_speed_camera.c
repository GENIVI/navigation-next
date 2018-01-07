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
 * data_phone.c: created 2007/11/13 by Andreas Lundeen.
 */

#include "data_speed_camera.h"

static void
data_speed_camera_clear(data_util_state* pds, data_speed_camera* psc)
{
	psc->limit = 0.0;
	psc->heading = 0.0;
	psc->bi_directional = FALSE;

	psc->camera_segment.valid = FALSE;
	psc->camera_segment.on_route = FALSE;
	psc->camera_segment.segment = 0;
	psc->camera_segment.maneuver = 0;
}

NB_Error
data_speed_camera_init(data_util_state* pds, data_speed_camera* psc)
{
	data_speed_camera_clear(pds, psc);
	data_string_init(pds, &psc->description);
	data_string_init(pds, &psc->status);

	return NE_OK;
}

void		
data_speed_camera_free(data_util_state* pds, data_speed_camera* psc)
{
	data_string_free(pds, &psc->description);
	data_string_free(pds, &psc->status);
}

NB_Error	
data_speed_camera_from_tps(data_util_state* pds, data_speed_camera* psc, tpselt te)
{
	tpselt ce;
	NB_Error err = NE_OK;
	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_speed_camera_free(pds, psc);

	err = data_speed_camera_init(pds, psc);

	if (err != NE_OK)
		return err;

	ce = te_getchild(te, "camera-segment");
	if (ce != NULL)
	{
		psc->camera_segment.valid = TRUE;
		psc->camera_segment.on_route = (boolean)te_getattru(ce, "on-route");
		psc->camera_segment.maneuver = te_getattru(ce, "man-num");
		psc->camera_segment.segment = te_getattru(ce, "seg-num");
		if (psc->camera_segment.segment != 0)
		{
			psc->camera_segment.segment -= 1;
		}
		if (psc->camera_segment.maneuver != 0)
		{
			psc->camera_segment.maneuver -= 1;
		}
	}

	if (!err)
	{
		if (!te_getattrf(te, "limit", &psc->limit))
		{
			err = NE_NOMEM;
		}
	}
	if (!err)
	{
		if (!te_getattrf(te, "heading", &psc->heading))
		{
			err = NE_NOMEM;
		}
	}
	if (!err)
	{
		psc->bi_directional = (boolean)te_getattru(te, "bi-directional");
		err = data_string_from_tps_attr(pds, &psc->status, te, "status");
        err = err ? err : data_string_from_tps_attr(pds, &psc->description, te, "description");
	}


errexit:
	if (err != NE_OK)
		data_speed_camera_free(pds, psc);
	return err;
}

tpselt		
data_speed_camera_to_tps(data_util_state* pds, data_speed_camera* psc)
{
	tpselt te;

	te = te_new("speed-camera");
	
	if (te == NULL)
		goto errexit;

	if (!te_setattrf(te, "limit", psc->limit))
		goto errexit;

	if (!te_setattrf(te, "heading", psc->heading))
		goto errexit;

	if (!te_setattru(te, "bi-directional", (uint32)psc->bi_directional))
		goto errexit;
	
	if (!te_setattrc(te, "status", data_string_get(pds, &psc->status)))
		goto errexit;

	if (!te_setattrc(te, "description", data_string_get(pds, &psc->description)))
		goto errexit;

	return te;

errexit:

	te_dealloc(te);
	return NULL;
}

boolean		
data_speed_camera_equal(data_util_state* pds, data_speed_camera* psc1, data_speed_camera* psc2)
{ 
	return (boolean)((psc1->bi_directional == psc2->bi_directional) &&
			(psc1->heading == psc2->heading) &&
			(psc1->limit == psc2->limit) &&
			data_string_equal(pds, &psc1->status, &psc2->status) &&
			data_string_equal(pds, &psc1->description, &psc2->description) &&
			(psc1->camera_segment.valid == psc2->camera_segment.valid) &&
			(psc1->camera_segment.on_route == psc2->camera_segment.on_route) &&
			(psc1->camera_segment.maneuver == psc2->camera_segment.maneuver) &&
			(psc1->camera_segment.segment == psc2->camera_segment.segment));
}

NB_Error	
data_speed_camera_copy(data_util_state* pds, data_speed_camera* psc_dest, data_speed_camera* psc_src)
{
	NB_Error err = NE_OK;

	data_speed_camera_free(pds, psc_dest);
	err = data_speed_camera_init(pds, psc_dest);

	psc_dest->limit = psc_src->limit;
	psc_dest->heading = psc_src->heading;
	psc_dest->bi_directional = psc_src->bi_directional;
	psc_dest->camera_segment = psc_src->camera_segment;
	err = err ? err : data_string_copy(pds, &psc_dest->status, &psc_src->status);
	err = err ? err : data_string_copy(pds, &psc_dest->description, &psc_src->description);
	return err;
}

double 
data_speed_camera_get_speed_limit(data_speed_camera *psc)
{
	if (psc)
		return psc->limit;
	else
		return 0.0;
}

uint32   data_speed_camera_get_tps_size(data_util_state* pds, data_speed_camera* psc)
{
    uint32 size = 0;

    size += sizeof(psc->camera_segment.valid);
    size += sizeof(psc->camera_segment.on_route);
    size += sizeof(psc->camera_segment.maneuver);
    size += sizeof(psc->camera_segment.segment);
    size += sizeof(psc->camera_segment.tmp);

    size += sizeof(psc->limit);
    size += sizeof(psc->heading);
    size += data_string_get_tps_size(pds, &psc->status);
    size += sizeof(psc->bi_directional);
    size += data_string_get_tps_size(pds, &psc->description);

    return size;
}

void     data_speed_camera_to_buf(data_util_state* pds, data_speed_camera* psc, struct dynbuf* pdb)
{
    dbufcat(pdb, (const byte*)&psc->camera_segment.valid, sizeof(psc->camera_segment.valid));
    dbufcat(pdb, (const byte*)&psc->camera_segment.on_route, sizeof(psc->camera_segment.on_route));
    dbufcat(pdb, (const byte*)&psc->camera_segment.maneuver, sizeof(psc->camera_segment.maneuver));
    dbufcat(pdb, (const byte*)&psc->camera_segment.segment, sizeof(psc->camera_segment.segment));
    dbufcat(pdb, (const byte*)&psc->camera_segment.tmp, sizeof(psc->camera_segment.tmp));

    dbufcat(pdb, (const byte*)&psc->limit, sizeof(psc->limit));
    dbufcat(pdb, (const byte*)&psc->heading, sizeof(psc->heading));
    data_string_to_buf(pds, &psc->status, pdb);
    dbufcat(pdb, (const byte*)&psc->bi_directional, sizeof(psc->bi_directional));
    data_string_to_buf(pds, &psc->description, pdb);
}

NB_Error data_speed_camera_from_binary(data_util_state* pds, data_speed_camera* psc, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_boolean_from_binary(pds, &psc->camera_segment.valid, pdata, pdatalen);
    err = err ? err : data_boolean_from_binary(pds, &psc->camera_segment.on_route, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &psc->camera_segment.maneuver, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &psc->camera_segment.segment, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &psc->camera_segment.tmp, pdata, pdatalen);

    err = err ? err : data_double_from_binary(pds, &psc->limit, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &psc->heading, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &psc->status, pdata, pdatalen);
    err = err ? err : data_boolean_from_binary(pds, &psc->bi_directional, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &psc->description, pdata, pdatalen);

    return err;
}

