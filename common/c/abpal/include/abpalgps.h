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

@file palgps.h
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

/*!
    @addtogroup abpalgps
    @{
*/


#ifndef ABPALGPS_H
#define ABPALGPS_H

#include "abpalexp.h"
#include "pal.h"
#include "palerror.h"
#include "paltypes.h"

typedef struct ABPAL_GpsContext ABPAL_GpsContext;

/*! GPS location information returned to the caller in WGS-84 format.
@see PAL_GPS_VALID
*/
typedef struct
{
    int32 status;                                   /*!< Response status. */
    nb_gpsTime gpsTime;                             /*!< Time, seconds since the GPS Epoch: Midnight, January 6, 1980 */
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
} ABPAL_GpsLocation;

/*! Position source */
typedef enum
{
    PPS_sourceAny = 0,                /*!< Any position sources, first available first used */
    PPS_sourceHighAccuracy,           /*!< Most accurate source only, with less accuracy */
    PPS_sourceMostAccurateOnly        /*!< Most accurate source only, ex. GPS */
} ABPAL_PositionSource;

/*! Criteria used in obtaining a location provider. */
typedef struct
{
    nb_boolean              desiredNetworkAssist;   /*!< Non-Zero - Attempt to get a fix using land based stations */
    uint16                  desiredInformation;     /*!< Must set at least one flag */
    uint16                  desiredSetting;         /*!< Indicates which settings that will be set or updated */
    uint16                  desiredAccuracy;        /*!< Desired position in meters. */
    uint16                  desiredTotalFixes;      /*!< Total number of fixes requested */
    uint16                  desiredPerformance;     /*!< Time in milliseconds to obtain a GPS fix */
    uint32                  desiredInterval;        /*!< Desired multi-fix interval in milliseconds. Also referred to as Time Between Fix (TBF) */
    uint32                  desiredTimeout;         /*!< Timeout in milliseconds */
    uint32                  desiredMaxAge;          /*!< Return fix that is <= to Max age (seconds). Specify 0 to not use cached gps fix and immediately request new fix. Specify MAX_AGE to return fixes regardless of age */
    ABPAL_PositionSource    desiredPositionSource;  /*!< NEW:position source */
    nb_boolean              desiredCalibrationPopup;/*!< NEW: Indicates that calibration popup shall be shown when value of magnetic heading become unreliable */
    double                  desiredHeadingSpeedLimitation;/*!< NEW: Indicates speed value after riching of which only gps heading shall be used when the speed limit was exceeded */
} ABPAL_GpsCriteria;


/// @todo ABPAL_GpsTrackingInfo should be an opaque type
/*! TrackingInfo is returned to the caller upon starting a tracking session.  This info is later used
to suspend, cancel, or stop a tracking session */
typedef struct
{
    uint16 Id;          /*!< A value of 1 - MAXINT that is assigned to a tracking session */
} ABPAL_GpsTrackingInfo;

#define INVALID_HEADING         -999        /*!< A value of invalid heading */
#define INVALID_SPEED           -1.0        /*!< A value of invalid speed */
#define INVALID_ALTITUDE        -999        /*!< A value of invalid altitude */
#define MAX_AGE                 0xFFFFFFFF  /*!< A value of max age */
#define INVALID_HEADING_TIMEOUT 10          /*!< A value of invalid  heading timeout in seconds after excessing of which the heading shall not be used */


/*! GPS device state status. */
typedef enum
{
    PGS_Undefined,
    PGS_InitializePending,
    PGS_DestroyPending,
    PGS_Initialized,
    PGS_Oneshot,
    PGS_Tracking,
    PGS_Suspended,
    PGS_Resume
} ABPAL_GpsState;


/*! Valid flags that are set when a location fix is received. */
typedef enum
{
    PGV_None = 0,
    PGV_Latitude = (1 << 0),
    PGV_Longitude = (1 << 1),
    PGV_Altitude = (1 << 2),
    PGV_Heading = (1 << 3),
    PGV_HorizontalVelocity = (1 << 4),
    PGV_VerticalVelocity = (1 << 5),
    PGV_HorizontalUncertainty = (1 << 6),
    PGV_AxisUncertainty = (1 << 7),
    PGV_PerpendicularUncertainty = (1 << 8),
    PGV_VerticalUncertainty = (1 << 9),
    PGV_UTCOffset = (1 << 10),
    /* 1 << 11 through 1 << 15 reserved to maintain BREW compatibility */
    PGV_SatelliteCount = (1 << 16)
} ABPAL_GpsValid;

