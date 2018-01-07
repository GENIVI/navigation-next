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
 @file     AnalyticsTypes.h
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

#ifndef __ANALYTICS_TYPES_H__
#define __ANALYTICS_TYPES_H__

#include "SingleSearchInformation.h"
#include "CommonTypes.h"

namespace nbsearch
{

enum ActionOrigin
{
    ActionOrigin_List,
    ActionOrigin_Map,
    ActionOrigin_Detail,
    ActionOrigin_Arrival
};

enum AnalyticsEventType
{
    Analytics_Undefined,
    AnalyticsPlaceEvent_ShowDetail,
    AnalyticsPlaceEvent_Call,
    AnalyticsPlaceEvent_Impression,
    AnalyticsPlaceEvent_WebUrl,
    AnalyticsPlaceEvent_AddFavorite
};

struct AnalyticsItem
{

    AnalyticsItem(): m_index(0), m_searchQueryEventId(0) {}

    std::string m_placeEventCookieSerialized;
    std::string m_poiAttributeCookieSerialized;
    std::string m_goldenCookieSerialized;
    std::string m_poiContentId;

    bool m_premiumPlacement;
    bool m_enhancedPoi;
    int  m_index;
    uint32 m_searchQueryEventId;
};

typedef shared_ptr<AnalyticsItem> AnalyticsItemSharedPtr;
typedef shared_ptr<nbcommon::InvocationContext> AnalyticsEventInvocationContextSharedPtr;

}

#endif /* __ANALYTICS_TYPES_H__ */

/*! @} */
