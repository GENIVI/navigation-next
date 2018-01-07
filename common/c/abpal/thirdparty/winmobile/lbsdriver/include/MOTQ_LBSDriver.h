//
// Copyright (c) Autodesk, Inc.  All rights reserved.
//
//
#pragma once
// The following ifdef block is the standard way of creating macros which
// make exporting from a DLL simpler. All files within this DLL are
// compiled with the LBSDRIVER_EXPORTS symbol defined on the command line.
// this symbol should not be defined on any project that uses this DLL.
// This way any other project whose source files include this file see 
// LBSDRIVER_API functions as being imported from a DLL, wheras this
// DLL sees symbols defined with this macro as being exported.

#ifdef _WIN32
    #ifdef LBSDRIVER_EXPORTS
        #define LBSDRIVER_API __declspec(dllexport)
    #else
        #define LBSDRIVER_API 
    #endif
#else
    #define LBSDRIVER_API 
#endif

#include <winsock2.h>
#include <service.h>

///////////////////////////////////////////////////////////////////////////////
//
//
// typedef Name:  _GPS_INIT
//
// Purpose:       This structure contains the parameters required to
//                initialize the LBS driver.
//
// Members:
//
//          dwClientID      The client application ID assigned to the
//                          LBS application by VZW.
//
//          szClientPWD     The client application password (secret key)
//                          assigned to the LBS application by VZW.
// Note:
//
//     LBS Driver checks that dwClientID and szClientPWD are both non-null.
//
///////////////////////////////////////////////////////////////////////////////


typedef struct _GPS_INIT {
	DWORD			dwClientID;
	const WCHAR		*szClientPWD;
	const WCHAR		*szClientName;
	const WCHAR		*szSMSPrefix;
} GPS_INIT;

#define GPS_MAX_CLIENT_PWD_LEN	256
#define GPS_MAX_CLIENT_NAME_LEN	64
#define GPS_MAX_SMS_PREFIX_LEN	MAX_PATH

#ifndef _GPSAPI_H_
#define GPS_MAX_SATELLITES      12
#define GPS_MAX_PREFIX_NAME     16
#define GPS_MAX_FRIENDLY_NAME   64

#define GPS_VERSION_1           1
#endif // _GPSAPI_H_
#define GPS_VERSION_2			2


//------------------------------------------------------------------------------------

#define GPS_OPEN_NO_HARDWARE_INIT	0x00000001

#define GPS_MAX_SERVER_URL_NAME		256
#define GPS_MAX_SECURITY_DATA		20


// Device Param types
#define GPS_CONFIG_INIT				0x00000000
#define GPS_CONFIG_FIX_MODE			0x00000001
#define GPS_CONFIG_FIX_RATE			0x00000002
#define GPS_CONFIG_QOS				0x00000003
#define GPS_START_FIX				0x00000004
#define GPS_STOP_FIX				0x00000005


//------------------------------------------------------------------------------------

#ifndef _GPSAPI_H_
typedef enum {
	GPS_FIX_UNKNOWN = 0,
	GPS_FIX_2D,
	GPS_FIX_3D
} GPS_FIX_TYPE;

typedef enum {
	GPS_FIX_SELECTION_UNKNOWN = 0,
	GPS_FIX_SELECTION_AUTO,
	GPS_FIX_SELECTION_MANUAL
} GPS_FIX_SELECTION;

typedef enum {
	GPS_FIX_QUALITY_UNKNOWN = 0,
	GPS_FIX_QUALITY_GPS,
	GPS_FIX_QUALITY_DGPS
} GPS_FIX_QUALITY;
#endif // _GPSAPI_H_

// GPS operation modes an application can request to control GPS hardware.
typedef enum {
	GPS_FIX_MODE_UNKNOWN = 0,
	GPS_FIX_MODE_MSA = 1,
	GPS_FIX_MODE_MSB = 2,
	GPS_FIX_MODE_MSS = 3,
	GPS_FIX_MODE_CUSTOM = 100,
	GPS_FIX_MODE_SPEED_OPTIMAL = 101,
	GPS_FIX_MODE_ACCURACY_OPTIMAL = 102,
	GPS_FIX_MODE_DATA_OPTIMAL = 103
} GPS_MODE;

// This structure contains current GPS hardware state.
typedef enum {
	GPS_HW_STATE_UNKNOWN = 0,
	GPS_HW_STATE_ON,
	GPS_HW_STATE_IDLE
} GPS_HW_STATE;

