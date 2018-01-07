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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
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

#include "NavApiSpeedLimitInformation.h"
#include "NavigationImpl.h"
extern "C"
{
#include "nbspeedlimitprocessor.h"
#include "nbnavigationstate.h"
#include "nbnavigation.h"
}

namespace nbnav
{

using namespace std;

const string SpeedLimitInformation::ZONE_TYPE_UNKNOWN      = "unknown"; //Unknown zone
const string SpeedLimitInformation::ZONE_TYPE_SCHOOL       = "sz";  //School zone
const string SpeedLimitInformation::ZONE_TYPE_CONSTRUCTION = "cz";  //Construction zone-Cone zone
const string SpeedLimitInformation::ZONE_TYPE_ENHANCED     = "ez";  //Safety enhanced zone-Double fine zone

string SpeedLimitInformation::Type() const
{
    return m_type;
}

const Image& SpeedLimitInformation::Information() const
{
    return m_slImage;
}

bool SpeedLimitInformation::IsWarning() const
{
    return m_isWarning;
}

double SpeedLimitInformation::SpeedLimit() const
{
    return m_speedLimit;
}

double SpeedLimitInformation::CurrentSpeed() const
{
    return m_curSpeed;
}

SpeedLimitInformation::SpeedLimitInformation(const NavigationImpl& impl, const void* data)
    : m_type(SpeedLimitInformation::ZONE_TYPE_UNKNOWN),
      m_slImage(NULL, 0, PNG)
{
    const NB_SpeedLimitStateData* speedlimitData = static_cast<const NB_SpeedLimitStateData*>(data);
    switch (speedlimitData->type)
    {
    case NB_SpeedZoneType_SchoolZone:
        m_type = "SZ";
        break;
    case NB_SpeedZoneType_ConstructionZone:
        m_type = "CZ";
        break;
    case NB_SpeedZoneType_SafetyEnhancedZone:
        m_type = "EZ";
        break;
    case NB_SpeedZoneType_None:
        m_type = "";
        break;
    }
    Image tmpImage(speedlimitData->speedlimitPictureData, speedlimitData->pictureSize, PNG);
    m_slImage = tmpImage;
    m_speedLimit = speedlimitData->limitSpeed;
    NB_NavigationPublicState publicState;
    nsl_memset(&publicState, 0, sizeof(publicState));
    NB_NavigationGetPublicState(impl.GetNbNavigation(), &publicState);
    m_curSpeed = publicState.speed;
    m_isWarning = m_curSpeed > m_speedLimit;
}

SpeedLimitInformation::SpeedLimitInformation()
    : m_slImage(Image(NULL, 0, PNG))
{
}

}
