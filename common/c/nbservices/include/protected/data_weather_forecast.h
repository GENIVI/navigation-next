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

#ifndef DATA_WEATHER_FORECAST_H
#define DATA_WEATHER_FORECAST_H

#include "datautil.h"
#include "data_string.h"
#include "nbexp.h"

typedef struct data_weather_forecast_ {

	/* Child Elements */

	/* Attributes */
	uint32			date;
	double			high_temp;
	double			low_temp;
	uint32			wind_dir;
	double			wind_speed;
	uint32			precipitation_probability;
	uint32			rel_humidity;
	uint32			condition_code;
	data_string		condition;
	uint32			uv_index;
	data_string		uv_desc;
	data_string		sunrise;
	data_string		sunset;
	data_string		moonrise;
	data_string		moonset;
	uint32			moon_phase;	
	uint32			update_time;
	int32           utc_offset;

} data_weather_forecast;

NB_Error	data_weather_forecast_init(data_util_state* pds, data_weather_forecast* pp);
void		data_weather_forecast_free(data_util_state* pds, data_weather_forecast* pft);

NB_Error	data_weather_forecast_from_tps(data_util_state* pds, data_weather_forecast* pp, tpselt te);

boolean		data_weather_forecast_equal(data_util_state* pds, data_weather_forecast* pp1, data_weather_forecast* pp2);
NB_Error	data_weather_forecast_copy(data_util_state* pds, data_weather_forecast* pp_dest, data_weather_forecast* pep_src);

uint32   data_weather_forecast_get_tps_size(data_util_state* pds, data_weather_forecast* pp);
void     data_weather_forecast_to_buf(data_util_state* pds, data_weather_forecast* pp, struct dynbuf* pdb);
NB_Error data_weather_forecast_from_binary(data_util_state* pds, data_weather_forecast* pp, byte** pdata, size_t* pdatalen);

#endif

