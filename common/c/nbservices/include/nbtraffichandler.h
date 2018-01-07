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

    @file     nbtraffichandler.h
       
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

#ifndef TRAFFICHANDLER_H
#define TRAFFICHANDLER_H

#include "nbcontext.h"
#include "nbhandler.h"
#include "nbtrafficparameters.h"
#include "nbtrafficinformation.h"

/*!
    @addtogroup nbtraffichandler
    @{
*/

/*! @struct NB_TrafficHandler
A TrafficHandler is used to download voice files
*/
typedef struct NB_TrafficHandler NB_TrafficHandler;


/*! Create and initialize a new TrafficHandler object

@param context NB_Context
@param callback Request handler status update callback
@param parameters NB_TrafficParameters object specifying the traffic information to retrieve
@param handler On success, the newly created handler; NULL otherwise
@returns NB_Error
*/
NB_DEC NB_Error NB_TrafficHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_TrafficHandler** handler);

/*! Destroy a previously created TrafficHandler object

@param handler A NB_TrafficHandler object created with NB_TrafficHandlerCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_TrafficHandlerDestroy(NB_TrafficHandler* handler);

/*! Start a network request to retrieve traffic information

@param handler A NB_TrafficHandler object
@param parameters NB_TrafficParameters object specifying the traffic parameters
@returns NB_Error
*/
NB_DEC NB_Error NB_TrafficHandlerStartRequest(NB_TrafficHandler* handler, NB_TrafficParameters* parameters);

/*! Cancel a previously started request

@param handler A NB_TrafficHandler object
@returns NB_Error
*/
NB_DEC NB_Error NB_TrafficHandlerCancelRequest(NB_TrafficHandler* handler);

/*! Check if a download request is in progress

@param handler A NB_TrafficHandler object
@returns Non-zero if a request is in progress; zero otherwise
*/
NB_DEC nb_boolean NB_TrafficHandlerIsRequestInProgress(NB_TrafficHandler* handler);

/*! Get the traffic information

@param handler A NB_TrafficHandler object
@param trafficInformation On success, a NB_TrafficInformation object; NULL othewise.  An object returned by this function must be destroyed with NB_TrafficInformationDestroy
@returns NB_Error
*/
NB_DEC NB_Error NB_TrafficHandlerGetTrafficInformation(NB_TrafficHandler* handler, NB_TrafficInformation** trafficInformation);


/*! @} */

#endif
