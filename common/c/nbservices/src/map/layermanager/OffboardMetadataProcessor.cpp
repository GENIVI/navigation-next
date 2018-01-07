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
   @file        MetadataProcessor.cpp
   @defgroup    nbmap

   Description: Implementation of MetadataProcessor.

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

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
#include "nbcontextprotected.h"
#include "cslqarecorddefinitions.h"
#include "nbqalog.h"
#include "nbanalyticseventsconfig.h"
}

#include "OffboardMetadataProcessor.h"
#include <algorithm>
#include "analyticsmanager.h"
#include "NBProtocolAppErrorEvent.h"
#include "NBProtocolAnalyticsEvent.h"
#include "NBProtocolMetadataSourceInformation.h"
#include "CommonTypes.h"
#include "TpsAtlasbookProtocol.h"
#include "StringUtility.h"

using namespace protocol;
using namespace nbmap;
using namespace nbcommon;

// Time interval when metadata request failed, UNIT: milliseconds;
static const uint32 METADATA_RETRY_INTERVAL_MSEC  = 5000;
// Time interval between two Metadata request. UNIT: seconds.
static const uint32 METADATA_REQUEST_INTERVAL_SEC = 3600;
// Unlimited metadata retry during navigation.
static const int    METADATA_RETRYTIME_UNLIMITED  = -1;
static const int    METADATA_USE_CACHED_THRESHOLD = 3;

static const uint32 DEFAULT_METADATA_REQUEST_RETRY_TIMES = 3;

const char METADATA_SOURCE_MODULE[] = "Maps";
const char METADATA_INPUT_SOURCE[] = "Maps";
const char METADATA_INVOCATION_METHOD[] = "Maps";
const char METADATA_SCREEN_ID[] = "Maps";

/*! Check if metadata source information is valid.

    We treat MetadataSourceInformation as invlid if it is empty or it does not have any layers.

  @return true if so.
*/
static bool
IsMetadataSourceInformationValid(MetadataSourceInformationSharedPtr& response)
{
    return (response && response->GetContentSourceArray() &&
            !response->GetContentSourceArray()->empty());
}

// create an invocation context.
static shared_ptr<protocol::InvocationContext>
CreateInvocationConext(const char* sourceModule,
                       const char* inputSource,
                       const char* invocationMethod,
                       const char* screenId);

/*! MetadataAsyncCallback

    Refer to AsyncCallbackWithRequest for more description.
 */
class nbmap::MetadataAsyncCallback : public AsyncCallbackWithRequest<protocol::MetadataSourceParametersSharedPtr,
                                                                     protocol::MetadataSourceInformationSharedPtr>
{
public:
    MetadataAsyncCallback(shared_ptr<AsyncCallback<bool> > callback,
                         OffboardMetadataProcessor* processor);
    virtual ~MetadataAsyncCallback(void);
    void Success(protocol::MetadataSourceParametersSharedPtr request,
                 protocol::MetadataSourceInformationSharedPtr response);
    void Error(protocol::MetadataSourceParametersSharedPtr request, NB_Error error);
    bool Progress(int percentage);

    /*! Reset this call back to invalid state */
    void Reset(void);

private:
    shared_ptr<AsyncCallback<bool> > m_pCallback;
    OffboardMetadataProcessor* m_pProcessor;
};

// Implementation of MetadataProcessor .....................................................

/* See description in header file. */
OffboardMetadataProcessor::OffboardMetadataProcessor(NB_Context* context)
        : UnifiedMetadataProcessor(context), m_currentRetryTimes(0), m_metadataMaxRetryTime(0)
{
    m_metadataMaxRetryTime = DEFAULT_METADATA_REQUEST_RETRY_TIMES;
    m_metadataRequestInProgress = false;
}

/* See description in header file. */
OffboardMetadataProcessor::~OffboardMetadataProcessor(void)
{
    PAL_TimerCancel(NB_ContextGetPal(GetContext()), ExpiringTimerCallback, (void*)this);
    if (m_pMetadataAsyncCallback)
    {
        m_pMetadataAsyncCallback->Reset();
    }
}

/* See description in header file. */
void OffboardMetadataProcessor::Reset(void)
{
    UnifiedMetadataProcessor::Reset();

    // Reset other statistics.
    m_metadataMaxRetryTime = DEFAULT_METADATA_REQUEST_RETRY_TIMES;
}

