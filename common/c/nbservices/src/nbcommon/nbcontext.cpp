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

    @file     nbcontext.c
    @defgroup nbcontext Context
*/
/*
    See file description in header file.

    (C) Copyright 2008 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

extern "C"
{
#include "nbcontext.h"
#include "nbcontextaccess.h"
#include "nbcontextprotected.h"
#include "nbnetworkprotected.h"
#include "cslnetwork.h"
#include "datautil.h"
#include "nbanalyticsprotected.h"
#include "nbwifiprobes.h"
#include "nbpersistentdataprotected.h"
#include "data_search_cookie.h"
#include "csdict.h"
#include "nbqalog.h"
#include "pallock.h"
#include "palevent.h"
#include "palfile.h"
#include "paltestlog.h"
}
#include "base.h"
#include "nbtaskqueue.h"
#include "workerqueuemanager.h"
#include "httpdownloadmanager.h"
#include "contextbasedsingleton.h"

#define INITIAL_MAPPING_SIZE 16
#define LOCAL_SETTING_SIZE 8

class ContextDestroyTask;

struct NB_Context
{
    PAL_Instance*                 pal;
    NB_Network*                   network;
    data_util_state               dataState;
    CSL_QaLog*                    qalog;
    NB_GpsHistory*                history;
    CSL_Cache*                    voiceCache;
    CSL_Cache*                    rasterTileCache;
    CSL_Cache*                    onboardRasterTileCache;
    NB_VectorTileManager*         vectorTileManager;
    NB_Analytics*                 analytics;
    NB_WifiProbes*                wifiProbes;
    NB_PersistentData*            persistentData;
    struct CSL_Dictionary*        targetMappings;
    boolean                       hasSearchCookie;
    data_search_cookie            searchCookie;
    uint32                        clientSessionId;  /*!< client sesison id, sent on iden-request to server, usually a GPS timestamp */
    NB_EnhancedContentManager*    enhancedContentManager;
    NB_EnhancedVectorMapManager*  enhancedVectorMapManager;
    uint32                        idenError;
    NB_NetworkNotifyEventCallback networkEventCallback;
    struct CSL_Vector*            callbackList;
    struct CSL_Vector*            masterClearCallbackList;
    boolean                       inMasterClearProcess;
    uint32                        httpDownloadConnectionCount;  /*!< Download connection count of generic download manager */
    ContextDestroyTask*           destroyTask;
    NB_GetAnnounceSnippetLength   snippetLenghtCallback;
    void*                         snippetLenghtCallbackData;
    char                          locale[LOCAL_SETTING_SIZE];
    tpselt                        iden; /*! A copy of iden used by csl_network.
                                            It may be updated and set to csl_network when
                                            necessary */
    char*                         credential;
};

static inline void NetworkNotifyEventCallback(NB_NetworkNotifyEvent event, void* data, void* userData);

static inline void SendNotification(NB_Context* context);
static inline void SendMasterClear(NB_Context* context);
static inline void DestroyContext(NB_Context* pThis);
static inline void ResetCSLCache(CSL_Cache* cache);
static inline void DoMasterClear(NB_Context* pThis);
static inline void RemoveCachedContets(NB_Context* pThis);

static const char  CACHE_PATH_SEPERATOR = ':';
static const char* CACHE_PATH_KEY       = "NBCONTEXT_CACHE_PATH";

class ContextDestroyTask : public Task
{
    enum DestroyState
    {
        Begin = 0,
        DestroyInProgress,
        ReadyToDestroy
    };

public:
    /*! ContextDestroyTask constructor

        Take ownership of parameter PAL_Event.
    */
    ContextDestroyTask(NB_Context* context, PAL_Event* event, uint32 taskCount)
        : m_context(context), m_taskLock(NULL), m_event(event), m_unfinishedTaskCount(taskCount)
    {
        PAL_LockCreate(NB_ContextGetPal(m_context), &m_taskLock);
    }

    /*! ContextDestroyTask destructor */
    ~ContextDestroyTask(void)
    {
        if (m_event)
        {
            PAL_EventDestroy(m_event);
            m_event = NULL;
        }

        if (m_taskLock)
        {
            PAL_LockDestroy(m_taskLock);
            m_taskLock = NULL;
        }
    }

    virtual void Execute(void)
    {
        uint32 unfinishedTaskCount = 0;

        // Decrease count of unfinished tasks.
        unfinishedTaskCount = DecreaseTaskCount(1);

        // Check if all tasks finishes.
        if (unfinishedTaskCount == 0)
        {
            if (m_taskLock)
            {
                PAL_LockDestroy(m_taskLock);
                m_taskLock = NULL;
            }

            // Fire the event to notify all tasks finishes.
            if (m_event)
            {
                // Ignore returned error.
                PAL_EventSet(m_event);
                m_event = NULL;
            }
        }
    }

