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

    @file     abqalog.h
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

#ifndef ABQALOG_H
#define ABQALOG_H


#include "abexp.h"
#include "paltypes.h"
#include "nbcontext.h"
#include "nbplace.h"

/*!
    @addtogroup abqaloginstance
    @{
*/

typedef struct
{
    const char* productName;
	nb_version  productVersion;
	uint32      platformId;
	uint64      mobileDirectoryNumber;
	uint64      mobileInformationNumber;
} AB_QaLogHeader;


typedef enum
{
    AB_QLBS_Subscribed    = 0,
    AB_QLBS_Available     = 1

} AB_QaLogBundleState;


/*! Create a instance of a QA Logging object

Creates a new instance of a QA Logging object and associates it with the NB_Context.  The created
QA Logging object is owned by the NB_Context and will be destroyed when the context is destroyer

@param context A pointer to the NB Context
@param header The QA Logging application information to be written to the file header
@param filename The name of the file
@param verbose Specify non-zero for verbose QA Logging; zero otherwise
@return NB_Error
*/
AB_DEC NB_Error AB_QaLogCreate(NB_Context* context, AB_QaLogHeader* header, const char* filename, nb_boolean verbose);


/*! Log the ASR recording start

Logs the start of an ASR recording.  Called by client app.

@param context A pointer to the shared NB_Context instance
@param fieldName Name of field starting recording for
*/
AB_DEC void AB_QaLogSpeechRecognitionRecordStart(NB_Context* context, const char* fieldName);


/*! Log the ASR recording stop

Logs the completion of an ASR recording.  Called by client app.

@param context A pointer to the shared NB_Context instance
@param fieldName Name of field recording for
@param dataSize Size of data recorded
*/
AB_DEC void AB_QaLogSpeechRecognitionRecordStop(NB_Context* context, const char* fieldName, uint32 dataSize);


/*! Log the ASR request

Logs the ASR data item supplied for recognition request.  Called by AB services for each data item supplied.

@param context A pointer to the shared NB_Context instance
@param fieldName Name of field supplied for recognition
@param fieldText Text for field value, if supplied
@param encoding Encoding of field voice data
@param dataId Id of previously recorded voice, if used
@param dataSize Size of recorded data, if raw data supplied
@param session Session id
*/
AB_DEC void AB_QaLogSpeechRecognitionRequest(NB_Context* context, const char* fieldName, const char* fieldText, const char* encoding, const char* dataId, uint32 dataSize, const char* session);


/*! Log the ASR recognized place reply

Logs a recognized place reply from an ASR request.  Called by AB services.

@param context A pointer to the shared NB_Context instance
@param place Recognized place
@param session Session id
*/
AB_DEC void AB_QaLogSpeechRecognitionRecognizedPlaceReply(NB_Context* context, NB_Place* place, const char* session);


/*! Log the ASR recognized location reply

Logs a recognized location reply from an ASR request.  Called by AB services.

@param context A pointer to the shared NB_Context instance
@param location Recognized location
@param session Session id
*/
AB_DEC void AB_QaLogSpeechRecognitionRecognizedLocationReply(NB_Context* context, NB_Location* location, const char* session);


/*! Log the ASR ambiguous reply

Logs an ambiguous reply from an ASR request.  Called by AB services.

@param context A pointer to the shared NB_Context instance
@param status Status code returned
@param optionsCount Count of options presented for ambiguity
@param session Session id
*/
AB_DEC void AB_QaLogSpeechRecognitionAmbiguousReply(NB_Context* context, const char* fieldName, uint32 status, uint32 optionsCount, const char* session);


/*! Log the Authentication request

Logs an Authentication request.  Called by AB services when the request is sent to the server.

@param context A pointer to the shared NB_Context instance
@param request Request enum value from from data_auth_request element
@param request Request-specific additional parameters
@param timeStamp Client timestamp
@param language Language
*/
AB_DEC void AB_QaLogAuthenticationRequest(
    NB_Context* context,
    uint32 request,
    const char* requestParameter,
    uint32 timeStamp,
    const char* language);


