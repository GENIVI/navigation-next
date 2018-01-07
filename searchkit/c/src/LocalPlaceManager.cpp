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
 @file     LocalPlaceManager.cpp
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

#include "LocalPlaceManager.h"
#include "PlaceStorageManager.h"
#include "dbmanager.h"
#include "datamanager.h"
#include "nbcontextprotected.h"
#include "contextbasedsingleton.h"

#define LPM_ASSERT(value)   nsl_assert(value);
using namespace nbsearch;

class FakeOneShotListener: public LocalPlaceManager::GetPOIsListener
{
public:
    FakeOneShotListener(){};
    virtual void Success(const POISharedPtrVector& POIs, const std::map<uint32, LocalPlaceUserData >& data){};
};

LocalPlaceManager::Listener::~Listener()
{
    if (m_localPlaceManager)
    {
        m_localPlaceManager->UnregisterListener(this);
    }
}

shared_ptr<LocalPlaceManager> LocalPlaceManager::GetInstance(NB_Context *context)
{
    return ContextBasedSingleton<LocalPlaceManager>::getInstance(context);
}

void LocalPlaceManager::InitializeDatabase(const std::string& dbFile)
{
    m_contextCallback.callback     = &(LocalPlaceManager::MasterClear);
    m_contextCallback.callbackData = (void*)this;
    NB_ContextRegisterMasterClearCallback(m_context, &m_contextCallback);

    m_placeStorageManager->InitializeDatabase(dbFile);
}

LocalPlaceManager::LocalPlaceManager(NB_Context* context):m_context(context), m_contextCallback()
{
    nsl_memset(&m_contextCallback, 0, sizeof(NB_ContextCallback));
    m_placeStorageManager = PlaceStorageManager::GetInstance(m_context);
}

LocalPlaceManager::~LocalPlaceManager()
{
    NB_ContextUnregisterMasterClearCallback(m_context, &m_contextCallback);
}

void LocalPlaceManager::AddFavoritePOI(const nbsearch::POI *favoritePOI, const LocalPlaceUserData& userData, OneShotListener* listener)
{
    m_placeStorageManager->AddPOI(favoritePOI, userData, POI_TYPE_FAVORITE, listener);
}

void LocalPlaceManager::AddRecentPOI(const nbsearch::POI *recentPOI, const LocalPlaceUserData& userData, OneShotListener* listener)
{
    m_placeStorageManager->AddPOI(recentPOI, userData, POI_TYPE_RECENT, listener);
}

void LocalPlaceManager::UpdateFavoritePOIName(POI const* favoritePOI, const std::string& newName, OneShotListener* listener)
{
    if (m_placeStorageManager)
    {
        m_placeStorageManager->UpdateFavoritePOIName(favoritePOI, newName, listener);
    }
}

void LocalPlaceManager::RemoveFavoritePOI(const nbsearch::POI *favoritePOI, OneShotListener* listener)
{
    m_placeStorageManager->RemovePOI(favoritePOI, POI_TYPE_FAVORITE, listener);
}

void LocalPlaceManager::RemoveRecentPOI(const nbsearch::POI *recentPOI, OneShotListener* listener)
{
    m_placeStorageManager->RemovePOI(recentPOI, POI_TYPE_RECENT, listener);
}

void LocalPlaceManager::RemoveAllFavoritePOIs(nbsearch::LocalPlaceManager::OneShotListener *listener)
{
    m_placeStorageManager->RemoveAllPOIs(POI_TYPE_FAVORITE, listener);
}

void LocalPlaceManager::RemoveAllRecentPOIs(nbsearch::LocalPlaceManager::OneShotListener *listener)
{
    m_placeStorageManager->RemoveAllPOIs(POI_TYPE_RECENT, listener);
}

void LocalPlaceManager::GetRecent(POISharedPtrVector& pois, std::map<uint32, LocalPlaceUserData >& userData)
{
    if (!m_placeStorageManager->GetCachedPOIs(POI_TYPE_RECENT, pois, userData))
    {
        m_placeStorageManager->GetPOIs(POI_TYPE_RECENT, new FakeOneShotListener());
    }
}

void LocalPlaceManager::GetFavorite(POISharedPtrVector& pois, std::map<uint32, LocalPlaceUserData >& userData)
{
    if (!m_placeStorageManager->GetCachedPOIs(POI_TYPE_FAVORITE, pois, userData))
    {
        m_placeStorageManager->GetPOIs(POI_TYPE_FAVORITE, new FakeOneShotListener());
    }
}

void LocalPlaceManager::GetRecent(nbsearch::LocalPlaceManager::GetPOIsListener *listener)
{
    m_placeStorageManager->GetPOIs(POI_TYPE_RECENT, listener);
}

void LocalPlaceManager::GetFavorite(nbsearch::LocalPlaceManager::GetPOIsListener *listener)
{
    m_placeStorageManager->GetPOIs(POI_TYPE_FAVORITE, listener);
}

void LocalPlaceManager::MoveFavorite(const nbsearch::POI *favoritePOI, int index, OneShotListener* listener)
{
    m_placeStorageManager->MoveFavorite(favoritePOI, index, listener);
}

void LocalPlaceManager::MasterClear(void* pThis)
{
    LocalPlaceManager* manager = static_cast<LocalPlaceManager*>(pThis);
    if (manager)
    {
        manager->m_placeStorageManager->MasterClear();
    }
}

void LocalPlaceManager::SetSyncWithServer(bool enable)
{
    m_placeStorageManager->SetSyncWithServer(enable);
}

void LocalPlaceManager::RegisterListener(LocalPlaceManager::Listener *listener)
{
    listener->m_localPlaceManager = m_placeStorageManager;
    m_placeStorageManager->RegisterListener(listener);
}

void LocalPlaceManager::UnregisterListener(LocalPlaceManager::Listener *listener)
{
    m_placeStorageManager->UnregisterListener(listener);
    listener->m_localPlaceManager.reset();
}

void LocalPlaceManager::RecoverFromDumpData(const std::vector<LocalPlaceManagerDumpData> & favorites,
                                            uint32 favoritesGenerationID,
                                            uint32 favoritesDBID,
                                            const std::vector<LocalPlaceManagerDumpData> & recents,
                                            uint32 recentsGenerationID,
                                            uint32 recentsDBID,
                                            OneShotListener* listener)
{
    m_placeStorageManager->RecoverFromDump(favorites, favoritesGenerationID, favoritesDBID, recents, recentsGenerationID, recentsDBID, listener);
}

/*! @} */
