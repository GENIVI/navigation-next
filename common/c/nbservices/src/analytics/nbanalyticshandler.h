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

    @file     nbanalyticshandler.h
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

#ifndef NBANALYTICSHANDLER_H
#define NBANALYTICSHANDLER_H

#include "nbcontext.h"
#include "nbhandler.h"
#include "nbanalyticsinformation.h"
#include "nbanalyticsparameters.h"
#include "vec.h"
/*!
    @addtogroup nbanalyticshandler
    @{
*/


typedef struct NB_AnalyticsHandler NB_AnalyticsHandler;


/*! Create and Initialize a new AnalyticsHandler object

@param context NB_Context
@param callback Request handler status update callback
@param handler On success, the newly created handler; NULL otherwise
@returns NB_Error
*/
NB_Error NB_AnalyticsHandlerCreate(NB_Context* context,
        NB_RequestHandlerCallback* callback, NB_AnalyticsHandler** handler);


/*! Destroy a previously created AnalyticsHandler object

@param handler A NB_AnalyticsHandler object created with
               NB_AnalyticsHandlerCreate()
@returns NB_Error
*/
NB_Error NB_AnalyticsHandlerDestroy(NB_AnalyticsHandler* handler);

/*! Start a network request to a reverse geocode

@param handler A NB_AnalyticsHandler object
@param parameters NB_AnalyticsParameters object specifying the geocode parameters
@returns NB_Error
*/
NB_Error NB_AnalyticsHandlerStartRequest(NB_AnalyticsHandler* handler,
        NB_AnalyticsParameters* parameters);

/*! Start a network analytics request.

@param handler An NB_AnalyticsHandler object
@param parameters NB_AnalyticsParameters object
@params events CSL_Vector structure, specifies events body.
@returns NB_Error
*/
NB_Error NB_AnalyticsHandlerStartRequestEx(NB_AnalyticsHandler* handler,
        NB_AnalyticsParameters* parameters,CSL_Vector* events);


/*! Cancel a previously started request

@param handler A NB_AnalyticsHandler object
@returns NB_Error
*/
NB_Error NB_AnalyticsHandlerCancelRequest(NB_AnalyticsHandler* handler);


/*! Check if a download request is in progress

@param handler A NB_AnalyticsHandler object
@returns Non-zero if a request is in progress; zero otherwise
*/
nb_boolean NB_AnalyticsHandlerIsRequestInProgress(
        NB_AnalyticsHandler* handler);


/*! Retrieves an NB_AnalyticsInformation object containing the result of the
    analytics-event-query

@param handler A NB_AnalyticsHandler object
@param information On success, a NB_AnalyticsInformation object with the result
of the last query; NULL otherwise.  An object returned from this function must
be destroyed using NB_AnalyticsInformationDestroy().
@returns NB_Error
*/
NB_Error NB_AnalyticsHandlerGetInformation(
        NB_AnalyticsHandler* handler, NB_AnalyticsInformation** information);


/*! @} */

#endif