/* See description in header file. */
bool OffboardMetadataProcessor::PrepareRetry(NB_Error error, shared_ptr<AsyncCallback <bool> > callback)
{
    bool retry = false;

    // Add to QaLog.
    unsigned char failureType = GETTING_TPS_FAILED;
    NB_QaLogUnifiedMapConfigInitializationFailure(m_pContext, failureType,
                                                  m_currentRetryTimes, (uint32) error);

    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError,"METADATA_REQUEST_FAILED: RetryTimes:%d NB_Error:%d",m_currentRetryTimes, (uint32) error);

    ++m_currentRetryTimes;
    if (m_metadataMaxRetryTime == METADATA_RETRYTIME_UNLIMITED ||
        (m_currentRetryTimes <= m_metadataMaxRetryTime &&
         error != NESERVERX_UNSUPPORTED_ERROR &&
         error != NESERVERX_PROCESSING_ERROR))
    {
        if (m_currentRetryTimes == METADATA_USE_CACHED_THRESHOLD)
        {
            // METADATA_USE_CACHED_THRESHOLD reached, use cached metadata if possible, but
            // keep requesting new ones at background.
            if (m_metadataInformation && callback)
            {
                // Assume metadata not changed and invoke callback, this will fool my
                // callback and they will use cached metadata.
                callback->Success(false);
            }
        }

        // Start timer, schedule next retry.
        PAL_TimerSet(NB_ContextGetPal(m_pContext), METADATA_RETRY_INTERVAL_MSEC,
                     ExpiringTimerCallback, (void*) this);
        retry = true;
    }

    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
                "%s: error = %d, retry: %d", __FUNCTION__, (int)error, (int)retry);
    return retry;
}

/* See description in header file. */
void OffboardMetadataProcessor::UpdateStatistics(bool succeeded)
{
    m_currentRetryTimes         = 0;
    m_metadataRequestInProgress = false;
    m_pMetadataSourceHandler.reset();
    if (succeeded)
    {
        m_lastSuccessTime = PAL_ClockGetUnixTime();
    }
}

void OffboardMetadataProcessor::ExpiringTimerCallback(PAL_Instance */*pal*/,
                                              void *userData,
                                              PAL_TimerCBReason reason)
{
    if (reason != PTCBR_TimerFired)
    {
        return;
    }

    OffboardMetadataProcessor* pThis = static_cast<OffboardMetadataProcessor*>(userData);
    if (!pThis)
    {
        return;
    }

    pThis->StartMetadataRequest();
}

bool OffboardMetadataProcessor::IsNeedUpdateMetadataForTime(shared_ptr<MetadataConfiguration> config,
                                                    bool skipTimeInterval)
{
    if (!skipTimeInterval &&
        (PAL_ClockGetUnixTime() - m_lastSuccessTime < METADATA_REQUEST_INTERVAL_SEC) &&
        (!config || *config == *m_pConfig))
    {
        return false;
    }
    else
    {
        return true;
    }
}

void OffboardMetadataProcessor::StartMetadataRequestWithCallback(shared_ptr<AsyncCallback<bool> > callback)
{
    //Create AsyncCallback for MetadataRequest.
    if(m_metadataRequestInProgress)
    {
        callback->Error(NE_BUSY);
    }
    MetadataAsyncCallbackPtr handlerCallback(new MetadataAsyncCallback(callback, this));
    if (!handlerCallback)
    {
        callback->Error(NE_NOMEM);
        return;
    }

    m_pMetadataAsyncCallback = handlerCallback; // Store this to enable retry.

    //Create Metadata Handler.
    if(m_pMetadataSourceHandler)
    {
        callback->Error(NE_BUSY);
        return;
    }

    TpsAtlasbookProtocol tpsAtlasbookProtocol(m_pContext);
    m_pMetadataSourceHandler = tpsAtlasbookProtocol.GetMetadataSourceHandler();
    if (!m_pMetadataSourceHandler)
    {
        callback->Error(NE_NOMEM);
        return;
    }

    StartMetadataRequest();
}

