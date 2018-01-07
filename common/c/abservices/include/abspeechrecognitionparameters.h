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

    @file abspeechrecognitionparameters.h
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

#ifndef ABSPEECHRECOGNITIONPARAMETERS_H
#define ABSPEECHRECOGNITIONPARAMETERS_H

#include "abexp.h"
#include "nbgpstypes.h"

/*! @addtogroup abspeechrecognitionparameters
    @{
*/


/*! @struct AB_SpeechRecognitionParameters
    Opaque data structure that defines the parameters of a speech recognition operation
*/
typedef struct AB_SpeechRecognitionParameters AB_SpeechRecognitionParameters;


/*! Combination of flags that can be specified when calling the AB_SpeechRecognitionParametersCreateWithOptions function. */
typedef enum
{
    AB_ASR_None = 0,                    /*!< No configuration options requested */
    AB_ASR_WantAccuracy = (1 << 0),     /*!< Request accuracy of returned match */
    AB_ASR_WantExtendedCodes = (1 << 1) /*!< Request extended codes */
} AB_SpeechRecognitionConfiguration;


/*! Create an AB_SpeechRecognitionParameters instance.

This function creates and returns a pointer to an AB_SpeechRecognitionParameters instance.
Call AB_SpeechRecognitionParametersDestroy() to destroy the object.

@param context pointer to NB_Context
@param gpsFix pointer to GPS fix if available
@param previousInteractionId pointer to previous ASR interaction ID, if available
@param parameters On success, pointer to the newly created parameter object; NULL otherwise

@return NB_Error

@see AB_SpeechRecognitionParametersDestroy
*/
AB_DEC NB_Error AB_SpeechRecognitionParametersCreate(NB_Context* context, NB_GpsLocation* gpsFix, const char* previousInteractionId, AB_SpeechRecognitionParameters** parameters);


/*! Destroy a previously created SpeechRecognitionParameters object

@param parameters An AB_SpeechRecognitionParameters object created with AB_SpeechRecognitionParametersCreate()
@returns NB_Error
*/
AB_DEC NB_Error AB_SpeechRecognitionParametersDestroy(AB_SpeechRecognitionParameters* parameters);


/*! Add a text field value for the next recognition query.

This function adds a text value for a specified field for the next recognition query.

Valid field names (specified in fieldName parameter) are: <br>
\a "address-full" <br>
\a "address-street" <br>
\a "address-cross-street" <br>
\a "address-city" <br>
\a "address-state" <br>
\a "address-postalcode" <br>
\a "airport" <br>
\a "local-search-poi" <br>

@todo (BUG 56471): determine where complete list of valid field names is

@param parameters Pointer to AB_SpeechRecognitionParameters object
@param fieldName Name of field that text is for
@param fieldText Text currently in this text box for this field
@return NB_error

@see AB_SpeechRecognitionParametersCreate
@see AB_SpeechRecognitionParametersClearFields
*/
AB_DEC NB_Error AB_SpeechRecognitionParametersAddText(AB_SpeechRecognitionParameters* parameters, const char* fieldName, const char* fieldText);


/*! Add a text field value from a specified source for the next recognition query.

This function adds a text value for a specified field for the next recognition query.

Valid field names (specified in fieldName parameter) are: <br>
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

@param parameters Pointer to AB_SpeechRecognitionParameters object
@param fieldName Name of field that text is for
@param fieldText Text currently in this text box for this field
@param fieldSource Source of text provided
@return NB_error

@see AB_SpeechRecognitionParametersCreate
@see AB_SpeechRecognitionParametersClearFields
*/
AB_DEC NB_Error AB_SpeechRecognitionParametersAddTextFromSource(AB_SpeechRecognitionParameters* parameters, const char* fieldName, const char* fieldText, const char* fieldSource);