typedef struct _GPS_POSITION_ERROR {
	DWORD dwHorizontalErrorAlong;
	DWORD dwHorizontalErrorAngle;
	DWORD dwHorizontalErrorPerp;
	DWORD dwVerticalError;
	DWORD dwHorizontalConfidence;
} GPS_POSITION_ERROR, *PGPS_POSITION_ERROR;

typedef struct _GPS_FIX_RATE {
	DWORD dwNumFixes;
	DWORD dwTimeBetweenFixes;
} GPS_FIX_RATE, *PGPS_FIX_RATE;

#define GPS_MAX_NUM_FIX             (65535) 
#define GPS_MAX_TIME_BETWEEN_FIXES	(65535)	//(in seconds)
//#define GPS_MAX_TIME_BETWEEN_FIXES	(30 * 60)	// 30 minutes (in seconds)
// Define a maximum value for the total tracking duration, which is the product
// of (dwNumFixes - 1) * dwTimeBetweenFixes. This is the maximum value allowed
// by the LBSDriver on the mobile. Note that the LPS server may limit duration
// even further.
//#define GPS_MAX_TRACKING_DURATION	(60 * 60 * 24 * 7)	// 1 week (in seconds)

typedef struct _GPS_QOS {
	DWORD dwAccuracy;
	DWORD dwPerformance;
} GPS_QOS, *PGPS_QOS;


typedef struct _GPS_START_FIX_CONFIG {
	DWORD dwFlags;
} GPS_START_FIX_CONFIG, *PGPS_START_FIX_CONFIG;

// This structure contains the status information about the GPS hardware used by the GPS
// Intermediate Driver. 
typedef struct _GPS_DEVICE_STATUS {
	DWORD dwValidFields;
	GPS_HW_STATE GPSHwState;
	DWORD dwEphSVMask;
	DWORD dwAlmSVMask;
	DWORD rgdwSatellitesInViewPRNs[GPS_MAX_SATELLITES];
	DWORD rgdwSatellitesInViewCarrierToNoiseRatio[GPS_MAX_SATELLITES];
	DWORD dwDeviceError;
} GPS_DEVICE_STATUS, *PGPS_DEVICE_STATUS;

//
// GPS_VALID_XXX bit flags in GPS_POSITION structure are valid.
//
#ifndef _GPSAPI_H_

#define GPS_VALID_UTC_TIME                                 0x00000001
#define GPS_VALID_LATITUDE                                 0x00000002
#define GPS_VALID_LONGITUDE                                0x00000004
#define GPS_VALID_SPEED                                    0x00000008
#define GPS_VALID_HEADING                                  0x00000010
#define GPS_VALID_MAGNETIC_VARIATION                       0x00000020
#define GPS_VALID_ALTITUDE_WRT_SEA_LEVEL                   0x00000040
#define GPS_VALID_ALTITUDE_WRT_ELLIPSOID                   0x00000080
#define GPS_VALID_POSITION_DILUTION_OF_PRECISION           0x00000100
#define GPS_VALID_HORIZONTAL_DILUTION_OF_PRECISION         0x00000200
#define GPS_VALID_VERTICAL_DILUTION_OF_PRECISION           0x00000400
#define GPS_VALID_SATELLITE_COUNT                          0x00000800
#define GPS_VALID_SATELLITES_USED_PRNS                     0x00001000
#define GPS_VALID_SATELLITES_IN_VIEW                       0x00002000
#define GPS_VALID_SATELLITES_IN_VIEW_PRNS                  0x00004000
#define GPS_VALID_SATELLITES_IN_VIEW_ELEVATION             0x00008000
#define GPS_VALID_SATELLITES_IN_VIEW_AZIMUTH               0x00010000
#define GPS_VALID_SATELLITES_IN_VIEW_SIGNAL_TO_NOISE_RATIO 0x00020000

// The following GPS_VALID fields only exist for a GPS_POSITION2 structure.

#define GPS_VALID_POSITION_UNCERTAINITY_ERROR              0x00040000
#define GPS_VALID_FIX_MODE                                 0x00080000
#define GPS_VALID_FIX_ERROR                                0x00100000
#define GPS_VALID_ENCRYPT_PARAM                            0x00200000
#define GPS_VALID_FIX_QUALITY                              0x00400000
#define GPS_VALID_FIX_TYPE                                 0x00800000
#define GPS_VALID_FIX_SELECTION                            0x01000000

//This family of constants specifies which of the fields in the GPS_DEVICE_STATUS
//structure are valid. The GPSGetDeviceState function fills a GPS_DEVICE_STATUS 
//structure with information about the GPS hardware state and device error returned 
//by GPS hardware. GPS_DEVICE_STATUS includes a field named dwValidFields, which 
//specifies the fields in the structure that are valid, using a bitwise combination 
//of the constants given below.

