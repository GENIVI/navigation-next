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

    @file     nbtrafficparameters.h
    
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

#ifndef TRAFFICPARAMETERS_H
#define TRAFFICPARAMETERS_H

#include "nbcontext.h"
#include "nbrouteinformation.h"


/*!
    @addtogroup nbtrafficparameters
    @{
*/

/*! @struct NB_TrafficConfiguration
Opaque data structure that defines configuration for traffic parameters
*/
typedef struct NB_TrafficConfiguration {
    nb_boolean           notify;
    nb_boolean           startNotification;
    NB_RouteInformation* route;
    char*                supportedPhoneticsFormats;
    uint32               navProgressPosition;
    char*                voiceStyle;
} NB_TrafficConfiguration;

/*! @struct NB_TrafficParameters
Opaque data structure that defines parameters for traffic information
*/
typedef struct NB_TrafficParameters NB_TrafficParameters;

/*! Create traffic parameters

@param context NB_Context
@param notify Whether or not asynchronous traffic notification messages should be delivered to client
@param realTimeSpeed Specify non-zero to use current, real-time, speed and zero for historical speed
@param realTimeTraffic Specify non-zero to use current, real-time, traffic and zero for historical traffic
@param parameters On success, a newly created NB_TrafficParameters object; NULL otherwise.  A valid object must be destroyed using NB_TrafficParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_TrafficParametersCreate(NB_Context* context, nb_boolean notify, nb_boolean startNotification, NB_RouteInformation* route, uint32 navProgressPosition, const char* voiceStyle, NB_TrafficParameters** parameters);

/*! Create traffic parameters

@param context NB_Context
@param configuration Traffic configuration
@param parameters On success, a newly created NB_TrafficParameters object; NULL otherwise.  A valid object must be destroyed using NB_TrafficParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_TrafficParametersCreateEx(NB_Context* context, NB_TrafficConfiguration* configuration, NB_TrafficParameters** parameters);


/*! Destroy a previously created NB_TrafficParameters object

@param parameters A NB_TrafficParameters object created with NB_TrafficParametersCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_TrafficParametersDestroy(NB_TrafficParameters* parameters);


/*! @} */

#endif //TRAFFICPARAMETERS_H
