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

 @file routepositionhelperlistener.cpp
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
 *
 */
#include "internal/routepositionhelperlistener.h"
#include "internal/laneinformationimpl.h"
#include "internal/maneuverlistimp.h"
#include "internal/speedlimitinformationimpl.h"
#include "internal/roadsignimpl.h"
#include "internal/typetranslater.h"

namespace locationtoolkit
{
RoutePositionHelperListener::RoutePositionHelperListener(QObject* parent): QObject(parent)
    , mNavigationUpdateSignals(parent)
{

}

RoutePositionHelperListener::~RoutePositionHelperListener()
{

}

void RoutePositionHelperListener::TripRemainingTime(int time)
{
    emit mNavigationUpdateSignals.TripRemainingTime((qint32)time);
}

void RoutePositionHelperListener::TripRemainingDelay(int delay)
{
    emit mNavigationUpdateSignals.TripRemainingDelay((qint32)delay);
}

void RoutePositionHelperListener::TripRemainingDistance(double distance)
{
    emit mNavigationUpdateSignals.TripRemainingDistance((qreal)distance);
}

void RoutePositionHelperListener::PositionUpdated(const nbnav::Coordinates& coordinates, double speed, double heading)
{
    double latitude = coordinates.latitude;
    double longitude = coordinates.longitude;
    Coordinates coordinate(latitude, longitude);
    emit mNavigationUpdateSignals.PositionUpdated(coordinate, (qreal)speed, (qreal)heading);
}

void RoutePositionHelperListener::ManeuverRemainingTime(int time)
{
    emit mNavigationUpdateSignals.ManeuverRemainingTime((qint32)time);
}

void RoutePositionHelperListener::ManeuverRemainingDelay(int delay)
{
    emit mNavigationUpdateSignals.ManeuverRemainingDelay((qint32)delay);
}

void RoutePositionHelperListener::ManeuverRemainingDistance(double distance)
{
    emit mNavigationUpdateSignals.ManeuverRemainingDistance((qreal)distance);
}

void RoutePositionHelperListener::CurrentRoadName(std::string primaryName, std::string secondaryName)
{
    emit mNavigationUpdateSignals.CurrentRoadName(QString::fromStdString(primaryName), QString::fromStdString(secondaryName));
}

void RoutePositionHelperListener::ManeuverExitNumber(std::string exitNumber)
{
    emit mNavigationUpdateSignals.ManeuverExitNumber(QString::fromStdString(exitNumber));
}

void RoutePositionHelperListener::NextRoadName(std::string primaryName, std::string secondaryName)
{
    emit mNavigationUpdateSignals.NextRoadName(QString::fromStdString(primaryName), QString::fromStdString(secondaryName));
}

void RoutePositionHelperListener::ManeuverType(std::string type)
{
    emit mNavigationUpdateSignals.ManeuverType(QString::fromStdString(type));
}

void RoutePositionHelperListener::ManeuverImageId(std::string imageId)
{
    emit mNavigationUpdateSignals.ManeuverImageId(QString::fromStdString(imageId));
}

void RoutePositionHelperListener::StackTurnImageTTF(std::string stackImageTTF)
{
    emit mNavigationUpdateSignals.StackTurnImageTTF(QString::fromStdString(stackImageTTF));
}

void RoutePositionHelperListener::ManeuverPoint(const nbnav::Coordinates& point)
{
    double latitude = point.latitude;
    double longitude = point.longitude;
    Coordinates coordinate(latitude, longitude);
    emit mNavigationUpdateSignals.ManeuverPoint(coordinate);
}

void RoutePositionHelperListener::SpeedLimit(const nbnav::SpeedLimitInformation& speedLimitInfo)
{
    SpeedLimitInformationImpl speedInfo(speedLimitInfo);
    emit mNavigationUpdateSignals.SpeedLimit(speedInfo);
}

void RoutePositionHelperListener::DisableSpeedLimit()
{
    emit mNavigationUpdateSignals.DisableSpeedLimit();
}

void RoutePositionHelperListener::RoadSign(const nbnav::RoadSign& roadSign)
{
    RoadSignImpl roadsign(roadSign);
    emit mNavigationUpdateSignals.RoadSign(roadsign);
}

void RoutePositionHelperListener::DisableRoadSign()
{
    emit mNavigationUpdateSignals.DisableRoadSign();
}

void RoutePositionHelperListener::LaneInformation(const nbnav::LaneInformation& LaneInformation)
{
    LaneInformationImpl laneInformation(LaneInformation);
    emit mNavigationUpdateSignals.LaneInformation(laneInformation);
}

void RoutePositionHelperListener::DisableLaneInformation()
{
    emit mNavigationUpdateSignals.DisableLaneInformation();
}

void RoutePositionHelperListener::UpdateManeuverList(const nbnav::ManeuverList& maneuvers)
{
    ManeuverListImpl maneuverList(maneuvers);
    emit mNavigationUpdateSignals.UpdateManeuverList(maneuverList);
}

void RoutePositionHelperListener::TripTrafficColor(char color)
{
    emit mNavigationUpdateSignals.TripTrafficColor(color);
}

void RoutePositionHelperListener::ReachViaPoint(nbnav::Place* nbPlace)
{
    Place place;
    TypeTranslater::Translate(*nbPlace, place);
    emit mNavigationUpdateSignals.ReachViaPoint(place);
}

const NavigationUpdateSignals& RoutePositionHelperListener::GetNavigationUpdateSignals() const
{
    return mNavigationUpdateSignals;
}

}  // namespace locationtoolkit
