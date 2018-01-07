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

   @file     nbmetadatainformation.h
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NBMETADATAINFORMATION_H
#define NBMETADATAINFORMATION_H

#include "nbexp.h"
#include "nberror.h"
#include "nbextapptemplatescontent.h"

/*!
    @addtogroup nbmetadatainformation
    @{
*/


/*! @struct NB_MetadataInformation
*/
typedef struct NB_MetadataInformation NB_MetadataInformation;


/*! Destroy a previously created NB_MetadataInformation object

@param information An NB_MetadataInformation object created with NB_MetadataHandlerGetInformation()
@returns NB_Error
*/
NB_DEC NB_Error NB_MetadataInformationDestroy(NB_MetadataInformation* information);


/*! Get the city summary display

@param information An NB_MetadataInformation object created with NB_MetadataHandlerGetInformation()
@param citySummaryDisplay On success, returns pointer to the internal string
@returns NB_Error
*/
NB_DEC NB_Error NB_MetadataInformationGetCitySummaryDisplay(NB_MetadataInformation* information, const char** citySummaryDisplay);


/*! Get the ext app templates content count

@param information An NB_MetadataInformation object created with NB_MetadataHandlerGetInformation()
@param count Contains number of ext app templates available
@returns NB_Error
*/
NB_DEC NB_Error NB_MetadataInformationGetExtAppTemplatesContentCount(NB_MetadataInformation* information, uint32* count);


/*! Get the ext app templates content by a particular index

@param information An NB_MetadataInformation object created with NB_MetadataHandlerGetInformation()
@param extAppTemplatesContent Pointer to already allocated NB_ExtAppTemplatesContent to be filled with data
@param index Index of ext app templates content to be retieved. Should be less than returned by NB_MetadataInformationGetExtAppTemplatesContentCount.
@returns NB_Error
*/
NB_DEC NB_Error NB_MetadataInformationGetExtAppTemplatesContent(NB_MetadataInformation* information, NB_ExtAppTemplatesContent* extAppTemplatesContent, uint32 index);

#endif
