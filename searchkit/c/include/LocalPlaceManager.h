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
 @file     LocalPlaceManager.h
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
#ifndef __LocalPlaceManager__
#define __LocalPlaceManager__
#include <string>
#include <map>
#include <vector>
#include "SingleSearchInformation.h"
#include "nbcontext.h"
#include "smartpointer.h"

template <typename T> class ContextBasedSingleton;

namespace nbsearch
{
class PlaceStorageManager;
typedef std::vector<shared_ptr<POI> > POISharedPtrVector;   //POI result list type
typedef std::map<std::string, std::string> LocalPlaceUserData;   //LocalPlace User data type

typedef enum
{
    LPME_OK                      =   0,      /*!< No error*/
    LPME_RecentsError            =   100,    /*!< Recents error*/
    LPME_FavoritesError          =   500,    /*!< Favorite error*/
    LPME_Unexpected              =   1000    /*!< Unexpected error*/
} LocalPlaceManagerError; ///LocalPlaceManager error code.

typedef struct
{
    shared_ptr<POI> poi;            /*!< POI for sync */
    uint32 syncStatus;              /*!< sync status */
    uint32 timestamp;               /*!< local time stamp */
    int64 serverId;                 /*!< server id */
    LocalPlaceUserData userData;    /*!< user data */
} LocalPlaceManagerDumpData;

class LocalPlaceManager
{
public:
    /*! Get instance
     *
     *  Get a instance binded with the nbcontext
     *  @param context nbcontext binded with the instance
     *  @return instance of LocalPlaceManager
     */
    static shared_ptr<LocalPlaceManager> GetInstance(NB_Context* context);

    /*! Initialize the database
     *
     *  Initialize the database (create tables if not exist)
     *  @param dbFile database file to load from
     *  @return None.
     */
    void InitializeDatabase(const std::string& dbFile);
    /*! Get Recent POIs
     *
     *  Get all recents in database, ordered by timestamp decreasing
     *  @param pois result list used to storage result.
     *  @param userData userdata map, key is the index of pois result list
     *  @return None.
     */
    void GetRecent(POISharedPtrVector& pois, std::map<uint32, LocalPlaceUserData >& userData);
    /*! Get Favorite POIs
     *
     *  Get all favorites in database, ordered by favorite order.
     *  @param pois result list used to storage result.
     *  @param userData userdata map, key is the index of pois result list
     *  @return None.
     */
    void GetFavorite(POISharedPtrVector& pois, std::map<uint32, LocalPlaceUserData >& userData);

    /*! One shot listener interface
     *
     *  This interface will be called one time for responce of any user request.
     *  @note User shall not delete a OneShotListener once passed it to the LPM, </br>
     *        if user have to keep his ownership, he should override Destroy() function
     */
    class OneShotListener
    {
    public:
        virtual ~OneShotListener(){}
        virtual void Error(LocalPlaceManagerError error){}
        virtual void Success(){}
        virtual void Destroy(){delete this;}  ///this function will be called by LocalPlaceManager to destroy the listener, after this call ,LocalPlaceManager will release the ownership.
    };

    /*! Get POIs listener interface
     *
     *  This interface will be called one time for responce of any user request.
     */
    class GetPOIsListener : public OneShotListener
    {
    public:
        virtual ~GetPOIsListener(){}
        virtual void Success(const POISharedPtrVector& POIs, const std::map<uint32, LocalPlaceUserData >& data) = 0;
    };
    /*! Get Recents asynchronously
     *
     *  Get all recents in database, ordered by timestamp decreasing.
     *  @param listener: LocalPlaceManager will take the owneship of the callback instance, and it will be destroied once any one of its public function was called
     *  @return None.
     *  @see class GetPOIsListener
     */
    void GetRecent(GetPOIsListener* listener);
    /*! Get Favorites asynchronously
     *
     *  Get all favorites in database, ordered by favorite order.
     *  @param listener: LocalPlaceManager will take the owneship of the callback instance, and it will be destroied once any one of its public function was called
     *  @return None.
     *  @see class GetPOIsListener
     */
    void GetFavorite(GetPOIsListener* listener);
    /*! Add a recent POI
     *
     *  Add a POI to recent database. if there's a POI have the same
     *  location information in db, update its time stamp to current
     *  time and mark the POI as modified.
     *  @param recentPOI POI to add
     *  @param userData User data k-v pair.
     *  @param listener LocalPlaceManager will take the owneship of the callback instance, and it will be destroied once any one of its public function was called
     *  @return None.
     *  @see class GetPOIsListener
     */
    void AddRecentPOI(POI const* recentPOI, const LocalPlaceUserData& userData, OneShotListener* listener);
    /*! Add a favorite POI
     *
     *  Add a POI to favorite database, if there's a POI have the same
     *  location information in db, update its time stamp to current 
     *  time and mark the POI as modified.
     *  @param recentPOI: POI to add
     *  @param
     *  @param listener: LocalPlaceManager will take the owneship of the callback instance, and it will be destroied once any one of its public function was called
     */
    void AddFavoritePOI(POI const* favoritePOI, const LocalPlaceUserData& userData, OneShotListener* listener);
    /*! Update a favorite poi's name.
     *
     *  If there's a favorite poi has the same location in db, update it's name by newName.
     *  @param favoritePOI: POI to update
     *  @param new favorite poi name
     *  @param listener: LocalPlaceManager will take the owneship of the callback instance, and it will be destroied once any one of its public function was called
     */
    void UpdateFavoritePOIName(POI const* favoritePOI, const std::string& newName, OneShotListener* listener);
    /*! Remove a recent POI
     *
     *  Remove a POI from recent database.
     *  @param recentPOI: POI to remove
     *  @param listener: LocalPlaceManager will take the owneship of the callback instance, and it will be destroied once any one of its public function was called
     */
    void RemoveRecentPOI(POI const* recentPOI, OneShotListener* listener);
    /*! Remove a favorite POI
     *
     *  Remove a POI from favorite database.
     *  @param recentPOI: POI to remove
     *  @param listener: LocalPlaceManager will take the owneship of the callback instance, and it will be destroied once any one of its public function was called
     */
    void RemoveFavoritePOI(POI const* favoritePOI, OneShotListener* listener);
    /*! Remove all recent POIs
     *
     *  Remove all POIs from recent database.
     *  @param listener: LocalPlaceManager will take the owneship of the callback instance, and it will be destroied once any one of its public function was called
     */
    void RemoveAllRecentPOIs(OneShotListener* listener);
    /*! Remove all favorite POIs
     *
     *  Remove all POIs from favorite database.
     *  @param listener: LocalPlaceManager will take the owneship of the callback instance, and it will be destroied once any one of its public function was called
     */
    void RemoveAllFavoritePOIs(OneShotListener* listener);
    /*! Reorder a favorite POI
     *
     *  Move a favorite POI to specified index, if index is larger or equal
     *  to the count, the POI will be moved to buttom of the list.
     *  Do nothing if the POI is not in database.
     *  @param favoritePOI: poi to move
     *  @param index: index the poi will move to
     *  @param listener: LocalPlaceManager will take the owneship of the callback instance, and it will be destroied once any one of its public function was called
     */
    void MoveFavorite(POI const* favoritePOI, int index, OneShotListener* listener);