#define GPS_DEV_VALID_HW_STATE                             0x00000001
#define GPS_DEV_VALID_EPH_SV_MASK                          0x00000002
#define GPS_DEV_VALID_ALM_SV_MASK                          0x00000004
#define GPS_DEV_VALID_SAT_IN_VIEW_PRN                      0x00000008
#define GPS_DEV_VALID_SAT_IN_CARRIER_TO_NOISE_RATIO        0x00000010
#define GPS_DEV_VALID_DEV_ERROR                            0x00000020

//
// GPS_DATA_FLAGS_XXX bit flags set in GPS_POSITION dwFlags field
// provide additional information about the state of the query.
//
// Set when GPS hardware is not connected to GPSID and we 
// are returning cached data.
//#define GPS_DATA_FLAGS_HARDWARE_OFF                        0x00000001


//
// GPS_POSITION contains our latest physical coordinates, the time, 
// and satellites used in determining these coordinates.
//
typedef struct _GPS_POSITION {
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
	SYSTEMTIME stUTCTime; 	//  UTC according to GPS clock.
	
	//** Position + heading related
	double dblLatitude;            // Degrees latitude.  North is positive
	double dblLongitude;           // Degrees longitude.  East is positive
	float  flSpeed;                // Speed in knots
	float  flHeading;              // Degrees heading (course made good).  True North=0
	double dblMagneticVariation;   // Magnetic variation.  East is positive
	float  flAltitudeWRTSeaLevel;  // Altitute with regards to sea level, in meters
	float  flAltitudeWRTEllipsoid; // Altitude with regards to ellipsoid, in meters

	//** Quality of this fix
	GPS_FIX_QUALITY     FixQuality;        // Where did we get fix from?
	GPS_FIX_TYPE        FixType;           // Is this 2d or 3d fix?
	GPS_FIX_SELECTION   SelectionType;     // Auto or manual selection between 2d or 3d mode
	float flPositionDilutionOfPrecision;   // Position Dilution Of Precision
	float flHorizontalDilutionOfPrecision; // Horizontal Dilution Of Precision
	float flVerticalDilutionOfPrecision;   // Vertical Dilution Of Precision

	//** Satellite information
	DWORD dwSatelliteCount;                                            // Number of satellites used in solution
	DWORD rgdwSatellitesUsedPRNs[GPS_MAX_SATELLITES];                  // PRN numbers of satellites used in the solution

	DWORD dwSatellitesInView;                      	                   // Number of satellites in view.  From 0-GPS_MAX_SATELLITES
	DWORD rgdwSatellitesInViewPRNs[GPS_MAX_SATELLITES];                // PRN numbers of satellites in view
	DWORD rgdwSatellitesInViewElevation[GPS_MAX_SATELLITES];           // Elevation of each satellite in view
	DWORD rgdwSatellitesInViewAzimuth[GPS_MAX_SATELLITES];             // Azimuth of each satellite in view
	DWORD rgdwSatellitesInViewSignalToNoiseRatio[GPS_MAX_SATELLITES];  // Signal to noise ratio of each satellite in view
} GPS_POSITION, *PGPS_POSITION;

#endif // _GPSAPI_H_

typedef struct _GPS_POSITION2 {
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
	SYSTEMTIME stUTCTime; 	//  UTC according to GPS clock.
	
	//** Position + heading related
	double dblLatitude;            // Degrees latitude.  North is positive
	double dblLongitude;           // Degrees longitude.  East is positive
	float  flSpeed;                // Speed in knots
	float  flHeading;              // Degrees heading (course made good).  True North=0
	double dblMagneticVariation;   // Magnetic variation.  East is positive
	float  flAltitudeWRTSeaLevel;  // Altitute with regards to sea level, in meters
	float  flAltitudeWRTEllipsoid; // Altitude with regards to ellipsoid, in meters

	//** Quality of this fix
	GPS_FIX_QUALITY     FixQuality;        // Where did we get fix from?
	GPS_FIX_TYPE        FixType;           // Is this 2d or 3d fix?
	GPS_FIX_SELECTION   SelectionType;     // Auto or manual selection between 2d or 3d mode
	float flPositionDilutionOfPrecision;   // Position Dilution Of Precision
	float flHorizontalDilutionOfPrecision; // Horizontal Dilution Of Precision
	float flVerticalDilutionOfPrecision;   // Vertical Dilution Of Precision

	//** Satellite information
	DWORD dwSatelliteCount;                                            // Number of satellites used in solution
	DWORD rgdwSatellitesUsedPRNs[GPS_MAX_SATELLITES];                  // PRN numbers of satellites used in the solution

	DWORD dwSatellitesInView;                      	                   // Number of satellites in view.  From 0-GPS_MAX_SATELLITES
	DWORD rgdwSatellitesInViewPRNs[GPS_MAX_SATELLITES];                // PRN numbers of satellites in view
	DWORD rgdwSatellitesInViewElevation[GPS_MAX_SATELLITES];           // Elevation of each satellite in view
	DWORD rgdwSatellitesInViewAzimuth[GPS_MAX_SATELLITES];             // Azimuth of each satellite in view
	DWORD rgdwSatellitesInViewSignalToNoiseRatio[GPS_MAX_SATELLITES];  // Signal to noise ratio of each satellite in view

	GPS_POSITION_ERROR PositionUncertainityError;
	GPS_MODE FixMode;
	DWORD dwFixError;
} GPS_POSITION2, *PGPS_POSITION2;

