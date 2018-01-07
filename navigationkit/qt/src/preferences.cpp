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
    @file routeoptions.cpp
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
#include "preferences.h"
#define OR_MANEUVER_TURN_ANGLE_TOLERANCE 40

namespace locationtoolkit
{
Preferences::Preferences()
    : m_recalcCriteria(RDL_Relaxed),
      m_wantMultipleRoutes(false),
      m_wantRelisticSign(false),
      m_wantNaturalGuidance(false),
      m_wantLaneGuidance(false),
      m_updateRate(0),
      m_wantDownloadableAudio(false),
      m_wantRouteTypeSelector(false),
      m_offRouteIgnoreCount(0),
      m_schoolZoneAlertEnabled(false),
      m_speedLimitAlertEnabled(false),
      m_speedLimitSignEnabled(false),
      m_wantBridgeManeuvers(false),
      m_measurement(NonMetric),
      m_trafficAnnouncementEnabled(false),
      m_needNCManeuver(false),
      m_speedingWarningLevel(2.235), //5KPH;
      m_orManeuverTurnAngleTolerance(OR_MANEUVER_TURN_ANGLE_TOLERANCE),
      m_secondaryNameForExitsEnabled(false),
      m_wantCheckUnsupportedCountries(true),
      m_enhancedStartupEnabled(false),
      m_speedingDisplayAlterEnabled(true)
{
}

qint32 Preferences::GetRecalcCriteria() const
{
    return m_recalcCriteria;
}

void Preferences::SetRecalcCriteria(qint32 recalcCriteria)
{
    m_recalcCriteria = recalcCriteria;
}

qint32 Preferences::GetOffRouteIgnoreCount() const
{
    return m_offRouteIgnoreCount;
}

void Preferences::SetOffRouteIgnoreCount(qint32 offRouteIgnoreCount)
{
    m_offRouteIgnoreCount = offRouteIgnoreCount;
}

bool Preferences::GetMultipleRoutesEnabled() const
{
    return m_wantMultipleRoutes;
}

void Preferences::SetMultipleRoutes(bool enable)
{
    m_wantMultipleRoutes = enable;
}

bool Preferences::GetRealisticSignEnabled() const
{
    return m_wantRelisticSign;
}

void Preferences::SetRealisticSign(bool enable)
{
    m_wantRelisticSign = enable;
}

bool Preferences::GetNaturalGuidanceEnabled() const
{
    return m_wantNaturalGuidance;
}

void Preferences::SetNaturalGuidance(bool enable)
{
    m_wantNaturalGuidance = enable;
}

bool Preferences::GetLaneGuidanceEnabled() const
{
    return m_wantLaneGuidance;
}

void Preferences::SetLaneGuidance(bool enable)
{
    m_wantLaneGuidance = enable;
}

void Preferences::SetDownloadableAudioEnabled(bool enabled)
{
    m_wantDownloadableAudio = enabled;
}

bool Preferences::GetDownloadableAudioEnabled() const
{
    return m_wantDownloadableAudio;
}

void Preferences::SetPositionUpdateRate(qint32 rate)
{
    m_updateRate = rate;
}

qint32 Preferences::GetPositionUpdateRate() const
{
    return m_updateRate;
}

void Preferences::SetSchoolZoneAlertEnabled(bool enable)
{
    m_schoolZoneAlertEnabled = enable;
}

bool Preferences::IsSchoolZoneAlertEnabled() const
{
    return m_schoolZoneAlertEnabled;
}

void Preferences::SetSpeedLimitAlertEnabled(bool enable)
{
    m_speedLimitAlertEnabled = enable;
}

bool Preferences::IsSpeedLimitAlertEnabled() const
{
    return m_speedLimitAlertEnabled;
}

void Preferences::SetSpeedLimitSignEnabled(bool enable)
{
    m_speedLimitSignEnabled = enable;
}

bool Preferences::IsSpeedLimitSignEnabled() const
{
    return m_speedLimitSignEnabled;
}

void Preferences::SetMeasurement(Preferences::Measurement _measurement)
{
    m_measurement = _measurement;
}

Preferences::Measurement Preferences::GetMeasurement() const
{
    return m_measurement;
}

void Preferences::SetEnableBridgeManeuvers(bool enable)
{
    m_wantBridgeManeuvers = enable;
}

bool Preferences::IsBridgeManeuversEnabled() const
{
    return m_wantBridgeManeuvers;
}

void Preferences::SetTrafficAnnouncements(bool enabled)
{
    m_trafficAnnouncementEnabled = enabled;
}

bool Preferences::IsTrafficAnnouncements() const
{
    return m_trafficAnnouncementEnabled;
}

bool Preferences::IsNeedNCManeuver() const
{
    return m_needNCManeuver;
}

void Preferences::SetNeedNCManeuver(bool enabled)
{
    m_needNCManeuver = enabled;
}

qreal Preferences::GetSpeedingWarningLevel() const
{
    return m_speedingWarningLevel;
}

void Preferences::SetSpeedingWarningLevel(qreal warningLevel)
{
    m_speedingWarningLevel = warningLevel;
}

qreal Preferences::GetOriginManeuverTurnAngleTolerance() const
{
    return m_orManeuverTurnAngleTolerance;
}

void Preferences::SetOriginManeuverTurnAngleTolerance(qreal tolerance)
{
    m_orManeuverTurnAngleTolerance = tolerance;
}

bool Preferences::IsSecondaryNameForExits() const
{
    return m_secondaryNameForExitsEnabled;
}

void Preferences::SetSecondaryNameForExitsEnabled(bool enabled)
{
    m_secondaryNameForExitsEnabled = enabled;
}

bool Preferences::IsCheckUnsupportedCountries() const
{
    return m_wantCheckUnsupportedCountries;
}

void Preferences::SetCheckUnsupportedCountriesEnabled(bool enabled)
{
    m_wantCheckUnsupportedCountries = enabled;
}

bool Preferences::GetEnableEnhancedNavigationStartup() const
{
    return m_enhancedStartupEnabled;
}

void Preferences::SetEnableEnhancedNavigationStartup(bool enable)
{
    m_enhancedStartupEnabled = enable;
}

bool Preferences::GetSpeedingDisplayAlertEnabled() const
{
    return m_speedingDisplayAlterEnabled;
}

void Preferences::SetSpeedingDisplayAlertEnabled(bool enabled)
{
    m_speedingDisplayAlterEnabled = enabled;
}

}
