//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
// ********************************************************
// GPS Intermediate Driver API
// ********************************************************





#include <Winsock2.h>

/*********************************************************************************************************
** READ ME. READ ME. READ ME. READ ME. READ ME. READ ME. READ ME.
** 
** 02/26/2009
**
** All typedefs, functions, struct names has been modified so that we can provide GPS functionality
** with one DLL.
**
** Any "GPS" names has been changed to "AGPS"
**
**
**********************************************************************************************************/






#ifndef _AGPSAPI_H_
#define _AGPSAPI_H_

#define AGPS_MAX_SATELLITES      12
#define AGPS_MAX_PREFIX_NAME     16
#define AGPS_MAX_FRIENDLY_NAME   64

#define AGPS_VERSION_1           1
#define AGPS_VERSION_2           2
#define AGPS_VERSION_CURRENT     AGPS_VERSION_2

#define AGPS_MAX_SERVER_URL_NAME			256

#define AGPS_CONFIG_CUSTOM				256
#define AGPS_CONFIG_CUSTOM_LOCAL_PDE     (AGPS_CONFIG_CUSTOM + 889)

#define AGPS_CONFIG_PDE_SERVER			256 //To set PDE server IP/Port.
#define AGPS_CONFIG_GPSLOCK_NVITEM		257 //To set GPS Lock state.

#define AGPS_CONFIG_CUSTOM_WRITE_GPS_LOCK          (AGPS_CONFIG_CUSTOM + 890) //For GPSSetDeviceParam
#define AGPS_CONFIG_CUSTOM_READ_GPS_LOCK           (AGPS_CONFIG_CUSTOM + 891)   //For GPSGetDeviceParam

#define AGPS_VALID_SERVER_IPADDRESS		0x00000001

//
// Poll Version 1 drivers can handle some fields of GPS_POSITION structure
// Poll Version 2 drivers can handle all fields of GPS_POSITION structure
//
#define POLL_VERSION_1          1
#define POLL_VERSION_2          2

typedef enum {
    AGPS_FIX_UNKNOWN = 0,
    AGPS_FIX_2D,
    AGPS_FIX_3D
}
AGPS_FIX_TYPE;

typedef enum {
    AGPS_FIX_SELECTION_UNKNOWN = 0,
    AGPS_FIX_SELECTION_AUTO,
    AGPS_FIX_SELECTION_MANUAL
}
AGPS_FIX_SELECTION;

typedef enum {
    AGPS_FIX_QUALITY_UNKNOWN = 0,
    AGPS_FIX_QUALITY_GPS,
    AGPS_FIX_QUALITY_DGPS
}
AGPS_FIX_QUALITY;


//
// The following define the fix modes known to the operating system.
// The OEMs are free to define their own GPS_FIX_MODE starting with values
// greater than or equal to GPS_FIX_MODE_CUSTOM.
//
typedef enum {
    AGPS_FIX_MODE_UNKNOWN = 0,
    AGPS_FIX_MODE_MSA,
    AGPS_FIX_MODE_MSB,        
    AGPS_FIX_MODE_MSS,    
    
    // This must be the last entry for OS-supplied GPS_FIX_MODE_XXX values.
    // It's not an actual fix mode value - It's only a place holder
    // to keep count of values in the enum.
    AGPS_FIX_MODE_COUNT
} 
AGPS_FIX_MODE;

//
// This is the value which defines the start of the range where OEMs are 
// permitted to use their own fix mode values.
//
#define AGPS_FIX_MODE_CUSTOM 256


//
// This structure contains the parameters to start the GPS device and trigger 
// a GPS fix operation. Pointer to this struct is used as pbParamData field in 
// calls to GPSSetDeviceParam function when dwParamName = GPS_START_FIX
//
typedef struct _AGPS_START_FIX_CONFIG {
    DWORD dwFlags;
} AGPS_START_FIX_CONFIG, *PAGPS_START_FIX_CONFIG;



#define AGPS_START_FIX          0
#define AGPS_STOP_FIX           1
#define AGPS_CONFIG_FIX_MODE    2
#define AGPS_CONFIG_FIX_RATE    3
#define AGPS_CONFIG_QOS         4
#define AGPS_QUERY_FIX          5
#define AGPS_CONFIG_COUNT       6 // Count of built-in GPS_PARAM values
#define AGPS_CONFIG_CUSTOM    256 // This is the value which defines the start of 
                                 // the range where OEMs are permitted to use 
                                 // their own parameter values for use in 
                                 // GPSGetDeviceParam and GPSSetDeviceParam.

