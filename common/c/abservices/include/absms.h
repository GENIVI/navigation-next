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

    @file     absms.h
    @date     04/01/2009
    @defgroup SMS_API_GROUP SMS API

    This interface provides incoming SMS message processing for an
    AtlasBook navigation application.
  
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

/*! @{ */

#ifndef ABSMS_H
#define ABSMS_H

#include "abexp.h"
#include "paltypes.h"
#include "nberror.h"
#include "nbplace.h"

#define AB_SMS_VERSION_LEN          10
#define AB_SMS_CMD_LEN              10
#define AB_SMS_SYNC_DBS_LEN         200
#define AB_SMS_MSG_FROM_LEN         50
#define AB_SMS_MSG_SEARCH_LEN       100
#define AB_SMS_TRAFFIC_SESSION_LEN  100
#define AB_SMS_MESSAGE_LEN          160


/*! External reference for an AtlasBook AB_Sms object.

    This typedef provides an opaque reference to a created AtlasBook AB_Sms object.
*/
typedef struct _AB_Sms AB_Sms;

/*! SMS command types */
typedef enum
{
    ASCT_Unknown = 0,           /*!< Unknown/undefined SMS command type */
    ASCT_SynchronizeDatabase,   /*!< Synchronize database with server notification */
    ASCT_InboundMessage,        /*!< Inbound message available notification */
    ASCT_MessageReceipt,        /*!< Place message delivery notification */
    ASCT_TrafficSession         /*!< Traffic session notification */
} AB_SmsCommandType;

/*! SMS Synchronize Database command data */
typedef struct
{
    char    msgFrom[AB_SMS_MSG_FROM_LEN + 1];
    char    syncDatabases[AB_SMS_SYNC_DBS_LEN + 1];
} AB_SmsCommandSynchronizeDatabase;

/*! SMS Inbound Message command data */
typedef struct
{
    char    msgFrom[AB_SMS_MSG_FROM_LEN + 1];
    uint32	msgNum;
    uint32	msgTime;
    uint32	msgGeneration;
    char	msgFromName[AB_SMS_MSG_FROM_LEN + 1];
} AB_SmsCommandInboundMessage;

/*! SMS Message Receipt command data */
typedef struct
{
    char	msgFrom[AB_SMS_MSG_FROM_LEN + 1];
    char    msgResult;
    char    msgId[AB_SMS_MESSAGE_LEN + 1];
} AB_SmsCommandMessageReceipt;

/*! SMS Traffic Session Notfication command data */
typedef struct
{
    char    msgFrom[AB_SMS_MSG_FROM_LEN + 1];
    char    trafficSession[AB_SMS_TRAFFIC_SESSION_LEN + 1];
} AB_SmsCommandTrafficSession;


/*! Create a AB_Sms instance.

    This function creates and returns a pointer to an AB_Sms instance.
    Call AB_SmsDestroy() to destroy the object.

    @return A NB_Error code, indicating success or failure

    @see AB_SmsDestroy
*/
AB_DEC NB_Error
AB_SmsCreate(
    AB_Sms** sms,                       /*!< Pointer to AB_Sms pointer to store created object, will be set to NULL on failure */
    const byte* smsKey,                 /*!< SMS key for incoming SMS messages */
    nb_size smsKeyLength                /*!< Length of SMS key */
    );

/*! Destroy an AB_DataStore instance.

    This function destroys the given AB_DataStore instance that was created with AB_SmsCreate().

    @return a NB_Error code, indicating success or failure

    @see AB_SmsCreate
*/
AB_DEC NB_Error
AB_SmsDestroy(
    AB_Sms* sms                         /*!< Pointer to AB_Sms object to destroy */
    );
    
/*! Decodes AtlasBook commands from an SMS message.

    This function decodes an AtlasBook inbound SMS message and returns the number of
    individual commands within the message.  The individual command types and data can
    then be retrieved via AB_SmsGetCommandType and the appropriatate get command data for
    that command type.

    @return NB_Error code, indicating success or failure

    @see AB_SmsGetCommandType
    @see AB_SmsGetSynchronizeDatabaseCommandData
    @see AB_SmsGetInboundMessageCommandData
    @see AB_SmsGetMessageReceiptCommandData
    @see AB_SmsGetTrafficSessionCommandData
*/
AB_DEC NB_Error
AB_SmsDecodeMessage(
    AB_Sms* sms,                        /*!< Pointer to AB_Sms object */
    const char* smsMessage,             /*!< Pointer to SMS message to parse */
    uint16* commandCount                /*!< Pointer for parsed command count */
    );

