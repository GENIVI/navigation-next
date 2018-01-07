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

    @file abshareparameters.h
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

#ifndef ABSHAREPARAMETERS_H
#define ABSHAREPARAMETERS_H

#include "abexp.h"

#include "nbcontext.h"
#include "nbplace.h"

/*!
    @addtogroup abshareparameters
    @{
*/


/*! @struct AB_ShareParameters
    Opaque data structure that defines the parameters of a share message operation
*/
typedef struct AB_ShareParameters AB_ShareParameters;

/*! Create parameters for a share message operation

This function is deprecated; @deprecated use AB_ShareParametersCreatePlaceMessage(), AB_ShareParametersCreateShareMessage(),
AB_ShareParametersCreatePlaceMessageWithBannerId() or AB_ShareParametersCreateTextMessage() instead.

This function creates an AB_ShareParameters object initialized with the specified parameters.
AB_ShareParametersAddAdditionalRecipient() can be used to add additional message recipients.
The AB_ShareParameters object can then be passed to a AB_ShareHandlerStartRequest() call.
The created parameter object should be freed using AB_ShareParametersDestroy() when no longer needed.

@param context Address of current NB_Context
@param fromMobileDirectoryNumber MDN of message sender
@param fromName Name of message sender
@param toEmailOrPhone Email or SMS phone number of message recipient
@param message Body of message
@param place Address of NB_Place object for message
@param shareParameters On success, a newly created AB_ShareParameters object; NULL otherwise.  A valid object must be destroyed using AB_ShareParametersDestroy().

@returns NB_Error

@see AB_ShareParametersAddAdditionalRecipient
@see AB_ShareHandlerStartRequest
@see AB_ShareParametersDestroy
*/
AB_DEC NB_Error AB_ShareParametersCreateMessage(
    NB_Context* context,
    const char* fromMobileDirectoryNumber,
    const char* fromName,
    const char* toEmailOrPhone,
    const char* message,
    const NB_Place* place,
    AB_ShareParameters** shareParameters
);

/*! Create parameters for a send plain text message operation

This function creates an AB_ShareParameters object initialized with the specified parameters for a
plain text message to be sent.  AB_ShareParametersAddAdditionalRecipient() can be used to add additional message recipients.
The AB_ShareParameters object can then be passed to a AB_ShareHandlerStartRequest() call.
The created parameter object should be freed using AB_ShareParametersDestroy() when no longer needed.

@param context Address of current NB_Context
@param fromMobileDirectoryNumber MDN of message sender
@param fromName Name of message sender
@param toEmailOrPhone Email or SMS phone number of message recipient
@param message Body of message
@param shareParameters On success, a newly created AB_ShareParameters object; NULL otherwise.  A valid object must be destroyed using AB_ShareParametersDestroy().

@returns NB_Error

@see AB_ShareParametersAddAdditionalRecipient
@see AB_ShareHandlerStartRequest
@see AB_ShareParametersDestroy
*/
AB_DEC NB_Error AB_ShareParametersCreateTextMessage(
    NB_Context* context,
    const char* fromMobileDirectoryNumber,
    const char* fromName,
    const char* toEmailOrPhone,
    const char* message,
    AB_ShareParameters** shareParameters
);

/*! Create parameters for a send place message operation

This function is deprecated; @deprecated use AB_ShareParametersCreatePlaceMessage(), AB_ShareParametersCreateShareMessage(),
AB_ShareParametersCreatePlaceMessageWithBannerId() or AB_ShareParametersCreateTextMessage() instead

This function creates an AB_ShareParameters object initialized with the specified parameters for a
place message to be sent.  AB_ShareParametersAddAdditionalRecipient() can be used to add additional message recipients.
The AB_ShareParameters object can then be passed to a AB_ShareHandlerStartRequest() call.
The created parameter object should be freed using AB_ShareParametersDestroy() when no longer needed.

@param context Address of current NB_Context
@param fromMobileDirectoryNumber MDN of message sender
@param fromName Name of message sender
@param toEmailOrPhone Email or SMS phone number of message recipient
@param message Body of message
@param place Address of NB_Place object for message
@param shareParameters On success, a newly created AB_ShareParameters object; NULL otherwise.  A valid object must be destroyed using AB_ShareParametersDestroy().

@returns NB_Error

@see AB_ShareParametersAddAdditionalRecipient
@see AB_ShareHandlerStartRequest
@see AB_ShareParametersDestroy
*/
AB_DEC NB_Error AB_ShareParametersCreatePlaceMessage(
    NB_Context* context,
    const char* fromMobileDirectoryNumber,
    const char* fromName,
    const char* toEmailOrPhone,
    const char* message,
    const NB_Place* place,
    AB_ShareParameters** shareParameters
    );


