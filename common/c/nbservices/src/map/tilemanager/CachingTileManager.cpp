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

    @file       CachingTileManager.cpp

    See header file for description.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

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
}

#include "CachingTileManager.h"
#include "datastreamimplementation.h"
#include "TileImpl.h"
#include "CachingTileCallbackWithRequest.h"
#include "StringUtility.h"
#include "UnifiedTileManager.h"
#include "CachedTileOperationCallback.h"

/*! @{ */

using namespace std;
using namespace nbcommon;
using namespace nbmap;

// Local Constants ..............................................................................

// Private structer
class nbmap::CachingTileRequest
{
public:
    CachingTileRequest(StringMapPtr templateParameters,
                       shared_ptr<AsyncCallbackWithRequest<StringMapPtr, TilePtr> > callback,
                       shared_ptr<string> contentId,
                       uint32 priority,
                       bool fastLoadOnly)
            : m_templateParameters(templateParameters),
              m_callback(callback),
              m_contentId(contentId),
              m_priority(priority),
              m_fastLoadOnly(fastLoadOnly) {}
    virtual ~CachingTileRequest() {}

    StringMapPtr m_templateParameters;
    shared_ptr<AsyncCallbackWithRequest<StringMapPtr, TilePtr> > m_callback;
    shared_ptr<string> m_contentId;
    uint32 m_priority;
    bool m_fastLoadOnly;
};
typedef shared_ptr<CachingTileRequest> CachingTileRequestPtr;

class EventQueueCallbackParameter
{
public:
    EventQueueCallbackParameter(CachingTileManager& manager, shared_ptr<bool> valid)
            : m_manager(manager),
              m_valid(valid) {}
    virtual ~EventQueueCallbackParameter(){}
    CachingTileManager&     m_manager;
    shared_ptr<bool>        m_valid;
    uint32                  m_id;
    NB_Error                m_error;
    CacheOperationEntityPtr m_result;
private:
    EventQueueCallbackParameter(const EventQueueCallbackParameter&);
    EventQueueCallbackParameter& operator=(const EventQueueCallbackParameter&);
};

// CachingTileManager Public functions ..........................................................

/* See header file for description */
CachingTileManager::CachingTileManager()
: m_context(NULL),
  m_contextCallback(),
  m_tileLayerInfo(),
  m_tileType(),
  m_tileManager(),
  m_cache(),
  m_callbackValid(),
  m_cachingType(),
  m_datasetId(),
  m_commonParametersContentIdTemplate(),
  m_currentRequestId(0)
{
    nsl_memset(&m_contextCallback, 0, sizeof(m_contextCallback));
}


/* See header file for description */
CachingTileManager::~CachingTileManager()
{
    NB_ContextUnregisterMasterClearCallback(m_context, &m_contextCallback);
    Reset();
}

CachingTileManager::CachingTileManager(const CachingTileManager& manager)
{
    m_context              = manager.m_context;
    m_contextCallback      = manager.m_contextCallback;
    m_tileLayerInfo        = manager.m_tileLayerInfo;
    m_tileType             = manager.m_tileType;
    m_cache                = manager.m_cache;
    m_cachingType          = manager.m_cachingType;
    m_datasetId            = manager.m_datasetId;
    m_fileOperationManager = manager.m_fileOperationManager;
    m_currentRequestId     = 0;
    /* Initialize to an empty pointer. Create when it is used.

       Note: Should not copy the m_callbackValid. Because they are different tile managers.
    */
    m_callbackValid = shared_ptr<bool>();

    m_commonParametersContentIdTemplate = manager.m_commonParametersContentIdTemplate;
    m_tileManager  = TileManagerPtr(manager.m_tileManager);
}

void CachingTileManager::UpdateTileLayerInfo(TileLayerInfoPtr info)
{
    m_tileLayerInfo = info;
    if (m_tileManager)
    {
        m_tileManager->UpdateTileLayerInfo(info);
    }
}

