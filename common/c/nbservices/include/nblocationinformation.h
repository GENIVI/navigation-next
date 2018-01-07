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

    @file nblocationinformation.h
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

#ifndef NB_LOCATIONINFORMATION_H
#define NB_LOCATIONINFORMATION_H

#include "nbexp.h"
#include "nberror.h"
#include "navpublictypes.h"
#include "nbcontext.h"

/*!
    @addtogroup nblocationinformation
    @{
*/

/*! @struct NB_LocationInformation
Location information from a cell ID query
*/
typedef struct NB_LocationInformation NB_LocationInformation;

/*! This structure describes location tile.
    @see NB_LocationInformationGetLocationTile
*/
typedef struct
{
    uint32          tileX;              /*!< x coordinate of the tile. */
    uint32          tileY;              /*!< y coordinate of the tile. */
    // Don't free it. This is a pointer copy. This memory is deallocated in NB_LocationInformationDestroy.
    byte*           dbFragment;         /*!< Database fragment containing the Cell ID and Wi-Fi data for the tile. */
    uint32          dbFragmentSize;

} NB_LocationTile;

typedef enum {
    NB_LRS_Success          = 0,    /*!< location request successful */
    NB_LRS_NotFound         = 1,    /*!< location not found */
    NB_LRS_ServerFailed     = 2,    /*!< location request failed/timed out on server */
    NB_LRS_InvalidPoint     = 3     /*!< invalid point specified as moving */
} NB_LocationRequestStatus;

/*! Destroy the NB_LocationInformation object

@param information Address of a valid NB_LocationInformation object
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationInformationDestroy(NB_LocationInformation* information);

/*! Get the requested network location position

@param information Address of a valid NB_LocationInformation object
@param requestStatus Pointer to buffer for location request status
@param position Pointer to buffer for cell ID position
@param accuracyMeters Pointer to buffer for radius of confidence (in meters) around the position
@param networkLocationSource Pointer to buffer for source used to acquire network location position
@param networkLocationSourceSize Size of locationSource buffer
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationInformationGetNetworkPosition(
    NB_LocationInformation* information,
    NB_LocationRequestStatus* requestStatus,
    NB_LatitudeLongitude* position,
    uint32* accuracyMeters,
    char* networkLocationSource,
    uint32 networkLocationSourceSize);

/*! Get if the configuration setting for the device’s platform is to associate LK with the location setting on the device.

@param information Address of a valid NB_LocationInformation object
@param useLocationSetting Pointer to buffer
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationInformationGetUseLocationSetting(
    NB_LocationInformation* information,
    nb_boolean* useLocationSetting);

/*! Get location tiles containing Cell ID and Wi-Fi positioning database fragments

@param information Address of a valid NB_LocationInformation object
@param index Result index
@param tile LocationTile. Don't free dbFragment inside it. This is a pointer copy. This memory is deallocated in NB_LocationInformationDestroy.
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationInformationGetLocationTile(
    NB_LocationInformation* information,
    int32 index,
    NB_LocationTile* tile);

/*! Get number of location tiles in the server response

@param information Address of a valid NB_LocationInformation object
@param count Number of results in the server response
@return NB_Error
*/
NB_DEC NB_Error NB_LocationInformationGetLocationTilesCount(
    NB_LocationInformation* information, 
    int32* count);

/*! Get the request status

@param information Address of a valid NB_LocationInformation object
@param requestStatus Pointer to buffer for location request status
@returns NB_Error
*/
NB_DEC NB_Error NB_LocationInformationGetRequestStatus(
    NB_LocationInformation* information,
    NB_LocationRequestStatus* requestStatus);

/*! @} */

#endif  // NB_LOCATIONINFORMATION_H
