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

    @file     data_location_position.c
    
    Implemention for position TPS element for the Location servlet.
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "data_location_position.h"

NB_Error
data_location_position_init(data_util_state* pds, data_location_position* pcp)
{
    NB_Error err = NE_OK;
    
    pcp->lat = 0.0;
    pcp->lon = 0.0;
    pcp->accuracy = 0;
    
    err = data_string_init(pds, &pcp->sourceType);

    return err;
}

void
data_location_position_free(data_util_state* pds, data_location_position* pcp)
{
    pcp->lat = 0.0;
    pcp->lon = 0.0;
    pcp->accuracy = 0;
    
    data_string_free(pds, &pcp->sourceType);
}

NB_Error
data_location_position_from_tps(data_util_state* pds, data_location_position* pcp, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_location_position_free(pds, pcp);
    err = data_location_position_init(pds, pcp);

    if (!te_getattrd(te, "lat", &pcp->lat)
        || !te_getattrd(te, "lon", &pcp->lon))
    {
        err = NE_NOMEM;
        goto errexit;
    }
    pcp->accuracy = te_getattru(te, "accuracy");

    err = err ? err : data_string_from_tps_attr(pds, &pcp->sourceType, te, "sourcetype");

errexit:
    if (err != NE_OK)
    {
        data_location_position_free(pds, pcp);
    }
    return err;
}

boolean
data_location_position_equal(data_util_state* pds, data_location_position* pcp1, data_location_position* pcp2)
{
    return (boolean)(pcp1->lat == pcp2->lat
        && pcp1->lon == pcp2->lon
        && pcp1->accuracy == pcp2->accuracy
        && data_string_equal(pds, &pcp1->sourceType, &pcp2->sourceType));
}

NB_Error
data_location_position_copy(data_util_state* pds, data_location_position* pcp_dest, data_location_position* pcp_src)
{
    NB_Error err = NE_OK;
    
    pcp_dest->lat = pcp_src->lat;
    pcp_dest->lon = pcp_src->lon;
    pcp_dest->accuracy = pcp_src->accuracy;

    err = data_string_copy(pds, &pcp_dest->sourceType, &pcp_src->sourceType);

    return err;
}

/*! @} */