//
// This enumeration contains values that specify the current GPS Hardware state.
//
typedef enum {
    AGPS_HW_STATE_UNKNOWN = 0,
    AGPS_HW_STATE_ON,
    AGPS_HW_STATE_IDLE
} AGPS_HW_STATE;


//
// GPS_VALID_XXX bit flags in GPS_POSITION structure are valid.
//
#define AGPS_VALID_UTC_TIME                                 0x00000001
#define AGPS_VALID_LATITUDE                                 0x00000002
#define AGPS_VALID_LONGITUDE                                0x00000004
#define AGPS_VALID_SPEED                                    0x00000008
#define AGPS_VALID_HEADING                                  0x00000010
#define AGPS_VALID_MAGNETIC_VARIATION                       0x00000020
#define AGPS_VALID_ALTITUDE_WRT_SEA_LEVEL                   0x00000040
#define AGPS_VALID_ALTITUDE_WRT_ELLIPSOID                   0x00000080
#define AGPS_VALID_POSITION_DILUTION_OF_PRECISION           0x00000100
#define AGPS_VALID_HORIZONTAL_DILUTION_OF_PRECISION         0x00000200
#define AGPS_VALID_VERTICAL_DILUTION_OF_PRECISION           0x00000400
#define AGPS_VALID_SATELLITE_COUNT                          0x00000800
#define AGPS_VALID_SATELLITES_USED_PRNS                     0x00001000
#define AGPS_VALID_SATELLITES_IN_VIEW                       0x00002000
#define AGPS_VALID_SATELLITES_IN_VIEW_PRNS                  0x00004000
#define AGPS_VALID_SATELLITES_IN_VIEW_ELEVATION             0x00008000
#define AGPS_VALID_SATELLITES_IN_VIEW_AZIMUTH               0x00010000
#define AGPS_VALID_SATELLITES_IN_VIEW_SIGNAL_TO_NOISE_RATIO 0x00020000
#define AGPS_VALID_POSITION_UNCERTAINTY_ERROR               0x00040000
#define AGPS_VALID_FIX_MODE                                 0x00080000
#define AGPS_VALID_FIX_ERROR                                0x00100000


//
// GPS_DEV_VALID_XXX bit flags in GPS_DEVICE_STATUS structure are valid. 
//
#define AGPS_DEV_VALID_HW_STATE                             0x00000001 
#define AGPS_DEV_VALID_EPH_SV_MASK                          0x00000002
#define AGPS_DEV_VALID_ALM_SV_MASK                          0x00000004 
#define AGPS_DEV_VALID_SAT_IN_VIEW_PRN                      0x00000008
#define AGPS_DEV_VALID_SAT_IN_VIEW_CARRIER_TO_NOISE_RATIO   0x00000010
#define AGPS_DEV_VALID_DEV_ERROR                            0x00000020


//
// GPS_DATA_FLAGS_XXX bit flags set in GPS_POSITION dwFlags field
// provide additional information about the state of the query.
// 

// Set when GPS hardware is not connected to GPSID and we 
// are returning cached data.
#define AGPS_DATA_FLAGS_HARDWARE_OFF                        0x00000001

//
// LBS Driver error codes
//
#define AGPS_ERR_SUCCESS                               ERROR_SUCCESS

#define AGPS_ERR_ACCESS_DENIED                         ERROR_ACCESS_DENIED
#define AGPS_ERR_INVALID_HANDLE                        ERROR_INVALID_HANDLE
#define AGPS_ERR_INVALID_PARAMETER                     ERROR_INVALID_PARAMETER
#define AGPS_ERR_DEVICE_BUSY                           ERROR_BUSY
#define AGPS_ERR_INVALID_STATE                         ERROR_INVALID_STATE
#define AGPS_ERR_NOT_AUTHENTICATED                     ERROR_NOT_AUTHENTICATED
#define AGPS_ERR_NETWORK_UNREACHABLE                   ERROR_NETWORK_UNREACHABLE
#define AGPS_ERR_SERVER_UNREACHABLE                    ERROR_HOST_UNREACHABLE
#define AGPS_ERR_GETPOS_TIMEOUT                        ERROR_TIMEOUT
#define AGPS_ERR_ERROR_NOT_ENOUGH_QUOTA                ERROR_NOT_ENOUGH_QUOTA

