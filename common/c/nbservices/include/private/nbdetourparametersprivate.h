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

@file     detourparametersprivate.h
@defgroup nbparams Parameters

This API is used to create Detour Parameters objects.  Detour Parameters objects can be used as arguments when
creating routea parameter objects
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

#ifndef DETOURPARAMETERSPRIVATE_H
#define DETOURPARAMETERSPRIVATE_H

#include "nbcontext.h"
#include "nbdetourparameters.h"
#include "navpublictypes.h"

typedef enum
{
    NB_DT_EntireRoute = 0,
    NB_DT_PartOfRoute,
    NB_DT_Maneuver,
    NB_DT_Incident,
    NB_DT_Congestion,
    NB_DT_TrafficEvent

} NB_DetourType;

typedef struct
{
    NB_DetourType       type;
    double              startDistance;
    double              length;
    char*               detourName;
    uint32              incidentIndex;
    uint32              startIndex;
    uint32              endIndex;
    uint32              eventIndex;
} NB_DetourDetails;


/*! Get the count of detours
@param parameters The NB_DetourParameters object
@return The number of individual detours in the object
*/
nb_usize NB_DetourParametersGetCount(NB_DetourParameters* pThis);

/*! Get details about a particular detour
@param parameters The NB_DetourParameters object to get the details from
@param index The index of the detour to get the details for
@param details A NB_DetourDetails structure that will receive the details
@return NB_Error
*/
NB_Error NB_DetourParametersGetDetourDetails(NB_DetourParameters* parameters, uint32 index, NB_DetourDetails* details);

#endif
