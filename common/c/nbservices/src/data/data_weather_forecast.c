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
 * data_weather_forecast.h: created 2007/10/03 by Sandeep Fatangare.
 */

#include "data_weather_forecast.h"

NB_Error	
data_weather_forecast_init(data_util_state* pds, data_weather_forecast* pft)
{
	NB_Error err = NE_OK;

    DATA_MEM_ZERO(pft, data_weather_forecast);

	pft->date						=	0;
	pft->high_temp					=	0.0;
	pft->low_temp					=	0.0;	
	pft->wind_dir					=	0;
	pft->wind_speed					=	0.0;
	pft->precipitation_probability	=	0;
	pft->rel_humidity				=	0;
	pft->condition_code				=	0;
	err = err ? err : data_string_init(pds, &pft->condition);	
	pft->uv_index					=	0;
	err = err ? err : data_string_init(pds, &pft->uv_desc);	
	err = err ? err : data_string_init(pds, &pft->sunrise);	
	err = err ? err : data_string_init(pds, &pft->sunset);	
	err = err ? err : data_string_init(pds, &pft->moonrise);	
	err = err ? err : data_string_init(pds, &pft->moonset);	
	pft->moon_phase					=	0;
	pft->update_time				=	0;
	pft->utc_offset                 =   0;

	return err;
}

void		
data_weather_forecast_free(data_util_state* pds, data_weather_forecast* pft)
{
	pft->date						=	0;
	pft->high_temp					=	0.0;
	pft->low_temp					=	0.0;	
	pft->wind_dir					=	0;
	pft->wind_speed					=	0.0;
	pft->precipitation_probability	=	0;
	pft->rel_humidity				=	0;
	pft->condition_code				=	0;
	data_string_free(pds, &pft->condition);
	pft->uv_index					=	0;
	data_string_free(pds, &pft->uv_desc);
	data_string_free(pds, &pft->sunrise);
	data_string_free(pds, &pft->sunset);
	data_string_free(pds, &pft->moonrise);
	data_string_free(pds, &pft->moonset);
	pft->moon_phase					=	0;
	pft->update_time					=	0;
	pft->utc_offset                  =   0;
}

NB_Error	
data_weather_forecast_from_tps(data_util_state* pds, data_weather_forecast* pft, tpselt te)
{
	NB_Error err = NE_OK;
	data_string	   utc_offset;

	if (te == NULL || pds == NULL || pft == NULL)
	{
		return NE_INVAL;
	}

	data_weather_forecast_free(pds, pft);
	err = data_string_init(pds, &utc_offset);

    err = err ? err : data_weather_forecast_init(pds, pft);

	if (err != NE_OK)
		return err;

	pft->date = te_getattru(te,"date");
	te_getattrf(te, "high-temperature",&pft->high_temp);
	te_getattrf(te, "low-temperature",&pft->low_temp);		
	pft->wind_dir= te_getattru(te,"wind-direction");
	te_getattrf(te, "wind-speed",&pft->wind_speed);
	pft->precipitation_probability = te_getattru(te,"precipitation-probability");
	pft->rel_humidity = te_getattru(te,"relative-humidity");
	pft->condition_code = te_getattru(te,"condition-code");
	err = err ? err : data_string_from_tps_attr(pds, &pft->condition, te, "condition");
	pft->uv_index = te_getattru(te,"uv-index");
	err = err ? err : data_string_from_tps_attr(pds, &pft->uv_desc, te, "uv-description");
	err = err ? err : data_string_from_tps_attr(pds, &pft->sunrise, te, "sunrise");
	err = err ? err : data_string_from_tps_attr(pds, &pft->sunset, te, "sunset");
	err = err ? err : data_string_from_tps_attr(pds, &pft->moonrise, te, "moonrise");
	err = err ? err : data_string_from_tps_attr(pds, &pft->moonset, te, "moonset");
	pft->moon_phase = te_getattru(te,"moon-phase");
//	pp->day_number = te_getattru(te,"day-number");
	pft->update_time = te_getattru(te,"update-time");
	err = err ? err : data_string_from_tps_attr(pds, &utc_offset, te, "utc-offset");
	if(utc_offset)
		pft->utc_offset = nsl_atoi(utc_offset);


	if (err != NE_OK)
		data_weather_forecast_free(pds, pft);

	data_string_free(pds, &utc_offset);
	return err;;
}

boolean		
data_weather_forecast_equal(data_util_state* pds, data_weather_forecast* pp1, data_weather_forecast* pp2)
{
	return (boolean)( (pp1->date == pp2->date)&&
					  (pp1->high_temp == pp2->high_temp) &&
					  (pp1->low_temp == pp2->low_temp) &&
					  (pp1->wind_dir == pp2->wind_dir) &&					   
					  (pp1->wind_speed == pp2->wind_speed) &&
					  (pp1->precipitation_probability == pp2->precipitation_probability) &&
					  (pp1->rel_humidity == pp2->rel_humidity) &&
					  (pp1->condition_code == pp2->condition_code) &&	
					   data_string_equal(pds, &pp1->condition, &pp2->condition) &&	
					  (pp1->uv_index == pp2->uv_index) &&
					   data_string_equal(pds, &pp1->uv_desc, &pp2->uv_desc) &&	
					   data_string_equal(pds, &pp1->sunrise, &pp2->sunrise) &&	
					   data_string_equal(pds, &pp1->sunset, &pp2->sunset) &&	
					   data_string_equal(pds, &pp1->moonrise, &pp2->moonrise) &&	
					   data_string_equal(pds, &pp1->moonset, &pp2->moonset) &&
					  (pp1->moon_phase == pp2->moon_phase) &&
					  (pp1->update_time == pp2->update_time) 
					);
}

