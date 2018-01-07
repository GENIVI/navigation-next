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

/*!--------------------------------------------------------------------------

 @file     data_special_speed_zone.c
 */
/*
 (C) Copyright 2011 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of Networks In Motion is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/

/*! @{ */

#include "data_special_speed_zone.h"
NB_Error data_special_speed_zone_init(data_util_state* state, data_special_speed_zone* specialSpeedZone)
{
    NB_Error error = NE_OK;
    if (specialSpeedZone == NULL || state == NULL)
    {
        return NE_INVAL;
    }
    error = error ? error : data_string_init(state, &specialSpeedZone->speed_zone_type);

    specialSpeedZone->warn_ahead = 0;

    return error;

}
void data_special_speed_zone_free(data_util_state* state, data_special_speed_zone* specialSpeedZone)
{
    if (specialSpeedZone == NULL || state == NULL)
    {
        return;
    }
    data_string_free(state, &specialSpeedZone->speed_zone_type);
}

NB_Error data_special_speed_zone_from_tps(data_util_state* state, data_special_speed_zone* specialSpeedZone, tpselt tpsElement)
{
    NB_Error error = NE_OK;
    if (specialSpeedZone == NULL || state == NULL)
    {
        return NE_INVAL;
    }

    if (tpsElement == NULL)
    {
        error = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, error, specialSpeedZone, data_special_speed_zone);

    error = error ? error : data_string_from_tps_attr(state, &specialSpeedZone->speed_zone_type, tpsElement, "speed-zone-type");

    if (error != NE_OK) {
        goto errexit;
    }

    te_getattrd(tpsElement, "warn-ahead", &specialSpeedZone->warn_ahead);

errexit:

    if (error != NE_OK)
    {
        data_special_speed_zone_free(state, specialSpeedZone);
    }

    return error;

}

NB_Error data_special_speed_zone_copy(data_util_state* state,
                                         data_special_speed_zone* destinationSpecialSpeedZone,
                                         data_special_speed_zone* sourceSpecialSpeedZone)
{
    NB_Error error = NE_OK;
    if (destinationSpecialSpeedZone == NULL || sourceSpecialSpeedZone == NULL || state == NULL)
    {
        return NE_INVAL;
    }
    DATA_COPY(state, error, &destinationSpecialSpeedZone->speed_zone_type, &sourceSpecialSpeedZone->speed_zone_type, data_string);
    destinationSpecialSpeedZone->warn_ahead = sourceSpecialSpeedZone->warn_ahead;

    return error;
}


/*! @} */
