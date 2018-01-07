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
 * (C) Copyright 2013 by TeleCommunication Systems, Inc.
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

/*! @{ */

#include "NavAnalytics.h"
#include "analyticsmanager.h"
#include "NBProtocolAnalyticsEvent.h"

extern "C"
{
#include "data_gps.h"
#include "data_gps_offset.h"
}

using namespace nbnav;
using namespace std;
using namespace nbcommon;
using namespace protocol;

static const char* EMPTY_ID    = "";
static const char* PAO_List    = "list";
static const char* DEFAULT_STRING               = "default";
static const char* ANALYTICES_NAV_EVENT_CATEGORY= "2";
static const char* ROUTE_REQUEST_EVENT_NAME     = "route-request-event";
static const char* ROUTE_REPLY_EVENT_NAME       = "route-reply-event";
static const char* ROUTE_STATE_EVENT_NAME       = "route-state-event";
static const char* ARRIVAL_EVENT_NAME           = "arrival-event";
static const char* GPS_PROBES_EVENT_NAME        = "gps-probes-event";
static const char* ANNOUNCEMENT_EVENT_NAME      = "announcement-event";
static const char* PACK_VERSION                 = "1.1";

#define CHECK_NEW_PTR(T,X)                  \
    shared_ptr<T> X(new T);                 \
    if(!X)                                  \
    {                                       \
        return NE_NOMEM;                    \
    }

#define CHECK_PTR(X)                        \
    if(!X)                                  \
    {                                       \
        return NE_NOMEM;                    \
    }

NB_Error ConvertToGPSPosition(shared_ptr<Position> position, NB_GpsLocation* gpsLocation)
{
    if (!gpsLocation)
    {
        return NE_INVAL;
    }
    position->SetVariant(shared_ptr<string>(new std::string("gps")));
    CHECK_NEW_PTR(PackedGps, packedGps);
    CHECK_NEW_PTR(BinaryData, data);
    data_gps gps;
    data_gps_from_gpsfix(NULL, &gps, gpsLocation);
    data->SetData((byte*)gps.packed, sizeof(gps.packed));
    packedGps->SetPacked(data);
    position->SetGps(packedGps);
    return NE_OK;
}

NavAnalytics::NavAnalytics()
:   m_palInstance(NULL),
    m_context(NULL),
    m_requestRouteID(0),
    m_replyRouteID(0),
    m_isLastGpsProbeTimeValid(false),
    m_lastGpsProbeTime(0),
    m_defaultPlace(new AnalyticsEventPlace)
{
    ResetAnalyticsEventPlace();
}

NB_Error NavAnalytics::Initialize(PAL_Instance* palInstance,
                                  NB_Context* nbContext,
                                  shared_ptr<bool> isValid)
{
    nsl_assert(palInstance);
    nsl_assert(nbContext);

    m_palInstance = palInstance;
    m_context     = nbContext;
    //create analyticsManager.
    AnalyticsManager::GetAnalyticsManager(m_context);
    return NE_OK;
}

void NavAnalytics::StartAnalyticsSession(bool isStart)
{
    AnalyticsManager* manager = AnalyticsManager::GetAnalyticsManager(m_context);
    if (manager)
    {
        manager->KeepAlive(isStart);
    }
}

NB_Error NavAnalytics::AddRouteRequestEvent(const char* reason, uint32 navSessionId)
{
    if (!reason)
    {
        return NE_INVAL;
    }
    CHECK_NEW_PTR(RouteRequestEvent, routeRequestEvent);
    routeRequestEvent->SetReason(shared_ptr<string>(new string(reason)));
    routeRequestEvent->SetNavSessionId(navSessionId);
    CHECK_NEW_PTR(AnalyticsEventOrigin, origin);
    origin->SetAnalyticsEventPlace(m_defaultPlace);
    routeRequestEvent->SetAnalyticsEventOrigin(origin);
    CHECK_NEW_PTR(AnalyticsEventDestination, destination);
    destination->SetAnalyticsEventPlace(m_defaultPlace);
    routeRequestEvent->SetAnalyticsEventDestination(destination);

    CHECK_NEW_PTR(AnalyticsEvent, analyticsEvent);
    analyticsEvent->SetRouteRequestEvent(routeRequestEvent);
    m_requestRouteID = PAL_ClockGetTimeMs();
    return AddEvent(ROUTE_REQUEST_EVENT_NAME, analyticsEvent, NULL);
}