NB_Error  
data_weather_forecast_copy(data_util_state* pds, data_weather_forecast* pp_dest, data_weather_forecast* pp_src)
{
	NB_Error err = NE_OK;

	pp_dest->date						=	pp_src->date;
	pp_dest->high_temp					=	pp_src->high_temp;
	pp_dest->low_temp					=	pp_src->low_temp;		
	pp_dest->wind_dir					=	pp_src->wind_dir;
	pp_dest->wind_speed					=	pp_src->wind_speed	;
	pp_dest->precipitation_probability	=	pp_src->precipitation_probability;
	pp_dest->rel_humidity				=	pp_src->rel_humidity;
	pp_dest->condition_code				=	pp_src->condition_code;
	err = err ? err : data_string_copy(pds, &pp_dest->condition, &pp_src->condition);
	pp_dest->uv_index					=	pp_src->uv_index;
	err = err ? err : data_string_copy(pds, &pp_dest->uv_desc, &pp_src->uv_desc);
	err = err ? err : data_string_copy(pds, &pp_dest->sunrise, &pp_src->sunrise);
	err = err ? err : data_string_copy(pds, &pp_dest->sunset, &pp_src->sunset);	
	err = err ? err : data_string_copy(pds, &pp_dest->moonrise, &pp_src->moonrise);
	err = err ? err : data_string_copy(pds, &pp_dest->moonset, &pp_src->moonset);
	pp_dest->moon_phase					=	pp_src->moon_phase;
	//pp_dest->day_number				=	pp_src->day_number;
	pp_dest->update_time				=	pp_src->update_time;

	return err;
}

uint32   data_weather_forecast_get_tps_size(data_util_state* pds, data_weather_forecast* pp)
{
    uint32 size = 0;

    size += sizeof(pp->date);
    size += sizeof(pp->high_temp);
    size += sizeof(pp->low_temp);
    size += sizeof(pp->wind_dir);
    size += sizeof(pp->wind_speed);
    size += sizeof(pp->precipitation_probability);
    size += sizeof(pp->rel_humidity);
    size += sizeof(pp->condition_code);
    size += data_string_get_tps_size(pds, &pp->condition);
    size += sizeof(pp->uv_index);
    size += data_string_get_tps_size(pds, &pp->uv_desc);
    size += data_string_get_tps_size(pds, &pp->sunrise);
    size += data_string_get_tps_size(pds, &pp->sunset);
    size += data_string_get_tps_size(pds, &pp->moonrise);
    size += data_string_get_tps_size(pds, &pp->moonset);
    size += sizeof(pp->moon_phase);
    size += sizeof(pp->update_time);
    size += sizeof(pp->utc_offset);

    return size;
}

void     data_weather_forecast_to_buf(data_util_state* pds, data_weather_forecast* pp, struct dynbuf* pdb)
{
    dbufcat(pdb, (const byte*)&pp->date, sizeof(pp->date));
    dbufcat(pdb, (const byte*)&pp->high_temp, sizeof(pp->high_temp));
    dbufcat(pdb, (const byte*)&pp->low_temp, sizeof(pp->low_temp));
    dbufcat(pdb, (const byte*)&pp->wind_dir, sizeof(pp->wind_dir));
    dbufcat(pdb, (const byte*)&pp->wind_speed, sizeof(pp->wind_speed));
    dbufcat(pdb, (const byte*)&pp->precipitation_probability, sizeof(pp->precipitation_probability));
    dbufcat(pdb, (const byte*)&pp->rel_humidity, sizeof(pp->rel_humidity));
    dbufcat(pdb, (const byte*)&pp->condition_code, sizeof(pp->condition_code));
    data_string_to_buf(pds, &pp->condition, pdb);
    dbufcat(pdb, (const byte*)&pp->uv_index, sizeof(pp->uv_index));
    data_string_to_buf(pds, &pp->uv_desc, pdb);
    data_string_to_buf(pds, &pp->sunrise, pdb);
    data_string_to_buf(pds, &pp->sunset, pdb);
    data_string_to_buf(pds, &pp->moonrise, pdb);
    data_string_to_buf(pds, &pp->moonset, pdb);
    dbufcat(pdb, (const byte*)&pp->moon_phase, sizeof(pp->moon_phase));
    dbufcat(pdb, (const byte*)&pp->update_time, sizeof(pp->update_time));
    dbufcat(pdb, (const byte*)&pp->utc_offset, sizeof(pp->utc_offset));
}


NB_Error data_weather_forecast_from_binary(data_util_state* pds, data_weather_forecast* pp, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_uint32_from_binary(pds, &pp->date, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->high_temp, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->low_temp, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->wind_dir, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->wind_speed, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->precipitation_probability, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->rel_humidity, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->condition_code, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->condition, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->uv_index, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->uv_desc, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->sunrise, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->sunset, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->moonrise, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->moonset, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->moon_phase, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->update_time, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, (uint32*)&pp->utc_offset, pdata, pdatalen);

    return err;
}


