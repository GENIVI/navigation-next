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

   @file     nbmetadatahandler.h
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NBMETADATAHANDLER_H
#define NBMETADATAHANDLER_H


#include "nbexp.h"
#include "nbcontext.h"
#include "nbhandler.h"
#include "nbmetadataparameters.h"
#include "nbmetadatainformation.h"


/*! @addtogroup nbmetadatahandler
    @{
*/


/*! @struct NB_MetadataHandler
*/
typedef struct NB_MetadataHandler NB_MetadataHandler;


/*! Create and initialize a new NB_MetadataHandler object

@param context NB_Context
@param callback Request handler status update callback
@param handler On success, the newly created handler; NULL otherwise
@returns NB_Error
*/
NB_DEC NB_Error NB_MetadataHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_MetadataHandler** handler);


/*! Destroy a previously created NB_MetadataHandler object

@param handler An NB_MetadataHandler object created with NB_MetadataHandlerCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_MetadataHandlerDestroy(NB_MetadataHandler* handler);


/*! Start a network request

@param handler An NB_MetadataHandler object
@param parameters Parameters for this request
@returns NB_Error
*/
NB_DEC NB_Error NB_MetadataHandlerStartRequest(NB_MetadataHandler* handler, NB_MetadataParameters* parameters);


/*! Cancel a previously started request

@param handler An NB_MetadataHandler object
@returns NB_Error
*/
NB_DEC NB_Error NB_MetadataHandlerCancelRequest(NB_MetadataHandler* handler);


/*! Check if a download request is in progress

@param handler An NB_MetadataHandler object
@returns Non-zero if a request is in progress; zero otherwise
*/
NB_DEC nb_boolean NB_MetadataHandlerIsRequestInProgress(NB_MetadataHandler* handler);


/*! Retrieves NB_MetadataInformation containing the result of the query

@param handler An NB_MetadataHandler object
@param information On success, a NB_MetadataInformation object with the result of the last query; NULL otherwise.  An object returned from this function must be destroyed using NB_MetadataInformationDestroy().
@returns NB_Error
*/
NB_DEC NB_Error NB_MetadataHandlerGetInformation(NB_MetadataHandler* handler, NB_MetadataInformation** information);


/*! @} */

#endif