CachingTileManager*  CachingTileManager::CloneWithLayerInfo(TileLayerInfoPtr info)
{
    CachingTileManager* manager = new CachingTileManager(*this);
    if (manager)
    {
        manager->UpdateTileLayerInfo(info);
    }
    return manager;
}

/* See header file for description */
NB_Error
CachingTileManager::Initialize(NB_Context* context,
                               TileTypePtr tileType,
                               TileManagerPtr tileManager,
                               shared_ptr<string> cachingType,
                               shared_ptr<string> datasetId,
                               TileLayerInfoPtr info)
{
    if ((!context) || (!tileManager) || !info)
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
    m_context     = context;
    m_tileType    = tileType;
    m_tileManager = tileManager;
    m_cache       = info->cache;
    m_cachingType = cachingType;
    m_datasetId   = datasetId;

    m_tileLayerInfo = info;

    m_contextCallback.callback  = &(CachingTileManager::MasterClear);
    m_contextCallback.callbackData = (void*)this;
    NB_ContextRegisterMasterClearCallback(m_context, &m_contextCallback);

    m_fileOperationManager = FileOperatingTaskManager::GetInstance(m_context);

    return NE_OK;
}

/* See description in TileManager.h */
void
CachingTileManager::GetTile(shared_ptr<map<string, string> > templateParameters,
                            uint32 priority)
{
    GetTile(templateParameters,
            shared_ptr<AsyncCallbackWithRequest<shared_ptr<std::map<std::string, std::string> >, TilePtr> >(),
            priority);
}

/* See description in TileManager.h */
void
CachingTileManager::GetTile(shared_ptr<map<string, string> > templateParameters,
                            shared_ptr<AsyncCallbackWithRequest<StringMapPtr, TilePtr> > callback,
                            uint32 priority,
                            bool fastLoadOnly)
{
    // Check if this object is initialized.
    if (!IsInitialized())
    {
        if (callback)
        {
            callback->Error(templateParameters, NE_NOTINIT);
        }
        return;
    }

    if (!m_callbackValid)
    {
        m_callbackValid.reset(new bool(true));
        if ((!m_callbackValid) && callback)
        {
            callback->Error(templateParameters, NE_NOMEM);
            return;
        }
    }
    // Check if the Cache object is set.
    // Get the content ID.
    shared_ptr<string> contentId = GetContentId(templateParameters);
    if (m_cache && m_cachingType)
    {
        if (!contentId)
        {
            if (callback)
            {
                callback->Error(templateParameters, NE_UNEXPECTED);
            }
            return;
        }

        uint32 requestId = ++m_currentRequestId;
        CachedTileOperationCallbackPtr cacheReadingCallback(
            new CachedTileOperationCallback(requestId,
                                            m_callbackValid,
                                            *this));
        if (!cacheReadingCallback)
        {
            callback->Error(templateParameters, NE_NOMEM);
            return;
        }

        CachingTileRequestPtr request(new CachingTileRequest(templateParameters, callback,
                                                             contentId, priority,
                                                             fastLoadOnly));
        if (!request)
        {
            callback->Error(templateParameters, NE_NOMEM);
            return;
        }
        pair<map<uint32, CachingTileRequestPtr>::iterator, bool> result =
                m_requests.insert(make_pair(requestId, request));
        if (result.second)
        {
            // Ok, get data from cache asynchronously.
            m_cache->GetData(m_cachingType, contentId, cacheReadingCallback);
            return;
        }
    }

    GetTileFromDTS(templateParameters, callback, contentId, priority, fastLoadOnly);
}

/* See description in TileManager.h */
void
CachingTileManager::RemoveAllTiles()
{
    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return;
    }

    /* Set the flag m_callbackValid false to disable the callback in the
       CachingTileCallbackWithRequest, and clear the flag.

       TRICKY: If this flag is false, the callback will not be called to respond to
               the map view, but the downloaded data is still saved to the cache.
    */
    if (m_callbackValid)
    {
        *m_callbackValid = false;
        m_callbackValid = shared_ptr<bool>();
    }

    // Cancel all tile requests in unified tile manager.
    if (m_tileManager)
    {
        m_tileManager->RemoveAllTiles();
    }
}

