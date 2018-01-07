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

#include "NavApiRouteOptions.h"

using namespace std;

namespace nbnav
{
RouteOptions::RouteOptions(RouteType routeType, TransportationMode transportationMode,
                           unsigned int avoid, string pronunciationStyle)
    : m_avoid(avoid),
      m_routeType(routeType),
      m_transportationMode(transportationMode)
{
    if (!pronunciationStyle.empty())
    {
        m_pronunciationStyle = pronunciationStyle;
    }
    else
    {
        m_pronunciationStyle = "female-5-aac-v3";
    }
}

RouteOptions::RouteOptions(const RouteOptions& routeOptions)
    : m_avoid(routeOptions.m_avoid),
      m_routeType(routeOptions.m_routeType),
      m_transportationMode(routeOptions.m_transportationMode),
      m_pronunciationStyle(routeOptions.m_pronunciationStyle)
{
}

unsigned int RouteOptions::GetAvoidFeatures() const
{
    return m_avoid;
}

void RouteOptions::SetAvoidFeatures(unsigned int avoid)
{
    m_avoid = avoid;
}

RouteType RouteOptions::GetRouteType() const
{
    return m_routeType;
}

void RouteOptions::SetRouteType(RouteType routeType)
{
    m_routeType = routeType;
}

TransportationMode RouteOptions::GetTransportationMode() const
{
    return m_transportationMode;
}

void RouteOptions::SetTransportationMode(TransportationMode transportationMode)
{
    m_transportationMode = transportationMode;
}

string RouteOptions::GetPronunciationStyle() const
{
    return m_pronunciationStyle;
}

void RouteOptions::SetPronunciationStyle(std::string pronunciationStyle)
{
    m_pronunciationStyle = pronunciationStyle;
}

RouteOptions& RouteOptions::operator=(const RouteOptions& right)
{
    if (this != &right)
    {
        m_avoid = right.m_avoid;
        m_routeType = right.m_routeType;
        m_transportationMode = right.m_transportationMode;
        m_pronunciationStyle = right.m_pronunciationStyle;
    }
    return *this;
}

}
