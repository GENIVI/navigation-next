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

    @file     nbsearchhandler.h
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

#ifndef NBSEARCHHANDLER_H
#define NBSEARCHHANDLER_H

#include "nbexp.h"
#include "nbhandler.h"
#include "nbrouteinformation.h"
#include "nbsearchparameters.h"
#include "nbsearchinformation.h"

/*!
    @addtogroup nbsearchhandler
    @{
*/

/*! @struct NB_SearchHandler
A SearchHandler is used to interact with the SeachBuilder servlet
*/
typedef struct NB_SearchHandler NB_SearchHandler;


/*! Create and initialize a new SearchHandler object

@ingroup searchhandler
@param context NB_Context
@param callback Request handler status update callback
@param handler On success, the newly created handler; NULL otherwise
@returns NB_Error
*/
NB_DEC NB_Error NB_SearchHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_SearchHandler** handler);


/*! Destroy a previously created SearchHandler object

@ingroup searchhandler
@param handler A NB_SearchHandler object created with NB_SearchHandlerCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_SearchHandlerDestroy(NB_SearchHandler* handler);


/*! Start a network request to perform a SearchBuilder search

Initiate a network request to perform a SearchBuilder search.  The search to perform is specified in
NB_SearchHandlerCreate() or NB_SearchHandlerReset().  Only one request may be active at a given time

@ingroup searchhandler
@param handler A NB_SearchHandler object
@param parameters Parameters object created by NB_SearchParametersCreate*() functions.
@returns NB_Error
*/
NB_DEC NB_Error NB_SearchHandlerStartRequest(NB_SearchHandler* handler, NB_SearchParameters* parameters);


/*! Cancel a previously started request

@ingroup searchhandler
@param handler A NB_SearchHandler object
@returns NB_Error
*/
NB_DEC NB_Error NB_SearchHandlerCancelRequest(NB_SearchHandler* handler);


/*! Check if a download request is in progress

@ingroup searchhandler
@param handler A NB_SearchHandler object
@returns Non-zero if a request is in progress; zero otherwise
*/
NB_DEC nb_boolean NB_SearchHandlerIsRequestInProgress(NB_SearchHandler* handler);


/*! Retrieves a NB_SearchInformation object containing the results of the last download

Retrieves a NB_SearchInformation object containing the results of the last download.  This function must be
called in the RequestHandler callback function.  The Search Information will not be available after that function
returns.  If this function is called, it becomes the responsibility of the caller to free the returned 
NB_SearchInformation when they are finished with it.

@ingroup searchhandler
@param handler A NB_SearchHandler object
@param searchInformation On success, a NB_SearchInformation object with the result of the last download; NULL otherwise.  An object returned from this function must be destroyed using NB_SearchInformationDestroy().
@returns NB_Error
*/
NB_DEC NB_Error NB_SearchHandlerGetSearchInformation(NB_SearchHandler* handler, NB_SearchInformation** searchInformation);

/*! @} */

#endif  // NBSEARCHHANDLER_H

