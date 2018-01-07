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

    @file     nbgeocodeinformation.h
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

#ifndef NBGEOCODEINFORMATION_H
#define NBGEOCODEINFORMATION_H

#include "paltypes.h"
#include "nblocation.h"
#include "nbextappcontent.h"


/*!
    @addtogroup nbgeocodeinformation
    @{
*/


/*! @struct NB_GeocodeInformation
Information about the results of a geocode request
*/
typedef struct NB_GeocodeInformation NB_GeocodeInformation;


/*! Get information about the slice

@param information A NB_GeocodeInformation object
@param total Optional.  On success, the total number of potential matches.  This parameter can be set to NULL
@param start Optional.  On success, the starting index of the current slice results.  This parameter can be set to NULL
@param end Optional.  On success, the ending index of the current slice results.  This parameter can be set to NULL
@returns NB_Error
*/
NB_DEC NB_Error NB_GeocodeInformationGetSliceInformation(NB_GeocodeInformation* information, int32* total, uint32* start, uint32* end);


/*! Get a geocode result

@param information A NB_GeocodeInformation object
@param index The zero-based index of the result to retrieve; must be less than the difference between the end and start indices returned by NB_GeocodeInformationGetSliceInformation()
@param location The address of a NB_Location object to be filled in with the result
@returns NB_Error
*/
NB_DEC NB_Error NB_GeocodeInformationGetLocation(NB_GeocodeInformation* information, uint32 index, NB_Location* location);


/*! Get a geocode result with accuracy

@param information A NB_GeocodeInformation object
@param index The zero-based index of the result to retrieve; must be less than the difference between the end and start indices returned by NB_GeocodeInformationGetSliceInformation()
@param location The address of a NB_Location object to be filled in with the result
@param accuracy The address of a uint32 to be filled in with the result accuracy (0-100)
@returns NB_Error
*/
NB_DEC NB_Error NB_GeocodeInformationGetLocationWithAccuracy(NB_GeocodeInformation* information, uint32 index, NB_Location* location, uint32* accuracy);


/*! Get a ext app content result

@param information A NB_GeocodeInformation object
@param index The zero-based index of the result to retrieve; must be less than the difference between the end and start indices returned by NB_GeocodeInformationGetSliceInformation()
@param extAppContent The address of a NB_ExtAppContent object to be filled in with the result
@returns NB_Error
*/
NB_DEC NB_Error NB_GeocodeInformationGetLocationExtAppContent(NB_GeocodeInformation* information, uint32 index, NB_ExtAppContent* extAppContent);


/*! Destroy a previously created GeocodeInformation object

@param information A NB_GeocodeHandler object created with NB_GeocodeHandlerGetGeocodeInformation()
@returns NB_Error
*/
NB_DEC NB_Error NB_GeocodeInformationDestroy(NB_GeocodeInformation* information);


/*! @} */

#endif
