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
 @file     PlaceStorageManager.cpp
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

#include "PlaceStorageManager.h"
#include "dbmanager.h"
#include "datamanager.h"
#include "nbcontextprotected.h"
#include "contextbasedsingleton.h"

#define DESTROY_LISTENER(listener)  if(listener){listener->Destroy();}\
                                    listener = NULL;

#ifndef DEBUG
#define LPM_ASSERT(value)
#else
#define LPM_ASSERT(value)   nsl_assert(value);
#endif

using namespace nbsearch;

template<typename T>
void ReleaseUserData(T*& userData)
{
    delete userData;
}

// Task
PlaceStorageTask::PlaceStorageTask(PlaceStorageManager* handler, PlaceTaskFunction function)
    : m_handler(handler)
    , m_function(function)
{
}

void PlaceStorageTask::Execute(void)
{
    if (m_handler)
    {
        (m_handler->*m_function)(this);
    }

    delete this;
}

inline LocalPlaceManagerError GenerateLPECode(DMPOIType type, PAL_DB_Error error)
{
    int value = 0;
    if (error != PAL_DB_OK)
    {
        switch (type)
        {
            case POI_TYPE_RECENT:
                value = LPME_RecentsError;
                break;
            case POI_TYPE_FAVORITE:
                value = LPME_FavoritesError;
                break;
            default:
                break;
        }
        value += error;
    }
    else
    {
        value = LPME_OK;
    }
    return (LocalPlaceManagerError)value;
};

PlaceStorageManager::PlaceStorageManager(NB_Context* context):
m_nbcontext(context), m_recentsCacheInitialized(false), m_favoriteCacheInitialized(false), m_enableSync(false)
{
    m_palInstance = NB_ContextGetPal(context);
    m_dbmanager = ContextBasedSingleton<DBManager>::getInstance(context);
    m_datamanager = shared_ptr<DataManager>(new DataManager(m_dbmanager));
    m_eventQueue = shared_ptr<EventTaskQueue>(new EventTaskQueue(m_palInstance));
    m_UITaskQueue = shared_ptr<UiTaskQueue>(new UiTaskQueue(m_palInstance));
    m_syncPlaceManager.reset(new SyncPlaceManager(context, m_datamanager->GetPOIDAO()));
    m_syncPlaceManager->SetListener(this);
}

PlaceStorageManager::~PlaceStorageManager()
{
    m_syncPlaceManager->SetListener(NULL);
    m_favorites.clear();
    m_recents.clear();
    m_nbcontext = NULL;
    m_palInstance = NULL;
}

void PlaceStorageManager::InitializeDatabase(const std::string& dbFile)
{
    m_dbFileName = dbFile;
    m_dbmanager->SetupDB(DBT_POI, dbFile.c_str());
    m_datamanager->GetPOIDAO()->Initialize();
}

void PlaceStorageManager::RecoverFromDump(const std::vector<LocalPlaceManagerDumpData> &favorites,
                                          uint32 favoritesGID,
                                          uint32 favoritesDBID,
                                          const std::vector<LocalPlaceManagerDumpData> &recents,
                                          uint32 recentsGID,
                                          uint32 recentsDBID,
                                          LocalPlaceManager::OneShotListener* listener)
{
    MasterClear();
    InitializeDatabase(m_dbFileName);
    //storage generation id
    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new PlaceStorageTask_Recover(this, &PlaceStorageManager::CCC_RecoverDump, favorites, recents, favoritesGID, recentsGID, favoritesDBID, recentsDBID, listener));
    }
}

void PlaceStorageManager::SetSyncWithServer(bool enable)
{
    if (m_enableSync == enable)
    {
        return;
    }
    m_enableSync = enable;
    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new PlaceStorageTask(this, &PlaceStorageManager::CCC_SyncStatus));
    }
}

shared_ptr<PlaceStorageManager> PlaceStorageManager::GetInstance(NB_Context *context)
{
    shared_ptr<PlaceStorageManager> instance = ContextBasedSingleton<PlaceStorageManager>::getInstance(context);

    return instance;
}

