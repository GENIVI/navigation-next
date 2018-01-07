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

    @file     nbgpstypes.h
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


#ifndef NBGPSTYPES_H
#define	NBGPSTYPES_H

#include "nbexp.h"
#include "pal.h"
#include "paltypes.h"
#include "nberror.h"

/*! @{ */

/*! GPS location information returned to the caller in WGS-84 format. 
@see PAL_GPS_VALID
*/
typedef struct 
{
    NB_Error status;                                /*!< Response status. */
    nb_gpsTime gpsTime;                             /*!< Time, seconds since the GPS Epoch: Midnight, 1/6/1980 */
    uint32 valid;                                   /*!< NB_GpsValid Flags indicating valid fields in the struct. */
    double latitude;                                /*!< Latitude in Degrees, WGS-84 ellipsoid */    
    double longitude;                               /*!< Longitude in Degrees, WGS-84 ellipsoid */   
    double heading;                                 /*!< Heading in degrees */
    double horizontalVelocity;                      /*!< Horizontal velocity in meters/second */
    double altitude;                                /*!< Altitude in meters, WGS-84 ellipsoid */
    double verticalVelocity;                        /*!< Vertical velocity in meters/second */
    double horizontalUncertaintyAngleOfAxis;        /*!< Heading in degrees of the axis of uncertainty */
    double horizontalUncertaintyAlongAxis;          /*!< Standard Deviation of horizontal uncertainty along the axis of uncertainty */
    double horizontalUncertaintyAlongPerpendicular; /*!< Standard Deviation of horizontal uncertainty perpendicular to the axis of uncertainty */
    double verticalUncertainty;                     /*!< Standard Deviation of vertical uncertainty */
    int16  utcOffset;                               /*!< difference in seconds between GPS time and UTC time (leap seconds) */
    uint16 numberOfSatellites;                      /*!< Number of satellites acquired. */
    double gpsHeading;                              /*!< GPS Heading in degrees. Shall be used in Debug UI perpose only */
    double compassHeading;                          /*!< Compass Heading in degrees. Shall be used in Debug UI perpose only */
    double compassAccuracy;                         /*!< Compass Heading Accuracy. Shall be used in Debug UI perpose only */
} NB_GpsLocation;

typedef struct
{
    double latitude;                                /*!< Latitude in Degrees, WGS-84 ellipsoid */
    double longitude;                               /*!< Longitude in Degrees, WGS-84 ellipsoid */   
    double heading;                                 /*!< Heading in degrees */
    uint32 maneuver;                                /*!< Maneuver ID */
    uint32 segment;                                 /*!< segment ID */
} NB_ExtrapolatePosition;

/*! Position source */
typedef enum
{
    NPS_sourceAny = 0,                /*!< Any position sources, first available first used */
    NPS_sourceHighAccuracy,           /*!< Most accurate source only, with less accuracy */
    NPS_sourceMostAccurateOnly        /*!< Most accurate source only, ex. GPS */
} NB_PositionSource;

/*! Criteria used in obtaining a location provider. */
typedef struct 
{
    nb_boolean				desiredNetworkAssist;   /*!< Non-Zero - Attempt to get a fix using land based stations */
    uint16                	desiredInformation;     /*!< Must set at least one flag */
    uint16					desiredSetting;         /*!< Indicates which settings that will be set or updated */
    uint16					desiredAccuracy;        /*!< Desired position in meters. */
    uint16                  desiredTotalFixes;      /*!< Total number of fixes requested */
    uint16                  desiredPerformance;     /*!< Time in milliseconds to obtain a GPS fix */
    uint16	    			desiredInterval;        /*!< Desired multi-fix interval in milliseconds. Also referred to as Time Between Fix (TBF) */
    uint16  				desiredTimeout;         /*!< Timeout in milliseconds */
    uint16  				desiredMaxAge;          /*!< Return fix that is <= to Max age (ms) */
    NB_PositionSource       desiredPositionSource;  /*!< NEW:position source */
    nb_boolean              desiredCalibrationPopup;/*!< NEW: Indicates that calibration popup shall be shown when value of magnetic heading become unreliable */
    double                  desiredHeadingSpeedLimitation;/*!< NEW: Indicates speed value after riching of which only gps heading shall be used when the speed limit was exceeded */
} NB_GpsCriteria;

#define INVALID_HEADING		-999
#define INVALID_SPEED		-1.0
#define INVALID_ALTITUDE	-999
#define INVALID_LATITUDE    -999.0
#define INVALID_LONGITUDE   -999.0

/*! GPS device state status. */
typedef enum 
{
    NGS_Undefined,
    NGS_InitializePending,
    NGS_DestroyPending,
    NGS_Initialized,
    NGS_Oneshot,
    NGS_Tracking,
    NGS_Suspended,
    NGS_Resume
} NB_GpsState;


/*! Valid flags that are set when a location fix is received. */
typedef enum 
{
    NGV_None = 0,
    NGV_Latitude = (1 << 0),
    NGV_Longitude = (1 << 1),
    NGV_Altitude = (1 << 2),
    NGV_Heading = (1 << 3),
    NGV_HorizontalVelocity = (1 << 4),
    NGV_VerticalVelocity = (1 << 5),
    NGV_HorizontalUncertainty = (1 << 6),
    NGV_AxisUncertainty = (1 << 7),
    NGV_PerpendicularUncertainty = (1 << 8),
    NGV_VerticalUncertainty = (1 << 9),
    NGV_UTCOffset = (1 << 10),
    /* 1 << 11 through 1 << 15 reserved to maintain BREW compatibility */
    NGV_SatelliteCount = (1 << 16)
} NB_GpsValid; 


/*! Flags that indicate information to request. */
typedef enum 
{
    NGI_None = 0,
    NGI_Location = (1 << 0),
    NGI_Velocity = (1 << 1),
    NGI_Altitude = (1 << 2)
} NB_GpsInforequest;



/*!< Flags that indicate what settings need to be set/updated */
typedef enum
{
    NGC_None = 0,
    NGC_Accuracy = (1 << 0),
    NGC_Performance = (1 << 1),
    NGC_Interval = (1 << 2),
    NGC_Timeout = (1 << 3),
    NGC_MaxAge = (1 << 4)
} NB_GpsCriteriaSetting;


/*! GPS mode */
typedef enum
{
    NGM_INVALID,                   /*!< Hide avatar on map */
    NGM_STAND_BY,                  /*!< Show normal avatar without heading */
    NGM_FOLLOW_ME_ANY_HEADING,     /*!< Show avatar with any direction */
    NGM_FOLLOW_ME                  /*!< Show avatar with heading*/
}NB_GpsMode;


/*! @} */

#endif