/* See description in TileManager.h */
NB_Error
CachingTileManager::SetCommonParameterMap(shared_ptr<std::map<std::string, std::string> > commonParameters)
{
    // @todo: This function is similiar with UnifiedTileManager::SetCommonParameterMap.

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

    // Set content ID template.
    contentIdTemplate = m_tileType->GetContentIDTemplate();
    if (!contentIdTemplate)
    {
        return NE_UNEXPECTED;
    }
    string& commonParametersContentIdTemplate = *contentIdTemplate;

    // Traverse the common parameter map and replace key with value in the content ID template.
    if (commonParameters)
    {
        map<string, string>::const_iterator iterator = commonParameters->begin();
        map<string, string>::const_iterator end = commonParameters->end();

        for (; iterator != end; ++iterator)
        {
            const string& key = iterator->first;
            const string& value = iterator->second;

            StringUtility::ReplaceString(commonParametersContentIdTemplate, key, value);
        }
    }

    // Set the common parameters to the unified tile manager.
    if (m_tileManager)
    {
        NB_Error error = m_tileManager->SetCommonParameterMap(commonParameters);

        if (error != NE_OK)
        {
            return error;
        }
    }

    m_commonParametersContentIdTemplate = commonParametersContentIdTemplate;

    return NE_OK;
}

// CachingTileManager Private functions .........................................................

/*! Check if this CachingTileManager object is initialized

    This function should be called in all public functions of class CachingTileManager.

    @return Flag if this CachingTileManager object is initialized
*/
bool
CachingTileManager::IsInitialized() const
{
    if (m_context && m_tileType)
    {
        return true;
    }

    return false;
}

/*! Reset an CachingTileManager object to be uninitialized

    Clear all members in this CachingTileManager object.

    @return None
*/
void
CachingTileManager::Reset()
{
    // Clear all members in this CachingTileManager object.
    m_context = NULL;
    if (m_callbackValid)
    {
        *m_callbackValid = false;
    }
    m_tileType.reset();
    m_tileManager.reset();
    m_cache.reset();
    m_cachingType.reset();
    m_datasetId.reset();
    m_commonParametersContentIdTemplate.clear();
}

/*! Get the content ID

    This function formats the content ID template with the template parameters.

    @return A content ID
*/
shared_ptr<string>
CachingTileManager::GetContentId(shared_ptr<map<string, string> > templateParameters)
{
    // Create the content ID with 'm_commonParametersContentIdTemplate'.
    shared_ptr<string> contentId(new string(m_commonParametersContentIdTemplate));
    if (!contentId)
    {
        return shared_ptr<string>();
    }

    // Traverse the template parameter map and replace key with value in the content ID.
    if (templateParameters)
    {
        map<string, string>::const_iterator iterator = templateParameters->begin();
        map<string, string>::const_iterator end = templateParameters->end();

        for (; iterator != end; ++iterator)
        {
            const string& key = iterator->first;
            const string& value = iterator->second;

            StringUtility::ReplaceString(*contentId, key, value);
        }
    }

    // Replace "/" with "-". ContentId should be used as file name to store tile into cache.
    // So some special characters should not appear in this ContentId.

    StringUtility::EscapeSpecialCharactersForBaseName(*contentId);

    return contentId;
}

void CachingTileManager::MasterClear(void* pTileManagerObject)
{
    CachingTileManager * pThis = (CachingTileManager*) pTileManagerObject;
    if (pThis == NULL)
    {
        return;
    }

    pThis->RemoveAllTiles();
}


