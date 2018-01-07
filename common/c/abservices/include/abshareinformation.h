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

    @file abshareinformation.h
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

#ifndef ABSHAREINFORMATION_H
#define ABSHAREINFORMATION_H

#include "abexp.h"
#include "paltypes.h"
#include "nberror.h"


/*!
    @addtogroup abshareinformation
    @{
*/


/*! [DEPRECATED] Delivery status of a sent message to a recipient.  Returned by AB_ShareInformationGetResult(). */
typedef enum
{
    ASPMS_Success = 0,      /*!< Message send successful */
    ASPMS_UnknownError,     /*!< Unknown error sending message */
    ASPMS_InboxFull,        /*!< Recipient inbox is full */
    ASPMS_InvalidNumber     /*!< Invalid recipient number */
} AB_SharePlaceMessageStatus;


/*! @struct AB_ShareInformation
    Opaque data structure that defines the returned data of a share message operation
*/
typedef struct AB_ShareInformation AB_ShareInformation;


/*! Get the count of share message results for each recipient

@param information Address of a valid AB_ShareInformation object
@param resultCount Address of a uint32 for the count to be stored at

@returns NB_Error
*/
AB_DEC NB_Error AB_ShareInformationGetResultCount(AB_ShareInformation* information, uint32* resultCount);


/*! [DEPRECATED] Get the send status, recipient name and id of the share message recipient by index

Used only for requests made using a parameter object created with the deprecated AB_ShareParametersCreateMessage()
function.  Use AB_ShareInformationGetResultEx() to retrieve results for requests made with parameter objects created with the 
AB_ShareParametersCreatePlaceMessage(), AB_ShareParametersCreateShareMessage(), or AB_ShareParametersCreateTextMessage() functions.

Returns the results from a for the recipient specified by index.

@param information Address of a valid AB_ShareInformation object
@param index The index of the result to retrieve; must be greater than or equal to 0 and less than the result count returned by AB_ShareInformationGetResultCount()
@param status Address of AB_SharePlaceMessageStatus variable for requested share message
@param to Address of buffer for message recipient to be stored
@param toSize Size of buffer for message recipient
@param id Address of buffer for recipient ID
@param idSize Size of buffer for recipient ID

@returns NB_Error

@see AB_ShareInformationGetResultCount
*/
AB_DEC NB_Error AB_ShareInformationGetResult(
    AB_ShareInformation* information,
    uint32 index,
    AB_SharePlaceMessageStatus* status,
    char* to,
    nb_size toSize,
    char* id,
    nb_size idSize
);


/*! Get the result of recipient send message request by index

@param information Address of a valid AB_ShareInformation object
@param index The index of the result to retrieve; must be greater than or equal to 0 and less than the result count returned by AB_ShareInformationGetResultCount()
@param errorCode Address of buffer to copy error code to
@param errorMessage Address of buffer to copy error message to 
@param errorMessageSize Size of buffer for errorMessage
@param to Address of buffer to copy message recipient to
@param toSize Size of buffer for to
@param id Address of buffer copy recipient ID to
@param idSize Size of buffer for recipient ID

@returns NB_Error

@see AB_ShareInformationGetResultCount
*/
AB_DEC NB_Error
AB_ShareInformationGetResultEx(
    AB_ShareInformation* information,
    uint32 index,
    uint32* errorCode,
    char* errorMessage,
    nb_size errorMessageSize,
    char* to,
    nb_size toSize,
    char* id,
    nb_size idSize
);


/*! Destroy the AB_ShareInformation object

This function should be called to destroy the AB_ShareInformation object obtained from
the AB_ShareHandlerGetInformation() function.

@param information Address of a valid AB_ShareInformation object

@returns NB_Error

@see AB_ShareHandlerGetInformation
*/
AB_DEC NB_Error AB_ShareInformationDestroy(AB_ShareInformation* information);

/*! @} */

#endif