/* See description in header file. */
void OffboardMetadataProcessor::StartMetadataRequest(void)
{
    if (!m_pMetadataAsyncCallback) // No callback, just return.
    {
        return;
    }

    if (!m_pMetadataSourceParams)
    {
        m_pMetadataAsyncCallback->Error(m_pMetadataSourceParams, NE_INVAL);
        return;
    }

    if (!m_pMetadataSourceHandler) // Recreate if Handler is invalid.
    {
        TpsAtlasbookProtocol tpsAtlasbookProtocol(m_pContext);
        m_pMetadataSourceHandler = tpsAtlasbookProtocol.GetMetadataSourceHandler();

        if (!m_pMetadataSourceHandler)
        {
            m_pMetadataAsyncCallback->Error(m_pMetadataSourceParams, NE_NOMEM);
            return;
        }
    }

    // QaLog of metadata.
    if (NB_ContextGetQaLog(m_pContext))
    {
        uint32 wantedContentCount =
                static_cast<uint32>(m_pMetadataSourceParams->GetWantedContentArray()->size());
        NB_QaLogMetadataSourceRequest(m_pContext,
                                      m_pMetadataSourceParams->GetLanguage()->c_str(),
                                      wantedContentCount,
                                      m_pMetadataSourceParams->GetWantExtendedMaps() ? \
                                      TRUE : FALSE,
                                      m_pMetadataSourceParams->GetScreenWidth(),
                                      m_pMetadataSourceParams->GetScreenHeight(),
                                      m_pMetadataSourceParams->GetScreenResolution());

        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo,"METADATA_REQUEST: LANG:%s WantedContentCount:%d WantExtentedMaps:%d ScreenWidth:%d ScreenHeight%d ScreenResolution%d",
                                   m_pMetadataSourceParams->GetLanguage()->c_str(),
                                   wantedContentCount,
                                   m_pMetadataSourceParams->GetWantExtendedMaps(),
                                   m_pMetadataSourceParams->GetScreenWidth(),
                                   m_pMetadataSourceParams->GetScreenHeight(),
                                   m_pMetadataSourceParams->GetScreenResolution());

        for (uint32 index = 0; index < wantedContentCount; index++)
        {
            WantedContentSharedPtr wantedContent =
                    m_pMetadataSourceParams->GetWantedContentArray()->at(index);
            if (wantedContent)
            {
                NB_QaLogWantedContent(m_pContext,
                                      NB_QLWCT_MetadataSource,
                                      wantedContent->GetType()->c_str(),
                                      wantedContent->GetCountry()->c_str());
            }
        }
    }

    m_pMetadataSourceHandler->Request(m_pMetadataSourceParams, m_pMetadataAsyncCallback);
    m_metadataRequestInProgress = true;
}

/* See description in header file. */
MetadataChangedFlag
OffboardMetadataProcessor::UpdateMetadataSourceInformation(MetadataSourceInformationSharedPtr response)
{
    if (!response)
    {
        return MCF_Invalid;
    }

    MetadataChangedFlag result = MCF_Unchanged;
    if (!m_metadataInformation) // Response is valid, but no metadata is cached before.
    {
        if (IsMetadataSourceInformationValid(response))
        {
            result = MCF_Changed;
        }
        else // no layers return in response, this is invalid, will retry.
        {
            result = MCF_Invalid;
        }
    }
    else if (response->GetTimeStamp() != m_metadataInformation->GetTimeStamp())
    {
        // If ts is not equal, always use the one returned from server.
        if (IsMetadataSourceInformationValid(response))
        {
            result = MCF_Changed;
        }
        else // time stamp is not equal, but no layers returned from server. Retry.
        {
            result = MCF_Invalid;
        }
    }
    else if (!IsMetadataSourceInformationValid(m_metadataInformation))
    {
        // time stamp is equal, but no layers in cached metadata, retry.
        result = MCF_Invalid;
    }

    switch (result)
    {
        case MCF_Changed:
        {
            (void)SetMetadataInformationToPersistentData(response);
            break;
        }
        case MCF_Invalid:
        {
            m_metadataInformation.reset();
            if (m_pMetadataSourceParams)
            {
                m_pMetadataSourceParams->SetTimeStamp(0);
            }
            break;
        }
        default:
        {
            break;
        }
    }

    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
                "%s: result = %d", __FUNCTION__, (int)result);

    return result;
}

void OffboardMetadataProcessor::ReportNetworkErrorEvent(NB_Error networkError)
{
    //Create the app error event.
    shared_ptr<ErrorCategory> errorCategoryPtr = shared_ptr<ErrorCategory>(new ErrorCategory());
    if(!errorCategoryPtr)
    {
        return;
    }
    errorCategoryPtr->SetType(shared_ptr<std::string>(new std::string(NB_AnalyticsErrorCategoryToString(NB_EC_Network))));

    shared_ptr<AppErrorEvent> appErrorEventPtr = shared_ptr<AppErrorEvent>(new AppErrorEvent());
    if(!appErrorEventPtr)
    {
        return;
    }

    appErrorEventPtr->SetErrorCategory(errorCategoryPtr);
    appErrorEventPtr->SetErrorCode(networkError);
    appErrorEventPtr->SetDetailedCode(shared_ptr<std::string>(new std::string("Metadata network request faile")));

    //Create the common analytics event.
    AnalyticsEventSharedPtr analyticsEvent(new protocol::AnalyticsEvent());
    AnalyticsEventMetadataSharedPtr metadata(new protocol::AnalyticsEventMetadata());
    metadata->SetTimestamp(PAL_ClockGetGPSTime());
    shared_ptr<string> categoryString = StringUtility::NumberToStringPtr(static_cast<int>(NB_AEC_AppError));
    metadata->SetCategory(categoryString);
    metadata->SetName(shared_ptr<std::string>(new std::string("app-error-event")));

    //set the app error event into common analytics event.
    analyticsEvent->SetAppErrorEvent(appErrorEventPtr);
    analyticsEvent->SetEventMetadata(metadata);
    //TODO: Need confirm with product team, what content be needed in this scenario(metadata request faile)
    shared_ptr<protocol::InvocationContext> invocation = CreateInvocationConext(METADATA_SOURCE_MODULE,
            METADATA_INPUT_SOURCE, METADATA_INVOCATION_METHOD, METADATA_SCREEN_ID);
    analyticsEvent->SetInvocationContext(invocation);

    //add event to analytics engine
    AnalyticsManager* manager = AnalyticsManager::GetAnalyticsManager(m_pContext);
    if (manager)
    {
        manager->AddEvent(analyticsEvent);
    }
}