// GPSOne specific error codes
#define AGPS_ERR_INSUFFICIENT_SAT_SIGNAL               0x20000000	// Poor signal strength from satellites
#define AGPS_ERR_FATAL_ERROR                           0x20000100	// Fatal error, close the application
#define AGPS_ERR_OPEN_FAIL_ALREADY_OPEN                0x20000101	// Handle already open
#define AGPS_ERR_OPEN_FAIL_MAX_APP_LIMIT_REACHED       0x20000102	// Max application running
#define AGPS_ERR_POSITION_INFO_NOT_AVAILABLE           0x20000103
#define AGPS_ERR_LOCATION_REQ_QUEUE_FULL               0x20000105
#define AGPS_ERR_PREV_LOCATION_REQ_PENDING             0x20000106
#define AGPS_ERR_RADIO_OFF                             0x20000109
#define AGPS_ERR_GPSONE                                0x2000010a

#define AGPS_ERR_NO_SYSTEM_RESOURCES                   0xe0011001
#define AGPS_ERR_TIMEOUT                               0xe0011002
#define AGPS_ERR_FAILED_TO_GET_MDN                     0xe0011003
#define AGPS_ERR_NOT_SUPPORTED                         0xe0011004
#define AGPS_ERR_INTERNAL                              0xe0011101
#define AGPS_ERR_DEVICE_NOT_OPEN                       0xe0012004	// Handle not open
#define AGPS_ERR_PRIVACY_REFUSED                       0xe0012007
#define AGPS_ERR_TRACKING_SESSION_EXPIRED              0xe0012008
#define AGPS_ERR_OPEN_INVALID_NEWLOCATIONDATA_HANDLE   0xe001200a	// hNewLocationData is NULL
#define AGPS_ERR_OPEN_INVALID_DEVICESTATECHANGE_HANDLE 0xe001200b	// hDeviceStateChange is NULL
#define AGPS_ERR_OPEN_INVALID_DEVICE_NAME              0xe001200c	// szDeviceName is not NULL
#define AGPS_ERR_OPEN_INVALID_FLAGS                    0xe001200d	// dwFlags is not 0
#define AGPS_ERR_SET_NOT_INITIALIZED                   0xe001200e	// GPSSetDeviceParam - GPS_CONFIG_INIT not done

#define AGPS_ERR_INVALID_PARAMETER_NAME                0xe0012020
#define AGPS_ERR_GETPOS_DATA_NULL                      0xe0012021
#define AGPS_ERR_BAD_GETPOS_VERSION_PARAM              0xe0012022
#define AGPS_ERR_GETPOS_PRIVACY_REFUSED                0xe0012024
#define AGPS_ERR_SERVICE_NOT_AUTHORIZED                0xe0012025
#define AGPS_ERR_AUTHENTICATION_FAIL                   0xe0012028
#define AGPS_ERR_INFORMATION_NOT_AVAILABLE             0xe0012029	// GPSGetDeviceState
#define AGPS_ERR_GETPOS_FLAGS_INVALID                  0xe001202a	// dwFlags is not 0
#define AGPS_ERR_GETPOS_MAXAGE_INVALID                 0xe001202b	// dwMaximumAge is 0
#define AGPS_ERR_GETPOS_LENGTH_INVALID                 0xe001202c	// dwSize doesn't correspond to dwVersion

#define AGPS_ERR_FIX_MODE_DATA_NULL                    0xe0012031	// dwParamData is NULL
#define AGPS_ERR_FIX_MODE_DATA_INVALID                 0xe0012032	// dwParamData contains invalid mode
#define AGPS_ERR_FIX_MODE_LENGTH_INVALID               0xe0012033	// dwDataLen is not sizeof(GPS_MODE)
#define AGPS_ERR_FIX_MODE_NOT_SUPPORTED                0xe0012034	// mode not supported on this device

