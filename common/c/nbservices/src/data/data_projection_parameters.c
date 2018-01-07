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

    @file     data_projection_parameters.c
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

#include "data_projection_parameters.h"

NB_Error
data_projection_parameters_init(data_util_state* state, data_projection_parameters* projectParameters)
{
    projectParameters->origin_latitude = 0.0;
    projectParameters->origin_longitude = 0.0;
    projectParameters->scale_factor = 0.0;
    projectParameters->false_easting = 0.0;
    projectParameters->false_northing = 0.0;
    projectParameters->z_offset = 0.0;

    data_string_init(state, &projectParameters->datum);

    return NE_OK;
}

void
data_projection_parameters_free(data_util_state* state, data_projection_parameters* projectParameters)
{
    data_string_free(state, &projectParameters->datum);
}

NB_Error
data_projection_parameters_from_tps(data_util_state* state, data_projection_parameters* projectParameters, tpselt tpsElement)
{
    NB_Error err = NE_OK;

    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, err, projectParameters, data_projection_parameters);

    err = err ? err : data_string_from_tps_attr(state, &projectParameters->datum, tpsElement, "datum");

    if (err != NE_OK)
    {
        goto errexit;
    }

    if (!te_getattrd(tpsElement, "origin-latitude", &projectParameters->origin_latitude) ||
        !te_getattrd(tpsElement, "origin-longitude", &projectParameters->origin_longitude) ||
        !te_getattrd(tpsElement, "scale-factor", &projectParameters->scale_factor) ||
        !te_getattrd(tpsElement, "false-easting", &projectParameters->false_easting) ||
        !te_getattrd(tpsElement, "false-northing", &projectParameters->false_northing) ||
        !te_getattrd(tpsElement, "z-offset", &projectParameters->z_offset))
    {
        err = NE_INVAL;
	}

errexit:

    if (err != NE_OK)
    {
        data_projection_parameters_free(state, projectParameters);
    }

    return err;
}

boolean
data_projection_parameters_equal(data_util_state* state, data_projection_parameters* projectParameters1, data_projection_parameters* projectParameters2)
{
    return (boolean) (projectParameters1->origin_latitude == projectParameters2->origin_latitude &&
					  projectParameters1->origin_longitude == projectParameters2->origin_longitude &&
					  projectParameters1->scale_factor == projectParameters2->scale_factor &&
					  projectParameters1->false_easting == projectParameters2->false_easting &&
					  projectParameters1->false_northing == projectParameters2->false_northing &&
					  projectParameters1->z_offset == projectParameters2->z_offset &&
				      data_string_equal(state, &projectParameters1->datum, &projectParameters2->datum));
}

NB_Error
data_projection_parameters_copy(data_util_state* state, data_projection_parameters* destinationProjectParameters, data_projection_parameters* sourceProjectParameters)
{
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, destinationProjectParameters, data_projection_parameters);

    DATA_COPY(state, err, &destinationProjectParameters->datum, &sourceProjectParameters->datum, data_string);

    if (err == NE_OK)
    {
        destinationProjectParameters->origin_latitude = sourceProjectParameters->origin_latitude;
        destinationProjectParameters->origin_longitude = sourceProjectParameters->origin_longitude;
        destinationProjectParameters->scale_factor = sourceProjectParameters->scale_factor;
        destinationProjectParameters->false_easting = sourceProjectParameters->false_easting;
        destinationProjectParameters->false_northing = sourceProjectParameters->false_northing;
        destinationProjectParameters->z_offset = sourceProjectParameters->z_offset;
    }

    return err;
}

/*! @} */