bool PlaceStorageManager::GetCachedPOIs(DMPOIType type,POISharedPtrVector& pois, std::map<uint32, LocalPlaceUserData >& userData)
{
    switch (type)
    {
        case POI_TYPE_FAVORITE:
            pois = m_favorites;
            userData = m_favoritesUserData;
            return m_favoriteCacheInitialized;
            break;
        case POI_TYPE_RECENT:
            pois = m_recents;
            userData = m_recentsUserData;
            return m_recentsCacheInitialized;
        default:
            return false;
            break;
    }
}

void PlaceStorageManager::UI_POIsUpdated(const std::vector<POI *> &POIs, const std::map<uint32, LocalPlaceUserData > &data, DMPOIType type)
{
    switch (type)
    {
        case POI_TYPE_FAVORITE:
        {
            m_favoriteCacheInitialized = true;
            m_favorites.clear();
            for (std::vector<POI*>::const_iterator iter = POIs.begin(); iter != POIs.end(); iter++)
            {
                m_favorites.push_back(shared_ptr<POI>(*iter));
            }
            m_favoritesUserData = data;
            for(std::vector<LocalPlaceManager::Listener*>::iterator iter = m_listeners.begin();iter != m_listeners.end();iter++)
            {
                (*iter)->FavoritesUpdated(m_favorites, m_favoritesUserData);
            }
        }
            break;
        case POI_TYPE_RECENT:
        {
            m_recentsCacheInitialized = true;
            m_recents.clear();
            for (std::vector<POI*>::const_iterator iter = POIs.begin(); iter != POIs.end(); iter++)
            {
                m_recents.push_back(shared_ptr<POI>(*iter));
            }
            m_recentsUserData = data;
            for(std::vector<LocalPlaceManager::Listener*>::iterator iter = m_listeners.begin();iter != m_listeners.end();iter++)
            {
                (*iter)->RecentsUpdated(m_recents, m_recentsUserData);
            }
        }
            break;
        default:
            break;
    }
}

void PlaceStorageManager::UI_NotifyError(LocalPlaceManagerError error)
{
    for(std::vector<LocalPlaceManager::Listener*>::iterator iter = m_listeners.begin();iter != m_listeners.end();iter++)
    {
        (*iter)->Error(error);
    }
}

void PlaceStorageManager::AddPOI(POI const* poi, const LocalPlaceUserData& userData, DMPOIType type, LocalPlaceManager::OneShotListener* listener)
{
    shared_ptr<POI> cachedPOI = shared_ptr<POI>(new POIImpl(*poi));
    switch (type)
    {
        case POI_TYPE_FAVORITE:
        {
            if (m_favorites.size() < FAVORITE_POI_RECORD_COUNT_LIMIT)
            {
                std::vector<shared_ptr<POI> >::iterator iter;
                for (iter = m_favorites.begin(); iter != m_favorites.end(); iter++)
                {
                    if (LocationUtil::isEqual(*((POIImpl*)cachedPOI.get())->m_place->m_location, *(*iter)->GetPlace()->GetLocation()))
                    {
                        break;
                    }
                }
                if (iter != m_favorites.end())
                {
                    break;
                }
                m_favorites.push_back(cachedPOI);
                m_favoritesUserData[m_favorites.size() - 1] = userData;
            }
        }
            break;
        case POI_TYPE_RECENT:
        {
            std::vector<shared_ptr<POI> >::iterator iter;
            for (iter = m_recents.begin(); iter != m_recents.end(); iter++)
            {
                if (LocationUtil::isEqual(*((POIImpl*)cachedPOI.get())->m_place->m_location, *(*iter)->GetPlace()->GetLocation()))
                {
                    break;
                }
            }
            if (iter != m_recents.end())
            {
                m_recents.erase(iter);
                break;
            }
            else if (m_recents.size() >= RECENT_POI_RECORD_COUNT_LIMIT)
            {
                m_recents.pop_back();
            }
            m_recents.insert(m_recents.begin(), cachedPOI);
            for (int i = m_recents.size() - 1; i >= 0; i --)
            {
                m_recentsUserData[i + 1] = m_recentsUserData[i];
            }
            m_recentsUserData[0] = userData;
    }
            break;
        default:
            break;
    }

    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new PlaceStorageTask_AddPOI(this, &PlaceStorageManager::CCC_AddPOI, poi, userData, type, listener));
    }
}

