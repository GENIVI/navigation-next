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

    @file abspeechstatisticshandler.h
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

#ifndef ABSPEECHSTATISTICSHANDLER_H
#define ABSPEECHSTATISTICSHANDLER_H


#include "abexp.h"
#include "nbcontext.h"
#include "nbhandler.h"
#include "abspeechstatisticsparameters.h"


/*! @addtogroup abspeechstatisticshandler
    @{
*/


/*! @struct AB_SpeechStatisticsHandler
    A SpeechRecognitionHandler handles speech recognition requests from the server
*/
typedef struct AB_SpeechStatisticsHandler AB_SpeechStatisticsHandler;


/*! Create and initialize a new AB_SpeechStatisticsHandler object

@param context NB_Context
@param callback Request handler status update callback
@param handler On success, the newly created handler; NULL otherwise
@returns NB_Error
*/
AB_DEC NB_Error AB_SpeechStatisticsHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, AB_SpeechStatisticsHandler** handler);


/*! Destroy a previously created AB_SpeechStatisticsHandler object

@param handler An AB_SpeechStatisticsHandler object created with AB_SpeechStatisticsHandlerCreate()
@returns NB_Error
*/
AB_DEC NB_Error AB_SpeechStatisticsHandlerDestroy(AB_SpeechStatisticsHandler* handler);


/*! Start a network request to a data store

@param handler An AB_SpeechStatisticsHandler object
@param parameters Parameters for this request
@returns NB_Error
*/
AB_DEC NB_Error AB_SpeechStatisticsHandlerStartRequest(AB_SpeechStatisticsHandler* handler, AB_SpeechStatisticsParameters* parameters);


/*! Cancel a previously started request

@param handler An AB_SpeechStatisticsHandler object
@returns NB_Error
*/
AB_DEC NB_Error AB_SpeechStatisticsHandlerCancelRequest(AB_SpeechStatisticsHandler* handler);


/*! Check if a download request is in progress

@param handler An AB_SpeechStatisticsHandler object
@returns Non-zero if a request is in progress; zero otherwise
*/
AB_DEC nb_boolean AB_SpeechStatisticsHandlerIsRequestInProgress(AB_SpeechStatisticsHandler* handler);


/*! @} */

#endif