    /*! Decrease task count by a number

        @return Task count after decreasing
    */
    uint32
    DecreaseTaskCount(uint32 number)
    {
        uint32 unfinishedTaskCount = 0;

        PAL_LockLock(m_taskLock);

        // Decrease task count.
        if (number <= m_unfinishedTaskCount)
        {
            m_unfinishedTaskCount -= number;
        }
        else
        {
            m_unfinishedTaskCount = 0;
        }
        unfinishedTaskCount = m_unfinishedTaskCount;

        PAL_LockUnlock(m_taskLock);

        return unfinishedTaskCount;
    }


private:
    NB_Context*   m_context;
    PAL_Lock*     m_taskLock;
    PAL_Event*    m_event;                  /*!< An event to fire when all tasks finishes */
    uint32        m_unfinishedTaskCount;    /*!< Count of unfinished tasks */
};

NB_DEF NB_Error
NB_ContextCreate(PAL_Instance* pal,
                 NB_NetworkConfiguration* networkConfig,
                 NB_NetworkNotifyEventCallback* callback,
                 NB_CacheConfiguration* voiceCacheConfiguration,
                 NB_CacheConfiguration* rasterTileCacheConfiguration,
                 NB_PersistentData* persistentData,
                 NB_Context** context)
{
    NB_Error        error = NE_OK;
    NB_Context*     pThis = 0;
    CSL_Network*    network = 0;

    if (!pal || !networkConfig || !persistentData || !context)
    {
        return NE_INVAL;
    }

    pThis = static_cast<NB_Context*>(nsl_malloc(sizeof(*pThis)));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->iden = te_clone(networkConfig->iden);
    if (!pThis->iden)
    {
        nsl_free(pThis);
        return NE_NOMEM;
    }

    pThis->clientSessionId =te_getattru(networkConfig->iden, "client-session");

    error = NB_NetworkCreate(pal, networkConfig, &network);
    error = error ? error : data_util_state_init(&pThis->dataState);

    if (!error && callback)
    {
        error = CSL_NetworkAddEventNotifyCallback(network, callback);
    }

    if (!error)
    {
        pThis->networkEventCallback.callback = NetworkNotifyEventCallback;
        pThis->networkEventCallback.callbackData = pThis;
        CSL_NetworkAddEventNotifyCallback(network, &pThis->networkEventCallback);
    }

    pThis->targetMappings = CSL_DictionaryAlloc(INITIAL_MAPPING_SIZE);
    if (!pThis->targetMappings)
    {
        error = NE_NOMEM;
    }

    if (error)
    {
        NB_ContextDestroy(pThis);
        return error;
    }

    pThis->pal = pal;
    pThis->network = network;

    if (voiceCacheConfiguration)
    {
        // Create voice cache
        pThis->voiceCache = CSL_CacheAlloc(pal,
                                           voiceCacheConfiguration->cacheName,
                                           voiceCacheConfiguration->maximumItemsInMemoryCache,
                                           voiceCacheConfiguration->maximumItemsInPersistentCache,
                                           (const byte*)voiceCacheConfiguration->obfuscateKey,
                                           voiceCacheConfiguration->obfuscateKeySize,
                                           voiceCacheConfiguration->configuration,
                                           TRUE);
        if (pThis->voiceCache)
        {
            // Load index-file for persistent cache. Don't check the result, it can fail if the file doesn't exist yet (e.g. for the first run).
            CSL_CacheLoadIndex(pThis->voiceCache);
        }
        else
        {
            NB_ContextDestroy(pThis);
            return NE_NOMEM;
        }
    }

    if (rasterTileCacheConfiguration)
    {
        // Create cache for raster tile manager
        pThis->rasterTileCache = CSL_CacheAlloc(pal,
                                                rasterTileCacheConfiguration->cacheName,
                                                rasterTileCacheConfiguration->maximumItemsInMemoryCache,
                                                rasterTileCacheConfiguration->maximumItemsInPersistentCache,
                                                (const byte*)rasterTileCacheConfiguration->obfuscateKey,
                                                rasterTileCacheConfiguration->obfuscateKeySize,
                                                rasterTileCacheConfiguration->configuration,
                                                TRUE);
        if (pThis->rasterTileCache)
        {
            // Load index-file for persistent cache. Don't check the result, it can fail if the file doesn't exist yet (e.g. for the first run).
            CSL_CacheLoadIndex(pThis->rasterTileCache);
        }
        else
        {
            NB_ContextDestroy(pThis);
            return NE_NOMEM;
        }

        // Set cache name for onboad raster tile manager
        char* onboardCaheName = NULL;
        if (rasterTileCacheConfiguration->cacheName)
        {
            int offboardCacheNameLen = nsl_strlen(rasterTileCacheConfiguration->cacheName);
            int onboardCacheNameLen = offboardCacheNameLen + 1 + 8;  // 8 - for "_ONBOARD", 1 - for '\0'

            onboardCaheName = static_cast<char*>(nsl_malloc(onboardCacheNameLen * sizeof(char)));
            nsl_strlcpy(onboardCaheName, rasterTileCacheConfiguration->cacheName, onboardCacheNameLen);
            nsl_strlcpy(onboardCaheName + offboardCacheNameLen, "_ONBOARD", onboardCacheNameLen);
        }

        // Create cache for onboard raster tile manager
        pThis->onboardRasterTileCache = CSL_CacheAlloc(pal,
                                                       onboardCaheName,
                                                       rasterTileCacheConfiguration->maximumItemsInMemoryCache,
                                                       rasterTileCacheConfiguration->maximumItemsInPersistentCache,
                                                       (const byte*)rasterTileCacheConfiguration->obfuscateKey,
                                                       rasterTileCacheConfiguration->obfuscateKeySize,
                                                       rasterTileCacheConfiguration->configuration,
                                                       TRUE);
        if (onboardCaheName)
        {
            nsl_free(onboardCaheName);
            onboardCaheName = NULL;
        }

        if (pThis->onboardRasterTileCache)
        {
            // Load index-file for persistent cache. Don't check the result, it can fail if the file doesn't exist yet (e.g. for the first run).
            CSL_CacheLoadIndex(pThis->onboardRasterTileCache);
        }
        else
        {
            NB_ContextDestroy(pThis);
            return NE_NOMEM;
        }
    }

    pThis->persistentData = persistentData;

    pThis->callbackList = CSL_VectorAlloc(sizeof(NB_ContextCallback));
    if (!pThis->callbackList)
    {
        NB_ContextDestroy(pThis);
        return NE_NOMEM;
    }

    pThis->masterClearCallbackList = CSL_VectorAlloc(sizeof(NB_ContextCallback));
    if (!pThis->masterClearCallbackList)
    {
        NB_ContextDestroy(pThis);
        return NE_NOMEM;
    }
    pThis->inMasterClearProcess = false;

    if (networkConfig->credential != NULL)
    {
        pThis->credential = (char *)nsl_malloc(strlen(networkConfig->credential)+1);
        nsl_strcpy(pThis->credential, networkConfig->credential);
    }

    *context = pThis;

    return NE_OK;
}


