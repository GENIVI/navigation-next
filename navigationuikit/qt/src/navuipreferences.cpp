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
    @file navuipreferences.cpp
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
#include "navuipreferences.h"
#define OR_MANEUVER_TURN_ANGLE_TOLERANCE 40

namespace locationtoolkit
{
NavUIPreferences::NavUIPreferences()
    : mRecalcCriteria(RDL_Relaxed),
      mWantMultipleRoutes(false),
      mWantRelisticSign(false),
      mWantNaturalGuidance(false),
      mWantLaneGuidance(false),
      mUpdateRate(0),
      mWantDownloadableAudio(false),
      mWantRouteTypeSelector(false),
      mOffRouteIgnoreCount(0),
      mSchoolZoneAlertEnabled(false),
      mSpeedLimitAlertEnabled(false),
      mSpeedLimitSignEnabled(false),
      mWantBridgeManeuvers(false),
      mMeasurement(NonMetric),
      mTrafficAnnouncementEnabled(false),
      mNeedNCManeuver(false),
      mSpeedingWarningLevel(2.235), //5KPH;
      mOrManeuverTurnAngleTolerance(OR_MANEUVER_TURN_ANGLE_TOLERANCE),
      mSecondaryNameForExitsEnabled(false),
      mWantCheckUnsupportedCountries(true),
      mEnhancedStartupEnabled(false),
      mSpeedingDisplayAlterEnabled(true),
      mMockGpsEnabled(false),
      mMockGpsSpeed(0.f)
{
}

NavUIPreferences::~NavUIPreferences()
{

}

qint32 NavUIPreferences::GetRecalcCriteria() const
{
    return mRecalcCriteria;
}

NavUIPreferences& NavUIPreferences::SetRecalcCriteria(qint32 recalcCriteria)
{
    mRecalcCriteria = recalcCriteria;
    return *this;
}

qint32 NavUIPreferences::GetOffRouteIgnoreCount() const
{
    return mOffRouteIgnoreCount;
}

NavUIPreferences& NavUIPreferences::SetOffRouteIgnoreCount(qint32 offRouteIgnoreCount)
{
    mOffRouteIgnoreCount = offRouteIgnoreCount;
    return *this;
}

bool NavUIPreferences::GetMultipleRoutesEnabled() const
{
    return mWantMultipleRoutes;
}

NavUIPreferences& NavUIPreferences::SetMultipleRoutes(bool enable)
{
    mWantMultipleRoutes = enable;
    return *this;
}

bool NavUIPreferences::GetRealisticSignEnabled() const
{
    return mWantRelisticSign;
}

NavUIPreferences& NavUIPreferences::SetRealisticSign(bool enable)
{
    mWantRelisticSign = enable;
    return *this;
}

bool NavUIPreferences::GetNaturalGuidanceEnabled() const
{
    return mWantNaturalGuidance;
}

NavUIPreferences& NavUIPreferences::SetNaturalGuidance(bool enable)
{
    mWantNaturalGuidance = enable;
    return *this;
}

bool NavUIPreferences::GetLaneGuidanceEnabled() const
{
    return mWantLaneGuidance;
}

NavUIPreferences& NavUIPreferences::SetLaneGuidance(bool enable)
{
    mWantLaneGuidance = enable;
    return *this;
}

NavUIPreferences& NavUIPreferences::SetDownloadableAudioEnabled(bool enabled)
{
    mWantDownloadableAudio = enabled;
    return *this;
}

bool NavUIPreferences::GetDownloadableAudioEnabled() const
{
    return mWantDownloadableAudio;
}

NavUIPreferences& NavUIPreferences::SetPositionUpdateRate(qint32 rate)
{
    mUpdateRate = rate;
}

qint32 NavUIPreferences::GetPositionUpdateRate() const
{
    return mUpdateRate;
}

NavUIPreferences& NavUIPreferences::SetSchoolZoneAlertEnabled(bool enable)
{
    mSchoolZoneAlertEnabled = enable;
    return *this;
}

bool NavUIPreferences::IsSchoolZoneAlertEnabled() const
{
    return mSchoolZoneAlertEnabled;
}

NavUIPreferences& NavUIPreferences::SetSpeedLimitAlertEnabled(bool enable)
{
    mSpeedLimitAlertEnabled = enable;
    return *this;
}

bool NavUIPreferences::IsSpeedLimitAlertEnabled() const
{
    return mSpeedLimitAlertEnabled;
}

NavUIPreferences& NavUIPreferences::SetSpeedLimitSignEnabled(bool enable)
{
    mSpeedLimitSignEnabled = enable;
    return *this;
}

bool NavUIPreferences::IsSpeedLimitSignEnabled() const
{
    return mSpeedLimitSignEnabled;
}

NavUIPreferences& NavUIPreferences::SetMeasurement(NavUIPreferences::Measurement _measurement)
{
    mMeasurement = _measurement;
    return *this;
}

NavUIPreferences::Measurement NavUIPreferences::GetMeasurement() const
{
    return mMeasurement;
}

NavUIPreferences& NavUIPreferences::SetEnableBridgeManeuvers(bool enable)
{
    mWantBridgeManeuvers = enable;
    return *this;
}

bool NavUIPreferences::IsBridgeManeuversEnabled() const
{
    return mWantBridgeManeuvers;
}

NavUIPreferences& NavUIPreferences::SetTrafficAnnouncements(bool enabled)
{
    mTrafficAnnouncementEnabled = enabled;
    return *this;
}

bool NavUIPreferences::IsTrafficAnnouncements() const
{
    return mTrafficAnnouncementEnabled;
}

bool NavUIPreferences::IsNeedNCManeuver() const
{
    return mNeedNCManeuver;
}

NavUIPreferences& NavUIPreferences::SetNeedNCManeuver(bool enabled)
{
    mNeedNCManeuver = enabled;
    return *this;
}

qreal NavUIPreferences::GetSpeedingWarningLevel() const
{
    return mSpeedingWarningLevel;
}

NavUIPreferences& NavUIPreferences::SetSpeedingWarningLevel(qreal warningLevel)
{
    mSpeedingWarningLevel = warningLevel;
    return *this;
}

qreal NavUIPreferences::GetOriginManeuverTurnAngleTolerance() const
{
    return mOrManeuverTurnAngleTolerance;
}

NavUIPreferences& NavUIPreferences::SetOriginManeuverTurnAngleTolerance(qreal tolerance)
{
    mOrManeuverTurnAngleTolerance = tolerance;
    return *this;
}

bool NavUIPreferences::IsSecondaryNameForExits() const
{
    return mSecondaryNameForExitsEnabled;
}

NavUIPreferences& NavUIPreferences::SetSecondaryNameForExitsEnabled(bool enabled)
{
    mSecondaryNameForExitsEnabled = enabled;
    return *this;
}

bool NavUIPreferences::IsCheckUnsupportedCountries() const
{
    return mWantCheckUnsupportedCountries;
}

NavUIPreferences& NavUIPreferences::SetCheckUnsupportedCountriesEnabled(bool enabled)
{
    mWantCheckUnsupportedCountries = enabled;
    return *this;
}

bool NavUIPreferences::GetEnableEnhancedNavigationStartup() const
{
    return mEnhancedStartupEnabled;
}

NavUIPreferences& NavUIPreferences::SetEnableEnhancedNavigationStartup(bool enable)
{
    mEnhancedStartupEnabled = enable;
    return *this;
}

bool NavUIPreferences::GetSpeedingDisplayAlertEnabled() const
{
    return mSpeedingDisplayAlterEnabled;
}

NavUIPreferences& NavUIPreferences::SetSpeedingDisplayAlertEnabled(bool enabled)
{
    mSpeedingDisplayAlterEnabled = enabled;
    return *this;
}

bool NavUIPreferences::GetMockGpsEnabled() const
{
    return mMockGpsEnabled;
}

void NavUIPreferences::SetMockGpsEnabled(bool enabled)
{
    mMockGpsEnabled = enabled;
}

double NavUIPreferences::GetMockGpsSpeed() const
{
    return mMockGpsSpeed;
}

void NavUIPreferences::SetMockGpsSpeed(double speed)
{
    mMockGpsSpeed = speed;
}

}