#define AGPS_ERR_FIX_RATE_DATA_NULL                    0xe0012041	// dwParamData is NULL
#define AGPS_ERR_FIX_RATE_NUM_FIXES_INVALID            0xe0012042	// must be >= 0
#define AGPS_ERR_FIX_RATE_INTERVAL_SINGLE_INVALID      0xe0012043	// single shot interval must be 0
#define AGPS_ERR_FIX_RATE_INTERVAL_TRACKING_INVALID    0xe0012044	// tracking session interval must be > 0
#define AGPS_ERR_FIX_RATE_INTERVAL_STOP_INVALID        0xe0012045	// stop interval must be 0
#define AGPS_ERR_FIX_RATE_LENGTH_INVALID               0xe0012046	// dwDataLen is not sizeof(GPS_FIX_RATE)
#define AGPS_ERR_FIX_RATE_TRACKING_DURATION_INVALID    0xe0012047	// tracking session duration > GPS_MAX_TRACKING_DURATION (Deprecated)

#define AGPS_ERR_QOS_DATA_NULL                         0xe0012051	// dwParamData is NULL
#define AGPS_ERR_QOS_LENGTH_INVALID                    0xe0012054	// dwDataLen is not sizeof(GPS_QOS)

#define AGPS_ERR_START_DATA_NULL                       0xe0012061	// dwParamData is NULL
#define AGPS_ERR_START_FLAGS_INVALID                   0xe0012062	// must be 0
#define AGPS_ERR_START_LENGTH_INVALID                  0xe0012063	// dwDataLen is not sizeof(GPS_START_FIX_CONFIG)

#define AGPS_ERR_STOP_DATA_NOT_NULL                    0xe0012071	// dwParamData is not NULL
#define AGPS_ERR_STOP_LENGTH_INVALID                   0xe0012072	// dwDataLen is not 0

#define AGPS_ERR_GETPARAM_DATA_NULL                    0xe0012081

#define AGPS_ERR_BAD_GETDEVICE_VERSION_PARAM           0xe0012091
#define AGPS_ERR_GETDEVICE_DATA_NULL                   0xe0012092
#define AGPS_ERR_GETDEVICE_LENGTH_INVALID              0xe0012093

#define AGPS_ERR_MODULE_NOT_TRUSTED                    0xe00120a1	// untrusted signature for app .exe or .dll 

#define AGPS_ERR_ESTABLISH_CONNECTION_FAILED           0xe00120a2  // Could not establish a packet-data (CDMA or GSM) connection
#define AGPS_ERR_SERVICE_UNAVAILABLE                   0xe00120a3  // PDE is down or location service otherwise currently unavailable

#define AGPS_ERR_NOT_PROVISIONED                       0xe00120b0  // Privacy not provisioned
#define AGPS_ERR_ROAMING_NOT_SUPPORTED                 0xe00120b1  // Unsupported roaming network
#define AGPS_ERR_UPDATE_REQUIRED                       0xe00120c0  // LBSDriver update is required

//
// This structure contains estimated position error information returned in GPS_POSITION
// structure with dwVersion = GPS_VERSION_2. 
//
typedef struct _AGPS_POSITION_ERROR
{
    //
    // Horizontal position uncertainty in meters of axis aligned with the angle 
    // specified in dWHorizontalErrorAngle of a two-dimension horizontal error ellipse. 
    // The value dwHorizontalConfidence gives the percentage of positions expected to 
    // fall within this ellipse, e.g. dwHorizontalConfidence = 39 indicates a 1-sigma 
    // error ellipse is given.
    //
    DWORD dwHorizontalErrorAlong;

    //
    // Angle of horizontal axis, with respect to true north, for a two-dimension 
    // horizontal error ellipse.
    //
    DWORD dwHorizontalErrorAngle;

    //
    // Horizontal position uncertainty in meters of axis perpendicular to angle 
    // specified in dwHorizontalErrorAngle of a two-dimension horizontal error ellipse. 
    // The value dwHorizontalConfidence gives the percentage of positions expected to 
    // fall within this ellipse, e.g. dwHorizontalConfidence = 39 indicates a 1-sigma 
    // error ellipse is given.
    //
    DWORD dwHorizontalErrorPerp;

    //
    // Vertical position uncertainty in meters with 1 sigma uncertainty. 
    // This value is always given as the 1-sigma estimate of vertical position error. 
    // It is not adjusted based on the value of  dwHorizontalConfidence.
    //
    DWORD dwVerticalError;

    //
    // Identifies the calculated probability in percent that the position estimate 
    // resides within the two dimension horizontal error ellipse specified by the 
    // three horizontal error values above. Note that appropriate rescaling of the 
    // ellipse dimensions can be used to achieve other confidence values. The special 
    // case where dwHorizontalErrorAlong are dwHorizontalErrorPerp set to the estimated 
    // 1 dimension standard deviation values will yield a confidence value of 39%. 
    // (2.45 sigma yield 95% confidence, 3.03 x sigma yields 99% confidence).
    //
    DWORD dwHorizontalConfidence;
} AGPS_POSITION_ERROR, *PAGPS_POSITION_ERROR;



