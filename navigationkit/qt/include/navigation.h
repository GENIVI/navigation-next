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
    @file navigation.h
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
#ifndef LOCATIONTOOLKIT_NAVIGATION_H
#define LOCATIONTOOLKIT_NAVIGATION_H

#include <QtGlobal>
#include <QList>
#include <QString>
#include "place.h"
#include "ltkcontext.h"
#include "trafficsignals.h"
#include "specialregionsignals.h"
#include "sessionsignals.h"
#include "navigationupdatesignals.h"
#include "announcementsignals.h"
#include "enhancednavigationstartupsignals.h"
#include "routeoptions.h"
#include "preferences.h"
#include "routeinformation.h"
#include "location.h"
#include "data/waypoint.h"
namespace locationtoolkit
{
/**
 * Navigation session
 */
class Navigation
{
public:
    virtual ~Navigation() {}
    /**
     * Navigation session factory
     * @param ltkContext Application context, must be created by client and pass here.
     * @param destination Destination.
     * @param routeOptions Specifies the various settings which control the route generated.
     * @param preferences Specifies the various settings which control the navigation engine.
     * @return Instance of Navigation session.
     */
    static Navigation* CreateNavigation( LTKContext& ltkContext, const Place& destination,
                                     const RouteOptions& routeOptions, const Preferences& preferences);

    /**
     * Navigation session factory
     * @param ltkContext Application context, must be created by client and pass here.
     * @param destination Destination.
     * @param routeOptions Specifies the various settings which control the route generated.
     * @param preferences Specifies the various settings which control the navigation engine.
     * @param wayPoints Specifies the way pionts in this navigation.
     * @return Instance of Navigation session.
     */
    static Navigation* CreateNavigation( LTKContext& ltkContext, const Place& destination,
                                     const RouteOptions& routeOptions, const Preferences& preferences,
                                     const QVector<WayPoint>& wayPoints);

    /*!
     * Create a Navigation instance works under given workFolder.
     *
     * @param ltkContext - Application context, must be created by client and pass here.
     * @param destination - Destination
     * @param routeOptions - Specifies the various settings which control the route generated.
     * @param preferences - Specifies the various settings which control the navigation engine
     * @param workFolder - Work folder of the fileset of nav
     *
     * @return Pointer of created Navigation instance.
     */
    static Navigation* CreateNavigationWithWorkFoler(LTKContext& ltkContext, const Place& destination,
                                     const RouteOptions& routeOptions, const Preferences& preferences,
                                     const QString& workFolder);
    /**
     * Receives the user position and updated internal navigation state.
     * Changes state would trigger navigation events to be fired.
     * @param location Current user position.
     */
    virtual void UpdatePosition(const Location& location) = 0;

    /**
     * Pause the navigation session
     * Calling this function would stop background navigation controller operation
     * such as route request/change route option/detour/traffic probe etc....
     */
    virtual void PauseSession() = 0;

    /**
     * Resume the navigation session
     * Calling this function would resume background navigation controller operation
     * such as route request/change route option/detour/traffic probe.
     */
    virtual void ResumeSession() = 0;

    /**
     * Check the status of navigation
     * @return {@code true} if navigation is pausing, {@code false} if navigation is not pausing.
     */
    virtual bool IsPaused() const = 0;

    /**
     * Stops the navigation session and releases all the resources.
     */
    virtual void StopSession() = 0;

    /**
     * Trigger announcement for current step within current session
     */
    virtual void Announce() = 0;

    /**
     * Trigger announcement for specified maneuver.
     * @param manueverNumber
     */
    virtual void Announce(qint32 manueverNumber) = 0;

    /**
     * Trigger announcement for specified maneuver of specified route.
     * @param manueverNumber
     * @param route
     */
    virtual void Announce(qint32 manueverNumber, const RouteInformation& route) = 0;

    /**
     * Calculates detours
     */
    virtual void DoDetour() = 0;


    /**
     * Cancel detours request
     */
    virtual void CancelDetour() = 0;

    /**
     * Sets active route for the current navigation session.
     * @param route new active route. Should be one of the {@link RouteInformation} objects emitted by {@link SessionSignals#RouteReceived} signal.
     * @return {@code true} if success, {@code false} if failed.
     */
    virtual bool SetActiveRoute(const RouteInformation& route) const = 0;

    /**
     * Recalculates the route
     */
    virtual void Recalculate() = 0;

    /**
     * Recalculates the route with RouteOptions,Preferences, wantAlternateRoute,update
     * @param routeOptions recalc RouteOptions
     * @param preference recalc Preferences
     * @param wantAlternateRoute true for multi-route, false for single route.
     * @return
     */
    virtual void Recalculate(const RouteOptions& routeOptions,const Preferences& preference,bool wantAlternateRoute) = 0;

    /**
     * Get navigation session signals for user bind to navigation session slots.
     * @return navigation session signals.
     */
    virtual const SessionSignals& GetSessionSignals() const = 0;

    /**
     * Get traffic signals for user bind to traffic slots.
     * @return traffic signals.
     */
    virtual const TrafficSignals& GetTrafficSignals() const = 0;

    /**
     * Get navigation update signals for user bind to navigation update slots.
     * @return navigation update signals.
     */
    virtual const NavigationUpdateSignals& GetNavigationUpdateSignals() const = 0;

    /**
     * Get special region signals for user bind to special region slots.
     * @return special region signals.
     */
    virtual const SpecialRegionSignals& GetSpecialRegionSignals() const = 0;

    /**
     * Get announcement signals for user bind to announcement slot.
     * @return announcement signals.
     */
    virtual const AnnouncementSignals& GetAnnouncementSignals() const = 0;

    /**
     * Get enhanced navigation startup signals for user bind to enhanced navigation startup slot.
     * @return announcement signals.
     */
    virtual const EnhancedNavigationStartupSignals& GetEnhancedNavigationStartupSignals() const = 0;

    //TODO: one more signal EnhancedNavigationStartup to be implemented

};
}  // namespace locationtoolkit
#endif
/*! @} */