// XXX: functions accessed by FileOperation thread. caller should switch tasks!
/* See description in header file. */
void CachingTileManager::TileCacheReadSuccess(uint32 requestId,
                                              nbcommon::CacheOperationEntityPtr result)
{
    EventQueueCallbackParameter* param = new EventQueueCallbackParameter(*this, m_callbackValid);
    if (param)
    {
        param->m_id     = requestId;
        param->m_error  = NE_OK;
        param->m_result = result;
        uint32 taskId = 0;
        if (PAL_EventTaskQueueAdd(NB_ContextGetPal(m_context),
                                  CachingTileManager::EventTaskCallback,
                                  param, &taskId) != PAL_Ok)
        {
            PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
                        "%s: Failed to add task for id: %u\n", __FUNCTION__, requestId);
            delete param;
        }
    }
}

/* See description in header file. */
void CachingTileManager::TileCacheReadError(uint32 requestId, NB_Error error)
{
    EventQueueCallbackParameter* param = new EventQueueCallbackParameter(*this, m_callbackValid);
    if (param)
    {
        param->m_id    = requestId;
        param->m_error = error;
        uint32 taskId = 0;
        if (PAL_EventTaskQueueAdd(NB_ContextGetPal(m_context),
                                  CachingTileManager::EventTaskCallback,
                                  param, &taskId) != PAL_Ok)
        {
            PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
                        "%s: Failed to add task for id: %u\n", __FUNCTION__, requestId);
            delete param;
        }
    }
}

std::string CachingTileManager::str() const
{
    std::string s = "(CachingTM ";
    s += m_tileManager->str();
    s += ")";
    return s;
}


/* See description in header file. */
void CachingTileManager::EventTaskCallback(PAL_Instance* pal, void* userData)
{
    EventQueueCallbackParameter* param = static_cast<EventQueueCallbackParameter*>(userData);
    if (!pal || !param)
    {
        return;
    }

    CachingTileManager&     manager = param->m_manager;
    shared_ptr<bool>        valid   = param->m_valid;
    uint32                  id      = param->m_id;
    CacheOperationEntityPtr result  = param->m_result;
    NB_Error                error   = param->m_error;
    delete param;

    if (valid && (*valid))
    {
        if (result)
        {
            manager.CCC_TileCacheReadSuccess(id, result);
        }
        else
        {
            manager.CCC_TileCacheReadError(id, error);
        }
    }
}

/* See description in header file. */
void CachingTileManager::CCC_TileCacheReadSuccess(uint32 requestId,
                                                  nbcommon::CacheOperationEntityPtr entry)
{
    if (!entry)
    {
        TileCacheReadError(requestId, NE_UNEXPECTED);
        return;
    }

    map<uint32, CachingTileRequestPtr>::iterator iter = m_requests.find(requestId);
    if (iter == m_requests.end())
    {
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
                    "%s: can't find request for id: %u\n", __FUNCTION__, requestId);
        return;
    }

    CachingTileRequestPtr request = iter->second;
    m_requests.erase(requestId);
    if (!request)
    {
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
                    "%s: empty request for id: %u\n", __FUNCTION__, requestId);
        return;
    }

    if (!entry->m_data)
    {
        entry->m_error = NE_NOENT;
    }

    AdditionalDataPtr& additionalData = entry->m_additionalData;
    shared_ptr<string> datasetId;
    shared_ptr<string> etag;
    shared_ptr<string> errorString;
    if (additionalData)
    {
        datasetId   = (*additionalData)[TILE_ADDITIONAL_KEY_DATASET_ID];
        etag        = (*additionalData)[TILE_ADDITIONAL_KEY_ETAG];
        errorString = (*additionalData)[TILE_ADDITIONAL_KEY_ERROR];
        if (errorString && (!(errorString->empty())))
        {
            int errorNumber = 0;
            StringUtility::StringToNumber(*errorString, errorNumber);
            entry->m_error = static_cast<NB_Error>(errorNumber);
        }
    }

    /* Check if the current and cached dataset IDs are equal. If no, request the
       tile with ETAG in unified tile manager. */
    if (StringUtility::IsStringEqual(m_datasetId, datasetId))
    {

        if (entry->m_error != NE_OK)
        {
            if (entry->m_error == NE_HTTP_NO_CONTENT)
            {
                // Error is 204, which will not be requested if gen-id is not changed.
                if (request->m_callback)
                {
                    request->m_callback->Error(request->m_templateParameters, entry->m_error);
                }
                return;
            }

            // For other errors, download tiles from DTS server.
            GetTileFromDTS(request->m_templateParameters, request->m_callback,
                           request->m_contentId, request->m_priority,
                           request->m_fastLoadOnly);
            return;
        }

        // Now if callback is provided, return tile through it.
        if (request->m_callback)
        {
            TileImplPtr tile(new TileImpl(request->m_contentId, m_tileLayerInfo));
            if (!tile)
            {
                request->m_callback->Error(request->m_templateParameters, NE_NOMEM);
                return;
            }

            tile->SetCachingPath(entry->m_cachingPath);
            tile->SetAdditionalData(entry->m_additionalData);
            tile->SetData(entry->m_data);

            request->m_callback->Success(request->m_templateParameters, tile);
        }
    }  // End of checking if the current and cached dataset IDs are equal
    else
    {
        if (etag)
        {
            if (!entry->m_data || !entry->m_additionalData)
            {
                // etag found but no data, remove old cache entry, and download new one.
                m_cache->RemoveData(m_cachingType, request->m_contentId);
            }
            else // Add the ETAG in the template parameters of the request.
            {
                (*(request->m_templateParameters))[TILE_ADDITIONAL_KEY_ETAG] = *etag;
            }
        }
        GetTileFromDTS(request->m_templateParameters, request->m_callback,
                       request->m_contentId, request->m_priority,
                       request->m_fastLoadOnly, entry->m_data, entry->m_additionalData);
    }
}

