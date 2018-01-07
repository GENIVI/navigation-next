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
 @file         GetDestinationOfGPSFile.h
 @defgroup     NavkitExample
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary to
 TeleCommunication Systems, Inc., and considered a trade secret as defined
 in section 499C of the penal code of the State of California. Use of this
 information by anyone other than authorized employees of TeleCommunication
 Systems is granted only under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 --------------------------------------------------------------------------*/

/*! @{ */
#include "place.h"

/*! GPSFileRecord Structure for reading the data from GPS file */
typedef struct
{
    unsigned int      dwTimeStamp;          /*! Time, seconds since 1/6/1980 */
    unsigned int      status;               /*! Response status; */
    int               dwLat;                /*! Lat, 180/2^25 degrees, WGS-84 ellipsoid */
    int               dwLon;                /*! Lon, 360/2^26 degrees, WGS-84 ellipsoid */
    short             wAltitude;            /*! Alt, meters, WGS-84 ellipsoid */
    unsigned short    wHeading;             /*! Heading, 360/2^10 degrees */
    unsigned short    wVelocityHor;         /*! Horizontal velocity, 0.25 meters/second */
    signed char       bVelocityVer;         /*! Vertical velocity, 0.5 meters/second */
    unsigned char     accuracy;             /*! Accuracy of the data */
    unsigned short    fValid;               /*! Flags indicating valid fields in the struct.*/
    unsigned char     bHorUnc;              /*! Horizontal uncertainity */
    unsigned char     bHorUncAngle;         /*! Hor. Uncertainity at angle */
    unsigned char     bHorUncPerp;          /*! Hor. uncertainty perpendicular */
    unsigned short    wVerUnc;              /*! Vertical uncertainity. */
} GPSFileRecord;

void GetDefaultDestinationAndOrigin(const std::string& gpsFile, locationtoolkit::Place* destination, locationtoolkit::Place* origin);

/*! @} */
