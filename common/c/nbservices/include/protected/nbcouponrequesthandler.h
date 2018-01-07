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

    @file     nbcouponrequesthandler.h
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems, Inc. is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef NBCOUPONREQUESTHANDLER_H
#define NBCOUPONREQUESTHANDLER_H

#include "nbcontext.h"
#include "nbcouponrequestinformation.h"
#include "nbcouponrequestparameters.h"
#include "nbhandler.h"

/*!
    @addtogroup nbcouponrequesthandler
    @{
*/

/*! @struct NB_CouponRequestHandler
Opaque data structure that defines the coupon handler
*/
typedef struct NB_CouponRequestHandler NB_CouponRequestHandler;

/*! Create and initialize a new CouponRequestHandler object

@param context NB_Context
@param callback Request handler status update callback
@param handler On success, the newly created handler; NULL otherwise
@return NB_Error
*/
NB_DEC NB_Error NB_CouponRequestHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_CouponRequestHandler** handler);


/*! Destroy a previously created CouponRequestHandler object

@param handler A NB_CouponRequestHandler object created with NB_CouponRequestHandlerCreate()
@return NB_Error
*/
NB_DEC NB_Error NB_CouponRequestHandlerDestroy(NB_CouponRequestHandler* handler);


/*! Start a network request to get coupons

@param handler A NB_CouponRequestHandler object
@param parameters NB_CouponRequestParameters object specifying the coupon parameters
@return NB_Error
*/
NB_DEC NB_Error NB_CouponRequestHandlerStartRequest(NB_CouponRequestHandler* handler, NB_CouponRequestParameters* parameters);


/*! Cancel a previously started request

@param handler A NB_CouponRequestHandler object
@return NB_Error
*/
NB_DEC NB_Error NB_CouponRequestHandlerCancelRequest(NB_CouponRequestHandler* handler);


/*! Check if a download request is in progress

@param handler A NB_CouponRequestHandler object
@return Non-zero if a request is in progress; zero otherwise
*/
NB_DEC nb_boolean NB_CouponRequestHandlerIsRequestInProgress(NB_CouponRequestHandler* handler);


/*! Retrieves a NB_CouponRequestInformation object containing the results of the operation

@param handler A NB_CouponRequestHandler object
@param information On success, a NB_CouponRequestInformation object with the result of the last download; NULL otherwise.  An object returned from this function must be destroyed using NB_CouponRequestInformationDestroy().
@return NB_Error
*/
NB_DEC NB_Error NB_CouponRequestHandlerGetInformation(NB_CouponRequestHandler* handler, NB_CouponRequestInformation** information);


/*! @} */

#endif
