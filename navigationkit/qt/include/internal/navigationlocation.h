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
    @file navlocation.h
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
/*! @{ */
#ifndef LOCATIONTOOLKIT_LTK_NAVIGATIONLOCATION_H
#define LOCATIONTOOLKIT_LTK_NAVIGATIONLOCATION_H

#include "NavApiTypes.h"
#include "location.h"

namespace locationtoolkit
{
class NavigationLocation : public nbnav::Location
{
public:
    /*! Constructor of NavigationLocation
    * @param ltklocation ltk Location type
        @return
    */
    NavigationLocation(const locationtoolkit::Location& ltklocation);

    /*! Destructor of NavigationLocation
        @return
    */
    virtual ~NavigationLocation();

   /*! Gets location altitude.

        @return Altitude in meters
    */
    virtual double Altitude() const;

    /*! Gets location GPS time.

        @return Time in seconds since the GPS Epoch: Midnight, January 6, 1980
    */
    virtual unsigned int GpsTime() const;

    /*! Gets location heading.

        @return Heading in degrees
    */
    virtual double Heading() const;

    /*! Gets standard deviation of horizontal uncertainty along the axis of uncertainty.

        @return Horizontal uncertainty in meters
    */
    virtual double HorizontalUncertaintyAlongAxis() const;

    /*! Gets Standard deviation of horizontal uncertainty perpendicular to the axis of uncertainty.

        @return Horizontal uncertainty in meters
    */
    virtual double HorizontalUncertaintyAlongPerpendicular() const;

    /*! Gets heading in degrees of the axis of uncertainty.

        @return Heading in degrees
    */
    virtual double HorizontalUncertaintyAngleOfAxis() const;

    /*! Gets horizontal velocity in meters/second.

        @return velocity in meters/second
    */
    virtual double HorizontalVelocity() const;

    /*! Gets location latitude in degrees.

        @return Latitude in degrees
    */
    virtual double Latitude() const;

    /*! Gets location longitude in degrees.

        @return Longitude in degrees
   */
    virtual double Longitude() const;

    /*! Gets number of satellites acquired.

        @return Number of satellites
    */
    virtual int NumberOfSatellites() const;

    /*! Gets difference in seconds between GPS time and UTC time (leap seconds).

        @return Difference in seconds
    */
    virtual int UtcOffset() const;

    /*! Gets {@link LocationValid} flags indicating which fields in the class have valid values.

        @return Valid flags
    */
    virtual unsigned int Valid() const;

    /*! Gets standard deviation of vertical uncertainty.

        @return Vertical uncertainty in meters
    */
    virtual double VerticalUncertainty() const;

    /*! Gets vertical velocity in meters/seconds.

        @return Vertical velocity in meters/seconds
    */
    virtual double VerticalVelocity() const;
    /*! Gets is this location is gps fix.

        @return {@code true} is gps fix
     */
    virtual bool IsGpsFix() const;

private:

    locationtoolkit::Location mltkLocation;

};

}
#endif //LOCATIONTOOLKIT_LTK_NAVIGATIONLOCATION_H
