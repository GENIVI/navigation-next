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
    @file trafficinformation.h
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
#ifndef LOCATIONTOOLKIT_TRAFFIC_EVENT_H
#define LOCATIONTOOLKIT_TRAFFIC_EVENT_H

#include <QtGlobal>
#include <QString>
#include <QTime>

namespace locationtoolkit
{
class TrafficEvent
{
public:
    /**
     * Traffic severity.
     */
    typedef enum
    {
        /**
         * Unknown severity.
         */
        TS_Unknown = 0,
        /**
         * Moderate severity.
         */
        TS_Moderate,
        /**
         * Severe severity.
         */
        TS_Severe

    } TrafficSeverity;

    typedef enum
    {
        TP_Incident = 0,
        TP_Congestion

    } Type;

public:
    virtual ~TrafficEvent(){}

    /**
     * Returns Returns the delay in seconds due to this traffic event
     *
     * @return delay
     */
    virtual quint32 GetDelay() const = 0;

    /**
     * Returns a description of the traffic event
     *
     * @return description.
     */
    virtual const QString& GetDescription() const = 0;

    /**
     * Returns the expected end time for this traffic event.
     *
     * @return time.
     */
    virtual const QTime& GetEndTime() const = 0;

    /**
     * Returns current distance to traffic event.
     *
     * @return current distance to traffic event.
     */
    virtual qreal GetDistanceToTrafficItem() const = 0;

    /**
     * Returns the total length of the congestion
     *
     * @return total length.
     */
    virtual qreal GetLength() const = 0;

    /**
     * Returns index of maneuver traffic event is within.
     *
     * @return index of maneuver.
     */
    virtual quint32 GetManeuverNumber() const = 0;

    /**
     * Returns the time at which the event was reported.
     *
     * @return time.
     */
    virtual const QTime& GetReportTime() const = 0;

    /**
     * Returns name of the road where the traffic event.
     *
     * @return name of the road.
     */
    virtual const QString& GetRoadName() const = 0;

    /**
     * Returns the severity of the traffic Severity (TS_Unknown/TS_Moderate/TS_Severe).
     *
     * @return severity.
     */
    virtual TrafficEvent::TrafficSeverity GetSeverity() const = 0;

    /**
     * Returns the traffic event start position measured from the start of the trip.
     *
     * @return start position.
     */
    virtual qreal GetStartFromTrip() const = 0;

    /**
     * Returns the type of traffic event (TP_Incident or TP_Congestion).
     *
     * @return type of traffic event.
     */
    virtual TrafficEvent::Type GetType() const = 0;
};
}  // namespace locationtoolkit
#endif
/*! @} */