void PlaceStorageManager::RemovePOI(POI const* poi, DMPOIType type, LocalPlaceManager::OneShotListener* listener)
{
    POIImpl* copiedPoi = new POIImpl(*poi);
    switch (type)
    {
        case POI_TYPE_FAVORITE:
        {
            std::vector<shared_ptr<POI> >::iterator iter;
            for (iter = m_favorites.begin(); iter != m_favorites.end(); iter++)
            {
                if (LocationUtil::isEqual(*copiedPoi->m_place->m_location, *(*iter)->GetPlace()->GetLocation()))
                {
                    break;
                }
            }
            if (iter == m_favorites.end())
            {
                break;
            }
            int index = iter - m_favorites.begin();
            for (int i = index; i < (int)m_favorites.size() - 1; i ++)
            {
                m_favoritesUserData[i] = m_favoritesUserData[i + 1];
            }
            m_favoritesUserData.erase(m_favorites.size() - 1);
            m_favorites.erase(iter);
        }
            break;
        case POI_TYPE_RECENT:
        {
            std::vector<shared_ptr<POI> >::iterator iter;
            for (iter = m_recents.begin(); iter != m_recents.end(); iter++)
            {
                if (LocationUtil::isEqual(*copiedPoi->m_place->m_location, *(*iter)->GetPlace()->GetLocation()))
                {
                    break;
                }
            }
            if (iter == m_recents.end())
            {
                break;
            }
            int index = iter - m_recents.begin();
            for (int i = index; i < (int)m_recents.size() - 1; i ++)
            {
                m_recentsUserData[i] = m_recentsUserData[i + 1];
            }
            m_recentsUserData.erase(m_recents.size() - 1);
            m_recents.erase(iter);
        }
            break;
        default:
            break;
    }

    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new PlaceStorageTask_RemovePOI(this, &PlaceStorageManager::CCC_RemovePOI, copiedPoi, type, listener));
    }
    delete copiedPoi;
}

void PlaceStorageManager::RemoveAllPOIs(DMPOIType type, LocalPlaceManager::OneShotListener *listener)
{
    switch (type)
    {
        case POI_TYPE_FAVORITE:
            m_favorites.clear();
            break;
        case POI_TYPE_RECENT:
            m_recents.clear();
            break;
        default:
            break;
    }

    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new PlaceStorageTask_RemoveAllPOIs(this, &PlaceStorageManager::CCC_RemoveAllPOIs, type, listener));
    }
}

void PlaceStorageManager::MoveFavorite(const nbsearch::POI *poi, int index, LocalPlaceManager::OneShotListener *listener)
{
    POIImpl* copiedPoi = new POIImpl(*poi);
    if (index < (int)m_favorites.size() - 1)
    {
        std::vector<shared_ptr<POI> >::iterator iter;
        for (iter = m_favorites.begin(); iter != m_favorites.end(); iter++)
        {
            if (LocationUtil::isEqual(*copiedPoi->m_place->m_location, *(*iter)->GetPlace()->GetLocation()))
            {
                break;
            }
        }
        if (iter != m_favorites.end())
        {
            int origin = iter - m_favorites.begin();
            if (origin != index)
            {
                int offset = 0;
                if (origin < index)
                {
                    offset = 1;
                }
                else
                {
                    offset = -1;
                }
                for (int i = origin; i != index; i += offset)
                {
                    std::swap(m_favorites.at(i + offset), m_favorites.at(i));
                }
                std::map<std::string, std::string> tmp;
                tmp = m_favoritesUserData[origin];
                m_favoritesUserData[origin] = m_favoritesUserData[index];
                m_favoritesUserData[index] = tmp;
            }
        }
    }

    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new PlaceStorageTask_MoveFavorite(this, &PlaceStorageManager::CCC_MoveFavorite, copiedPoi, index, listener));
    }
    delete copiedPoi;
}