#ifndef _GPSAPI_H_

// 
// GPS_DEVICE contains information about the device driver and the
// service itself and is returned on a call to GPSGetDeviceState().
// 
typedef struct _GPS_DEVICE {
	DWORD    dwVersion;                                 // Current version of GPSID client is using.
	DWORD    dwSize;                                    // sizeof this structure
	DWORD    dwServiceState;                            // State of the GPS Intermediate Driver service.  
	DWORD    dwDeviceState;                             // Status of the actual GPS device driver.
	FILETIME ftLastDataReceived;                        // Last time that the actual GPS device sent information to the intermediate driver.
	WCHAR    szGPSDriverPrefix[GPS_MAX_PREFIX_NAME];    // Prefix name we are using to communicate to the base GPS driver
	WCHAR    szGPSMultiplexPrefix[GPS_MAX_PREFIX_NAME]; // Prefix name that GPS Intermediate Driver Multiplexer is running on
	WCHAR    szGPSFriendlyName[GPS_MAX_FRIENDLY_NAME];  // Friendly name real GPS device we are currently using
} *PGPS_DEVICE, GPS_DEVICE;

#endif // _GPSAPI_H_

// 
// GPS_DEVICE contains information about the device driver and the
// service itself and is returned on a call to GPSGetDeviceState().
// 
typedef struct _GPS_DEVICE2 {
	DWORD    dwVersion;                                 // Current version of GPSID client is using.
	DWORD    dwSize;                                    // sizeof this structure
	DWORD    dwServiceState;                            // State of the GPS Intermediate Driver service.  
	DWORD    dwDeviceState;                             // Status of the actual GPS device driver.
	FILETIME ftLastDataReceived;                        // Last time that the actual GPS device sent information to the intermediate driver.
	WCHAR    szGPSDriverPrefix[GPS_MAX_PREFIX_NAME];    // Prefix name we are using to communicate to the base GPS driver
	WCHAR    szGPSMultiplexPrefix[GPS_MAX_PREFIX_NAME]; // Prefix name that GPS Intermediate Driver Multiplexer is running on
	WCHAR    szGPSFriendlyName[GPS_MAX_FRIENDLY_NAME];  // Friendly name real GPS device we are currently using
	GPS_DEVICE_STATUS DeviceStatus;
} *PGPS_DEVICE2, GPS_DEVICE2;


// LBS Driver states returned by GPSGetDeviceState().
#define GPS_LBSDRIVER_STATE_OFF				SERVICE_STATE_OFF
#define GPS_LBSDRIVER_STATE_COMING_UP		SERVICE_STATE_STARTING_UP
#define GPS_LBSDRIVER_STATE_ON				SERVICE_STATE_ON



///////////////////////////////////////////////////////////////////////////////
//
//  Following the convention described in winerror.h,
//  error values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag (set to 1)
//
//      R - is a reserved bit (set to 0)
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
///////////////////////////////////////////////////////////////////////////////

#define GPS_ERR_SUCCESS							ERROR_SUCCESS

// General error codes
#define GPS_ERR_ACCESS_DENIED					ERROR_ACCESS_DENIED
#define GPS_ERR_INVALID_HANDLE					ERROR_INVALID_HANDLE
#define GPS_ERR_INVALID_PARAMETER				ERROR_INVALID_PARAMETER
#define GPS_ERR_DEVICE_BUSY						ERROR_BUSY
#define GPS_ERR_INVALID_STATE					ERROR_INVALID_STATE
#define GPS_ERR_NOT_AUTHENTICATED				ERROR_NOT_AUTHENTICATED
#define GPS_ERR_NETWORK_UNREACHABLE				ERROR_NETWORK_UNREACHABLE
#define GPS_ERR_SERVER_UNREACHABLE				ERROR_HOST_UNREACHABLE
#define GPS_ERR_GETPOS_TIMEOUT					ERROR_TIMEOUT
#define GPS_ERR_ERROR_NOT_ENOUGH_QUOTA			ERROR_NOT_ENOUGH_QUOTA

