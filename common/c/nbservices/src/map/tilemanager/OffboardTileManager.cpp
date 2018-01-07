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

    @file       OffboardTileManager.cpp

    See header file for description.
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

extern "C"
{
    #include "palfile.h"
    #include "nbcontextprotected.h"
}

#include "OffboardTileManager.h"
#include "downloadmanager.h"
#include "urlparser.h"
#include "StringUtility.h"
#include <algorithm>
#include <functional>
#include "LayerFunctors.h"

/*! @{ */

using namespace std;
using namespace nbmap;
using namespace nbcommon;

// Local Constants ...........................................................................

static const char* NB_DTS_PARAMETER_APPEND_ROUTE = "&rid%d=%s&rc%d=%s";
#define DTS_PARAMETER_LENGTH       (nsl_strlen(NB_DTS_PARAMETER_APPEND_ROUTE))


typedef shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> > \
TileRequestCallback;


/*! Functor to convert a map into a vector.

  @template: T1: key type of map.
  @template: T2: value type of map.
  @param:    vec: pointer of vector.
*/
template <typename T1, typename T2>
class Map2VecFunctor : public Base
{
public:
    Map2VecFunctor(vector<T2>* vec)
    {
        m_vec = vec;
    }
    virtual ~Map2VecFunctor()
    {
    }

    void operator()(const pair<T1, T2>& pair)
    {
        m_vec->push_back(pair.second);
    }
private:
    vector<T2>* m_vec;
};

//static const uint32 MAX_CACHE_PATH_LENGTH = 512;

struct DownloadRequestFinder
{
    DownloadRequestFinder(DownloadRequest* request)
    {
        m_request = request;
    }

    bool operator() (const DownloadRequestPtr& request) const
    {
        return (m_request && request && (void*)m_request == (void*)request.get()) ?
                true : false;
    }

    DownloadRequest* m_request;
};

// OffboardTileManager Public functions .........................................................

/* See header file for description */
OffboardTileManager::OffboardTileManager()
        : UnifiedTileManager(),
          m_context(NULL),
          m_compressed(false),
          m_port(DEFAULT_TILE_REQUEST_PORT),
          m_maximumTileRequestCount(DEFAULT_MAX_TILE_REQUEST_COUNT),
          m_maxRetryTimes(DEFAULT_TILE_REQUEST_RETRY_TIMES),
          m_retryDelay(0),
          m_downloadLevel(INVALID_DOWNLOAD_LEVEL),
          m_protocol(DOWNLOAD_PROTOCOL_UNDEFINED)
{
    nsl_memset(&m_contextCallback, 0, sizeof(m_contextCallback));
    m_valid.reset(new bool(true));
}

/* See he   ader file for description */
OffboardTileManager::~OffboardTileManager()
{
    NB_ContextUnregisterMasterClearCallback(m_context, &m_contextCallback);
    Reset();
    *m_valid = false;
}

/* See header file for description */
NB_Error OffboardTileManager::Initialize(NB_Context* context,
                                         shared_ptr<protocol::UrlArgsTemplate> urlArgsTemplate,
                                         bool compressed,
                                         uint32 retryDelay,
                                         uint32 maximumTileRequestCount,
                                         UnifiedTileTypePtr tileType,
                                         TileLayerInfoPtr info,
                                         shared_ptr<string> clientGuid)
{
    if ((!context) || (!tileType))
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (IsInitialized())
    {
        return NE_EXIST;
    }

    Reset();

    // Initialize the members of this object.
    m_context       = context;
    m_compressed    = compressed;
    m_maxRetryTimes = DEFAULT_TILE_REQUEST_RETRY_TIMES;
    m_retryDelay    = retryDelay;
    m_tileType      = tileType;
    m_tileLayerInfo = info;
    m_clientGuid    = clientGuid;

    // @todo: Calculated download level cannot be out of range of type uint32.
    uint32 downloadPriority = urlArgsTemplate->GetDownloadPriority();
    m_downloadLevel = TILE_DOWNLOAD_LEVEL_BASE + downloadPriority;
    m_maximumTileRequestCount      = maximumTileRequestCount;
    m_contextCallback.callback     = &(OffboardTileManager::MasterClear);
    m_contextCallback.callbackData = (void*)this;
    NB_ContextRegisterMasterClearCallback(m_context, &m_contextCallback);

    return NE_OK;
}

