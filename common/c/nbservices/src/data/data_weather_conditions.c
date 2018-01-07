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
 * data_weather_condtions.h: created 2007/10/03 by Sandeep Fatangare.
 */


#include "data_weather_conditions.h"

NB_Error
data_weather_conditions_init(data_util_state* pds, data_weather_conditions* pft)
{
    NB_Error err = NE_OK;

    pft->temp=0.0;
    pft->dewpt=0.0;
    pft->rel_humidity=0;
    pft->wind_speed=0.0;
    pft->wind_dir=0;
    pft->wind_gust=0.0;
    pft->pressure=0.0;
    pft->condition_code=0;
    err = data_string_init(pds, &pft->condition);
    err = err ? err : data_string_init(pds, &pft->sky);
    pft->ceil=0.0;
    pft->visibility=0;
    pft->heat_index=0.0;
    pft->wind_chill=0.0;
    pft->snow_depth=0.0;
    pft->max_temp_24hr=0.0;
    pft->min_temp_6hr=0.0;
    pft->min_temp_24hr=0.0;
    pft->precipitation_3hr=0.0;
    pft->precipitation_6hr=0.0;
    pft->precipitation_24hr=0.0;
    pft->update_time=0;
    pft->utc_offset =0;
    return err;
}

void
data_weather_conditions_free(data_util_state* pds, data_weather_conditions* pft)
{
    pft->temp=0.0;
    pft->dewpt=0.0;
    pft->rel_humidity=0;
    pft->wind_speed=0.0;
    pft->wind_dir=0;
    pft->wind_gust=0.0;
    pft->pressure=0.0;
    pft->condition_code=0;
    data_string_free(pds, &pft->condition);
    data_string_free(pds, &pft->sky);
    pft->ceil=0.0;
    pft->visibility=0;
    pft->heat_index=0.0;
    pft->wind_chill=0.0;
    pft->snow_depth=0.0;
    pft->max_temp_24hr=0.0;
    pft->min_temp_6hr=0.0;
    pft->min_temp_24hr=0.0;
    pft->precipitation_3hr=0.0;
    pft->precipitation_6hr=0.0;
    pft->precipitation_24hr=0.0;
    pft->update_time=0;
    pft->utc_offset=0;
}

NB_Error
data_weather_conditions_from_tps(data_util_state* pds, data_weather_conditions* pft, tpselt te)
{
    NB_Error err = NE_OK;
    data_string    utc_offset;

    if (te == NULL || pds == NULL || pft == NULL)
    {
        return NE_INVAL;
    }

    data_weather_conditions_free(pds, pft);
    err = data_string_init(pds, &utc_offset);

    err = err ? err : data_weather_conditions_init(pds, pft);

    if (err != NE_OK)
        return err;

    te_getattrf(te, "temperature",&pft->temp);
    te_getattrf(te, "dew-point",&pft->dewpt);
    pft->rel_humidity = te_getattru(te,"rel-humidity");
    te_getattrf(te, "wind-speed",&pft->wind_speed);
    pft->wind_dir = te_getattru(te,"wind-direction");
    te_getattrf(te, "wind-gust",&pft->wind_gust);
    te_getattrf(te, "pressure",&pft->pressure);
    pft->condition_code = te_getattru(te,"condition-code");
    err = err ? err : data_string_from_tps_attr(pds, &pft->condition, te, "condition");
    err = err ? err : data_string_from_tps_attr(pds, &pft->sky, te, "sky"); 
    te_getattrf(te, "ceiling",&pft->ceil);
    pft->visibility = te_getattru(te,"visibility");
    te_getattrf(te, "heat-index",&pft->heat_index);
    te_getattrf(te, "wind-chill",&pft->wind_chill);
    te_getattrf(te, "snow-depth",&pft->snow_depth);
    te_getattrf(te, "max-temp-6hr",&pft->max_temp_6hr);
    te_getattrf(te, "max-temp-24hr",&pft->max_temp_24hr);
    te_getattrf(te, "min-temp-6hr",&pft->min_temp_6hr);
    te_getattrf(te, "min-temp-24hr",&pft->min_temp_24hr);
    te_getattrf(te, "precipitation-3hr",&pft->precipitation_3hr);
    te_getattrf(te, "precipitation-6hr",&pft->precipitation_6hr);
    te_getattrf(te, "precipitation-24hr",&pft->precipitation_24hr);
    pft->update_time = te_getattru(te,"update-time");
    err = err ? err : data_string_from_tps_attr(pds, &utc_offset, te, "utc-offset");
    if(utc_offset)
        pft->utc_offset = nsl_atoi(utc_offset);

    if (err != NE_OK)
        data_weather_conditions_free(pds, pft);

    data_string_free(pds, &utc_offset);

    return err;
}

