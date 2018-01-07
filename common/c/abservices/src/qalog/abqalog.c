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

    @file     abqalog.c
    @defgroup abqalog QA Logging
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

#include "abqalog.h"
#include "palclock.h"
#include "cslqalog.h"
#include "cslqarecorddefinitions.h"
#include "nbcontextprotected.h"

static const uint16 INITIAL_QARECORD_BUFFER_SIZE = 250;

AB_DEF NB_Error
AB_QaLogCreate(NB_Context* context, AB_QaLogHeader* header, const char* filename, nb_boolean verbose)
{
    NB_Error err = NE_OK;
    CSL_QaLog* log = 0;

    if (!context || !header || !filename || (nsl_strlen(filename) == 0))
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaLogCreate(NB_ContextGetPal(context), (CSL_QaLogHeaderData*)header, filename, verbose, &log);
    if (!err)
    {
        err = NB_ContextSetQaLog(context, log);
    }

    return err;
}


AB_DEF void
AB_QaLogSpeechRecognitionRecordStart(NB_Context* context, const char* fieldName)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_ASR_START_RECORDING, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteText(record, fieldName, ASR_FIELD_LENGTH);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }    
}

AB_DEF void
AB_QaLogSpeechRecognitionRecordStop(NB_Context* context, const char* fieldName, uint32 dataSize)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_ASR_STOP_RECORDING, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteText(record, fieldName, ASR_FIELD_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint32(record, dataSize);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }    
}

AB_DEF void
AB_QaLogSpeechRecognitionRequest(NB_Context* context, const char* fieldName, const char* fieldText, const char* encoding, const char* dataId, uint32 dataSize, const char* session)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_ASR_REQUEST, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        if (!dataId || !(*dataId))
        {
            char dataSizeString[40] = { 0 };
            nsl_sprintf(dataSizeString, "%u", dataSize);
            err = err ? err : CSL_QaRecordWriteUint8(record, 1);
            err = err ? err : CSL_QaRecordWriteText(record, dataSizeString, ASR_FIELD_LENGTH);
        }
        else
        {
            err = err ? err : CSL_QaRecordWriteUint8(record, 0);
            err = err ? err : CSL_QaRecordWriteText(record, dataId, ASR_FIELD_LENGTH);
        }
        err = err ? err : CSL_QaRecordWriteText(record, fieldName, ASR_FIELD_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, encoding, ASR_ENCODING_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, fieldText, ASR_FIELD_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, session, ASR_SESSION_LENGTH);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

AB_DEF void
AB_QaLogSpeechRecognitionRecognizedPlaceReply(NB_Context* context, NB_Place* place, const char* session)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_ASR_RECOGNIZED_REPLY, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        char recognizedPlace[ASR_RESULT_LENGTH] = { 0 };
        nsl_strlcpy(recognizedPlace, place->name, ASR_RESULT_LENGTH);

        err = err ? err : CSL_QaRecordWriteText(record, recognizedPlace, ASR_RESULT_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, session, ASR_SESSION_LENGTH);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

AB_DEF void
AB_QaLogSpeechRecognitionRecognizedLocationReply(NB_Context* context, NB_Location* location, const char* session)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_ASR_RECOGNIZED_REPLY, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        char recognizedLocation[ASR_RESULT_LENGTH] = { 0 };
        nsl_strlcat(recognizedLocation, location->streetnum, ASR_RESULT_LENGTH);
        nsl_strlcat(recognizedLocation, "|", ASR_RESULT_LENGTH);
        nsl_strlcat(recognizedLocation, location->street1, ASR_RESULT_LENGTH);
        nsl_strlcat(recognizedLocation, "|", ASR_RESULT_LENGTH);
        nsl_strlcat(recognizedLocation, location->city, ASR_RESULT_LENGTH);
        nsl_strlcat(recognizedLocation, "|", ASR_RESULT_LENGTH);
        nsl_strlcat(recognizedLocation, location->state, ASR_RESULT_LENGTH);
        nsl_strlcat(recognizedLocation, "|", ASR_RESULT_LENGTH);
        nsl_strlcat(recognizedLocation, location->postal, ASR_RESULT_LENGTH);

        err = err ? err : CSL_QaRecordWriteText(record, recognizedLocation, ASR_RESULT_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, session, ASR_SESSION_LENGTH);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

