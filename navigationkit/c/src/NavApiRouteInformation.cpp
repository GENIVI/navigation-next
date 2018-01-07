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

#include "NavApiRouteInformation.h"
#include "NavigationImpl.h"
#include "NavApiNavUtils.h"
#include "NavApiTypes.h"
#include "ManeuverImpl.h"
#include <map>
extern "C"
{
#include "palfile.h"
#include "nbcontextprotected.h"
#include "nbspatial.h"
}

namespace nbnav
{

using namespace std;

const float ROUTE_POLYLINE_WIDTH             = 18.0f;
#define LENGTH_THRESHOLD        0.1     // meters
typedef std::map<double, TrafficSegmentColor> TrafficSegmentMap;

Place NbPlaceToPlace(const NB_Place& nbPlace);

//////////////////////////////////////////////////////
//  class TrafficEvent
//////////////////////////////////////////////////////

TrafficEvent::TrafficEvent()
{
}

TrafficEvent::TrafficEvent(NavigationImpl* impl, void* data, void* routeData)
{
    if (data && routeData)
    {
        NB_TrafficEvent* trafficEvent = static_cast<NB_TrafficEvent*>(data);
        NB_RouteInformation* route = static_cast<NB_RouteInformation*>(routeData);

        m_distanceToTrafficItem = trafficEvent->distanceToEvent;
        m_maneuverNumber = impl->GetSignificantManeuverIndex(trafficEvent->maneuverIndex);
        double routeTotalDistance = 0.0;
        (void)NB_RouteInformationGetSummaryInformation(route, NULL, NULL, &routeTotalDistance);
        m_startFromTrip = routeTotalDistance - trafficEvent->routeRemainAfterEvent;
        if (m_startFromTrip < 0.0)
        {
            m_startFromTrip = 0.0;
        }

        if (trafficEvent->type == NB_TET_Congestion)
        {
            m_type = TrafficEvent::Congestion;
            m_delay = trafficEvent->detail.congestion.delaySeconds;
            m_endTime = 0;
            m_reportTime = 0;
            m_roadName = trafficEvent->detail.congestion.road;

            switch (trafficEvent->detail.congestion.severity)
            {
                case NB_NTC_Severe:
                {
                    m_severity = TrafficEvent::Severe;
                    break;
                }
                case NB_NTC_Moderate:
                {
                    m_severity = TrafficEvent::Moderate;
                    break;
                }
                default:
                {
                    m_severity = TrafficEvent::Unknown;
                    break;
                }
            }
            m_length = trafficEvent->detail.congestion.length;
        }
        else
        {
            m_type = TrafficEvent::Incident;
            m_delay = 0;
            m_description = trafficEvent->detail.incident.description;
            m_endTime = trafficEvent->detail.incident.end_time;
            m_reportTime = trafficEvent->detail.incident.entry_time;
            m_roadName = trafficEvent->detail.incident.road;

            switch (trafficEvent->detail.incident.severity)
            {
                case NB_TrafficIncident_Major:
                {
                    m_severity = TrafficEvent::Severe;
                    break;
                }
                case NB_TrafficIncident_Minor:
                {
                    m_severity = TrafficEvent::Moderate;
                    break;
                }
                case NB_TrafficIncident_LowImpact:
                {
                    // ;TODO;LOW PRIORITY;CLARIFY;There is no item in TrafficSeverity enum for LowImpact
                    m_severity = TrafficEvent::Moderate;
                    break;
                }
                default:
                {
                    m_severity = TrafficEvent::Unknown;
                    break;
                }
            }
            m_length = 0;
        }
    }
}

TrafficEvent::~TrafficEvent()
{
}

uint32 TrafficEvent::GetDelay() const
{
    return m_delay;
}

string TrafficEvent::GetDescription() const
{
    return m_description;
}

double TrafficEvent::GetDistanceToTrafficItem() const
{
    return m_distanceToTrafficItem;
}

uint32 TrafficEvent::GetEndTime() const
{
    return m_endTime;
}

double TrafficEvent::GetLength() const
{
    return m_length;
}

unsigned int TrafficEvent::GetManeuverNumber() const
{
    return m_maneuverNumber;
}

uint32 TrafficEvent::GetReportTime() const
{
    return m_reportTime;
}

string TrafficEvent::GetRoad() const
{
    return m_roadName;
}

TrafficEvent::TrafficSeverity TrafficEvent::GetSeverity() const
{
    return m_severity;
}

double TrafficEvent::GetStartFromTrip() const
{
    return m_startFromTrip;
}

TrafficEvent::Type TrafficEvent::GetType() const
{
    return m_type;
}

//////////////////////////////////////////////////////
//  class RouteInformation
//////////////////////////////////////////////////////

RouteInformation::RouteInformation(NavigationImpl* impl, void* data, unsigned int index)
    : m_maneuverList(ManeuverList(impl, index)),
      m_uncollapsedManeuver(impl, index, false, false),
      m_index(index),
      m_trafficColor('\0'),
      m_streetSide(Center),
      m_isPlanRoute(false),
      m_routeInfo(NULL)
{
    if (!data)
    {
        return;
    }
    m_navigationImpl = impl;
    m_selectedRoute = index;
    m_resources = static_cast<void*>(NB_ContextGetPal(impl->GetNbContext()));
    NB_RouteInformation* routeInfo = static_cast<NB_RouteInformation*>(data);
    m_routeInfo = data;

    uint32 count = NB_RouteInformationGetManeuverCount(routeInfo);

    NB_MercatorPolyline* polylines = NULL;
    if (NE_OK == NB_RouteInformationGetRoutePolyline(routeInfo, 0, count, &polylines))
    {
        for (uint32 segIdx = 0; segIdx < (uint32)polylines->count; ++segIdx)
        {
            Coordinates coord;
            memset(&coord, 0, sizeof(coord));
            NB_SpatialConvertMercatorToLatLong(polylines->points[segIdx].mx, polylines->points[segIdx].my,
                &coord.latitude, &coord.longitude);
            m_routePolylines.push_back(coord);
        }
        NB_MercatorPolylineDestroy(polylines);
    }

    if (count > 0)
    {
        NB_LatitudeLongitude min, max;
        if (NB_RouteInformationGetRouteExtent(routeInfo, 0, count, &min, &max) == NE_OK)
        {
            m_boundingBox.point1.latitude = min.latitude;
            m_boundingBox.point1.longitude = min.longitude;
            m_boundingBox.point2.latitude = max.latitude;
            m_boundingBox.point2.longitude = max.longitude;
        }
    }

    NB_Place nbPlaceDestination;
    nsl_memset(&nbPlaceDestination, 0, sizeof(nbPlaceDestination));
    (void)NB_RouteInformationGetDestination(routeInfo, &nbPlaceDestination);
    m_destination = NbPlaceToPlace(nbPlaceDestination);

    NB_Place nbPlaceOrigin;
    nsl_memset(&nbPlaceOrigin, 0, sizeof(nbPlaceOrigin));
    (void)NB_RouteInformationGetOrigin(routeInfo, &nbPlaceOrigin);

    m_origin = NbPlaceToPlace(nbPlaceOrigin);
    m_routeError = impl->GetRouteError();
    NB_RouteInformationGetSummaryInformation(routeInfo, &m_time, &m_delay, &m_distance);

    NB_RouteId* id = NULL;
    if (NB_RouteInformationGetRouteId(routeInfo, &id) == NE_OK)
    {
        char* encodedRouteId = NULL;
        if (encode_base64((const char*)id->data, id->size,
                          &encodedRouteId) == NE_OK)
        {
            m_routeID.assign(encodedRouteId, nsl_strlen(encodedRouteId));
            nsl_free(encodedRouteId);
        }
    }

    NB_OnRouteInformation onRouteInformation;
    memset(&onRouteInformation, 0, sizeof(onRouteInformation));
    if (NB_RouteInformationOnRouteInfo(routeInfo, &onRouteInformation) == NE_OK)
    {
        if (onRouteInformation.isHovLanesOnRoute)
        {
            m_routeProperties.push_back(HOV);
        }
        if (onRouteInformation.isHighWayOnRoute)
        {
            m_routeProperties.push_back(HIGHWAY);
        }
        if (onRouteInformation.isTollsOnRoute)
        {
            m_routeProperties.push_back(TOLL_ROAD);
        }
        if (onRouteInformation.isFerryOnRoute)
        {
            m_routeProperties.push_back(FERRY);
        }
        if (onRouteInformation.isUnpavedOnRoute)
        {
            m_routeProperties.push_back(UNPAVED_ROAD);
        }
        if (onRouteInformation.isGatedAccessOnRoute)
        {
            m_routeProperties.push_back(GATED_ACCESS_ROAD);
        }
        if (onRouteInformation.isPrivateRoadOnRoute)
        {
            m_routeProperties.push_back(PRIVATE_ROAD);
        }
        if (onRouteInformation.isEnterCoutryOnRoute)
        {
            m_routeProperties.push_back(ENTER_COUNTRY);
        }
        if (onRouteInformation.isCongestionOnRoute)
        {
            m_routeProperties.push_back(CONGESTION);
        }
        if (onRouteInformation.isAccidentOnRoute)
        {
            m_routeProperties.push_back(ACCIDENT);
        }
        if (onRouteInformation.isConstructionOnRoute)
        {
            m_routeProperties.push_back(CONSTRUCTION);
        }
    }

    const NB_GpsLocation* currentLocation = impl->GetLastLocation();
    if (currentLocation)
    {
        NB_GuidanceMessage* pMessage = NULL;
        NB_RouteInformationGetTurnAnnouncementExt(routeInfo, impl->GetNbGuidanceInfo(), NAV_MANEUVER_INITIAL, NB_NMP_Continue,
                                                  NB_NAS_Lookahead, NB_NAT_Street, impl->GetMeasurementUnits(), &pMessage,
                                                  currentLocation->latitude, currentLocation->longitude);
        impl->ConvertGuidanceMessageToString(pMessage, m_initialInstruction);
        if (pMessage)
        {
            NB_GuidanceMessageDestroy(pMessage);
        }

        NB_GuidanceMessage* pMessageStart = NULL;
        NB_RouteInformationGetTurnAnnouncementExt(routeInfo, impl->GetNbGuidanceInfo(), NAV_MANEUVER_START, NB_NMP_Continue,
                                                  NB_NAS_Lookahead, NB_NAT_Street, impl->GetMeasurementUnits(), &pMessageStart,
                                                  currentLocation->latitude, currentLocation->longitude);
        impl->ConvertGuidanceMessageToString(pMessageStart, m_startupInstruction);
        if (pMessageStart)
        {
            NB_GuidanceMessageDestroy(pMessageStart);
        }
    }
    const char* routeDescription = NULL;
    NB_RouteInformationGetRouteDescription(routeInfo, &routeDescription);
    if (routeDescription)
    {
        m_description.append(routeDescription);
    }
    if (m_uncollapsedManeuver.GetNumberOfManeuvers() > 0)
    {
        const Maneuver* destinationManeuver = m_uncollapsedManeuver.GetManeuver(m_uncollapsedManeuver.GetNumberOfManeuvers() - 1);
        if (destinationManeuver)
        {
            if (destinationManeuver->GetCommand().compare("DT.L") == 0)
            {
                m_streetSide = Left;
            }
            else if (destinationManeuver->GetCommand().compare("DT.R") == 0)
            {
                m_streetSide = Right;
            }
        }
    }

    NB_LatitudeLongitude labelLocation;
    memset(&labelLocation, 0, sizeof(labelLocation));
    if (NB_RouteInformationGetLabelPoint(routeInfo, &labelLocation) == NE_OK)
    {
        m_labelPoint.latitude = labelLocation.latitude;
        m_labelPoint.longitude = labelLocation.longitude;
        m_labelPoint.accuracy = 0;
    }

    char primaryNameBuffer[NB_NAVIGATION_STREET_NAME_MAX_LEN] = {0};
    NB_RouteInformationGetFirstMajorRoadName(routeInfo, primaryNameBuffer, NB_NAVIGATION_STREET_NAME_MAX_LEN);
    m_firstMajorRoad.assign(primaryNameBuffer);
    UpdateTraffic(false);
}

void RouteInformation::UpdateTraffic(bool newTraffic)
{
    if(!m_routeInfo)
    {
        return;
    }
    NB_RouteInformation* routeInfo = static_cast<NB_RouteInformation*>(m_routeInfo);
    NB_TrafficInformation* old = NULL;
    if(newTraffic)
    {
        NavigationImpl* impl = (NavigationImpl*)m_navigationImpl;
        old = NB_RouteInformationGetTrafficInformationPointer(routeInfo);
        NB_RouteInformationSetTrafficInformationPointer(routeInfo, impl->GetNbNavTrafficInfo());
    }
    
    NB_RouteInformationGetTrafficColor(routeInfo, &m_trafficColor);
    uint32 trafficEventCount = 0;
    NB_TrafficEvent trafficEvent;
    m_trafficEvents.clear();
    if (NB_RouteInformationGetTrafficEventCount(routeInfo, &trafficEventCount) == NE_OK)
    {
        for (uint32 i = 0; i < trafficEventCount; i++)
        {
            nsl_memset(&trafficEvent, 0, sizeof(NB_TrafficEvent));
            if (NB_RouteInformationGetTrafficEvent(routeInfo, i, &trafficEvent) == NE_OK)
            {
                if (trafficEvent.type == NB_TET_Congestion)
                {
                    m_trafficEvents.push_back(TrafficEvent((NavigationImpl*)m_navigationImpl, static_cast<void*>(&trafficEvent), m_routeInfo));
                }
            }
        }
    }
    uint32 count;
    //route summary
    NB_RouteInformationGetRouteSummaryInformationCount(routeInfo, &count);
    for(uint32 i = 0; i< count; i++)
    {
        uint32 time = 0;
        uint32 delay = 0;
        uint32 distance = 0;
        const char* via = NULL;
        if(NB_RouteInformationGetRouteSummaryInformation(routeInfo, &distance, &time, &via, &delay, i) == NE_OK)
        {
            RouteSummaryInformation info(time,distance,delay,via);
            m_routeSummarys.push_back(info);
        }
    }
    CreateTrafficColorSegment();
    //restore traffic inforamtion
    if(newTraffic)
    {
        NB_RouteInformationSetTrafficInformationPointer(routeInfo,old);
    }
}

ManeuverList RouteInformation::GetManeuverList() const
{
    return m_uncollapsedManeuver;
}
    
ManeuverList RouteInformation::GetUpcomingManeuverList(double tripRemainingDistance)
{
    if(tripRemainingDistance != this->m_distance)
    {
        double distance = 0;
        uint32 index = m_uncollapsedManeuver.GetNumberOfManeuvers() - 1;
        while (index > 0)
        {
            const Maneuver* man = m_uncollapsedManeuver.GetManeuver(index);
            distance += man->GetDistance();
            if(distance >= tripRemainingDistance)
            {
                break;
            }
            index--;
        }
        m_maneuverList.Update((NavigationImpl*)m_navigationImpl, m_selectedRoute, true, index);
    }
    return m_maneuverList;
}

const string& RouteInformation::GetRouteID() const
{
    return m_routeID;
}

double RouteInformation::GetDistance() const
{
    return m_distance;
}

uint32 RouteInformation::GetTime() const
{
    return m_time;
}

uint32 RouteInformation::GetDelay() const
{
    return m_delay;
}

NavigateRouteError RouteInformation::GetRouteError() const
{
    return m_routeError;
}

Place RouteInformation::GetOrigin() const
{
    return m_origin;
}

Place RouteInformation::GetDestination() const
{
    return m_destination;
}

BoundingBox RouteInformation::GetBoundingBox() const
{
    return m_boundingBox;
}

BinaryBuffer RouteInformation::GetVoiceFile(string name) const
{
    PAL_Instance* pal = static_cast<PAL_Instance*>(m_resources);
    string workPath(PAL_FileGetWorkPath());
    string commonPath(workPath + PATH_DELIMITER + "NK_" + PAL_GetLocale(pal) + PATH_DELIMITER);
    string fullName(commonPath + name + ".aac");
    unsigned char* pBuf = NULL;
    uint32 size = 0;

    if (PAL_FileLoadFile(pal, fullName.c_str(), &pBuf, &size) == PAL_Ok)
    {
        return BinaryBuffer(pBuf, size);
    }
    return BinaryBuffer(NULL, 0);
}

vector<Coordinates> RouteInformation::GetPolyline() const
{
    return m_routePolylines;
}

string RouteInformation::GetInitialGuidanceText() const
{
    return m_initialInstruction;
}

string RouteInformation::GetRouteDescriptions() const
{
    return m_description;
}

vector<RouteProperty> RouteInformation::GetRouteProperties() const
{
    return m_routeProperties;
}

const TrafficEvent* RouteInformation::GetTrafficEvent(unsigned int index) const
{
    if (index >= m_trafficEvents.size())
    {
        return NULL;
    }
    return &m_trafficEvents[index];
}

unsigned int RouteInformation::GetNumberOfTrafficEvent() const
{
    return (unsigned int)m_trafficEvents.size();
}

char RouteInformation::GetTrafficColor() const
{
    return m_trafficColor;
}

string RouteInformation::GetStartupGuidanceText() const
{
    return m_startupInstruction;
}

DestinationStreetSide RouteInformation::GetDestinationStreetSide() const
{
    return m_streetSide;
}

Coordinates RouteInformation::GetLabelPoint() const
{
    return m_labelPoint;
}

string RouteInformation::GetFirstMajorRoad() const
{
    return m_firstMajorRoad;
}

vector<RouteSummaryInformation>& RouteInformation::GetRouteSummary()
{
    return m_routeSummarys;
}

const TrafficColorSegment& RouteInformation::GetTrafficColorSegment() const
{
    return m_trafficColorSegment;
}
    
const Maneuver* RouteInformation::GetManeuver(unsigned int index) const
{
    return m_maneuverList.GetManeuver(index);
}
    
unsigned int RouteInformation::GetNumberOfManeuvers() const
{
    return m_maneuverList.GetNumberOfManeuvers();
}

static inline Coordinates
SplitSegment(const Coordinates& P0, const Coordinates& P1, double reminder, double length)
{
    // insert a point at the start of the traffic event
    double interpolate = reminder / length;
    Coordinates coordinate = {P0.latitude * (1.0 - interpolate) + P1.latitude * interpolate,
        P0.longitude * (1.0 - interpolate) + P1.longitude * interpolate};
    return coordinate;
}

void RouteInformation::CreateTrafficColorSegment()
{
    m_trafficColorSegment.points.clear();
    m_trafficColorSegment.colorSegments.clear();
    m_trafficColorSegment.width = ROUTE_POLYLINE_WIDTH;
    // 1. Check if there is enough points....
    const vector<nbnav::Coordinates>& polyline = m_routePolylines;
    if (polyline.size() < 2)
    {
        return ;
    }
    //if not have traffic, let polylineParameter empty
    if (GetTrafficColor() == '\0' || GetTrafficColor() == 'U')
    {
        return;
    }
    m_trafficColorSegment.points.reserve(polyline.size());

    // 2. sort traffic evevt from near to far
    std::map<double, const nbnav::TrafficEvent*> sortedTraffics;
    std::vector<nbnav::TrafficEvent>::const_iterator iterTraffic = m_trafficEvents.begin();

    while (iterTraffic != m_trafficEvents.end())
    {
        const nbnav::TrafficEvent& trafficEvent = *iterTraffic;
        sortedTraffics.insert(std::make_pair(trafficEvent.GetStartFromTrip(), &trafficEvent));
        iterTraffic++;
    }

    // 3. split route into segment by event start point
    TrafficSegmentMap coloredSegments;
    map<double, const TrafficEvent*>::iterator iterSortedTraffic = sortedTraffics.begin();
    double distanceFromOrigin = 0.0;
    while (iterSortedTraffic != sortedTraffics.end())
    {
        const nbnav::TrafficEvent* pTrafficEvent = iterSortedTraffic->second;
        double startPoint = iterSortedTraffic->first;
        if (startPoint - distanceFromOrigin > LENGTH_THRESHOLD)
        {
            coloredSegments.insert(std::make_pair(distanceFromOrigin, TSC_Green));
        }
        TrafficSegmentColor color = TSC_Green;
        switch (pTrafficEvent->GetSeverity())
        {
            case nbnav::TrafficEvent::Moderate:
                color = TSC_Yellow;
                break;
            case nbnav::TrafficEvent::Severe:
                color = TSC_Red;
                break;
            default:
                color = TSC_Green;
        }
        coloredSegments.insert(std::make_pair(startPoint, color));
        distanceFromOrigin = startPoint + pTrafficEvent->GetLength();
        iterSortedTraffic++;
    }

    coloredSegments.insert(std::make_pair(distanceFromOrigin, TSC_Green));

    // 4. Walk through all points and create PolylineParameter
    size_t                  rawPolylineSize = polyline.size();
    size_t                  index           = 1;
    vector<Coordinates>& rPoints         = m_trafficColorSegment.points;
    rPoints.push_back(polyline[0]);
    vector<ColorSegmentAttribute>&      allColors   = m_trafficColorSegment.colorSegments;
    TrafficSegmentMap::iterator iterSegment = coloredSegments.begin();

    TrafficSegmentColor currentColor        = iterSegment->second;
    double           targetDistance      = iterSegment->first;
    double           accumulatedDistance = 0;
    double           realDistance        = 0;
    while (index < rawPolylineSize)
    {
        const Coordinates&     P    = rPoints.at(rPoints.size() - 1);
        Coordinates           P0   = {P.latitude, P.longitude};
        const Coordinates&    P1   = polyline[index];
        double                step = NavUtils::CalculateDistance(P0.latitude, P0.longitude,
                                                                 P1.latitude, P1.longitude,
                                                                 NULL);
        if (step == 0)
        {
            index ++;
            continue;
        }

        realDistance= step + accumulatedDistance;
        if (realDistance > targetDistance && targetDistance != 0 &&
            iterSegment != coloredSegments.end())
        {
            rPoints.push_back(SplitSegment(P0, P1, targetDistance - accumulatedDistance, step));
            allColors.push_back(ColorSegmentAttribute((uint32)rPoints.size()-1,
                                                          currentColor));
            accumulatedDistance = targetDistance;
            currentColor = iterSegment->second;
            if (++iterSegment != coloredSegments.end())
            {
                targetDistance = iterSegment->first;
            }
        }
        else
        {
            rPoints.push_back(P1);
            accumulatedDistance += step;
            index ++;
            if ((targetDistance == 0 || realDistance == targetDistance) &&
                ++iterSegment != coloredSegments.end())
            {
                targetDistance = iterSegment->first;
            }
        }
    }

    // Set color of the Polyline to green
    allColors.push_back(ColorSegmentAttribute((uint32)rPoints.size()-1, TSC_Green));
}


//////////////////////////////////////////////////////
//  class TrafficInformation
//////////////////////////////////////////////////////

TrafficInformation::TrafficInformation()
{
}

TrafficInformation::TrafficInformation(NavigationImpl* impl, void* routeData)
{
    vector<TrafficEvent> traffic;

    if (routeData)
    {
        uint32 count = 0;
        NB_TrafficEvent* nbEvents = NULL;

        if (NB_NavigationGetUpcomingTrafficEvents(impl->GetNbNavigation(),
                                                  &count, &nbEvents) == NE_OK && count > 0)
        {
            for (uint32 i = 0; i < count; ++i)
            {
                traffic.push_back(TrafficEvent(impl, &nbEvents[i], routeData));
            }
            (void)NB_NavigationFreeTrafficEvents(nbEvents);
        }
    }
    m_events = traffic;
}

vector<TrafficEvent> TrafficInformation::GetTrafficEvents() const
{
    return m_events;
}
    


}
