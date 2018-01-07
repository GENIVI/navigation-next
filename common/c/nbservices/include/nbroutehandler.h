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

    @file     nbroutehandler.h

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

#ifndef NBROUTEHANDLER_H
#define NBROUTEHANDLER_H

#include "nbrouteinformation.h"
#include "nbhandler.h"
#include "nbrouteparameters.h"
#include "nbtrafficinformation.h"
#include "nbcamerainformation.h"

/*!
    @addtogroup nbroutehandler
    @{
*/

/*! @struct NB_RouteHandler
A RouteHandler is used to download a route
*/
typedef struct NB_RouteHandler NB_RouteHandler;


/*! Create and initialize a new RouteHandler object

@param context NB_Context
@param callback Request handler status update callback
@param parameters NB_RouteParameters object specifying the type of route to retrieve
@param handler On success, the newly created handler; NULL otherwise
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_RouteHandler** handler);


/*! Destroy a previously created RouteHandler object

@param handler A NB_RouteHandler object created with NB_RouteHandlerCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteHandlerDestroy(NB_RouteHandler* handler);


/*! Start a network request to retrieve a route

@param handler A NB_RouteHandler object
@param parameters NB_RouteParameters object specifying the route parameters
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteHandlerStartRequest(NB_RouteHandler* handler, NB_RouteParameters* parameters);


/*! Cancel a previously started request

@param handler A NB_RouteHandler object
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteHandlerCancelRequest(NB_RouteHandler* handler);


/*! Check if a download request is in progress

@param handler A NB_RouteHandler object
@returns Non-zero if a request is in progress; zero otherwise
*/
NB_DEC nb_boolean NB_RouteHandlerIsRequestInProgress(NB_RouteHandler* handler);


/*! Retrieves a NB_Route object containing the route

@param handler A NB_RouteHandler object
@param route On success, a NB_Route object with the result of the last download; NULL otherwise.  An object returned from this function must be destroyed using NB_RouteDestroy().
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteHandlerGetRouteInformation(NB_RouteHandler* handler, NB_RouteInformation** route);

/*! Retrieves a NB_Route object containing the route

@param handler A NB_RouteHandler object
@param route On success, a NB_TrafficInformation object with the result of the last download; NULL otherwise.  An object returned from this function must be destroyed using NB_RouteDestroy().
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteHandlerGetTrafficInformation(NB_RouteHandler* handler, NB_TrafficInformation** information);

/*! Get a route error associated with the request

@param handler A NB_RouteHandler object
@param routeError The route error value
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteHandlerGetRouteError(NB_RouteHandler* handler, NB_NavigateRouteError* error);

/*! @} */

#endif //NBROUTEHANDLER_H
