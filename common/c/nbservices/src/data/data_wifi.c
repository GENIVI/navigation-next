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

    @file     data_wifi.c
    
    Implemention for wifi TPS element for the Location servlet. 
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

#include "data_wifi.h"

NB_Error
data_wifi_init(data_util_state* pds, data_wifi* pwf)
{
    pwf->signalStrength = 0;
    return data_string_init(pds, &pwf->macAddress);
}

void
data_wifi_free(data_util_state* pds, data_wifi* pwf)
{
    pwf->signalStrength = 0;
    data_string_free(pds, &pwf->macAddress);
}

tpselt
data_wifi_to_tps(data_util_state* pds, data_wifi* pwf)
{
    tpselt te = NULL;

    te = te_new("wifi");

    if (!te) 
    {
        goto errexit;
    }

    if (!te_setattrc(te, "mac-address", data_string_get(pds, &pwf->macAddress)))
    {
        goto errexit;
    }
    
    if (!te_setattru(te, "signal-strength", pwf->signalStrength < 0 ? -pwf->signalStrength : pwf->signalStrength))
    {
        goto errexit;
    }

    return te;

errexit:
    te_dealloc(te);
    return NULL;
}

boolean data_wifi_equal(data_util_state* pds, data_wifi* pwf1, data_wifi* pwf2)
{
    return (boolean)(pwf1->signalStrength == pwf2->signalStrength
        && data_string_equal(pds, &pwf1->macAddress, &pwf2->macAddress));
}

NB_Error
data_wifi_copy(data_util_state* pds, data_wifi* pwf_dest, data_wifi* pwf_src)
{
    pwf_dest->signalStrength = pwf_src->signalStrength;
    return data_string_copy(pds, &pwf_dest->macAddress, &pwf_src->macAddress);
}

/*! @} */
