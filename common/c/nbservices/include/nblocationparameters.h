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

    @file nblocationparameters.h
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

#ifndef NB_LOCATIONPARAMETERS_H
#define NB_LOCATIONPARAMETERS_H

#include "nbexp.h"
#include "nbcontext.h"

/*!
    @addtogroup nblocationparameters
    @{
*/

// The cause code for the invalidation:
typedef enum {
    NB_CLMPI_None        = 0,
    NB_CLMPI_OutsideCell = 1,        // Outside Cell ID radius
    NB_CLMPI_Mismatched  = 2         // Mismatched position from neighbors
} NB_CauseLocationMovedPointInvalidation;

typedef struct NB_LocationParameters NB_LocationParameters;

/*! Create parameters for a location request

@param context Address of current NB_Context
@param parameters On success, a newly created NB_LocationParameters object; NULL otherwise.  A valid object must be destroyed using NB_LocationParametersDestroy().
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationParametersCreate(
    NB_Context* context,
    NB_LocationParameters** parameters);

/*! Destroy a previously created parameters object

@param parameters Address of a valid NB_LocationParameters object
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationParametersDestroy(NB_LocationParameters* parameters);

/*! Add parameters for a GSM location request

@param parameters Address of a valid NB_LocationParameters object
@param mobileCountryCode Mobile country code
@param mobileNetworkCode Mobile network code
@param locationAreaCode Location area code
@param cellId Cell ID
@param signalStrengthDbm Signal strength in dbm
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationParametersAddGsm(
    NB_LocationParameters* parameters,
    uint32 mobileCountryCode,
    uint32 mobileNetworkCode,
    uint32 locationAreaCode,
    uint32 cellId,
    int16 signalStrengthDbm);

/*! Add parameters for a CDMA location request

@param parameters Address of a valid NB_LocationParameters object
@param systemId System ID
@param networkId Network ID
@param cellId Cell ID
@param signalStrengthDbm Signal strength in dbm
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationParametersAddCdma(
    NB_LocationParameters* parameters,
    uint32 systemId,
    uint32 networkId,
    uint32 cellId,
    int16 signalStrengthDbm);

/*! Add parameters for Wifi location request

@param parameters Address of a valid NB_LocationParameters object
@param macAddress MAC address of the Wifi node acquired from the device
@param signalStrengthDbm Signal strength in dbm
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationParametersAddWifi(
    NB_LocationParameters* parameters,
    const char* macAddress,
    int16 signalStrengthDbm);

/*! Add parameters for Proxy API-key request

@param parameters Address of a valid NB_LocationParameters object
@param key Location servlet will use the Api Key passed inside of it as the one making the location request for rate limiting purposes
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationParametersSetProxyApiKey(
    NB_LocationParameters* parameters,
    const char* key);

/*! Add parameters for want-location-tiles and want-location-settings

@param parameters Address of a valid NB_LocationParameters object
@param wantLocationTiles If this element is present, the server will return location-tile elements when available in the reply.
@param wantLocationSetting If this element is present, the location reply might include an optional use-location-setting element that specifies if LocationKit has to associate its behavior with the current location setting in the phone.
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationParametersAskWanted(
    NB_LocationParameters* parameters,
    nb_boolean wantLocationTiles,
    nb_boolean wantLocationSetting);

/*! Add parameters for Wi-Fi access point that was detected as moving during validation

@param parameters Address of a valid NB_LocationParameters object
@param macAddress MAC address of the Wifi node acquired from the device
@param cause The cause code for the invalidation
@param time Time (UTC) when the invalidation happened
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationParametersAddMovingWifi(
    NB_LocationParameters* parameters,
    const char* macAddress,
    NB_CauseLocationMovedPointInvalidation cause,
    uint32 time);
/*! @} */

#endif  // NB_LOCATIONPARAMETERS_H
