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

    @file absynchronizationhandler.h
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

#ifndef ABSYNCHRONIZATIONHANDLER_H
#define ABSYNCHRONIZATIONHANDLER_H

#include "abexp.h"
#include "absynchronizationparameters.h"
#include "absynchronizationinformation.h"
#include "nbcontext.h"
#include "nbhandler.h"
#include "nberror.h"


/*!
    @addtogroup absynchandler
    @{
*/


/*! @struct AB_SynchronizationHandler
    An AB_SynchronizationHandler....
*/
typedef struct AB_SynchronizationHandler AB_SynchronizationHandler;


/*! Create and initialize a new AB_SynchronizationHandler object

@param context NB_Context
@param callback Request handler status update callback
@param parameters AB_SynchronizationParameters object specifying the parameters
@param handler On success, the newly created handler; NULL otherwise
@returns NB_Error
*/
AB_DEC NB_Error AB_SynchronizationHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, AB_SynchronizationHandler** handler);


/*! Destroy a previously created AB_SynchronizationHandler object

@param handler An AB_SynchronizationHandler object created with AB_SynchronizationHandlerCreate()
@returns NB_Error
*/
AB_DEC NB_Error AB_SynchronizationHandlerDestroy(AB_SynchronizationHandler* handler);


/*! Start a network request to a reverse geocode

@param handler An AB_SynchronizationHandler object
@returns NB_Error
*/
AB_DEC NB_Error AB_SynchronizationHandlerStartRequest(AB_SynchronizationHandler* handler, AB_SynchronizationParameters* parameters);


/*! Cancel a previously started request

@param handler An AB_SynchronizationHandler object
@returns NB_Error
*/
AB_DEC NB_Error AB_SynchronizationHandlerCancelRequest(AB_SynchronizationHandler* handler);


/*! Check if a status request is in progress

@param handler An AB_SynchronizationHandler object
@returns Non-zero if a request is in progress; zero otherwise
*/
AB_DEC nb_boolean AB_SynchronizationHandlerIsRequestInProgress(AB_SynchronizationHandler* handler);


/*! Retrieves an AB_SynchronizationHandler object containing the result of the synchronization query

@param handler An AB_SynchronizationHandler object
@param information On success, a AB_SynchronizationHandler object with the result of the last request; NULL otherwise.
An object returned from this function must be destroyed using AB_SynchronizationInformationDestroy().
@returns NB_Error
*/
AB_DEC NB_Error AB_SynchronizationHandlerGetInformation(AB_SynchronizationHandler* handler, AB_SynchronizationInformation** information);


/*! @} */

#endif