/*! Flags that indicate information to request. */
typedef enum
{
    PGI_None = 0,
    PGI_Location = (1 << 0),
    PGI_Velocity = (1 << 1),
    PGI_Altitude = (1 << 2)
} ABPAL_GpsInforequest;



/*!< Flags that indicate what settings need to be set/updated */
typedef enum
{
    PGC_None = 0,
    PGC_Accuracy = (1 << 0),
    PGC_Performance = (1 << 1),
    PGC_Interval = (1 << 2),
    PGC_Timeout = (1 << 3),
    PGC_MaxAge = (1 << 4)
} ABPAL_GpsCriteriaSetting;




/*! GPS configuration comprised of name/value pairs and passed as array of structures to ABPAL_GpsInitialize function.

    Available values are (values and names are NOT case sensitive):

    <table border="1">
    <tr><th>Configuration</th><th>Values</th></tr>
    <tr><td>ProviderType</td><td>Select one value: EMU, LPS, GPSID, AGPSID, FILE, RS232, HTTP</td></tr>
    <tr><td>GpsFilename</td><td>Path to the file that contains gps locations.</td></tr>
    <tr><td>GpsHttpPort</td><td>Integer value. HTTP port that will supply NMEA GPS strings</td></tr>
    <tr><td>ComPort</td><td>Provide one valid serial port: COM1, COM2, COMx</td></tr>
    <tr><td>ClientID</td><td>Carrier specific</td></tr>
    <tr><td>ClientName</td><td>Application Name e.g. AtlasBook Global</td></tr>
    <tr><td>SMSPrefix</td><td>Carrier specific</td></tr>
    <tr><td>PDEIPAddress</td><td>Carrier speficic e.g. 192.168.1.1</td></tr>
    <tr><td>PDEPort</td><td>Carrier speficic e.g. 8899</td></tr>
    <tr><td>AssistedRetries</td><td>Integer Value.  Number of times to retry network assisted fix</td></tr>
    <tr><td>WarmUpFix</td><td>Yes or No, If yes, GPS implementation will request a fix during initialization</td></tr>
    <tr><td>Roaming</td><td>Enable</td></tr>
    <tr><td>EmuPlayStart</td><td>Integer value. Number of seconds to skip the playback file </td></tr>
    </table>

@see ABPAL_GpsInitialize
*/
typedef struct
{
    char*   name;   /*!< Configuration name */
    char*   value;  /*!< Value */
} ABPAL_GpsConfig;



/*! GPS Device state callback function prototype

Caller supplied callback function that will be called when the
GPS hardware's state changes.  State information varies
between OEMs and also varies between OEM's mobile device product
lines. In addition, OEMs also does not consistently
report state changes.  Use data received from
this callback for informational use only.

@return None

@see ABPAL_GpsInitialize
*/
typedef void ABPAL_GpsDeviceStateCallback(
    const void* userData,       /*!< User supplied data */
    const char* stateChangeXml, /*!< Device state information in XML format */
    PAL_Error error,            /*!< Result of the initialization */
    const char* errorInfoXml    /*!< XML string that contains detailed error information */
    );



/*! GPS initialization callback function prototype

Caller supplied callback function that will be called
when the GPS has been initialized.

@return None

@see ABPAL_GpsInitialize
*/
typedef void ABPAL_GpsInitializeCallback(
    const void* userData,       /*!< User supplied data */
    PAL_Error error,            /*!< Result of the initialization */
    const char* errorInfoXml    /*!< XML string that contains detailed error information */
    );




/*! GPS location callback function prototype

Caller supplied callback function that will be called
when there is a GPS location available to the caller

@return None

@see ABPAL_GpsGetLocation
@see ABPAL_GpsBeginTracking
*/
typedef void ABPAL_GpsLocationCallback(
    const void* userData,             /*!< User supplied data */
    PAL_Error error,                  /*!< Result of the location event */
    const ABPAL_GpsLocation* location,/*!< Location information, lat/long, etc. */
    const char* errorInfoXml          /*!< XML string that contains detailed error information */
    );



