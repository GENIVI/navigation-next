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

    @file     absms.c
    @date     04/01/2009
    @defgroup SMS_API_GROUP AtlasBook SMS API
    
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

#include "absms.h"
#include "abutil.h"

#include "nbroutesettings.h"
#include "nberror.h"
#include "gpsutil.h"
#include "sha1.h"
#include "palmath.h"
#include "pal.h"

// Disable "warning C4127: conditional expression is constant" due to the "while(1)" statement below
#pragma warning(disable:4127)


// Local constants ...............................................................................

#define MIN_VALID_MESSAGE_LEN	    23      /* 20 (hmac) + 3 (single command, no from) */
#define MAX_VALID_MESSAGE_LEN       160     /* SMS message max */

// field indexes per supported SMS command
#define SMS_INDEX_FROM                  0
#define SMS_INDEX_COMMAND               1

#define SMS_INDEX_SYNC_DBLIST           2

#define SMS_INDEX_MESSAGE_DBLIST        2
#define SMS_INDEX_MESSAGE_TIME          3
#define SMS_INDEX_MESSAGE_NUMBER        4
#define SMS_INDEX_MESSAGE_GENERATION    5
#define SMS_INDEX_MESSAGE_FROMNAME      6

#define SMS_INDEX_RECEIPT_RESULT        2
#define SMS_INDEX_RECEIPT_MESSAGEID     3

#define SMS_INDEX_TRAFFIC_SESSIONID     2

static const char VALID_SMS_COMMANDS[] = "SMRN";  /* SMS commands currently supported */


// Local types ...................................................................................

struct _AB_Sms
{
    byte*               smsKey;         /*!< SMS key */
    nb_size             smsKeyLength;   /*!< SMS key length */
    char                smsMessage[MAX_VALID_MESSAGE_LEN + 1];  /*!< SMS message decoded */
};


// Local functions ...............................................................................

static boolean GetMessageField(char* message, int fieldNumber, char* fieldData, nb_size fieldDataSize);

// Public functions ..............................................................................

AB_DEF NB_Error
AB_SmsCreate(AB_Sms** sms, const byte* smsKey, nb_size smsKeyLength)
{
    AB_Sms* pThis = NULL;

    if (!smsKey || !smsKeyLength)
    {
        return NE_BADDATA;
    }
    
    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NE_NOMEM;
    }
    
    nsl_memset(pThis, 0, sizeof(*pThis));
    
    pThis->smsKeyLength = smsKeyLength;
    if ((pThis->smsKey = nsl_malloc(smsKeyLength)) == NULL)
    {
        nsl_free(pThis);
        return NE_NOMEM;
    }
    nsl_memcpy(pThis->smsKey, smsKey, smsKeyLength);
    nsl_memset(pThis->smsMessage, 0, sizeof pThis->smsMessage);
    
    *sms = pThis;
    return NE_OK;
}

AB_DEF NB_Error
AB_SmsDestroy(AB_Sms* sms)
{
    NB_Error err = NE_OK;
    AB_Sms* pThis = sms;
    
    nsl_free(pThis->smsKey);
    nsl_free(pThis);

    return err;
}

AB_DEF NB_Error
AB_SmsDecodeMessage(AB_Sms* sms, const char* smsMessage, uint16* commandCount)
{
    boolean valid = TRUE;
    
    byte xmit_mac[10];
    byte calc_mac[SHS_DIGESTSIZE];

    char msg[MAX_VALID_MESSAGE_LEN + 1];
    int32 msglen = 0;
    
    char fld[MAX_VALID_MESSAGE_LEN + 1];
    
    nsl_memset(sms->smsMessage, 0, sizeof sms->smsMessage);
    *commandCount = 0;
    
    nsl_memset(xmit_mac, 0, sizeof(xmit_mac));
    nsl_memset(calc_mac, 0, sizeof(calc_mac));
    nsl_memset(msg, 0, sizeof(msg));

    if (sms == NULL || smsMessage == NULL || nsl_strlen(smsMessage) < MIN_VALID_MESSAGE_LEN)
    {
        return NE_BADDATA;
    }
  
    if (!unhexlify(sizeof(xmit_mac), smsMessage, (char*)xmit_mac))
    {
        return NE_BADDATA;
    }
    
    nsl_strlcpy(msg, smsMessage + 20, sizeof(msg));
    msglen = nsl_strlen(msg);

    hmac((const byte*)msg, msglen, sms->smsKey, sms->smsKeyLength, calc_mac);

    if (nsl_memcmp(xmit_mac, calc_mac, sizeof(xmit_mac)) != 0)
    {
        return NE_INVAL;
    }
    
    msglen = decode_base64(msg, msg);
    if (msglen <= 0)
    {
        return NE_INVAL;
    }
    
    obfuscate_buffer((byte*)msg, (uint32)msglen, sms->smsKey, sms->smsKeyLength);

    if (msglen >= sizeof(msg))
    {
        msglen = sizeof(msg) - 1;
    }
    msg[msglen] = 0;
    nsl_strcpy(sms->smsMessage, msg);
    
    if (GetMessageField(sms->smsMessage, 1, fld, sizeof(fld)))
    {
        char* c = 0;
        valid = TRUE;
        
        for (c = fld; *c != 0; c++)
        {
            if (nsl_strchr(VALID_SMS_COMMANDS, *c) == 0)
            {
                valid = FALSE;
                break;
            }
        }
    }

    if (valid == FALSE)
    {
        return NE_INVAL;
    }
    
    *commandCount = (uint16)nsl_strlen(fld);
    return NE_OK;
}