/* See header file for description */
void
OffboardTileManager::RequestAddedToDownloadQueue(DownloadRequestPtr request)
{
    m_requestsInDownloadQueue.insert(make_pair(request->GetPriority(), request));
    LimitRequestCount();
}

/* See header file for description */
void
OffboardTileManager::RequestRemovedFromDownloadQueue(DownloadRequestPtr request)
{
    if (request)
    {
        m_requestsInDownloadQueue.erase(request->GetPriority());
    }
}

/* See description in TileManager.h */
void
OffboardTileManager::GetTile(shared_ptr<map<string, string> > templateParameters,
                             uint32 priority)
{
    GetTile(templateParameters,
            shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> >(),
            priority);
}

NB_Error
OffboardTileManager::CreateRequest(uint32 priority,
                                   shared_ptr<map<string, string> > templateParameters,
                                   TileRequestCallback callback,
                                   DownloadRequest** request)
{
    NB_Error error = NE_OK;
    do
    {
        if (!request)
        {
            error = NE_INVAL;
            break;
        }
        /* Check if the hostname, port and URL template are parsed before. If there is no
           common parameters the function SetCommonParameterMap is not called. */
        if (!m_hostname)
        {
            SetCommonParameterMap(shared_ptr<map<string, string> >());
        }

        // Create the URL with 'm_commonParametersUrlTemplate'.
        shared_ptr<string> url(new string(m_commonParametersUrlTemplate));
        if (!url)
        {
            error = NE_NOMEM;
            break;
        }

        // Create the content ID with 'm_commonParametersContentIdTemplate'.
        shared_ptr<string> contentId(new string(m_commonParametersContentIdTemplate));
        if (!contentId)
        {
            error = NE_NOMEM;
            break;
        }

        // Traverse the template parameter map and replace key with value in the URL and
        // content ID.
        if (templateParameters)
        {
            map<string, string>::const_iterator iterator = templateParameters->begin();
            map<string, string>::const_iterator end = templateParameters->end();

            for (; iterator != end; ++iterator)
            {
                const string& key = iterator->first;
                const string& value = iterator->second;

                StringUtility::ReplaceString(*url, key, value);
                StringUtility::ReplaceString(*contentId, key, value);
            }
        }

        // ContentId should be used as file name to store tile into cache. So some special
        // characters should not appear in this ContentId.
        StringUtility::EscapeSpecialCharactersForBaseName(*contentId);

        // Append extra rid and rc if this is tile manager for route layers.
        // @todo: It would be better to move this into RouteLayer.cpp.
        // If this is a request for route/DVRT/GVRT, and "$rid"/"rc" exists in
        // commonParameters, update url and append them.
        if (*(m_tileLayerInfo->tileDataType) == TILE_TYPE_ROUTE ||
            *(m_tileLayerInfo->tileDataType) == TILE_TYPE_DVRT ||
            *(m_tileLayerInfo->tileDataType) == TILE_TYPE_GVRT)
        {
            map<string, string>::iterator ridIter;
            map<string, string>::iterator cidIter;
            map<string, string>::iterator end = templateParameters->end();

            int i = 1;
            while (true)
            {
                string strNumber = nbcommon::StringUtility::NumberToString(i);
                string ridKey = CONTENTID_PARAMETER_ROUTE_ID + strNumber;
                string cidKey = CONTENTID_PARAMETER_ROUTE_COLOR + strNumber;

                ridIter = templateParameters->find(ridKey);
                cidIter = templateParameters->find(cidKey);
                if (ridIter != end && cidIter != end && !ridIter->second.empty() &&
                    cidIter != end && !cidIter->second.empty())
                {
                    char* tmp = new char[DTS_PARAMETER_LENGTH + ridIter->second.size() +
                                         cidIter->second.size() + 1];
                    nsl_sprintf(tmp, NB_DTS_PARAMETER_APPEND_ROUTE, i,
                                ridIter->second.c_str(), i, cidIter->second.c_str());
                    url->append(tmp);
                    delete[] tmp;
                }
                else
                {
                    break;
                }
                ++i;
            }
        }

        DownloadRequest* tileRequest = CreateRealRequest(priority,
                                                         m_context,
                                                         m_compressed,
                                                         m_port,
                                                         m_maxRetryTimes,
                                                         m_protocol,
                                                         m_tileLayerInfo,
                                                         m_hostname,
                                                         url,
                                                         contentId,
                                                         m_clientGuid,
                                                         templateParameters,
                                                         callback);

        if (!tileRequest)
        {
            error = NE_UNEXPECTED;
        }
        else
        {
            *request = tileRequest;
        }
    } while (0);
    return error;
}
/* See description in TileManager.h */
void
OffboardTileManager::GetTile(shared_ptr<map<string, string> > templateParameters,
                             shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> > callback,
                             uint32 priority,
                             bool fastLoadOnly)
{
    NB_Error error = NE_OK;
    DownloadRequest* tileRequest = NULL;
    do
    {
        // Check if this object is initialized.
        if (!IsInitialized())
        {
            error = NE_NOTINIT;
            break;
        }

        if (fastLoadOnly)         // There is no cached data.
        {
            error = NE_NOENT;
            break;
        }
        error = CreateRequest(priority, templateParameters, callback, &tileRequest);
    } while (0);

    if (error != NE_OK && callback)
    {
        callback->Error(templateParameters, error);
        return;
    }

    shared_ptr<DownloadRequest> request(tileRequest);
    m_requestList.insert(make_pair(*(request->GetUrl()), request));
    // Request the tile by the generic HTTP download manager.
    vector<DownloadRequestPtr> tempRequests;
    tempRequests.push_back(request);
    StartTileRequests(tempRequests);
}

