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
    @file navigationimpl.h
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
#ifndef LOCATIONTOOLKIT_NAVIGATIOIMPL_H
#define LOCATIONTOOLKIT_NAVIGATIOIMPL_H

#include "sessionhelperlistener.h"
#include "traffichelperlistener.h"
#include "announcementhelperlistener.h"
#include "specialregionhelperlistener.h"
#include "routepositionhelperlistener.h"
#include "enhancednavigationstartuphelplistener.h"
#include "navigation.h"
#include "typetranslater.h"
#include "data/waypoint.h"

namespace locationtoolkit
{


class NavigationImpl: public  Navigation
{

public:
    /**

     * navigation session constructor.
     */
    NavigationImpl();
    /**
     * navigation session destructor.

     */
    virtual ~NavigationImpl();
    /**
     * navigation session init.
     * @param ltkContext Application context, must be created by client and pass here.
     * @param destination Destination.
     * @param routeOptions Specifies the various settings which control the route generated.
     * @param preferences Specifies the various settings which control the navigation engine.
     * @param wayPoints Specifies the way pionts in this navigation.
     * @return {@code true} if navigation init succeesfully, {@code false} if navigation init failed.
     */
    bool InitializeNavigation( LTKContext& ltkContext, const Place& destination,
                               const RouteOptions& routeOptions, const Preferences& preferences,
                               const QVector<WayPoint>& wayPoints, const QString& workFolder = QString(""));

    /**
     * Receives the user position and updated internal navigation state.
     * Changes state would trigger navigation events to be fired.
     * @param location Current user position.
     */
    virtual void UpdatePosition(const Location& location);

    /**
     * Pause the navigation session
     * Calling this function would stop background navigation controller operation
     * such as route request/change route option/detour/traffic probe etc....
     */
    virtual void PauseSession();

    /**
     * Resume the navigation session
     * Calling this function would resume background navigation controller operation
     * such as route request/change route option/detour/traffic probe.
     */
    virtual void ResumeSession();

    /**
     * Check the status of navigation
     * @return {@code true} if navigation is pausing, {@code false} if navigation is not pausing.
     */
    virtual bool IsPaused() const;

    /**
     * Stops the navigation session and releases all the resources.
     */
    virtual void StopSession();

    /**
     * Trigger announcement for current step within current session
     */
    virtual void Announce();

    /**
     * Trigger announcement for specified maneuver.
     * @param manueverNumber
     */
    virtual void Announce(qint32 manueverNumber);

    /**
     * Trigger announcement for specified maneuver of specified route.
     * @param manueverNumber
     * @param route
     */
    virtual void Announce(qint32 manueverNumber, const RouteInformation& route);

    /**
     * Calculates detours
     */
    virtual void DoDetour();


    /**
     * Cancel detours request
     */
    virtual void CancelDetour();

    /**
     * Sets active route for the current navigation session.
     * @param route new active route. Should be one of the {@link RouteInformation} objects emitted by {@link SessionSignals#RouteReceived} signal.
     * @return {@code true} if success, {@code false} if failed.
     */
    virtual bool SetActiveRoute(const RouteInformation& route) const;

    /**
     * Recalculates the route
     */
    virtual void Recalculate();

    /**
     * Recalculates the route with RouteOptions,Preferences, wantAlternateRoute,update
     * @param routeOptions recalc RouteOptions
     * @param preference recalc Preferences
     * @param wantAlternateRoute true for multi-route, false for single route.
     * @return
     */
    virtual void Recalculate(const RouteOptions& routeOptions,const Preferences& preference, bool wantAlternateRoute);

    /**
     * Get navigation session signals for user bind to navigation session slots.
     * @return navigation session signals.
     */
    virtual const SessionSignals& GetSessionSignals() const;

    /**
     * Get traffic signals for user bind to traffic slots.
     * @return traffic signals.
     */
    virtual const TrafficSignals& GetTrafficSignals() const;

    /**
     * Get navigation update signals for user bind to navigation update slots.
     * @return navigation update signals.
     */
    virtual const NavigationUpdateSignals& GetNavigationUpdateSignals() const;


    /**
     * Get special region signals for user bind to special region slots.
     * @return special region signals.
     */
    virtual const SpecialRegionSignals& GetSpecialRegionSignals() const;

    /**
     * Get announcement signals for user bind to announcement slot.
     * @return announcement signals.
     */
    virtual const AnnouncementSignals& GetAnnouncementSignals() const;

    /**
     * Get enhanced navigation startup signals for user bind to enhanced navigation startup slot.
     * @return announcement signals.
     */
    virtual const EnhancedNavigationStartupSignals& GetEnhancedNavigationStartupSignals() const;

private:
    /**
     * Remove all added listeners.

     * @return .
     */
    void RemoveAllListeners();


private:
    nbnav::Navigation* mNbNavNavigation;
    Place mDestPlace;
    RouteOptions mRouteOptions;
    Preferences mPreferences;
    SessionHelperListener mSessionHelperListener;
    RoutePositionHelperListener mRoutePositionHelperListener;
    TrafficHelperListener mTrafficHelperListener;
    SpecialRegionHelperListener mSpecialRegionHelperListener;
    AnnouncementHelperListener mAnnouncementHelperListener;
    EnhancedNavigationStartupListener mEnhancedNavigationStartupListener;
    bool mbNavigaitonStart;
    bool mbNavigaitonPaused;

};

}

#endif //LOCATIONTOOLKIT_NAVIGATIOIMPL_H
