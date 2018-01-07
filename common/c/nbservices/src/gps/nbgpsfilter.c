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

    @file     nbgpsfilter.c
    
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

#include "cslqalog.h"
#include "nbcontextprotected.h"
#include "nbgpsfilter.h"
#include "nbqalog.h"


struct NB_GpsFilter
{
    NB_Context* context;
    double      s;	/* Speed threshold */
    double      a;	/* Filter constant */

    double      lat;	/* Latitude - Position Accumulator */
    double      lon; /* Longitude - Position Accumulator */
    double      n;   /* Filter Sum */
};


NB_Error
NB_GpsFilterCreate(NB_Context* context, double minimumSpeed, double filterConstant, NB_GpsFilter** filter)
{
    NB_GpsFilter* pThis = 0;

    if (!context || !filter)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;
    pThis->s = minimumSpeed;
    pThis->a = filterConstant;

    *filter = pThis;

    return NE_OK;
}

NB_Error
NB_GpsFilterDestroy(NB_GpsFilter* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    nsl_free(pThis);
    return NE_OK;
}

NB_Error
NB_GpsFilterGetFilteredFix(NB_GpsFilter* pThis, NB_GpsLocation* originalFix, NB_GpsLocation* filteredFix)
{
    if (!pThis || !originalFix ||!filteredFix)
    {
        return NE_INVAL;
    }
    *filteredFix = *originalFix;

    if (filteredFix->valid & NGV_HorizontalVelocity && filteredFix->horizontalVelocity >= pThis->s)
    {
        /* Valid speed above threshold - Reset Filter */
        pThis->n = 0.0;
        pThis->lat = 0.0;
        pThis->lon = 0.0;

        /* no change to fix */
    }
    else
    {
        /* Slow or invalid speed - Apply filter */
        pThis->n = 1.0 + (pThis->a * pThis->n);
        pThis->lat = filteredFix->latitude + (pThis->a * pThis->lat);
        pThis->lon = filteredFix->longitude + (pThis->a * pThis->lon);

        filteredFix->latitude = pThis->lat / pThis->n;
        filteredFix->longitude = pThis->lon / pThis->n;
    }

    if (CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(pThis->context)))
    {
        NB_QaLogGPSFixFiltered(pThis->context, filteredFix);
    }

    return NE_OK;
}
