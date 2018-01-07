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
    @file routepositionhelperlistener.h
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
#ifndef LOCATIONTOOLKIT_ROUTE_POSITION_HELPER_LISTENER_H
#define LOCATIONTOOLKIT_ROUTE_POSITION_HELPER_LISTENER_H

#include <QObject>
#include <string>

#include "signal/navigationupdatesignals.h"
#include "NavApiListeners.h"

namespace locationtoolkit
{
class RoutePositionHelperListener: public QObject, public nbnav::RoutePositionListener
{
    Q_OBJECT
public:
    explicit RoutePositionHelperListener(QObject* parent = 0);

    ~RoutePositionHelperListener();

    virtual void TripRemainingTime(int time);

    virtual void TripRemainingDelay(int delay);

    virtual void TripRemainingDistance(double distance);

    virtual void PositionUpdated(const nbnav::Coordinates& coordinates, double speed, double heading);

    virtual void ManeuverRemainingTime(int time);

    virtual void ManeuverRemainingDelay(int delay);

    virtual void ManeuverRemainingDistance(double distance);

    virtual void CurrentRoadName(std::string primaryName, std::string secondaryName);

    virtual void ManeuverExitNumber(std::string exitNumber);

    virtual void NextRoadName(std::string primaryName, std::string secondaryName);

    virtual void ManeuverType(std::string type);

    virtual void ManeuverImageId(std::string imageId);

    virtual void StackTurnImageTTF(std::string stackImageTTF);

    virtual void ManeuverPoint(const nbnav::Coordinates& point);

    virtual void SpeedLimit(const nbnav::SpeedLimitInformation& speedLimitInfo);

    virtual void DisableSpeedLimit();

    virtual void RoadSign(const nbnav::RoadSign& signInfo);

    virtual void DisableRoadSign();

    virtual void LaneInformation(const nbnav::LaneInformation& LaneInformation);

    virtual void DisableLaneInformation();

    virtual void UpdateManeuverList(const nbnav::ManeuverList& maneuvers);

    virtual void TripTrafficColor(char color);

    virtual void ReachViaPoint(nbnav::Place *place);

    const NavigationUpdateSignals& GetNavigationUpdateSignals() const;

private:
    NavigationUpdateSignals mNavigationUpdateSignals;
};
}  // namespace locationtoolkit
#endif
/*! @} */
