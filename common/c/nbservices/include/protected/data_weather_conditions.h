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

#ifndef DATA_WEATHER_CONDITIONS_H
#define DATA_WEATHER_CONDITIONS_H

#include "datautil.h"
#include "data_string.h"
#include "nbexp.h"
//#include "nbplace.h"

typedef struct data_weather_conditions_ {

	/* Child Elements */
	//None

	/* Attributes */
	double			temp;
	double			dewpt;
	uint32			rel_humidity;
	double			wind_speed;
	uint32			wind_dir;
	double			wind_gust;
	double			pressure;
	uint32			condition_code;
	data_string		condition;
	data_string		sky;
	double			ceil;
	uint32			visibility;
	double			heat_index;
	double			wind_chill;
	double			snow_depth;
	double			max_temp_6hr;
	double			max_temp_24hr;
	double			min_temp_6hr;
	double			min_temp_24hr;
	double			precipitation_3hr;
	double			precipitation_6hr;
	double			precipitation_24hr;
	uint32			update_time;
	int32           utc_offset;
} data_weather_conditions;

NB_Error	data_weather_conditions_init(data_util_state* pds, data_weather_conditions* pp);
void		data_weather_conditions_free(data_util_state* pds, data_weather_conditions* pp);

NB_Error	data_weather_conditions_from_tps(data_util_state* pds, data_weather_conditions* pp, tpselt te);

boolean		data_weather_conditions_equal(data_util_state* pds, data_weather_conditions* pp1, data_weather_conditions* pp2);
NB_Error	data_weather_conditions_copy(data_util_state* pds, data_weather_conditions* pp_dest, data_weather_conditions* pp_src);

uint32   data_weather_conditions_get_tps_size(data_util_state* pds, data_weather_conditions* pp);
void     data_weather_conditions_to_buf(data_util_state* pds, data_weather_conditions* pp, struct dynbuf* pdb);
NB_Error data_weather_conditions_from_binary(data_util_state* pds, data_weather_conditions* pp, byte** pdata, size_t* pdatalen);

#endif