//
// GPS_POSITION contains our latest physical coordinates, the time, 
// and satellites used in determining these coordinates. 
// dwVersion = GPS_VERSION_1 uses the fields upto (but NOT including) PositionUncertaintyError field.
// dwVersion = GPS_VERSION_2 uses all the defined fields upto (and including) dwFixError field.
// 
typedef struct _AGPS_POSITION {
    DWORD dwVersion;             // Current version of GPSID client is using.
    DWORD dwSize;                // sizeof(_GPS_POSITION)

    // Not all fields in the structure below are guaranteed to be valid.  
    // Which fields are valid depend on GPS device being used, how stale the API allows
    // the data to be, and current signal.
    // Valid fields are specified in dwValidFields, based on GPS_VALID_XXX flags.
    DWORD dwValidFields;

    // Additional information about this location structure (GPS_DATA_FLAGS_XXX)
    DWORD dwFlags;
    
    //** Time related
    SYSTEMTIME stUTCTime;   //  UTC according to GPS clock.
    
    //** Position + heading related
    double dblLatitude;            // Degrees latitude.  North is positive
    double dblLongitude;           // Degrees longitude.  East is positive
    float  flSpeed;                // Speed in knots
    float  flHeading;              // Degrees heading (course made good).  True North=0
    double dblMagneticVariation;   // Magnetic variation.  East is positive
    float  flAltitudeWRTSeaLevel;  // Altitute with regards to sea level, in meters
    float  flAltitudeWRTEllipsoid; // Altitude with regards to ellipsoid, in meters

    //** Quality of this fix
    AGPS_FIX_QUALITY     FixQuality;        // Where did we get fix from?
    AGPS_FIX_TYPE        FixType;           // Is this 2d or 3d fix?
    AGPS_FIX_SELECTION   SelectionType;     // Auto or manual selection between 2d or 3d mode
    float flPositionDilutionOfPrecision;   // Position Dilution Of Precision
    float flHorizontalDilutionOfPrecision; // Horizontal Dilution Of Precision
    float flVerticalDilutionOfPrecision;   // Vertical Dilution Of Precision

    //** Satellite information
    DWORD dwSatelliteCount;                                            // Number of satellites used in solution
    DWORD rgdwSatellitesUsedPRNs[AGPS_MAX_SATELLITES];                  // PRN numbers of satellites used in the solution

    DWORD dwSatellitesInView;                                          // Number of satellites in view.  From 0-GPS_MAX_SATELLITES
    DWORD rgdwSatellitesInViewPRNs[AGPS_MAX_SATELLITES];                // PRN numbers of satellites in view
    DWORD rgdwSatellitesInViewElevation[AGPS_MAX_SATELLITES];           // Elevation of each satellite in view
    DWORD rgdwSatellitesInViewAzimuth[AGPS_MAX_SATELLITES];             // Azimuth of each satellite in view
    DWORD rgdwSatellitesInViewSignalToNoiseRatio[AGPS_MAX_SATELLITES];  // Signal to noise ratio of each satellite in view

    //** GPS API Extended fields (API v2)
    AGPS_POSITION_ERROR PositionUncertaintyError;                       // Estimated position uncertainty error 
    AGPS_FIX_MODE       FixMode;                                        // Fix mode used by the GPS hardware to calculate the position
    DWORD              dwFixError;                                     // GPS fix session error reported by GPS hardware
} AGPS_POSITION, *PAGPS_POSITION;


