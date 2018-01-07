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
 @file         RouteSelectInformation.cpp
 @defgroup     nkui
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.
 
 The information contained herein is confidential, proprietary to
 TeleCommunication Systems, Inc., and considered a trade secret as defined
 in section 499C of the penal code of the State of California. Use of this
 information by anyone other than authorized employees of TeleCommunication
 Systems is granted only under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 --------------------------------------------------------------------------*/

/*! @{ */

#include "RouteSelectInformation.h"
#include "palclock.h"
#include "FormatController.h"

using namespace nkui;
using namespace nbnav;
using namespace std;

#define DISTANCE_THRESHOLD  (1609.344*500)          // 500 miles
#define TIME_THRESHOLD      (3600*24)               // 24 hours

RouteSelectInformation::RouteSelectInformation(NKUIStringHelperPtr stringHelper, NKUIContext* navContext)
    : m_pStringHelper(stringHelper),
      m_pNavUIContex(navContext)
{

}

RouteSelectInformation::RouteSelectInformation()
{

}

RouteSelectInformation::~RouteSelectInformation()
{

}

vector<RouteInfoData>& RouteSelectInformation::GetRouteInfoData()
{
    bool isDetour = false;
    if(m_pNavUIContex->GetPublicState() == NPS_Detour)
    {
        isDetour = true;
    }
    const vector<RouteInformation>& routes = m_pNavUIContex->GetAllRoutes();
    FormatController formater(m_pStringHelper);
    RouteInfoData data = {0};
    m_routeInfos.clear();
    for(int i = 0; i<routes.size(); i++)
    {
        const RouteInformation& routeInfo = routes[i];
        // Format Distance
        double tripDistance = routeInfo.GetDistance();
        string distanceText;
        string distanceUnit;
        formater.FormatDistance(m_pNavUIContex->GetPreferences().GetMeasurement(), tripDistance, distanceText, distanceUnit);
        data.tripDistance = distanceText + " " + distanceUnit;
        // Format Time
        uint32 tripTime = (uint32)routeInfo.GetTime();
        string timeText;
        string timeText2;
        string timeUnit;
        string timeUnit2;
        formater.FormatTime(tripTime, timeText, timeUnit, timeText2, timeUnit2);
        timeText = timeText + " " + timeUnit;
        if(timeText2.size() > 0)
        {
            timeText = timeText + " " + timeText2 + " " + timeUnit2;
        }
        data.tripTime = timeText;
        data.majorRoad =routeInfo.GetRouteDescriptions();
        if(!isDetour)
        {
            data.majorRoadPrefix = m_pStringHelper->GetLocalizedString("IDS_VIA");
        }
        else
        {
            //the last routes is current route
            if( i == routes.size()-1)
            {
                data.majorRoadPrefix = m_pStringHelper->GetLocalizedString("IDS_CURRENT") + " " + m_pStringHelper->GetLocalizedString("IDS_VIA") + " ";
            }
            else
            {
                data.majorRoadPrefix = m_pStringHelper->GetLocalizedString("IDS_DETOUR") + " " + m_pStringHelper->GetLocalizedString("IDS_VIA") + " ";
            }
        }
        uint32 currentTime = PAL_ClockGetUnixTime();
        if (tripDistance < DISTANCE_THRESHOLD || tripTime < TIME_THRESHOLD)
        {
            data.arrivalTime = currentTime + (uint32)routeInfo.GetTime();
        }
        if(m_pNavUIContex->GetRouteOptions().GetTransportationMode() != Pedestrian)
        {
            data.traffic = routeInfo.GetTrafficColor();
        }
        else
        {
            data.traffic = 0;
        }
        GetTipViewText(routeInfo.GetRouteProperties(), data.routeTipText);
        data.mode = m_pNavUIContex->GetRouteOptions().GetTransportationMode();
        m_routeInfos.push_back(data);
    }
    return m_routeInfos;
}

void RouteSelectInformation::GetTipViewText(vector<RouteProperty> routeProperty, std::string& routeTip)
{
    routeTip = m_pStringHelper->GetLocalizedString("IDS_THIS_ROUTE_INC") + " ";
    const RouteOptions& routeOption = m_pNavUIContex->GetRouteOptions();
    nbnav::RouteAvoid avoid = (nbnav::RouteAvoid)routeOption.GetAvoidFeatures();
    int infoCount = 0;
    if (find(routeProperty.begin(), routeProperty.end(), HIGHWAY) != routeProperty.end() && (avoid & AVD_Highway))
    {
        routeTip += m_pStringHelper->GetLocalizedString("IDS_HIGHWAYS");
        infoCount++;
    }
    if(find(routeProperty.begin(), routeProperty.end(), TOLL_ROAD) != routeProperty.end())
    {
        if (infoCount > 0)
        {
            routeTip += ", ";
        }
        routeTip += m_pStringHelper->GetLocalizedString("IDS_TOLL_ROADS");
        infoCount++;
    }
    if(find(routeProperty.begin(), routeProperty.end(), HOV) != routeProperty.end() && (avoid & AVD_HOV))
    {
        if (infoCount > 0)
        {
            routeTip += ", ";
        }
        routeTip += m_pStringHelper->GetLocalizedString("IDS_CARPOOL_LANES");
        infoCount++;
    }
    if(find(routeProperty.begin(), routeProperty.end(), UNPAVED_ROAD) != routeProperty.end())
    {
        if (infoCount > 0)
        {
            routeTip += ", ";
        }
        routeTip += m_pStringHelper->GetLocalizedString("IDS_UNPAVED_ROADS");
        infoCount++;
    }
    if(find(routeProperty.begin(), routeProperty.end(), FERRY) != routeProperty.end())
    {
        if (infoCount > 0)
        {
            routeTip += ", ";
        }
        routeTip += m_pStringHelper->GetLocalizedString("IDS_FERRIES");
        infoCount++;
    }
    if (infoCount > 0)
    {
        routeTip += ".";
    }
    else
    {
        routeTip = "";
    }
}

/*! @} */
