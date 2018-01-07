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

    @file abspeechrecognitionstatisticsparameters.h
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

#ifndef ABSPEECHSTATISTICSPARAMETERS_H
#define ABSPEECHSTATISTICSPARAMETERS_H

#include "abexp.h"


/*! @addtogroup abspeechstatisticsparameters
    @{
*/


/*! @struct AB_SpeechStatisticsParameters
    Opaque data structure that defines the parameters of a speech recognition operation
*/
typedef struct AB_SpeechStatisticsParameters AB_SpeechStatisticsParameters;


/*! Create an AB_SpeechStatisticsParameters instance.

This function creates and returns a pointer to an AB_SpeechStatisticsParameters instance.
Call AB_SpeechStatisticsParametersDestroy() to destroy the object.

@return NB_Error

@see AB_SpeechStatisticsParametersDestroy
*/
AB_DEC NB_Error AB_SpeechStatisticsParametersCreate(NB_Context* context, const char* previousInteractionId, AB_SpeechStatisticsParameters** parameters);


/*! Destroy a previously created SpeechRecognitionParameters object

@param parameters An AB_SpeechStatisticsParameters object created with AB_SpeechStatisticsParametersCreate()
@returns NB_Error
*/
AB_DEC NB_Error AB_SpeechStatisticsParametersDestroy(AB_SpeechStatisticsParameters* parameters);


/*! Add a text field value for the next recognition query.

This function adds a text value for a specified field for the next recognition query.

Valid field names are: <br>
\a "address-full" <br>
\a "address-street" <br>
\a "address-cross-street" <br>
\a "address-city" <br>
\a "address-state" <br>
\a "address-postalcode" <br>
\a "airport" <br>
\a "local-search-poi" <br>

@todo (BUG 56471): determine where complete list of valid field names is

@param parameters Pointer to AB_SpeechStatisticsParameters object
@param fieldName Name of field that text is for
@param fieldText Text currently in this text box for this field
@return NB_error

@see AB_SpeechStatisticsParametersCreate
@see AB_SpeechStatisticsParametersClearFields
*/
AB_DEC NB_Error AB_SpeechStatisticsParametersAddText(AB_SpeechStatisticsParameters* parameters, const char* fieldName, const char* fieldText);


/*! Add a text field value from a specified source for the next recognition query.

This function adds a text value for a specified field for the next recognition query.

Valid field names are: <br>
\a "address-full" <br>
\a "address-street" <br>
\a "address-cross-street" <br>
\a "address-city" <br>
\a "address-state" <br>
\a "address-postalcode" <br>
\a "airport" <br>
\a "local-search-poi" <br>

@todo (BUG 56471): determine where complete list of valid field names is

Valid text field sources (specified in fieldSource parameter) that indicate the source of the given fieldText are: <br>
\a "text" for text of any type (default if fieldType is NULL or an empty string)<br>
\a "usertext" for text entered/modified by the user<br>
\a "recotext" for text from speech recognition; not modified by the user <br>
\a "geotext" for text from geocode lookup; not modified by the user<br>

@param parameters Pointer to AB_SpeechStatisticsParameters object
@param fieldName Name of field that text is for
@param fieldText Text currently in this text box for this field
@param fieldSource Source of text provided
@return NB_error

@see AB_SpeechStatisticsParametersCreate
@see AB_SpeechStatisticsParametersClearFields
*/
AB_DEC NB_Error AB_SpeechStatisticsParametersAddTextFromSource(AB_SpeechStatisticsParameters* parameters, const char* fieldName, const char* fieldText, const char* fieldSource);


/*! Add an audio field value for the next recognition query.

This function adds an audio utterance for a specified field for the next recognition query.

Valid field names are: <br>
\a "address-full" <br>
\a "address-street" <br>
\a "address-cross-street" <br>
\a "address-city" <br>
\a "address-state" <br>
\a "address-postalcode" <br>
\a "airport" <br>
\a "local-search-poi" <br>

@todo (BUG 56471): determine where complete list of valid field names is

@param parameters Pointer to AB_SpeechStatisticsParameters object
@param fieldName Name of field that utterance is for
@param fieldText Text currently in text box for this field
@param utteranceEncoding Audio data encoding type ("wav", "amr", "qcp")
@param utteranceData Audio recorded data buffer
@param utteranceSize Size of utteranceData

@return NB_error

@see AB_SpeechStatisticsParametersCreate
@see AB_SpeechStatisticsParametersClearFields
*/
AB_DEC NB_Error AB_SpeechStatisticsParametersAddUtterance(AB_SpeechStatisticsParameters* parameters, const char* fieldName, const char* fieldText, const char* utteranceEncoding, byte* utteranceData, uint32 utteranceSize);


/*! Add a reference to a previously stored audio field value for the next recognition query.

This function adds a data store ID of a previously stored audio utterance for the next recognition query.
The data store is identified with the data store ID that is available after a successful ::AB_SpeechStatisticsStore
call by calling ::AB_SpeechStatisticsGetResults.

Valid field names are: <br>
\a "address-full" <br>
\a "address-street" <br>
\a "address-cross-street" <br>
\a "address-city" <br>
\a "address-state" <br>
\a "address-postalcode" <br>
\a "airport" <br>
\a "local-search-poi" <br>

@todo (BUG 56471): determine where complete list of valid field names is

@param parameters Pointer to AB_SpeechStatisticsParameters object
@param fieldName Name of field that utterance in data store is for
@param fieldText Text currently in text box for this field
@param utterance Encoding Audio data encoding type ("wav", "amr", "qcp")
@param dataStoreId Datastore ID of previously uploaded audio recording

@return NB_error

@see AB_SpeechStatisticsParametersCreate
@see AB_SpeechStatisticsParametersClearFields
*/
AB_DEC NB_Error AB_SpeechStatisticsParametersAddUtteranceFromDataStore(AB_SpeechStatisticsParameters* parameters, const char* fieldName, const char* fieldText, const char* utteranceEncoding, const char* dataStoreId);


/*! Clear all fields for the next recognition query.

This function clears all previously added field items for a recognition query.

@see parameters Pointer to AB_SpeechStatisticsParameters object
@return NB_error

@see AB_SpeechStatisticsParametersCreate
@see AB_SpeechStatisticsParametersClearFields
*/
AB_DEC NB_Error AB_SpeechStatisticsParametersClearFields(AB_SpeechStatisticsParameters* parameters);


/*! Set timing statistics for ASR

This function allows optional timing information to be provided for ASR interaction.

@param parameters Pointer to AB_SpeechRecognitionParameters object
@param timing Comma-separated timing information, described below

The timing parameter should be formatted as CSV in the form "name:field:value".
All values are in milliseconds and given as an offset from when the user first entered the screen.
If a timing value is not associated with a field, the field should be left blank ("name::value").

Field-specific names: <br>
\a BOS beginning of recording (SEND pressed) <br>
\a EOS end of speech (SEND released) <br>
\a DSQ Datastore query sent <br>
\a DSR Datastore reply received <br>

General names: <br>
\a ARQ Asr-reco query sent <br>
\a ARR Asr-reco reply received <br>
\a EOI End of interaction (user leaves screen) <br>

@return NB_Error
*/
AB_DEC NB_Error AB_SpeechStatisticsParametersSetTiming(AB_SpeechStatisticsParameters* parameters, const char* timing);

/*! @} */

#endif