/*! Add an audio field value for the next recognition query.

This function adds an audio utterance for a specified field for the next recognition query.

Valid field names (specified in fieldName parameter) are: <br>
\a "address-full" <br>
\a "address-street" <br>
\a "address-cross-street" <br>
\a "address-city" <br>
\a "address-state" <br>
\a "address-postalcode" <br>
\a "airport" <br>
\a "local-search-poi" <br>

@todo (BUG 56471): determine where complete list of valid field names is

@param parameters Pointer to AB_SpeechRecognitionParameters object
@param fieldName Name of field that utterance is for
@param fieldText Text currently in text box for this field
@param utteranceEncoding Audio data encoding type ("wav", "amr", "qcp")
@param utteranceData Audio recorded data buffer
@param utteranceSize Size of utteranceData

@return NB_error

@see AB_SpeechRecognitionParametersCreate
@see AB_SpeechRecognitionParametersClearFields
*/
AB_DEC NB_Error AB_SpeechRecognitionParametersAddUtterance(AB_SpeechRecognitionParameters* parameters, const char* fieldName, const char* fieldText, const char* utteranceEncoding, byte* utteranceData, uint32 utteranceSize);


/*! Add a reference to a previously stored audio field value for the next recognition query.

This function adds a data store ID of a previously stored audio utterance for the next recogntion query.
The data store is identified with the data store ID that is available after a successful ::AB_SpeechRecognitionStore
call by calling ::AB_SpeechRecognitionGetResults.

Valid field names (specified in fieldName parameter) are: <br>
\a "address-full" <br>
\a "address-street" <br>
\a "address-cross-street" <br>
\a "address-city" <br>
\a "address-state" <br>
\a "address-postalcode" <br>
\a "airport" <br>
\a "local-search-poi" <br>

@todo (BUG 56471): determine where complete list of valid field names is

@param parameters Pointer to AB_SpeechRecognitionParameters object
@param fieldName Name of field that utterance in data store is for
@param fieldText Text currently in text box for this field
@param utterance Encoding Audio data encoding type ("wav", "amr", "qcp")
@param dataStoreId Datastore ID of previously uploaded audio recording

@return NB_error

@see AB_SpeechRecognitionParametersCreate
@see AB_SpeechRecognitionParametersClearFields
*/
AB_DEC NB_Error AB_SpeechRecognitionParametersAddUtteranceFromDataStore(AB_SpeechRecognitionParameters* parameters, const char* fieldName, const char* fieldText, const char* utteranceEncoding, const char* dataStoreId);


/*! Clear all fields for the next recognition query.

This function clears all previously added field items for a recognition query.

@see parameters Pointer to AB_SpeechRecognitionParameters object
@return NB_error

@see AB_SpeechRecognitionParametersCreate
@see AB_SpeechRecognitionParametersClearFields
*/
AB_DEC NB_Error AB_SpeechRecognitionParametersClearFields(AB_SpeechRecognitionParameters* parameters);


/*! Set the interaction ID for the next recognition query to reference a previous query.

This function sets the interaction ID for the next query.  For the initial recognition request,
the interaction ID is should be empty.  After the initial ::AB_SpeechRecognitionParametersRecognize call, an interaction
ID will be available via ::AB_SpeechRecognitionParametersGetRecognitionResults.  If additional recognition query calls are needed
to disambiguate fields etc., this ID should be supplied to provide a context for the recognition engine.

@param parameters Pointer to AB_SpeechRecognitionParameters object
@param interactionId Interaction ID returned from a previous query, empty string if next query is the initial query

@return NB_error

@see AB_SpeechRecognitionParametersCreate
*/
AB_DEC NB_Error AB_SpeechRecognitionParametersSetInteractionID(AB_SpeechRecognitionParameters* parameters, const char* interactionId);


/*! Set configuration options for AB_SpeechRecognitionParameters

This function allows optional configuration flags to be set for the AB_SpeechRecognintionParameters object.

@param parameters Pointer to AB_SpeechRecognitionParameters object
@param options Combination of desired AB_SpeechRecognitionConfiguration option flags

@return NB_Error

@see AB_SpeechRecognitionParametersDestroy
*/
AB_DEC NB_Error AB_SpeechRecognitionParametersSetConfigurationOptions(AB_SpeechRecognitionParameters* parameters, uint32 options);


/*! @} */

#endif
