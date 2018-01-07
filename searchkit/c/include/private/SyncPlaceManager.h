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
 @file     SyncPlaceManager.h
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

#ifndef __SYNCPLACEMANAGER_H__
#define __SYNCPLACEMANAGER_H__

#include "nbcontext.h"
#include "nberror.h"
#include "smartpointer.h"
#include "Handler.h"
#include "poidao.h"
#include <vector>
#include <map>
#include "nbtaskqueue.h"
#include "NBProtocolSyncPlacesParameters.h"
#include "NBProtocolSyncPlacesInformation.h"
#include "NBProtocolSyncStatusParameters.h"
#include "NBProtocolSyncStatusInformation.h"
#include "pal.h"
#include "paltimer.h"

namespace nbsearch
{
class SyncPlacesAsyncCallback;
class SyncStatusAsyncCallback;
class SyncPlaceTask;

extern const char* LTKSPM_RecentcsGenID;
extern const char* LTKSPM_RecentcsDBID;
extern const char* LTKSPM_FavoritesGenID;
extern const char* LTKSPM_FavoritesDBID;


/*! This type provides interface for a type which describing result state.
 */
class SyncCompleteListener
{
public:
    virtual ~SyncCompleteListener() {}
    virtual void SyncComplete(DMPOIType type) = 0;
    virtual void SyncError(NB_Error error) = 0;
};

/*!
 * @brief The SyncPlaceManager used to be synchronisation the favorites and recents with server.
 * All public fuctions should only be called on CCC thread.
 *
 */
class SyncPlaceManager
{
public:

    /*!
     * @brief Singleton method
     *
     * @param context
     * @param poiDAO
     *
     * @return
     */
    static shared_ptr<SyncPlaceManager> GetInstance(NB_Context* context, IPOIDAO* poiDAO);

    /*!
     * @brief Start to sync if local db has changed.
     */
    void SyncRequestIfNeeded();

    /*!
     * @brief Start to sync with special type.
     *
     * @param type favorite or recent
     */
    void SyncDB(DMPOIType type);

    /*!
     * @brief Start to sync status.
     */
    void SyncStatus();

    /*!
     * @brief Cancel sync status request.
     */
    void CancelSyncStatus();

    /*!
     * @brief Cancel all sync requests.
     */
    void CancelAllSyncRequest();

    /*!
     * @brief Set sync complete listener.
     */
    void SetListener(SyncCompleteListener *listener);

    //these functions are for ContextBasedSingleton, user should not call them
    NB_Context* GetContext() {return m_context;};
    SyncPlaceManager(NB_Context* context, IPOIDAO* poiDAO);
    ~SyncPlaceManager();

    //these functions should not call them, because these are internal used.
    void HandlerSyncStatusReply(const protocol::SyncStatusInformationSharedPtr response);
    void HandlerSyncPlacesReply(const protocol::SyncPlacesInformationSharedPtr response);
    void CompleteSyncPlace(SyncPlacesAsyncCallback* callback);
    void CompleteSyncStatus(SyncStatusAsyncCallback* callback);
    void UnCompleteSyncPlace(NB_Error error);

private:

    /*!
     * @brief Start to sync with special type.
     *
     * @param type favorite or recent
     */
    void SyncRequest(uint32 type);

    /*!
     * @brief create handler for sync status request.
     *
     * @return NE_OK, if success
     */
    NB_Error CreateSyncStatusHandler();

    /*!
     * @brief create hander for sync request and make it into a pool.
     *
     * @return NE_OK, if success
     */
    NB_Error CreateSyncHandler();

    /*!
     * @brief if FTT return (1 << POI_TYPE_RECENT) | (1 << POI_TYPE_FAVORITE), or db have been change return true;
     *
     * @return uint32
     */
    uint32 NeedSync();

    /*!
     * @brief Set sync places into query.
     *
     * @param dbQuery
     * @param type
     *
     * @return NE_OK, if success
     */
    NB_Error SetSyncPlaceParameter(protocol::SyncPlacesDBQuerySharedPtr& dbQuery, DMPOIType type);
    protocol::PlaceSharedPtr GetPlaceToSync(POIForSync poi);