/* See description in TileManager.h */
void
OffboardTileManager::RemoveAllTiles()
{
    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return;
    }

    /* Clear pending requests which are not downloading in progress.

       TRICKY: It is better to not cancel the requests which are downloading in progress.
               Because canceling the in progress requests must call the function
               PAL_NetDestroyConnection. The logic of canceling the in progress
               requests is implemented in the CachingTileManager. Save the downloaded
               data of the in progress requests but do not respond to the map view in the
               CachingTileManager.
    */
    ClearRequests(false);
}

/* See description in TileManager.h */
NB_Error
OffboardTileManager::SetCommonParameterMap(shared_ptr<map<string, string> > commonParameters)
{
    shared_ptr<string> entireUrl;
    shared_ptr<string> contentIdTemplate;

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return NE_NOTINIT;
    }

    if (!m_tileType)
    {
        return NE_UNEXPECTED;
    }

    // Set the URL template and content ID template.
    entireUrl = m_tileType->GetUrlTemplate();
    contentIdTemplate = m_tileType->GetContentIDTemplate();
    if ((!entireUrl) || (!contentIdTemplate))
    {
        return NE_UNEXPECTED;
    }

    // Parse the entire URL to hostname, port and URL template.
    {
        nbcommon::UrlParser urlParser(entireUrl);

        // Set DOWNLOAD_PROTOCOL_HTTP as default value.
        m_protocol = DOWNLOAD_PROTOCOL_HTTP;

        //Parse the protocol.
        shared_ptr<string> protocol = urlParser.GetProtocol();
        if (protocol && (!(protocol->empty())))
        {
            const char* protocolChar = protocol->c_str();
            if (protocolChar && (nsl_strlen(protocolChar) > 0))
            {
                if (nsl_stricmp(protocolChar, HTTP_PROTOCOL_IDENTIFIER) == 0)
                {
                    m_protocol = DOWNLOAD_PROTOCOL_HTTP;
                }
                else if (nsl_stricmp(protocolChar, HTTPS_PROTOCOL_IDENTIFIER) == 0)
                {
                    m_protocol = DOWNLOAD_PROTOCOL_HTTPS;
                }
            }
        }

        m_port = urlParser.GetPort();
        m_hostname = urlParser.GetHostname();
        shared_ptr<string> urlTemplate = urlParser.GetLocation();

        if ((!m_hostname) || (!urlTemplate))
        {
            return NE_UNEXPECTED;
        }

        m_commonParametersUrlTemplate = *urlTemplate;
    }

    m_commonParametersContentIdTemplate = *contentIdTemplate;

    /* Traverse the common parameter map and replace key with value in the URL template
       and content ID template.
    */
    if (commonParameters)
    {
        map<string, string>::const_iterator iterator = commonParameters->begin();
        map<string, string>::const_iterator end = commonParameters->end();

        for (; iterator != end; ++iterator)
        {
            const string& key = iterator->first;
            const string& value = iterator->second;

            StringUtility::ReplaceString(m_commonParametersUrlTemplate, key, value);
            StringUtility::ReplaceString(m_commonParametersContentIdTemplate, key, value);
        }
    }
    return NE_OK;
}


void OffboardTileManager::UnregisterMasterClearCallback()
{
    NB_ContextUnregisterMasterClearCallback(m_context, &m_contextCallback);
}
// OffboardTileManager Private functions ........................................................

