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
    @file navuipreferences.h
    @date 10/24/2014
    @addtogroup navigationuikit
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
#ifndef LOCATIONTOOLKIT_NAVUIPREFERENCES_H
#define LOCATIONTOOLKIT_NAVUIPREFERENCES_H

#include <QString>


namespace locationtoolkit
{

/*! Preferences

    Navigation engine preferences, this is not thread safe.
*/
class NavUIPreferences
{
public:
    /*! This setting controls the parameters used by the navigation engine for map
        matching during the navigation session.

        There are two settings: tight and relaxed (default).
    */
    typedef enum
    {
        RDL_Relaxed = 0,  /*!< Default option. */
        RDL_Tight         /*!< If tight is selected, then the map matching is tighter,
                           resulting in faster detection of turns and off-route conditions,
                           but also more unwanted recalculations due to poor GPS fixes. */
    } RouteDetectionLevel;

    typedef enum
    {
        Metric = 0,
        NonMetric,
        NonMetricYards
    } Measurement;

public:
    /*! Constructor. */
    NavUIPreferences();
   ~NavUIPreferences();

    /*! Returns currently in use recalc criteria

        @return recalc criteria
    */
    qint32 GetRecalcCriteria() const;

    /*! Used to set the recalc criteria, Relaxed or Tight
        Defaults to Relaxed

        @param recalcCriteria
        @return None
    */
    NavUIPreferences& SetRecalcCriteria(qint32 recalcCriteria);

    /*! GetOffRouteIgnoreCount

        @return the Count
    */
    qint32 GetOffRouteIgnoreCount() const;

    /*! SetOffRouteIgnoreCount

        @param offRouteIgnoreCount
        @return None
    */
    NavUIPreferences& SetOffRouteIgnoreCount(qint32 offRouteIgnoreCount);

    /*! Returns the current setting of multiple routes preference

        @return {@code true} if enabled, {@code false} if disabled.
    */
    bool GetMultipleRoutesEnabled() const;

    /*! Enables/disables the receiving of alternate routes for the navigation session.

        The list of alternate routes can be obtained through
        {@link SessionListener#routeReceived(com.nbi.common.route.RouteInformation[])} callback.

        @param enable {@code true} to enable, {@code false} to disable.
        @return None
    */
    NavUIPreferences& SetMultipleRoutes(bool enable);

    /*! Returns the current setting of realistic sign

        @return {@code true} if enabled, {@code false} if disabled.
    */
    bool GetRealisticSignEnabled() const;

    /*! Enables/disables the receiving of realistic sign for the navigation session.

        @param enable {@code true} to enable, {@code false} to disable.
        @return None
    */
    NavUIPreferences& SetRealisticSign(bool enable);

    /*! Returns the current setting of natural guidance

        @return {@code true} if enabled, {@code false} if disabled.
    */
    bool GetNaturalGuidanceEnabled() const;

    /*! Enables/disables the receiving of natural guidance for the navigation session.

        @param enable {@code true} to enable, {@code false} to disable.
        @return None
    */
    NavUIPreferences& SetNaturalGuidance(bool enable);

    /*! Returns the current setting of lane guidance

        @return {@code true} if enabled, {@code false} if disabled.
    */
    bool GetLaneGuidanceEnabled() const;

    /*! Enables/disables the receiving of lane guidance for the navigation session.

        @param enable {@code true} to enable, {@code false} to disable.
        @return None
    */
    NavUIPreferences& SetLaneGuidance(bool enable);

    /*! Enables/disables the using of downloadable audio for announcements.

        @param enable {@code true} to enable, {@code false} to disable.
        @return None
    */
    NavUIPreferences& SetDownloadableAudioEnabled(bool enabled);

    /*! Returns the current setting.

        @return {@code true} if enabled, {@code false} if disabled.
    */
    bool GetDownloadableAudioEnabled() const;

    /*! Sets the value of position update rate for SMM.

        This setting controls the behavior of the GPS extrapolator
        in the navigation engine. The extrapolator generates fixes at
        a specified rate between each pair of real GPS fixes to enable
        the host application to support smooth map movement.
        The update rate should be set by the application based
        on the device's screen size and update speed.

        @param rate
        @return None
    */
    NavUIPreferences& SetPositionUpdateRate(qint32 rate);

    /*! Gets the value of position update rate.

        @return update rate
    */
    qint32 GetPositionUpdateRate() const;

    /*! This setting controls school zone alert feature. If enabled, the navigation session
        will provide an alert tone at least 200 meters before a school zone.

        @param enable
        @return None
    */
    NavUIPreferences& SetSchoolZoneAlertEnabled(bool enable);

    /*! Gets the value of school zone alert enabling.

        @return {@code true} if enabled, {@code false} if disabled.
    */
    bool IsSchoolZoneAlertEnabled() const;

    /*! This setting controls the speed limit alert feature. If enabled, the navigation session
        will provide an alert tone when the user is driving over the speed limit.

        @param enable
        @return None
    */
    NavUIPreferences& SetSpeedLimitAlertEnabled(bool enable);

    /*! Gets the value of speed limit alert enabling.

         @return {@code true} if enabled, {@code false} if disabled.
    */
    bool IsSpeedLimitAlertEnabled() const;

    /*! If enabled, speed limit signs will be provided via the speed sign listener.

        @param enable
        @return None
    */
    NavUIPreferences& SetSpeedLimitSignEnabled(bool enable);