NB_DEF NB_Error
NB_ContextDestroy(NB_Context* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->credential != NULL)
    {
        nsl_free(pThis->credential);
    }

    // Get PAL instance.
    PAL_Instance* pal = NB_ContextGetPal(pThis);
    if (!pal)
    {
        DestroyContext(pThis);
        return NE_OK;
    }

    // Get manager of worker queues.
    shared_ptr<WorkerQueueManager>  manager = ContextBasedSingleton<WorkerQueueManager>::getInstance(pThis);
    if (!manager)
    {
        DestroyContext(pThis);
        return NE_OK;
    }

    // Create an event to wait for finishing all worker task queues.
    vector<shared_ptr<WorkerTaskQueue> > taskQueues = manager->RetrieveAllTaskQueues();
    PAL_Event* event  = NULL;
    if (taskQueues.empty() || PAL_EventCreate(pal, &event) != PAL_Ok)
    {
        DestroyContext(pThis);
        return NE_OK;
    }

    // Create a task to add to task queues.
    uint32 taskCount   = taskQueues.size();
    pThis->destroyTask = new ContextDestroyTask(pThis, event, taskCount);
    if (!(pThis->destroyTask))
    {
        PAL_EventDestroy(event);
        event = NULL;
        DestroyContext(pThis);
        return NE_OK;
    }

    // Add this task in each task queue. 'taskCount' saves failed count when adding this task.
    vector<shared_ptr<WorkerTaskQueue> >::iterator iter = taskQueues.begin();
    vector<shared_ptr<WorkerTaskQueue> >::iterator end  = taskQueues.end();
    for (; iter != end; ++iter)
    {
        if (*iter)
        {
            (*iter)->RemoveAllTasks();
            if ((*iter)->AddTask(pThis->destroyTask, MIN_POSSIBLE_TASK_PRIORITY) == PAL_Ok)
            {
                --taskCount;
            }
        }
    }

    if ((taskCount > 0) && (pThis->destroyTask->DecreaseTaskCount(taskCount) == 0))
    {
        DestroyContext(pThis);
        return NE_OK;
    }

    // Wait for finishing all tasks in worker queues.
    PAL_EventWaitForEvent(event);
    PAL_EventDestroy(event);
    DestroyContext(pThis);
    return NE_OK;
}

