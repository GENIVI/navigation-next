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

    @file     nbhandler.h
    @defgroup nbhandler Handler Support
*/
/*
    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef NBHANDLER_H
#define NBHANDLER_H

#include "paltypes.h"
#include "nberror.h"
#include "nbnetwork.h"


/*! @{ */

typedef NB_NetworkRequestStatus NB_RequestStatus;

/*! Handler callback function signature

A handler may invoke this callback multiple times during the course of a server request.  It may be called when
the query is being sent to the server, when the reply is received from the server or in the case of a network
error

@param handler The handler that is invoking the callback
@param status The status of the call
@param err A specific error code associated with the status
@param up Non-zero when the query is being sent to the server; zero when the reply is being received from the server
@param percent An integer value between 0 and 100, inclusive, that indicates how much of the query has been sent or how much of the reply has been received
@param userData A pointer to opaque user data
*/
typedef void (*NB_RequestHandlerCallbackFunction)(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);


/*! Defines the request handler callback to invoke and specifies the user data to be supplied to the callback
*/
typedef struct 
{
    NB_RequestHandlerCallbackFunction   callback;
	void*                               callbackData;
} NB_RequestHandlerCallback;


/*! @} */

#endif
