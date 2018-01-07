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

#ifndef _OFFBOARDROUTEHANDLER_H_
#define _OFFBOARDROUTEHANDLER_H_

#include "nbroutehandlerprivate.h"

typedef struct NB_RouteHandler OffBoardRouteHandler;
/*! Create and initialize a new RouteHandler object

  @param context NB_Context
  @param callback Request handler status update callback
  @param parameters NB_RouteParameters object specifying the type of route to retrieve
  @param handler On success, the newly created handler; NULL otherwise
  @returns NB_Error
*/
NB_Error OffboardRouteHandlerSetup(NB_Context*      context,
                                   NB_RouteHandler* handler);

NB_Error OffboardRouteHandlerTearDown(NB_RouteHandler* handler);


/*! Start a network request to retrieve a route

  @param handler A NB_RouteHandler object
  @param parameters NB_RouteParameters object specifying the route parameters
  @returns NB_Error
*/
NB_Error OffboardRouteHandlerStartRequest(NB_RouteHandler* handler);


/*! Cancel a previously started request

  @param handler A NB_RouteHandler object
  @returns NB_Error
*/
NB_Error OffboardRouteHandlerCancelRequest(NB_RouteHandler* handler);

/*! Retrieves a NB_Route object containing the route

  @param handler A NB_RouteHandler object
  @param route On success, a NB_Route object with the result of the last download;
         NULL otherwise.  An object returned from this function must be destroyed
         using NB_RouteDestroy().
  @returns NB_Error
*/
NB_Error OffboardRouteHandlerGetRouteInformation(NB_RouteHandler* handler,
                                                 NB_RouteInformation** route);
#endif /* _OFFBOARDROUTEHANDLER_H_ */
