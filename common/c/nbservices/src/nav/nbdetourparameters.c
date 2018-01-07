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

@file nbdetourparameters.c
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

#include "palstdlib.h"
#include "nbdetourparameters.h"
#include "nbdetourparametersprivate.h"
#include "vec.h"

/*!
    @addtogroup nbdetourparameters
    @{
*/

struct NB_DetourParameters 
{
    struct CSL_Vector*  detours;
};

static int CopyDetourDetails(void* callbackData, void* dest, const void* src);
static int FreeDetourDetails(void* callbackData, void* details);

NB_DEF NB_Error 
NB_DetourParametersCreate(NB_Context* context, NB_DetourParameters** parameters)
{
    NB_DetourParameters* pThis = 0;

    if (!context || !parameters)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));
    pThis->detours = CSL_VectorAlloc(sizeof(NB_DetourDetails));
    if (!pThis->detours)
    {
        nsl_free(pThis);
        return NE_NOMEM;
    }

    *parameters = pThis;
    return NE_OK;
}

NB_DEF NB_Error 
NB_DetourParametersClone(NB_DetourParameters* pThis, NB_DetourParameters** clone)
{
    NB_DetourParameters* temp = 0;
    if (!pThis || !clone)
    {
        return NE_INVAL;
    }
    *clone = 0;

    temp = nsl_malloc(sizeof(*temp));
    if (!temp)
    {
        return NE_NOMEM;
    }

    if (!CSL_VectorCopy(&temp->detours, pThis->detours, CopyDetourDetails, NULL))
    {
        nsl_free(temp);
        return NE_NOMEM;
    }
    *clone = temp;

    return NE_OK;
}

NB_DEF NB_Error 
NB_DetourParametersDestroy(NB_DetourParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    CSL_VectorForEach(pThis->detours, FreeDetourDetails, NULL);
    CSL_VectorDealloc(pThis->detours);
    nsl_free(pThis);

    return NE_OK;
}

NB_DEF NB_Error 
NB_DetourParametersAddAvoidEntireRoute(NB_DetourParameters* pThis, const char* label)
{
    NB_DetourDetails details = { 0 };

    if (!pThis)
    {
        return NE_INVAL;
    }

    details.detourName = nsl_strdup(label);
    if (!details.detourName)
    {
        return NE_NOMEM;
    }

    details.type = NB_DT_EntireRoute;

    return CSL_VectorAppend(pThis->detours, &details) ? NE_OK : NE_NOMEM;
}

NB_DEF NB_Error 
NB_DetourParametersAddAvoidPartOfRoute(NB_DetourParameters* pThis, const char* label, double start, double length)
{
    NB_DetourDetails details = { 0 };

    if (!pThis)
    {
        return NE_INVAL;
    }

    details.detourName = nsl_strdup(label);
    if (!details.detourName)
    {
        return NE_NOMEM;
    }

    details.type = NB_DT_PartOfRoute;
    details.startDistance = start;
    details.length = length;

    return CSL_VectorAppend(pThis->detours, &details) ? NE_OK : NE_NOMEM;
}

NB_DEF NB_Error 
NB_DetourParametersAddAvoidManeuvers(NB_DetourParameters* pThis, const char* label, uint32 startIndex, uint32 endIndex)
{
    NB_DetourDetails details = { 0 };

    if (!pThis)
    {
        return NE_INVAL;
    }

    details.detourName = nsl_strdup(label);
    if (!details.detourName)
    {
        return NE_NOMEM;
    }

    details.type = NB_DT_Maneuver;
    details.startDistance = 0.0;
    details.startIndex = startIndex;
    details.endIndex = endIndex;

    return CSL_VectorAppend(pThis->detours, &details) ? NE_OK : NE_NOMEM;
}

NB_DEF NB_Error 
NB_DetourParametersAddAvoidTrafficIncident(NB_DetourParameters* pThis, const char* label, uint32 index)
{
    NB_DetourDetails details = { 0 };

    if (!pThis)
    {
        return NE_INVAL;
    }

    details.detourName = nsl_strdup(label);
    if (!details.detourName)
    {
        return NE_NOMEM;
    }

    details.type = NB_DT_Incident;
    details.incidentIndex = index;

    return CSL_VectorAppend(pThis->detours, &details) ? NE_OK : NE_NOMEM;
}

NB_DEF NB_Error
NB_DetourParametersAddAvoidCongestion(NB_DetourParameters* pThis, const char* label)
{
    NB_DetourDetails details = { 0 };

    if (!pThis)
    {
        return NE_INVAL;
    }

    details.detourName = nsl_strdup(label);
    if (!details.detourName)
    {
        return NE_NOMEM;
    }

    details.type = NB_DT_Congestion;

    return CSL_VectorAppend(pThis->detours, &details) ? NE_OK : NE_NOMEM;
}

NB_DEF NB_Error
NB_DetourParametersAddAvoidTrafficEvent(NB_DetourParameters* pThis, const char* label, uint32 trafficEventIndex)
{
    NB_DetourDetails details = { 0 };

    if (!pThis)
    {
        return NE_INVAL;
    }

    details.detourName = nsl_strdup(label);
    if (!details.detourName)
    {
        return NE_NOMEM;
    }

    details.type = NB_DT_TrafficEvent;
    details.eventIndex = trafficEventIndex;

    return CSL_VectorAppend(pThis->detours, &details) ? NE_OK : NE_NOMEM;
}

nb_usize NB_DetourParametersGetCount(NB_DetourParameters* pThis)
{
    if (!pThis)
    {
        return 0;
    }

    return CSL_VectorGetLength(pThis->detours);
}

NB_Error NB_DetourParametersGetDetourDetails(NB_DetourParameters* pThis, uint32 index, NB_DetourDetails* details)
{
    if (!pThis || !details)
    {
        return NE_INVAL;
    }
    nsl_memset(details, 0, sizeof(*details));

    if ((int)index > CSL_VectorGetLength(pThis->detours))
    {
        return NE_NOENT;
    }

    CSL_VectorGet(pThis->detours, index, details);

    return NE_OK;
}

int
CopyDetourDetails(void* callbackData, void* dest, const void* src)
{
    const NB_DetourDetails* original = src;
    NB_DetourDetails* copy = dest;

    copy->type = original->type;
    copy->startDistance = original->startDistance;
    copy->length = original->length;
    copy->detourName = nsl_strdup(original->detourName);
    copy->incidentIndex = original->incidentIndex;
    copy->startIndex = original->startIndex;
    copy->endIndex = original->incidentIndex;

    return TRUE;
}

int
FreeDetourDetails(void* callbackData, void* object)
{
    NB_DetourDetails* details = object;
    nsl_free(details->detourName);
    return TRUE;
}

/*! @} */