AB_DEF AB_SmsCommandType AB_SmsGetCommandType(AB_Sms* sms, uint16 commandIndex)
{
    AB_Sms* pThis = sms;
    AB_SmsCommandType commandType = ASCT_Unknown;
        
    char fld[MAX_VALID_MESSAGE_LEN + 1];
    
    if (pThis == NULL || pThis->smsMessage == 0)
    {
        return commandType;
    }

    if (GetMessageField(pThis->smsMessage, 1, fld, sizeof fld))
    {
        if (commandIndex < nsl_strlen(fld))
        {
            switch (fld[commandIndex])
            {
                case 'S':
                    commandType = ASCT_SynchronizeDatabase;
                    break;
                case 'M':
                    commandType = ASCT_InboundMessage;
                    break;
                case 'R':
                    commandType = ASCT_MessageReceipt;
                    break;
                case 'N':
                    commandType = ASCT_TrafficSession;
                    break;
            }
        }
    }
    
    return commandType;
}

AB_DEF NB_Error AB_SmsGetSynchronizeDatabaseCommandData(AB_Sms* sms, uint16 commandIndex, AB_SmsCommandSynchronizeDatabase* data)
{
    if (sms == NULL || data == NULL || AB_SmsGetCommandType(sms, commandIndex) != ASCT_SynchronizeDatabase)
    {
        return NE_INVAL;
    }

    nsl_memset(data, 0, sizeof(AB_SmsCommandSynchronizeDatabase));

    GetMessageField(sms->smsMessage, SMS_INDEX_FROM, data->msgFrom, sizeof data->msgFrom);
    GetMessageField(sms->smsMessage, SMS_INDEX_SYNC_DBLIST, data->syncDatabases, sizeof data->syncDatabases);
    
    return NE_OK;
}

AB_DEF NB_Error AB_SmsGetInboundMessageCommandData(AB_Sms* sms, uint16 commandIndex, AB_SmsCommandInboundMessage* data)
{
    char field[20];
    
    if (sms == NULL || data == NULL || AB_SmsGetCommandType(sms, commandIndex) != ASCT_InboundMessage)
    {
        return NE_INVAL;
    }
    
    nsl_memset(data, 0, sizeof(AB_SmsCommandInboundMessage));

    GetMessageField(sms->smsMessage, SMS_INDEX_FROM, data->msgFrom, sizeof data->msgFrom);

    GetMessageField(sms->smsMessage, SMS_INDEX_MESSAGE_TIME, field, sizeof field);
    data->msgTime = nsl_strtoul(field, NULL, 10);

    GetMessageField(sms->smsMessage, SMS_INDEX_MESSAGE_NUMBER, field, sizeof field);
    data->msgNum = nsl_strtoul(field, NULL, 10);

    GetMessageField(sms->smsMessage, SMS_INDEX_MESSAGE_GENERATION, field, sizeof field);
    data->msgGeneration = nsl_strtoul(field, NULL, 10);

    GetMessageField(sms->smsMessage, SMS_INDEX_MESSAGE_FROMNAME, data->msgFromName, sizeof data->msgFromName);
    
    return NE_OK;
}