void PlaceStorageManager::MasterClear()
{
    if (m_enableSync && m_syncPlaceManager)
    {
        m_syncPlaceManager->CancelAllSyncRequest();
        m_syncPlaceManager->CancelSyncStatus();
    }
    if (m_dbmanager)
    {
        m_dbmanager->MasterClear();
    }
}

void PlaceStorageManager::GetPOIs(DMPOIType type, LocalPlaceManager::GetPOIsListener *listener)
{
    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new PlaceStorageTask_GetPOIs(this, &PlaceStorageManager::CCC_GetPOIs, type, listener));
    }
}

void PlaceStorageManager::RegisterListener(LocalPlaceManager::Listener *listener)
{
    if (listener == NULL)
    {
        return;
    }
    for(std::vector<LocalPlaceManager::Listener*>::iterator iter = m_listeners.begin();iter != m_listeners.end();iter++)
    {
        if (*iter == listener)
        {
            return;
        }
    }
    m_listeners.push_back(listener);
}

void PlaceStorageManager::UnregisterListener(LocalPlaceManager::Listener *listener)
{
    if (listener == NULL || m_listeners.size() == 0)
    {
        return;
    }
    for(std::vector<LocalPlaceManager::Listener*>::iterator iter = m_listeners.begin();iter != m_listeners.end();iter++)
    {
        if (*iter == listener)
        {
            m_listeners.erase(iter);
            return;
        }
    }
}

void PlaceStorageManager::UpdateFavoritePOIName(POI const* poi, const std::string& newName, LocalPlaceManager::OneShotListener* listener)
{
    POIImpl* poiImpl = new POIImpl(*poi);
    poiImpl->m_place->m_name = newName;
    shared_ptr<POI> cachedPOI = shared_ptr<POI>(poiImpl);

    std::vector<shared_ptr<POI> >::iterator iter;
    for (iter = m_favorites.begin(); iter != m_favorites.end(); iter++)
    {
        if (LocationUtil::isEqual(*((POIImpl*)cachedPOI.get())->m_place->m_location, *(*iter)->GetPlace()->GetLocation()))
        {
            std::vector<shared_ptr<POI> >::iterator next = m_favorites.erase(iter);
            m_favorites.insert(next, cachedPOI);
            break;
        }
    }

    if (m_eventQueue)
    {
        m_eventQueue->AddTask(new PlaceStorageTask_UpdatePOIName(this, &PlaceStorageManager::CCC_UpdatePOIName, poi, newName, listener));
    }
}

void PlaceStorageManager::CCC_SyncStatus(PlaceStorageTask* task)
{
    if (m_syncPlaceManager && m_enableSync)
    {
        m_syncPlaceManager->SyncStatus();
    }
}

void PlaceStorageManager::CCC_AddPOI(PlaceStorageTask* task)
{
    PlaceStorageTask_AddPOI* pTask = static_cast<PlaceStorageTask_AddPOI*>(task);
    if (pTask)
    {
        PAL_DB_Error result = PAL_DB_OK;
        result = m_datamanager->GetPOIDAO()->AddPOI(pTask->m_poi, pTask->m_type, pTask->m_userData);
        LPM_ASSERT(result == PAL_DB_OK || result == PAL_DB_FULL);
        if (result != PAL_DB_OK)
        {
            if (m_UITaskQueue)
            {
                m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerError(this, &PlaceStorageManager::UI_HanlderError, pTask->m_type, result, pTask->m_listener));
            }
            return;
        }
        std::vector<POI*> pois;
        std::map<uint32, LocalPlaceUserData >* savedData = new std::map<uint32, LocalPlaceUserData >();
        shared_ptr<void> guard(savedData, ReleaseUserData<std::map<uint32, LocalPlaceUserData > >);
        result = m_datamanager->GetPOIDAO()->GetPOI(pTask->m_type, pois, *savedData);
        LPM_ASSERT(result == PAL_DB_OK);
        if (result != PAL_DB_OK)
        {
            if (m_UITaskQueue)
            {
                m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerError(this, &PlaceStorageManager::UI_HanlderError, pTask->m_type, result, pTask->m_listener));
            }
            return;
        }

        if (m_UITaskQueue)
        {
            m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerSucess(this, &PlaceStorageManager::UI_HandlerSuccess, pTask->m_type, pTask->m_listener, pois, savedData));
        }

        if (m_enableSync && m_syncPlaceManager)
        {
            m_syncPlaceManager->SyncDB(pTask->m_type);
        }
    }
}

