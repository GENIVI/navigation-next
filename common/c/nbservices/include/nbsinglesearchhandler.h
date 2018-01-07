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

    @file     nbsinglesearchhandler.h
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NBSINGLESEARCHHANDLER_H
#define NBSINGLESEARCHHANDLER_H

/*!
    @addtogroup nbsinglesearchhandler
    @{
*/

#include "nbhandler.h"
#include "nbcontext.h"
#include "nbsinglesearchparameters.h"
#include "nbsinglesearchinformation.h"

// Constants ....................................................................................


// Types ........................................................................................

typedef struct NB_SingleSearchHandler NB_SingleSearchHandler;


// Functions ....................................................................................

/*! Create a new instance of a NB_SingleSearchHandler object

    Use this function to create a new instance of a NB_SingleSearchHandler object.
    Call NB_SingleSearchHandlerDestroy() to destroy the object.

    @return NE_OK if success, NE_NOMEM if memory allocation failed
    @see NB_SingleSearchHandlerDestroy
*/
NB_DEC NB_Error
NB_SingleSearchHandlerCreate(
    NB_Context* context,                                                /*!< Pointer to current context */
    NB_RequestHandlerCallback* callback,                                /*!< Request handler callback for single search */
    NB_SingleSearchHandler** handler                                    /*!< On success, returns pointer to single search handler */
    );

/*! Destroy an instance of a NB_SingleSearchHandler object

    Use this function to destroy an existing instance of a NB_SingleSearchHandler object created by
    NB_SingleSearchHandlerCreate().

    @return None
    @see NB_SingleSearchHandlerCreate
 */
NB_DEC void
NB_SingleSearchHandlerDestroy(
    NB_SingleSearchHandler* handler                                     /*!< Single search handler instance */
    );

/*! Start a network request to perform a single search

    @return NE_OK if success
    @see NB_SingleSearchHandlerCancelRequest
*/
NB_DEC NB_Error
NB_SingleSearchHandlerStartRequest(
    NB_SingleSearchHandler* handler,                                    /*!< Single search handler instance */
    NB_SingleSearchParameters* parameters                               /*!< Single search parameter instance created by NB_SingleSearchParametersCreate*() functions */
    );

/*! Cancel a previously network request to perform a single search

    @return NE_OK if success
    @see NB_SingleSearchHandlerStartRequest
*/
NB_DEC NB_Error
NB_SingleSearchHandlerCancelRequest(
    NB_SingleSearchHandler* handler                                     /*!< Single search handler instance */
    );

/*! Check if a network request to perform a single search is in progress

    @return TRUE if a network request is in progress, FALSE otherwise
*/
NB_DEC nb_boolean
NB_SingleSearchHandlerIsRequestInProgress(
    NB_SingleSearchHandler* handler                                     /*!< Single search handler instance */
    );

/*! Get a NB_SingleSearchInformation object containing the results of the last single search request

    This function should be called in the NB_RequestHandlerCallback callback function when the request is successful.
    The single search information will not be available after this function returns. If this function is called it becomes
    the responsibility of the caller to destroy the returned NB_SingleSearchInformation object using NB_SingleSearchInformationDestroy()
    function when they are finished with it.

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SingleSearchHandlerGetInformation(
    NB_SingleSearchHandler* handler,                                    /*!< Single search handler instance */
    NB_SingleSearchInformation** information                            /*!< On return a NB_SingleSearchInformation object.
                                                                             An object returned from this function must be destroyed using NB_SingleSearchInformationDestroy() function */
    );

/*! @} */

#endif
