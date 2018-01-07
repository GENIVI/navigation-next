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

    @file     nbanalyticseventstailq.c
    @defgroup nbanalyticseventstailq
*/
/*
    (C) Copyright 2005 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "palstdlib.h"
#include "nbcontext.h"
#include "nbcontextaccess.h"
#include "nbcontextprotected.h"
#include "nbanalytics.h"
#include "nbanalyticsprivate.h"
#include "nbanalyticsprotected.h"
#include "nbanalyticseventstailq.h"

/*! @{ */

NB_AnalyticsEventsTailQEntry*
NB_AnalyticsEventsTailQEntryCreate(void)
{
    NB_AnalyticsEventsTailQEntry* pEntry;

    pEntry =(NB_AnalyticsEventsTailQEntry*) nsl_malloc(sizeof(*pEntry));
    if (pEntry)
    {
        nsl_memset(pEntry, 0, sizeof(*pEntry));
    }

    return pEntry;
}

void
NB_AnalyticsEventsTailQEntryDestroy(
        data_util_state* pds, NB_AnalyticsEventsTailQEntry* pEntry)
{
    nsl_assert(pds != NULL);

    if (pEntry)
    {
        if (pEntry->event)
        {
            data_analytics_event_free(pds, pEntry->event);
            nsl_free(pEntry->event);
        }
        nsl_free(pEntry);
    }
}

void
NB_AnalyticsEventsTailQFreeAllEvents(
        data_util_state* pds, NB_AnalyticsEventsTailQ* tailq)
{
    NB_AnalyticsEventsTailQEntry* pTempEntry;
    NB_AnalyticsEventsTailQEntry* pEntry;

    NB_AE_TAILQ_FOREACH_SAFE(pEntry, tailq, pTempEntry)
    {
        NB_AE_TAILQ_REMOVE(tailq, pEntry);
        NB_AnalyticsEventsTailQEntryDestroy(pds, pEntry);
    }
}

/* @} */