NB_Error NavAnalytics::AddRouteReplyEvent(uint32 navSessionId, uint32 duration, double distance)
{
    CHECK_NEW_PTR(RouteReplyEvent, routeReplyEvent);
    routeReplyEvent->SetNavSessionId(navSessionId);
    routeReplyEvent->SetRouteDuration(duration);
    routeReplyEvent->SetRouteDistance((float)distance);
    routeReplyEvent->SetRouteRequestEventId(m_requestRouteID);

    CHECK_NEW_PTR(AnalyticsEvent, analyticsEvent);
    analyticsEvent->SetRouteReplyEvent(routeReplyEvent);
    m_replyRouteID = PAL_ClockGetTimeMs();
    return AddEvent(ROUTE_REPLY_EVENT_NAME, analyticsEvent, NULL);
}

NB_Error NavAnalytics::AddRouteStateEvent(uint32 navSessionId, const char* state, uint32 duration,
                                          double distance, double distanceTraveled, NB_GpsLocation* location)
{
    if (!state)
    {
        return NE_INVAL;
    }
    CHECK_NEW_PTR(RouteStateEvent, routeStateEvent);
    routeStateEvent->SetNavSessionId(navSessionId);
    routeStateEvent->SetRouteRequestEventId(m_requestRouteID);
    routeStateEvent->SetRouteReplyEventId(m_replyRouteID);
    routeStateEvent->SetDuration(duration);
    routeStateEvent->SetDistance((float)distance);
    routeStateEvent->SetState(shared_ptr<string>(new string(state)));
    CHECK_NEW_PTR(Position, position);
    if (ConvertToGPSPosition(position, location) != NE_OK)
    {
        return NE_NOMEM;
    }
    routeStateEvent->SetPosition(position);

    CHECK_NEW_PTR(AnalyticsEvent, analyticsEvent);
    analyticsEvent->SetRouteStateEvent(routeStateEvent);
    return AddEvent(ROUTE_STATE_EVENT_NAME, analyticsEvent, state);
}

NB_Error NavAnalytics::AddArrivalEvent(uint32 navSessionId)
{
    CHECK_NEW_PTR(ArrivalEvent, arrivalEvent);
    m_defaultPlace->SetOrigin(shared_ptr<string>(new string(PAO_List)));
    arrivalEvent->SetAnalyticsEventPlace(m_defaultPlace);

    CHECK_NEW_PTR(AnalyticsEvent, analyticsEvent);
    analyticsEvent->SetArrivalEvent(arrivalEvent);
    ResetAnalyticsEventPlace();
    return AddEvent(ARRIVAL_EVENT_NAME, analyticsEvent, NULL);
}

NB_Error NavAnalytics::AddGpsProbesEvent(uint32 navSessionId, NB_GpsLocation& location)
{
    CHECK_NEW_PTR(GpsProbesEvent, gpsprobes);
    gpsprobes->SetPackingVersion(shared_ptr<string>(new string(PACK_VERSION)));
    gpsprobes->SetSessionId(navSessionId);
    //create and set gps
    CHECK_NEW_PTR(Gps, gps);
    CHECK_NEW_PTR(Point, point);
    point->SetLat(location.latitude);
    point->SetLon(location.longitude);

    gps->SetPoint(point);
    gps->SetHeading((float)location.heading);
    gps->SetAltitude((float)location.altitude);
    gps->SetSpeed((float)location.horizontalVelocity);
    gps->SetUa((float)location.horizontalUncertaintyAlongAxis);
    gps->SetUang((float)location.horizontalUncertaintyAngleOfAxis);
    gps->SetUp((float)location.horizontalUncertaintyAlongPerpendicular);
    gps->SetTime(location.gpsTime);

    gpsprobes->SetGps(gps);
    CHECK_NEW_PTR(AnalyticsEvent, analyticsEvent);
    analyticsEvent->SetGpsProbesEvent(gpsprobes);
    m_lastGpsProbeTime = location.gpsTime;
    m_isLastGpsProbeTimeValid = true;
    return AddEvent(GPS_PROBES_EVENT_NAME, analyticsEvent, NULL);
}