/*! Create parameters for a send place message operation

This function creates an AB_ShareParameters object initialized with the specified parameters for a
place message to be sent.  AB_ShareParametersAddAdditionalRecipient() can be used to add additional message recipients.
The AB_ShareParameters object can then be passed to a AB_ShareHandlerStartRequest() call.
The created parameter object should be freed using AB_ShareParametersDestroy() when no longer needed.

@param context Address of current NB_Context
@param fromMobileDirectoryNumber MDN of message sender
@param fromName Name of message sender
@param toEmailOrPhone Email or SMS phone number of message recipient
@param message Body of message
@param place Address of NB_Place object for message
@param bannerId Banner ID for message (optional)
@param shareParameters On success, a newly created AB_ShareParameters object; NULL otherwise.  A valid object must be destroyed using AB_ShareParametersDestroy().

@returns NB_Error

@see AB_ShareParametersAddAdditionalRecipient
@see AB_ShareHandlerStartRequest
@see AB_ShareParametersDestroy
*/
AB_DEC NB_Error AB_ShareParametersCreatePlaceMessageWithBannerId(
    NB_Context* context,
    const char* fromMobileDirectoryNumber,
    const char* fromName,
    const char* toEmailOrPhone,
    const char* message,
    const NB_Place* place,
    const char* bannerId,
    AB_ShareParameters** shareParameters
    );


/*! Create parameters for a send share message operation

This function creates an AB_ShareParameters object initialized with the specified parameters for a
share message to be sent to a social networking site (e.g. Facebook).
AB_ShareParametersAddAdditionalRecipient() can be used to add additional message recipients.
The AB_ShareParameters object can then be passed to a AB_ShareHandlerStartRequest() call.
The created parameter object should be freed using AB_ShareParametersDestroy() when no longer needed.

@param context Address of current NB_Context
@param fromMobileDirectoryNumber MDN of message sender
@param fromName Name of message sender
@param toEmailOrPhone Email or SMS phone number of message recipient
@param message Body of message
@param place Address of NB_Place object for message
@param shareParameters On success, a newly created AB_ShareParameters object; NULL otherwise.  A valid object must be destroyed using AB_ShareParametersDestroy().

@returns NB_Error

@see AB_ShareParametersAddAdditionalRecipient
@see AB_ShareHandlerStartRequest
@see AB_ShareParametersDestroy
*/
AB_DEC NB_Error AB_ShareParametersCreateShareMessage(
    NB_Context* context,
    const char* fromMobileDirectoryNumber,
    const char* fromName,
    const char* toEmailOrPhone,
    const char* message,
    const NB_Place* place,
    AB_ShareParameters** shareParameters
);


/*! Adds an additional recipient to a share message parameter object

This function adds an additional share message recipient to the specified AB_ShareParameters object.

@param parameters Address of a valid AB_ShareParameters object
@param toEmailOrPhone Email or SMS phone number of message recipient

@returns NB_Error

@see AB_ShareParametersCreateMessage
*/
AB_DEC NB_Error AB_ShareParametersAddAdditionalRecipient(AB_ShareParameters* parameters, const char* toEmailOrPhone);


/*! Specifes new server value for a given key.
 
 @param parameters An AB_ShareParameters object
 @param key Key of setting that value is being set for
 @param value  New value for specifed setting
 @returns NB_Error
 */
AB_DEC NB_Error AB_ShareParametersAddSetStringValue(AB_ShareParameters* parameters, const char* key, const char* value);


/*! Destroy a previously created parameters object

@param parameters Address of a valid AB_ShareParameters object

@returns NB_Error

@see AB_ShareParametersCreateMessage
*/
AB_DEC NB_Error AB_ShareParametersDestroy(AB_ShareParameters* parameters);


/*! @} */

#endif