#define GPS_ERR_INSUFFICIENT_SAT_SIGNAL			0x20000000	// Poor signal strength from satellites
#define GPS_ERR_FATAL_ERROR						0x20000100	// Fatal error, close the application
#define GPS_ERR_OPEN_FAIL_ALREADY_OPEN			0x20000101	// Handle already open
#define GPS_ERR_OPEN_FAIL_MAX_APP_LIMIT_REACHED	0x20000102	// Max application running
#define GPS_ERR_POSITION_INFO_NOT_AVAILABLE		0x20000103
#define GPS_ERR_LOCATION_REQ_QUEUE_FULL			0x20000105
#define GPS_ERR_PREV_LOCATION_REQ_PENDING		0x20000106
#define GPS_ERR_RADIO_OFF						0x20000109
#define GPS_ERR_GPSONE							0x2000010a

#define GPS_ERR_NO_SYSTEM_RESOURCES				0xe0011001
#define GPS_ERR_TIMEOUT							0xe0011002
#define GPS_ERR_FAILED_TO_GET_MDN				0xe0011003
#define GPS_ERR_NOT_SUPPORTED					0xe0011004

// Internal errors
#define GPS_ERR_INTERNAL			0xe0011101

#define GPS_ERR_DEVICE_NOT_OPEN					0xe0012004	// Handle not open
#define GPS_ERR_PRIVACY_REFUSED					0xe0012007
#define GPS_ERR_TRACKING_SESSION_EXPIRED		0xe0012008
#define GPS_ERR_OPEN_INVALID_NEWLOCATIONDATA_HANDLE		0xe001200a	// hNewLocationData is NULL
#define GPS_ERR_OPEN_INVALID_DEVICESTATECHANGE_HANDLE	0xe001200b	// hDeviceStateChange is NULL
#define GPS_ERR_OPEN_INVALID_DEVICE_NAME				0xe001200c	// szDeviceName is not NULL
#define GPS_ERR_OPEN_INVALID_FLAGS						0xe001200d	// dwFlags is not 0
#define GPS_ERR_SET_NOT_INITIALIZED						0xe001200e	// GPSSetDeviceParam - GPS_CONFIG_INIT not done

#define GPS_ERR_INIT_REQ_PENDING				0xe0012011
#define GPS_ERR_FAILED_AUTHENTICATION			0xe0012012
#define GPS_ERR_FAILED_EULA_DECLINED			0xe0012013
#define GPS_ERR_FAILED_FTT_CANCELLED			0xe0012014
#define GPS_ERR_FAILED_LOCATION_DISABLED		0xe0012015
#define GPS_ERR_FAILED_LPS_CONNECTION			0xe0012016

#define GPS_ERR_INIT_DATA_NULL					0xe0012017	// dwParamData is NULL
#define GPS_ERR_INIT_PASSWORD_NULL				0xe0012018	// dwParamData->sqClientPWD is NULL
#define GPS_ERR_INIT_PASSWORD_INVALID			0xe0012019	// dwParamData->sqClientPWD is invalid
#define GPS_ERR_INIT_PRODUCT_NAME_NULL			0xe001201a	// dwParamData->sqClientName is NULL
#define GPS_ERR_INIT_PRODUCT_NAME_INVALID		0xe001201b	// dwParamData->sqClientName too long
#define GPS_ERR_INIT_LENGTH_INVALID				0xe001201c	// dwDataLen is not sizeof(GPS_INIT)
#define GPS_ERR_INIT_SMS_PREFIX_INVALID			0xe001201d	// dwParamData->szSMSPrefix too long
#define GPS_ERR_INIT_SMS_PREFIX_FAILED			0xe001201e	// set/clear of SMS Prefix failed

#define GPS_ERR_INVALID_PARAMETER_NAME			0xe0012020
#define GPS_ERR_GETPOS_DATA_NULL				0xe0012021
#define GPS_ERR_BAD_GETPOS_VERSION_PARAM		0xe0012022
#define GPS_ERR_GETPOS_PRIVACY_REFUSED			0xe0012024
#define GPS_ERR_SERVICE_NOT_AUTHORIZED			0xe0012025
#define GPS_ERR_AUTHENTICATION_FAIL				0xe0012028
#define GPS_ERR_INFORMATION_NOT_AVAILABLE		0xe0012029	// GPSGetDeviceState
#define GPS_ERR_GETPOS_FLAGS_INVALID			0xe001202a	// dwFlags is not 0
#define GPS_ERR_GETPOS_MAXAGE_INVALID			0xe001202b	// dwMaximumAge is 0
#define GPS_ERR_GETPOS_LENGTH_INVALID			0xe001202c	// dwSize doesn't correspond to dwVersion

