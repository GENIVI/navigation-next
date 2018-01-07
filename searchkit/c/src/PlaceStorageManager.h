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
 @file     PlaceStorageManager.h
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
#ifndef search_PlaceStorageManager_h
#define search_PlaceStorageManager_h

#include "LocalPlaceManager.h"
#include <vector>
#include "poidao.h"
#include "paltypes.h"
#include "nbtaskqueue.h"
#include "nbuitaskqueue.h"
#include "SyncPlaceManager.h"
#include "offboard/SingleSearchOffboardInformation.h"
#include "palevent.h"


class DBManager;
class DataManager;

namespace nbsearch
{

class PlaceStorageTask;

class PlaceStorageManager : public SyncCompleteListener
{
public:
    //All fuctions below should only be called on UI thread
    static shared_ptr<PlaceStorageManager> GetInstance(NB_Context* context);
    //initialize POIDAO
    void InitializeDatabase(const std::string& dbFile);
    //recover from dumped
    void RecoverFromDump(const std::vector<LocalPlaceManagerDumpData> & favorites,
                         uint32 favoritesGID,
                         uint32 favoritesDBID,
                         const std::vector<LocalPlaceManagerDumpData> & recents,
                         uint32 recentsGID,
                         uint32 recentsDBID,
                         LocalPlaceManager::OneShotListener* listener);
    //get the cached POIs, return true if the cache is loaded
    bool GetCachedPOIs(DMPOIType type, std::vector<shared_ptr<POI> >& pois, std::map<uint32, std::map<std::string, std::string> >& userData);
    //make a db get data request, all results will be set to cached list once success
    void GetPOIs(DMPOIType type, LocalPlaceManager::GetPOIsListener* listener);
    //make a db add data request, all results will be set to cached list once success
    void AddPOI(POI const* poi, const LocalPlaceUserData& userData, DMPOIType type, LocalPlaceManager::OneShotListener* listener);
    //make a db remove data request, all results will be set to cached list once success
    void RemovePOI(POI const* poi, DMPOIType type, LocalPlaceManager::OneShotListener* listener);
    //make a db clear data request, cached list will be cleared immediatly
    void RemoveAllPOIs(DMPOIType type, LocalPlaceManager::OneShotListener* listener);
    //make a update favorite order request, all results will be set to cached list once success
    void MoveFavorite(POI const* poi, int index, LocalPlaceManager::OneShotListener* listener);
    // Clear all local data. Delete all db files.
    void MasterClear();
    // Update favorite poi name
    void UpdateFavoritePOIName(POI const* poi, const std::string& newName, LocalPlaceManager::OneShotListener* listener);

    // Set whether could sync places(favorites and recents) with server, default is not support.
    void SetSyncWithServer(bool enable);

    void RegisterListener(LocalPlaceManager::Listener* listener);
    void UnregisterListener(LocalPlaceManager::Listener* listener);

    //these functions are for ContextBasedSingleton, user should not call them
    inline NB_Context* GetContext(){return m_nbcontext;};
    PlaceStorageManager(NB_Context* context);
    ~PlaceStorageManager();

    //Implement SyncCompleteListener
    virtual void SyncComplete(DMPOIType type);
    virtual void SyncError(NB_Error error);

private:

    //called in ccc thread
    void CCC_SyncStatus(PlaceStorageTask* task);
    void CCC_AddPOI(PlaceStorageTask* task);
    void CCC_RemovePOI(PlaceStorageTask* task);
    void CCC_RemoveAllPOIs(PlaceStorageTask* task);
    void CCC_MoveFavorite(PlaceStorageTask* task);
    void CCC_GetPOIs(PlaceStorageTask* task);
    void CCC_UpdatePOIName(PlaceStorageTask* task);
    void CCC_RecoverDump(PlaceStorageTask* task);

    //called in UI thread
    void UI_HanlderError(PlaceStorageTask* task);
    void UI_HandlerSuccess(PlaceStorageTask* task);
    void UI_UpdatePOIs(PlaceStorageTask* task);

    void UI_POIsUpdated(const std::vector<POI*>& POIs, const std::map<uint32, std::map<std::string, std::string> >& data, DMPOIType type);
    void UI_NotifyError(LocalPlaceManagerError error);

    NB_Context* m_nbcontext;
    PAL_Instance* m_palInstance;
    shared_ptr<DataManager> m_datamanager;
    shared_ptr<DBManager> m_dbmanager;

