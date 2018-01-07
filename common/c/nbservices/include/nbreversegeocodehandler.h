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

    @file     nbreversegeocodehandler.h
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

#ifndef NBREVERSEGEOCODEHANDLER_H
#define NBREVERSEGEOCODEHANDLER_H

#include "nbcontext.h"
#include "nbhandler.h"
#include "nbreversegeocodeinformation.h"
#include "nbreversegeocodeparameters.h"

/*!
    @addtogroup nbreversegeocodehandler
    @{
*/


typedef struct NB_ReverseGeocodeHandler NB_ReverseGeocodeHandler;


/*! Create and initialize a new ReverseGeocodeHandler object

@param context NB_Context
@param callback Request handler status update callback
@param handler On success, the newly created handler; NULL otherwise
@returns NB_Error
*/
NB_DEC NB_Error NB_ReverseGeocodeHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_ReverseGeocodeHandler** handler);


/*! Destroy a previously created ReverseGeocodeHandler object

@param handler A NB_ReverseGeocodeHandler object created with NB_ReverseGeocodeHandlerCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_ReverseGeocodeHandlerDestroy(NB_ReverseGeocodeHandler* handler);


/*! Start a network request to a reverse geocode

@param handler A NB_ReverseGeocodeHandler object
@param parameters NB_ReverseGeocodeParameters object specifying the geocode parameters
@returns NB_Error
*/
NB_DEC NB_Error NB_ReverseGeocodeHandlerStartRequest(NB_ReverseGeocodeHandler* handler, NB_ReverseGeocodeParameters* parameters);


/*! Cancel a previously started request

@param handler A NB_ReverseGeocodeHandler object
@returns NB_Error
*/
NB_DEC NB_Error NB_ReverseGeocodeHandlerCancelRequest(NB_ReverseGeocodeHandler* handler);


/*! Check if a download request is in progress

@param handler A NB_ReverseGeocodeHandler object
@returns Non-zero if a request is in progress; zero otherwise
*/
NB_DEC nb_boolean NB_ReverseGeocodeHandlerIsRequestInProgress(NB_ReverseGeocodeHandler* handler);


/*! Retrieves an NB_Location object containing the result of the reverse geocode operation

@param handler A NB_ReverseGeocodeHandler object
@param information On success, a NB_ReverseGeocodeInformation object with the result of the last download; NULL otherwise.  An object returned from this function must be destroyed using NB_ReverseGeocodeInformationDestroy().
@returns NB_Error
*/
NB_DEC NB_Error NB_ReverseGeocodeHandlerGetReverseGeocodeInformation(NB_ReverseGeocodeHandler* handler, NB_ReverseGeocodeInformation** information);


/*! @} */

#endif
