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

@file abersinformation.h
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

#ifndef AB_ERS_INFORMATION_H
#define AB_ERS_INFORMATION_H

#include "abexp.h"
#include "nberror.h"
#include "paltypes.h"
#include "nbplace.h"
#include "csltypes.h"

/*!
    @addtogroup abersinformation
    @{
*/

typedef struct AB_ErsInformation AB_ErsInformation;

typedef struct
{
    const char*     name;
    NB_Phone        phone;
} AB_ErsContact;


/*! Destroy the AB_ErsInformation object

@param information Address of a valid AB_ErsInformation object
@returns NB_Error
*/
AB_DEC NB_Error AB_ErsInformationDestroy(AB_ErsInformation* information);

/*! Get the number of ERS contacts

@param information Address of a valid AB_ErsInformation object
@param count On success, receives the number of contacts
@return NB_Error
*/
AB_DEC NB_Error AB_ErsInformationGetContactCount(AB_ErsInformation* information, uint32* count);

/*! Get an ERS contact

@param information Address of a valid AB_ErsInformation object
@param index The index of the contact to retrieve
@param contact On success, receives a pointer to a new AB_ErsContact.  A valid AB_ErsContact must be deleted with AB_ErsContactDestroy
@return NB_Error
*/
AB_DEC NB_Error AB_ErsInformationGetContact(AB_ErsInformation* information, uint32 index, AB_ErsContact** contact);

/*! Destroy a AB_ErsContact object

@param contact Address of a valid AB_ErsContact object
@returns NB_Error
*/
AB_DEC NB_Error AB_ErsContactDestroy(AB_ErsContact* contact);


/*! @} */

#endif
