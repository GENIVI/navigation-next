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
* data_traffic_incident.c: created 2007/11/27 by Huey Ly
*/

#include "data_traffic_incident.h"

NB_Error
data_traffic_incident_init(data_util_state* pds, data_traffic_incident* pft)
{
	NB_Error err = NE_OK;
	
	pft->criticality=0;
	pft->type = 0;
	pft->end_time = 0;
	pft->entry_time = 0;
	pft->start_time = 0;
	pft->utc_offset = 0;

	err = data_string_init(pds, &pft->road);
	err = err ? err : data_string_init(pds, &pft->description);

	pft->distance_from_start = 0.0;
	pft->nman_closest = 0xFFFFFFFF;
	pft->nseg_closest = 0xFFFFFFFF;
	pft->new_item = FALSE;

	return err;
}

void		
data_traffic_incident_free(data_util_state* pds, data_traffic_incident* pft)
{
	pft->criticality=0;
	pft->type = 0;
	pft->end_time = 0;
	pft->entry_time = 0;
	pft->start_time = 0;
	pft->utc_offset = 0;
	data_string_free(pds, &pft->road);
	data_string_free(pds, &pft->description);	
}

NB_Error	
data_traffic_incident_from_tps(data_util_state* pds, data_traffic_incident* pft, tpselt te)
{
	NB_Error err = NE_OK;
	data_string	   utc_offset;

	if (te == NULL || pds == NULL || pft == NULL)
	{
		return NE_INVAL;
	}

	data_traffic_incident_free(pds, pft);
	err = data_string_init(pds, &utc_offset);

    err = err ? err : data_traffic_incident_init(pds, pft);

	if (err != NE_OK)
		return err;


	pft->criticality= te_getattru(te,"criticality");
	pft->type =te_getattru(te,"type");
	pft->end_time = te_getattru(te,"end-time");
	pft->entry_time = te_getattru(te,"entry-time");
	pft->start_time = te_getattru(te,"start-time");

	err = err ? err : data_string_from_tps_attr(pds, &pft->road, te, "road");
	err = err ? err : data_string_from_tps_attr(pds, &pft->description, te, "description");
	err = err ? err : data_string_from_tps_attr(pds, &utc_offset, te, "utc-offset");
	if(utc_offset)
		pft->utc_offset = nsl_atoi(utc_offset);

	if (err != NE_OK)
		data_traffic_incident_free(pds, pft);

	data_string_free(pds, &utc_offset);

	return err;
}

boolean		
data_traffic_incident_equal(data_util_state* pds, data_traffic_incident* pft1, data_traffic_incident* pft2)
{
	return (boolean) ((pft1->type == pft2->type)&&
					  (pft1->criticality == pft2->criticality)&&
					  (pft1->end_time == pft2->end_time)&&
					  (pft1->entry_time == pft2->entry_time)&&
					  (pft1->start_time == pft2->start_time)&&
					   data_string_equal(pds, &pft1->road, &pft2->road) &&
					   data_string_equal(pds, &pft1->description, &pft2->description)
					  );
}

NB_Error
data_traffic_incident_copy(data_util_state* pds, data_traffic_incident* pft_dest, data_traffic_incident* pft_src)
{
	NB_Error err = NE_OK;

	data_traffic_incident_free(pds, pft_dest);
	err = data_traffic_incident_init(pds, pft_dest);
	pft_dest->type			=	pft_src->type;
	pft_dest->criticality	=	pft_src->criticality;
	pft_dest->end_time		=	pft_src->end_time;
	pft_dest->entry_time	=	pft_src->entry_time;
	pft_dest->start_time	=	pft_src->start_time;
    pft_dest->utc_offset    =   pft_src->utc_offset;
	err = err ? err : data_string_copy(pds, &pft_dest->road, &pft_src->road);
	err = err ? err : data_string_copy(pds, &pft_dest->description, &pft_src->description);

	pft_dest->distance_from_start = pft_src->distance_from_start;
	pft_dest->nman_closest = pft_src->nman_closest;
	pft_dest->nseg_closest = pft_src->nseg_closest;
	pft_dest->new_item = pft_src->new_item;

	return err;
}

uint32   data_traffic_incident_get_tps_size(data_util_state* pds, data_traffic_incident* pp)
{
    uint32 size = 0;

    size += sizeof(pp->type);
    size += sizeof(pp->criticality);
    size += sizeof(pp->entry_time);
    size += sizeof(pp->start_time);
    size += sizeof(pp->end_time);
    size += sizeof(pp->utc_offset);
    size += data_string_get_tps_size(pds, &pp->road);
    size += data_string_get_tps_size(pds, &pp->description);

    size += sizeof(pp->distance_from_start);
    size += sizeof(pp->nman_closest);
    size += sizeof(pp->nseg_closest);
    size += sizeof(pp->new_item);

    return size;
}

void     data_traffic_incident_to_buf(data_util_state* pds, data_traffic_incident* pp, struct dynbuf* pdb)
{
    dbufcat(pdb, (const byte*) &pp->type, sizeof(pp->type));
    dbufcat(pdb, (const byte*) &pp->criticality, sizeof(pp->criticality));
    dbufcat(pdb, (const byte*) &pp->entry_time, sizeof(pp->entry_time));
    dbufcat(pdb, (const byte*) &pp->start_time, sizeof(pp->start_time));
    dbufcat(pdb, (const byte*) &pp->end_time, sizeof(pp->end_time));
    dbufcat(pdb, (const byte*) &pp->utc_offset, sizeof(pp->utc_offset));
    data_string_to_buf(pds, &pp->road, pdb);
    data_string_to_buf(pds, &pp->description, pdb);

    dbufcat(pdb, (const byte*) &pp->distance_from_start, sizeof(pp->distance_from_start));
    dbufcat(pdb, (const byte*) &pp->nman_closest, sizeof(pp->nman_closest));
    dbufcat(pdb, (const byte*) &pp->nseg_closest, sizeof(pp->nseg_closest));
    dbufcat(pdb, (const byte*) &pp->new_item, sizeof(pp->new_item));
}

NB_Error data_traffic_incident_from_binary(data_util_state* pds, data_traffic_incident* pp, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_uint32_from_binary(pds, &pp->type, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->criticality, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->entry_time, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->start_time, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->end_time, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, (uint32*)&pp->utc_offset, pdata, pdatalen);

    err = err ? err : data_string_from_binary(pds, &pp->road, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->description, pdata, pdatalen);

    err = err ? err : data_double_from_binary(pds, &pp->distance_from_start, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->nman_closest, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->nseg_closest, pdata, pdatalen);
    err = err ? err : data_boolean_from_binary(pds, &pp->new_item, pdata, pdatalen);

    return err;
}