/*! Get the type AtlasBook SMS command within the currently decoded message.

    This function returns the AtlasBook command type within the currently decoded SMS command
    by index.  The data for this command can be retrieved via the appropriate get command data
    function for that command type.

    @return AtlasBook command type of command requested via index

    @see AB_SmsDecodeSmsMessage
    @see AB_SmsGetSynchronizeDatabaseCommandData
    @see AB_SmsGetInboundMessageCommandData
    @see AB_SmsGetMessageReceiptCommandData
    @see AB_SmsGetTrafficSessionCommandData
*/
AB_DEC AB_SmsCommandType
AB_SmsGetCommandType(
    AB_Sms* sms,                        /*!< Pointer to AB_Sms object */
    uint16 commandIndex                 /*!< Index of command to return type of */
    );

/*! Get AtlasBook Synchronize Database command data from the currently decoded message.

    This function returns the AtlasBook Synchronize Database command data from the currently
    decoded SMS message by index.  The data is stored in the structure pointed to by command data.
    
    The command index must be a ASCT_SynchronizeDatabase, which can be determined with the
    AB_SmsGetSmsCommandType function.

    @return NB_Error code, indicating success or failure

    @see AB_SmsDecodeSmsMessage
    @see AB_SmsGetSmsCommandType
*/
AB_DEC NB_Error
AB_SmsGetSynchronizeDatabaseCommandData(
    AB_Sms* sms,                            /*!< Pointer to AB_Sms object */
    uint16 commandIndex,                    /*!< Index of ASCT_SynchronizeDatabase command to return data of */
    AB_SmsCommandSynchronizeDatabase* data  /*!< Pointer to structure to store requested command data */
    );

/*! Get AtlasBook Inbound Message command data from the currently decoded SMS message.

    This function returns the AtlasBook Inbound Message command data from the currently
    decoded SMS message by index.  The data is stored in the structure pointed to by command data.
    
    The command index must be a ASCT_InboundMessage, which can be determined with the
    AB_SmsGetSmsCommandType function.

    @return NB_Error code, indicating success or failure

    @see AB_SmsDecodeSmsMessage
    @see AB_SmsGetSmsCommandType
*/
AB_DEC NB_Error
AB_SmsGetInboundMessageCommandData(
    AB_Sms* sms,                            /*!< Pointer to AB_Sms object */
    uint16 commandIndex,                    /*!< Index of ASCT_InboundMessage command to return data of */
    AB_SmsCommandInboundMessage* data       /*!< Pointer to structure to store requested command data */
    );

/*! Get AtlasBook Message Receipt command data from the currently decoded SMS message.

    This function returns the AtlasBook Message Receipt command data from the currently
    decoded SMS message by index.  The data is stored in the structure pointed to by command data.
    
    The command index must be a ASCT_MessageReceipt, which can be determined with the
    AB_SmsGetSmsCommandType function.

    @return NB_Error code, indicating success or failure

    @see AB_SmsDecodeSmsMessage
    @see AB_SmsGetSmsCommandType
*/
AB_DEC NB_Error
AB_SmsGetMessageReceiptCommandData(
    AB_Sms* sms,                            /*!< Pointer to AB_Sms object */
    uint16 commandIndex,                    /*!< Index of ASCT_MessageReceipt command to return data of */
    AB_SmsCommandMessageReceipt* data       /*!< Pointer to structure to store requested command data */
    );

/*! Get AtlasBook Traffic Session command data from the currently decoded SMS message.

    This function returns the AtlasBook Traffic Session command data from the currently
    decoded SMS message by index.  The data is stored in the structure pointed to by command data.
    
    The command index must be a ASCT_TrafficSession, which can be determined with the
    AB_SmsGetSmsCommandType function.

    @return NB_Error code, indicating success or failure

    @see AB_SmsDecodeSmsMessage
    @see AB_SmsGetSmsCommandType
*/
AB_DEC NB_Error
AB_SmsGetTrafficSessionCommandData(
    AB_Sms* sms,                            /*!< Pointer to AB_Sms object */
    uint16 commandIndex,                    /*!< Index of ASCT_TrafficSession command to return data of */
    AB_SmsCommandTrafficSession* data       /*!< Pointer to structure to store requested command data */
    );

/*! Validates that a message matches the HMAC 

@param smsKey SMS key data
@param smsKeyLength The length of the SMS key data
@param message SMS message payload
@param hmac HMAC string
@return NE_OK if message matches hmac; NE_INVAL otherwise
*/
AB_DEC NB_Error AB_SmsValidateHmac(const byte* smsKey, nb_size smsKeyLength, const char* message, const char* hmac);

#endif  // ABSMS_H

/*! @} */
