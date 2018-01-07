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

    @file     data_projection_parameters.h
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

#ifndef DATA_PROJECTION_PARAMETERS_H
#define DATA_PROJECTION_PARAMETERS_H

#include "datautil.h"
#include "data_string.h"

typedef struct data_projection_parameters_
{
    /* Child Elements */

    /* Attributes */
    double          origin_latitude;    /*!< Latitude of origin for the projection. */
    double          origin_longitude;   /*!< Longitude of the origin of the projection. */
    double          scale_factor;       /*!< Always 0.9996 for "universal-transverse-mercator". */
    double          false_easting;      /*!< 500000.0 for "universal-transverse-mercator". */
    double          false_northing;     /*!< 10000000.0 for "universal-transverse-mercator" in
                                             the southern hemisphere. */
    double          z_offset;           /*!< A constant value to add to all z-coordinates in
                                             the model to obtain the actual height off the
                                             referenced ellipsoid. */
    data_string     datum;              /*!< "WGS84"or "D_North_American_1983" */
} data_projection_parameters;

NB_Error    data_projection_parameters_init(data_util_state* state, data_projection_parameters* projectParameters);
void        data_projection_parameters_free(data_util_state* state, data_projection_parameters* projectParameters);

NB_Error    data_projection_parameters_from_tps(data_util_state* state, data_projection_parameters* projectParameters, tpselt tpsElement);

boolean     data_projection_parameters_equal(data_util_state* state, data_projection_parameters* projectParameters1, data_projection_parameters* projectParameters2);
NB_Error    data_projection_parameters_copy(data_util_state* state, data_projection_parameters* destinationProjectParameters, data_projection_parameters* sourceProjectParameters);

#endif // DATA_PROJECTION_PARAMETERS_H

/*! @} */
