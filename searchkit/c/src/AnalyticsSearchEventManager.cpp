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
 @file     AnalyticsSearchEventManager.cpp
 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

#include "contextbasedsingleton.h"
#include "AnalyticsSearchEventTask.h"
#include "AnalyticsSearchEvent.h"
#include "AnalyticsSearchEventManagerPrivate.h"
#include "AnalyticsSearchEventManager.h"
#include "analyticsmanager.h"
#include "CommonFunction.h"
#include "NBProtocolAnalyticsEvent.h"
#include "palclock.h"
#include "offboard/SingleSearchOffboardInformation.h"
#include "StringUtility.h"

namespace nbsearch
{

// AnalyticsSearchEvent
//
Analytics::Analytics(NB_Context* context): m_context(context)
{
    m_eventQueue.reset(new EventTaskQueue(NB_ContextGetPal(m_context)));
}

shared_ptr<Analytics> Analytics::GetInstance(NB_Context* context)
{
    return ContextBasedSingleton<Analytics>::getInstance(context);
}

void Analytics::AddEvent(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin, AnalyticsEventType eventType, const AnalyticsEventInvocationContextSharedPtr& invocationContext)
{
    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new AnalyticsSearchEventTaskRecord(this,
                                                                 &Analytics::CCC_Event,
                                                                 analyticsItem,
                                                                 origin,
                                                                 eventType,
                                                                 invocationContext));
    }
}

void Analytics::CCC_Event(const AnalyticsSearchEventTask* task)
{
    const AnalyticsSearchEventTaskRecord* pThis = static_cast<const AnalyticsSearchEventTaskRecord*>(task);

    AnalyticsManager::GetInstance(m_context)->AddEvent(pThis->m_analyticsItem, pThis->m_origin, pThis->m_eventType, pThis->m_invocationContext);
}

// AnalyticsSearchEventManager
//
AnalyticsManager::AnalyticsManager(NB_Context* context)
{
    m_private.reset(new AnalyticsManagerPrivate(context));
}

AnalyticsManager::~AnalyticsManager()
{
    m_private.reset();
}

shared_ptr<AnalyticsManager> AnalyticsManager::GetInstance(NB_Context* context)
{
    return ContextBasedSingleton<AnalyticsManager>::getInstance(context);
}

NB_Context* AnalyticsManager::GetContext() const
{
    return m_private->GetContext();
}

void AnalyticsManager::AddEvent(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin, AnalyticsEventType eventType, const AnalyticsEventInvocationContextSharedPtr& invocationContext)
{
    m_private->AddEvent(analyticsItem, origin, eventType, invocationContext);
}

void AnalyticsManager::LogSearchQueryEvent(protocol::SingleSearchSourceParametersSharedPtr param)
{
    if (m_private)
    {
        m_private->LogSearchQueryEvent(param);
    }
}

uint32 AnalyticsManager::GetSearchQueryEventId()
{
    if (m_private)
    {
        return m_private->GetSearchQueryEventId();
    }
    return 0;
}

// AnalyticsSearchEventManagerPrivate
//
AnalyticsManagerPrivate::AnalyticsManagerPrivate(NB_Context* context): m_context(context), m_searchQueryEventId(0)
{
    nsl_assert(context != NULL);
}

AnalyticsManagerPrivate::~AnalyticsManagerPrivate()
{
    m_searchQueryEventId = 0;
}

