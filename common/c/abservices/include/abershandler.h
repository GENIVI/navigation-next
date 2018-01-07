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

@file abershandler.h
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

#ifndef AB_ERS_HANDLER_H
#define AB_ERS_HANDLER_H

#include "nbcontext.h"
#include "abersparameters.h"
#include "abersinformation.h"
#include "nbhandler.h"

/*!
    @addtogroup abershandler
    @{
*/

typedef struct AB_ErsHandler AB_ErsHandler;

/*! Create and initialize a new AB_ErsHandler object

@param context Address of current NB_Context
@param callback Address of NB_RequestHandler callback structure, specifying the desired callback function and user data to be used to report network progress
@param handler On success, the newly created handler; NULL otherwise
@returns NB_Error
*/
AB_DEC NB_Error AB_ErsHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, AB_ErsHandler** handler);

/*! Destroy a previously created AB_ErsHandler object

@param handler An AB_ErsHandler object created with AB_ErsHandlerCreate()
@returns NB_Error
*/
AB_DEC NB_Error AB_ErsHandlerDestroy(AB_ErsHandler* handler);

/*! Start a network request to perform a authentication operation

@param handler An AB_ErsHandler object
@param parameters Request parameters
@returns NB_Error
*/
AB_DEC NB_Error AB_ErsHandlerStartRequest(AB_ErsHandler* handler, AB_ErsParameters* parameters);

/*! Cancel a previously started request

@param handler An AB_ErsHandler
@returns NB_Error
*/
AB_DEC NB_Error AB_ErsHandlerCancelRequest(AB_ErsHandler* handler);

/*! Check if a download request is in progress

This function reports if a server request is currently in progress.

@param handler An AB_ErsHandler object
@returns Non-zero if a request is in progress; zero otherwise
*/
AB_DEC nb_boolean AB_ErsHandlerIsRequestInProgress(AB_ErsHandler* handler);

/*! Retrieves an AB_ErsInformation containing the result of the request query

@param handler An AB_ErsHandler object
@param info On success, a AB_ErsInformation object with the result of the last request; NULL otherwise. An object returned from this function must be destroyed using AB_ErsInformationDestroy.
@returns NB_Error
*/
AB_DEC NB_Error AB_ErsHandlerGetErsInformation(AB_ErsHandler* handler, AB_ErsInformation** info);


/*! @} */

#endif