boolean
data_weather_conditions_equal(data_util_state* pds, data_weather_conditions* pft1, data_weather_conditions* pft2)
{
    return (boolean) ((pft1->temp == pft2->temp)&&
                      (pft1->dewpt == pft2->dewpt)&&
                      (pft1->rel_humidity == pft2->rel_humidity)&&
                      (pft1->wind_speed == pft2->wind_speed)&&
                      (pft1->wind_dir == pft2->wind_dir)&&
                      (pft1->wind_gust == pft2->wind_gust)&&
                      (pft1->pressure == pft2->pressure)&&
                      (pft1->condition_code == pft2->condition_code)&&
                       data_string_equal(pds, &pft1->condition, &pft2->condition) &&
                       data_string_equal(pds, &pft1->sky, &pft2->sky)&&
                      (pft1->ceil == pft2->ceil)&&
                      (pft1->visibility == pft2->visibility)&&
                      (pft1->heat_index == pft2->heat_index)&&
                      (pft1->wind_chill == pft2->wind_chill)&&
                      (pft1->snow_depth == pft2->snow_depth)&&
                      (pft1->max_temp_6hr == pft2->max_temp_6hr)&&
                      (pft1->max_temp_24hr == pft2->max_temp_24hr)&&
                      (pft1->min_temp_6hr == pft2->min_temp_6hr)&&
                      (pft1->min_temp_24hr == pft2->min_temp_24hr)&&
                      (pft1->precipitation_3hr == pft2->precipitation_3hr)&&
                      (pft1->precipitation_6hr == pft2->precipitation_6hr)&&
                      (pft1->precipitation_24hr == pft2->precipitation_24hr)&&
                      (pft1->update_time == pft2->update_time)
                      );
}

NB_Error
data_weather_conditions_copy(data_util_state* pds, data_weather_conditions* pft_dest, data_weather_conditions* pft_src)
{
    NB_Error err = NE_OK;

    data_weather_conditions_free(pds, pft_dest);

    err = data_weather_conditions_init(pds, pft_dest);

    pft_dest->temp = pft_src->temp;
    pft_dest->dewpt = pft_src->dewpt;
    pft_dest->rel_humidity = pft_src->rel_humidity;
    pft_dest->wind_speed = pft_src->wind_speed;
    pft_dest->wind_dir = pft_src->wind_dir;
    pft_dest->wind_gust = pft_src->wind_gust;
    pft_dest->pressure = pft_src->pressure;
    pft_dest->condition_code = pft_src->condition_code;
    err = err ? err : data_string_copy(pds, &pft_dest->condition, &pft_src->condition);
    err = err ? err : data_string_copy(pds, &pft_dest->sky, &pft_src->sky);
    pft_dest->ceil = pft_src->ceil;
    pft_dest->visibility = pft_src->visibility;
    pft_dest->heat_index = pft_src->heat_index;
    pft_dest->wind_chill = pft_src->wind_chill;
    pft_dest->snow_depth = pft_src->snow_depth;
    pft_dest->max_temp_24hr = pft_src->max_temp_24hr;
    pft_dest->min_temp_6hr = pft_src->min_temp_6hr;
    pft_dest->min_temp_24hr = pft_src->min_temp_24hr;
    pft_dest->precipitation_3hr = pft_src->precipitation_3hr;
    pft_dest->precipitation_6hr = pft_src->precipitation_6hr;
    pft_dest->precipitation_24hr = pft_src->precipitation_24hr;
    pft_dest->update_time = pft_src->update_time;

    return err;
}