/*! Check if this OffboardTileManager object is initialized

    This function should be called in all public functions of class OffboardTileManager.

    @return Flag if this OffboardTileManager object is initialized
*/
bool
OffboardTileManager::IsInitialized() const
{
    if (m_context && m_tileType && NB_ContextGetPal(m_context))
    {
        return true;
    }

    return false;
}


/*! Reset an OffboardTileManager object to be uninitialized

    Clear all members in this OffboardTileManager object.

    @return None
*/
void
OffboardTileManager::Reset()
{
    // Clear all pending requests.
    ClearRequests(true);

    PAL_TimerCancel(NB_ContextGetPal(m_context),
                    ExpiringTimerCallback, (void*) this);

    // Clear all members in this OffboardTileManager object.
    m_context = NULL;
    m_port = DEFAULT_TILE_REQUEST_PORT;
    m_maxRetryTimes = DEFAULT_TILE_REQUEST_RETRY_TIMES;
    m_maximumTileRequestCount = DEFAULT_MAX_TILE_REQUEST_COUNT;
    m_downloadLevel = INVALID_DOWNLOAD_LEVEL;
    m_protocol = DOWNLOAD_PROTOCOL_UNDEFINED;
    m_tileType.reset();
    m_hostname.reset();
    m_commonParametersUrlTemplate.clear();
    m_commonParametersContentIdTemplate.clear();
    m_requestsInDownloadQueue.clear();
}

/*! Get the generic HTTP download manager from the context

    @return A pointer to the generic HTTP download manager
*/
DownloadManager*
OffboardTileManager::GetGenericHttpDownloadManager()
{
    if (m_context)
    {
        return (DownloadManager*) NB_ContextGetGenericHttpDownloadManager(m_context);
    }

    return NULL;
}

/*! Clear pending requests of this tile manager

    @return None
*/
void
OffboardTileManager::ClearRequests(bool inProgress      /*!< Whether to clear requests which are
                                                             downloading in progress */
)
{
    // Invalid all requests.
    if (m_valid)
    {
        *m_valid = false;
    }

    // Create a new valid flag.
    m_valid = shared_ptr<bool>(new bool(true));

    DownloadManager* downloadManager = GetGenericHttpDownloadManager();
    map<uint32, DownloadRequestPtr> requestsToCancel;

    /* The member 'm_requestsInDownloadQueue' cannot be used directly. Because the function
       'RequestRemovedFromDownloadQueue' is called when traverse 'm_requestsInDownloadQueue'
       and the iterator is invalid when requests are removed. */
    requestsToCancel.swap(m_requestsInDownloadQueue);

    // Cancel pending requests.
    if (downloadManager)
    {
        map<uint32, DownloadRequestPtr> notCanceledRequests;

        map<uint32, DownloadRequestPtr>::const_iterator iterator = requestsToCancel.begin();
        map<uint32, DownloadRequestPtr>::const_iterator end = requestsToCancel.end();

        /* Check whether to clear requests which are downloading in progress. Using two 'for'
           to avoid checking 'inProgress' flag in each iteration. */
        if (inProgress)
        {
            for (; iterator != end; ++iterator)
            {
                shared_ptr<DownloadRequest> request = iterator->second;

                if (!request)
                {
                    continue;
                }

                OffboardTileRequest* offboardTileRequest = static_cast<OffboardTileRequest*>(request.get());
                if (!offboardTileRequest)
                {
                    continue;
                }

                // Check if the request could be canceled.
                if (offboardTileRequest->WhetherToCancel())
                {
                    // Ignore the returned error.
                    downloadManager->CancelRequest(request);
                    request->DataError(NE_CANCELLED);
                }
                else
                {
                    notCanceledRequests.insert(*iterator);
                }
            }
        }
        else
        {
            for (; iterator != end; ++iterator)
            {
                shared_ptr<DownloadRequest> request = iterator->second;

                if (!request)
                {
                    continue;
                }

                OffboardTileRequest* offboardTileRequest = static_cast<OffboardTileRequest*>(request.get());
                if (!offboardTileRequest)
                {
                    continue;
                }

                /* If the request is downloading in progress, do not cancel this request and
                   add it to the vector for not canceled requests. */
                if (request->GetInProgressFlag())
                {
                    // Reset the tile manager and valid flag.
                    offboardTileRequest->SetTileManager(this, m_valid);

                    notCanceledRequests.insert(*iterator);
                    continue;
                }

                // Check if the request could be canceled.
                if (offboardTileRequest->WhetherToCancel())
                {
                    // Ignore the returned error.
                    downloadManager->CancelRequest(request);
                    request->DataError(NE_CANCELLED);
                }
                else
                {
                    notCanceledRequests.insert(*iterator);
                }
            }
        }

        // Set not canceled requests to the vector for requests in the download queue.
        notCanceledRequests.swap(m_requestsInDownloadQueue);
    }
}

