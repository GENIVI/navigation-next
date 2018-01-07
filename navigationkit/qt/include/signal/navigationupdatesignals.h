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
    @file navigationupdatesignals.h
    @date 09/22/2014
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
#ifndef LOCATIONTOOLKIT_NAVIGATION_UPDATE_SIGNALS_H
#define LOCATIONTOOLKIT_NAVIGATION_UPDATE_SIGNALS_H

#include <QtGlobal>
#include <QObject>
#include <QString>

#include "coordinate.h"
#include "maneuverlist.h"
#include "laneinformation.h"
#include "roadsign.h"
#include "speedlimitinformation.h"

namespace locationtoolkit
{
class Coordinates;

/**
 * Provides the navigation related notifications like current road name and trip remaining distance
 */
class NavigationUpdateSignals: public QObject
{
    Q_OBJECT
public:
    ~NavigationUpdateSignals(){}
Q_SIGNALS:
    /**
     * Emit when trip remaining time updated
     * @param time time in seconds
     */
    void TripRemainingTime(quint32 time);

    /**
     * Emit when trip remaining delay updated
     * @param delay time in seconds
     */
    void TripRemainingDelay(quint32 time);

    /**
     * Emit when trip remaining distance updated
     * @param distance meters
     */
    void TripRemainingDistance(const qreal& distance);

    /**
     * Emit when the name of current road updated.
     * @param primaryName The primary name of the current road.
     * @param secondaryName The secondary name of current road.
     */
    void CurrentRoadName(const QString& primaryName, const QString& secondaryName);

    /**
     * Emit when the exit number of current maneuver updated.
     * @param exitNumber exit number of maneuver
     */
    void ManeuverExitNumber(const QString& exitNumber);

    /**
     * Emit when the name of next road updated.
     * @param primaryName The primary name of next road.
     * @param secondaryName The secondary name of next road.
     */
    void NextRoadName(const QString& primaryName, const QString& secondaryName);

    /**
     * Emit when the type of current updated.
     * @param type Maneuver code
     */
    void ManeuverType(const QString& type);

    /**
     * Emit when remaining time of maneuver updated.
     * @param time Time in seconds
     */
    void ManeuverRemainingTime(quint32 time);

    /**
     * Emit when the remaining time delay of current maneuver updated.
     * @param delay The remaining time delay in seconds.
     */
    void ManeuverRemainingDelay(quint32 delay);

    /**
     * Emit when the remaining distance of the current maneuver updated.
     * @param distance The remaining distance in meters.
     */
    void ManeuverRemainingDistance(const qreal& distance);

    /**
     * Emit when maneuver image ID returned. Use MapUtils for getting bitmap.
     * @param imageId  Maneuver image ID
     */
    void ManeuverImageId(const QString& imageId);

    /**
     * Emit when the stack maneuver of turn image text returned.
     * @param stackImageTTF Stack maneuver image TTF
     */
    void StackTurnImageTTF(const QString& stackImageTTF);

    /**
     * Emit when position on route has been changed.
     * @param coordinates {@link Coordinates} - adjusted user coordinates. This coordinates differ from latitude/longitude from original {@link Location}. It's tied to the route line.
     * @param speed from the original {@link Location}
     * @param heading - adjusted avatar heading. It always points to route directions when the user is on route.
     * In case the user position is off route it equals to heading from original  {@link Location}
     */
    void PositionUpdated(const Coordinates& coordinates, qint32 speed, qint32 heading);

    /**
     * Emit when the turn point coordinate of maneuver changed.
     * @param point The turn point coordinate of maneuver.
     */
    void ManeuverPoint(const Coordinates& point);

    /**
     * Emit when updating upcoming maneuver list when maneuver changed.
     */
    void UpdateManeuverList(const ManeuverList& maneuvers);

    /**
     * Emit when reach the via point.
     */
    void ReachViaPoint(const Place& place);

    /**
     * Provide update to traffic color.
     * @param color Traffic color char value.
    */
    void TripTrafficColor(char color);

    /**
     * Emit when lane information is available.
     * @param laneInfo Lane information {@link LaneInformation}
     */
    void LaneInformation(const locationtoolkit::LaneInformation& laneInfo);

    /**
     * Emit when lane information is NOT available.
     */
    void DisableLaneInformation();

    /**
     * Emit when signs as real information is available
     * @param signInfo Road sign information {@link RoadSignInformation}
     */
    void RoadSign(const locationtoolkit::RoadSign& roadSign);

    /**
     * Emit when disable signs as real display when no information is available for the given position
     */
    void DisableRoadSign();

    /**
     * Emit when speed limit warning information is available
     * @param speedLimitInfo speed limit information
     */
    void SpeedLimit(const SpeedLimitInformation& speedLimitInfo);

    /**
     * Emit when disable speed limit display when no information is available for the given position
     */
    void DisableSpeedLimit();
private:
    explicit NavigationUpdateSignals(QObject* parent): QObject(parent){}
    friend class RoutePositionHelperListener;
};
}  // namespace locationtoolkit
#endif
/*! @} */