uint32 data_weather_conditions_get_tps_size(data_util_state* pds, data_weather_conditions* pp)
{
    uint32 size = 0;

    size += sizeof(pp->temp);
    size += sizeof(pp->dewpt);
    size += sizeof(pp->rel_humidity);
    size += sizeof(pp->wind_speed);
    size += sizeof(pp->wind_dir);
    size += sizeof(pp->wind_gust);
    size += sizeof(pp->pressure);
    size += sizeof(pp->condition_code);
    size += data_string_get_tps_size(pds, &pp->condition);
    size += data_string_get_tps_size(pds, &pp->sky);
    size += sizeof(pp->ceil);
    size += sizeof(pp->visibility);
    size += sizeof(pp->heat_index);
    size += sizeof(pp->wind_chill);
    size += sizeof(pp->snow_depth);
    size += sizeof(pp->max_temp_6hr);
    size += sizeof(pp->max_temp_24hr);
    size += sizeof(pp->min_temp_6hr);
    size += sizeof(pp->min_temp_24hr);
    size += sizeof(pp->precipitation_3hr);
    size += sizeof(pp->precipitation_6hr);
    size += sizeof(pp->precipitation_24hr);
    size += sizeof(pp->update_time);
    size += sizeof(pp->utc_offset);

    return size;
}

void data_weather_conditions_to_buf(data_util_state* pds, data_weather_conditions* pp, struct dynbuf* pdb)
{
    dbufcat(pdb, (byte*) &pp->temp, sizeof(pp->temp));
    dbufcat(pdb, (byte*) &pp->dewpt, sizeof(pp->dewpt));
    dbufcat(pdb, (byte*) &pp->rel_humidity, sizeof(pp->rel_humidity));
    dbufcat(pdb, (byte*) &pp->wind_speed, sizeof(pp->wind_speed));
    dbufcat(pdb, (byte*) &pp->wind_dir, sizeof(pp->wind_dir));
    dbufcat(pdb, (byte*) &pp->wind_gust, sizeof(pp->wind_gust));
    dbufcat(pdb, (byte*) &pp->pressure, sizeof(pp->pressure));
    dbufcat(pdb, (byte*) &pp->condition_code, sizeof(pp->condition_code));
    data_string_to_buf(pds, &pp->condition, pdb);
    data_string_to_buf(pds, &pp->sky, pdb);
    dbufcat(pdb, (byte*) &pp->ceil, sizeof(pp->ceil));
    dbufcat(pdb, (byte*) &pp->visibility, sizeof(pp->visibility));
    dbufcat(pdb, (byte*) &pp->heat_index, sizeof(pp->heat_index));
    dbufcat(pdb, (byte*) &pp->wind_chill, sizeof(pp->wind_chill));
    dbufcat(pdb, (byte*) &pp->snow_depth, sizeof(pp->snow_depth));
    dbufcat(pdb, (byte*) &pp->max_temp_6hr, sizeof(pp->max_temp_6hr));
    dbufcat(pdb, (byte*) &pp->max_temp_24hr, sizeof(pp->max_temp_24hr));
    dbufcat(pdb, (byte*) &pp->min_temp_6hr, sizeof(pp->min_temp_6hr));
    dbufcat(pdb, (byte*) &pp->min_temp_24hr, sizeof(pp->min_temp_24hr));
    dbufcat(pdb, (byte*) &pp->precipitation_3hr, sizeof(pp->precipitation_3hr));
    dbufcat(pdb, (byte*) &pp->precipitation_6hr, sizeof(pp->precipitation_6hr));
    dbufcat(pdb, (byte*) &pp->precipitation_24hr, sizeof(pp->precipitation_24hr));
    dbufcat(pdb, (byte*) &pp->update_time, sizeof(pp->update_time));
    dbufcat(pdb, (byte*) &pp->utc_offset, sizeof(pp->utc_offset));
}

NB_Error data_weather_conditions_from_binary(data_util_state* pds, data_weather_conditions* pp, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_double_from_binary(pds, &pp->temp, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->dewpt, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->rel_humidity, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->wind_speed, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->wind_dir, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->wind_gust, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->pressure, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->condition_code, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->condition, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pp->sky, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->ceil, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->visibility, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->heat_index, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->wind_chill, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->snow_depth, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->max_temp_6hr, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->max_temp_24hr, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->min_temp_6hr, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->min_temp_24hr, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->precipitation_3hr, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->precipitation_6hr, pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &pp->precipitation_24hr, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pp->update_time, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, (uint32*)&pp->utc_offset, pdata, pdatalen);

    return err;
}
