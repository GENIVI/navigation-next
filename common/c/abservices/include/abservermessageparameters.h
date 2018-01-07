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

    @file     abservermessageparameters.h
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

#ifndef ABSERVERSERVERMESSAGEPARAMETERS_H
#define ABSERVERSERVERMESSAGEPARAMETERS_H


#include "abexp.h"


/*!
    @addtogroup abservermessageparameters
    @{
*/


/*! @struct AB_ServerMessageParameters
    Opaque data structure that defines the parameters of a ServerMessage operation
*/
typedef struct AB_ServerMessageParameters AB_ServerMessageParameters;


/*! Nag time value for nag messages that are to never expire. */
#define MESSAGE_NAG_EXPIRE_NEVER	0xFFFFFFFF


/*! Enumeration of message query for AB_ServerMessageSetup(). */
typedef enum
{
    ASMQT_MessageQuery = 0,            /*!< Query for requested messages */
    ASMQT_MessageConfirm,              /*!< Notify that a message has been confirmed */
    ASMQT_MessageStatus                /*!< Query if any MOTDs are pending */
} AB_ServerMessageQueryType;


/*! Combination of flags that can be specified during a call to any of the AB_ServerMessageParametersCreateXXX functions. */
typedef enum
{
    AB_None = 0,                       /*!< No POI extended configuration is requested */
    AB_WantNotification = (1 << 0),    /*!< Request premium placement Ad in the response */
} AB_MessageParameterConfiguration;


/*! Create parameters for a server message status query request

@param context NB_Context instance
@param language Desired language
@param timestamp The timestamp of the latest message processed; specify 0 if no messages have been processed
@param parameters On success, a newly created AB_ServerMessageParameters object; NULL otherwise.  A valid object must be destroyed using AB_ServerMessageParametersDestroy().
@returns NB_Error
*/
AB_DEC NB_Error AB_ServerMessageParametersCreateStatus(NB_Context* context, const char* language, uint64 timestamp, AB_ServerMessageParameters** parameters);

/*! Create parameters for a server notification message download query request
 
 @param context NB_Context instance
 @param language Desired language
 @param timestamp The timestamp of the latest message processed; specify 0 if no messages have been processed
 @param config Want element for notification message
 @param parameters On success, a newly created AB_ServerMessageParameters object; NULL otherwise.  A valid object must be destroyed using AB_ServerMessageParametersDestroy().
 @returns NB_Error
 */

AB_DEC NB_Error AB_ServerMessageParametersNotificationCreateQuery(NB_Context* context, const char* language, uint64 timestamp, AB_MessageParameterConfiguration config, AB_ServerMessageParameters** parameters);

/*! Create parameters for a server message download query request

If all of the messages downloaded are succesfully processed by the client, the timestamp associated with the messages must be saved on the client
to be used in subsequent calls to AB_ServerMessageParametersCreate functions.  This ensures that a client sees every message exactly once

@param context NB_Context instance
@param language Desired language
@param timestamp The timestamp of the latest message processed; specify 0 if no messages have been processed
@param parameters On success, a newly created AB_ServerMessageParameters object; NULL otherwise.  A valid object must be destroyed using AB_ServerMessageParametersDestroy().
@returns NB_Error
*/
AB_DEC NB_Error AB_ServerMessageParametersCreateQuery(NB_Context* context, const char* language, uint64 timestamp, AB_ServerMessageParameters** parameters);


/*! Create parameters for a server message confirmation request

This function notifies the server that a user has chosen an action for a message.  If the confirm flag of a message is true, then the client must send the
user confirmation action (accept-text, decline-text) with these parameters

@param context NB_Context instance
@param messageId The id of the message being confirmed
@param action The action taken by the user.  This value is either the accept-text or decline-text of the message
@param parameters On success, a newly created AB_ServerMessageParameters object; NULL otherwise.  A valid object must be destroyed using AB_ServerMessageParametersDestroy().
@returns NB_Error
*/
AB_DEC NB_Error AB_ServerMessageParametersCreateConfirm(NB_Context* context, const char* messageId, const char* action, AB_ServerMessageParameters** parameters);


/*! Create parameters for a server message review confirmed message request

This function retrieves the message that the user previously confirmed

@param context NB_Context instance
@param messageType The type of the message that was confirmed
@param parameters On success, a newly created AB_ServerMessageParameters object; NULL otherwise.  A valid object must be destroyed using AB_ServerMessageParametersDestroy().
@returns NB_Error
*/
AB_DEC NB_Error AB_ServerMessageParametersCreateReview(NB_Context* context, const char* messageType, AB_ServerMessageParameters** parameters);


/*! Destroy a previously created ServerMessageParameters object

@param parameters An AB_ServerMessageParameters object created with AB_ServerMessageParametersCreate()
@returns NB_Error
*/
AB_DEC NB_Error AB_ServerMessageParametersDestroy(AB_ServerMessageParameters* parameters);


/*! @} */

#endif