/*! Limit maximum count of requests to download tiles

    Maximum count of requests should not be greater than 'm_maximumTileRequestCount'.

    @return None
*/
void
OffboardTileManager::LimitRequestCount()
{
    DownloadManager* downloadManager = GetGenericHttpDownloadManager();

    int currentRequestCount = static_cast<int>(m_requestsInDownloadQueue.size());
    int maximumTileRequestCount = static_cast<int>(m_maximumTileRequestCount);

    if (currentRequestCount <= maximumTileRequestCount)
    {
        return;
    }

    map<uint32, DownloadRequestPtr>::reverse_iterator end  = m_requestsInDownloadQueue.rend();
    map<uint32, DownloadRequestPtr>::reverse_iterator iter = m_requestsInDownloadQueue.rbegin();
    while (iter != end)
    {
        DownloadRequestPtr request = iter->second;
        if (request && (request->GetInProgressFlag()))
        {
            ++iter;
            continue;
        }

        OffboardTileRequest* offboardTileRequest = static_cast<OffboardTileRequest*>(request.get());
        if (!offboardTileRequest)
        {
            ++iter;
            continue;
        }

        // Invalid this request.
        offboardTileRequest->SetTileManager(NULL, shared_ptr<bool>());
        if (!(offboardTileRequest->WhetherToCancel()))
        {
            ++iter;
            continue;
        }

        /* Function RequestRemovedFromDownloadQueue is called when a request is canceled.
           So erasing the request must be called before canceling. */
        m_requestsInDownloadQueue.erase(request->GetPriority());

        // Ignore the returned error.
        downloadManager->CancelRequest(request);

        // Tell client to retry requesting again.
        request->DataError(NE_AGAIN);
        RemoveRequestFromList(request.get());

        currentRequestCount = static_cast<int>(m_requestsInDownloadQueue.size());
        if (currentRequestCount <= maximumTileRequestCount)
        {
            break;
        }
        iter++;
    }
}

void OffboardTileManager::MasterClear(void* pTileManagerObject)
{
    OffboardTileManager * pThis = (OffboardTileManager *) pTileManagerObject;
    if (pThis == NULL)
    {
        return;
    }

    pThis->RemoveAllTiles();

}

void
OffboardTileManager::ExpiringTimerCallback(PAL_Instance * pal,
                                           void * userData,
                                           PAL_TimerCBReason reason)
{
    //@todo: walk through m_materialAvailability to start request for non-existing
    //materials.
    if (reason != PTCBR_TimerFired)
    {
        return;
    }

    OffboardTileManager* pThis = static_cast<OffboardTileManager*>(userData);
    if (pThis)
    {
        vector<DownloadRequestPtr> tempRequests;
        Map2VecFunctor<string, DownloadRequestPtr> functor(&tempRequests);
        for_each (pThis->m_waitRetryList.begin(),
                  pThis->m_waitRetryList.end(), functor);
        pThis->m_waitRetryList.clear();
        pThis->StartTileRequests(tempRequests);
    }
}

void OffboardTileManager::StartTileRequests(const vector<DownloadRequestPtr>& requests)
{
    // Request the tile by the generic HTTP download manager.
    if (!requests.empty())
    {
        NB_Error error = NE_OK;
        DownloadManager* downloadManager = NULL;
        downloadManager = GetGenericHttpDownloadManager();
        if (!downloadManager)
        {
            // Return NE_RES if there is no generic HTTP download manager.
            error = NE_RES;
        }
        else
        {
            error = downloadManager->RequestData(requests, m_downloadLevel, NULL);
        }
        if (error != NE_OK)
        {
            for (size_t i = 0; i < requests.size(); ++i)
            {
                requests[i]->DataError(error);
            }
        }
    }
}