// local functions..
static inline shared_ptr<protocol::InvocationContext>
CreateInvocationConext(const char* sourceModule,
                       const char* inputSource,
                       const char* invocationMethod,
                       const char* screenId)
{
    nsl_assert(sourceModule != NULL && inputSource != NULL && invocationMethod != NULL && screenId != NULL);
    shared_ptr<protocol::InvocationContext> invocationContext(new protocol::InvocationContext());
    if (invocationContext)
    {
        if (sourceModule)
        {
            invocationContext->SetSourceModule(shared_ptr<std::string>(new std::string(sourceModule)));
        }

        if (inputSource)
        {
            invocationContext->SetInputSource(shared_ptr<std::string>(new std::string(inputSource)));
        }


        if (invocationMethod)
        {
            invocationContext->SetInvocationMethod(shared_ptr<std::string>(new std::string(invocationMethod)));
        }


        if (screenId)
        {
            invocationContext->SetScreenId(shared_ptr<std::string>(new std::string(screenId)));
        }

    }
    return invocationContext;
}

// Implementation of MetadataAsyncCallback .................................................

/* See description in header file. */
MetadataAsyncCallback::MetadataAsyncCallback(shared_ptr<AsyncCallback<bool> > callback,
                                             OffboardMetadataProcessor* processor)
        : m_pCallback(callback),
          m_pProcessor(processor)
{
}

/* See description in header file. */
MetadataAsyncCallback::~MetadataAsyncCallback(void)
{
}

/* See description in header file. */
void MetadataAsyncCallback::Success(MetadataSourceParametersSharedPtr request,
                                    MetadataSourceInformationSharedPtr response)
{
    if (!m_pProcessor)
    {
        // Do not need to retry, just return
        if (m_pCallback)
        {
            m_pCallback->Error(NE_INVAL);
        }
        return;
    }

    MetadataChangedFlag result = m_pProcessor->UpdateMetadataSourceInformation(response);
    if (result == MCF_Invalid)
    {
        Error(request, NE_AGAIN);
    }
    else if (m_pCallback)
    {
        m_pCallback->Success(result == MCF_Changed);
    }
    ///IMPORT: m_pProcessor->UpdateStatistics(true) MUST be put at end of this routine
    ///The metadata source processor will delete metadata source handler in UpdateStatistics()
    ///This will cause this MetadataAsyncCallback be deleted
    m_pProcessor->UpdateStatistics(true);
}

/* See description in header file. */
void MetadataAsyncCallback::Error(MetadataSourceParametersSharedPtr /*request*/,
                                  NB_Error error)
{
    if (!m_pProcessor)
    {
        return;
    }

    //Only report the network error.
    if(error >= NESERVER_UNKNOWN &&
       error <= NESERVER_UNSUPPORTEDLOCATION)
    {
        m_pProcessor->ReportNetworkErrorEvent(error);
    }

    if (!m_pProcessor->PrepareRetry(error, m_pCallback) && m_pCallback)
    {
        m_pCallback->Error(error);
        ///IMPORT: m_pProcessor->UpdateStatistics(false) MUST be put at end of this routine
        ///The metadata source processor will delete metadata source handler in UpdateStatistics()
        ///This will cause this MetadataAsyncCallback be deleted
        m_pProcessor->UpdateStatistics(false);
    }
}

/* See description in header file. */
bool MetadataAsyncCallback::Progress(int /*percentage*/)
{
    return  true;
}

/* See description in header file. */
void MetadataAsyncCallback::Reset(void)
{
    m_pProcessor = NULL;
}

/*! @} */