void AnalyticsManagerPrivate::LogSearchQueryEvent(protocol::SingleSearchSourceParametersSharedPtr param)
{
    ++m_searchQueryEventId;

    //@TODO: need to remove the default value.
    protocol::AnalyticsEventSharedPtr analyticsEvent(new protocol::AnalyticsEvent());
    protocol::AnalyticsEventMetadataSharedPtr metadata(new protocol::AnalyticsEventMetadata());
    metadata->SetTimestamp(PAL_ClockGetGPSTime());
    metadata->SetCategory(nbcommon::StringUtility::NumberToStringPtr<int>(1));
    metadata->SetName(shared_ptr<std::string>(new std::string("search-query-event")));
    analyticsEvent->SetEventMetadata(metadata);

    protocol::SearchQueryEventSharedPtr query(new protocol::SearchQueryEvent());
    query->SetScheme(shared_ptr<std::string>(new std::string(param->GetDataScheme()->c_str())));
    std::string inputMethod;
    if (param->GetIterCommand() && param->GetIterCommand()->GetCommand() && param->GetIterCommand()->GetCommand()->compare("start") != 0)
    {
        inputMethod = "more";
    }
    else
    {
        if (param->GetSearchFilter() && param->GetSearchFilter()->GetResultStyle() && param->GetSearchFilter()->GetResultStyle()->GetKey()->compare("suggest") == 0)
        {
            inputMethod = "suggestion";
        }
        else
        {
            inputMethod = "screen";
        }
    }

    query->SetInputMethod(shared_ptr<std::string>(new std::string(inputMethod)));

    if (!param->GetPositionArray()->empty())
    {
        protocol::PositionSharedPtr point = param->GetPositionArray()->at(0);
        if (point)
        {
            query->SetPoint(point->GetPoint());
        }
    }

    if (param->GetSearchFilter())
    {
        query->SetSearchFilter(param->GetSearchFilter());
    }

    analyticsEvent->SetSearchQueryEvent(query);

    shared_ptr<protocol::InvocationContext> invocation = GetInvocationConext("search", "default", "default", "default");
    analyticsEvent->SetInvocationContext(invocation);

    nbcommon::AnalyticsManager::GetAnalyticsManager(m_context)->AddEvent(analyticsEvent);
}

uint32 AnalyticsManagerPrivate::GetSearchQueryEventId()
{
    return m_searchQueryEventId;
}

void AnalyticsManagerPrivate::AddEvent(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin, AnalyticsEventType eventType, const AnalyticsEventInvocationContextSharedPtr& invocationContext)
{
    protocol::AnalyticsEventSharedPtr analyticsEvent(new protocol::AnalyticsEvent());
    protocol::AnalyticsEventMetadataSharedPtr metadata(new protocol::AnalyticsEventMetadata());
    metadata->SetTimestamp(PAL_ClockGetGPSTime());
    metadata->SetCategory(GetEventPriority(analyticsItem));
    switch (eventType)
    {
        case AnalyticsPlaceEvent_ShowDetail:
        {
            analyticsEvent->SetSearchDetailEvent(GetSearchDetailEvent(analyticsItem, origin));
            metadata->SetName(shared_ptr<std::string>(new std::string("search-detail-event")));
        }
            break;
        case AnalyticsPlaceEvent_Call:
        {
            analyticsEvent->SetCallEvent(GetCallEvent(analyticsItem, origin));
            metadata->SetName(shared_ptr<std::string>(new std::string("call-event")));
        }
            break;
        case AnalyticsPlaceEvent_Impression:
        {
            analyticsEvent->SetImpressionEvent(GetImpressionEvent(analyticsItem, origin));
            metadata->SetName(shared_ptr<std::string>(new std::string("impression-event")));
        }
            break;
        case AnalyticsPlaceEvent_WebUrl:
        {
            analyticsEvent->SetWeburlEvent(GetWebUrlEvent(analyticsItem, origin));
            metadata->SetName(shared_ptr<std::string>(new std::string("weburl-event")));
        }
            break;
        case AnalyticsPlaceEvent_AddFavorite:
        {
            analyticsEvent->SetAddFavoritesEvent(GetAddFavoriteEvent(analyticsItem, origin));
            metadata->SetName(shared_ptr<std::string>(new std::string("add-favorite-event")));
        }
            break;
        default:
            return;
    }
    analyticsEvent->SetEventMetadata(metadata);
    shared_ptr<protocol::InvocationContext> invocation = GetInvocationConext(invocationContext->sourceModule.c_str(), invocationContext->inputSource.c_str(), invocationContext->invocationMethod.c_str(), invocationContext->screenID.c_str());
    analyticsEvent->SetInvocationContext(invocation);

    nbcommon::AnalyticsManager::GetAnalyticsManager(m_context)->AddEvent(analyticsEvent);
}