void DestroyContext(NB_Context* pThis)
{
    if (!pThis)
    {
        return;
    }

    // free cached search cookie
    NB_ContextSetSearchCookie(pThis, 0);

    data_util_state_free(&pThis->dataState);
    NB_NetworkDestroy(pThis->network);

    if (pThis->voiceCache)
    {
        //Saving the index file for the voice cache
        CSL_CacheSaveIndex(pThis->voiceCache);
        CSL_CacheDealloc(pThis->voiceCache);
    }

    if (pThis->rasterTileCache)
    {
        // Saving the index file for the raster tile manager cache
        CSL_CacheSaveIndex(pThis->rasterTileCache);
        CSL_CacheDealloc(pThis->rasterTileCache);
    }

    if (pThis->persistentData)
    {
        (void)NB_PersistentDataDestroy(pThis->persistentData);
    }

    // QA Log must be destroyed last as other things may be using it
    if (pThis->qalog)
    {
        (void)CSL_QaLogDestroy(pThis->qalog);
    }

    if (pThis->targetMappings)
    {
        CSL_DictionaryDealloc(pThis->targetMappings);
    }


    //@todo: This is a TEMPORARY solution, should be updated.
    //       When some callbacks are invoked here, it may add new callbacks to NB_Context. So we
    //  need to recursively check callbackList to make sure there's no callback left. Or unit
    //  test will report MEMORY LEAK.
    //       Actually, I think we need to implement a mechanism to ensure the sequence of
    //  destroying objects. For example, don't destroy HttpDownloadManager before destroying
    //  TileManager.
    //       Organise these callbacks hierarchy may help, but the use of smart pointer make it more
    //  complicated.
    if (pThis->callbackList)
    {
        int counter = 0;
        do
        {
            int i = 0;
            int size = CSL_VectorGetLength(pThis->callbackList);
            SendNotification(pThis);
            for (i = size-1; i >= 0; i--)
            {
                CSL_VectorRemove(pThis->callbackList, i);
            }
        } while (CSL_VectorGetLength(pThis->callbackList) != 0 && ++counter < 3);
        CSL_VectorDealloc(pThis->callbackList);
        pThis->callbackList = NULL;
    }

    if ( pThis->masterClearCallbackList )
    {
        //No need to send master clear on NB_Context Destroy
        //just dealloc the vector
        int i =0;
        int size = CSL_VectorGetLength(pThis->masterClearCallbackList);
        for ( i=0; i<size; i++ )
        {
           CSL_VectorRemove(pThis->masterClearCallbackList, 0);
        }
        CSL_VectorDealloc(pThis->masterClearCallbackList);
    }

    if (pThis->destroyTask)
    {
        delete pThis->destroyTask;
    }

    if (pThis->iden)
    {
        te_dealloc(pThis->iden);
    }

    nsl_free(pThis);
}

