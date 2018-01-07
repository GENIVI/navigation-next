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
    @file     NavApiListeners.h
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __NAVAPILISTENERS_H__
#define __NAVAPILISTENERS_H__

#include "NavApiTypes.h"
#include "NavApiSpeedLimitInformation.h"
#include "NavApiLaneInformation.h"
#include "NavApiRouteInformation.h"
#include "NavApiSpecialRegionInformation.h"
#include "smartpointer.h"

namespace nbnav
{

typedef enum
{
    NETP_None                     = 0,
    NETP_RoadSignEnable           = 1<<0,
    NETP_RoadSignDisable          = 1<<1,
    NETP_SpeedLimitEnable         = 1<<2,
    NETP_SpeedLimitDisable        = 1<<3,
    NETP_LaneInformationEnable    = 1<<4,
    NETP_LaneInformationDisable   = 1<<5,
    NETP_TrafficEventNotification = 1<<6,
    NETP_TrafficUpdate            = 1<<7,
    NETP_TrafficEventDisable      = 1<<8
} NavEventTypeMask;

/*! All listeners are executed in special NavKit thread

    Client needs to reschedule all listeners in client's thread if neccessary
*/

/*! RoutePositionListener

    Provides the route position related notifications like current road name and trip remaining distance
*/
class RoutePositionListener
{
public:

    /*! Provides updates to trip remaining time in seconds

        @param time
        @return None
    */
    virtual void TripRemainingTime(int /*time*/) {}

    /*! Provides updates to trip remaining delay in seconds

        @param delay
        @return None
    */
    virtual void TripRemainingDelay(int /*delay*/) {}

    /*! Provides updates to trip remaining distance in meters

        @param distance
        @return None
    */
    virtual void TripRemainingDistance(double /*distance*/) {}

    /*! Position on route has been changed.

        In case the user position is off route it equals to heading from original  {@link NBI_Location}

        @param coordinates {@link Coordinates} - adjusted user coordinates. This coordinates
               differ from latitude/longitude from original {@link NBI_Location}. It's tied to the route line.
        @param speed from the original {@link NBI_Location}
        @param heading - adjusted avatar heading. It always points to route directions when the user is on route.
        @return None
    */
    virtual void PositionUpdated(const Coordinates& /*coordinates*/, double /*speed*/, double /*heading*/) {}

    /*! Provides updates to maneuver remaining time in seconds

        @param time
        @return None
    */
    virtual void ManeuverRemainingTime(int /*time*/) {}

    /*! Provides updates to maneuver remaining delay in seconds

        @param delay
        @return None
    */
    virtual void ManeuverRemainingDelay(int /*delay*/) {}

    /*! Provides updates to maneuver remaining distance in meters

       @param distance
       @return None
    */
    virtual void ManeuverRemainingDistance(double /*distance*/) {}

    /*! Provides updates to current road name

        @param name
        @param secondaryName
        @return None
    */
    virtual void CurrentRoadName(std::string /*primaryName*/, std::string /*secondaryName*/) {}

    /*! Provides updates to current road name

        @param exitNumber
        @return None
    */
    virtual void ManeuverExitNumber(std::string /*exitNumber*/) {}

    /*! Provides updates to next road name

        @param name
        @param secondaryName
        @return None
    */
    virtual void NextRoadName(std::string /*primaryName*/, std::string /*secondaryName*/) {}

    /*! Provides updates to current maneuver type

        @param type
        @return None
    */
    virtual void ManeuverType(std::string /*type*/) {}

    /*! Provides maneuver image ID. Use MapUtils for getting bitmap.

        @param imageId - maneuver image ID
        @return None
    */
    virtual void ManeuverImageId(std::string /*imageId*/) {}

    /*! Provides stack turn image text in TTF format.

        @param stackImageTTF - stack maneuver image TTF
        @return None
    */
    virtual void StackTurnImageTTF(std::string /*stackImageTTF*/) {}

    /*! Provides turn point coordinate

        @param point turn point coordinate
        @return None
    */
    virtual void ManeuverPoint(const Coordinates& /*point*/) {}

    /*! Updated on other trigger points. */

    /*! To notify when speed limit warning information is available.

        Client needs copy the class if it wants to work with the class
        outside of listener method

        @param speedLimitInfo
        @return None
    */
    virtual void SpeedLimit(const SpeedLimitInformation& /*speedLimitInfo*/) {}

    /*! To notify to disable speed limit display when no information is available for the given position

        @return None
    */
    virtual void DisableSpeedLimit() {}

    /*! To notify when signs as real information is available

        Client needs copy the class if it wants to work with the class
        outside of listener method

        @param signInfo
        @return None
    */
    virtual void RoadSign(const RoadSign& /*signInfo*/) {}