#define GPS_ERR_FIX_MODE_DATA_NULL					0xe0012031	// dwParamData is NULL
#define GPS_ERR_FIX_MODE_DATA_INVALID				0xe0012032	// dwParamData contains invalid mode
#define GPS_ERR_FIX_MODE_LENGTH_INVALID				0xe0012033	// dwDataLen is not sizeof(GPS_MODE)
#define GPS_ERR_FIX_MODE_NOT_SUPPORTED				0xe0012034	// mode not supported on this device

#define GPS_ERR_FIX_RATE_DATA_NULL					0xe0012041	// dwParamData is NULL
#define GPS_ERR_FIX_RATE_NUM_FIXES_INVALID			0xe0012042	// must be >= 0
#define GPS_ERR_FIX_RATE_INTERVAL_SINGLE_INVALID	0xe0012043	// single shot interval must be 0
#define GPS_ERR_FIX_RATE_INTERVAL_TRACKING_INVALID	0xe0012044	// tracking session interval must be > 0
#define GPS_ERR_FIX_RATE_INTERVAL_STOP_INVALID		0xe0012045	// stop interval must be 0
#define GPS_ERR_FIX_RATE_LENGTH_INVALID				0xe0012046	// dwDataLen is not sizeof(GPS_FIX_RATE)
#define GPS_ERR_FIX_RATE_TRACKING_DURATION_INVALID	0xe0012047	// tracking session duration > GPS_MAX_TRACKING_DURATION (Deprecated)

#define GPS_ERR_QOS_DATA_NULL						0xe0012051	// dwParamData is NULL
#define GPS_ERR_QOS_LENGTH_INVALID					0xe0012054	// dwDataLen is not sizeof(GPS_QOS)

#define GPS_ERR_START_DATA_NULL						0xe0012061	// dwParamData is NULL
#define GPS_ERR_START_FLAGS_INVALID					0xe0012062	// must be 0
#define GPS_ERR_START_LENGTH_INVALID				0xe0012063	// dwDataLen is not sizeof(GPS_START_FIX_CONFIG)

#define GPS_ERR_STOP_DATA_NOT_NULL					0xe0012071	// dwParamData is not NULL
#define GPS_ERR_STOP_LENGTH_INVALID					0xe0012072	// dwDataLen is not 0

#define GPS_ERR_GETPARAM_DATA_NULL					0xe0012081

#define GPS_ERR_BAD_GETDEVICE_VERSION_PARAM			0xe0012091
#define GPS_ERR_GETDEVICE_DATA_NULL					0xe0012092
#define GPS_ERR_GETDEVICE_LENGTH_INVALID			0xe0012093

#define GPS_ERR_MODULE_NOT_TRUSTED					0xe00120a1	// untrusted signature for app .exe or .dll 

#define GPS_ERR_ESTABLISH_CONNECTION_FAILED         0xe00120a2  // Could not establish a packet-data (CDMA or GSM) connection
#define GPS_ERR_SERVICE_UNAVAILABLE                 0xe00120a3  // PDE is down or location service otherwise currently unavailable

#define GPS_ERR_NOT_PROVISIONED                     0xe00120b0  // Privacy not provisioned

