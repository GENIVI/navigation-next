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

    @file     nbdetourparameters.h

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

#ifndef DETOURPARAMETERS_H
#define DETOURPARAMETERS_H

#include "nbcontext.h"

/*!
    @addtogroup nbdetourparameters
    @{
*/

/*! @struct NB_DetourParameters
Opaque data structure that defines the detour parameters of a detour operation 
*/
typedef struct NB_DetourParameters NB_DetourParameters;

/*! Create and initialize a new DetourParameters object

@param context NB_Context
@param parameters On success, the newly created parameters object; NULL otherwise.  A valid object must be destroyed using NB_DetourParametersDestroy
@returns NB_Error
*/
NB_DEC NB_Error NB_DetourParametersCreate(NB_Context* context, NB_DetourParameters** parameters);


/*! Clone a DetourParameters object

@param original The NB_DetourParameters object to clone
@param clone On success, the newly created parameters object; NULL otherwise.  A valid object must be destroyed using NB_DetourParametersDestroy
@returns NB_Error
*/
NB_DEC NB_Error NB_DetourParametersClone(NB_DetourParameters* original, NB_DetourParameters** clone);


/*! Destroy a previously created DetourParameters object

@param parameters A NB_DetourParameters object created with NB_DetourParametersCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_DetourParametersDestroy(NB_DetourParameters* parameters);


/*! Add a detour around the entire route

@param parameters A NB_DetourParameters object
@param label The name to associate with this detour
@returns NB_Error
*/
NB_DEC NB_Error NB_DetourParametersAddAvoidEntireRoute(NB_DetourParameters* parameters, const char* label);


/*! Add a detour around part of the route

@param parameters A NB_DetourParameters object
@param label The name to associate with this detour
@param start Distance, in meters, from the start of the route to avoid
@param length Distance, in meters, from the start position to avoid
@returns NB_Error
*/
NB_DEC NB_Error NB_DetourParametersAddAvoidPartOfRoute(NB_DetourParameters* parameters, const char* label, double start, double length);


/*! Add a detour around a set of maneuvers

@param parameters A NB_DetourParameters object
@param label The name to associate with this detour
@param startIndex The first maneuver index to detour around
@param endIndex The last maneuver index to detour around
@returns NB_Error
*/
NB_DEC NB_Error NB_DetourParametersAddAvoidManeuvers(NB_DetourParameters* parameters, const char* label, uint32 startIndex, uint32 endIndex);


/*! Add a detour around a traffic incident

@param parameters A NB_DetourParameters object
@param label The name to associate with this detour
@param index The index of the traffic incident to detour around
@returns NB_Error
*/
NB_DEC NB_Error NB_DetourParametersAddAvoidTrafficIncident(NB_DetourParameters* parameters, const char* label, uint32 index);

/*! Add a detour around congestion

@param parameters A NB_DetourParameters object
@param label The name to associate with this detour
@returns NB_Error
*/
NB_DEC NB_Error NB_DetourParametersAddAvoidCongestion(NB_DetourParameters* parameters, const char* label);

/*! Add a detour around traffic event

@param parameters A NB_DetourParameters object
@param label The name to associate with this detour
@param trafficEventIndex The index of the traffic event to detour around
@returns NB_Error
*/
NB_DEC NB_Error NB_DetourParametersAddAvoidTrafficEvent(NB_DetourParameters* parameters, const char* label, uint32 trafficEventIndex);

/*! @} */

#endif