    /*! Gets the value of speed limit sign enabling.

        @return {@code true} if enabled, {@code false} if disabled.
    */
    bool IsSpeedLimitSignEnabled() const;

    /*! Set type of measurement

        @param measurement
        @return None
    */
    NavUIPreferences& SetMeasurement(Measurement _measurement);

    /*! Gets the type of measurement

        @return Measurement
    */
    Measurement GetMeasurement() const;

    /*! If enabled, the routing engine will generate bridge maneuvers for the route.
        The defaults is disabled.

        @param enable
        @return None
    */
    NavUIPreferences& SetEnableBridgeManeuvers(bool enable);

    /*! Returns the current setting of bridge maneuvers

        @return {@code true} if enabled, {@code false} if disabled.
    */
    bool IsBridgeManeuversEnabled() const;


    /*! Returns the setting of traffic announcement enabled

        @return {@code true} if enabled, {@code false} is disabled.
    */
    bool IsTrafficAnnouncements() const;

    /*! Sets the traffic announcement enabled.

        @param enabled
    */
    NavUIPreferences& SetTrafficAnnouncements(bool enabled);

    /*! Returns need name changed Maneuvers enabled

        @return {@code true} if enabled, {@code false} is disabled.
    */
    bool IsNeedNCManeuver() const;

    /*! Sets need name changed Maneuvers enabled.

        @param enabled
    */
    NavUIPreferences& SetNeedNCManeuver(bool enabled);

    /*! Get speed limit sound alert threshold.

        Sound alert will be triggered only when current speed exceed speed limit by threshold.

        @return the value of speeding warning level, the unit is m/s.
    */
    qreal GetSpeedingWarningLevel() const;

    /*! Set speed limit sound alert threshold.

        Sound alert will be triggered only when current speed exceed speed limit by threshold.

        @param warningLevel the unit is m/s
    */
    NavUIPreferences& SetSpeedingWarningLevel(qreal warningLevel);

    /*! Get turn angle tolerance of startup offroute.

        @return tolerance
     */
    qreal GetOriginManeuverTurnAngleTolerance() const;

    /*! Set turn angle tolerance of startup offroute.

        This tolerance will be used to check if current location is in nearly area or far area,
        then the announcement of startup offroute will be different.

        @param turn angle tolerance.
     */
    NavUIPreferences& SetOriginManeuverTurnAngleTolerance(qreal tolerance);

    /*! Get is secondary name for exits

        @return {@code true} if secondary name for exits, {@code false} if not
     */
    bool IsSecondaryNameForExits() const;

    /*! Set secondary name for exits enabled.

        @param enabled
     */
    NavUIPreferences& SetSecondaryNameForExitsEnabled(bool enabled);

    /*! Get is check unsupported countries.

        @return {@code true} if check unsupported countries, {@code false} if not.
     */
    bool IsCheckUnsupportedCountries() const;

    /*! Set check unsupported countries enabled.

        @param enabled
     */
    NavUIPreferences& SetCheckUnsupportedCountriesEnabled(bool enabled);

    /*! Get is enhanced navigation startup.

        @return {@code true} if enhanced navigation startup, {@code false} if not
     */
    bool GetEnableEnhancedNavigationStartup() const;

    /*! Set enhanced navigation startup enabled.

        @param enabled
     */
    NavUIPreferences& SetEnableEnhancedNavigationStartup(bool enabled);

    /*! Get if speed limit image will be displayed with alter or not.

        @return {@code true} if speeding display alter enabled, {@code false} if not
     */
    bool GetSpeedingDisplayAlertEnabled() const;

    /*! Set if speed limit image will be displayed with alter or not.

        @param enabled
     */
    NavUIPreferences& SetSpeedingDisplayAlertEnabled(bool enabled);

    /*! Get if mock gps will be enabled or not.

        @return {@code true} if mock gps enabled, {@code false} if not
    */
    bool GetMockGpsEnabled() const;

    /*! Set if mock gps will be enabled or not.

        @param enabled
    */
    void SetMockGpsEnabled(bool enabled);

    /*! Get the mock gps speed.

        @return {@code double} the mock gps speed
    */
    double GetMockGpsSpeed() const;

    /*! Set the mock gps speed which is used to mock gps location.

        @param speed
    */
    void SetMockGpsSpeed(double speed);

private:
    qint32       mRecalcCriteria;
    bool        mWantMultipleRoutes;
    bool        mWantRelisticSign;
    bool        mWantNaturalGuidance;
    bool        mWantLaneGuidance;
    qint32       mUpdateRate;
    bool        mWantDownloadableAudio;
    bool        mWantRouteTypeSelector;
    qint32       mOffRouteIgnoreCount;
    bool        mSchoolZoneAlertEnabled;
    bool        mSpeedLimitAlertEnabled;
    bool        mSpeedLimitSignEnabled;
    bool        mWantBridgeManeuvers;
    Measurement  mMeasurement;
    bool        mTrafficAnnouncementEnabled;
    bool        mNeedNCManeuver;
    qreal        mSpeedingWarningLevel;
    qreal        mOrManeuverTurnAngleTolerance;
    bool        mSecondaryNameForExitsEnabled;
    bool        mWantCheckUnsupportedCountries;
    bool        mEnhancedStartupEnabled;
    bool        mSpeedingDisplayAlterEnabled;
    bool        mMockGpsEnabled;
    double       mMockGpsSpeed;
};
}
#endif //LOCATIONTOOLKIT_PREFERENCES_H