NB_DEF NB_Error
NB_ContextNetworkReset(NB_Context* pThis, NB_NetworkConfiguration* networkConfig, NB_NetworkNotifyEventCallback* callback)
{
    NB_Error result = NE_OK;
    NB_Network* sourceNetwork = NULL;
    NB_Network* newNetwork = NULL;

    if (!pThis || !(pThis->network) || !networkConfig)
    {
        return NE_INVAL;
    }

    sourceNetwork = pThis->network;

    // Create a new NB_Network object by the NB_Network object in NB_Context object.
    result = NB_NetworkCreateByCopy(sourceNetwork, networkConfig, &newNetwork);
    if (result != NE_OK)
    {
        return result;
    }

    if (callback)
    {
        // Ignore the returned error. This function returns NE_NOENT if the callback is not found.
        CSL_NetworkRemoveEventNotifyCallback(newNetwork, callback);
        result = CSL_NetworkAddEventNotifyCallback(newNetwork, callback);
        if (result != NE_OK)
        {
            NB_NetworkDestroy(newNetwork);
            newNetwork = NULL;

            return result;
        }
    }

    pThis->network = newNetwork;

    if (sourceNetwork)
    {
        NB_NetworkDestroy(sourceNetwork);
        sourceNetwork = NULL;
    }

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_ContextNetworkSetIden(NB_Context* pThis, NB_NetworkConfiguration* networkConfig)
{
    if (!pThis || !(pThis->network) || !networkConfig)
    {
        return NE_INVAL;
    }

    if (pThis->iden)
    {
        te_dealloc(pThis->iden);
    }

    pThis->iden = te_clone(networkConfig->iden);
    return NB_NetworkSetIden(pThis->network, networkConfig);
}

NB_DEF PAL_Instance*
NB_ContextGetPal(NB_Context* pThis)
{
    if (!pThis)
    {
        return 0;
    }

    return pThis->pal;
}


NB_DEF NB_Network*
NB_ContextGetNetwork(NB_Context* pThis)
{
    if (!pThis)
    {
        return 0;
    }

    return pThis->network;
}


NB_DEF data_util_state*
NB_ContextGetDataState(NB_Context* pThis)
{
    if (!pThis)
    {
        return 0;
    }

    return &pThis->dataState;
}


NB_DEF NB_Error
NB_ContextSetQaLog(NB_Context* pThis, CSL_QaLog* log)
{
    NB_Error err = NE_OK;

    if (!pThis || !log)
    {
        return NE_INVAL;
    }

    if (pThis->qalog)
    {
        err = CSL_QaLogDestroy(pThis->qalog);
    }
    pThis->qalog = log;

    if (pThis->network)
    {
        CSL_NetworkSetQaLog(pThis->network, log);
    }

    /* log client session Id immediately */
    NB_QaLogClientSessionId(pThis, pThis->clientSessionId);

    return err;
}


NB_DEF CSL_QaLog*
NB_ContextGetQaLog(const NB_Context* pThis)
{
    if (!pThis)
    {
        return 0;
    }

    return pThis->qalog;
}

NB_DEF CSL_Cache*
NB_ContextGetVoiceCache(NB_Context* pThis)
{
    if (!pThis)
    {
        return 0;
    }

    return pThis->voiceCache;
}

NB_DEF CSL_Cache*
NB_ContextGetRasterTileManagerCache(NB_Context* pThis)
{
    if (!pThis)
    {
        return 0;
    }

    return pThis->rasterTileCache;
}

NB_DEF CSL_Cache*
NB_ContextGetOnboardRasterTileManagerCache(NB_Context* pThis)
{
    if (!pThis)
    {
        return 0;
    }

    return pThis->onboardRasterTileCache;
}

NB_DEF NB_Error
NB_ContextSetGpsHistoryNoOwnershipTransfer(NB_Context* context, NB_GpsHistory* history)
{
    if (!context || !history)
    {
        return NE_INVAL;
    }

    context->history = history;

    return NE_OK;
}


NB_DEF NB_GpsHistory*
NB_ContextGetGpsHistory(NB_Context* context)
{
    if (!context)
    {
        return 0;
    }

    return context->history;
}

NB_DEF NB_Error
NB_ContextSetAnalyticsNoOwnershipTransfer(NB_Context* pThis, NB_Analytics* analytics)
{
    if (!pThis || !analytics)
    {
        return NE_INVAL;
    }

    pThis->analytics = analytics;

    return NE_OK;
}

NB_DEF NB_Analytics*
NB_ContextGetAnalytics(NB_Context* pThis)
{
    if (!pThis)
    {
        return NULL;
    }

    return pThis->analytics;
}

NB_DEF NB_Error
NB_ContextSetWifiProbesNoOwnershipTransfer(NB_Context* pThis, NB_WifiProbes* wifiProbes)
{
    if (!pThis || !wifiProbes)
    {
        return NE_INVAL;
    }

    pThis->wifiProbes = wifiProbes;

    return NE_OK;
}

NB_DEF NB_WifiProbes*
NB_ContextGetWifiProbes(NB_Context* pThis)
{
    if (!pThis)
    {
        return NULL;
    }

    return pThis->wifiProbes;
}

NB_DEF NB_Error
NB_ContextSetVectorTileManagerNoOwnershipTransfer(NB_Context* context, NB_VectorTileManager* manager)
{
    if (!context || !manager)
    {
        return NE_INVAL;
    }

    context->vectorTileManager = manager;

    return NE_OK;
}

NB_DEF NB_VectorTileManager*
NB_ContextGetVectorTileManager(NB_Context* context)
{
    if (!context)
    {
        return 0;
    }

    return context->vectorTileManager;
}

NB_DEF NB_PersistentData*
NB_ContextGetPersistentData(NB_Context* context)
{
    if (!context)
    {
        return 0;
    }

    return context->persistentData;
}

NB_DEF NB_Error
NB_ContextSetSearchCookie(NB_Context* context, data_search_cookie* searchCookie)
{
    NB_Error err = NE_OK;

    if (!context)
    {
        return NE_INVAL;
    }

    data_search_cookie_free(&context->dataState, &context->searchCookie);
    data_search_cookie_init(&context->dataState, &context->searchCookie);
    context->hasSearchCookie = FALSE;

    if (searchCookie)
    {
        err = data_search_cookie_copy(&context->dataState, &context->searchCookie, searchCookie);
        if (err == NE_OK)
        {
            context->hasSearchCookie = TRUE;
        }
    }

    return err;
}

NB_DEF const data_search_cookie*
NB_ContextGetSearchCookie(NB_Context* context)
{
    if (!context)
    {
        return 0;
    }

    if (context->hasSearchCookie)
    {
        return &context->searchCookie;
    }

    return 0;
}

NB_DEF NB_Error
NB_ContextSetTargetMappings(NB_Context* context, NB_TargetMapping* mapping, nb_size mappingCount)
{
    nb_size i = 0;

    if (!context)
    {
        return NE_INVAL;
    }

    if (!context->targetMappings)
    {
        return NE_UNEXPECTED;
    }

    for (i = 0; i < mappingCount; i++)
    {
        if (CSL_DictionarySet(context->targetMappings, mapping[i].key, mapping[i].value, nsl_strlen(mapping[i].value) + 1) == 0)
        {
            return NE_NOMEM;
        }
    }

    return NE_OK;
}


NB_DEF const char*
NB_ContextGetTargetMapping(NB_Context* context, const char* target)
{
    const char* mapping = target;

    if (context && context->targetMappings)
    {
        const char* value = CSL_DictionaryGet(context->targetMappings, target, NULL);
        if (value)
        {
            mapping = value;
        }
    }

    return mapping;
}

/* See header file for description */
NB_Error
NB_ContextGetClientGuid(NB_Context* context,
                        NB_NetworkNotifyEventClientGuid* clientGuid)
{
    if ((!context) || (!clientGuid))
    {
        return NE_INVAL;
    }

    // Get the client guid from the network instance.
    if (!(context->network))
    {
        return NE_NOENT;
    }
    return CSL_NetworkGetClientGuid(context->network, clientGuid);
}

/* See header file for description */
void*
NB_ContextGetGenericHttpDownloadManager(NB_Context* context)
{
    if (!context)
    {
        return NULL;
    }

    nbcommon::Tuple<TYPELIST_1(uint32)> otherParameters = nbcommon::MakeTuple(context->httpDownloadConnectionCount);
    shared_ptr<nbcommon::HttpDownloadManager> genericDownloadManager = ContextBasedSingleton<nbcommon::HttpDownloadManager>::getInstance(context, otherParameters);
    return (void*) (genericDownloadManager.get());
}

NB_DEF NB_Error
NB_ContextSetEnhancedContentManagerNoOwnershipTransfer(NB_Context* context, NB_EnhancedContentManager* manager)
{
    if (!context)
    {
        return NE_INVAL;
    }

    context->enhancedContentManager = manager;

    return NE_OK;
}



NB_DEF NB_EnhancedContentManager*
NB_ContextGetEnhancedContentManager(NB_Context* context)
{
    if (!context)
    {
        return 0;
    }

    return context->enhancedContentManager;
}

NB_DEF NB_Error
NB_ContextGetIdenError(NB_Context* context)
{
    if (!context)
    {
        return NE_INVAL;
    }

    return static_cast<NB_Error>(context->idenError);
}

NB_DEF NB_Error
NB_ContextRegisterCallback(NB_Context* context, const NB_ContextCallback* callback)
{
    if( !context || !callback )
    {
        return NE_INVAL;
    }

    if( context->callbackList )
    {
        CSL_VectorAppend(context->callbackList, callback);
    }

    return NE_OK;
}

NB_DEF NB_Error
NB_ContextSendMasterClear(NB_Context* context /*!< Pointer to current context */)
{
    if( !context )
    {
        return NE_INVAL;
    }

    SendMasterClear(context);

    return NE_OK;
}

NB_DEF NB_Error
NB_ContextRegisterMasterClearCallback(NB_Context* context, const NB_ContextCallback* callback)
{
    if ( !context || !callback )
    {
        return NE_INVAL;
    }

    if ( context->masterClearCallbackList )
    {
        CSL_VectorAppend(context->masterClearCallbackList, callback);
    }

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_ContextUnregisterMasterClearCallback(NB_Context* context, const NB_ContextCallback* callback)
{
    if ( !context || !callback )
    {
        return NE_INVAL;
    }

    if ( context->masterClearCallbackList )
    {
        NB_ContextCallback * pCallBackItem = NULL;
        int i  = CSL_VectorGetLength(context->masterClearCallbackList);
        while (i > 0)
        {
            i -- ;
            pCallBackItem = (NB_ContextCallback *)CSL_VectorGetPointer(context->masterClearCallbackList, i);
            if (pCallBackItem->callbackData == callback->callbackData &&
                pCallBackItem->callback == callback->callback)
            {
                pCallBackItem ->callback = NULL;
                pCallBackItem ->callbackData = NULL;
                if( ! context->inMasterClearProcess )
                {
                    CSL_VectorRemove(context->masterClearCallbackList, i);
                }
            }
        }
    }

    return NE_OK;
}



/* See header file for description */
NB_DEF NB_Error
NB_ContextInitializeGenericHttpDownloadManager(NB_Context* context,
                                               uint32 httpDownloadConnectionCount)
{
    if ((!context) || (httpDownloadConnectionCount == 0))
    {
        return NE_INVAL;
    }

    // Set Count of connections for the generic HTTP download manager.
    context->httpDownloadConnectionCount = httpDownloadConnectionCount;

    nbcommon::Tuple<TYPELIST_1(uint32)> otherParameters = nbcommon::MakeTuple(httpDownloadConnectionCount);
    shared_ptr<nbcommon::HttpDownloadManager> genericDownloadManager = ContextBasedSingleton<nbcommon::HttpDownloadManager>::getInstance(context, otherParameters);
    return (genericDownloadManager.get() ? NE_OK : NE_UNEXPECTED);
}

static void SendMasterClear(NB_Context* context)
{
    int i = 0;
    int size = CSL_VectorGetLength(context->masterClearCallbackList);
    NB_ContextCallback *pFunction = NULL;

    context->inMasterClearProcess = TRUE;
    for ( i=0; i<size; i++ )
    {
        pFunction = (NB_ContextCallback*)CSL_VectorGetPointer(context->masterClearCallbackList, i);
        if ( pFunction )
        {
            if (pFunction->callback != NULL)
            {
                pFunction->callback(pFunction->callbackData);
            }
        }
    }

    /* clear all NULL callback */
    i = size;
    while (i > 0)
    {
        i--;
        pFunction = (NB_ContextCallback*)CSL_VectorGetPointer(context->masterClearCallbackList, i);
        if ( pFunction )
        {
            if (pFunction->callback == NULL)
            {
                CSL_VectorRemove(context->masterClearCallbackList, i);
            }
        }
    }

    DoMasterClear(context);
    context->inMasterClearProcess = FALSE;
}

static void SendNotification(NB_Context* context)
{
    int i;
    int size = CSL_VectorGetLength(context->callbackList);
    NB_ContextCallback *Func = NULL;

    for( i=0; i<size; i++ )
    {
       Func = (NB_ContextCallback*)CSL_VectorGetPointer(context->callbackList, i);
       if( Func )
       {
           Func->callback(Func->callbackData);
       }
    }
}

static void NetworkNotifyEventCallback(NB_NetworkNotifyEvent event, void* data, void* userData)
{
    if (!userData || !data)
    {
        return;
    }

    NB_Context *pThis = (NB_Context*)userData;
    switch (event)
    {
        case NB_NetworkNotifyEvent_IdenError:
        {
            pThis->idenError = ((NB_NetworkNotifyEventError*)data)->code;
            break;
        }
        case NB_NetworkNotifyEvent_ClientGuidAssigned:
        {
            NB_NetworkNotifyEventClientGuid* guid =
                    static_cast<NB_NetworkNotifyEventClientGuid*>(data);
            tpselt elt = te_getchild(pThis->iden, "formatted-iden-parameters");
            if (elt)
            {
                elt = te_getchild(elt, "user-info");
                if (elt)
                {
                    te_setattr(elt, "client-guid", (char*)guid->guidData, guid->guidDataLength);
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

NB_DEF NB_Error
NB_ContextSetEnhancedVectorMapManagerNoOwnershipTransfer(NB_Context* context,
                                                         NB_EnhancedVectorMapManager* manager)
{
    NB_Error err = NE_OK;

    if (!context || !manager)
    {
        return NE_INVAL;
    }

    if (context->enhancedVectorMapManager)
    {
//        NB_EnhancedVectorMapManagerDestroy(context->enhancedVectorMapManager);
    }
    context->enhancedVectorMapManager = manager;

    return err;
}


NB_DEF NB_EnhancedVectorMapManager*
NB_ContextGetEnhancedVectorMapManager(NB_Context* context)
{
    if (!context)
    {
        return 0;
    }

    return context->enhancedVectorMapManager;
}

NB_DEF NB_Error
NB_ContextSetSnippetLengthCallback(NB_Context* context, NB_GetAnnounceSnippetLength callback, void* userData)
{
    NB_Error err = NE_OK;

    if (!context || !callback)
    {
        return NE_INVAL;
    }

    context->snippetLenghtCallback = callback;
    context->snippetLenghtCallbackData = userData;

    return err;
}


NB_DEF NB_Error
NB_ContextGetSnippetLengthCallback(NB_Context* context, NB_GetAnnounceSnippetLength* callback, void** userData)
{
    if (!context || !callback)
    {
        return NE_INVAL;
    }

    if (!context->snippetLenghtCallback)
    {
        return NE_NOENT;
    }

    *callback = context->snippetLenghtCallback;

    if (userData)
    {
        *userData = context->snippetLenghtCallbackData;
    }

    return NE_OK;
}


NB_DEF NB_Error NB_ContextSetLocale(NB_Context* context, const char* local)
{
    if (!context)
    {
        return NE_INVAL;
    }
    nsl_strncpy(context->locale, local, sizeof(context->locale));
    return NE_OK;
}

NB_DEF const char* NB_ContextGetLocale(NB_Context* context)
{
    if (context && nsl_strlen(context->locale) > 0)
    {
        return context->locale;
    }
    return "en-US";//default locale setting
}

NB_DEF void
NB_ContextUpdateActiveSession(NB_Context* pThis, const char* sessionId, uint32 length)
{
    if (!pThis || !pThis->iden || !sessionId || !length)
    {
        return;
    }

    // Update active-seesion of this iden.
    tpselt element = te_getchild(pThis->iden, "formatted-iden-parameters");
    if (element)
    {
        element = te_getchild(element, "session-info");
        if (element)
        {
            te_setattr(element, "active-session", sessionId, length);
            NB_NetworkConfiguration networkConfig;
            nsl_memset(&networkConfig, 0, sizeof(NB_NetworkConfiguration));
            networkConfig.iden = pThis->iden;
            (void)NB_NetworkSetIden(pThis->network, &networkConfig);
        }
    }
}

NB_DEF void
NB_ContextAddCachePath(NB_Context* pThis, const char* path)
{
    if (!pThis || !pThis->persistentData || nsl_strempty(path))
    {
        return;
    }

    std::string newData;
    char*   data     = NULL;
    nb_size dataSize = 0;
    NB_Error error = NB_PersistentDataGet(pThis->persistentData, CACHE_PATH_KEY,
                                          (uint8 **)&data, &dataSize);
    if (error == NE_NOENT)
    {
        newData = std::string(path) + CACHE_PATH_SEPERATOR;
    }
    else if (error == NE_OK && data && dataSize)
    {
        std::string oldData(data, dataSize);

        const char* ptr     = oldData.c_str();
        const char* end     = ptr + oldData.size();
        bool        existed = true;
        size_t      length  = nsl_strlen(path);

        while (ptr < end)
        {
            ptr = nsl_strstr(ptr, path);
            if (!ptr)
            {
                existed = false;
                break;
            }

            ptr += length;
            if (*ptr == 0 || *ptr == ':')
            {
                break;
            }
        }

        if (!existed)
        {
            newData = oldData + string(1, CACHE_PATH_SEPERATOR) + path;
        }
        nsl_free(data);
    }

    if (!newData.empty())
    {
        NB_PersistentDataSet(pThis->persistentData, CACHE_PATH_KEY,
                             (const uint8*)newData.c_str(), newData.size(), FALSE, TRUE);
    }
}


void DoMasterClear(NB_Context* pThis)
{
    ResetCSLCache(pThis->voiceCache);
    ResetCSLCache(pThis->rasterTileCache);
    RemoveCachedContets(pThis);
}

/*! Reset CSL Cache.

    We did not remove cache directory directly here, because the CSL_Cache may be still used
    by others.

    @todo: Move this function to coreserices if this function is going to be reused by others.
 */
void ResetCSLCache(CSL_Cache* cache)
{
    if (cache)
    {
        CSL_CacheClear(cache);
        CSL_CacheSaveIndex(cache);
    }
}

/*! Remove cached contents.
*/
void RemoveCachedContets(NB_Context* pThis)
{
    if (!pThis || !pThis->persistentData || !NB_ContextGetPal(pThis))
    {
        return;
    }

    char*    data     = NULL;
    nb_size  dataSize = 0;
    PAL_Instance* pal = NB_ContextGetPal(pThis);
    NB_Error error    = NB_PersistentDataGet(pThis->persistentData, CACHE_PATH_KEY,
                                             (uint8 **)&data, &dataSize);
    if (error != NE_OK || !data || !dataSize)
    {
        return;
    }

    char* copy = (char*)nsl_malloc(dataSize+1);
    if (!copy)
    {
        return;
    }
    nsl_memset(copy, 0, dataSize+1);
    nsl_memcpy(copy, data, dataSize);
    nsl_free(data);

    char* ptr    = copy;
    char* tmpPtr = NULL;
    while (ptr < copy + dataSize)
    {
        tmpPtr = nsl_strchr(ptr, CACHE_PATH_SEPERATOR);
        if (tmpPtr)
        {
            *tmpPtr = '\0';
        }

        if (!nsl_strempty(ptr)) // Just in case...
        {
            PAL_Error palError = PAL_FileRemoveDirectory(pal, ptr, TRUE);
            if (palError != PAL_Ok &&
                (palError = PAL_FileRemove(pal, ptr)) != PAL_Ok)
            {
                PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
                            "%s: Failed to clean up cache: %s\n", __FUNCTION__, ptr);
            }
        }

        ptr+= nsl_strlen(ptr)+1;
    }
    nsl_free(copy);
}

NB_DEC NB_Error
NB_ContextGetCredential(NB_Context* context, char **cred)
{        
    if (context->credential == NULL)
    {
        return NE_INVAL;
    }

    *cred = (char *)nsl_malloc(strlen(context->credential)+1);
    if (*cred != NULL)
    {
        nsl_strcpy(*cred, context->credential);
        return NE_OK;
    }

    return NE_NOMEM;
}

/*! @} */