void PlaceStorageManager::CCC_RemovePOI(PlaceStorageTask* task)
{
    PlaceStorageTask_RemovePOI* pTask = static_cast<PlaceStorageTask_RemovePOI*>(task);
    if (pTask)
    {
        PAL_DB_Error result = PAL_DB_OK;
        result = m_datamanager->GetPOIDAO()->RemovePOI(pTask->m_poi, pTask->m_type);
        LPM_ASSERT(result == PAL_DB_OK);
        if (result != PAL_DB_OK)
        {
            if (m_UITaskQueue)
            {
                m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerError(this, &PlaceStorageManager::UI_HanlderError, pTask->m_type, result, pTask->m_listener));
            }
            return;
        }
        std::vector<POI*> pois;
        std::map<uint32, LocalPlaceUserData >* savedData = new std::map<uint32, LocalPlaceUserData >();
        shared_ptr<void> guard(savedData, ReleaseUserData<std::map<uint32, LocalPlaceUserData > >);
        result = m_datamanager->GetPOIDAO()->GetPOI(pTask->m_type, pois, *savedData);
        LPM_ASSERT(result == PAL_DB_OK);
        if (result != PAL_DB_OK)
        {
            if (m_UITaskQueue)
            {
                m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerError(this, &PlaceStorageManager::UI_HanlderError, pTask->m_type, result, pTask->m_listener));
            }
            return;
        }

        if (m_UITaskQueue)
        {
            m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerSucess(this, &PlaceStorageManager::UI_HandlerSuccess, pTask->m_type, pTask->m_listener, pois, savedData));
        }

        if (m_enableSync && m_syncPlaceManager)
        {
            m_syncPlaceManager->SyncDB(pTask->m_type);
        }
    }
}

void PlaceStorageManager::CCC_RemoveAllPOIs(PlaceStorageTask* task)
{
    PlaceStorageTask_RemoveAllPOIs* pTask = static_cast<PlaceStorageTask_RemoveAllPOIs*>(task);
    if (pTask)
    {
        PAL_DB_Error result = PAL_DB_OK;
        result = m_datamanager->GetPOIDAO()->RemovePOIs(pTask->m_type);
        LPM_ASSERT(result == PAL_DB_OK);
        if (result != PAL_DB_OK)
        {
            if (m_UITaskQueue)
            {
                m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerError(this, &PlaceStorageManager::UI_HanlderError, pTask->m_type, result, pTask->m_listener));
            }
            return;
        }
        std::vector<POI*> pois;
        std::map<uint32, LocalPlaceUserData >* savedData = new std::map<uint32, LocalPlaceUserData >();
        shared_ptr<void> guard(savedData, ReleaseUserData<std::map<uint32, LocalPlaceUserData > >);
        result = m_datamanager->GetPOIDAO()->GetPOI(pTask->m_type, pois, *savedData);
        LPM_ASSERT(result == PAL_DB_OK);
        if (result != PAL_DB_OK)
        {
            if (m_UITaskQueue)
            {
                m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerError(this, &PlaceStorageManager::UI_HanlderError, pTask->m_type, result, pTask->m_listener));
            }
            return;
        }

        if (m_UITaskQueue)
        {
            m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerSucess(this, &PlaceStorageManager::UI_HandlerSuccess, pTask->m_type, pTask->m_listener, pois, savedData));
        }

        if (m_enableSync && m_syncPlaceManager)
        {
            m_syncPlaceManager->SyncDB(pTask->m_type);
        }
    }
}