    /*! To notify to disable signs as real display when no information is available for the given position

        @return None
    */
    virtual void DisableRoadSign() {}

    /*! To notify when lane information is available

        @param laneInfo
        @return None
    */
    virtual void LaneInformation(const LaneInformation& /*laneInfo*/) {}

    /*! To notify when lane information is NOT available

        @return None
    */
    virtual void DisableLaneInformation() {}

    /*! To notify updating upcoming maneuver list when maneuver changed

        @param maneuvers The maneuvers which are updated.
    */
    virtual void UpdateManeuverList(const ManeuverList& /*maneuvers*/) {}

    /*! Provide update to traffic color.

        @param color Traffic color char value.
    */
    virtual void TripTrafficColor(char /*color*/) {}

    virtual ~RoutePositionListener() {}

    virtual void ReachViaPoint(Place* /*place*/) {}
};


/*! AnnouncementListener

    Provides the navigation announcement notifications
*/
class AnnouncementListener
{
public:
    /*! provides the announcement related information output stream to get audio data to be played audio text for TTS support

        Informs client that announcement needs to be played
        When you get an announcement and not completed playing or not started it yet and get new announcement
        you MUST cancel playing because the previous announcement becomes invalid

        @param announcement
        @return None
    */
    virtual void Announce(shared_ptr<Announcement> /*announcement*/) {}

    /*! announcement Heads up
     */
    virtual void AnnounceHeadsUp() {}

    virtual ~AnnouncementListener() {}
};


/*! SessionListener

    Provides the session related notifications like route updates and errors
*/
class SessionListener
{
public:
    /*! The reason why new route is requested.*/
    typedef enum
    {
        Calculate = 0,
        Recalculate,
        RouteSelector,
        Detour,
        TrafficUpdate
    } RouteRequestReason;

    /*! Informs the client that the locations being received by the
        navigation session are not considered on route to destination.

        @return None
    */
    virtual void OffRoute() {}

    /*! Informs the client that the locations being received by the
        navigation session are considered on route to destination.

        @return None
    */
    virtual void OnRoute() {}

    /*! Provides the list of possible routes for the current navigation session.

        More than one route will be passed only if the appropriate multiple routes preference is enabled.
        The default route has zero index.
        Client needs copy the vector if it wants to work with the classes
        outside of listener method

        @param reason
        @param routes array of {@link RouteInformation} representing the list of possible routes.
        @return None
    */
    virtual void RouteReceived(RouteRequestReason /*reason*/, vector<RouteInformation>& /*routes*/) {}

    /*! Informs the client that new route requested

        @param reason why new route is requested.
        @return None
    */
    virtual void RouteRequested(RouteRequestReason /*reason*/) {}

    /*! Provides the route generation progress

        @param progress
        @return None
    */
    virtual void RouteProgress(int /*progress*/) {}

    /*! Indicates that an error happened and this navigation session is no longer valid.

        Need to recreate a new one to continue the navigation session.

        @return None
    */
    virtual void RouteError(NavigateRouteError /*error*/) {}

    /*! Informs the client that current navigation arriving to destination.

       @param streetSide The enum value of destination side.
       @return None
    */
    virtual void ArrivingToDestination(DestinationStreetSide /*streetSide*/) {}

    /*! Informs the client that current navigation session ends. The client will not
        receive any route updates.

       This navigation session needs to be stopped by the client.

       @return None
    */
    virtual void RouteFinish() {}

    /*! Informs the heading to origin when startup off route.

        @param headingToRoute The heading of current place to origin.
        @return None
    */
    virtual void OffRoutePositionUpdate(double /*headingToRoute*/) {}

    virtual ~SessionListener() {}
};

/*! TrafficListener

    Provides the traffic related notifications like upcoming traffic events and congestions
*/
class TrafficListener
{
public:
    /*! Called when a traffic event is detected within a specified distance (default is 10km).

        Client needs copy the class if it wants to work with the class
        outside of listener method

        @param trafficInfo {@link TrafficInformation} - new traffic information.
        @return None
    */
    virtual void TrafficAlerted(const TrafficEvent& /*trafficEvent*/) {}

    /*! Called when traffic information is updated

        Client needs copy the class if it wants to work with the class
        outside of listener method

        @param trafficInfo {@link TrafficInformation} - new traffic information.
        @return None
    */
    virtual void TrafficChanged(const TrafficInformation& /*trafficInfo*/) {}

    /*! Disable the traffic alert.
    */
    virtual void DisableTrafficAlerted() {}

