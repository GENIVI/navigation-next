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

    @file     nblocationtypes.h
    @date     05/05/2011
    @defgroup NBLOCATIONSERVICES Location Services

    This is the C-Style DLL interface to the NBLocationTypes
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#pragma once
#ifndef __NBLOCATIONTYPES_H__
#define __NBLOCATIONTYPES_H__


#include "nberror.h"
#include "palfile.h"

// Types and data structures .......................................................................
/*!
@defgroup NBLOCATIONTYPES Types
@{
*/

/*! Location Services constants
*/

/*! Local time in seconds since Jan 6, 1980
*/
typedef unsigned int NB_LS_LocalTime;

/*! @struct NB_LS_LocationConfig
*   <summary>
*       Defines a configuration parameters for NB_LocationInitialize
*   </summary>
*/
typedef struct
{
    nb_boolean emulationMode;          /*!< If TRUE, do emulation */
    char emulationFilename[PAL_FILE_MAX_NAME_LENGTH];  /*!< Path to the emulation file that contains location fixes. */
    nb_boolean warmUpFix;              /*!< If TRUE, LK will request a fix during initialization to warm up the Hardware. */
    nb_boolean collectWiFiProbes;      /*!< If FALSE, LK will not do Wifi probes and will not use Wifi info to get location.
                                    If TRUE, LK will do Wifi probes on GPS fixes and use Wifi info to get location.
                                    Note: If collectWiFiProbes is TRUE, Wifi will be powered on even if it was off on the
                                    mobile device. After Wifi scan complete the previous Wifi power state will be set. */
    int emulationPlayStart;         /*!< Number of seconds to skip the playback file. */
} NB_LS_LocationConfig;

/*! @struct NB_LS_Location
*   <summary>
*       The structure contains the location information returned to the caller every time there is new positioning data available
*   </summary>
*/
typedef struct
{
    double          altitude;           /*!< Altitude in meters */
    NB_LS_LocalTime gpsTime;            /*!< Time in seconds since the GPS Epoch: Midnight, January 6, 1980. */
    double          heading;            /*!< Heading in degrees. */
    double          horizontalUncertaintyAlongAxis;     /*!< Standard deviation of horizontal uncertainty along the axis of uncertainty */
    double          horizontalUncertaintyAlongPerpendicular;    /*!< Standard deviation of horizontal uncertainty perpendicular to the axis of uncertainty */
    double          horizontalUncertaintyAngleOfAxis;   /*!< Heading in degrees of the axis of uncertainty */
    double          horizontalVelocity; /*!< Horizontal velocity in meters/second */
    double          latitude;           /*!< Latitude in degrees */
    double          longitude;          /*!< Longitude in degrees */
    int             numberOfSatellites; /*!< Number of satellites acquired */
    int             status;             /*!< Response status */
    int             utcOffset;          /*!< Difference in seconds between GPS time and UTC time (leap seconds) */
    unsigned int    valid;              /*!< NB_LS_LocationValid flags indicating which fields in the struct have valid values */
    double          verticalUncertainty;/*!< Standard deviation of vertical uncertainty */
    double          verticalVelocity;   /*!< Vertical velocity in meters/seconds */
} NB_LS_Location;

/*! @struct NB_LocationContext
*   <summary>
*       The NB_LocationContext maintains a context for all Loaction kit functionality.
*   </summary>
*   This is an opaque structure, use the API calls to create and access it.
*/
typedef struct NB_LocationContext NB_LocationContext;

/*! Location Device status
*/
typedef enum
{
    NBPGS_Undefined,
    NBPGS_InitializePending,
    NBPGS_DestroyPending,
    NBPGS_Initialized,
    NBPGS_Oneshot,
    NBPGS_Tracking_GPS,
    NBPGS_Tracking_Network,
    NBPGS_Suspended,
    NBPGS_Resume,
} NB_LS_LocationState;

/*! Valid flags set when a location fix is received.
    Used to indicate which fields in the structure contain valid data.
    These values are ORed together, so multiple fields can be valid.
*/
typedef enum
{
    NBPGV_None                    = 0,
    NBPGV_Latitude                = (1<<0),
    NBPGV_Longitude               = (1<<1),
    NBPGV_Altitude                = (1<<2),
    NBPGV_Heading                 = (1<<3),
    NBPGV_HorizontalVelocity      = (1<<4),
    NBPGV_VerticalVelocity        = (1<<5),
    NBPGV_HorizontalUncertainty   = (1<<6),
    NBPGV_AxisUncertainty         = (1<<7),
    NBPGV_PerpendicularUncertainty= (1<<8),
    NBPGV_VerticalUncertainty     = (1<<9),
    NBPGV_UTCOffset               = (1<<10),
    NBPGV_SatelliteCount          = (1<<11)
} NB_LS_LocationValid;

/*! Type of fix for NB_LocationGetOneFix
*/
typedef enum
{
    NBFixType_Fast,
    NBFixType_Normal,
    NBFixType_Accurate,
} NB_LS_FixType;

#ifndef CALLBACK
#define CALLBACK
#endif

// Callbacks .......................................................................................

/*!  Location Device state callback function prototype

    @param appData Application data
    @param context Location context object
*/

typedef void (CALLBACK* NB_LocationDeviceStateCallback)(
    void* appData,
    NB_LocationContext* context,
    NB_LS_LocationState state
    );

/*!  Location initialization callback function prototype

    @param appData Application data
    @param context Location context object
*/
typedef void (CALLBACK* NB_LocationInitializeCallback)(
    void* appData,
    NB_LocationContext* context
    );

/*!  Location callback function prototype

    @param appData Application data
    @param context Location context object
    @param location Current location information
    @param error Error status
*/
typedef void (CALLBACK* NB_LS_LocationCallback)(
    void* appData,
    NB_LocationContext* context,
    const NB_LS_Location* location,
    NB_Error           error
    );

#endif // __NBLOCATIONTYPES_H__

/*! @} */