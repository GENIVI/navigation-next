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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "nbcontextprotected.h"
#include "pal.h"
#include "nbgpshistory.h"
#include "nbexp.h"
#include "vec.h"
#include "gpsutil.h"
#include "palclock.h"

#define DEFAULT_MAX_FIXES 1

struct NB_GpsHistory
{
    NB_Context* context;
    struct CSL_Vector* fixes;
    int max;
    double minSpeedValidHeading;
    double locationThreshHold;
    nb_boolean usedForAdding;
};

static nb_boolean IsGpsFix(NB_GpsLocation* fix, double threadhold)
{
    if (fix)
    {
        if ((fix->valid & NGV_Latitude) && (fix->valid & NGV_Longitude) &&
            fix->horizontalUncertaintyAlongAxis <= threadhold)
        {
            return TRUE;
        }
    }
    return FALSE;
}

NB_DEF NB_Error
NB_GpsHistoryCreate(NB_Context* context, int max, double filterSpeed, NB_GpsHistory** history)
{
    NB_GpsHistory* pThis = NULL;

    if (!context || max <= 0)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(context);

    if ((pThis = nsl_malloc(sizeof(NB_GpsHistory))) == NULL)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->max = max;
    pThis->minSpeedValidHeading = filterSpeed;
    pThis->context = context;
    pThis->locationThreshHold = 50;
    pThis->usedForAdding = FALSE;

    if ((pThis->fixes = CSL_VectorAlloc(sizeof(NB_GpsLocation))) == NULL)
    {
        nsl_free(pThis);
        return NE_NOMEM;
    }

    *history = pThis;

    return NE_OK;
}

NB_DEF NB_Error	
NB_GpsHistoryDestroy(NB_GpsHistory* history)
{
    if (!history)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(history->context);
    
    if (history->fixes)
        CSL_VectorDealloc(history->fixes);

    nsl_free(history);
    
    return NE_OK;
}

NB_DEF NB_Error	
NB_GpsHistoryAdd(NB_GpsHistory* history, NB_GpsLocation* fix)
{
    int size = 0;
    if (!history || !history->fixes )
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(history->context);

    if (history->usedForAdding && !IsGpsFix(fix, history->locationThreshHold))
    {
        return NE_GPS;
    }

    size = CSL_VectorGetLength(history->fixes);
    if (size > 0)
    {
        NB_GpsLocation* lastFix = (NB_GpsLocation*) CSL_VectorGetPointer(history->fixes, size-1);
        if (lastFix && gpsfix_identical(lastFix, fix))
        {
            /* skip fixes that is identical to the last fix */
            return NE_OK;
        }
    }

    if (size >= history->max)
    {
        CSL_VectorRemove(history->fixes, 0);
    }

    if(!CSL_VectorAppend(history->fixes, fix))
    {
        return NE_NOMEM;
    }

    return NE_OK;
}

NB_DEF NB_Error	
NB_GpsHistoryClear(NB_GpsHistory* history)
{
    if (!history || !history->fixes)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(history->context);

    CSL_VectorRemoveAll(history->fixes);

    return NE_OK;
}

NB_DEF NB_Error   
NB_GpsHistorySetMinimumFixSpeed(NB_GpsHistory* history, double filterSpeed)
{
    if (!history)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(history->context);

    history->minSpeedValidHeading = filterSpeed;

    return NE_OK;
}


NB_DEF NB_Error	
NB_GpsHistoryGetLatest(NB_GpsHistory* history, NB_GpsLocation* fix)
{    
    int size = 0;
    if (!history || !history->fixes || !fix)
    {
        return NE_INVAL;
    }
    
    NB_ASSERT_VALID_THREAD(history->context);

    size = CSL_VectorGetLength(history->fixes);
    if (size > 0)
    {
        NB_GpsLocation* t = (NB_GpsLocation*) CSL_VectorGetPointer(history->fixes, size-1);
        if (t)
        {
            *fix = *t;
        }

    }
    else 
    {
        return NE_NOENT;
    }

    return NE_OK;
}

