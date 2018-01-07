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

    @file abspeechrecognitioninformation.h
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

#ifndef ABSPEECHRECOGNITIONINFORMATION_H
#define ABSPEECHRECOGNITIONINFORMATION_H

#include "abexp.h"
#include "nblocation.h"
#include "nbplace.h"
#include "nberror.h"


/*! @addtogroup abspeechrecognitioninformation
    @{
*/


/*! ASR recognition request completion codes.

    Returned in AB_RecognizeResults struct after a AB_SpeechRecognitionRecognize request by calling
    ::AB_SpeechRecognitionGetRecognizedResults.
*/
#define ARCC_SUCCESS                        0       /*!< Success */ 
#define ARCC_NO_RESULTS_FOUND               10      /*!< No results found */ 

#define ARCC_ERROR_ASR_ENGINE_CONNECTION    101     /*!< Recognition engine connection error */ 
#define ARCC_ERROR_DATASTORE_RETRIEVING     201     /*!< Datastore retrieval error */ 
#define ARCC_ERROR_RESEND_REQUEST           202     /*!< Resend data request; actual data required (not datastore IDs) */ 
#define ARCC_ERROR_IN_QUERY_DATA            251     /*!< Data error in query (DEPRECATED: will raise TPS error instead) */ 
#define ARCC_ERROR_MISSING_CONTEXT          301     /*!< Missing context in query (ex., trying to recognize city without valid state) */ 


/*! ASR recognition request extended completion codes.

    Extended completion codes provided if AB_ASR_WantExtendedCodes is specified when creating the
    AB_SpeechRecognitionParameters object with AB_SpeechRecognitionParametersCreateWithOptions().
    
    Returned in AB_RecognizeResults struct after a AB_SpeechRecognitionRecognize request by calling
    ::AB_SpeechRecognitionGetRecognizedResults.
*/
#define ARCC_SUCCESS_SPEECH_TOO_SOFT        1       /*!< Results returned -- speech too soft */
#define ARCC_SUCCESS_SPEECH_TOO_LOUD        2       /*!< Results returned -- speech too loud */
#define ARCC_SUCCESS_SPEECH_TOO_FAST        3       /*!< Results returned -- speech too fast */
#define ARCC_SUCCESS_SPEECH_TOO_SLOW        4       /*!< Results returned -- speech too slow */
#define ARCC_SUCCESS_SPEECH_TOO_SOON        5       /*!< Results returned -- speech too soon */
#define ARCC_SUCCESS_TOO_MUCH_SPEECH        6       /*!< Results returned -- too much speech */

#define ARCC_NO_RESULTS_SPEECH_TOO_SOFT     11      /*!< No results found -- speech too soft */
#define ARCC_NO_RESULTS_SPEECH_TOO_LOUD     12      /*!< No results found -- speech too loud */
#define ARCC_NO_RESULTS_SPEECH_TOO_FAST     13      /*!< No results found -- speech too fast */
#define ARCC_NO_RESULTS_SPEECH_TOO_SLOW     14      /*!< No results found -- speech too slow */
#define ARCC_NO_RESULTS_SPEECH_TOO_SOON     15      /*!< No results found -- speech too soon */
#define ARCC_NO_RESULTS_TOO_MUCH_SPEECH     16      /*!< No results found -- too much speech */

#define ARCC_ERROR_ASR_ENGINE_TIMEOUT       102     /*!< Recognition engine timeout */
#define ARCC_ERROR_INVALID_CONTEXT          302     /*!< Context in query is invalid (ex., invalid zip code) */


/*! Result information from completed ASR query.

    This structure contains result information of a completed recognition query.
*/
typedef struct
{
    uint32 completionCode;             /*!< Recognition completion code */
    uint32 placeMatchCount;            /*!< Count of POI matches in recognition */
    uint32 locationMatchCount;         /*!< Count of Location matches in recognition */   
    uint32 dataIdCount;                /*!< Count of recorded audio data store IDs submitted for recognition */
    const char* interactionID;         /*!< Pointer to ASR ID for this query to be used for future recognition requests */
    const char* lastFieldName;         /*!< Pointer to last field name that was ambiguous in last query; empty string if none were ambiguous */
} AB_SpeechRecognitionResults;


/*! @struct AB_SpeechRecognitionInformation
Information about the results of a data store request
*/
typedef struct AB_SpeechRecognitionInformation AB_SpeechRecognitionInformation;


/*! Get results from completed recognition query.

This function may be called after a recognition query completes to get the query results.
The results are stored in the ::AB_RecognizeResults structure pointed to by the resultInfo parameter.

@param information Pointer to AB_SpeechRecognitionInformation object
@param resultInfo Pointer to AB_RecognizeResults structure to store the results of the completed recognition

@return NB_Error
*/
AB_DEC NB_Error AB_SpeechRecognitionInformationGetRecognizeResults(AB_SpeechRecognitionInformation* information, AB_SpeechRecognitionResults *resultInfo);


/*! Get recognized place from completed recognition query by index.

This function may be called after a recognition query to get a NBI_Place recognized by index.
Searches for POI or airports will be returned here as a NBI_Place, but not addresses.

@param information Pointer to AB_SpeechRecognitionInformation object
@param poiIndex Index of recognized place to get; must be < placeMatchCount returned by AB_SpeechRecognitionGetRecognizeResults
@param place Pointer to NBI_Place struct for requested place
@param distance Pointer to distance from GPS fix, if supplied

@return NB_Error

@see AB_SpeechRecognitionGetRecognizeResults
*/
AB_DEC NB_Error AB_SpeechRecognitionInformationGetRecognizedPlace(AB_SpeechRecognitionInformation* information, uint32 poiIndex, NB_Place* place, double* distance);


/*! Get recognized location from completed recognition query by index.

This function may be called after a recognition query to get a NBI_Place recognized by index.
Searches for addresses will be returned here as a NBI_Location, but not POI or airports.

@param information Pointer to AB_SpeechRecognitionInformation object
@param locationIndex Index of recognized location to get; must be < locationMatchCount returned by AB_SpeechRecognitionGetRecognizeResults
@param location Pointer to NBI_Location object for requested location

@return NB_Error

@see AB_SpeechRecognitionGetRecognizeResults
*/
AB_DEC NB_Error AB_SpeechRecognitionInformationGetRecognizedLocation(AB_SpeechRecognitionInformation* information, uint32 locationIndex, NB_Location* location);


/*! Get ID of saved data used in completed recognition query by index.

    This function may be called after a recognition query to get a the data store ID used in the query by index.
    The data store IDs returned are for any binary audio data submitted via the ::AB_SpeechRecognitionAddUtterance function so
    they may be referenced later.  The data store IDs are returned in the order they were submitted.

    @return NB_Error

    @see AB_SpeechRecognitionGetRecognizeResults
*/
AB_DEC NB_Error AB_SpeechRecognitionInformationGetSavedDataId(AB_SpeechRecognitionInformation* information, uint32 savedDataIdIndex, const char** savedDataId);


/*! Destroy a previously created DataStoreInformation object

@param information An AB_SpeechRecognitionInformation object created with AB_SpeechRecognitionHandlerGetInformation()
@returns NB_Error
*/
AB_DEC NB_Error AB_SpeechRecognitionInformationDestroy(AB_SpeechRecognitionInformation* information);


#endif