    //members below can only be accessed from UI thread
    bool m_recentsCacheInitialized;
    bool m_favoriteCacheInitialized;
    std::vector<shared_ptr<POI> > m_recents;
    std::vector<shared_ptr<POI> > m_favorites;
    std::map<uint32, std::map<std::string, std::string> > m_recentsUserData;
    std::map<uint32, std::map<std::string, std::string> > m_favoritesUserData;
    std::vector<LocalPlaceManager::Listener*> m_listeners;

    /*! CCC worker thread */
    shared_ptr<EventTaskQueue> m_eventQueue;
    /*! UI task queue */
    shared_ptr<UiTaskQueue> m_UITaskQueue;
    SyncPlaceManagerPtr m_syncPlaceManager;
    bool m_enableSync;
    std::string m_dbFileName;
};

typedef void (PlaceStorageManager::*PlaceTaskFunction)(PlaceStorageTask* pTask);

class PlaceStorageTask : public Task
{
public:
    PlaceStorageTask(PlaceStorageManager* handler, PlaceTaskFunction function);
    virtual ~PlaceStorageTask() {}

    virtual void Execute(void);

private:
    PlaceStorageManager* m_handler;
    PlaceTaskFunction  m_function;
};

class PlaceStorageTask_AddPOI : public PlaceStorageTask
{
public:
    PlaceStorageTask_AddPOI(PlaceStorageManager* handler, PlaceTaskFunction function, const POI* poi, const LocalPlaceUserData& userData, DMPOIType type, LocalPlaceManager::OneShotListener*& listener)
    : PlaceStorageTask(handler, function), m_poi(poi != NULL ? new POIImpl(*poi) : NULL), m_userData(userData), m_type(type), m_listener(listener) {}
    ~PlaceStorageTask_AddPOI() { delete m_poi; }

    const POI* m_poi;
    const LocalPlaceUserData m_userData;
    DMPOIType m_type;
    LocalPlaceManager::OneShotListener* m_listener;
};

class PlaceStorageTask_RemovePOI : public PlaceStorageTask
{
public:
    PlaceStorageTask_RemovePOI(PlaceStorageManager* handler, PlaceTaskFunction function, POIImpl* poi, DMPOIType type, LocalPlaceManager::OneShotListener*& listener)
    : PlaceStorageTask(handler, function), m_poi(poi != NULL ? new POIImpl(*poi) : NULL), m_type(type), m_listener(listener) {}
    virtual ~PlaceStorageTask_RemovePOI() { delete m_poi; }

    POIImpl* m_poi;
    DMPOIType m_type;
    LocalPlaceManager::OneShotListener* m_listener;
};

class PlaceStorageTask_RemoveAllPOIs : public PlaceStorageTask
{
public:
    PlaceStorageTask_RemoveAllPOIs(PlaceStorageManager* handler, PlaceTaskFunction function, DMPOIType type, LocalPlaceManager::OneShotListener*& listener)
    : PlaceStorageTask(handler, function), m_type(type), m_listener(listener) {}
    virtual ~PlaceStorageTask_RemoveAllPOIs() {}

    DMPOIType m_type;
    LocalPlaceManager::OneShotListener* m_listener;
};

class PlaceStorageTask_MoveFavorite : public PlaceStorageTask
{
public:
    PlaceStorageTask_MoveFavorite(PlaceStorageManager* handler, PlaceTaskFunction function, POIImpl* poi, int index, LocalPlaceManager::OneShotListener*& listener)
    : PlaceStorageTask(handler, function), m_index(index), m_poi(poi != NULL ? new POIImpl(*poi) : NULL), m_listener(listener) {}
    virtual ~PlaceStorageTask_MoveFavorite() { delete m_poi; }

    int m_index;
    POIImpl* m_poi;
    LocalPlaceManager::OneShotListener* m_listener;
};

class PlaceStorageTask_GetPOIs : public PlaceStorageTask
{
public:
    PlaceStorageTask_GetPOIs(PlaceStorageManager* handler, PlaceTaskFunction function, DMPOIType type, LocalPlaceManager::GetPOIsListener*& listener)
    : PlaceStorageTask(handler, function), m_type(type), m_listener(listener) {}
    virtual ~PlaceStorageTask_GetPOIs() {}