/*! Creates the GPS provider instance.

Call this function first before any other ABPAL_GpsXXX API functions to create the GPS provider.
The caller is responsible for saving the pointer pointed to by the OUT parameter gpsContext.
GPS context is used on all subsequent calls to PAL GPS API.

ABPAL_GpsInitialize must be called next to initialize the GPS provider.

@return PAL_Ok if the GPS provider is created successfully

@see ABPAL_GpsInitialize
*/
ABPAL_DEC
PAL_Error
ABPAL_GpsCreate(
    PAL_Instance* pal,            /*!< PAL instance */
    ABPAL_GpsContext** gpsContext /*!< [OUT parameter] that will return a pointer to gpsContext.
                                       The caller must pass this gpsContext to every ABPAL_GpsXXXX API */
    );

/// @todo Collapse multiple callbacks into one struct
/// @todo This needs to take a criteria to influence the provider
/*! Initialize GPS provider.

This function is called to initialized the GPS location provider.  This function
will return PAL_Ok for any subsequent calls to this function after it has been
successfully initialized.

Call ABPAL_GpsDestroy when you no longer require the GPS provider.

@return PAL_Error

@see ABPAL_GpsDestroy
@see ABPAL_GpsConfig
*/
ABPAL_DEC
PAL_Error
ABPAL_GpsInitialize(
    ABPAL_GpsContext* gpsContext,                       /*!< Pointer to gpsContext */
    const ABPAL_GpsConfig* config,                      /*!< Array of value pairs describing the desired GPS configuration */
    uint32 configCount,                                 /*!< Number of elements contained in config (PalGpsConfig) */
    ABPAL_GpsInitializeCallback* userCallback,          /*!< Callback function that is called when the provider is initialized */
    ABPAL_GpsDeviceStateCallback* userCallbackDevState, /*!< [OPTIONAL] Callback function that is called when the GPS hardware state's changes.
                                                             Pass NULL if you do not wish to receive device state notification.*/
    const void* userData                                /*!< User supplied data that is passed to both callbacks above */
    );



/*! Release GPS provider.

This function closes the GPS provider and releases associated resources.

@return PAL_Error

@see ABPAL_GpsInitialize
*/
ABPAL_DEC
PAL_Error
ABPAL_GpsDestroy(
    ABPAL_GpsContext* gpsContext  /*!< Pointer to gpsContext */
    );



/// @todo Is this actually useful?
/*! Get the current state of the GPS provider.

This function gets the current state of the GPS provider.

@return PAL_Error

@see ABPAL_GpsState
*/
ABPAL_DEC
PAL_Error
ABPAL_GpsGetState(
    ABPAL_GpsContext* gpsContext, /*!< Pointer to gpsContext */
    ABPAL_GpsState* gpsState      /*!< Out parameter that will contain the GPS state */
    );


/// @todo GetLocation needs to return an id that can be used to cancel
/*! Gets a single location fix.

This function will return one location fix (lat/long) according to
the criteria set.

@return PAL_Error

@see ABPAL_GpsCancelGetLocation
@see ABPAL_GpsBeginTracking
*/
ABPAL_DEC
PAL_Error
ABPAL_GpsGetLocation(
    ABPAL_GpsContext* gpsContext,            /*!< Pointer to gpsContext */
    const ABPAL_GpsCriteria* criteria,       /*!< GPS Criteria */
    ABPAL_GpsLocationCallback* userCallback, /*!< Pointer to callback function that will be called with location */
    const void* userData                     /*!< User supplied data */
    );


/// @todo Cancel needs to be able to support multiple single fix requests, either with an id or cb/cbData combo
/*! Cancels the outstanding location request initiated by ABPAL_GpsGetLocation

This function cancels the current ABPAL_GpsGetLocation operation.

@return PAL_Error

@see ABPAL_GpsGetLocation
*/
ABPAL_DEC
PAL_Error
ABPAL_GpsCancelGetLocation(
    ABPAL_GpsContext* gpsContext    /*!< Pointer to gpsContext */
    );



