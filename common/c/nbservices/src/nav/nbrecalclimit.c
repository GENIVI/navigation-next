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

    @file     nbrecalclimit.c
    
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#include "nbrecalclimit.h"


struct NB_RecalcLimit
{
    uint32          count;
    uint32          timePeriod;
    uint32*         recalcTimes;
};


NB_Error
NB_RecalcLimitCreate(NB_Context* context, uint32 maxCount, uint32 timePeriod, NB_RecalcLimit** limit)
{
    NB_RecalcLimit* pThis = 0;

    if (!context || !limit)
    {
        return NE_INVAL;
    }
    
    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->recalcTimes = nsl_malloc(maxCount * sizeof(nb_gpsTime));
    if (!pThis->recalcTimes)
    {
        nsl_free(pThis);
        return NE_NOMEM;
    }
    nsl_memset(pThis->recalcTimes, 0, maxCount * sizeof(nb_gpsTime));

    pThis->count = maxCount;
    pThis->timePeriod = timePeriod;

    *limit = pThis;

    return NE_OK;
}

NB_Error
NB_RecalcLimitDestroy(NB_RecalcLimit* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->recalcTimes)
    {
        nsl_free(pThis->recalcTimes);
    }
    nsl_free(pThis);

    return NE_OK;
}

NB_Error
NB_RecalcLimitAdd(NB_RecalcLimit* pThis, nb_gpsTime recalcTime)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->recalcTimes)
    {
        if (pThis->count > 1)
        {
            int i = 0;
            for (i = pThis->count - 1; i >= 1; i--)
            {
                pThis->recalcTimes[i] = pThis->recalcTimes[i - 1];
            }
        }

        pThis->recalcTimes[0] = recalcTime;
    }

    return NE_OK;
}

NB_Error
NB_RecalcLimitReset(NB_RecalcLimit* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->recalcTimes)
    {
        nsl_memset(pThis->recalcTimes, 0, pThis->count * sizeof(nb_gpsTime));
    }

    return NE_OK;
}

nb_boolean
NB_RecalcLimitExceeded(NB_RecalcLimit* pThis, nb_gpsTime recalcTime)
{
    if (!pThis)
    {
        return FALSE;
    }

    if (pThis->count == 0 || !pThis->recalcTimes)
    {
		return TRUE;
    }

    return (nb_boolean)((recalcTime - pThis->recalcTimes[pThis->count - 1]) <= pThis->timePeriod);
}
