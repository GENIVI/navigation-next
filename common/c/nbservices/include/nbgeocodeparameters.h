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

    @file     nbgeocodeparameters.h
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

#ifndef NBGEOCODEPARAMETERS_H
#define NBGEOCODEPARAMETERS_H


#include "paltypes.h"
#include "navpublictypes.h"
#include "nbcontext.h"
#include "nbgeocodeinformation.h"
#include "nbiterationcommand.h"


/*!
    @addtogroup nbgeocodeparameters
    @{
*/


/*! @struct NB_GeocodeParameters
Opaque data structure that defines the parameters of a geocode operation 
*/
typedef struct NB_GeocodeParameters NB_GeocodeParameters;


/*! @struct NB_Address
All possible information necessary to geocode an address
*/
/// @todo (Bug 55812) This needs to go someplace else
typedef struct {
	char	number[NB_LOCATION_MAX_STREETNUM_LEN + 1];
	char	street[NB_LOCATION_MAX_STREET_LEN + 1];
	char	city[NB_LOCATION_MAX_CITY_LEN + 1];
	char	county[NB_LOCATION_MAX_COUNTY_LEN + 1];
	char	state[NB_LOCATION_MAX_STATE_LEN + 1];
	char	postal[NB_LOCATION_MAX_POSTAL_LEN + 1];
	char	country[NB_LOCATION_MAX_COUNTRY_LEN + 1];
} NB_Address;


/*! Create geocode parameters for an address search

@param context NAVBuilder Context Instance
@param address The address to search for
@param size The slice size used when retrieving multiple matches from the server
@param parameters On success, a newly created NB_GeocodeParameters object; NULL otherwise.  A valid object must be destroyed using NB_GeocodeParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_GeocodeParametersCreateAddress(NB_Context* context, NB_Address* address, uint32 size, NB_GeocodeParameters** parameters);


/*! Create geocode parameters for an airport search

@param context NAVBuilder Context Instance
@param airport The airport name, code or region
@param size The slice size used when retrieving multiple matches from the server
@param parameters On success, a newly created NB_GeocodeParameters object; NULL otherwise.  A valid object must be destroyed using NB_GeocodeParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_GeocodeParametersCreateAirport(NB_Context* context, const char* airport, uint32 size, NB_GeocodeParameters** parameters);


/*! Create geocode parameters for a free form text search

@param context NAVBuilder Context Instance
@param address The address to geocode
@param country Optional country code to use when matching.  May be NULL
@param size The slice size used when retrieving multiple matches from the server
@param parameters On success, a newly created NB_GeocodeParameters object; NULL otherwise.  A valid object must be destroyed using NB_GeocodeParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_GeocodeParametersCreateFreeForm(NB_Context* context, const char* address, const char* country, uint32 size, NB_GeocodeParameters** parameters);


/*! Create geocode parameters for an intersection search

@param context NAVBuilder Context Instance
@param address One of the streets to search for
@param crossStreet The name of the cross street
@param size The slice size used when retrieving multiple matches from the server
@param parameters On success, a newly created NB_GeocodeParameters object; NULL otherwise.  A valid object must be destroyed using NB_GeocodeParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_GeocodeParametersCreateIntersection(NB_Context* context, NB_Address* address, const char* crossStreet, uint32 size, NB_GeocodeParameters** parameters);


/*! Create geocode parameters for another geocode result set

@param context NAVBuilder Context Instance
@param information A previously returned NB_GeocodeInformation object used as the base for the iteration
@param command The iteration command to use to get additional results
@param parameters On success, a newly created NB_GeocodeParameters object; NULL otherwise.  A valid object must be destroyed using NB_GeocodeParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_GeocodeParametersCreateIteration(NB_Context* context, NB_GeocodeInformation* information, NB_IterationCommand command, NB_GeocodeParameters** parameters);


/*! Set country for geocode parameters

@param parameters A NB_GeocodeParameters object created with a call to one of the NB_GeocodeParametersCreate functions
@param country Valid country to set for the given parameters
@returns NB_Error
*/
NB_DEC NB_Error NB_GeocodeParametersSetCountry(NB_GeocodeParameters* parameters, const char* country);


/*! Destroy a previously created ReverseGeocodeParameters object

@param parameters A NB_GeocodeParameters object created with a call to one of the NB_GeocodeParametersCreate functions
@returns NB_Error
*/
NB_DEC NB_Error NB_GeocodeParametersDestroy(NB_GeocodeParameters* parameters);


/*! Set geographic position for geocode parameters

@param parameters A NB_GeocodeParameters object created with a call to one of the NB_GeocodeParametersCreate functions
@param point Latitude and longitude
@param accuracy Geographic location accuracy
@param time Geographic location time
@returns NB_Error
*/
NB_DEC NB_Error NB_GeocodeParametersSetGeographicPosition(NB_GeocodeParameters* parameters, NB_LatitudeLongitude* point, uint32 accuracy, uint32 time);


/*! @} */

#endif