NB_DEF NB_Error	
NB_GpsHistoryGetLatestHeadingFix(NB_GpsHistory* history, NB_GpsLocation* fix)
{
    int size = 0;
    int i = 0;
    if (!history || !history->fixes || !fix)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(history->context);

    size = CSL_VectorGetLength(history->fixes);
    if (size > 1)
    {
        for (i = size-1; i > 0; i--)
        {
            NB_GpsLocation* t = (NB_GpsLocation*) CSL_VectorGetPointer(history->fixes, i);
            if (t)
            {
                if( (t->valid & NGV_Heading) && (t->valid & NGV_HorizontalVelocity) &&
                    (t->horizontalVelocity >= history->minSpeedValidHeading) )
                {
                    *fix = *t;
                    return NE_OK;
                }
            }
        }
    }

    return NE_NOENT;
}

NB_DEF NB_Error
NB_GpsHistoryGet(NB_GpsHistory* history, int index, NB_GpsLocation* fix)
{
    if (!history || !history->fixes)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(history->context);

    *fix = *(NB_GpsLocation*) CSL_VectorGetPointer(history->fixes, index);

    return NE_OK;
}

NB_DEF int
NB_GpsHistoryGetCount(NB_GpsHistory* history)
{
    if (!history || !history->fixes)
    {
        return 0;
    }

    NB_ASSERT_VALID_THREAD(history->context);

    return CSL_VectorGetLength(history->fixes);
}

NB_DEF NB_Error
NB_GpsHistoryForEach(NB_GpsHistory* history, NB_GpsHistoryCallbackFunction callback, void* callbackData)
{
    int size = 0;
    int n = 0;
    NB_GpsLocation* fix = NULL;

    if (!history || !history->fixes || !callback)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(history->context);

    size = CSL_VectorGetLength(history->fixes);

    for (n = 0; n < size; n++)
    {
       fix = (NB_GpsLocation*) CSL_VectorGetPointer(history->fixes, n);
       (callback)(callbackData, fix);
    }

    return NE_OK;
}

NB_DEF nb_boolean
NB_GpsHistoryIsGpsPoor(NB_GpsHistory* history)
{
    int size = 0;
    NB_GpsLocation* fix = NULL;
    nb_boolean ret = FALSE;
    nb_gpsTime currentTime = 0;
    int goodNum = 8; // TODO we should receive this parameter from server
    int timeInterval = 10; // TODO we should receive this parameter from server
    int i = 0;

    if (!history || !history->fixes)
    {
        return TRUE;
    }

    if(history->fixes)
    {
        size = CSL_VectorGetLength(history->fixes);
    }

    if(size < goodNum)
    {
        ret = TRUE;
    }
    currentTime = PAL_ClockGetGPSTime();
    if(!ret)
    {
        for(i = size - 1; i >= size - goodNum && !ret; i--)
        {
            fix = (NB_GpsLocation*) CSL_VectorGetPointer(history->fixes, i);
            if(fix)
            {
                if(fix->horizontalUncertaintyAlongAxis > history->locationThreshHold ||
                   fix->gpsTime - timeInterval > currentTime)
                {
                    ret = TRUE;
                }
            }
            else
            {
                ret = TRUE;
            }
        }
    }
    return ret;
}

NB_DEF NB_Error
NB_GpsHistorySetLocationThreshhold(NB_GpsHistory* history, double threshhold, nb_boolean usedForAdding)
{
    if (!history || threshhold <=0)
    {
        return NE_INVAL;
    }

    history->locationThreshHold = threshhold;
    history->usedForAdding = usedForAdding;
    return NE_OK;
}


NB_DEF NB_Error
NB_GpsHistoryIfLastHeadingValid(NB_GpsHistory* history, nb_boolean* isValid)
{
    NB_GpsLocation* fix = NULL;
    int size = 0;
    if (!history || !isValid || !history->fixes ||
        CSL_VectorGetLength(history->fixes) == 0)
    {
        return NE_INVAL;
    }

    size = CSL_VectorGetLength(history->fixes);
    fix = (NB_GpsLocation*) CSL_VectorGetPointer(history->fixes, size - 1);
    if (!fix)
    {
        return NE_NOENT;
    }

    *isValid = FALSE;
    if( (fix->valid & NGV_Heading) && (fix->valid & NGV_HorizontalVelocity) &&
       (fix->horizontalVelocity >= history->minSpeedValidHeading) )
    {
        *isValid = TRUE;
    }
    return NE_OK;
}