#ifdef __cplusplus
extern "C" {
#endif
///////////////////////////////////////////////////////////////////////////////
//
// Function Name:  GPSOpenDevice
//
// Purpose:  opens the driver, saves the events handles specified
//           by the application, and returns a handle to the LBS driver.
//
// Parameters:
//           hNewLocationData       Handle to a Windows CE event created
//                                  using CreateEvent. The LBS driver signals
//                                  the passed event whenever it has new GPS
//                                  location information.
// 
//           hDeviceStateChange     Handle to a Windows CE event created
//                                  using CreateEvent. The LBS driver signals
//                                  the passed event when the device state
//                                  changes.
// 
//           szDeviceName           Must be NULL.
// 
//           dwFlags                Must be 0.
//
// Return Values:
//
//          If successful, returns a handle to the LBS driver.
//          This handle should be used for all subsequent calls to
//          the LBS driver.
//
//          If unsuccessful, returns NULL.
//
// Remarks:
//          An application must call this API first to obtain a handle to
//          the LBS driver before making any requests for security
//          updates or location data.
//
///////////////////////////////////////////////////////////////////////////////
LBSDRIVER_API HANDLE GPSOpenDevice (
                HANDLE          hNewLocationData,    // CE event
                HANDLE          hDeviceStateChange,  // CE event
                const WCHAR *   szDeviceName,
                DWORD           dwFlags
);


///////////////////////////////////////////////////////////////////////////////
//
// Function Name:  GPSSetDeviceParam
//
// Purpose:        This function is used to set parameters that control
//                 the LBS driver and GPS engine.
//
// Parameters:
//         hGpsDevice      Handle returned by a call to GPSOpenDevice.
// 
//         dwParamName
//             The following dwParamName values are supported:
// 
//                     dwParamName Value    Description
//                     -----------------    -------------------------------------
//                     GPS_CONFIG_INIT      Initialize LBS driver. This must be
//                                          the first call after an application
//                                          calls GPSOpenDevice().  After this
//                                          function is called, applications
//                                          should wait until hDeviceStateChange
//                                          is signaled to indicate the driver
//                                          was successfully initialized or an
//                                          error occurred.
//
//                                          pbParamData     pointer to a
//                                                          GPS_INIT structure,
//                                                          which contains fields
//                                                          for the client
//                                                          application ID, and
//                                                          password (both
//                                                          assigned by VZW).
// 
//                     GPS_START_FIX        Starts a GPS fix.  When a GPS fix is
//                                          available, the application is sent a
//                                          signal to the event that was specified
//                                          as the hNewLocationData parameter of
//                                          the GPSOpenDevice function.
// 
//                                          pbParamData     pointer to a
//                                                          GPS_START_FIX_CONFIG
//                                                          structure, which
//                                                          contains a single
//                                                          field set to zero.
// 
//                     GPS_STOP_FIX         Stops a GPS fix.
// 
//                                          pbParamData     NULL
// 
//                     GPS_CONFIG_FIX_MODE  Set the GPS Operation Mode
//                                          for subsequent fixes.
// 
//                                          pbParamData     pointer to a
//                                                          GPS_MODE
//                                                          enumerated value.
// 
//                     GPS_CONFIG_FIX_RATE  Set the GPS Operation Mode for
//                                          subsequent fixes.
// 
//                                          pbParamData      pointer to a
//                                                           GPS_FIX_RATE
//                                                           structure, which
//                                                           contains fields for
//                                                           the number of fixes,
//                                                           and the time between
//                                                           fixes.
// 
//                     GPS_CONFIG_QOS       Set the GPS Operation Mode for
//                                          subsequent fixes.
// 
//                                          pbParamData      pointer to a
//                                                           GPS_QOS structure,
//                                                           which contains for
//                                                           the desired accuracy
//                                                           (in meters), and the
//                                                           desired response time
//                                                           (in seconds).
// 
//         pbParamData         Pointer to a parameter data buffer. Refer to
//                             the dwParamName information above.
// 
//         dwDataLen           Size of data (in bytes) pointed to by pbParamData
// 
// Return values:
// 
//     If successful, this function returns ERROR_SUCESS.
//     If unsuccessful, returns an error.
// 
// Remarks:
//     When GPSSetDeviceParam is invoked with dwParamName set to
//     GPS_CONFIG_INIT or GPS_START_FIX, it operates in an asynchronous mode,
//     otherwise it operates in a synchronous mode.
//
///////////////////////////////////////////////////////////////////////////////

LBSDRIVER_API DWORD GPSSetDeviceParam (
                        HANDLE        hGPSDevice,
                        DWORD        dwParamName,
                        BYTE *        pbParamData,
                        DWORD        dwDataLen
);



///////////////////////////////////////////////////////////////////////////////
//
//  Function Name:  GPSGetDeviceParam
//
//  Purpose:        This function is used to retrieve the current configuration
//                  parameters for the calling application.
//
// Parameters:
//
//      hGpsDevice      Handle returned by a call to GPSOpenDevice.
//
//      dwParamName
//           The following dwParamName values are supported:
//
//                     dwParamName Value    Description
//                     -----------------    -------------------------------------
//                     GPS_CONFIG_FIX_MODE  Get the GPS Operation Mode for
//                                          subsequent fixes.
//
//                                          pbParamData  pointer to a
//                                                       GPS_MODE variable.
//
//                     GPS_CONFIG_FIX_RATE  Get the GPS Operation Mode for
//                                          subsequent fixes.
//
//                                          pbParamData  pointer to a
//                                                       GPS_FIX_RATE structure.
//
//                     GPS_CONFIG_QOS       Get the GPS Operation Mode for
//                                          subsequent fixes.
//
//                                          pbParamData   pointer to a
//                                                        GPS_QOS structure.
//
//       pbParamData         Pointer to a parameter data buffer. Refer to
//                           the dwParamName information above.
//
//       pdwDataLen          On input, a pointer to a variable that contains
//                           the size of data (in bytes) pointed to by
//                           pbParamData. On output, this variable contains
//                           the size of the data retrieved.
//
// Return values:
// 
//     If successful, this function returns ERROR_SUCESS.
//     If unsuccessful, returns an error.
// 
// Remarks:
//
//     None.
//
///////////////////////////////////////////////////////////////////////////////

LBSDRIVER_API DWORD GPSGetDeviceParam (
                        HANDLE        hGPSDevice,
                        DWORD        dwParamName,
                        BYTE *        pbParamData,
                        DWORD *        pdwDataLen
);




///////////////////////////////////////////////////////////////////////////////
//
// Function Name:  GPSGetDeviceState
//
// Purpose:        This function is used to retrieve information about
//                 the current state of the GPS hardware.
//
// Parameters:
//
//      hGpsDevice      Handle returned by a call to GPSOpenDevice.
//
//      pGPSDevice      Pointer to a data buffer of type GPS_DEVICE or
//                      GPS_DEVICE2. To receive data corresponding to
//                      GPS_DEVICE, the application should set the dwVersion
//                      parameter to GPS_VERSION_1 before calling
//                      GPSGetDeviceState. To receive data corresponding to
//                      GPS_DEVICE2, the application should set the
//                      dwVersion parameter to GPS_VERSION_2 before
//                      calling GPSGetDeviceState.
//
// Return values:
// 
//     If successful, this function returns ERROR_SUCESS.
//     If unsuccessful, returns an error.
// 
// Remarks:
//
//     Applications may call this function when a signal is received
//     by the event that was passed in the hDeviceStateChange parameter
//     of the GPSOpenDevice function.
//
///////////////////////////////////////////////////////////////////////////////

LBSDRIVER_API DWORD GPSGetDeviceState (
                        HANDLE          hGPSDevice,
                        GPS_DEVICE *    pGPSDevice
);


///////////////////////////////////////////////////////////////////////////////
//
// Function Name:  GPSGetPosition
//
// Purpose:        This function is used to retrieve location
//                  information that meets the specified maximum age criteria.
//
// Parameters:
//
//      hGpsDevice      Handle returned by a call to GPSOpenDevice.
//
//      pGPSPosition    Pointer to a data buffer of type GPS_POSITION or
//                      GPS_POSITION2.  To receive data corresponding to
//                      GPS_POSITION, the application should set the
//                      dwVersion parameter to GPS_VERSION_1 before calling
//                      GPSGetPosition. To receive data corresponding to
//                      GPS_POSITION2, the application should set the
//                      dwVersion parameter to GPS_VERSION_2 before calling
//                      GPSGetPosition
//
//      dwMaximumAge    The driver returns only location information that
//                      has been received within the time (in milliseconds)
//                      specified by this maximum age parameter.
//
//      dwFlags         Must be 0.
//
// Return values:
// 
//     If successful, this function returns ERROR_SUCESS.
//     If unsuccessful, returns an error.
// 
// Remarks:
//
//     Applications may call this function when a signal is received
//     by the event that was passed in the hNewLocationData parameter
//     of the GPSOpenDevice function. The signal is sent as the result
//     of the application requesting a GPS fix by calling GPSSetDeviceParam
//     with a dwParamName value of GPS_START_FIX.
//
///////////////////////////////////////////////////////////////////////////////
LBSDRIVER_API DWORD GPSGetPosition (
                        HANDLE           hGPSDevice,
                        GPS_POSITION *   pGPSPosition,
                        DWORD            dwMaximumAge,
                        DWORD            dwFlags
);



///////////////////////////////////////////////////////////////////////////////
//
//  Function Name:      GPSCloseDevice
//
//  Purpose:            This function closes the handle to the LBS driver
//                      that was returned in the call to GPSOpenDevice.
//
// Parameters:
//
//      hGpsDevice      Handle returned by a call to GPSOpenDevice.
//
//
// Return values:
// 
//     If successful, this function returns ERROR_SUCESS.
//     If unsuccessful, returns an error.
// 
// Remarks:
//
//     None.
//
///////////////////////////////////////////////////////////////////////////////

LBSDRIVER_API DWORD GPSCloseDevice (
                        HANDLE        hGPSDevice
);

#ifdef __cplusplus
}
#endif
