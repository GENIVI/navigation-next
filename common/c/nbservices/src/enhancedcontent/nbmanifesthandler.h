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

    @file     nbmanifesthandler.h
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef NBMANIFESTHANDLER_H
#define NBMANIFESTHANDLER_H


#include "nbexp.h"
#include "nbcontext.h"
#include "nbhandler.h"
#include "nbmanifestparameters.h"
#include "nbmanifestinformation.h"


/*! @addtogroup nbmanifesthandler
    @{
*/


/*! @struct NB_ManifestHandler
*/
typedef struct NB_ManifestHandler NB_ManifestHandler;


/*! Create and initialize a new NB_ManifestHandler object

@param context NB_Context
@param callback Request handler status update callback
@param handler On success, the newly created handler; NULL otherwise
@returns NB_Error
*/
NB_Error NB_ManifestHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_ManifestHandler** handler);


/*! Destroy a previously created NB_ManifestHandler object

@param handler An NB_ManifestHandler object created with NB_ManifestHandlerCreate()
@returns NB_Error
*/
NB_Error NB_ManifestHandlerDestroy(NB_ManifestHandler* handler);


/*! Start a network request

@param handler An NB_ManifestHandler object
@param parameters Parameters for this request
@returns NB_Error
*/
NB_Error NB_ManifestHandlerStartRequest(NB_ManifestHandler* handler, NB_ManifestParameters* parameters);


/*! Cancel a previously started request

@param handler An NB_ManifestHandler object
@returns NB_Error
*/
NB_Error NB_ManifestHandlerCancelRequest(NB_ManifestHandler* handler);


/*! Check if a download request is in progress

@param handler An NB_ManifestHandler object
@returns Non-zero if a request is in progress; zero otherwise
*/
nb_boolean NB_ManifestHandlerIsRequestInProgress(NB_ManifestHandler* handler);


/*! Retrieves NB_ManifestInformation containing the result of the query

@param handler An NB_ManifestHandler object
@param information On success, a NB_ManifestInformation object with the result of the last query; NULL otherwise.  An object returned from this function must be destroyed using NB_ManifestInformationDestroy().
@returns NB_Error
*/
NB_Error NB_ManifestHandlerGetInformation(NB_ManifestHandler* handler, NB_ManifestInformation** information);


/*! @} */

#endif
