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
  @file        nbanalytics_legacy.cpp
  @defgroup    nbanalytics
*/
/*
  (C) Copyright 2014 by TeleCommunication Systems, Inc.

  The information contained herein is confidential, proprietary to
  TeleCommunication Systems, Inc., and considered a trade secret as defined
  in section 499C of the penal code of the State of California. Use of this
  information by anyone other than authorized employees of TeleCommunication
  Systems is granted only under a written non-disclosure agreement, expressly
  prescribing the scope and manner of such use.
  --------------------------------------------------------------------------*/

/*! @{ */

extern "C"
{
#include "palclock.h"
#include "nbanalytics.h"
#include "nbanalyticsprivate.h"
#include "nbanalyticseventsconfig.h"
}

#include "analyticsmanager.h"
#include "NBProtocolAppErrorEvent.h"
#include "NBProtocolAnalyticsEventMetadata.h"
#include "NBProtocolInvocationContext.h"
#include "NBProtocolAnalyticsEvent.h"
#include "StringUtility.h"

using namespace nbcommon;

/* common function for adding search-detail-event, map-event,
 * place-message-event, call-event, weburl-event, arrived-event
 * reveal-review-event, reveal-description-event and add-favorite-event.
 * all these events contains only need 1 NB_AnalyticsEventPlace.
 *
 * @note: this file is here to implement legacy APIs that are used by C code.
 */

#define CHECK_NEW_PTR(T,X)                      \
    shared_ptr<T> X(new T);                     \
    if(!X)                                      \
    {                                           \
        return NE_NOMEM;                        \
    }

#define CHECK_PTR(X)                            \
    if(!X)                                      \
    {                                           \
        return NE_NOMEM;                        \
    }

#define DEFAULT_STRING       "default"

static inline protocol::AnalyticsEventMetadataSharedPtr
CreateMetadate(NB_AnalyticsEventsCategory category, const char* name)
{
    protocol::AnalyticsEventMetadataSharedPtr metadata;
    if (name)
    {
        metadata.reset(new protocol::AnalyticsEventMetadata);
        if (metadata)
        {
            metadata->SetCategory(StringUtility::NumberToStringPtr(category));
            metadata->SetName(CHAR_PTR_TO_UTF_STRING_PTR(name));
            metadata->SetTimestamp(PAL_ClockGetTimeMs());
        }
    }

    return metadata;
}

static inline protocol::InvocationContextSharedPtr
CreateInvocation(const char* sourceModule     = DEFAULT_STRING,
                 const char* screenId         = DEFAULT_STRING,
                 const char* inputSource      = DEFAULT_STRING,
                 const char* invocationMethod = DEFAULT_STRING)
{
    protocol::InvocationContextSharedPtr invocation(new protocol::InvocationContext);
    if (invocation && sourceModule && screenId && inputSource && invocationMethod)
    {
        invocation->SetSourceModule(CHAR_PTR_TO_UTF_STRING_PTR(sourceModule));
        invocation->SetScreenId(CHAR_PTR_TO_UTF_STRING_PTR(screenId));
        invocation->SetInputSource(CHAR_PTR_TO_UTF_STRING_PTR(inputSource));
        invocation->SetInvocationMethod(CHAR_PTR_TO_UTF_STRING_PTR(invocationMethod));
    }
    return invocation;
}

NB_DEF NB_Error
NB_AnalyticsAddAppFileSystemErrorEvent(NB_Context* context,
                                       NB_Error errorCode,
                                       const char* detailedCode,
                                       const char* detailedDescription,
                                       const uint32* navSessionID,
                                       const char* fileName)
{
    if (!context || nsl_strempty(detailedCode) || nsl_strempty(detailedDescription) ||
        nsl_strempty(fileName))
    {
        return NE_INVAL;
    }

    AnalyticsManager* manager = AnalyticsManager::GetAnalyticsManager(context);
    if (!manager)
    {
        return NE_UNEXPECTED;
    }

    const char* category = NB_AnalyticsErrorCategoryToString(NB_EC_FileSystem);
    if (nsl_strempty(category))
    {
        return NE_UNEXPECTED;
    }

    CHECK_NEW_PTR(protocol::AppErrorEvent, errorEvent);
    errorEvent->SetErrorCode(errorCode);
    errorEvent->SetDetailedCode(CHAR_PTR_TO_UTF_STRING_PTR(detailedCode));
    errorEvent->SetDetailedDescription(CHAR_PTR_TO_UTF_STRING_PTR(detailedDescription));

    CHECK_NEW_PTR(protocol::ErrorCategory, errorCategory);
    errorCategory->SetType(CHAR_PTR_TO_UTF_STRING_PTR(category));
    errorEvent->SetErrorCategory(errorCategory);

    protocol::PairSharedPtrList pairs = errorEvent->GetPairArray();
    if (!pairs)
    {
        return NE_NOMEM;
    }

    if (navSessionID)
    {
        CHECK_NEW_PTR(protocol::Pair, pair);
        pair->SetKey(CHAR_PTR_TO_UTF_STRING_PTR("nav-session-id"));
        pair->SetValue(StringUtility::NumberToStringPtr(*navSessionID));
        pairs->push_back(pair);
    }

    CHECK_NEW_PTR(protocol::Pair, pair);
    pair->SetKey(CHAR_PTR_TO_UTF_STRING_PTR("fileName"));
    pair->SetValue(CHAR_PTR_TO_UTF_STRING_PTR(fileName));
    pairs->push_back(pair);

    CHECK_NEW_PTR(protocol::AnalyticsEvent, analyticsEvent);
    analyticsEvent->SetAppErrorEvent(errorEvent);
    analyticsEvent->SetInvocationContext(CreateInvocation());
    analyticsEvent->SetEventMetadata(CreateMetadate(NB_AEC_AppError, "app-error-event"));

    manager->AddEvent(analyticsEvent);

    return NE_OK;
}

/*! @} */