    /*!
     * @brief After sync callback, update db status.
     */
    void UpdateLocalDBStatus(DMPOIType type);

    /*!
     * @brief send sync request in ccc thread.
     *
     * @param pTask
     */
    void CCC_SyncRequest(const SyncPlaceTask* pTask);

    /*!
     * @brief Sync timer call back.
     */
    static void SyncTimerCallback(
                                       PAL_Instance *pal,       /*!< caller-supplied reference to PAL structure */
                                       void *userData,          /*!< caller-supplied data reference for timer */
                                       PAL_TimerCBReason reason /*!< caller-supplied reason for callback being called */
    );

private:
    NB_Context* m_context;
    shared_ptr<protocol::Handler<protocol::SyncPlacesParameters, protocol::SyncPlacesInformation> > m_handler;
    shared_ptr<protocol::Handler<protocol::SyncStatusParameters, protocol::SyncStatusInformation> > m_statusHandler;
    std::map<shared_ptr<protocol::Handler<protocol::SyncPlacesParameters, protocol::SyncPlacesInformation> >, shared_ptr<SyncPlacesAsyncCallback> > m_handlerPool;
    std::map<shared_ptr<protocol::Handler<protocol::SyncStatusParameters, protocol::SyncStatusInformation> >, shared_ptr<SyncStatusAsyncCallback> > m_statusHandlerPool;
    IPOIDAO* m_poiDAO;
    std::vector<POIForSync>  m_placesForSync;
    shared_ptr<EventTaskQueue> m_eventQueue;
    uint32 m_syncType;
    SyncCompleteListener *m_listener;
};

typedef shared_ptr<SyncPlaceManager> SyncPlaceManagerPtr;


/**
 * @brief callback for sync places request.
 */
class SyncPlacesAsyncCallback : public nbmap::AsyncCallbackWithRequest<protocol::SyncPlacesParametersSharedPtr,
                                                                       protocol::SyncPlacesInformationSharedPtr>
{
public:
    explicit SyncPlacesAsyncCallback(SyncPlaceManager* syncPlacesManager);
    virtual ~SyncPlacesAsyncCallback() {}
    void Success(protocol::SyncPlacesParametersSharedPtr request,
                 protocol::SyncPlacesInformationSharedPtr response);
    void Error(protocol::SyncPlacesParametersSharedPtr request, NB_Error error);
    bool Progress(int percentage);

    /*! Reset this call back to invalid state */
    void Reset() { m_isValid = false; }
private:
    SyncPlaceManager* m_syncPlacesManager;
    bool m_isValid;
};

/**
 * @brief Callback for sync status.
 */
class SyncStatusAsyncCallback : public nbmap::AsyncCallbackWithRequest<protocol::SyncStatusParametersSharedPtr,
                                                                       protocol::SyncStatusInformationSharedPtr>
{
public:
    explicit SyncStatusAsyncCallback(SyncPlaceManager* syncPlacesManager);
    virtual ~SyncStatusAsyncCallback() {}
    void Success(protocol::SyncStatusParametersSharedPtr request,
                 protocol::SyncStatusInformationSharedPtr response);
    void Error(protocol::SyncStatusParametersSharedPtr request, NB_Error error);
    bool Progress(int percentage);

    /*! Reset this call back to invalid state */
    void Reset() { m_isValid = false; }
private:
    SyncPlaceManager* m_syncPlacesManager;
    bool m_isValid;
};

typedef void (SyncPlaceManager::*TaskFunction)(const SyncPlaceTask* pTask);

class SyncPlaceTask : public Task
{
public:
    SyncPlaceTask(SyncPlaceManager* handler, TaskFunction fucntion, uint32 type);
    virtual ~SyncPlaceTask();

    // Override
    virtual void Execute(void);

    uint32            m_type;
private:
    SyncPlaceManager* m_handler;
    TaskFunction      m_function;
};

}

#endif //__SYNCPLACEMANAGER_H__
/*! @} */
