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

    @file     location.h
    @date     07/29/2014
    @defgroup locationkit Locationkit

    Location definition
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
#ifndef LOCATION_H
#define LOCATION_H
#include <qglobal.h>

namespace locationtoolkit
{
struct Location
{
    enum LocationFixType
    {
        LFT_Fast = 0, /*!< Use Cell ID / Wifi or sGPS (if GPS is warm) */
        LFT_Normal,   /*!< Use Cell ID / Wifi or sGPS with short timeout */
        LFT_Accurate  /*!< Force sGPS fix with long timeout */
    };

    qint32 status;                                 /*!< Response status. */
    quint64 gpsTime;                               /*!< Time, seconds since the GPS Epoch: Midnight, January 6, 1980 */
    quint32 valid;                                 /*!< NB_GpsValid Flags indicating valid fields in the struct. */
    double latitude;                                /*!< Latitude in Degrees, WGS-84 ellipsoid */
    double longitude;                               /*!< Longitude in Degrees, WGS-84 ellipsoid */
    double heading;                                 /*!< Heading in degrees */
    qreal horizontalVelocity;                      /*!< Horizontal velocity in meters/second */
    qreal altitude;                                /*!< Altitude in meters, WGS-84 ellipsoid */
    qreal verticalVelocity;                        /*!< Vertical velocity in meters/second */
    qreal horizontalUncertaintyAngleOfAxis;        /*!< Heading in degrees of the axis of uncertainty */
    qreal horizontalUncertaintyAlongAxis;          /*!< Standard Deviation of horizontal uncertainty along the axis of uncertainty */
    qreal horizontalUncertaintyAlongPerpendicular; /*!< Standard Deviation of horizontal uncertainty perpendicular to the axis of uncertainty */
    qreal verticalUncertainty;                     /*!< Standard Deviation of vertical uncertainty */
    qint16  utcOffset;                             /*!< difference in seconds between GPS time and UTC time (leap seconds) */
    quint16 numberOfSatellites;                    /*!< Number of satellites acquired. */
    qreal gpsHeading;                              /*!< GPS Heading in degrees. Shall be used in Debug UI perpose only */
    qreal compassHeading;                          /*!< Compass Heading in degrees. Shall be used in Debug UI perpose only */
    qreal compassAccuracy;                         /*!< Compass Heading Accuracy. Shall be used in Debug UI perpose only */
};
}

#endif

/*! @} */
