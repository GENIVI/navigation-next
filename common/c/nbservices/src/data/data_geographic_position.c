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

    @file   data_geographic_position.c
*/
/*
    See file description in header file.

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

#include "data_geographic_position.h"
#include "nbgpstypes.h"

NB_Error
data_geographic_position_init(data_util_state* state, data_geographic_position* geographicPosition)
{
    geographicPosition->latitude = INVALID_LATITUDE;
    geographicPosition->longitude = INVALID_LONGITUDE;
    geographicPosition->accuracy = 0;
    geographicPosition->time = 0;

    return NE_OK;
}

void
data_geographic_position_free(data_util_state* state, data_geographic_position* geographicPosition)
{
    /* nothing to free */
}

tpselt
data_geographic_position_to_tps(data_util_state* state, data_geographic_position* geographicPosition)
{
    tpselt tpsElement = NULL;

    tpsElement = te_new("geographic-position");

    if (tpsElement == NULL)
    {
        goto errexit;
    }

    if (!te_setattrd(tpsElement, "lat", geographicPosition->latitude))
    {
        goto errexit;
    }

    if (!te_setattrd(tpsElement, "lon", geographicPosition->longitude))
    {
        goto errexit;
    }

    if (!te_setattru(tpsElement, "accuracy", geographicPosition->accuracy))
    {
        goto errexit;
    }

    if (!te_setattru(tpsElement, "time", geographicPosition->time))
    {
        goto errexit;
    }

    return tpsElement;

errexit:

    te_dealloc(tpsElement);
    return NULL;
}

NB_Error
data_geographic_position_from_tps(data_util_state* state, data_geographic_position* geographicPosition, tpselt tpsElement)
{
    if (!te_getattrd(tpsElement, "lat", &geographicPosition->latitude) ||
        !te_getattrd(tpsElement, "lon", &geographicPosition->longitude))
    {
        return NE_INVAL;
    }

    geographicPosition->accuracy = te_getattru(tpsElement, "accuracy");
	geographicPosition->time = te_getattru(tpsElement, "time");

	return NE_OK;
}

boolean
data_geographic_position_equal(data_util_state* state, data_geographic_position* geographicPosition1, data_geographic_position* geographicPosition2)
{
    return (boolean) (geographicPosition1->latitude == geographicPosition2->latitude &&
                      geographicPosition1->longitude == geographicPosition2->longitude &&
                      geographicPosition1->accuracy == geographicPosition2->accuracy &&
                      geographicPosition1->time == geographicPosition2->time);
}

NB_Error
data_geographic_position_copy(data_util_state* state, data_geographic_position* destinationGeographicPosition, data_geographic_position* sourceGeographicPosition)
{
	destinationGeographicPosition->latitude = sourceGeographicPosition->latitude;
	destinationGeographicPosition->longitude = sourceGeographicPosition->longitude;
	destinationGeographicPosition->accuracy = sourceGeographicPosition->accuracy;
	destinationGeographicPosition->time = sourceGeographicPosition->time;

	return NE_OK;
}

/*! @} */
