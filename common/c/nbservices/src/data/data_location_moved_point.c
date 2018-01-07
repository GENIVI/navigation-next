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

    @file     data_location_moved_point.c
    
    Implemention for wifi TPS element for the Location servlet. 
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.      
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_location_moved_point.h"

NB_Error
data_location_moved_point_init(data_util_state* pds, data_location_moved_point* plp)
{
    plp->cause = 0;
    plp->timeStamp = 0;
    return data_string_init(pds, &plp->macAddress);
}

void
data_location_moved_point_free(data_util_state* pds, data_location_moved_point* plp)
{
    plp->cause = 0;
    plp->timeStamp = 0;
    data_string_free(pds, &plp->macAddress);
}

tpselt
data_location_moved_point_to_tps(data_util_state* pds, data_location_moved_point* plp)
{
    tpselt te = NULL;

    te = te_new("location-moved-point");

    if (!te) 
    {
        goto errexit;
    }

    if (!te_setattrc(te, "mac-address", data_string_get(pds, &plp->macAddress)))
    {
        goto errexit;
    }
    
    if (!te_setattru(te, "cause", (uint8) plp->cause))
    {
        goto errexit;
    }
    
    if (!te_setattru(te, "timestamp", plp->timeStamp))
    {
        goto errexit;
    }
    return te;

errexit:
    te_dealloc(te);
    return NULL;
}

boolean data_location_moved_point_equal(data_util_state* pds, data_location_moved_point* plp1, data_location_moved_point* plp2)
{
    return (boolean)(plp1->cause == plp2->cause
        && plp1->timeStamp == plp2->timeStamp
        && data_string_equal(pds, &plp1->macAddress, &plp2->macAddress));
}

NB_Error
data_location_moved_point_copy(data_util_state* pds, data_location_moved_point* plp_dest, data_location_moved_point* plp_src)
{
    plp_dest->cause = plp_src->cause;
    plp_dest->timeStamp = plp_src->timeStamp;
    return data_string_copy(pds, &plp_dest->macAddress, &plp_src->macAddress);
}

/*! @} */