/// @todo Is observeOnly useful?  Engine is started in other code but maybe thats an implementation detail
/// @todo GpsTrackingInfo should be opaque, so trackingInfo needs to be a **
/// @todo Tracking shouldn't take GpsCritera
/*! Begins a tracking session

This function starts a multi-fix tracking session based
on the provided criteria.  The tracking session will run
until ABPAL_GpsEndTracking is called.  Depending on the platform,
this function will start multiple tracking sessions.

@return PAL_Error

@see ABPAL_GpsEndTracking, ABPAL_GpsSuspend, ABPAL_GpsResume
*/
ABPAL_DEC
PAL_Error
ABPAL_GpsBeginTracking(
    ABPAL_GpsContext* gpsContext,            /*!< Pointer to gpsContext */
    uint16 observeOnly,                      /*!< non-zero - does not start tracking, listen for fixes only */
    const ABPAL_GpsCriteria* criteria,       /*!< Tracking criteria */
    ABPAL_GpsLocationCallback* userCallback, /*!< Pointer to callback function that will be called with each location fix */
    const void* userData,                    /*!< User supplied data */
    ABPAL_GpsTrackingInfo* trackingInfo      /*!< A unique tracking session Id will be set in this out parameter */
    );



/*! Ends a previous tracking session.

This function ends a previous tracking session that
was started with ABPAL_GpsBeginTracking.

@return PAL_Error

@see ABPAL_GpsBeginTracking
*/
ABPAL_DEC
PAL_Error
ABPAL_GpsEndTracking(
    ABPAL_GpsContext* gpsContext,                /*!< Pointer to gpsContext */
    const ABPAL_GpsTrackingInfo* trackingInfo    /*!< The tracking session to end identified by TrackingInfo that was returned from ABPAL_GpsBeginTracking call */
    );


/// @todo Update what criteria?  The trackers are the only thing that currently have criteria, not the providers themselves
/*! Updates GPS Criteria.

This function is used to update the GPS provider operational
criteria. It can be called after ABPAL_GpsInitialize has been called and
before a tracking session has been established or during tracking.
An example use of this function is to establish a tracking session with
an interval that uses minimal power to keep the GPS warm and then call this
function to change the interval frequency for navigation.

@return PAL_Ok if the criteria was updated.

@see ABPAL_GpsBeginTracking
*/
ABPAL_DEC
PAL_Error
ABPAL_GpsUpdateCriteria(
    ABPAL_GpsContext* gpsContext,             /*!< Pointer to gpsContext */
    const ABPAL_GpsCriteria* criteria,        /*!< Tracking criteria, platform dependent */
    const ABPAL_GpsTrackingInfo* trackingInfo /*!< The tracking session to update identified by TrackingInfo that was returned from ABPAL_GpsBeginTracking call */
    );



/*! Suspends the current tracking session.

This function suspends the tracking session previously started
with ABPAL_GpsBeginTracking.  It does not terminate the current
tracking session, it merely stops forwarding fixes to the callback
function that was passed in to ABPAL_GpsBeginTracking.

@return PAL_Error

@see ABPAL_GpsResume
*/
ABPAL_DEC
PAL_Error
ABPAL_GpsSuspend(
    ABPAL_GpsContext* gpsContext,                /*!< Pointer to gpsContext */
    const ABPAL_GpsTrackingInfo* trackingInfo    /*!< The tracking session to suspend identified by TrackingInfo that was returned from ABPAL_GpsBeginTracking call */
    );




/*! Resumes the previously suspended tracking session.

This function resumes the previously suspended tracking session
by ABPAL_GpsSuspend.  A call to this function is ignored if the tracking
session was not previously suspended.

@return PAL_Error

@see ABPAL_GpsSuspend
*/
ABPAL_DEC
PAL_Error
ABPAL_GpsResume(
    ABPAL_GpsContext* gpsContext,                /*!< Pointer to gpsContext */
    const ABPAL_GpsTrackingInfo* trackingInfo    /*!< The tracking session to resume identified by TrackingInfo that was returned from ABPAL_GpsBeginTracking call */
    );

/*! Check if GPS is enabled on the phone (for other requests than E911).

@return nb_boolean
*/
ABPAL_DEC
nb_boolean
ABPAL_IsGpsEnabled(
    ABPAL_GpsContext* gpsContext  /*!< Pointer to gpsContext */
    );

/*! Set whether or not geolocation events will be sent while device is in standby.

@return PAL_Error
*/
ABPAL_DEC
PAL_Error
ABPAL_EnableBackgroundGeolocation(
    ABPAL_GpsContext* gpsContext,  /*!< Pointer to gpsContext */
    nb_boolean isEnabled           /*!< If true geolocation events are enabled in standby */
    );

#endif

/*! @} */