NB_Error NavAnalytics::AddAnnouncementEvent(uint32 navSessionId, string announcement, float distance,
                                            uint32 duration, TrafficAnnouncementType type,
                                            TrafficColor color, NB_GpsLocation* location)
{
    CHECK_NEW_PTR(AnnouncementEvent, announcementEvent);
    announcementEvent->SetAnnouncement(shared_ptr<string>(new string(announcement)));
    announcementEvent->SetDistance(distance);
    announcementEvent->SetDuration(duration);
    announcementEvent->SetNavSessionId(navSessionId);
    announcementEvent->SetRouteReplyEventId(m_replyRouteID);
    announcementEvent->SetRouteRequestEventId(m_requestRouteID);
    CHECK_NEW_PTR(Position, position);
    if (ConvertToGPSPosition(position, location) != NE_OK)
    {
        return NE_NOMEM;
    }
    announcementEvent->SetPosition(position);
    CHECK_NEW_PTR(TrafficAnanouncementInfo, trafficAnanouncementInfo);
    trafficAnanouncementInfo->SetTrafficColor((uint32)color);
    trafficAnanouncementInfo->SetType((uint32)type);
    announcementEvent->SetTrafficAnanouncementInfo(trafficAnanouncementInfo);

    CHECK_NEW_PTR(AnalyticsEvent, analyticsEvent);
    analyticsEvent->SetAnnouncementEvent(announcementEvent);
    return AddEvent(ANNOUNCEMENT_EVENT_NAME, analyticsEvent, NULL);
}

void NavAnalytics::ResetAnalyticsEventPlace()
{
    m_defaultPlace->SetGpsBased(true);
    shared_ptr<GoldenCookie> goldenCookie;
    m_defaultPlace->SetGoldenCookie(goldenCookie);
    shared_ptr<PlaceEventCookie> placeEventCookie;
    m_defaultPlace->SetPlaceEventCookie(placeEventCookie);
    m_defaultPlace->SetPremiumPlacement(false);
    m_defaultPlace->SetEnhancedPoi(false);
    shared_ptr<PoiAttributeCookie> poiAttributeCookie;
    m_defaultPlace->SetPoiAttributeCookie(poiAttributeCookie);
    m_defaultPlace->SetId(shared_ptr<string>(new string(EMPTY_ID)));
    m_defaultPlace->SetSearchQueryEventId(0);
    m_defaultPlace->SetIndex(0);
    m_defaultPlace->SetOrigin(shared_ptr<string>(new string(PAO_List)));
}

AnalyticsEventMetadataSharedPtr NavAnalytics::CreateMetaDate(const char* name, const char* routeState)
{
    AnalyticsEventMetadataSharedPtr metadata(new AnalyticsEventMetadata);
    if (metadata)
    {
        metadata->SetCategory(shared_ptr<std::string>(new string(ANALYTICES_NAV_EVENT_CATEGORY)));
        metadata->SetName(shared_ptr<std::string>(new string(name)));
        metadata->SetTimestamp(PAL_ClockGetTimeMs());
        if (routeState)
        {
            metadata->SetRouteState(shared_ptr<std::string>(new string(routeState)));
        }
    }
    return metadata;
}

InvocationContextSharedPtr NavAnalytics::CreateInvocation(const char* sourceModule,
                                                          const char* screenId,
                                                          const char* inputSource,
                                                          const char* invocationMethod)
{
    InvocationContextSharedPtr invocation(new protocol::InvocationContext);
    if (invocation && sourceModule && screenId && inputSource && invocationMethod)
    {
        invocation->SetSourceModule(shared_ptr<string>(new string(sourceModule)));
        invocation->SetScreenId(shared_ptr<string>(new string(screenId)));
        invocation->SetInputSource(shared_ptr<string>(new string(inputSource)));
        invocation->SetInvocationMethod(shared_ptr<string>(new string(invocationMethod)));
    }
    return invocation;
}

NB_Error NavAnalytics::AddEvent(const char* name, shared_ptr<AnalyticsEvent> event, const char* routeState)
{
    AnalyticsManager* manager = AnalyticsManager::GetAnalyticsManager(m_context);
    if (!manager)
    {
        return NE_NOTINIT;
    }
    /*! create Invocation object, as navigation do not need it, we use default value here */
    InvocationContextSharedPtr invocation = CreateInvocation(DEFAULT_STRING,
                                                             DEFAULT_STRING,
                                                             DEFAULT_STRING,
                                                             DEFAULT_STRING);
    CHECK_PTR(invocation);
    AnalyticsEventMetadataSharedPtr metadata = CreateMetaDate(name, routeState);
    CHECK_PTR(metadata);
    event->SetInvocationContext(invocation);
    event->SetEventMetadata(metadata);
    return manager->AddEvent(event);
}