AB_DEF void
AB_QaLogSpeechRecognitionAmbiguousReply(NB_Context* context, const char* fieldName, uint32 status, uint32 optionsCount, const char* session)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_ASR_AMBIGUOUS_REPLY, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteText(record, fieldName, ASR_FIELD_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint32(record, status);
        err = err ? err : CSL_QaRecordWriteUint32(record, optionsCount);
        err = err ? err : CSL_QaRecordWriteText(record, session, ASR_SESSION_LENGTH);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

AB_DEF void
AB_QaLogAuthenticationRequest(NB_Context* context, uint32 request, const char* requestParameter, uint32 timeStamp, const char* language)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_AUTHENTICATION_REQUEST, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteUint32(record, request);
        err = err ? err : CSL_QaRecordWriteText(record, requestParameter, AUTHENTICATION_PARAMETER_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint32(record, timeStamp);
        err = err ? err : CSL_QaRecordWriteText(record, language, AUTHENTICATION_LOCALE_LENGTH);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

AB_DEF void
AB_QaLogAuthenticationReply(NB_Context* context, AB_QaLogBundleState bundleState, uint32 timeStamp, uint32 tokenId, const char* bundleName, const char* bundleRegion, const char* featureCodes, const char* priceOptions, uint32 status)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_AUTHENTICATION_REPLY, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteUint32(record, bundleState);
        err = err ? err : CSL_QaRecordWriteUint32(record, timeStamp);
        err = err ? err : CSL_QaRecordWriteUint32(record, tokenId);
        err = err ? err : CSL_QaRecordWriteText(record, bundleName, AUTHENTICATION_BUNDLE_NAME_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, bundleRegion, AUTHENTICATION_BUNDLE_REGIONS_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, featureCodes, AUTHENTICATION_FEATURE_CODES_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, priceOptions, AUTHENTICATION_PRICE_OPTIONS_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint32(record, status);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

AB_DEF void
AB_QaLogLicenseRequest(NB_Context* context, const char* requestId, const char* action, const char* subscriberKey, const char* vendorName,
                       const char* country, const char* productId, const char* bundleType, const char* bundleName, const char* language)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_LICENSE_REQUEST, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteText(record, requestId, LICENCE_REQUEST_ID_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, action, LICENSE_ACTION_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, subscriberKey, LICENSE_KEY_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, vendorName, LICENSE_VENDOR_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, country, LICENCE_LOCALE_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, productId, LICENSE_DATA_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, bundleName, LICENSE_DATA_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, bundleType, LICENSE_DATA_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, language, LICENCE_LOCALE_LENGTH);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

AB_DEF void
AB_QaLogLicenseReply(NB_Context* context, const char* key, const char* productId, const char* requestId, uint32 status)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_LICENSE_REPLY, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteText(record, requestId, LICENCE_REQUEST_ID_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, key, LICENSE_KEY_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, productId, LICENSE_DATA_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint32(record, status);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

AB_DEF void
AB_QaLogProfileSetting(NB_Context* context, const char* getKey, const char* setKey, const char* setValue, const char* returnedValue, uint32 errorCode, const char* errorMessage)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_PROFILE_SETTING, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteText(record, getKey, PROFILE_KEY_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, setKey, PROFILE_KEY_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, setValue, PROFILE_VALUE_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, returnedValue, PROFILE_VALUE_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint32(record, errorCode);
        err = err ? err : CSL_QaRecordWriteText(record, errorMessage, ERROR_MESSAGE_LENGTH);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

AB_DEF void
AB_QaLogOneShotAsrEvent(NB_Context* context, const char* sessionID, uint32 event)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_ONE_SHOT_ASR_EVENT, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteText(record, sessionID, ONE_SHOT_ASR_SESSION_ID_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint32(record, event);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

AB_DEF void
AB_QaLogSpeechStreamRequest(NB_Context* context, const char* sessionID, byte language, byte asrScreen, uint32 sequenceID,
                            byte endSpeech, byte audioFormat, uint32 audioSize)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_SPEECH_STREAM_REQUEST, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteText(record, sessionID, ONE_SHOT_ASR_SESSION_ID_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint8(record, language);
        err = err ? err : CSL_QaRecordWriteUint8(record, asrScreen);
        err = err ? err : CSL_QaRecordWriteUint32(record, sequenceID);
        err = err ? err : CSL_QaRecordWriteUint8(record, endSpeech);
        err = err ? err : CSL_QaRecordWriteUint8(record, audioFormat);
        err = err ? err : CSL_QaRecordWriteUint32(record, audioSize);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

AB_DEF void
AB_QaLogSpeechStreamReply(NB_Context* context, const char* sessionID, uint32 completionCode, const char* providerSessionID, uint32 resultsCount)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_SPEECH_STREAM_REPLY, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteText(record, sessionID, ONE_SHOT_ASR_SESSION_ID_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint32(record, completionCode);
        err = err ? err : CSL_QaRecordWriteText(record, providerSessionID, ONE_SHOT_ASR_SESSION_ID_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint32(record, resultsCount);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

AB_DEF void
AB_QaLogOneShotAsrResult(NB_Context* context, const char* sessionID, uint32 number, const char* text)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_ONE_SHOT_ASR_RESULT, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteText(record, sessionID, ONE_SHOT_ASR_SESSION_ID_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint32(record, number);
        err = err ? err : CSL_QaRecordWriteText(record, text, ONE_SHOT_ASR_RESULT_TEXT_LENGTH);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

AB_DEF void
AB_QaLogClear(NB_Context* context, uint32 sessionCount)
{
    if(context)
    {
        CSL_QaLogClear(NB_ContextGetQaLog(context), sessionCount);
    }
}
