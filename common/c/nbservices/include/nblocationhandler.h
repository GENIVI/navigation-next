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

    @file nblocationhander.h
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

#ifndef NB_LOCATIONHANDLER_H
#define NB_LOCATIONHANDLER_H

#include "nbcontext.h"
#include "nblocationparameters.h"
#include "nblocationinformation.h"
#include "nbhandler.h"

/*!
    @addtogroup nblocationhandler
    @{
*/

typedef struct NB_LocationHandler NB_LocationHandler;

/*! Create and initialize a new NB_LocationHandler object

@param context Address of current NB_Context
@param callback Address of NB_RequestHandler callback structure, specifying the desired callback function and user data to be used to report network progress
@param handler On success, the newly created handler; NULL otherwise
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_LocationHandler** handler);

/*! Destroy a previously created NB_LocationHandler object

@param handler An NB_LocationHandler object created with NB_LocationHandlerCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationHandlerDestroy(NB_LocationHandler* handler);

/*! Start a network request to perform a location query

@param handler An NB_LocationHandler object
@param parameters Request parameters
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationHandlerStartRequest(NB_LocationHandler* handler, NB_LocationParameters* parameters);

/*! Cancel a previously started request

@param handler An NB_LocationHandler
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationHandlerCancelRequest(NB_LocationHandler* handler);

/*! Check if a download request is in progress

This function reports if a server request is currently in progress.

@param handler An NB_LocationHandler object
@returns Non-zero if a request is in progress; zero otherwise
*/
NB_DEC nb_boolean NB_LocationHandlerIsRequestInProgress(NB_LocationHandler* handler);

/*! Retrieves an NB_LocationInformation containing the result of the request query

@param handler An NB_LocationHandler object
@param info On success, a NB_LocationInformation object with the result of the last request; NULL otherwise. An object returned from this function must be destroyed using NB_LocationInformationDestroy.
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationHandlerGetInformation(NB_LocationHandler* handler, NB_LocationInformation** info);

/*! @} */

#endif  // NB_LOCATIONHANDLER_H