void PlaceStorageManager::CCC_MoveFavorite(PlaceStorageTask* task)
{
    PlaceStorageTask_MoveFavorite* pTask = static_cast<PlaceStorageTask_MoveFavorite*>(task);
    if (pTask)
    {
        PAL_DB_Error result = PAL_DB_OK;
        result = m_datamanager->GetPOIDAO()->UpdateFavoritePOIIndex(pTask->m_poi, pTask->m_index);
        LPM_ASSERT(result == PAL_DB_OK);
        if (result != PAL_DB_OK)
        {
            if (m_UITaskQueue)
            {
                m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerError(this, &PlaceStorageManager::UI_HanlderError, POI_TYPE_FAVORITE, result, pTask->m_listener));
            }
            return;
        }
        std::vector<POI*> pois;
        std::map<uint32, LocalPlaceUserData >* savedData = new std::map<uint32, LocalPlaceUserData >();
        shared_ptr<void> guard(savedData, ReleaseUserData<std::map<uint32, LocalPlaceUserData > >);
        result = m_datamanager->GetPOIDAO()->GetPOI(POI_TYPE_FAVORITE, pois, *savedData);
        LPM_ASSERT(result == PAL_DB_OK);
        if (result != PAL_DB_OK)
        {
            if (m_UITaskQueue)
            {
                m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerError(this, &PlaceStorageManager::UI_HanlderError, POI_TYPE_FAVORITE, result, pTask->m_listener));
            }
            return;
        }

        if (m_UITaskQueue)
        {
            m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerSucess(this, &PlaceStorageManager::UI_HandlerSuccess, POI_TYPE_FAVORITE, pTask->m_listener, pois, savedData));
        }

        if (m_enableSync && m_syncPlaceManager)
        {
            m_syncPlaceManager->SyncDB(POI_TYPE_FAVORITE);
        }
    }
}

void PlaceStorageManager::CCC_GetPOIs(PlaceStorageTask* task)
{
    PlaceStorageTask_GetPOIs* pTask = static_cast<PlaceStorageTask_GetPOIs*>(task);
    if (pTask)
    {
        std::vector<POI*> pois;
        std::map<uint32, LocalPlaceUserData >* savedData = new std::map<uint32, LocalPlaceUserData >();
        shared_ptr<void> guard(savedData, ReleaseUserData<std::map<uint32, LocalPlaceUserData > >);
        PAL_DB_Error result = m_datamanager->GetPOIDAO()->GetPOI(pTask->m_type, pois, *savedData);
        LPM_ASSERT(result == PAL_DB_OK);
        if (result != PAL_DB_OK)
        {

            if (m_UITaskQueue)
            {
                m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerError(this,
                                                                           &PlaceStorageManager::UI_HanlderError,
                                                                           pTask->m_type,
                                                                           result,
                                                                           pTask->m_listener));
            }
            return;
        }

        if (m_UITaskQueue)
        {
            m_UITaskQueue->AddTask(new PlaceStorageUITask_UpdatePOIs(this, &PlaceStorageManager::UI_UpdatePOIs, pois, pTask->m_type, savedData, pTask->m_listener));
        }
    }
}

