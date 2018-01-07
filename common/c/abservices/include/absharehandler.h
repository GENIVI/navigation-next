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

    @file absharehandler.h
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

#ifndef AB_SHAREHANDLER_H
#define AB_SHAREHANDLER_H

#include "nbcontext.h"
#include "nbhandler.h"
#include "abshareparameters.h"
#include "abshareinformation.h"


/*!
    @addtogroup absharehandler
    @{
*/


/*! @struct AB_ShareHandler
    Opaque data structure that provides server communication functionality for share message operations
*/
typedef struct AB_ShareHandler AB_ShareHandler;


/*! Create and initialize a new AB_ShareHandler object

This function creates an AB_ShareHandler object.  The created object can then submit requests defined
by the AB_ShareParameters object.  The created handler object should be freed using AB_ShareHandlerDestroy()
when no longer needed.

@param context Address of current NB_Context
@param callback Address of NB_RequestHandler callback structure, specifying the desired callback function and user data to be used to report network progress
@param handler On success, the newly created handler; NULL otherwise

@returns NB_Error

@see AB_ShareHandlerDestroy
*/
AB_DEC NB_Error AB_ShareHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, AB_ShareHandler** handler);


/*! Destroy a previously created AB_ShareHandler object

@param handler An AB_ShareHandler object created with AB_ShareHandlerCreate()

@returns NB_Error

@see AB_ShareHandlerCreate
*/
AB_DEC NB_Error AB_ShareHandlerDestroy(AB_ShareHandler* handler);


/*! Start a network request to perform a share message operation

This function submits a share message operation request to the server, defined by the specified
AB_ShareParameters object.  Request progress and completion will be reported via the callback
function specified in the NB_RequestHandlerCallback structure at Handler object creation.

@param handler An AB_ShareHandler object
@param parameters Request parameters

@returns NB_Error
*/
AB_DEC NB_Error AB_ShareHandlerStartRequest(AB_ShareHandler* handler, AB_ShareParameters* parameters);

/*! Cancel a previously started request

This function cancels a request in progress.

@param handler An AB_ShareHandler 

@returns NB_Error
*/
AB_DEC NB_Error AB_ShareHandlerCancelRequest(AB_ShareHandler* handler);


/*! Check if a download request is in progress

This function reports if a server request is currently in progress.

@param handler An AB_ShareHandler object

@returns Non-zero if a request is in progress; zero otherwise
*/
AB_DEC nb_boolean AB_ShareHandlerIsRequestInProgress(AB_ShareHandler* handler);


/*! Retrieves an AB_ShareInformation containing the result of the request query

This function will create an return an AB_ShareInformation object containing the data
returned from the last server request and that provides accessor functions to retrieve data.
This function may only be called and is only valid from within the network callback function
that indicates the service request was successful and is complete.  The AB_ShareInformation
should be destroyed with AB_ShareInformationDestroy() when no longer needed.

@param handler An AB_ShareHandler object
@param info On success, a AB_ShareInformation object with the result of the last request; NULL otherwise.  An object returned from this function must be destroyed using AB_ShareInformationDestroy.

@returns NB_Error

@see AB_ShareInformationDestroy()
*/
AB_DEC NB_Error AB_ShareHandlerGetShareInformation(AB_ShareHandler* handler, AB_ShareInformation** info);


/*! @} */

#endif
