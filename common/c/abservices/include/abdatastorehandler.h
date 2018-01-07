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

    @file     abdatastorehandler.h
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

#ifndef ABDATASTOREHANDLER_H
#define ABDATASTOREHANDLER_H


#include "abexp.h"
#include "nbcontext.h"
#include "nbhandler.h"
#include "abdatastoreparameters.h"
#include "abdatastoreinformation.h"


/*! @addtogroup abdatastorehandler
    @{
*/


/*! @struct AB_DataStoreHandler
    A DataStoreHandler is used to store, retrieve, or delete buffers of data on the server
*/
typedef struct AB_DataStoreHandler AB_DataStoreHandler;


/*! Create and initialize a new AB_DataStoreHandler object

@param context NB_Context
@param callback Request handler status update callback
@param handler On success, the newly created handler; NULL otherwise
@returns NB_Error
*/
AB_DEC NB_Error AB_DataStoreHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, AB_DataStoreHandler** handler);


/*! Destroy a previously created AB_DataStoreHandler object

@param handler An AB_DataStoreHandler object created with AB_DataStoreHandlerCreate()
@returns NB_Error
*/
AB_DEC NB_Error AB_DataStoreHandlerDestroy(AB_DataStoreHandler* handler);


/*! Start a network request to a data store

@param handler An AB_DataStoreHandler object
@param parameters Parameters for this request
@returns NB_Error
*/
AB_DEC NB_Error AB_DataStoreHandlerStartRequest(AB_DataStoreHandler* handler, AB_DataStoreParameters* parameters);


/*! Cancel a previously started request

@param handler An AB_DataStoreHandler object
@returns NB_Error
*/
AB_DEC NB_Error AB_DataStoreHandlerCancelRequest(AB_DataStoreHandler* handler);


/*! Check if a download request is in progress

@param handler An AB_DataStoreHandler object
@returns Non-zero if a request is in progress; zero otherwise
*/
AB_DEC nb_boolean AB_DataStoreHandlerIsRequestInProgress(AB_DataStoreHandler* handler);


/*! Retrieves AB_DataStoreInformation containing the result of the data store operation

@param handler An AB_DataStoreHandler object
@param information On success, a AB_DataStoreInformation object with the result of the last download; NULL otherwise.  An object returned from this function must be destroyed using AB_DataStoreInformationDestroy().
@returns NB_Error
*/
AB_DEC NB_Error AB_DataStoreHandlerGetInformation(AB_DataStoreHandler* handler, AB_DataStoreInformation** information);


/*! @} */

#endif
