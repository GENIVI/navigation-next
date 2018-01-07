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
    @file     NavApiNavigation.h
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

#ifndef __NAVAPINAVIGATION_H__
#define __NAVAPINAVIGATION_H__

#include "NavApiTypes.h"
#include "NavApiRouteOptions.h"
#include "NavApiPreferences.h"
#include "NavApiListeners.h"
#include <vector>
extern "C"
{
#include "nbcontext.h"
}

namespace nbnav
{

class NavigationImpl;

/*! Navigation session

    Thread safe class
    All listeners are executed in UI thread
    Client needs to reschedule all listeners in client's thread if neccessary
*/
class Navigation
{
public:
    /*! Navigation session factory

        @param nbContext NB context, must be created by client and pass here
        @param destination Destination
        @param routeOptions Specifies the various settings which control the route generated.
        @param preferences Specifies the various settings which control the navigation engine
        @return pointer to Navigation
    */
    static Navigation* GetNavigation(NB_Context* nbContext, const Place& destination,
                                     const RouteOptions& routeOptions, const Preferences& preferences);

    /*!
     * Get a Navigation instance works under given workFolder.
     *
     * @param nbContext - NB context, must be created by client and pass here
     * @param destination - Destination
     * @param routeOptions - Specifies the various settings which control the route generated.
     * @param preferences - Specifies the various settings which control the navigation engine
     * @param workFolder - Work folder of the fileset of nav
     * @param viaPoint - array of via point
     *
     * @return Pointer of created Navigation instance.
     */
    static Navigation*
    GetNavigationWithWorkFoler(NB_Context* nbContext,
                               const Place& destination,
                               const RouteOptions& routeOptions,
                               const Preferences& preferences,
                               const string& workFolder,
                               const vector<WayPoint> viaPoint);

    /*! Receives the user position and provides session and position update events

        @param location Current user position
        @return None
    */
    void UpdatePosition(const Location& location);

    /*! Pause the navigation session

        @return None
    */
    void PauseSession();

    /*! Resume the navigation session

        @return None
    */
    void ResumeSession();

    /*! Stops the navigation session and releases all the resources.

        @return None
    */
    void StopSession();

    /*! Adds session listener to receive notifications the route information is updated

        @param listener Pointer to listener. This class does not take ownership of the object.
        @return None
    */
    void AddSessionListener(SessionListener* listener);

    /*! Removes session listener if already added

        @param listener Pointer to listener.
        @return None
    */
    void RemoveSessionListener(SessionListener* listener);

    /*! Adds route position listener to receive notifications on changes in the position
        with respect to the entire route as well as the current maneuver

        @param listener Pointer to listener. This class does not take ownership of the object.
        @return None
    */
    void AddRoutePositionListener(RoutePositionListener* listener);

    /*! Removes the route position listener if already added

        @param listener Pointer to listener.
        @return None
    */
    void RemoveRoutePositionListener(RoutePositionListener* listener);

    /*! Adds announcement listener to receive notifications when some announcement needs to be played

        @param listener Pointer to listener. This class does not take ownership of the object.
        @return None
    */
    void AddAnnouncementListener(AnnouncementListener* listener);

    /*! Removes announcement listener if already added

        @param listener Pointer to listener.
        @return None
    */
    void RemoveAnnouncementListener(AnnouncementListener* listener);

    /*! Adds traffic listener to receive notifications the traffic information is updated

        @param listener Pointer to listener. This class does not take ownership of the object.
        @return None
    */
    void AddTrafficListener(TrafficListener* listener);

    /*! Removes traffic listener if already added

        @param listener Pointer to listener.
        @return None
    */
    void RemoveTrafficListener(TrafficListener* listener);

    /*! Adds position update listener to receive events generated whenever the position changes

        @param listener
        @return None
    */
    void AddPositionUpdateListener(PositionUpdateListener* listener);

    /*! Removes the position update listener if already added

        @param listener
        @return None
    */
    void RemovePositionUpdateListener(PositionUpdateListener* listener);

    /*! Adds maneuver update listener to receive events associated with a maneuver change.

        @param listener
        @return None
    */
    void AddManeuverUpdateListener(ManeuverUpdateListener* listener);

    /*! Removes the maneuver update listener if already added

        @param listener
        @return None
    */
    void RemoveManeuverUpdateListener(ManeuverUpdateListener* listener);

    /*! Adds navigation event listener to receive events associated with the navigation session
        that are not directly associated with position or maneuver updates.
        The listener contains a navEventTypeMask parameter that is used to specify which
        event parameters actually include valid data.

        @param listener
        @return None
    */
    void AddNavEventListener(NavEventListener* listener);

    /*! Removes the navigation event listener if already added

        @param listener
        @return None
    */
    void RemoveNavEventListener(NavEventListener* listener);

     /*! Adds special region listener.

        Adds special region listener to receive special region data.
        @param listener
     */
    void AddSpecialRegionListener(SpecialRegionListener* listener);

    /*! Removes special region listener

        Removes the special region listener if already added
        @param listener
     */
    void RemoveSpecialRegionListener(SpecialRegionListener* listener);

    /*! Adds enhanced navigation startup listener

        @param listener
     */
    void AddEnhancedNavigationStartupListener(EnhancedNavigationStartupListener* listener);

    /*! Removes enhanced navigation startup listener

        Removes the enhanced navigation startup listener if already added
        @param listener
     */
    void RemoveEnhancedNavigationStartupListener(EnhancedNavigationStartupListener* listener);

    /*! Trigger announcement for current step within current session

        @return None
    */
    void Announce();

    /*! Trigger announcement for specified maneuver.

        @param manueverNumber
        @return None
    */
    void Announce(int manueverNumber);

    /*! Trigger announcement for specified maneuver of specified route.

        @param manueverNumber
        @param route
        @return None
    */
    void Announce(int manueverNumber, const RouteInformation& route);

    /*! Calculates detours.

        @return None
    */
    void DoDetour();

    /*! Cancel detour request. */
    void CancelDetour();

    /*! Sets active route for the current navigation session.

        @param route new active route.
               Should be one of the {@link RouteInformation} objects received
               from {@link SessionListener#routeReceived} callback.
        @return {@code true} if success, {@code false} if failed.
    */
    bool SetActiveRoute(const RouteInformation& route);

    /*! Recalculates the route

        @return None
    */
    void Recalculate();

    /*! Recalculates the route with RouteOptions update

        @param routeOptions the new route option.
        @param preference the new preference.
        @param wantAlternateRoute true for multi-route, false for single route.
        @return None
     */
    void Recalculate(const RouteOptions& routeOptions, const Preferences& preference,
                     bool wantAlternateRoute);
    
    /*! Recalculates the route with Waypoint
     @return None
     */
    void Recalculate(const vector<WayPoint>& wayPoint);


    /*! Start plan trip, this is an internal interface.

        @param originPlace the origin place of trip.
        @return None
     */
    void PlanTrip(const Place& originPlace);

    /*! Destructor.*/
    ~Navigation();

private:
    NavigationImpl* m_impl;

    friend class RouteRequest;
};

}

#endif

/*! @} */