/*! Log the Authentication reply

Logs an Authentication reply.  Called by AB services when the reply is received from the server.

@param context A pointer to the shared NB_Context instance
@param bundleState Enum of bundle state (i.e. subscribed, available)
@param timeStamp Timestamp from server
@param tokenId Token from server
@param bundleName Bundle name
@param bundleRegions Bundle regions delimited
@param featureCodes Feature codes delimited
@param priceOptions Price options delimited
@param status Enum of status returned
*/
AB_DEC void AB_QaLogAuthenticationReply(
    NB_Context* context,
    AB_QaLogBundleState bundleState,
    uint32 timeStamp,
    uint32 tokenId,
    const char* bundleName,
    const char* bundleRegion,
    const char* featureCodes,
    const char* priceOptions,
    uint32 status);


/*! Log the License request

Logs an License request.  Called by AB services when the request is sent to the server.

@param context A pointer to the shared NB_Context instance
@param requestId Request Id
@param action Action
@param subscriberKey Subscriber key
@param vendorName Vendor name
@param country Country
@param bundleType Bundle type
@param bundleName Bundle name
@param language Language
*/
AB_DEC void AB_QaLogLicenseRequest(
    NB_Context* context,
    const char* requestId,
    const char* action,
    const char* subscriberKey,
    const char* vendorName,
    const char* country,
    const char* productId,
    const char* bundleType,
    const char* bundleName,
    const char* language);


/*! Log the License reply

Logs an License reply.  Called by AB services when the request is sent to the server.

@param context A pointer to the shared NB_Context instance
@param key Key
@param productId Product Id
@param requestId Request Id
@param status Status returned
*/
AB_DEC void AB_QaLogLicenseReply(
    NB_Context* context,
    const char* key,
    const char* productId,
    const char* requestId,
    uint32 status);

/*! Log profile setting

@param context A pointer to the shared NB_Context instance
@param getKey Profile key to get
@param setKey Profile key to set
@param setValue Profile value to set
@param returnedValue Profile value returned (if any)
@param errorCode Error code returned (if any)
@param errorMessage Error message returned (if any)
*/
AB_DEC void AB_QaLogProfileSetting(
    NB_Context* context,
    const char* getKey,
    const char* setKey,
    const char* setValue,
    const char* returnedValue,
    uint32 errorCode,
    const char* errorMessage);


/*! Log One Shot ASR Event

@param context A pointer to the shared NB_Context instance
@param sessionID Interaction session id
@param event Event ID

*/
AB_DEC void AB_QaLogOneShotAsrEvent(
    NB_Context* context,
    const char* sessionID,
    uint32 event);


/*! Log Speech stream request

@param context A pointer to the shared NB_Context instance
@param sessionID Interaction session id
@param language Client language
@param asrScreen Screen from which asr was initiated
@param sequenceID Audio chuck sequence
@param endSpeech Last chunk is sequence
@param audioFormat Audio format of the data to be sent on server
@param audioSize The size of the sent audio data

*/
AB_DEC void AB_QaLogSpeechStreamRequest(
    NB_Context* context,
    const char* sessionID,
    byte language,
    byte asrScreen,
    uint32 sequenceID,
    byte endSpeech,
    byte audioFormat,
    uint32 audioSize);


/*! Log Speech stream reply

@param context A pointer to the shared NB_Context instance
@param sessionID Interaction session id
@param completionCode Completion code
@param providerSessionID Provider session id
@param resultsCount Results count

*/
AB_DEC void AB_QaLogSpeechStreamReply(
    NB_Context* context,
    const char* sessionID,
    uint32 completionCode,
    const char* providerSessionID,
    uint32 resultsCount);


/*! Log One Shot ASR result

@param context A pointer to the shared NB_Context instance
@param sessionID Interaction session id
@param number The position number of recognized text
@param text Recognized text

*/
AB_DEC void AB_QaLogOneShotAsrResult(
    NB_Context* context,
    const char* sessionID,
    uint32 number,
    const char* text);

/*! Clear qalog file, keep the latest 'sessionCount' sessions, clear other's */
AB_DEC void AB_QaLogClear(NB_Context* context, uint32 sessionCount);

#endif

/*! @} */