void PlaceStorageManager::CCC_UpdatePOIName(PlaceStorageTask* task)
{
    PlaceStorageTask_UpdatePOIName* pTask = static_cast<PlaceStorageTask_UpdatePOIName*>(task);
    if (pTask)
    {
        PAL_DB_Error result = m_datamanager->GetPOIDAO()->UpdateFavoriteName(pTask->m_poi, POI_TYPE_FAVORITE, pTask->m_newName);
        LPM_ASSERT((result == PAL_DB_OK) || (result == (SQLITE_DONE + 1)));
        if (result != PAL_DB_OK)
        {

            if (m_UITaskQueue)
            {
                m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerError(this,
                                                                           &PlaceStorageManager::UI_HanlderError,
                                                                           POI_TYPE_FAVORITE,
                                                                           result,
                                                                           pTask->m_listener));
            }
            return;
        }
        if (m_UITaskQueue)
        {
            std::vector<POI*> pois;
            std::map<uint32, LocalPlaceUserData >* savedData = new std::map<uint32, LocalPlaceUserData >();
            shared_ptr<void> guard(savedData, ReleaseUserData<std::map<uint32, LocalPlaceUserData > >);
            result = m_datamanager->GetPOIDAO()->GetPOI(POI_TYPE_FAVORITE, pois, *savedData);
            LPM_ASSERT(result == PAL_DB_OK);
            if (result != PAL_DB_OK)
            {
                if (m_UITaskQueue)
                {
                    m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerError(this, &PlaceStorageManager::UI_HanlderError, POI_TYPE_FAVORITE, result, pTask->m_listener));
                }
                return;
            }

            if (m_UITaskQueue)
            {
                m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerSucess
                                       (this, &PlaceStorageManager::UI_HandlerSuccess, POI_TYPE_FAVORITE, pTask->m_listener, pois, savedData));
            }

            if (m_enableSync && m_syncPlaceManager)
            {
                m_syncPlaceManager->SyncDB(POI_TYPE_FAVORITE);
            }
        }
    }
}

void PlaceStorageManager::CCC_RecoverDump(nbsearch::PlaceStorageTask *task)
{
    PlaceStorageTask_Recover* pTask = static_cast<PlaceStorageTask_Recover*>(task);
    if (pTask)
    {
        IPOIDAO* poiDAO = m_datamanager->GetPOIDAO();
        char strDBID[32] = {0};
        char strGenID[32] = {0};
        nsl_sprintf(strDBID, "%d", pTask->m_favDBID);
        nsl_sprintf(strGenID, "%d", pTask->m_favGID);
        poiDAO->SetStringUserData(LTKSPM_FavoritesDBID, strDBID);
        poiDAO->SetStringUserData(LTKSPM_FavoritesGenID, strGenID);
        nsl_sprintf(strDBID, "%d", pTask->m_recDBID);
        nsl_sprintf(strGenID, "%d", pTask->m_recGID);
        poiDAO->SetStringUserData(LTKSPM_RecentcsDBID, strDBID);
        poiDAO->SetStringUserData(LTKSPM_RecentcsGenID, strGenID);

        PAL_DB_Error result = PAL_DB_OK;
        DMPOIType type = POI_TYPE_FAVORITE;
        do
        {
            std::vector<LocalPlaceManagerDumpData>::const_reverse_iterator iter = pTask->m_favorites.rbegin();
            std::vector<LocalPlaceManagerDumpData>::const_reverse_iterator crend = pTask->m_favorites.rend();
            for (; iter != crend; iter++)
            {
                result = poiDAO->AddPOI(iter->poi.get(), type, iter->userData);
                if (result != PAL_DB_OK)
                {
                    break;
                }
                result = poiDAO->UpdatePOISyncStatus(iter->poi.get(), type, (DMSyncStatusType)iter->syncStatus);
                if (result != PAL_DB_OK)
                {
                    break;
                }
                result = poiDAO->UpdatePOITimeStamp(iter->poi.get(), type, iter->timestamp);
                if (result != PAL_DB_OK)
                {
                    break;
                }
            }
            if (result != PAL_DB_OK)
            {
                break;
            }
            type = (DMPOIType) (type - 1);
        }while (type != POI_TYPE_NONE);

        LPM_ASSERT(result == PAL_DB_OK || result == PAL_DB_FULL);
        if (result != PAL_DB_OK)
        {
            if (m_UITaskQueue)
            {
                m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerError(this, &PlaceStorageManager::UI_HanlderError, type, result, pTask->m_listener));
            }
            return;
        }
        std::vector<POI*> favorites;
        std::vector<POI*> recents;
        std::map<uint32, LocalPlaceUserData >* favoriteUserData = new std::map<uint32, LocalPlaceUserData >();
        std::map<uint32, LocalPlaceUserData >* recentUserData = new std::map<uint32, LocalPlaceUserData >();
        shared_ptr<void> guardF(favoriteUserData, ReleaseUserData<std::map<uint32, LocalPlaceUserData > >);
        shared_ptr<void> guardR(recentUserData, ReleaseUserData<std::map<uint32, LocalPlaceUserData > >);
        type = POI_TYPE_FAVORITE;
        do
        {
            result = poiDAO->GetPOI(type, favorites, *favoriteUserData);
            if (result != PAL_DB_OK)
            {
                break;
            }
            type = POI_TYPE_RECENT;
            result = poiDAO->GetPOI(type, recents, *recentUserData);
        }while (0);
        LPM_ASSERT(result == PAL_DB_OK);
        if (result != PAL_DB_OK)
        {
            if (m_UITaskQueue)
            {
                m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerError(this, &PlaceStorageManager::UI_HanlderError, type, result, pTask->m_listener));
            }
            return;
        }

        if (m_UITaskQueue)
        {
            m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerSucess(this, &PlaceStorageManager::UI_HandlerSuccess, POI_TYPE_FAVORITE, NULL, favorites, favoriteUserData));
            m_UITaskQueue->AddTask(new PlaceStorageUITask_HandlerSucess(this, &PlaceStorageManager::UI_HandlerSuccess, POI_TYPE_RECENT, pTask->m_listener, recents, recentUserData));
        }

