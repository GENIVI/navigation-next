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

    @file abservermessageinformation.h
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

#ifndef ABSERVERMESSAGEINFORMATION_H
#define ABSERVERMESSAGEINFORMATION_H

#include "abexp.h"
#include "abservermessageparameters.h"
#include "nbplace.h"
#include "csltypes.h"


/*!
    @addtogroup abservermessageinformation
    @{
*/


/*! @struct AB_ServerMessageInformation
Information about the results of a server message request
*/
typedef struct AB_ServerMessageInformation AB_ServerMessageInformation;

/*! Message enumeration order */
typedef enum
{
    ASMEO_NoSorting,                    /*! No sorting */
    ASMEO_NewestFirst,                  /*! Newest first */
    ASMEO_OldestFirst,                  /*! Oldest first */
} AB_ServerMessageEnumerationOrder;


/*! Units of time for nag messages. */
typedef enum {
    ASNU_Days,                          /*!< Days */
    ASNU_Seconds,                       /*!< Seconds */
    ASNU_Uses                           /*!< Uses */
} AB_ServerMessageNagUnit;


/*! Definition of a nag message frequency and expiration. */
typedef struct {
    AB_ServerMessageNagUnit freq_unit;      /*!< Frequency unit */
    uint32				    freq_count;     /*!< Frequency count */
    AB_ServerMessageNagUnit	expire_unit;    /*!< Expire unit */
    uint32				    expire_count;   /*!< Expire count */
    uint32				    expire_date;    /*!< Expire date */
} AB_ServerMessageNag;


/*! Message information retrieved from server. */
typedef struct {
    char*               id;             /*!< Message ID */
    char*               title;          /*!< Message title */
    char*               text;           /*!< Message text */
    char*               type;           /*!< Message type */
    char*               language;       /*!< Message language */
    char*               accept_text;    /*!< Display text for accept */
    char*               center_text;    /*!< Display text for center */
    char*               decline_text;   /*!< Display txt for decline */
    char*               url;            /*!< Message url */

    AB_ServerMessageNag message_nag;    /*!< Message nag frequency  */

    nb_boolean          confirm;        /*!< Confirmation required */
    nb_boolean          exit_on_decline;/*!< App should exit if message declined */

    uint32              time;           /*!< Download time for comparisons */
} AB_ServerMessageDetail;


/*! Callback function signature for function passed into AB_ServerMessageEnumMessageText().

    This is the function signature required of the callback function required for
    AB_ServerMessageEnumMessageText().  This callback function will be called for each message
    text enumerated by AB_ServerMessageEnumMessageText().

    @return a NB_Error code, indicating success or failure

    @see AB_ServerMessageEnumMessageText
*/
typedef NB_Error (*AB_ServerMessageTextCallback)(
    NB_Font font,                               /*!< Font to be used for the message text */
    nb_color color,                             /*!< Color to be used for the message text */
    const char* text,                           /*!< Text of message */
    nb_boolean newLine,                         /*!< Indicates if new line should follow message text */
    void* userData                              /*!< User data provided at AB_ServerMessageEnumMessageText call  */
);


/*! Begins an enumeration of queried messages

This function initializes an enumeration of queried messages of the specified AB_Message.
The messages will be filtered and ordered based on the ORed bit flags provided.

AB_ServerMessageInformationEnumerateNext() will return the AB_ServerMessageDetail data for each filtered message.

@param information An AB_ServerMessageInformation object
@param messageType Message type to enumerate, all types if NULL or empty string
@param messageOrder Age order to enumerate messages in
@returns NB_Error
*/
AB_DEC NB_Error AB_ServerMessageInformationEnumerateInitialize(AB_ServerMessageInformation* information, const char* messageType, AB_ServerMessageEnumerationOrder messageOrder);


/*! Get next server message in enumeration

This function returns the AB_MessageInfo data structure for the next message in the AB_Message object current query.

@param information An AB_ServerMessageInformation object
@param serverMessageInfo Pointer to AB_MessageInfo pointer for the next message, AB_ServerMessageDetail pointer will be set to NULL if no more messages exist
@returns NB_Error
*/
AB_DEC nb_boolean AB_ServerMessageInformationEnumerateNext(AB_ServerMessageInformation* information, AB_ServerMessageDetail** serverMessageInfo);


/*! Requests message text and formatting callback for specified message ID

This function requests the message text and formatting info for a specified message ID.
The message info will be returned via the specified callback.

@param information An AB_ServerMessageInformation object
@param id  Message ID of message to retrieve
@param serverMessageTextCallback Pointer to AB_ServerMessageTextCallback callback function to be called with message info
@param userData Pointer to user data that will be included in the messageTextCallback function call
@returns NB_Error
*/
AB_DEC NB_Error AB_ServerMessageInformationEnumerateMessageText(AB_ServerMessageInformation* information, const char* id, AB_ServerMessageTextCallback serverMessageTextCallback, void* userData);


/*! Indicates if any messages are pending

This function indicates if messages are pending.  This value can be retrieved after a ASMQT_MessageStatus query has been made.

@param information An AB_ServerMessageInformation object
@returns boolean indicating if there are server messages pending
*/
AB_DEC nb_boolean AB_ServerMessageInformationGetMessagePending(AB_ServerMessageInformation* information);


/*! Returns the messages timestamp from server

This is function returns the server timestamp for the messages reply.

@param information An AB_ServerMessageInformation object
@param timeStamp Pointer to timestamp
@returns NB_Error
*/
AB_DEC NB_Error AB_ServerMessageInformationGetMessagesTimeStamp(AB_ServerMessageInformation* information, uint64* timeStamp);


/*! Destroy a previously created ServerMessageInformation object

@param information An AB_ServerMessageInformation object created with AB_ServerMessageHandlerGetInformation()
@returns NB_Error
*/
AB_DEC NB_Error AB_ServerMessageInformationDestroy(AB_ServerMessageInformation* information);


#endif