//
// This structure contains the status information about GPS Hardware used by GPSID. 
//
typedef struct _AGPS_DEVICE_STATUS {
    DWORD        dwValidFields;
    AGPS_HW_STATE ghsHardwareState;
    DWORD        dwEphSVMask;
    DWORD        dwAlmSVMask;
    DWORD        rgdwSatellitesInViewPRNs[AGPS_MAX_SATELLITES];
    DWORD        rgdwSatellitesInViewCarrierToNoiseRatio[AGPS_MAX_SATELLITES];
    DWORD        dwDeviceError;
} AGPS_DEVICE_STATUS, *PAGPS_DEVICE_STATUS;


//
// GPS_DEVICE contains information about the device driver and the
// service itself and is returned on a call to GPSGetDeviceState().
// States are indicated with SERVICE_STATE_XXX flags defined in service.h
// dwVersion = GPS_VERSION_1 uses the first 8 fields, upto gdsDeviceStatus. 
// dwVersion = GPS_VERSION_2 uses the first 9 fields, including gdsDeviceStatus.
// 
typedef struct _AGPS_DEVICE {
    DWORD             dwVersion;                                 // Current version of GPSID client is using.
    DWORD             dwSize;                                    // sizeof this structure
    DWORD             dwServiceState;                            // State of the GPS Intermediate Driver service.  
    DWORD             dwDeviceState;                             // Status of the actual GPS device driver.
    FILETIME          ftLastDataReceived;                        // Last time that the actual GPS device sent information to the intermediate driver.
    WCHAR             szGPSDriverPrefix[AGPS_MAX_PREFIX_NAME];    // Prefix name we are using to communicate to the base GPS driver
    WCHAR             szGPSMultiplexPrefix[AGPS_MAX_PREFIX_NAME]; // Prefix name that GPS Intermediate Driver Multiplexer is running on
    WCHAR             szGPSFriendlyName[AGPS_MAX_FRIENDLY_NAME];  // Friendly name real GPS device we are currently using
    AGPS_DEVICE_STATUS gdsDeviceStatus;                           // Information about GPS Hardware
} *PAGPS_DEVICE, AGPS_DEVICE;

typedef struct _AGPS_FIX_RATE {
    DWORD dwNumFixes;
    DWORD dwTimeBetweenFixes;
} AGPS_FIX_RATE, *PAGPS_FIX_RATE;


typedef struct _AGPS_QOS {
    DWORD dwAccuracy;
    DWORD dwPerformance;
} AGPS_QOS, *PAGPS_QOS;

//Server config struct for specifying PDE for use with standard AKU AGPS 
typedef struct _AGPS_SERVER_CONFIG {
	HANDLE				hGPSDevice;     // Handle returned by a call to GPSOpenDevice. Server configuration is applicable to this handle
	DWORD				dwValidFields;  // Indicates which field is valid in this structure either pGPSServerAddr or PGPSServerUrl.
	SOCKADDR_STORAGE	GPSServerAddr;  // Pointer to GPS server IP address and TCP port that will be used to exchange aiding data
	WCHAR				szGPSServerUrl[AGPS_MAX_SERVER_URL_NAME];	// Pointer to GPS server URL for Domain Name Resolution.
	DWORD				dwFlags;        // Reserved, must be 0
} AGPS_SERVER_CONFIG, *PAGPS_SERVER_CONFIG;

typedef struct _SGPS_SERVER_CONFIG {
	DWORD gpsOneLock;     
	DWORD PDE_IP_Address;   
	DWORD PDE_IP_Port;     
} SGPS_SERVER_CONFIG, *PSGPS_SERVER_CONFIG;

#ifdef __cplusplus
extern "C" {
#endif

HANDLE AGPSOpenDevice(HANDLE hNewLocationData, HANDLE hDeviceStateChange, const WCHAR *szDeviceName, DWORD dwFlags);
DWORD  AGPSCloseDevice(HANDLE hGPSDevice);
DWORD  AGPSGetPosition(HANDLE hGPSDevice, AGPS_POSITION *pGPSPosition, DWORD dwMaximumAge, DWORD dwFlags);
DWORD  AGPSGetDeviceState(AGPS_DEVICE *pGPSDevice);
DWORD  AGPSGetDeviceParam(HANDLE hGPSDevice, DWORD dwParamName, PBYTE pbParamData, PDWORD pcbDataLen);
DWORD  AGPSSetDeviceParam(HANDLE hGPSDevice, DWORD dwParamName, PBYTE pbParamData, DWORD cbDataLen);

#ifdef __cplusplus
}
#endif

#endif // _AGPSAPI_H_


