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

@file     utility.h
@defgroup Utility file to include common code

Function declarations for general utility functions used for system test.
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

/*! @{ */

#ifndef _UTILITY_H_
#define _UTILITY_H_


#include "pal.h"
#include "paltypes.h"
#include "nbgpstypes.h"

typedef enum
{
    LogLevelNone = 0,
    LogLevelLow,
    LogLevelMedium,
    LogLevelHigh
}Test_LogLevel;

typedef enum
{
    TestLevelSmoke = 0,
    TestLevelIntermediate,
    TestLevelFull
}Test_TestLevel;

typedef enum
{
    TestNetworkTcp = 0,
    TestNetworkTcpTls,
    TestNetworkHttp,
    TestNetworkHttps
}Test_NetworkType;

typedef enum
{
    TestOverwriteFlagFalse = 0,
    TestOverwriteFlagTrue
}Test_OverwriteFlag;

typedef enum
{
    TestNetworkDomainNavBuilder = 0,
    TestNetworkDomainVerizon,
    TestNetworkDomainDevLocal
} Test_NetworkDomain;

typedef enum
{
    TestCarrierVerizon = 0x001,
    TestCarrierAtlasbook = 0x010
} Test_Carrier;

/*! Default MIN value -- taken from wm4main/main/vnavwm/main/winbrewutil/util.c */
#define DEFAULT_MOBILE_IDENTIFIER_NUMBER    999999999999999ULL


/*! Default MDN value */
//#define DEFAULT_MOBILE_DIRECTORY_NUMBER     9494533800ULL
// for V6 GBB -- use whitelisted MDN as default to get around V6 license issues
#define DEFAULT_MOBILE_DIRECTORY_NUMBER     9999990014ULL


/*! Read MDN from mdn.txt file located in executable directory */
uint64 GetMobileDirectoryNumberFromFile(PAL_Instance* pal);


/*! Get version number of system tests application */
nb_version GetApplicationVersion();

/*! Delete all temporary test files in the current directory that begin with the given prefix */
void DeleteTemporaryTestFiles(PAL_Instance* pal, const char* fileNamePrefix);


int LoadFile(PAL_Instance* pal, const char* directory, const char* name, nb_size name_len, char** pbuf, nb_size* psize);

// Utilities for GPS


/*! AEEGPS Flags to be used in navigation */
#define  AEEGPS_VALID_LAT     0x0001
#define  AEEGPS_VALID_LON     0x0002
#define  AEEGPS_VALID_ALT     0x0004
#define  AEEGPS_VALID_HEAD    0x0008
#define  AEEGPS_VALID_HVEL    0x0010
#define  AEEGPS_VALID_VVEL    0x0020
#define  AEEGPS_VALID_HUNC    0x0040
#define  AEEGPS_VALID_AUNC    0x0080
#define  AEEGPS_VALID_PUNC    0x0100
#define  AEEGPS_VALID_VUNC    0x0200
#define  AEEGPS_VALID_UTCOFFSET 0x0400

/*! Default spacing value for POIs */
#define  POIS_SPACING_VALUE   250

/*! Taken from the BREW code to be used to decode and encode for GPS fixes*/
static const double unctbl[] = { 0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 4.0, 6.0, 8.0, 12.0, 16.0, 24.0, 32.0,
48.0, 64.0, 96.0, 128.0, 192.0, 256.0, 384.0, 512.0, 768.0, 1024.0, 1536.0, 2048.0, 3072.0, 4096.0,
6144.0, 8192.0, 12288.0, -1.0, -2.0 };

/*! GPSFileRecord Structure for reading the data from GPS file */
typedef struct {
    uint32             dwTimeStamp;          /*! Time, seconds since 1/6/1980 */
    uint32             status;               /*! Response status; */
    int32              dwLat;                /*! Lat, 180/2^25 degrees, WGS-84 ellipsoid */
    int32              dwLon;                /*! Lon, 360/2^26 degrees, WGS-84 ellipsoid */
    int16              wAltitude;            /*! Alt, meters, WGS-84 ellipsoid */
    uint16             wHeading;             /*! Heading, 360/2^10 degrees */
    uint16             wVelocityHor;         /*! Horizontal velocity, 0.25 meters/second */
    int8               bVelocityVer;         /*! Vertical velocity, 0.5 meters/second */
    uint8              accuracy;             /*! Accuracy of the data */
    uint16             fValid;               /*! Flags indicating valid fields in the struct.*/
    uint8              bHorUnc;              /*! Horizontal uncertainity */
    uint8              bHorUncAngle;         /*! Hor. Uncertainity at angle */
    uint8              bHorUncPerp;          /*! Hor. uncertainty perpendicular */
    uint16             wVerUnc;              /*! Vertical uncertainity. */
} GPSFileRecord;

typedef struct {

    uint32          fileSize;
    unsigned char*  fileBuffer;

    GPSFileRecord*  nextFix;
    uint32          numFixes;
    uint32          curFix;

    NB_GpsLocation  curGPSLocation;

} TestGPSFile;

nb_boolean GetNextGpsFix(char** current, int* gpsTime, double* latitude, double* longitude);
void GPSInfo2GPSFix(GPSFileRecord* pos, NB_GpsLocation* cur);
double UncDecode(uint8 unc);
uint8 UncEncode(double v);
int GetMaxFixes();

#endif //_UTILITY_H_

/*! @} */
