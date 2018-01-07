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

 @file     analyticsmanagerimpl.h

 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*!
 @{
 */

extern "C" {
    #include "palfile.h"
    #include "palclock.h"
    #include "paltaskqueue.h"
    #include "nbmacros.h"
    #include "nbcontextprotected.h"
    #include "nbpersistentdataprotected.h"
    #include "data_gps.h"
    #include "data_gps_offset.h"
}
#include "analyticsmanagerimpl.h"
#include "NBProtocolSettingsChangeEvent.h"
#include "NBProtocolShareEvent.h"
#include "NBProtocolGoldenCookie.h"
#include "NBProtocolGoldenCookieSerializer.h"
#include "NBProtocolPlaceEventCookie.h"
#include "NBProtocolPlaceEventCookieSerializer.h"
#include "NBProtocolPoiAttributeCookie.h"
#include "NBProtocolPoiAttributeCookieSerializer.h"
#include "NBProtocolUserActionEvent.h"
#include "NBProtocolSessionStartEvent.h"
#include <sstream>

using namespace nbcommon;
using namespace protocol;

#define CHECK_PTR(X)                        \
if(!X)                                  \
{                                       \
return NE_NOMEM;                    \
}

#define APPERROR_CATEGORY "4"
#define APPERROR_NAME "app-error-event"
#define SETTING_CATEGORY "7"
#define SETTING_NAME "settings-change-event"
#define ACTION_CATEGORY "9"
#define ACTION_NAME "user-action-event"
#define SHARE_CATEGORY "8"
#define SHARE_NAME "share-event"
#define FEEDBACK_CATEGORY "12"
#define FEEDBACK_NAME "feedback-event"
#define SESSION_CATEGORY "6"
#define SESSION_NAME "session-start-event"

struct AddEventData
{
    AddEventData(AnalyticsManagerImpl* self, shared_ptr<protocol::AnalyticsEvent> aEvent)
    {
        this->pThis = self;
        this->event = aEvent;
    }
    AnalyticsManagerImpl* pThis;
    shared_ptr<protocol::AnalyticsEvent> event;
};

struct KeepAliveData
{
    KeepAliveData(AnalyticsManagerImpl* self, bool value)
    {
        this->pThis = self;
        this->value = value;
    }
    AnalyticsManagerImpl* pThis;
    bool value;
};

struct AnalyticsListenerData
{
    AnalyticsListenerData(AnalyticsManagerImpl* self, AnalyticsListener* listener)
    {
        this->pThis = self;
        this->listener = listener;
    }
    AnalyticsManagerImpl* pThis;
    AnalyticsListener* listener;
};

AnalyticsManagerImpl::AnalyticsManagerImpl(NB_Context* context)
     :m_context(context),
      m_inited(false),
      m_sessionId(0),
      m_id(0)
{
}


void AnalyticsManagerImpl::AddEventInCCCThread(PAL_Instance* /*pal*/, void* userData)
{
    AddEventData* eventData = static_cast<AddEventData*>(userData);
    protocol::BinaryDataSharedPtr data = protocol::BinaryDataSharedPtr(new protocol::BinaryData);
    if(data)
    {
        shared_ptr<std::string> str = shared_ptr<std::string>(new std::string(eventData->pThis->GetSessionId()));
        if(str)
        {
            data->SetData((byte*)(str->c_str()), str->length());
        }
        eventData->event->SetActionSession(data);
        uint32 time = PAL_ClockGetGPSTime();
        eventData->event->SetTs(time);
        eventData->event->SetId(eventData->pThis->m_id++);
        eventData->event->SetSessionId(eventData->pThis->m_sessionId);
        const shared_ptr<std::string> xml = eventData->event->ToXML();
        if(xml)
        {
            AnalyticsEngine::GetInstance(eventData->pThis->m_context)->AddAnalyticsEvent(*xml);
        }
        delete eventData;
    }
}

void AnalyticsManagerImpl::AddAnalyticsListenerInCCCThread(PAL_Instance* /*pal*/, void* userData)
{
    AnalyticsListenerData* data = static_cast<AnalyticsListenerData*>(userData);
    if(data)
    {
        AnalyticsEngine::GetInstance(data->pThis->m_context)->SetAnalyticsListener(data->listener);
        delete data;
    }
}

/*! keep alive in ccc thread. */
void AnalyticsManagerImpl::KeepAliveInCCCThread(PAL_Instance* /*pal*/, void* userData)
{
    KeepAliveData* data = static_cast<KeepAliveData*>(userData);
    if(data)
    {
        AnalyticsEngine::GetInstance(data->pThis->m_context)->KeepAlive(data->value);
        delete data;
    }
}
/*! notice alive in ccc thread. */
void AnalyticsManagerImpl::NoticeAliveInCCCThread(PAL_Instance* /*pal*/, void* userData)
{
    AnalyticsManagerImpl* pThis = static_cast<AnalyticsManagerImpl*>(userData);
    if(pThis)
    {
        AnalyticsEngine::GetInstance(pThis->m_context)->NoticeAlive();
    }
}

void AnalyticsManagerImpl::InitializeInCCCThread(PAL_Instance* /*pal*/, void* userData)
{
    AnalyticsManagerImpl* pThis = (AnalyticsManagerImpl*)userData;
    //inited analytics engine
    char dbPath[PAL_FILE_MAX_NAME_LENGTH+1];
    char schema[PAL_FILE_MAX_NAME_LENGTH+1];
    const char* workpath = PAL_FileGetWorkPath();
    if(workpath)
    {
        nsl_strlcpy(dbPath, workpath, sizeof(dbPath));
        nsl_strlcpy(schema, workpath, sizeof(schema));
        PAL_FileAppendPath(NB_ContextGetPal(pThis->m_context), dbPath, PAL_FILE_MAX_NAME_LENGTH, "analytics.sqlite");
        PAL_FileAppendPath(NB_ContextGetPal(pThis->m_context), schema, PAL_FILE_MAX_NAME_LENGTH, "analytics.xsd");
        AnalyticsEngine::GetInstance(pThis->m_context)->Initialize(dbPath,schema);
    }
}

NB_Error AnalyticsManagerImpl::Initialize()
{
    if(!m_inited)
    {
        if(m_context)
        {
            m_inited = true;
            //inited id
            m_sessionId = PAL_ClockGetGPSTime();
            m_id = 1;
            TaskId id;
            PAL_EventTaskQueueAdd(NB_ContextGetPal(m_context), InitializeInCCCThread, this, &id);
            return NE_OK;
        }
        else
        {
            return NE_NOTINIT;
        }
    }
    else
    {
        return NE_OK;
    }
}

NB_Error AnalyticsManagerImpl::AddEvent(const shared_ptr<protocol::AnalyticsEvent> event)
{
    TaskId id;
    PAL_EventTaskQueueAdd(NB_ContextGetPal(m_context), AddEventInCCCThread, CCC_NEW AddEventData(this, event), &id);
    return NE_OK;
}

NB_Error AnalyticsManagerImpl::AddAppGpsErrorEvent(const nbcommon::InvocationContext& invocation,
                                                   NB_Error errorCode,
                                                   const std::string& detailedCode,
                                                   const std::string& detailedDescription,
                                                   const std::string& gpsMode,
                                                   const std::string& gpsFixType
                                                   )
{
    shared_ptr<AppErrorEvent> appError = shared_ptr<AppErrorEvent>(new AppErrorEvent);
    CHECK_PTR(appError);
    shared_ptr<ErrorCategory> category = shared_ptr<ErrorCategory>(new ErrorCategory);
    CHECK_PTR(category);
    shared_ptr<std::string> str = shared_ptr<std::string>(new std::string("gps-data"));
    CHECK_PTR(str);
    category->SetType(str);
    appError->SetErrorCategory(category);
    appError->SetErrorCode(errorCode);
    str = shared_ptr<std::string>(new std::string(detailedCode));
    CHECK_PTR(str);
    appError->SetDetailedCode(shared_ptr<std::string>(str));
    str = shared_ptr<std::string>(new std::string(detailedDescription));
    CHECK_PTR(str);
    appError->SetDetailedDescription(shared_ptr<std::string>(str));
    shared_ptr<Pair> mode = shared_ptr<Pair>(new Pair);
    str = shared_ptr<std::string>(new std::string("mode"));
    CHECK_PTR(str);
    mode->SetKey(shared_ptr<std::string>(str));
    str = shared_ptr<std::string>(new string(gpsMode));
    CHECK_PTR(str);
    mode->SetValue(shared_ptr<std::string>(str));
    shared_ptr<Pair> fixType = shared_ptr<Pair>(new Pair);
    fixType->SetKey(shared_ptr<std::string>(new string("fix-type")));
    CHECK_PTR(fixType->GetKey());
    fixType->SetValue(shared_ptr<std::string>(new string(gpsFixType)));
    CHECK_PTR(fixType->GetValue());
    appError->GetPairArray()->push_back(mode);
    appError->GetPairArray()->push_back(fixType);
    protocol::AnalyticsEventMetadataSharedPtr metadata;
    NB_Error err = GetAnalyticsMetadata(APPERROR_NAME, APPERROR_CATEGORY, metadata);
    if(err != NE_OK)
    {
        return err;
    }
    protocol::AnalyticsEventSharedPtr analytics = protocol::AnalyticsEventSharedPtr(new protocol::AnalyticsEvent);
    CHECK_PTR(analytics);
    analytics->SetAppErrorEvent(appError);
    return SendAnalyticsEvent(invocation, metadata, analytics);
}

NB_Error AnalyticsManagerImpl::AddSettingChangeEvent(const nbcommon::InvocationContext& invocation,
                                                     const GeneralSetting& generalSetting,
                                                     const NavigationSetting& navSetting,
                                                     const RouteSetting& routeSetting,
                                                     const MapSetting& mapSetting)
{
    shared_ptr<SettingsChangeEvent> event = shared_ptr<SettingsChangeEvent>(new SettingsChangeEvent);
    CHECK_PTR(event);
    //general setting
    shared_ptr<protocol::GeneralSettings> general = shared_ptr<protocol::GeneralSettings>(new protocol::GeneralSettings);
    CHECK_PTR(general);
    general->SetDistanceDisplay(generalSetting.distance);
    general->SetNightMode(generalSetting.mode);
    event->SetGeneralSetting(general);
    //nav setting
    shared_ptr<protocol::NavigationSettings> nav = shared_ptr<protocol::NavigationSettings>(new protocol::NavigationSettings);
    CHECK_PTR(nav);
    nav->SetHighwaySigns(navSetting.hightWaySigns);
    nav->SetSpeedAlerts(navSetting.speedAlerts);
    nav->SetSpeedLimitSigns(navSetting.speedLimitSigns);
    nav->SetSpeedWarningTone(navSetting.speedWarningTone);
    nav->SetWarningSpeed(navSetting.warningSpeed);
    event->SetNavigationSetting(nav);
    //route setting
    shared_ptr<protocol::RouteSettings> route = shared_ptr<protocol::RouteSettings>(new protocol::RouteSettings);
    CHECK_PTR(route);
    route->SetAvoidCarPool(routeSetting.avoidCarPool);
    route->SetAvoidFerries(routeSetting.avoidFerries);
    route->SetAvoidHighways(routeSetting.avoidHighWays);
    route->SetAvoidTolls(routeSetting.avoidTolls);
    route->SetVehicleMode(routeSetting.mode);
    event->SetRouteSetting(route);
    //map setting
    shared_ptr<protocol::MapSettings> map = shared_ptr<protocol::MapSettings>(new protocol::MapSettings);
    CHECK_PTR(map);
    map->SetDopplerLayer(mapSetting.dopperLayer);
    map->SetSatelliteLayer(mapSetting.satelliteLayer);
    map->SetTrafficLayer(mapSetting.trafficLayer);
    event->SetMapsettings(map);
    protocol::AnalyticsEventMetadataSharedPtr metadata;
    NB_Error err = GetAnalyticsMetadata(SETTING_NAME, SETTING_CATEGORY, metadata);
    if(err != NE_OK)
    {
        return err;
    }
    protocol::AnalyticsEventSharedPtr analytics = protocol::AnalyticsEventSharedPtr(new protocol::AnalyticsEvent);
    CHECK_PTR(analytics);
    analytics->SetSettingsChangeEvent(event);
    return SendAnalyticsEvent(invocation, metadata, analytics);
}

NB_Error AnalyticsManagerImpl::AddFeedbackEvent(const nbcommon::InvocationContext& invocation,
                                                const AnalyticsFeedbackEvent& feedback)
{
    shared_ptr<protocol::FeedbackEvent> feedBackEvent = shared_ptr<protocol::FeedbackEvent>(new protocol::FeedbackEvent);
    CHECK_PTR(feedBackEvent);
    shared_ptr<protocol::Place> place;
    NB_Error err = GetPlace(*feedback.poiPlace, place);
    if(err != NE_OK)
    {
        return err;
    }
    feedBackEvent->SetPlace(place);
    //origin
    shared_ptr<protocol::Origin> origin = shared_ptr<protocol::Origin>(new protocol::Origin);
    CHECK_PTR(origin);
    shared_ptr<protocol::Point> point = shared_ptr<protocol::Point>(new protocol::Point);
    CHECK_PTR(point);
    point->SetLat(feedback.originLocation->latitude);
    point->SetLon(feedback.originLocation->longitude);
    origin->SetPoint(point);
    origin->SetStreetAddress(shared_ptr<std::string>(new string(feedback.originLocation->freeform)));
    CHECK_PTR(origin->GetStreetAddress());
    origin->SetStreetName(shared_ptr<std::string>(new string(feedback.originLocation->street1)));
    CHECK_PTR(origin->GetStreetName());
    feedBackEvent->SetOrigin(origin);
    //dest
    shared_ptr<protocol::Destination> dest = shared_ptr<protocol::Destination>(new protocol::Destination);
    CHECK_PTR(dest);
    point = shared_ptr<protocol::Point>(new protocol::Point);
    CHECK_PTR(point);
    point->SetLat(feedback.destinationLocation->latitude);
    point->SetLon(feedback.destinationLocation->longitude);
    dest->SetPoint(point);
    dest->SetStreetAddress(shared_ptr<std::string>(new string(feedback.destinationLocation->freeform)));
    CHECK_PTR(dest->GetStreetAddress());
    dest->SetStreetName(shared_ptr<std::string>(new string(feedback.destinationLocation->street1)));
    CHECK_PTR(dest->GetStreetName());
    feedBackEvent->SetDestination(dest);
    //issue type
    shared_ptr<protocol::IssueType> issue = shared_ptr<protocol::IssueType>(new protocol::IssueType);
    CHECK_PTR(issue);
    issue->SetType(shared_ptr<std::string>(new string(feedback.issueType)));
    CHECK_PTR(issue->GetType());
    feedBackEvent->GetIssueTypeArray()->push_back(issue);
    feedBackEvent->SetScreenId(shared_ptr<std::string>(new string(feedback.screenId)));
    CHECK_PTR(feedBackEvent->GetScreenId());
    feedBackEvent->SetProviderId(shared_ptr<std::string>(new string(feedback.providerId)));
    CHECK_PTR(feedBackEvent->GetProviderId());
    feedBackEvent->SetPoiId(shared_ptr<std::string>(new string(feedback.poiId)));
    CHECK_PTR(feedBackEvent->GetPoiId());
    shared_ptr<protocol::Text> text = shared_ptr<protocol::Text>(new protocol::Text);
    CHECK_PTR(text);
    text->SetData(shared_ptr<std::string>(new string(feedback.enteredText)));
    CHECK_PTR(text->GetData());
    feedBackEvent->SetText(text);
    feedBackEvent->SetNavSessionId(feedback.navSessionId);
    feedBackEvent->SetRouteRequestEventId(feedback.routeRequestEventId);
    feedBackEvent->SetSearchQueryEventId(feedback.searchQueryEventId);
    protocol::AnalyticsEventMetadataSharedPtr metadata;
    err = GetAnalyticsMetadata(FEEDBACK_NAME, FEEDBACK_CATEGORY, metadata);
    if(err != NE_OK)
    {
        return err;
    }
    protocol::AnalyticsEventSharedPtr analytics = protocol::AnalyticsEventSharedPtr(new protocol::AnalyticsEvent);
    CHECK_PTR(analytics);
    analytics->SetFeedbackEvent(feedBackEvent);
    return SendAnalyticsEvent(invocation, metadata, analytics);
}

NB_Error AnalyticsManagerImpl::AddSessionStartEvent(const nbcommon::InvocationContext& invocation,
                                                    const NB_GpsLocation& location)
{
    protocol::SessionStartEventSharedPtr session = protocol::SessionStartEventSharedPtr(new protocol::SessionStartEvent);
    CHECK_PTR(session);
    protocol::PositionSharedPtr pos = protocol::PositionSharedPtr(new protocol::Position);
    CHECK_PTR(pos);
    shared_ptr<std::string> str = shared_ptr<std::string>(new std::string("gps"));
    CHECK_PTR(str);
    pos->SetVariant(str);
    shared_ptr<protocol::PackedGps> gpsPacket = shared_ptr<protocol::PackedGps>(new protocol::PackedGps);
    CHECK_PTR(gpsPacket);
    data_gps gps;
    NB_GpsLocation tmp = location;
    data_gps_from_gpsfix(NULL, &gps, &tmp);
    shared_ptr<protocol::BinaryData> data = shared_ptr<protocol::BinaryData>(new protocol::BinaryData);
    CHECK_PTR(data);
    data->SetData((byte*)gps.packed, sizeof(gps.packed));
    gpsPacket->SetPacked(data);
    pos->SetGps(gpsPacket);
    session->SetPosition(pos);
    protocol::AnalyticsEventMetadataSharedPtr metadata;
    NB_Error err = GetAnalyticsMetadata(SESSION_NAME, SESSION_CATEGORY, metadata);
    if(err != NE_OK)
    {
        return err;
    }
    protocol::AnalyticsEventSharedPtr analytics = protocol::AnalyticsEventSharedPtr(new protocol::AnalyticsEvent);
    CHECK_PTR(analytics);
    analytics->SetSessionStartEvent(session);
    return SendAnalyticsEvent(invocation, metadata, analytics);
}

NB_Error AnalyticsManagerImpl::AddUserActionEvent(const nbcommon::InvocationContext& invocation,
                                                  UserAction action)
{
    protocol::UserActionEventSharedPtr actionEvent = protocol::UserActionEventSharedPtr(new protocol::UserActionEvent);
    CHECK_PTR(actionEvent);
    shared_ptr<std::string> actionName = shared_ptr<std::string>(new std::string(""));
    CHECK_PTR(actionName);
    switch (action)
    {
        case AnalyticsManager::UA_NavTripOverview:
            actionName->assign("nav-trip-overview");
            break;
        case AnalyticsManager::UA_NavTurnList:
            actionName->assign("nav-turn-list");
            break;
        case AnalyticsManager::UA_SearchBoxSelected:
            actionName->assign("search-box-selected");
            break;
        case AnalyticsManager::UA_SessionCleared:
            actionName->assign("search-session-cleared");
            break;
        default:
            break;
    }
    if(!actionName->empty())
    {
        actionEvent->SetActionName(actionName);
    }
    protocol::AnalyticsEventMetadataSharedPtr metadata;
    NB_Error err = GetAnalyticsMetadata(ACTION_NAME, ACTION_CATEGORY, metadata);
    if(err != NE_OK)
    {
        return err;
    }
    protocol::AnalyticsEventSharedPtr analytics = protocol::AnalyticsEventSharedPtr(new protocol::AnalyticsEvent);
    CHECK_PTR(analytics);
    analytics->SetUserActionEvent(actionEvent);
    return SendAnalyticsEvent(invocation, metadata, analytics);
}

NB_Error AnalyticsManagerImpl::AddShareEvent(const nbcommon::InvocationContext& invocation,
                                             const std::string& subject,
                                             const std::string& body,
                                             const std::string& url,
                                             const std::string& urlText,
                                             const std::string& signature,
                                             uint channnel,
                                             const AnalyticsEventPlace* place)
{
    shared_ptr<protocol::ShareEvent> share = shared_ptr<protocol::ShareEvent>(new protocol::ShareEvent);
    CHECK_PTR(share);
    shared_ptr<std::string> str = shared_ptr<std::string>(new std::string(subject));
    CHECK_PTR(str);
    share->SetSubject(str);
    str = shared_ptr<std::string>(new std::string(body));
    CHECK_PTR(str);
    share->SetBody(str);
    str = shared_ptr<std::string>(new std::string(url));
    CHECK_PTR(str);
    share->SetUrl(str);
    str = shared_ptr<std::string>(new std::string(urlText));
    CHECK_PTR(str);
    share->SetUrlText(str);
    str = shared_ptr<std::string>(new std::string(signature));
    CHECK_PTR(str);
    share->SetSignature(str);
    share->SetChannel(channnel);
    if(place)
    {
        protocol::AnalyticsEventPlaceSharedPtr eventPlace;
        NB_Error error = GetAnalyticsPlace(*place, eventPlace);
        if(error == NE_OK)
        {
            share->SetAnalyticsEventPlace(eventPlace);
        }
    }
    protocol::AnalyticsEventMetadataSharedPtr metadata;
    NB_Error err = GetAnalyticsMetadata(SHARE_NAME, SHARE_CATEGORY, metadata);
    if(err != NE_OK)
    {
        return err;
    }
    protocol::AnalyticsEventSharedPtr analytics = protocol::AnalyticsEventSharedPtr(new protocol::AnalyticsEvent);
    CHECK_PTR(analytics);
    analytics->SetShareEvent(share);
    return SendAnalyticsEvent(invocation, metadata, analytics);
}

void AnalyticsManagerImpl::NoticeAlive()
{
    TaskId id;
    PAL_EventTaskQueueAdd(NB_ContextGetPal(m_context), NoticeAliveInCCCThread, this, &id);
}

void AnalyticsManagerImpl::KeepAlive(bool alive)
{
    TaskId id;
    PAL_EventTaskQueueAdd(NB_ContextGetPal(m_context), KeepAliveInCCCThread, CCC_NEW KeepAliveData(this, alive), &id);
}

std::string AnalyticsManagerImpl::GetSessionId()
{
    return AnalyticsEngine::GetInstance(m_context)->GetSessionId();
}

NB_Error AnalyticsManagerImpl::SetAnalyticsListener(AnalyticsListener* listener)
{
    TaskId id;
    PAL_EventTaskQueueAdd(NB_ContextGetPal(m_context), AddAnalyticsListenerInCCCThread, CCC_NEW AnalyticsListenerData(this, listener), &id);
    return NE_OK;
}

NB_Error AnalyticsManagerImpl::GetPlace(const NB_Place& place, shared_ptr<protocol::Place>& proPlace)
{
    proPlace = shared_ptr<protocol::Place>(new protocol::Place);
    CHECK_PTR(proPlace);
    shared_ptr<protocol::Location> location;
    NB_Error err = GetLocation(place.location, location);
    if(err != NE_OK)
    {
        return err;
    }
    proPlace->SetLocation(location);
    proPlace->SetName(shared_ptr<std::string>(new string(place.name)));
    CHECK_PTR(proPlace->GetName());
    std::ostringstream id;
    id << place.placeID;
    proPlace->SetUniqueID(shared_ptr<std::string>(new string(id.str())));
    CHECK_PTR(proPlace->GetUniqueID());
    for(int i = 0; i < place.numphone; i++)
    {
        shared_ptr<protocol::Phone> phone = shared_ptr<protocol::Phone>(new protocol::Phone);
        CHECK_PTR(phone);
        phone->SetAreaCode(shared_ptr<std::string>(new string(place.phone[i].area)));
        CHECK_PTR(phone->GetAreaCode());
        phone->SetCountryCode(shared_ptr<std::string>(new string(place.phone[i].country)));
        CHECK_PTR(phone->GetCountryCode());
        phone->SetNumber(shared_ptr<std::string>(new string(place.phone[i].number)));
        CHECK_PTR(phone->GetNumber());
        switch (place.phone[i].type) {
            case NB_Phone_Primary:
                phone->SetKind(shared_ptr<std::string>(new string("primary")));
                break;
            case NB_Phone_Secondary:
                phone->SetKind(shared_ptr<std::string>(new string("secondary")));
                break;
            case NB_Phone_National:
                phone->SetKind(shared_ptr<std::string>(new string("national")));
                break;
            case NB_Phone_Mobile:
                phone->SetKind(shared_ptr<std::string>(new string("mobile")));
                break;
            case NB_Phone_Fax:
                phone->SetKind(shared_ptr<std::string>(new string("fax")));
                break;
        }
        CHECK_PTR(phone->GetKind());
        proPlace->GetPhoneArray()->push_back(phone);
    }
    for(int i = 0; i< place.numcategory; i++)
    {
        shared_ptr<protocol::Category> category = shared_ptr<protocol::Category>(new protocol::Category);
        CHECK_PTR(category);
        category->SetCategoryCode(shared_ptr<std::string>(new string(place.category[i].code)));
        CHECK_PTR(category->GetCategoryCode());
        category->SetCategoryName(shared_ptr<std::string>(new string(place.category[i].name)));
        CHECK_PTR(category->GetCategoryName());
        proPlace->GetCategoryArray()->push_back(category);
    }
    return NE_OK;
}

NB_Error AnalyticsManagerImpl::GetLocation(const NB_Location& location, shared_ptr<protocol::Location>& proLocation)
{
    proLocation = shared_ptr<protocol::Location>(new protocol::Location);
    CHECK_PTR(proLocation);
    shared_ptr<protocol::Address> address = shared_ptr<protocol::Address>(new protocol::Address);
    CHECK_PTR(address);
    address->SetType(shared_ptr<std::string> (new string()));
    CHECK_PTR(address->GetType());
    address->SetStreetAddress(shared_ptr<std::string> (new string(location.street2)));
    CHECK_PTR(address->GetStreetAddress());
    address->SetCrossStreet(shared_ptr<std::string> (new string(location.street2)));
    CHECK_PTR(address->GetCrossStreet());
    address->SetAirport(shared_ptr<std::string>(new string(location.airport)));
    CHECK_PTR(address->GetAirport());
    address->SetCity(shared_ptr<std::string>(new string(location.city)));
    CHECK_PTR(address->GetCity());
    if(location.compactAddress.lineCount > 0)
    {
        shared_ptr<protocol::CompactAddress> comAddress = shared_ptr<protocol::CompactAddress>(new protocol::CompactAddress);
        CHECK_PTR(comAddress);
        if(location.compactAddress.hasLineSeparator)
        {
            shared_ptr<LineSeparator> separator = shared_ptr<LineSeparator>(new LineSeparator);
            CHECK_PTR(separator);
            separator->SetSeparator(shared_ptr<std::string>(new string(location.compactAddress.lineSeparator)));
            CHECK_PTR(separator->GetSeparator());
            comAddress->SetLineSeparator(separator);
        }
        shared_ptr<protocol::AddressLine> line = shared_ptr<protocol::AddressLine>(new protocol::AddressLine);
        CHECK_PTR(line);
        line->SetText(shared_ptr<std::string>(new string(location.compactAddress.addressLine1)));
        CHECK_PTR(line->GetText());
        comAddress->GetAddressLineArray()->push_back(line);
        if(location.compactAddress.lineCount == 2)
        {
            shared_ptr<protocol::AddressLine> line2 = shared_ptr<protocol::AddressLine>(new protocol::AddressLine);
            CHECK_PTR(line2);
            line2->SetText(shared_ptr<std::string>(new string(location.compactAddress.addressLine2)));
            CHECK_PTR(line2->GetText());
            comAddress->GetAddressLineArray()->push_back(line2);
        }
        address->SetCompactAddress(comAddress);
    }
    address->SetCountry(shared_ptr<std::string>(new string(location.country)));
    CHECK_PTR(address->GetCountry());
    address->SetCountryName(shared_ptr<std::string>(new string(location.country_name)));
    CHECK_PTR(address->GetCountryName());
    address->SetCounty(shared_ptr<std::string>(new string(location.county)));
    CHECK_PTR(address->GetCounty());
    if(location.extendedAddress.lineCount > 0)
    {
        shared_ptr<protocol::ExtendedAddress> extAddress = shared_ptr<protocol::ExtendedAddress>(new protocol::ExtendedAddress);
        CHECK_PTR(extAddress);
        if(location.extendedAddress.lineCount >= 1)
        {
            shared_ptr<protocol::AddressLine> line = shared_ptr<protocol::AddressLine>(new protocol::AddressLine);
            CHECK_PTR(line);
            line->SetText(shared_ptr<std::string>(new string(location.extendedAddress.addressLine1)));
            CHECK_PTR(line->GetText());
            extAddress->GetAddressLineArray()->push_back(line);
        }
        if(location.extendedAddress.lineCount >= 2)
        {
            shared_ptr<protocol::AddressLine> line = shared_ptr<protocol::AddressLine>(new protocol::AddressLine);
            CHECK_PTR(line);
            line->SetText(shared_ptr<std::string>(new string(location.extendedAddress.addressLine2)));
            CHECK_PTR(line->GetText());
            extAddress->GetAddressLineArray()->push_back(line);
        }
        if(location.extendedAddress.lineCount >= 3)
        {
            shared_ptr<protocol::AddressLine> line = shared_ptr<protocol::AddressLine>(new protocol::AddressLine);
            CHECK_PTR(line);
            line->SetText(shared_ptr<std::string>(new string(location.extendedAddress.addressLine3)));
            CHECK_PTR(line->GetText());
            extAddress->GetAddressLineArray()->push_back(line);
        }
        if(location.extendedAddress.lineCount >= 4)
        {
            shared_ptr<protocol::AddressLine> line = shared_ptr<protocol::AddressLine>(new protocol::AddressLine);
            CHECK_PTR(line);
            line->SetText(shared_ptr<std::string>(new string(location.extendedAddress.addressLine4)));
            CHECK_PTR(line->GetText());
            extAddress->GetAddressLineArray()->push_back(line);
        }
        if(location.extendedAddress.lineCount >= 5)
        {
            shared_ptr<protocol::AddressLine> line = shared_ptr<protocol::AddressLine>(new protocol::AddressLine);
            CHECK_PTR(line);
            line->SetText(shared_ptr<std::string>(new string(location.extendedAddress.addressLine5)));
            CHECK_PTR(line->GetText());
            extAddress->GetAddressLineArray()->push_back(line);
        }
        address->SetExtendedAddress(extAddress);
    }
    address->SetFreeform(shared_ptr<std::string>(new string(location.freeform)));
    CHECK_PTR(address->GetFreeform());
    address->SetPostal(shared_ptr<std::string>(new string(location.postal)));
    CHECK_PTR(address->GetPostal());
    address->SetState(shared_ptr<std::string>(new string(location.state)));
    CHECK_PTR(address->GetState());
    address->SetStreet(shared_ptr<std::string>(new string(location.street1)));
    CHECK_PTR(address->GetStreet());
    proLocation->SetAddress(address);
    proLocation->SetName(shared_ptr<std::string>(new string(location.areaname)));
    shared_ptr<protocol::Point> point = shared_ptr<protocol::Point>(new protocol::Point);
    CHECK_PTR(point);
    point->SetLat(location.latitude);
    point->SetLon(location.longitude);
    proLocation->SetPoint(point);
    return NE_OK;
}

NB_Error AnalyticsManagerImpl::GetAnalyticsPlace(const AnalyticsEventPlace& place,
                                                 shared_ptr<protocol::AnalyticsEventPlace>& analyticsPlace)
{
    analyticsPlace = shared_ptr<protocol::AnalyticsEventPlace>(new protocol::AnalyticsEventPlace);
    CHECK_PTR(analyticsPlace);
    analyticsPlace->SetGpsBased(place.isGpsBased == TRUE);
    analyticsPlace->SetPremiumPlacement(place.isPremiumPlacement == TRUE);
    analyticsPlace->SetEnhancedPoi(place.isEnhancedPoi == TRUE);
    shared_ptr<std::string> str = shared_ptr<std::string>(new std::string(place.id));
    CHECK_PTR(str);
    analyticsPlace->SetId(str);
    str = shared_ptr<std::string>(new std::string(place.origin));
    CHECK_PTR(str);
    analyticsPlace->SetOrigin(str);
    analyticsPlace->SetSearchQueryEventId(place.searchQueryEventId);
    analyticsPlace->SetIndex(place.searchResultsIndex);
    //cookies is optional
    if(!place.goldenCookie.empty())
    {
        protocol::GoldenCookieSharedPtr golden = protocol::GoldenCookieSerializer::deserialize(CreateTpsElementByBase64Data(place.goldenCookie));
        if(golden)
        {
            analyticsPlace->SetGoldenCookie(golden);
        }
    }
    if(!place.placeEventCookie.empty())
    {
        protocol::PlaceEventCookieSharedPtr eventCookie = protocol::PlaceEventCookieSerializer::deserialize(CreateTpsElementByBase64Data(place.placeEventCookie));
        if(eventCookie)
        {
            analyticsPlace->SetPlaceEventCookie(eventCookie);
        }
    }
    if(!place.placeAttributeCookie.empty())
    {
        protocol::PoiAttributeCookieSharedPtr poiCookie = protocol::PoiAttributeCookieSerializer::deserialize(CreateTpsElementByBase64Data(place.placeAttributeCookie));
        if(poiCookie)
        {
            analyticsPlace->SetPoiAttributeCookie(poiCookie);
        }
    }

    return NE_OK;
}

NB_Error AnalyticsManagerImpl::GetAnalyticsMetadata(const std::string& name, const std::string& category, protocol::AnalyticsEventMetadataSharedPtr& metadata)
{
    metadata = protocol::AnalyticsEventMetadataSharedPtr(new protocol::AnalyticsEventMetadata);
    CHECK_PTR(metadata);
    shared_ptr<std::string> str = shared_ptr<std::string>(new std::string(name));
    CHECK_PTR(str);
    metadata->SetName(str);
    str = shared_ptr<std::string>(new std::string(category));
    CHECK_PTR(str);
    metadata->SetCategory(str);
    metadata->SetTimestamp((uint64)PAL_ClockGetTimeMs());
    return NE_OK;
}

NB_Error AnalyticsManagerImpl::GetInvocation(const nbcommon::InvocationContext& context, shared_ptr<protocol::InvocationContext>& invocation)
{
    invocation = protocol::InvocationContextSharedPtr(new protocol::InvocationContext);
    CHECK_PTR(invocation);
    shared_ptr<std::string> str = shared_ptr<std::string>(new std::string(context.sourceModule));
    CHECK_PTR(str);
    invocation->SetInputSource(str);
    str = shared_ptr<std::string>(new std::string(context.invocationMethod));
    CHECK_PTR(str);
    invocation->SetInvocationMethod(str);
    str = shared_ptr<std::string>(new std::string(context.screenID));
    CHECK_PTR(str);
    invocation->SetScreenId(str);
    str = shared_ptr<std::string>(new std::string(context.sourceModule));
    CHECK_PTR(str);
    invocation->SetSourceModule(str);
    return NE_OK;
}

NB_Error AnalyticsManagerImpl::SendAnalyticsEvent(const nbcommon::InvocationContext& context, const protocol::AnalyticsEventMetadataSharedPtr& metadata, shared_ptr<protocol::AnalyticsEvent> analytics)
{
    if(!analytics)
    {
        return NE_BADDATA;
    }
    protocol::InvocationContextSharedPtr invocation;
    NB_Error err = GetInvocation(context, invocation);
    if(err != NE_OK)
    {
        return err;
    }
    analytics->SetInvocationContext(invocation);
    analytics->SetEventMetadata(metadata);
    return AddEvent(analytics);
}

protocol::TpsElementPtr AnalyticsManagerImpl::CreateTpsElementByBase64Data(const std::string& serializedData)
{
    protocol::TpsElementPtr tpsElement;

    if (serializedData.empty())
    {
        return tpsElement;
    }

    uint8* decodedSerializedData = (uint8*)nsl_malloc(serializedData.length()); // Base64-encoded data size is about 135.1% of Original data size. (See also Wiki)
    // So, serializedData.length() is enough for decoded data.
    if (decodedSerializedData == NULL)
    {
        return tpsElement;
    }

    nsl_memset(decodedSerializedData, 0, serializedData.length());
    int decodeSerializedDataSize = decode_base64(serializedData.c_str(), (char*)decodedSerializedData);
    if (decodeSerializedDataSize == -1)
    {
        nsl_free(decodedSerializedData);
        return tpsElement;
    }

    NB_PersistentData* persistentData = NULL;
    NB_Error err = NB_PersistentDataCreate(decodedSerializedData, decodeSerializedDataSize, NULL, &persistentData);
    nsl_free(decodedSerializedData);

    if (err != NE_OK)
    {
        return tpsElement;
    }

    tpselt tps = NULL;
    err = NB_PersistentDataGetToTpsElement(persistentData, &tps);
    if (err != NE_OK)
    {
        NB_PersistentDataDestroy(persistentData);
        return tpsElement;
    }

    tpsElement.reset(new protocol::TpsElement(tps));

    NB_PersistentDataDestroy(persistentData);
    return tpsElement;
}

/*!
 @}
 */
