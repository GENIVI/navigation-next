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

#ifndef GEOLOCATION_H
#define GEOLOCATION_H
#include <QObject>

#define GETSET(type, var, default_val) \
    private: \
       type m_##var = default_val; \
    public: \
       type get##var() \
       {\
          return m_##var; \
       }\
       void set##var(type val) \
       {\
          m_##var = val; \
       }

class GeoLocation
{

public:
    GETSET(qint32, Status, 0)                                   /*!< Response status. */
    GETSET(quint64, GpsTime, 0)                                 /*!< Time, seconds since the GPS Epoch: Midnight, January 6, 1980 */
    GETSET(quint32, Valid, 0)                                   /*!< NB_GpsValid Flags indicating valid fields in the struct. */
    GETSET(double, Latitude, 0.0)                               /*!< Latitude in Degrees, WGS-84 ellipsoid */
    GETSET(double, Longitude, 0.0)                              /*!< Longitude in Degrees, WGS-84 ellipsoid */
    GETSET(double, Heading, 0.0)                                /*!< Heading in degrees */
    GETSET(qreal, HorizontalVelocity, 0.0)                      /*!< Horizontal velocity in meters/second */
    GETSET(qreal, Altitude, 0.0)                                /*!< Altitude in meters, WGS-84 ellipsoid */
    GETSET(qreal, VerticalVelocity, 0.0)                        /*!< Vertical velocity in meters/second */
    GETSET(qreal, HorizontalUncertaintyAngleOfAxis, 0.0)        /*!< Heading in degrees of the axis of uncertainty */
    GETSET(qreal, HorizontalUncertaintyAlongAxis, 0.0)          /*!< Standard Deviation of horizontal uncertainty along the axis of uncertainty */
    GETSET(qreal, HorizontalUncertaintyAlongPerpendicular, 0.0) /*!< Standard Deviation of horizontal uncertainty perpendicular to the axis of uncertainty */
    GETSET(qreal, VerticalUncertainty, 0.0)                     /*!< Standard Deviation of vertical uncertainty */
    GETSET(qint16,  UtcOffset, 0)                               /*!< difference in seconds between GPS time and UTC time (leap seconds) */
    GETSET(quint16, NumberOfSatellites, 0)                      /*!< Number of satellites acquired. */
    GETSET(qreal, GpsHeading, 0.0)                              /*!< GPS Heading in degrees. Shall be used in Debug UI perpose only */
    GETSET(qreal, CompassHeading, 0.0)                          /*!< Compass Heading in degrees. Shall be used in Debug UI perpose only */
    GETSET(qreal, CompassAccuracy, 0.0)                         /*!< Compass Heading Accuracy. Shall be used in Debug UI perpose only */

};

#endif // GEOLOCATION_H
