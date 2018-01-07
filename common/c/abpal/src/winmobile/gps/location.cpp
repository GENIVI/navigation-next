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

@file location.cpp
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
    @addtogroup abpalgpswinmobile
    @{
*/

#include "location.h"

// convert to units of 180/2^25 degrees, WGS-84 ellipsoid 
const double Location::LATITUDE_CONSTANT      = 0.00000536441803;
const double Location::LONGITUDE_CONSTANT     = 0.00000536441803;

// convert heading to units of  360/2^10 degrees 
const double Location::HEADING_CONSTANT       = 0.3515625;

// convert knots to meters/sec (1 knot = 0.514444444 meters / second)
const double Location::SPEED_CONSTANT         = 0.514444444;

// Difference, in seconds, between GPS and Unix time values
const int Location::GPS_TO_UNIX_OFFSET        = 315964800;

Location::Location()
{
    Reset();
}

void Location::Reset()
{
    status = 0;
    gpsTime = 0;
    valid = 0;
    latitude = -999.0;
    longitude = -999.0;
    heading = 0;
    horizontalVelocity = 0;
    altitude = 0;
    verticalVelocity = 0;
    horizontalUncertaintyAngleOfAxis = 0;
    horizontalUncertaintyAlongAxis = 0;
    horizontalUncertaintyAlongPerpendicular = 0;
    verticalUncertainty = 0;
    utcOffset = 0;
    numberOfSatellites = 0;
    m_isValid = false;
    m_errorInfoXml = "";
}

void Location::Validate()
{
    m_isValid = (valid != 0); /// @todo && (numberOfSatellites > 0);
}

nb_gpsTime Location::SystemTimeToGpsTime(const SYSTEMTIME* systemTime)
{
    LARGE_INTEGER jan1970FT = { 0 };
    jan1970FT.QuadPart = 116444736000000000I64; // january 1st 1970 (unix time)

    LARGE_INTEGER utcFT = { 0 };
    SystemTimeToFileTime(systemTime, (FILETIME*)&utcFT);

    unsigned __int64 unixTime = (utcFT.QuadPart - jan1970FT.QuadPart) / 10000000;

    return (nb_gpsTime)(unixTime - Location::GPS_TO_UNIX_OFFSET);
}

bool Location::IsSameFix(const Location& other) const
{
    bool sameFix = gpsTime == other.gpsTime;

    if (sameFix && (valid & other.valid & PGV_Latitude))
    {
        sameFix = latitude == other.latitude;
    }

    if (sameFix && (valid & other.valid & PGV_Longitude))
    {
        sameFix = longitude == other.longitude;
    }

    if (sameFix && (valid & other.valid & PGV_Heading))
    {
        sameFix = heading == other.heading;
    }

    if (sameFix && (valid & other.valid & PGV_HorizontalVelocity))
    {
        sameFix = horizontalVelocity == other.horizontalVelocity;
    }

    return sameFix;
}


/*! @} */
