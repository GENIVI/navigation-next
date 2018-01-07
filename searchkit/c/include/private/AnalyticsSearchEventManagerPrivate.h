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
 @file     AnalyticsSearchEventManagerPrivate.h
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

#ifndef __ANALYTICS_SEARCH_EVENT_MANAGER_PRIVATE_H__
#define __ANALYTICS_SEARCH_EVENT_MANAGER_PRIVATE_H__

#include "nbcontext.h"
#include "nbsearchtypes.h"
#include "NBProtocolProxMatch.h"
#include "NBProtocolSingleSearchSourceParameters.h"
#include "NBProtocolSingleSearchSourceInformation.h"
#include "AnalyticsTypes.h"
#include "NBProtocolSearchDetailEvent.h"
#include "NBProtocolCallEvent.h"
#include "NBProtocolImpressionEvent.h"
#include "NBProtocolWeburlEvent.h"
#include "NBProtocolAnalyticsEventPlace.h"
#include "NBProtocolAddFavoritesEvent.h"

namespace nbsearch
{

class AnalyticsManagerPrivate
{
public:
    AnalyticsManagerPrivate(NB_Context* context);
    ~AnalyticsManagerPrivate();

    void AddEvent(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin, AnalyticsEventType eventType,  const AnalyticsEventInvocationContextSharedPtr& invocationContext);

    NB_Context* GetContext() const {return m_context;}

    void LogSearchQueryEvent(protocol::SingleSearchSourceParametersSharedPtr param);
    uint32 GetSearchQueryEventId();
private:
    protocol::SearchDetailEventSharedPtr GetSearchDetailEvent(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin);
    protocol::CallEventSharedPtr GetCallEvent(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin);
    protocol::ImpressionEventSharedPtr GetImpressionEvent(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin);
    protocol::WeburlEventSharedPtr GetWebUrlEvent(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin);
    protocol::AddFavoritesEventSharedPtr GetAddFavoriteEvent(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin);
    protocol::AnalyticsEventPlaceSharedPtr GetAnalyticsEventPlace(const AnalyticsItemSharedPtr& analyticsItem, ActionOrigin origin);
    shared_ptr<std::string> GetEventPriority(const AnalyticsItemSharedPtr& analyticsItem);

    AnalyticsManagerPrivate(const AnalyticsManagerPrivate&);
    AnalyticsManagerPrivate& operator= (const AnalyticsManagerPrivate&);

    NB_Context* m_context;

    uint32 m_searchQueryEventId;
};

}

#endif // __ANALYTICS_SEARCH_EVENT_MANAGER_PRIVATE_H__

/*! @} */
