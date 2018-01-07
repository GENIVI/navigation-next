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

    @file     data_location_error.c
    
    Implemention for cellid_error TPS element for the Location servlet.    
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

#include "data_location_error.h"

NB_Error
data_location_error_init(data_util_state* pds, data_location_error* pce)
{
    pce->code = 0;

    return NE_OK;
}

void
data_location_error_free(data_util_state* pds, data_location_error* pce)
{
    pce->code = 0;
}

NB_Error
data_location_error_from_tps(data_util_state* pds, data_location_error* pce, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_location_error_free(pds, pce);
    err = data_location_error_init(pds, pce);

    pce->code = te_getattru(te, "code");

errexit:
    if (err != NE_OK)
    {
        data_location_error_free(pds, pce);
    }
    return err;
}

boolean
data_location_error_equal(data_util_state* pds, data_location_error* pce1, data_location_error* pce2)
{
    return (boolean)(pce1->code == pce2->code);
}

NB_Error
data_location_error_copy(data_util_state* pds, data_location_error* pce_dest, data_location_error* pce_src)
{
    pce_dest->code = pce_src->code;
    
    return NE_OK;
}

/*! @} */
