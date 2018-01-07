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

#ifndef __CoreSDK__ManeuverImpl__
#define __CoreSDK__ManeuverImpl__

#include "NavApiTypes.h"
#include "NavigationImpl.h"
#include "NavApiNavUtils.h"
#include <vector>
extern "C"
{
#include "nberror.h"
#include "nbcontext.h"
#include "nbrouteinformation.h"
#include "nbnavigation.h"
#include "nbspatial.h"
#include "nbcontextaccess.h"
#include "abpalaudio.h"
#include "cslcache.h"
#include "palfile.h"
}

namespace nbnav
{
typedef shared_ptr<ManeuverImpl> ManeuverImplPtr;
class ManeuverImpl
{
public:
    ManeuverImpl(NavigationImpl* impl, unsigned int startIndex, unsigned int index, unsigned int routeIndex,
                 double redundantDistance, bool collapsed = TRUE);
    
    int GetManeuverID() const
    {
        return m_id;
    }
    
    const vector<Coordinates>& GetPolyline() const
    {
        return m_polyline;
    }
    
    string GetRoutingTTF() const
    {
        return m_ttfChar;
    }
    
    double GetDistance() const
    {
        return m_distance;
    }
    
    string GetPrimaryStreet() const
    {
        return m_primaryStreet;
    }
    
    string GetSecondaryStreet() const
    {
        return m_secondaryStreet;
    }
    
    double GetTime() const
    {
        return m_time;
    }
    
    string GetCommand() const
    {
        return m_command;
    }
    
    Coordinates GetPoint() const
    {
        return m_point;
    }
    
    FormattedTextBlock GetManeuverText(bool isMetric) const;
    
    string GetDescription(bool isMetric, bool isFormatted) const;
    
    double GetTrafficDelay() const
    {
        return m_trafficDelay;
    }
    
    bool GetStackAdvise() const
    {
        return m_stackAdvise;
    }
    
    string GetExitNumber() const
    {
        return m_exitNumber;
    }
    
    bool IsDestination() const
    {
        return m_command.compare(0, 3, "DT.") == 0;
    }
    
    std::vector<RouteProperty> GetManeuverProperties() const
    {
        return m_maneuverProperties;
    }

    double GetSpeed() const
    {
        return m_speed;
    }

private:
    int                      m_id;
    std::string              m_command;
    std::string              m_primaryStreet;
    std::string              m_secondaryStreet;
    double                   m_time;
    double                   m_trafficDelay;
    double                   m_distance;
    Coordinates              m_point;
    std::vector<Coordinates> m_polyline;
    std::string              m_ttfChar;
    bool                     m_stackAdvise;
    std::string              m_exitNumber;
    NavigationImpl*          m_impl;
    unsigned int             m_routeIndex;
    std::vector<RouteProperty> m_maneuverProperties;
    double                   m_speed;
};
}
#endif /* defined(__CoreSDK__ManeuverImpl__) */