// @todo: Move the logic of retry to HttpDownloadManager!
bool OffboardTileManager::ScheduleRetry(OffboardTileRequest* request)
{
    bool result = false;
    shared_ptr<string> url;
    if (request && (url = request->GetUrl()))
    {
        if (!m_retryDelay) // No delay, add this request into DownloadQeueue immediately.
        {
            map<string, DownloadRequestPtr>::iterator iter = m_requestList.find(*url);
            if (iter->second)
            {
                StartTileRequests(vector<DownloadRequestPtr>(1, iter->second));
                result = true;
            }
        }
        else // Some special requests(MAT) need to delay several seconds before retry.
        {
            map<string, DownloadRequestPtr>::iterator iter = m_requestList.find(*url);
            if (iter != m_requestList.end())
            {
                m_waitRetryList.insert(pair<string, DownloadRequestPtr>(*url, iter->second));
            }

            if (PAL_TimerSet(NB_ContextGetPal(m_context), m_retryDelay,
                             ExpiringTimerCallback, (void*) this) == PAL_Ok)
            {
                result = true;
            }
        }
    }
    return result;
}

bool OffboardTileManager::IsTheSameRequest(DownloadRequest* request1,
                                           DownloadRequestPtr request2)
{
    bool equal = false;
    if (request1 && request2)
    {
        equal = request1 == request2.get();
    }
    return equal;
}

DownloadRequest*
OffboardTileManager::CreateRealRequest(int priority,
                                       NB_Context* context,
                                       bool compressed,
                                       uint16 port,
                                       uint32 retryTimes,
                                       DownloadProtocol protocol,
                                       TileLayerInfoPtr info,
                                       shared_ptr<string> hostname,
                                       shared_ptr<string> url,
                                       shared_ptr<string> contentID,
                                       shared_ptr<string> clientGuid,
                                       shared_ptr<map<string, string> > templateParameters,
                                       TileRequestCallback callbackWithRequest)
{
    OffboardTileRequest* request = new OffboardTileRequest(priority);
    if (request)
    {
        NB_Error error = request->Initialize(context,
                                             compressed,
                                             port,
                                             retryTimes,
                                             m_protocol,
                                             info,
                                             hostname,
                                             url,
                                             contentID,
                                             clientGuid,
                                             templateParameters,
                                             callbackWithRequest,
                                             m_tileConverter);
        if (error)
        {
            delete request;
            return NULL;
        }
        request->SetTileManager(this, m_valid);
    }
    return request;
}

void OffboardTileManager::RemoveRequestFromList(DownloadRequest* request)
{
    if (request)
    {
        shared_ptr<string> url = request->GetUrl();
        if (url)
        {
            m_waitRetryList.erase(*url);
            m_requestList.erase(*url);
        }
    }
}

OffboardTileManager*  OffboardTileManager::Clone()
{
    OffboardTileManager* manager = new OffboardTileManager(*this);
    return manager;
}

OffboardTileManager::OffboardTileManager(const OffboardTileManager& tileManager) : UnifiedTileManager()
{
    m_context                           = tileManager.m_context;
    m_contextCallback                   = tileManager.m_contextCallback;
    m_compressed                        = tileManager.m_compressed;
    m_tileLayerInfo                     = tileManager.m_tileLayerInfo;
    m_port                              = tileManager.m_port;
    m_maximumTileRequestCount           = tileManager.m_maximumTileRequestCount;
    m_maxRetryTimes                     = tileManager.m_maxRetryTimes;
    m_retryDelay                        = tileManager.m_retryDelay;
    m_downloadLevel                     = tileManager.m_downloadLevel;
    m_protocol                          = tileManager.m_protocol;
    m_tileType                          = tileManager.m_tileType;
    m_clientGuid                        = tileManager.m_clientGuid;
    m_hostname                          = tileManager.m_hostname;
    m_commonParametersUrlTemplate       = tileManager.m_commonParametersUrlTemplate;
    m_commonParametersContentIdTemplate = tileManager.m_commonParametersContentIdTemplate;
    m_requestsInDownloadQueue           = tileManager.m_requestsInDownloadQueue;
    m_waitRetryList                     = tileManager.m_waitRetryList;
    m_requestList                       = tileManager.m_requestList;
    m_tileConverter                     = tileManager.m_tileConverter;
    m_valid                             = shared_ptr<bool> (new bool(true));
}

void OffboardTileManager::UpdateTileLayerInfo(TileLayerInfoPtr info)
{
    m_tileLayerInfo = info;
}

std::string OffboardTileManager::str() const
{
    std::string s = "(OffboardTM)";
    return s;
}

/* See description in header file. */
void OffboardTileManager::SetTileConverter(shared_ptr <TileConverter> tileConverter)
{
    m_tileConverter = tileConverter;
}

/*! @} */
