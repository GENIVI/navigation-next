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

    @file     nbreversegeocodeparameters.h
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

#ifndef NBREVERSEGEOCODEPARAMETERS_H
#define NBREVERSEGEOCODEPARAMETERS_H

#include "paltypes.h"
#include "navpublictypes.h"


/*!
    @addtogroup nbreversegeocodeparameters
    @{
*/


typedef struct NB_ReverseGeocodeParameters NB_ReverseGeocodeParameters;


/*! Create reverse geocode parameters

@param context NAVBuilder Context Instance
@param point The point to reverse geocode
@param routable Set to non-zero to return a routable address, if possible; zero otherwise.  A routable address may not be the closest address to the point
@param parameters On success, a newly created NB_ReverseGeocodeParameters object; NULL otherwise.  A valid object must be destroyed using NB_ReverseGeocodeParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_ReverseGeocodeParametersCreate(NB_Context* context, NB_LatitudeLongitude* point, nb_boolean routable, NB_ReverseGeocodeParameters** parameters);


/*! Create reverse geocode parameters with a map scale

@param context NAVBuilder Context Instance
@param point The point to reverse geocode
@param routable Set to non-zero to return a routable address, if possible; zero otherwise.  A routable address may not be the closest address to the point
@param scale A map scale value to use when determining the level of detail to reverse geocode to
@param parameters On success, a newly created NB_ReverseGeocodeParameters object; NULL otherwise.  A valid object must be destroyed using NB_ReverseGeocodeParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_ReverseGeocodeParametersCreateWithScale(NB_Context* context, NB_LatitudeLongitude* point, nb_boolean routable, double scale, NB_ReverseGeocodeParameters** parameters);


/*! Setups avoid local road flag to reverse geocode parameters. This flag will allow the server to understand that the reverse geocode query should avoid local road being sent in the reply.
    The current implementation requires that the client only send this flag from the carousel screen. By default this flag set to FALSE.

@param context NAVBuilder Context Instance
@param parameters NB_ReverseGeocodeParameters to be updated with avoild local road flag
@param avoidLocalRoad Pass TRUE to setup avoid local road, and FALSE otherwise.
@returns NB_Error
*/
NB_DEC NB_Error NB_ReverseGeocodeParametersSetAvoidLocalRoad(NB_Context* context, NB_ReverseGeocodeParameters* parameters, nb_boolean avoidLocalRoad);


/*! Destroy a previously created ReverseGeocodeParameters object

@param parameters A NB_ReverseGeocodeParameters object created with NB_ReverseGeocodeParametersCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_ReverseGeocodeParametersDestroy(NB_ReverseGeocodeParameters* parameters);


/*! @} */

#endif
