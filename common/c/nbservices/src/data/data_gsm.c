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

    @file     data_gsm.c
    
    Implemention for gsm TPS element for the Location servlet.    
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

#include "data_gsm.h"

NB_Error
data_gsm_init(data_util_state* pds, data_gsm* pg)
{
    pg->mcc = 0;
    pg->mnc = 0;
    pg->lac = 0;
    pg->cellid = 0;
    pg->signalStrength = 0;

    return NE_OK;
}

void
data_gsm_free(data_util_state* pds, data_gsm* pg)
{
    pg->mcc = 0;
    pg->mnc = 0;
    pg->lac = 0;
    pg->cellid = 0;
    pg->signalStrength = 0;
}

tpselt
data_gsm_to_tps(data_util_state* pds, data_gsm* pg)
{
    tpselt te = NULL;

    te = te_new("gsm");

    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattru(te, "mcc", pg->mcc)
        || !te_setattru(te, "mnc", pg->mnc)
        || !te_setattru(te, "lac", pg->lac)
        || !te_setattru(te, "cellid", pg->cellid)
        || !te_setattru(te, "signal-strength", pg->signalStrength < 0 ? -pg->signalStrength : pg->signalStrength))
    {
        goto errexit;
    }

    return te;

errexit:
    te_dealloc(te);
    return NULL;
}

boolean
data_gsm_equal(data_util_state* pds, data_gsm* pg1, data_gsm* pg2)
{
    return (boolean)(pg1->mcc == pg2->mcc
                    && pg1->mnc == pg2->mnc
                    && pg1->lac == pg2->lac
                    && pg1->cellid == pg2->cellid
                    && pg1->signalStrength == pg2->signalStrength);
}

NB_Error
data_gsm_copy(data_util_state* pds, data_gsm* pg_dest, data_gsm* pg_src)
{
    pg_src->mcc = pg_dest->mcc;
    pg_src->mnc = pg_dest->mnc;
    pg_src->lac = pg_dest->lac;
    pg_src->cellid = pg_dest->cellid;
    pg_src->signalStrength = pg_dest->signalStrength;

    return NE_OK;
}

/*! @} */
