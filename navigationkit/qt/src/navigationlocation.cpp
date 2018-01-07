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
    @file navigationlocation.cpp
    @date 10/08/2014
    @addtogroup navigationkit
*/
/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
---------------------------------------------------------------------------*/
#include "navigationlocation.h"

namespace locationtoolkit
{
/*! Constructor*/
NavigationLocation::NavigationLocation(const locationtoolkit::Location& ltklocation)
{
    mltkLocation = ltklocation;
}
/*! Destructor*/
NavigationLocation::~NavigationLocation()
{

}
/*! Gets location altitude.*/
double NavigationLocation::Altitude() const
{
    return mltkLocation.altitude;
}
/*! Gets location GPS time.*/
unsigned int NavigationLocation::GpsTime() const
{
    return (unsigned int )mltkLocation.gpsTime;
}

/*! Gets location heading.*/
double NavigationLocation::Heading() const
{
    return mltkLocation.heading;
}

/*! Gets standard deviation of horizontal uncertainty along the axis of uncertainty. */
double NavigationLocation::HorizontalUncertaintyAlongAxis() const
{
    return mltkLocation.horizontalUncertaintyAlongAxis;
}

/*! Gets Standard deviation of horizontal uncertainty perpendicular to the axis of uncertainty.*/
double NavigationLocation::HorizontalUncertaintyAlongPerpendicular() const
{
    return mltkLocation.horizontalUncertaintyAlongPerpendicular;
}

/*! Gets heading in degrees of the axis of uncertainty.*/
double NavigationLocation::HorizontalUncertaintyAngleOfAxis() const
{
    return mltkLocation.horizontalUncertaintyAngleOfAxis;
}

/*! Gets horizontal velocity in meters/second.*/
double NavigationLocation::HorizontalVelocity() const
{
    return mltkLocation.horizontalVelocity;
}

/*! Gets location latitude in degrees.*/
double NavigationLocation::Latitude() const
{
    return mltkLocation.latitude;
}

/*! Gets location longitude in degrees.*/
double NavigationLocation::Longitude() const
{
    return mltkLocation.longitude;
}

/*! Gets number of satellites acquired.*/
int NavigationLocation::NumberOfSatellites() const
{
    return (int)mltkLocation.numberOfSatellites;
}

/*! Gets difference in seconds between GPS time and UTC time (leap seconds).*/
int NavigationLocation::UtcOffset() const
{
    return (int)mltkLocation.utcOffset;
}

/*! Gets {@link LocationValid} flags indicating which fields in the class have valid values.*/
unsigned int NavigationLocation::Valid() const
{
    return (unsigned int)mltkLocation.valid;
}

/*! Gets standard deviation of vertical uncertainty.*/
double NavigationLocation::VerticalUncertainty() const
{
    return mltkLocation.verticalUncertainty;
}

/*! Gets vertical velocity in meters/seconds.*/
double NavigationLocation::VerticalVelocity() const
{
    return mltkLocation.verticalVelocity;
}

/*! Gets is this location is gps fix.*/
bool NavigationLocation::IsGpsFix() const
{
    return mltkLocation.horizontalUncertaintyAlongAxis<=50 ;
}
}