    virtual ~TrafficListener() {}
};


/*! PositionUpdateListener*/
class PositionUpdateListener
{
public:
    /*! Bundled updates for position on route

        @param maneuverRemainingDelay maneuver remaining delay in seconds
        @param maneuverRemainingDistance maneuver remaining distance in meters
        @param maneuverRemainingTime maneuver remaining time in seconds
        @param coordinates {@link Coordinates} - adjusted user coordinates.
               This coordinates differ from latitude/longitude from original {@link Location}.
               It's tied to the route line.
        @param speed from the original {@link Location}
        @param heading adjusted avatar heading. It always points to route directions when the user is on route.
        @param tripRemainingDelay trip remaining delay in minutes
        @param tripRemainingDistance trip remaining distance in meters
        @param tripRemainingTime trip remaining time in seconds
        @return None
    */
    virtual void PositionUpdate(int /*maneuverRemainingDelay*/, double /*maneuverRemainingDistance*/,
            int /*maneuverRemainingTime*/, Coordinates /*coordinates*/,
            int /*speed*/, int /*heading*/, int /*tripRemainingDelay*/,
            double /*tripRemainingDistance*/, int /*tripRemainingTime*/) {}
    virtual ~PositionUpdateListener() {}
};

/*! ManeuverUpdateListener*/
class ManeuverUpdateListener
{
public:
    /*! Bundled updates for maneuvers on route

        @param maneuverID The maneuver ID indicate which maneuver these message is effective on.
        @param currentRoadPrimaryName primary name for current road
        @param currentRoadSecondaryName secondary name for current road
        @param maneuverExitNumber current maneuver exit number
        @param maneuverImageId maneuver image ID. Use MapUtils for getting bitmap.
        @param maneuverPoint turn point coordinate
        @param maneuverType current maneuver type
        @param nextRoadPrimaryName primary name for next road
        @param nextRoadSecondaryName secondary name for next road
        @param maneuverStackTurnImageTTF stack turn image text in TTF format
        @return None
    */
    virtual void ManeuverUpdate(int /*manueverID*/,
            std::string /*currentRoadPrimaryName*/, std::string /*currentRoadSecondaryName*/,
            std::string /*maneuverExitNumber*/, std::string /*maneuverImageId*/,
            Coordinates /*maneuverPoint*/, std::string /*maneuverType*/,
            std::string /*nextRoadPrimaryName*/, std::string /*nextRoadSecondaryName*/,
            std::string /*maneuverStackTurnImageTTF*/) {}

    virtual ~ManeuverUpdateListener() {}
};

/*! NavEventListener*/
class NavEventListener
{
public:

    /*! Bundled updates for navigation events

        @param navEventTypeMask The navEventTypeMask parameter specifies the following bit masks:
               <ul>
                <li>roadSignEnable (if present then information is in signInfo parameter)
                <li>roadSignDisable
                <li>speedLimitEnable (if present then information is in speedLimitInfo parameter)
                <li>speedLimitDisable
                <li>laneInformationEnable (if present then information is in laneInfo parameter)
                <li>laneInformationDisable
                <li>trafficEventNotification (if present then information is in trafficEvent parameter)
                <li>trafficUpdate (if present then information is in trafficInfo parameter)
               </ul>
        @param laneInfo provide lane information
        @param signInfo provide signs as real information
        @param speedLimitInfo provide speed limit warning information
        @param trafficEvent provide traffic event
        @param trafficInfo provide traffic information
        @return None
    */
    virtual void NavEvent(NavEventTypeMask /*navEventTypeMask*/, const LaneInformation& /*laneInfo*/,
            const RoadSign& /*signInfo*/, const SpeedLimitInformation& /*speedLimitInfo*/,
            const TrafficEvent& /*trafficEvent*/, const TrafficInformation& /*trafficInfo*/) {}
    virtual ~NavEventListener() {}
};

/*! Provides the special region related notifications.
 */
class SpecialRegionListener
{
public:
    /*! Run into a new special region.

        @param specialRegionInfo The special region information instance
     */
    virtual void SpecialRegion(const SpecialRegionInformation& /*specialRegionInfo*/) {}

    /*! Run out off one special region.

        @param specialRegionInfo The special region information instance.
     */
    virtual void DisableSpecialRegion(const SpecialRegionInformation& /*specialRegionInfo*/) {}
    virtual ~SpecialRegionListener(){}
};

/*! Provides the enhanced navigation startup notifications.
 */
class EnhancedNavigationStartupListener
{
public:
    /*! This method shall get called when enters enhanced startup mode,
        so the host application can display the route(s) appropriately

        @param coordinate Last non-GPS fix received before
     */
    virtual void EnterEnhancedNavigationStartup(Coordinates /*coordinate*/) {}

    /*! This method shall get called when during enhanced startup mode, receives a valid GPS fix.
        Following this call, regular navigation events will be sent such as onRoute/offRoute
        and positionUpdate.
     */
    virtual void ExitEnhancedNavigationStartup() {}
};
}
#endif

/*! @} */
