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
 * data_place.h: created 2004/12/09 by Mark Goddard.
 */

#ifndef DATA_PLACE_H
#define DATA_PLACE_H

#include "datautil.h"
#include "data_location.h"
#include "data_phone.h"
#include "data_email.h"
#include "data_url.h"
#include "data_category.h"
#include "vec.h"
#include "nbplace.h"
#include "data_fuelproduct.h"
#include "data_price.h"
#include "data_fueltype.h"
#include "data_weather_conditions.h"
#include "data_weather_forecast.h"
#include "data_traffic_incident.h"
#include "data_speed_camera.h"
#include "nbexp.h"

enum PlaceElement
{
    PlaceElement_None               = 0,
    PlaceElement_TrafficIncident    = 0x01,
    PlaceElement_WeatherCondition   = 0x02,
    PlaceElement_WeatherForecast    = 0x04,
    PlaceElement_SpeedCamera        = 0x08
};

typedef struct data_place
{
    /* Child elements */
    data_location                   location;
    struct CSL_Vector*              vec_phone;
    struct CSL_Vector*              vec_category;
    struct CSL_Vector*              vec_detail; /* place details */
    struct CSL_Vector*              vec_event;
    struct CSL_Vector*              vec_fuel_product;
    data_traffic_incident           traffic_incident;
    data_weather_conditions         weather_conditions;
    struct CSL_Vector*              vec_weather_forecast;
    data_speed_camera               camera;

    /* Attributes */
    data_string                     name;
    data_string                     id;
    uint64                          modificationTime;

    /* Contains the elements present in a place created from TPS */
    enum PlaceElement               elements;
} data_place;

NB_DEC NB_Error     data_place_init(data_util_state* pds, data_place* pdat);
NB_DEC void         data_place_free(data_util_state* pds, data_place* pdat);

NB_DEC NB_Error     data_place_from_tps(data_util_state* pds, data_place* pdat, tpselt te);
NB_DEC tpselt       data_place_to_tps(data_util_state* pds, data_place* pdat);

NB_DEC boolean      data_place_equal(data_util_state* pds, data_place* lhs, data_place* rhs);
NB_DEC NB_Error     data_place_copy(data_util_state* pds, data_place* dst, data_place* src);

NB_DEC NB_Error     data_place_from_nimplace(data_util_state* pds, data_place* pdat, const NB_Place* pPlace);

uint32   data_place_get_tps_size(data_util_state* pds, data_place* pdatl);
void     data_place_to_buf(data_util_state* pds, data_place* pdatl, struct dynbuf* pdb);
NB_Error data_place_from_binary(data_util_state* pds, data_place* pdatl, byte** pdata, size_t* pdatalen);

#endif