//        if (m_enableSync && m_syncPlaceManager)
//        {
//            m_syncPlaceManager->SyncDB(POI_TYPE_FAVORITE);
//            m_syncPlaceManager->SyncDB(POI_TYPE_RECENT);
//        }

    }
}

void PlaceStorageManager::UI_UpdatePOIs(PlaceStorageTask* task)
{
    PlaceStorageUITask_UpdatePOIs* pTask = static_cast<PlaceStorageUITask_UpdatePOIs*>(task);
    if (pTask)
    {
        UI_POIsUpdated(pTask->m_pois, *pTask->m_savedData, pTask->m_type);
        if (pTask->m_listener)
        {
            switch (pTask->m_type)
            {
                case POI_TYPE_FAVORITE:
                    pTask->m_listener->Success(m_favorites, m_favoritesUserData);
                    break;
                case POI_TYPE_RECENT:
                    pTask->m_listener->Success(m_recents, m_recentsUserData);
                    break;
                default:
                    break;
            }
            DESTROY_LISTENER(pTask->m_listener);
        }
    }
}

void PlaceStorageManager::UI_HanlderError(PlaceStorageTask* task)
{
    PlaceStorageUITask_HandlerError* pTask = static_cast<PlaceStorageUITask_HandlerError*>(task);
    if (pTask)
    {
        LocalPlaceManagerError error = GenerateLPECode(pTask->m_type, pTask->m_result);
        if (pTask->m_listener)
        {
            pTask->m_listener->Error(error);
            DESTROY_LISTENER(pTask->m_listener);
        }
        else if (pTask->m_getPOIsListener)
        {
            pTask->m_getPOIsListener->Error(error);
            DESTROY_LISTENER(pTask->m_getPOIsListener);
        }
        UI_NotifyError(error);
    }
}

void PlaceStorageManager::UI_HandlerSuccess(PlaceStorageTask* task)
{
    PlaceStorageUITask_HandlerSucess* pTask = static_cast<PlaceStorageUITask_HandlerSucess*>(task);
    if (pTask)
    {
        if (pTask->m_listener != NULL)
        {
            pTask->m_listener->Success();
        }
        UI_POIsUpdated(pTask->m_pois, *pTask->m_savedData, pTask->m_type);
        DESTROY_LISTENER(pTask->m_listener);
    }
}

void PlaceStorageManager::SyncComplete(DMPOIType type)
{
    GetPOIs(type, NULL);
}

void PlaceStorageManager::SyncError(NB_Error error)
{
    if (error == NESERVERX_UNAUTHORIZED_ACCESS)
    {
        SetSyncWithServer(false);
    }
}

/*! @} */