/* See description in header file. */
void CachingTileManager::CCC_TileCacheReadError(uint32 requestId, NB_Error error)
{
    map<uint32, CachingTileRequestPtr>::iterator iter = m_requests.find(requestId);
    if (iter != m_requests.end())
    {
        CachingTileRequestPtr request = iter->second;
        m_requests.erase(requestId);
        if (request)
        {
            GetTileFromDTS(request->m_templateParameters, request->m_callback,
                           request->m_contentId, request->m_priority,
                           request->m_fastLoadOnly);
        }
    }
}

/* See description in header file. */
void CachingTileManager::GetTileFromDTS(StringMapPtr templateParameters,
                                        shared_ptr <AsyncCallbackWithRequest <StringMapPtr, TilePtr> > callback,
                                        shared_ptr <string> contentId,
                                        uint32 priority,
                                        bool fastLoadOnly,
                                        nbcommon::DataStreamPtr origData,
                                        AdditionalDataPtr additionalData)
{
    NB_Error error = NE_OK;
    do
    {
        // Check if it only requests the tile for cache.
        if (fastLoadOnly)
        {
            error = NE_NOENT;
            break;
        }

        if (!m_tileManager)
        {
            error = NE_NOTINIT;
            break;
        }

        if (!contentId)
        {
            // Get the content ID.
            contentId = GetContentId(templateParameters);
            if (!contentId)
            {
                error = NE_UNEXPECTED;
                break;
            }
        }

        if (callback && (!m_callbackValid))
        {
            // Create the flag m_callbackValid.
            m_callbackValid.reset(new bool(true));
            if (!m_callbackValid)
            {
                error = NE_NOMEM;
                break;
            }
        }

        shared_ptr<CachingTileCallbackWithRequest>
                cachingTileCallback(new CachingTileCallbackWithRequest(contentId,
                                                                       m_datasetId,
                                                                       m_cachingType,
                                                                       m_cache,
                                                                       m_callbackValid,
                                                                       callback,
                                                                       origData,
                                                                       additionalData));
        if (!cachingTileCallback)
        {
            error = NE_NOMEM;
            break;
        }

        m_tileManager->GetTile(templateParameters,
                               cachingTileCallback,
                               priority,
                               fastLoadOnly);

    } while (0);

    if (error != NE_OK && callback)
    {
        callback->Error(templateParameters, error);
    }
}

/*! @} */