    /*! Set whether could sync places(favorites and recents) with server, default is not support.
     *
     *  @param enable: true, could sync with server; false not sync.
     */
    void SetSyncWithServer(bool enable);

    /*! LocalPlaceManager listener interface
     *  
     *  @see RegisterListener(LocalPlaceManager::Listener*)
     *  @see UnregisterListener(LocalPlaceManager::Listener*)
     */
    class Listener
    {
    public:
        virtual ~Listener(); ///Listener will be unregistered from LocalPlaceManager once deleted
        virtual void Error(LocalPlaceManagerError error) = 0;
        /*! Recents data updated
         *
         *  Recents data was updated, this just indicates an update operation successed, the results could be the same like before (for example user just updated the timestamp of the first recent POI)
         */
        virtual void RecentsUpdated(const POISharedPtrVector& recents, const std::map<uint32, LocalPlaceUserData >& data) = 0;
        /*! Favorties data updated
         *
         *  Favorites data was updated, this just indicates an update operation successed, the results could be the same like before (for example user just moved the first favorite POI to index 0)
         */
        virtual void FavoritesUpdated(const POISharedPtrVector& favorites, const std::map<uint32, LocalPlaceUserData >& data) = 0;
    private:
        friend class LocalPlaceManager;
        shared_ptr<PlaceStorageManager> m_localPlaceManager; ///will reset to NULL after LocalPlaceManager::UnregisterListener was called
    };
    /*! Register a listener
     *
     *  @param listener LocalPlaceManager listener
     *  @see class LocalPlaceManager::Listener
     *  @see LocalPlaceManager::Listener.m_localPlaceManager
     */
    void RegisterListener(LocalPlaceManager::Listener* listener);
    /*! Unregister a listener
     *
     *  @param listener LocalPlaceManager listener
     *  @see class LocalPlaceManager::Listener
     *  @see LocalPlaceManager::Listener.m_localPlaceManager
     */
    void UnregisterListener(LocalPlaceManager::Listener* listener);
    /*! Used to generate db based on provided data. Will delete current db
     *
     *  @param favorites favorites data
     *  @param favoritesGenerationID generation id of favorites
     *  @param favoritesDBID db id of favorites
     *  @param recents recents data
     *  @param recentsGenerationID generation id of recents
     *  @param recentsDBID db id of recents
     *  @param listener: LocalPlaceManager will take the owneship of the callback instance, and it will be destroied once any one of its public function was called
     */
    void RecoverFromDumpData(const std::vector<LocalPlaceManagerDumpData> & favorites,
                             uint32 favoritesGenerationID,
                             uint32 favoritesDBID,
                             const std::vector<LocalPlaceManagerDumpData> & recents,
                             uint32 recentsGenerationID,
                             uint32 recentsDBID,
                             OneShotListener* listener);

private:
    /*! Clear all local data.
     *
     *  After invoke this method, app must been restartted.
     */
    static void MasterClear(void* pThis);

private:
    friend class ContextBasedSingleton<LocalPlaceManager>;
    /// Functions for ContextBasedSingleton
    NB_Context* GetContext(){return m_context;}
    LocalPlaceManager(NB_Context* context);
    ~LocalPlaceManager();

    shared_ptr<PlaceStorageManager> m_placeStorageManager;
    NB_Context* m_context;
    NB_ContextCallback m_contextCallback;
};
}

#endif //__LocalPlaceManager__

/*! @} */