    DMPOIType m_type;
    LocalPlaceManager::GetPOIsListener* m_listener;
};

class PlaceStorageUITask_UpdatePOIs : public PlaceStorageTask
{
public:
    PlaceStorageUITask_UpdatePOIs(PlaceStorageManager* handler, PlaceTaskFunction function, std::vector<POI*> pois, DMPOIType type, std::map<uint32, LocalPlaceUserData >* savedData, LocalPlaceManager::GetPOIsListener*& listener)
    : PlaceStorageTask(handler, function), m_pois(pois), m_savedData(new std::map<uint32, LocalPlaceUserData >(*savedData)), m_type(type), m_listener(listener) {}
    virtual ~PlaceStorageUITask_UpdatePOIs() { delete m_savedData; }

    std::vector<POI*> m_pois;
    std::map<uint32, LocalPlaceUserData >* m_savedData;
    DMPOIType m_type;
    LocalPlaceManager::GetPOIsListener* m_listener;
};

class PlaceStorageUITask_HandlerError : public PlaceStorageTask
{
public:
    PlaceStorageUITask_HandlerError(PlaceStorageManager* handler, PlaceTaskFunction function, DMPOIType type, PAL_DB_Error result, LocalPlaceManager::OneShotListener*& listener) : PlaceStorageTask(handler, function), m_type(type), m_result(result), m_listener(listener), m_getPOIsListener(NULL) {}
    PlaceStorageUITask_HandlerError(PlaceStorageManager* handler, PlaceTaskFunction function, DMPOIType type, PAL_DB_Error result, LocalPlaceManager::GetPOIsListener*& listener) : PlaceStorageTask(handler, function), m_type(type), m_result(result), m_listener(NULL), m_getPOIsListener(listener) {}
    virtual ~PlaceStorageUITask_HandlerError() {}

    DMPOIType m_type;
    PAL_DB_Error m_result;
    LocalPlaceManager::OneShotListener* m_listener;
    LocalPlaceManager::GetPOIsListener* m_getPOIsListener;

};

class PlaceStorageUITask_HandlerSucess : public PlaceStorageTask
{
public:
    PlaceStorageUITask_HandlerSucess(PlaceStorageManager* handler, PlaceTaskFunction function, DMPOIType type, LocalPlaceManager::OneShotListener* listener, std::vector<POI*> pois, std::map<uint32, LocalPlaceUserData >*& savedData)
    : PlaceStorageTask(handler, function), m_type(type), m_listener(listener), m_pois(pois), m_savedData(new std::map<uint32, LocalPlaceUserData >(*savedData)) {}
    virtual ~PlaceStorageUITask_HandlerSucess() { delete m_savedData; }

    DMPOIType m_type;
    LocalPlaceManager::OneShotListener* m_listener;
    std::vector<POI*> m_pois;
    std::map<uint32, LocalPlaceUserData >* m_savedData;
};

class PlaceStorageTask_UpdatePOIName : public PlaceStorageTask
{
public:
    PlaceStorageTask_UpdatePOIName(PlaceStorageManager* handler, PlaceTaskFunction function, const POI* poi, const std::string& newName, LocalPlaceManager::OneShotListener*& listener) : PlaceStorageTask(handler, function), m_newName(newName), m_poi(poi != NULL ? new POIImpl(*poi) : NULL), m_listener(listener) {}
    virtual ~PlaceStorageTask_UpdatePOIName() { delete m_poi; }

    std::string m_newName;
    POI* m_poi;
    LocalPlaceManager::OneShotListener* m_listener;
};

class PlaceStorageTask_Recover : public PlaceStorageTask
{
public:
    PlaceStorageTask_Recover(PlaceStorageManager* handler, PlaceTaskFunction function, const std::vector<LocalPlaceManagerDumpData>& favorites, const std::vector<LocalPlaceManagerDumpData>& recents, uint32 favGID, uint32 recGID, uint32 favDBID, uint32 recDBID, LocalPlaceManager::OneShotListener* listener):PlaceStorageTask(handler, function), m_favorites(favorites), m_recents(recents), m_favGID(favGID), m_recGID(recGID), m_listener(listener){}
    virtual ~PlaceStorageTask_Recover(){};
    std::vector<LocalPlaceManagerDumpData> m_favorites;
    std::vector<LocalPlaceManagerDumpData> m_recents;
    uint32 m_favGID;
    uint32 m_recGID;
    uint32 m_favDBID;
    uint32 m_recDBID;
    LocalPlaceManager::OneShotListener* m_listener;
};

}

#endif
/*! @}*/