shared_ptr<std::string> AnalyticsManagerPrivate::GetEventPriority(const AnalyticsItemSharedPtr& analyticsItem)
{
    //@TODO: Depends on nbanalyticsprivate.h. Now the file cannot be saw.
    if (analyticsItem->m_premiumPlacement)
    {
        // NB_AEC_Gold
        return nbcommon::StringUtility::NumberToStringPtr<int>(0);
    }
    // NB_AEC_Poi
    return nbcommon::StringUtility::NumberToStringPtr<int>(1);
}

protocol::SearchDetailEventSharedPtr AnalyticsManagerPrivate::GetSearchDetailEvent(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin)
{
    protocol::SearchDetailEventSharedPtr event(new protocol::SearchDetailEvent());

    event->SetAnalyticsEventPlace(GetAnalyticsEventPlace(analyticsItem, origin));
    return event;
}

protocol::CallEventSharedPtr AnalyticsManagerPrivate::GetCallEvent(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin)
{
    protocol::CallEventSharedPtr event(new protocol::CallEvent());

    event->SetAnalyticsEventPlace(GetAnalyticsEventPlace(analyticsItem, origin));

    return event;
}

protocol::ImpressionEventSharedPtr AnalyticsManagerPrivate::GetImpressionEvent(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin)
{
    protocol::ImpressionEventSharedPtr event(new protocol::ImpressionEvent());
    event->SetAnalyticsEventPlace(GetAnalyticsEventPlace(analyticsItem, origin));
    return event;
}

protocol::WeburlEventSharedPtr AnalyticsManagerPrivate::GetWebUrlEvent(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin)
{
    protocol::WeburlEventSharedPtr event(new protocol::WeburlEvent());
    event->SetAnalyticsEventPlace(GetAnalyticsEventPlace(analyticsItem, origin));
    return event;
}

protocol::AddFavoritesEventSharedPtr AnalyticsManagerPrivate::GetAddFavoriteEvent(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin)
{
    protocol::AddFavoritesEventSharedPtr event(new protocol::AddFavoritesEvent());
    event->SetAnalyticsEventPlace(GetAnalyticsEventPlace(analyticsItem, origin));
    return event;
}

protocol::AnalyticsEventPlaceSharedPtr AnalyticsManagerPrivate::GetAnalyticsEventPlace(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin)
{
    protocol::AnalyticsEventPlaceSharedPtr place(new protocol::AnalyticsEventPlace());
    place->SetGpsBased(false);
    place->SetSearchQueryEventId(analyticsItem->m_searchQueryEventId);
    place->SetIndex(analyticsItem->m_index);
    switch (origin)
    {
        case ActionOrigin_List:
            place->SetOrigin(shared_ptr<std::string>(new std::string("list")));
            break;
        case ActionOrigin_Map:
            place->SetOrigin(shared_ptr<std::string>(new std::string("map")));
            break;
        case ActionOrigin_Detail:
            place->SetOrigin(shared_ptr<std::string>(new std::string("detail")));
            break;
        case ActionOrigin_Arrival:
            place->SetOrigin(shared_ptr<std::string>(new std::string("arrival")));
            break;
        default:
            break;
    }
    place->SetPremiumPlacement(analyticsItem->m_premiumPlacement);
    place->SetEnhancedPoi(analyticsItem->m_enhancedPoi);
    if (analyticsItem->m_enhancedPoi)
    {
        place->SetId(shared_ptr<std::string>(new std::string(analyticsItem->m_poiContentId)));
    }
    else
    {
        place->SetId(nbcommon::StringUtility::NumberToStringPtr(analyticsItem->m_index));
    }

    if (!analyticsItem->m_goldenCookieSerialized.empty())
    {
        place->SetGoldenCookie(nbsearch::GoldenCookieImpl::Deserialize(analyticsItem->m_goldenCookieSerialized));
    }

    if (!analyticsItem->m_placeEventCookieSerialized.empty())
    {
        place->SetPlaceEventCookie(nbsearch::PlaceEventCookieImpl::Deserialize(analyticsItem->m_placeEventCookieSerialized));
    }

    if (!analyticsItem->m_poiAttributeCookieSerialized.empty())
    {
        place->SetPoiAttributeCookie(nbsearch::PoiAttributeCookieUtil::Deserialize(analyticsItem->m_poiAttributeCookieSerialized));
    }

    return place;
}

}

/*! @} */
