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

    @file abprofileinformation.h
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

#ifndef ABPROFILEINFO_H
#define ABPROFILEINFO_H

#include "abexp.h"
#include "nberror.h"


/*!
    @addtogroup abprofileinformation
    @{
*/


/*! @struct AB_ProfileInformation
Information about the results of a profile request
*/
typedef struct AB_ProfileInformation AB_ProfileInformation;

/*! Gets profile value from server request.

@param information  An AB_ProfileInformation object
@param key Key of profile setting to get value of
@param value Pointer to a pointer to requested value
@return a NB_Error code, indicating success or failure
*/
AB_DEC NB_Error AB_ProfileInformationGetStringValue(AB_ProfileInformation* profile, const char* key,  const char** value);


/*! Gets error code and description from query.

@param information  An AB_ProfileInformation object
@param description Pointer to a pointer to requested error description, if any
@returns error code returned from query, 0 if no error returned
*/
AB_DEC uint32 AB_ProfileInformationGetError(AB_ProfileInformation* profile, const char** errorDescription);


/*! Destroy a previously created ProfileInformation object

@param information An AB_ProfileInformation object created with AB_ProfileHandlerGetInformation()
@returns NB_Error
*/
AB_DEC NB_Error AB_ProfileInformationDestroy(AB_ProfileInformation* information);


#endif