AB_DEF NB_Error AB_SmsGetMessageReceiptCommandData(AB_Sms* sms, uint16 commandIndex, AB_SmsCommandMessageReceipt* data)
{
    char field[20];

    if (sms == NULL || data == NULL || AB_SmsGetCommandType(sms, commandIndex) != ASCT_MessageReceipt)
    {
        return NE_INVAL;
    }

    nsl_memset(data, 0, sizeof(AB_SmsCommandMessageReceipt));

    GetMessageField(sms->smsMessage, SMS_INDEX_FROM, data->msgFrom, sizeof data->msgFrom);
            
    GetMessageField(sms->smsMessage, SMS_INDEX_RECEIPT_RESULT, field, sizeof field);
    data->msgResult = field[0];
            
    GetMessageField(sms->smsMessage, SMS_INDEX_RECEIPT_MESSAGEID, data->msgId, sizeof data->msgId);
    
    return NE_OK;
}

AB_DEF NB_Error AB_SmsGetTrafficSessionCommandData(AB_Sms* sms, uint16 commandIndex, AB_SmsCommandTrafficSession* data)
{
    if (sms == NULL || data == NULL || AB_SmsGetCommandType(sms, commandIndex) != ASCT_TrafficSession)
    {
        return NE_INVAL;
    }
    
    nsl_memset(data, 0, sizeof(AB_SmsCommandTrafficSession));
    
    GetMessageField(sms->smsMessage, SMS_INDEX_FROM, data->msgFrom, sizeof data->msgFrom);
    GetMessageField(sms->smsMessage, SMS_INDEX_TRAFFIC_SESSIONID, data->trafficSession, sizeof data->trafficSession);
    
    return NE_OK;
}

// Local functions ...............................................................................

boolean
GetMessageField(char* message, int fieldNumber, char* fieldData, nb_size fieldDataSize)
{
    boolean result = FALSE;

    const char* start = message;
    const char* p = message;
    const char sep = '|';
    
    int currentFieldNum = 0;
    nsl_memset(fieldData, 0, fieldDataSize);

    while (1)
    {
        if (*p == 0 || *p == sep) {
            if (currentFieldNum == fieldNumber)
            {
                nb_size fieldSize = MIN(p - start, fieldDataSize - 1);
                nsl_strncpy(fieldData, start, fieldSize);
                fieldData[fieldSize + 1] = 0;
                result = TRUE;
                break;
            }
            
            if (p == 0)
            {
                break;
            }

            start = p + 1;
            currentFieldNum++;
        }
        p++;
    }
    
    return result;
}

/*
    HMAC validation process:
        1) Get the hex bytes corresponding to the HMAC
        2) Obfuscate the message
        3) Base64 encode the obfuscated message
        4) Server dev says to remove all \n characters, but those can't exist in base64 output
        5) Calculate HMAC on base64 encoded, obfuscated buffer
        6) Compare given HMAC bytes with first 10 bytes of calculated HMAC
*/
AB_DEF NB_Error
AB_SmsValidateHmac(const byte* smsKey, nb_size smsKeyLength, const char* message, const char* hmacString)
{
    NB_Error err = NE_OK;
    byte hmacBytes[10] = { 0 };
    byte calculatedMac[SHS_DIGESTSIZE] = { 0 };
    byte* obfuscatedBuffer = 0;
    nb_size obfuscatedBufferSize = 0;
    char* base64Buffer = 0;

    if (!smsKey || !message || !hmacString || smsKeyLength == 0)
    {
        return NE_INVAL;
    }

    if (!unhexlify(sizeof(hmacBytes), hmacString, (char*)hmacBytes))
    {
        return NE_BADDATA;
    }

    obfuscatedBufferSize = nsl_strlen(message);
    obfuscatedBuffer = nsl_malloc(obfuscatedBufferSize);
    if (!obfuscatedBuffer)
    {
        return NE_NOMEM;
    }
    nsl_memcpy(obfuscatedBuffer, message, obfuscatedBufferSize);
    obfuscate_buffer(obfuscatedBuffer, obfuscatedBufferSize, smsKey, smsKeyLength);

    err = encode_base64((char*)obfuscatedBuffer, obfuscatedBufferSize, &base64Buffer);
    if (!err)
    {
        hmac((const byte*)base64Buffer, nsl_strlen(base64Buffer), smsKey, smsKeyLength, calculatedMac);
        if (nsl_memcmp(hmacBytes, calculatedMac, sizeof(hmacBytes)) != 0)
        {
            err = NE_INVAL;
        }

        nsl_free(base64Buffer);
    }
    nsl_free(obfuscatedBuffer);

    return err;
}


/*! @} */
