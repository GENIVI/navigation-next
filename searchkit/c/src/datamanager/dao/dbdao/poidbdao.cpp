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

    @file POIDBDAO.cpp
    See header file for description.

*/
/*
    (C) Copyright 2014 by TeleCommunication Systems

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

#define __STDC_LIMIT_MACROS

#include "poidbdao.h"
#include "palclock.h"
#include "offboard/SingleSearchOffboardInformation.h"
#include "dbmanager.h"
#include "dbdaofactory.h"
#include <string>
#include <time.h>
#include <iostream>
#include <stdint.h>
using namespace std;

#ifndef DEBUG
#define CHECK_SQL_ERROR(ret)\
            if(ret != SQLITE_OK){break;}
#define LLM_ASSERT(value)\
            if(!(value)){break;}
#else
#define CHECK_SQL_ERROR(ret)\
            nsl_assert(ret == SQLITE_OK);
#define LLM_ASSERT(value)\
            nsl_assert(value);
#endif

// transaction
static const char* g_startTransaction = "BEGIN TRANSACTION;";
static const char* g_commitTransaction = "COMMIT TRANSACTION;";
static const char* g_rollbackTransaction = "ROLLBACK TRANSACTION;";

// insert
static const char* g_insertRecentPlaceLocation = "INSERT INTO Recents (area, streetnumber, street1, street2, city, state, county, postal, country, airport, latitude, longitude, ExData, UserData, locationtype, Name, SearchFilter, timestamp, SyncStatus) VALUES (@area, @streetnumber, @street1, @street2, @city, @state, @county, @postal, @country, @airport, @latitude, @longitude, @exdata, @userdata, @locationtype, @name, @sf, @ts, @syncstatus);";
static const char* g_insertRecentPlaceCategory = "INSERT INTO PlaceCategory (locationID, name, code, parentname,parentcode, placetype) VALUES (@locationid, @name, @code,@parentname,@parentcode, 'R');";
static const char* g_insertRecentPlacePhoneNumber = "INSERT INTO PlacePhoneNumber (locationID, phoneNumberType, areaCode, countryCode, localNumber, formattedNumber, placetype) VALUES (@locationid, @phonenumbertype, @areacode, @countrycode, @localnumber, @formattednumber, 'R');";
static const char* g_insertFavoritePlaceLocation = "INSERT INTO Favorites(area, streetnumber, street1, street2, city, state, county, postal, country, airport, latitude, longitude, ExData, UserData, locationtype, Name,SearchFilter, favOrder, timestamp, SyncStatus) VALUES (@area, @streetnumber, @street1, @street2, @city, @state, @county, @postal, @country, @airport, @latitude, @longitude,@exdata, @userdata, @locationtype, @name,@sf, @favOrder, @ts, @syncstatus);";
static const char* g_insertFavoritePlaceCategory = "INSERT INTO PlaceCategory (locationID, name, code, parentname,parentcode, placetype) VALUES (@locationid, @name, @code,@parentname,@parentcode, 'F');";
static const char* g_insertFavoritePlacePhoneNumber = "INSERT INTO PlacePhoneNumber (LocationID, PhoneNumberType, AreaCode, CountryCode, LocalNumber, FormattedNumber, PlaceType) VALUES (@locationid, @phonenumbertype, @areacode, @countrycode, @localnumber, @formattednumber,'F');";

static const char* const g_insertUserData = "INSERT OR REPLACE into UserData ('key', 'data') VALUES (@key, @data)";

// remove
static const char* g_deleteRecentPlaceLocation = "DELETE FROM Recents WHERE locationID=@locationid;";
static const char* g_deleteRecentPlaceCategory = "DELETE FROM PlaceCategory WHERE locationID = @locationid AND placetype = 'R';";
static const char* g_deleteRecentPlacePhoneNumber = "DELETE FROM PlacePhoneNumber WHERE locationID = @locationid AND placetype = 'R';";
static const char* g_deleteAllRecentPlace = "DELETE FROM Recents;DELETE FROM PlaceCategory WHERE placetype = 'R';DELETE FROM PlacePhoneNumber WHERE placetype = 'R';";

static const char* g_deleteFavoritePlaceLocation = "DELETE FROM Favorites WHERE LocationID=@locationid;";
static const char* g_deleteFavoritePlaceCategory = "DELETE FROM PlaceCategory WHERE locationID = @locationid AND placetype = 'F';";
static const char* g_deleteFavoritePlacePhoneNumber = "DELETE FROM PlacePhoneNumber WHERE locationID = @locationid AND placetype = 'F';";
static const char* g_deleteAllFavoritePlace = "DELETE FROM Favorites;DELETE FROM PlaceCategory WHERE placetype = 'F';DELETE FROM PlacePhoneNumber WHERE placetype = 'F';";


static const char* const g_resetUserData = "DELETE FROM UserData";

// update
static const char* g_updateRecentPlaceLocation = "UPDATE Recents SET area=@area, streetnumber=@streetnumber, street1=@street1, street2=@street2, city=@city, state=@state, county=@county, postal=@postal, country=@country, airport=@airport, latitude=@latitude, longitude=@longitude, locationtype=@locationtype, name=@name,searchfilter=@sf, timestamp=@ts, SyncStatus=@syncstatus, ExData=@exdata WHERE locationId=@locationid;";
static const char* g_updateRecentPlaceLocationWithUD = "UPDATE Recents SET area=@area, streetnumber=@streetnumber, street1=@street1, street2=@street2, city=@city, state=@state, county=@county, postal=@postal, country=@country, airport=@airport, latitude=@latitude, longitude=@longitude, locationtype=@locationtype, name=@name,searchfilter=@sf, timestamp=@ts, SyncStatus=@syncstatus, ExData=@exdata, userdata=@userdata WHERE locationId=@locationid;";
static const char* g_updateRecentSyncStatus = "UPDATE Recents SET SyncStatus = @syncStatus WHERE locationId = @locationid;";
static const char* g_updateRecentSyncStatusWithServerId = "UPDATE Recents SET SyncStatus = @syncStatus, serverId = @serverid WHERE locationId = @locationid;";
static const char* g_updateRecentTimeStamp = "UPDATE Recents SET TimeStamp=@ts WHERE locationId=@locationid;";
static const char* g_markRecentAsDeletedByLocationID = "UPDATE Recents SET SyncStatus = 16 WHERE LocationId = @locationid;";

static const char* g_updateFavoritePlaceLocation = "UPDATE Favorites SET area=@area, streetnumber=@streetnumber, street1=@street1, street2=@street2, city=@city, state=@state, county=@county, postal=@postal, country=@country, airport=@airport, latitude=@latitude, longitude=@longitude, locationtype=@locationtype, name=@name,searchfilter=@sf, timestamp=@ts, SyncStatus=@syncstatus, ExData=@exdata, WHERE locationId=@locationid;";
static const char* g_updateFavoritePlaceLocationWithUD = "UPDATE Favorites SET area=@area, streetnumber=@streetnumber, street1=@street1, street2=@street2, city=@city, state=@state, county=@county, postal=@postal, country=@country, airport=@airport, latitude=@latitude, longitude=@longitude, locationtype=@locationtype, name=@name,searchfilter=@sf, timestamp=@ts, SyncStatus=@syncstatus, ExData=@exdata, userdata=@userdata WHERE locationId=@locationid;";
static const char* g_updateFavoriteSyncStatus = "UPDATE Favorites SET SyncStatus = @syncStatus WHERE locationId = @locationid;";
static const char* g_updateFavoriteSyncStatusWithServerId = "UPDATE Favorites SET SyncStatus = @syncStatus, serverId = @serverid WHERE locationId = @locationid;";
static const char* g_updateFavoriteTimeStamp = "UPDATE Favorites SET TimeStamp=@ts WHERE locationId=@locationid;";
static const char* g_updateFavoriteOrder = "UPDATE Favorites SET favOrder=@favOrder WHERE locationId=@locationid;";
static const char* g_markFavoriteAsDeletedByLocationID = "UPDATE Favorites SET SyncStatus = 16 WHERE LocationId = @locationid;";
static const char* g_pdateFavoriteName = "UPDATE Favorites SET Name=@name, SyncStatus=@syncstatus WHERE locationId=@locationid;";

// get
static const char* g_getRecentPlaces = "SELECT * From Recents WHERE SyncStatus != 16 ORDER by Timestamp DESC;";
static const char* g_getRecentPlacesByName = "SELECT * FROM Recents WHERE Name = @name AND SyncStatus != 16 ORDER by Timestamp DESC;";
static const char* g_getRecentPlacesForSync = "SELECT * FROM Recents WHERE SyncStatus != 0 ORDER by Timestamp DESC;";
static const char* g_getRecentPlaceCount = "SELECT COUNT(*) FROM Recents WHERE SyncStatus != 16;";
static const char* g_getRecentPlacesCategories = "SELECT a.* FROM PlaceCategory a, Recents b WHERE a.LocationID = b.LocationID AND PlaceType = 'R';";
static const char* g_getRecentPlacesPhoneNumbers = "SELECT a.* FROM PlacePhoneNumber a, Recents b WHERE a.LocationID=b.LocationID AND PlaceType = 'R';";
static const char* g_getRecentPlaceSyncStatus = "SELECT SyncStatus FROM Recents WHERE LocationId = @locationid;";
static const char* g_getRecentIDHasMinTimeStamp = "SELECT locationId FROM Recents WHERE timestamp = (SELECT min(timestamp) FROM Recents)";

static const char* g_getFavoritePlaces = "SELECT * FROM Favorites WHERE SyncStatus !=16 ORDER BY favOrder ASC;";
static const char* g_getFavoritePlacesIDs = "SELECT LocationId FROM Favorites WHERE SyncStatus !=16 ORDER BY favOrder ASC;";
static const char* g_getFavoritePlacesByName = "SELECT * FROM Favorites WHERE Name = @name AND SyncStatus != 16 ORDER by Timestamp ASC;";
static const char* g_getFavoritePlacesForSync = "SELECT * FROM Favorites WHERE SyncStatus != 0 ORDER by Timestamp ASC;";
static const char* g_getFavoritePlaceCount = "SELECT COUNT(*) FROM Favorites WHERE SyncStatus != 16;";
static const char* g_getFavoritePlacesCategories = "SELECT a.* FROM PlaceCategory a, Favorites b WHERE a.LocationID = b.LocationID AND PlaceType = 'F';";
static const char* g_getFavoritePlacesPhoneNumbers = "SELECT a.* FROM PlacePhoneNumber a, Favorites b WHERE a.LocationID=b.LocationID AND PlaceType = 'F';";
static const char* g_getFavoritePlaceSyncStatus = "SELECT SyncStatus FROM Favorites WHERE LocationId = @locationid;";
static const char* g_getFavoritePlacesOrder = "SELECT favOrder FROM Favorites WHERE LocationId = @locationid;";
static const char* g_getFavoritePlacesMaxOrder = "SELECT max(favOrder) FROM Favorites;";

static const char* const g_getUserData = "SELECT Data FROM UserData WHERE Key=@key";

//update order
static const char* g_updateFavoritePlacesMoveOrderIncrease = "UPDATE Favorites SET favOrder=favOrder-1 WHERE favOrder>@orderold AND favOrder<=@ordernew;";
static const char* g_updateFavoritePlacesMoveOrderDecrease = "UPDATE Favorites SET favOrder=favOrder+1 WHERE favOrder<@orderold AND favOrder>=@ordernew;";
static const char* g_updateFavoritePlacesUpdateOrderForRemove = "UPDATE Favorites SET favOrder=favOrder-1 WHERE favOrder>(SELECT favOrder FROM Favorites WHERE LocationId = @locationid);";
static const char* g_updateFavoritePlacesSyncStatusMoveOrderIncrease = "UPDATE Favorites SET SyncStatus=1 WHERE favOrder>@orderold AND favOrder<=@ordernew AND SyncStatus==0;";
static const char* g_updateFavoritePlacesSyncStatusMoveOrderDecrease = "UPDATE Favorites SET SyncStatus=1 WHERE favOrder<@orderold AND favOrder>=@ordernew AND SyncStatus==0;";

//check
//don't check lat/long to follow android implement
static const char* g_checkFavoritePlace = "SELECT LocationId FROM Favorites WHERE Name=@name AND Area=@area AND StreetNumber=@streetnumber AND Street1=@street1 AND Street2=@street2 AND City=@city AND State=@state AND County=@county AND Postal=@postal AND Country=@country AND Airport=@airport;";// AND Latitude=@latitude AND Longitude=@longitude;";
static const char* g_checkRecentPlace = "SELECT LocationId FROM Recents WHERE Name=@name AND Area=@area AND StreetNumber=@streetnumber AND Street1=@street1 AND Street2=@street2 AND City=@city AND State=@state AND County=@county AND Postal=@postal AND Country=@country AND Airport=@airport;";// AND Latitude=@latitude AND Longitude=@longitude;";
static const char* g_checkFavoriteServerId = "SELECT LocationId FROM Favorites WHERE serverid=@serverid;";
static const char* g_checkRecentServerId = "SELECT LocationId FROM Recents WHERE serverid=@serverid;";

static const char* g_getLastInsertedRowID = "SELECT last_insert_rowid();";

//create table
static const char* g_createFavoriteTalbe = "CREATE TABLE IF NOT EXISTS Favorites (Area VARCHAR(100), Name VARCHAR(100),StreetNumber VARCHAR(20),Street1 VARCHAR(50),Street2 VARCHAR(50),City VARCHAR(50),State VARCHAR(50),County VARCHAR(50),Postal VARCHAR(10),Country VARCHAR(3),Airport VARCHAR(3),Latitude FLOAT,Longitude FLOAT,LocationType INTEGER, ExData BLOB, UserData BLOB, SearchFilter TEXT, favOrder INTEGER, LocationId INTEGER PRIMARY KEY, Timestamp INTEGER, ServerId INTEGER, SyncStatus INTEGER);";
static const char* g_createRecnetTable = "CREATE TABLE IF NOT EXISTS Recents (Area VARCHAR(100),StreetNumber VARCHAR(20),Street1 VARCHAR(50),Street2 VARCHAR(50),City VARCHAR(50),State VARCHAR(50),County VARCHAR(50),Postal VARCHAR(10),Country VARCHAR(3),Airport VARCHAR(3),Latitude FLOAT,Longitude FLOAT,LocationType INTEGER, ExData BLOB, UserData BLOB,Name VARCHAR(100), SearchFilter TEXT,LocationId INTEGER PRIMARY KEY ,Timestamp INTEGER, ServerId INTEGER, SyncStatus INTEGER);";

static const char* g_createPlaceCategoryTable = "CREATE TABLE IF NOT EXISTS PlaceCategory (LocationID INTEGER NOT NULL ,Name VARCHAR(50),Code VARCHAR(5) NOT NULL , ParentName VARCHAR(50),ParentCode VARCHAR(5), PlaceType CHAR, PRIMARY KEY (LocationID,Code,PlaceType) );";
static const char* g_createPlacePhoneNumberTable = "CREATE TABLE IF NOT EXISTS PlacePhoneNumber (LocationID INTEGER NOT NULL ,PhoneNumberType INTEGER NOT NULL ,AreaCode VARCHAR(6),CountryCode VARCHAR(6),LocalNumber VARCHAR(15), formattedNumber VARCHAR(30), PlaceType CHAR);";

static const char* const g_createUserDataTable = "CREATE TABLE IF NOT EXISTS 'UserData' ('Key' VARCHAR PRIMARY KEY  NOT NULL , 'Data' BLOB);";

//extradata keys
static const char* k_POIPlaceLocationCompactAddressLine1 = "POIPlaceLocationCompactAddressLine1";
static const char* k_POIPlaceLocationCompactAddressLine2 = "POIPlaceLocationCompactAddressLine2";
static const char* k_POIPlaceLocationCompactAddressLineSeparator = "POIPlaceLocationCompactAddressLineSeparator";
static const char* k_POIPlaceLocationCompactAddressAddressLine = "POIPlaceLocationCompactAddressAddressLine";

inline std::map<std::string, std::string> CreateExDataFromPOI(const nbsearch::POI* poi)
{
    std::map<std::string, std::string> map;
    if (poi->GetPlace() != NULL)
    {
        nbsearch::Place const* place = poi->GetPlace();
        if (place->GetLocation() != NULL)
        {
            nbsearch::Location const* location = place->GetLocation();
            nbsearch::CompactAddress const& caddr = location->GetCompactAddress();
            map[k_POIPlaceLocationCompactAddressLine1] = caddr.GetLine1();
            map[k_POIPlaceLocationCompactAddressLine2] = caddr.GetLine2();
            map[k_POIPlaceLocationCompactAddressLineSeparator] = caddr.GetLineSeparator();
            map[k_POIPlaceLocationCompactAddressAddressLine] = caddr.GetAddressLine();
        }
    }
    return map;
}

inline void SetExDataToPOIImpl(const std::map<std::string, std::string>& exdata, nbsearch::POIImpl* poi)
{
    nbsearch::PlaceImpl*& place = poi->m_place;
    if (place == NULL)
    {
        place = new nbsearch::PlaceImpl();
    }
    nbsearch::Location*& location = place->m_location;
    if (location == NULL)
    {
        location = new nbsearch::Location();
    }
    nbsearch::CompactAddress& caddr = location->m_compAddress;
    std::map<std::string, std::string>::const_iterator iter;
    //setting compact address
    iter = exdata.find(k_POIPlaceLocationCompactAddressLine1);
    if (iter != exdata.end())
    {
        caddr.m_line1 = iter->second;
    }
    iter = exdata.find(k_POIPlaceLocationCompactAddressLine2);
    if (iter != exdata.end())
    {
        caddr.m_line2 = iter->second;
    }
    iter = exdata.find(k_POIPlaceLocationCompactAddressLineSeparator);
    if (iter != exdata.end())
    {
        caddr.m_lineSeparator = iter->second;
    }
    iter = exdata.find(k_POIPlaceLocationCompactAddressAddressLine);
    if (iter != exdata.end())
    {
        caddr.m_addressLine = iter->second;
    }
}

POIDBDAO::POIDBDAO(shared_ptr<DBManager> manager)
{
    m_dbmanager = manager;
}

POIDBDAO::~POIDBDAO()
{
}

PAL_DB_Error
POIDBDAO::Initialize()
{
    PAL_DB_Error result = PAL_DB_ERROR;

    DBConnection* connection = m_dbmanager->GetConnection(DBT_POI);
    if (connection)
    {
        do
        {
            result = ExecuteOneLineCommand(connection, g_createFavoriteTalbe);
            CHECK_SQL_ERROR(result);
            result = ExecuteOneLineCommand(connection, g_createRecnetTable);
            CHECK_SQL_ERROR(result);
            result = ExecuteOneLineCommand(connection, g_createPlaceCategoryTable);
            CHECK_SQL_ERROR(result);
            result = ExecuteOneLineCommand(connection, g_createPlacePhoneNumberTable);
            CHECK_SQL_ERROR(result);
            result = ExecuteOneLineCommand(connection, g_createUserDataTable);
            CHECK_SQL_ERROR(result);
        }
        while (0);

        m_dbmanager->ReleaseConnection(connection);
        connection = NULL;
    }

    return result;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::GetPOI(DMPOIType type, std::vector<nbsearch::POI *> &result, std::map<uint32, std::map<std::string, std::string> > &userData)
{
    PAL_DB_Error ret = SQLITE_ERROR;

    // Get a connection from the pool
    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if ( pConnection != NULL )
    {
        do
        {
            // Get all places
            // init command
            const char* pSql = NULL;
            switch ( type )
            {
                case POI_TYPE_RECENT:
                    pSql = g_getRecentPlaces;
                    break;
                case POI_TYPE_FAVORITE:
                    pSql = g_getFavoritePlaces;
                    break;
                default:
                    break;
            }
            LLM_ASSERT(pSql != NULL);
            DBCommand cmd = pConnection->CreateCommand( pSql );
            LLM_ASSERT(cmd.IsValid());

            // read data
            DBReader* pReader = cmd.ExecuteReader();
            nbsearch::POIImpl* pPOI = NULL;
            uint32 id = 0;
            DMSyncStatusType syncstatus = SYNC_STATUS_NONE;
            uint32 timeStamp = 0;
            int64 serverId = INT64_MAX;
            std::map<std::string, std::string> userdata;
            ret = ExtractAPOI(pReader, pPOI, id, syncstatus, timeStamp, serverId, userdata);
            result.clear();
            int index = 0;
            while ( ret == SQLITE_OK )
            {
                ret = GetAllMatchedCategories(pConnection, type, pPOI, id);
                LLM_ASSERT(ret == SQLITE_DONE || ret == SQLITE_OK);
                ret = GetAllMatchedPhoneNumbers(pConnection, type, pPOI, id);
                LLM_ASSERT(ret == SQLITE_DONE || ret == SQLITE_OK);
                result.push_back( pPOI );
                if (!userdata.empty())
                {
                    userData[index] = userdata;
                }
                index ++;
                ret = ExtractAPOI(pReader, pPOI, id, syncstatus, timeStamp, serverId, userdata);
            }
            if ( ret == SQLITE_DONE )
            {
                ret = SQLITE_OK;
            }
        } while ( 0 );

        // Release connection to the pool
        m_dbmanager->ReleaseConnection( pConnection );

        if (ret != SQLITE_OK)
        {
            // release all
            std::vector<nbsearch::POI*>::iterator iter = result.begin();
            while ( iter != result.end() )
            {
                delete *iter;
                iter ++;
            }
            result.clear();
        }
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::GetPOI(const char *name, DMPOIType type, std::vector<nbsearch::POI *> &result)
{
    PAL_DB_Error ret = SQLITE_ERROR;

    // Get a connection from the pool
    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if ( pConnection != NULL )
    {
        do
        {
            // Get all places
            {
                // init command
                const char* pSql = NULL;
                switch ( type )
                {
                    case POI_TYPE_RECENT:
                        pSql = g_getRecentPlacesByName;
                        break;
                    case POI_TYPE_FAVORITE:
                        pSql = g_getFavoritePlacesByName;
                        break;
                    default:
                        break;
                }
                LLM_ASSERT(pSql != NULL);
                DBCommand cmd = pConnection->CreateCommand( pSql );
                if ( !cmd.IsValid() )
                {
                    break;
                }
                ret = cmd.BindStringParam( "@name", name );
                CHECK_SQL_ERROR(ret);

                // read data
                DBReader* pReader = cmd.ExecuteReader();
                nbsearch::POIImpl* pPOI = NULL;
                uint32 internalId = 0;
                DMSyncStatusType syncstatus = SYNC_STATUS_NONE;
                uint32 timeStamp = 0;
                int64 serverId = INT64_MAX;
                std::map<std::string, std::string> userdata;
                ret = ExtractAPOI(pReader, pPOI, internalId, syncstatus, timeStamp, serverId, userdata);
                result.clear();
                while ( ret == SQLITE_OK )
                {
                    ret = GetAllMatchedCategories(pConnection, type, pPOI, internalId);
                    LLM_ASSERT(ret == SQLITE_DONE || ret == SQLITE_OK);
                    ret = GetAllMatchedPhoneNumbers( pConnection, type, pPOI, internalId);
                    LLM_ASSERT(ret == SQLITE_DONE || ret == SQLITE_OK);
                    result.push_back( pPOI );
                    ret = ExtractAPOI(pReader, pPOI, internalId, syncstatus, timeStamp,serverId, userdata);
                }
                LLM_ASSERT(ret == SQLITE_DONE);
            }
            if (ret == SQLITE_DONE)
            {
                ret = SQLITE_OK;
            }
        } while ( 0 );

        // Release connection to the pool
        m_dbmanager->ReleaseConnection( pConnection );

        if (ret != SQLITE_OK)
        {
            // release all
            std::vector<nbsearch::POI*>::iterator iter = result.begin();
            while ( iter != result.end() )
            {
                delete *iter;
                iter ++;
            }
            result.clear();
        }
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::GetPOIForSync(DMPOIType type, std::vector<POIForSync> &result)
{
    PAL_DB_Error ret = SQLITE_ERROR;

    // Get a connection from the pool
    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if ( pConnection != NULL )
    {
        do
        {
            // Get all places
            {
                // init command
                const char* pSql = NULL;
                switch ( type )
                {
                    case POI_TYPE_RECENT:
                        pSql = g_getRecentPlacesForSync;
                        break;
                    case POI_TYPE_FAVORITE:
                        pSql = g_getFavoritePlacesForSync;
                        break;
                    default:
                        break;
                }
                if ( pSql == NULL )
                {
                    break;
                }
                DBCommand cmd = pConnection->CreateCommand( pSql );
                if ( !cmd.IsValid() )
                {
                    break;
                }

                // read data
                DBReader* pReader = cmd.ExecuteReader();
                nbsearch::POIImpl* pPOI = NULL;
                uint32 internalId = 0;
                DMSyncStatusType syncstatus = SYNC_STATUS_NONE;
                uint32 timeStamp = 0;
                int64 serverId = 0;
                std::map<std::string, std::string> userdata;
                result.clear();
                ret = ExtractAPOI(pReader, pPOI, internalId, syncstatus, timeStamp, serverId, userdata);
                while ( ret == SQLITE_OK )
                {
                    ret = GetAllMatchedCategories(pConnection, type, pPOI, internalId);
                    LLM_ASSERT(ret == SQLITE_DONE || ret == SQLITE_OK);
                    ret = GetAllMatchedPhoneNumbers( pConnection, type, pPOI, internalId);
                    LLM_ASSERT(ret == SQLITE_DONE || ret == SQLITE_OK);
                    POIForSync pfs = {0};
                    pfs.poi = pPOI;
                    pfs.syncStatus = syncstatus;
                    pfs.timestamp = timeStamp;
                    pfs.serverId = serverId;
                    pfs.internalId = internalId;
                    Serialize<std::map<std::string, std::string> >(userdata, pfs.userData, pfs.userDataSize);
                    pfs.type = type;
                    result.push_back(pfs);
                    ret = ExtractAPOI(pReader, pPOI, internalId, syncstatus, timeStamp,serverId, userdata);
                }
                LLM_ASSERT(ret == SQLITE_DONE);
            }
            if (ret == SQLITE_DONE)
            {
                ret = SQLITE_OK;
            }
        } while ( 0 );

        if (ret == SQLITE_OK)
        {
            if (type == POI_TYPE_FAVORITE)
            {
                for (std::vector<POIForSync>::iterator iter = result.begin(); iter != result.end(); iter++)
                {
                    uint32 index = UINT32_MAX;
                    ret = GetFavoriteOrderIndex(pConnection, (*iter).internalId, index);
                    CHECK_SQL_ERROR(ret);
                    LLM_ASSERT(index != UINT32_MAX);
                    (*iter).index = index;
                }
            }
        }
        // Release connection to the pool
        m_dbmanager->ReleaseConnection( pConnection );

        if (ret != SQLITE_OK)
        {
            // release all
            std::vector<POIForSync>::iterator iter = result.begin();
            while ( iter != result.end() )
            {
                delete (*iter).poi;
                nsl_free((*iter).userData);
                iter ++;
            }
            result.clear();
        }
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::GetPOICount(DMPOIType type, uint32 &result)
{
    PAL_DB_Error ret = SQLITE_ERROR;

    // Get a connection from the pool
    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if ( pConnection != NULL )
    {
        do
        {
            const char* pSql = NULL;
            switch ( type )
            {
                case POI_TYPE_RECENT:
                    pSql = g_getRecentPlaceCount;
                    break;
                case POI_TYPE_FAVORITE:
                    pSql = g_getFavoritePlaceCount;
                    break;
                default:
                    break;
            }
            if ( pSql == NULL )
            {
                break;
            }
            DBCommand cmd = pConnection->CreateCommand( pSql );
            if ( !cmd.IsValid() )
            {
                break;
            }
            ret = cmd.ExecuteInt( (int*)&result );
        } while ( 0 );

        // Release connection to the pool
        m_dbmanager->ReleaseConnection( pConnection );
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::AddPOI(const nbsearch::POI *poi, DMPOIType type, const std::map<std::string, std::string>& userData)
{
    PAL_DB_Error ret = SQLITE_ERROR;

    // Get a connection from the pool
    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if ( pConnection != NULL )
    {
        do
        {
            uint32 locationID = CheckDuplicatePOI(pConnection, poi, type);
            if ( locationID > 0 )
            {
                uint32 order = UINT32_MAX;

                // update sync status, incase place is marked for delete, but added again
				DMSyncStatusType syncStatus = SYNC_STATUS_NONE;
                ret = GetPOISyncStatus(pConnection, locationID, type, syncStatus);
                CHECK_SQL_ERROR(ret);
				if(syncStatus != SYNC_STATUS_ADDED)
				{
                    if (syncStatus == SYNC_STATUS_DELETED && type == POI_TYPE_FAVORITE)
                    {
                        DBCommand cmd = pConnection->CreateCommand(g_getFavoritePlaceCount);
                        ret = cmd.ExecuteInt((int*)&order);
                        CHECK_SQL_ERROR(ret);
                    }
                    syncStatus = SYNC_STATUS_TIME_STAMP_MODIFIED;
				}
				else
				{
                    syncStatus = SYNC_STATUS_ADDED;
				}
                ret = UpdateAPOI(pConnection, locationID, poi, (uint32)PAL_ClockGetUnixTime(), type, syncStatus,&userData);
                CHECK_SQL_ERROR(ret);
                if (order != UINT32_MAX && type == POI_TYPE_FAVORITE)
                {
                    DBCommand cmd = pConnection->CreateCommand( g_updateFavoriteOrder );
                    LLM_ASSERT(cmd.IsValid());
                    ret = cmd.BindIntParam("@locationid", locationID);
                    CHECK_SQL_ERROR(ret);
                    ret = cmd.BindIntParam( "@favOrder", order);
                    CHECK_SQL_ERROR(ret);
                    ret = cmd.ExecuteNonQuery();
                    CHECK_SQL_ERROR(ret);
                }
                break;
            }

            // start transaction
            {
                DBCommand cmd = pConnection->CreateCommand( g_startTransaction );
                LLM_ASSERT(cmd.IsValid());
                ret = cmd.ExecuteNonQuery();
                CHECK_SQL_ERROR(ret);
            }

            do
            {
                // insert place
                uint32 count = 0;
                uint32 order = 0;
                {
                    ret = SQLITE_ERROR;
                    DMSyncStatusType syncStatus = SYNC_STATUS_ADDED;
                    uint32 timeStamp = (uint32)PAL_ClockGetUnixTime();
                    // initiallize sql command
                    const char* pSql = NULL;
                    switch ( type )
                    {
                        case POI_TYPE_RECENT:
                        {
                            DBCommand cmd = pConnection->CreateCommand(g_getRecentPlaceCount);
                            ret = cmd.ExecuteInt((int*)&count);
                            CHECK_SQL_ERROR(ret);
                            if (count >= RECENT_POI_RECORD_COUNT_LIMIT)
                            {
                                //remove first recent in db
                                uint32 oldestId = 0;
                                {
                                    DBCommand cmd = pConnection->CreateCommand(g_getRecentIDHasMinTimeStamp);
                                    ret = cmd.ExecuteInt((int*)&oldestId);
                                }
                                CHECK_SQL_ERROR(ret);
                                if (oldestId == 0)
                                {
                                    ret = SQLITE_ERROR;
                                    break;
                                }
                                DMSyncStatusType status = SYNC_STATUS_NONE;
                                ret = GetPOISyncStatus(pConnection, oldestId, type, status);
                                CHECK_SQL_ERROR(ret);
                                if ( status == SYNC_STATUS_ADDED || status == SYNC_STATUS_DELETED )
                                {
                                    ret = SQLITE_ERROR;
                                    // delete place location data
                                    {
                                        DBCommand cmd = pConnection->CreateCommand( g_deleteRecentPlaceLocation );
                                        if ( !cmd.IsValid() )
                                        {
                                            break;
                                        }
                                        ret = cmd.BindIntParam( "@locationid", oldestId );
                                        CHECK_SQL_ERROR(ret);
                                        ret = cmd.ExecuteNonQuery();
                                        CHECK_SQL_ERROR(ret);
                                    }

                                    // delete place categories
                                    {
                                        DBCommand cmd = pConnection->CreateCommand( g_deleteRecentPlaceCategory );
                                        if ( !cmd.IsValid() )
                                        {
                                            ret = SQLITE_ERROR;
                                            break;
                                        }
                                        ret = cmd.BindIntParam( "@locationid", oldestId );
                                        CHECK_SQL_ERROR(ret);
                                        ret = cmd.ExecuteNonQuery();
                                        CHECK_SQL_ERROR(ret);
                                    }

                                    // delete place phone numbers
                                    {
                                        DBCommand cmd = pConnection->CreateCommand( g_deleteRecentPlacePhoneNumber );
                                        if ( !cmd.IsValid() )
                                        {
                                            ret = SQLITE_ERROR;
                                            break;
                                        }
                                        ret = cmd.BindIntParam( "@locationid", oldestId );
                                        CHECK_SQL_ERROR(ret);
                                        ret = cmd.ExecuteNonQuery();
                                    }
                                }
                                else
                                {
                                    DBCommand cmd = pConnection->CreateCommand( g_markRecentAsDeletedByLocationID );
                                    LLM_ASSERT(cmd.IsValid());
                                    ret = cmd.BindIntParam( "@locationid", oldestId );
                                    CHECK_SQL_ERROR(ret);
                                    ret = cmd.ExecuteNonQuery();
                                    CHECK_SQL_ERROR(ret);
                                }
                            }
                            pSql = g_insertRecentPlaceLocation;
                        }
                            break;
                        case POI_TYPE_FAVORITE:
                        {
                            DBCommand cmd = pConnection->CreateCommand(g_getFavoritePlaceCount);
                            ret = cmd.ExecuteInt((int*)&count);
                            CHECK_SQL_ERROR(ret);
                            if (count >= FAVORITE_POI_RECORD_COUNT_LIMIT)
                            {
                                ret = SQLITE_FULL;
                                break;
                            }
                            order = count;
                            pSql = g_insertFavoritePlaceLocation;
                        }
                            break;
                        default:
                            break;
                    }
                    if ( pSql == NULL )
                    {
                        break;
                    }
                    DBCommand cmd = pConnection->CreateCommand( pSql );
                    if ( !cmd.IsValid() )
                    {
                        break;
                    }
                    ret = SetAllPOIParam(cmd, poi, syncStatus, timeStamp, &userData);
                    CHECK_SQL_ERROR(ret);
                    ret = cmd.ExecuteNonQuery();
                    CHECK_SQL_ERROR(ret);
                }

                // get id
                {
                    ret = SQLITE_ERROR;
                    DBCommand cmd = pConnection->CreateCommand( g_getLastInsertedRowID );
                    LLM_ASSERT(cmd.IsValid());
                    ret = cmd.ExecuteInt( (int*)&locationID );
                    CHECK_SQL_ERROR(ret);
                }

                if (type == POI_TYPE_FAVORITE)
                {
                    ret = UpdateFavoritePOIIndex(pConnection, locationID, order);
                    CHECK_SQL_ERROR(ret);
                }

                // insert categories
                {
                    ret = InsertPOICategories(pConnection, locationID, poi, type);
                    CHECK_SQL_ERROR(ret);
                }

                // insert phone numbers
                {
                    ret = InsertPOIPhoneNumbers(pConnection, locationID, poi, type);
                    CHECK_SQL_ERROR(ret);
                }
            } while ( 0 );

            if ( ret == SQLITE_OK )
            {
                // commit transaction
                DBCommand cmd = pConnection->CreateCommand( g_commitTransaction );
                LLM_ASSERT(cmd.IsValid());
                if ( !cmd.IsValid() )
                {
                    ret = SQLITE_ERROR;
                    break;
                }
                ret = cmd.ExecuteNonQuery();
            }
            else
            {
                // rollback transaction
                DBCommand cmd = pConnection->CreateCommand( g_rollbackTransaction );
                LLM_ASSERT(cmd.IsValid());
                cmd.ExecuteNonQuery();
            }
        } while ( 0 );

        // Release connection to the pool
        m_dbmanager->ReleaseConnection( pConnection );
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::RemovePOI(nbsearch::POI const*POI, DMPOIType type)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if (pConnection == NULL)
    {
        return SQLITE_ERROR;
    }
    uint32 locationId = CheckDuplicatePOI(pConnection, POI, type);
    if (locationId == 0)
    {
        m_dbmanager->ReleaseConnection( pConnection );
        return  SQLITE_NOTFOUND;
    }
    DMSyncStatusType status = SYNC_STATUS_NONE;
    ret = GetPOISyncStatus(pConnection, locationId, type, status);
    if (ret != SQLITE_OK)
    {
        return ret;
    }
    if ( status == SYNC_STATUS_ADDED || status == SYNC_STATUS_DELETED )
    {
        {
            DBCommand cmd = pConnection->CreateCommand( g_startTransaction );
            if ( !cmd.IsValid() )
            {
                return ret;
            }
            ret = cmd.ExecuteNonQuery();
            if ( ret != SQLITE_OK )
            {
                return ret;
            }
        }

        do
        {
            ret = SQLITE_ERROR;

            // set sql command
            const char* pSqlDeletePlace = NULL;
            const char* pSqlDeleteCategory = NULL;
            const char* pSqlDeletePhone = NULL;
            switch ( type )
            {
                case POI_TYPE_RECENT:
                    pSqlDeletePlace = g_deleteRecentPlaceLocation;
                    pSqlDeleteCategory = g_deleteRecentPlaceCategory;
                    pSqlDeletePhone = g_deleteRecentPlacePhoneNumber;
                    break;
                case POI_TYPE_FAVORITE:
                    pSqlDeletePlace = g_deleteFavoritePlaceLocation;
                    pSqlDeleteCategory = g_deleteFavoritePlaceCategory;
                    pSqlDeletePhone = g_deleteFavoritePlacePhoneNumber;
                    break;
                default:
                    break;
            }
            if ( pSqlDeletePlace == NULL )
            {
                break;
            }

            // delete place location data
            {
                DBCommand cmd = pConnection->CreateCommand( pSqlDeletePlace );
                if ( !cmd.IsValid() )
                {
                    break;
                }
                ret = cmd.BindIntParam( "@locationid", locationId );
                CHECK_SQL_ERROR(ret);
                ret = cmd.ExecuteNonQuery();
                CHECK_SQL_ERROR(ret);
            }

            // delete place categories
            {
                DBCommand cmd = pConnection->CreateCommand( pSqlDeleteCategory );
                if ( !cmd.IsValid() )
                {
                    ret = SQLITE_ERROR;
                    break;
                }
                ret = cmd.BindIntParam( "@locationid", locationId );
                CHECK_SQL_ERROR(ret);
                ret = cmd.ExecuteNonQuery();
                CHECK_SQL_ERROR(ret);
            }

            // delete place phone numbers
            {
                DBCommand cmd = pConnection->CreateCommand( pSqlDeletePhone );
                if ( !cmd.IsValid() )
                {
                    ret = SQLITE_ERROR;
                    break;
                }
                ret = cmd.BindIntParam( "@locationid", locationId );
                CHECK_SQL_ERROR(ret);
                ret = cmd.ExecuteNonQuery();
            }

            if (type == POI_TYPE_FAVORITE)
            {
                std::vector<uint32> ids;
                {
                    DBCommand cmd = pConnection->CreateCommand(g_getFavoritePlacesIDs);
                    LLM_ASSERT(cmd.IsValid());
                    DBReader* reader = cmd.ExecuteReader();
                    while (ret == SQLITE_OK)
                    {
                        uint32 lid = 0;
                        ret = reader->Read();
                        if (ret != SQLITE_ROW)
                        {
                            break;
                        }
                        ret = reader->GetInt("locationid", (int*)&lid);
                        CHECK_SQL_ERROR(ret);
                        ids.push_back(lid);
                    }
                    if (ret == SQLITE_DONE)
                    {
                        ret = SQLITE_OK;
                    }
                    CHECK_SQL_ERROR(ret);
                }
                uint32 index = 0;
                for (std::vector<uint32>::iterator iter = ids.begin(); iter!=ids.end(); iter++)
                {
                    DBCommand cmd = pConnection->CreateCommand( g_updateFavoriteOrder );
                    LLM_ASSERT(cmd.IsValid());
                    ret = cmd.BindIntParam("@locationid", *iter);
                    CHECK_SQL_ERROR(ret);
                    ret = cmd.BindIntParam( "@favOrder", index++);
                    CHECK_SQL_ERROR(ret);
                    ret = cmd.ExecuteNonQuery();
                    CHECK_SQL_ERROR(ret);
                }
            }
        } while ( 0 );

        if ( ret == SQLITE_OK )
        {
            // commit transaction
            DBCommand cmd = pConnection->CreateCommand( g_commitTransaction );
            if ( !cmd.IsValid() )
            {
                ret = SQLITE_ERROR;
            }
            else
            {
                ret = cmd.ExecuteNonQuery();
            }
        }
        else
        {
            // rollback transaction
            DBCommand cmd = pConnection->CreateCommand( g_rollbackTransaction );
            if ( cmd.IsValid() )
            {
                cmd.ExecuteNonQuery();
            }
        }
    }
    else
    {
        do
        {
            // create sql command.
            const char* pSql = NULL;
            switch ( type )
            {
                case POI_TYPE_RECENT:
                    pSql = g_markRecentAsDeletedByLocationID;
                    break;
                case POI_TYPE_FAVORITE:
                    pSql = g_markFavoriteAsDeletedByLocationID;
                    break;
                default:
                    break;
            }
            LLM_ASSERT(pSql != NULL);
            DBCommand cmd = pConnection->CreateCommand( pSql );
            LLM_ASSERT(cmd.IsValid());
            ret = cmd.BindIntParam( "@locationid", locationId );
            CHECK_SQL_ERROR(ret);
            ret = cmd.ExecuteNonQuery();
            CHECK_SQL_ERROR(ret);

            if (type == POI_TYPE_FAVORITE)
            {
                DBCommand cmd = pConnection->CreateCommand(g_updateFavoritePlacesUpdateOrderForRemove);
                LLM_ASSERT(cmd.IsValid());
                ret = cmd.BindIntParam( "@locationid", locationId );
                CHECK_SQL_ERROR(ret);
                ret = cmd.ExecuteNonQuery();
                CHECK_SQL_ERROR(ret);
            }
        } while ( 0 );
    }
    // Release connection to the pool

    m_dbmanager->ReleaseConnection( pConnection );
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::RemovePOI(int64 serverId, DMPOIType type)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if (pConnection == NULL)
    {
        return SQLITE_ERROR;
    }
    uint32 locationId = CheckDuplicatePOI(pConnection, serverId, type);
    if (locationId == 0)
    {
        return  SQLITE_NOTFOUND;
    }
    {
        DBCommand cmd = pConnection->CreateCommand( g_startTransaction );
        if ( !cmd.IsValid() )
        {
            return ret;
        }
        ret = cmd.ExecuteNonQuery();
        if ( ret != SQLITE_OK )
        {
            return ret;
        }
    }

    do
    {
        ret = SQLITE_ERROR;

        // set sql command
        const char* pSqlDeletePlace = NULL;
        const char* pSqlDeleteCategory = NULL;
        const char* pSqlDeletePhone = NULL;
        switch ( type )
        {
            case POI_TYPE_RECENT:
                pSqlDeletePlace = g_deleteRecentPlaceLocation;
                pSqlDeleteCategory = g_deleteRecentPlaceCategory;
                pSqlDeletePhone = g_deleteRecentPlacePhoneNumber;
                break;
            case POI_TYPE_FAVORITE:
                pSqlDeletePlace = g_deleteFavoritePlaceLocation;
                pSqlDeleteCategory = g_deleteFavoritePlaceCategory;
                pSqlDeletePhone = g_deleteFavoritePlacePhoneNumber;
                break;
            default:
                break;
        }
        if ( pSqlDeletePlace == NULL )
        {
            break;
        }

        // delete place location data
        {
            DBCommand cmd = pConnection->CreateCommand( pSqlDeletePlace );
            if ( !cmd.IsValid() )
            {
                break;
            }
            ret = cmd.BindIntParam( "@locationid", locationId );
            CHECK_SQL_ERROR(ret);
            ret = cmd.ExecuteNonQuery();
            CHECK_SQL_ERROR(ret);
        }

        // delete place categories
        {
            DBCommand cmd = pConnection->CreateCommand( pSqlDeleteCategory );
            if ( !cmd.IsValid() )
            {
                ret = SQLITE_ERROR;
                break;
            }
            ret = cmd.BindIntParam( "@locationid", locationId );
            CHECK_SQL_ERROR(ret);
            ret = cmd.ExecuteNonQuery();
            CHECK_SQL_ERROR(ret);
        }

        // delete place phone numbers
        {
            DBCommand cmd = pConnection->CreateCommand( pSqlDeletePhone );
            if ( !cmd.IsValid() )
            {
                ret = SQLITE_ERROR;
                break;
            }
            ret = cmd.BindIntParam( "@locationid", locationId );
            CHECK_SQL_ERROR(ret);
            ret = cmd.ExecuteNonQuery();
        }

        if (type == POI_TYPE_FAVORITE)
        {
            std::vector<uint32> ids;
            {
                DBCommand cmd = pConnection->CreateCommand(g_getFavoritePlacesIDs);
                LLM_ASSERT(cmd.IsValid());
                DBReader* reader = cmd.ExecuteReader();
                while (ret == SQLITE_OK)
                {
                    uint32 lid = 0;
                    ret = reader->Read();
                    if (ret != SQLITE_ROW)
                    {
                        break;
                    }
                    ret = reader->GetInt("locationid", (int*)&lid);
                    CHECK_SQL_ERROR(ret);
                    ids.push_back(lid);
                }
                if (ret == SQLITE_DONE)
                {
                    ret = SQLITE_OK;
                }
                CHECK_SQL_ERROR(ret);
            }
            uint32 index = 0;
            for (std::vector<uint32>::iterator iter = ids.begin(); iter!=ids.end(); iter++)
            {
                DBCommand cmd = pConnection->CreateCommand( g_updateFavoriteOrder );
                LLM_ASSERT(cmd.IsValid());
                ret = cmd.BindIntParam("@locationid", *iter);
                CHECK_SQL_ERROR(ret);
                ret = cmd.BindIntParam( "@favOrder", index++);
                CHECK_SQL_ERROR(ret);
                ret = cmd.ExecuteNonQuery();
                CHECK_SQL_ERROR(ret);
            }
        }
    } while ( 0 );

    if ( ret == SQLITE_OK )
    {
        // commit transaction
        DBCommand cmd = pConnection->CreateCommand( g_commitTransaction );
        if ( !cmd.IsValid() )
        {
            ret = SQLITE_ERROR;
        }
        else
        {
            ret = cmd.ExecuteNonQuery();
        }
    }
    else
    {
        // rollback transaction
        DBCommand cmd = pConnection->CreateCommand( g_rollbackTransaction );
        if ( cmd.IsValid() )
        {
            cmd.ExecuteNonQuery();
        }
    }

    // Release connection to the pool

    m_dbmanager->ReleaseConnection( pConnection );
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::RemovePOIs(DMPOIType type)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    // Get a connection from the pool
    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if ( pConnection != NULL )
    {
        do
        {
            const char* pSql = NULL;
            switch ( type )
            {
                case POI_TYPE_RECENT:
                    pSql = g_deleteAllRecentPlace;
                    break;
                case POI_TYPE_FAVORITE:
                    pSql = g_deleteAllFavoritePlace;
                    break;
                default:
                    break;
            }
            if ( pSql == NULL )
            {
                break;
            }
            DBCommand cmd = pConnection->CreateCommand( pSql );
            if ( !cmd.IsValid() )
            {
                break;
            }
            // delete location
            ret = cmd.ExecuteNonQuery();
            CHECK_SQL_ERROR(ret);
            // delete categories
            ret = cmd.Step();
            CHECK_SQL_ERROR(ret);
            ret = cmd.ExecuteNonQuery();
            CHECK_SQL_ERROR(ret);
            // delete phone numbers
            ret = cmd.Step();
            CHECK_SQL_ERROR(ret);
            ret = cmd.ExecuteNonQuery();
        } while ( 0 );

        // Release connection to the pool
        m_dbmanager->ReleaseConnection( pConnection );
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::UpdatePOI(nbsearch::POI const*POI, DMPOIType type)
{
    PAL_DB_Error ret = SQLITE_ERROR;

    // Get a connection from the pool
    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );

    if ( pConnection != NULL )
    {
        DMSyncStatusType syncStatus = SYNC_STATUS_NONE;
        uint32 locationId = CheckDuplicatePOI(pConnection, POI, type);
        GetPOISyncStatus(pConnection, locationId, type, syncStatus);

        // if current sync status is ADDED then user must be editing non synched place. In this case we do not
        // have to change status of place as modified, as it will cause server to throw error 4017
        // for modified something which never existed on server.
        if(syncStatus != SYNC_STATUS_ADDED)
        {
            syncStatus = SYNC_STATUS_PLACE_MODIFIED;
        }
        ret = UpdateAPOI(pConnection, locationId, POI, (uint32)PAL_ClockGetUnixTime(), type, syncStatus, NULL);
        // Release connection to the pool
        m_dbmanager->ReleaseConnection( pConnection );
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::GetPOISyncStatus(nbsearch::POI const*POI, DMPOIType type, DMSyncStatusType &syncStatus)
{
	PAL_DB_Error ret = SQLITE_ERROR;
    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if ( pConnection != NULL )
    {
        do
        {
            uint32 locationid = CheckDuplicatePOI(pConnection, POI, type);
            if (locationid == 0)
            {
                ret = SQLITE_NOTFOUND;
                break;
            }
            ret = GetPOISyncStatus(pConnection,locationid, type, syncStatus);
        } while ( 0 );
        m_dbmanager->ReleaseConnection(pConnection);
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::UpdatePOISyncStatus(nbsearch::POI const*POI, DMPOIType type, DMSyncStatusType syncStatus, int64 serverId)
{
	PAL_DB_Error ret = SQLITE_ERROR;
    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if ( pConnection != NULL )
    {
        do
        {
            uint32 locationid = CheckDuplicatePOI(pConnection, POI, type);
            if (locationid == 0)
            {
                ret = SQLITE_NOTFOUND;
                break;
            }
            ret = UpdatePOISyncStatus(pConnection, locationid, type, syncStatus, serverId);
        } while ( 0 );
        m_dbmanager->ReleaseConnection(pConnection);
    }
    return ret;
}

PAL_DB_Error
POIDBDAO::UpdatePOISyncStatus(uint32 locationId, DMPOIType type, DMSyncStatusType syncStatus,  int64 serverId)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if ( pConnection != NULL )
    {
        do
        {
            if (locationId == 0)
            {
                ret = SQLITE_NOTFOUND;
                break;
            }
            ret = UpdatePOISyncStatus(pConnection, locationId, type, syncStatus, serverId);
        } while ( 0 );
        m_dbmanager->ReleaseConnection(pConnection);
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::UpdatePOITimeStamp(nbsearch::POI const* POI, DMPOIType type, uint32 timeStamp)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if ( pConnection != NULL )
    {
        do
        {
            uint32 locationid = CheckDuplicatePOI(pConnection, POI, type);
            if (locationid == 0)
            {
                ret = SQLITE_NOTFOUND;
                break;
            }
            ret = UpdatePOITimeStamp(pConnection, locationid, type, timeStamp);
        } while ( 0 );
        m_dbmanager->ReleaseConnection(pConnection);
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::GetPOISyncStatus(DBConnection* pConnection,uint32 locationId, DMPOIType type, DMSyncStatusType &syncStatus)
{
    nsl_assert(locationId != 0);
	PAL_DB_Error ret = SQLITE_ERROR;
    if ( pConnection != NULL )
    {
        do
        {
			const char* sql = NULL;
			if (type == POI_TYPE_RECENT)
			{
				sql = g_getRecentPlaceSyncStatus;
			}
			else if (type == POI_TYPE_FAVORITE)
			{
				sql = g_getFavoritePlaceSyncStatus;
			}
			DBCommand cmd = pConnection->CreateCommand( sql );
            if ( !cmd.IsValid() )
            {
                break;
            }
			ret = cmd.BindDoubleParam( "@locationid", locationId);
			if ( ret != SQLITE_OK )
			{
				return ret;
			}
            ret = cmd.ExecuteInt( (int*)&syncStatus );
        } while ( 0 );
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::UpdatePOISyncStatus(DBConnection* pConnection, uint32 locationId, DMPOIType type, DMSyncStatusType syncStatus, int64 serverId)
{
    nsl_assert(locationId != 0);
	PAL_DB_Error ret = SQLITE_ERROR;
    if ( pConnection != NULL )
    {
        do
        {
            const char* sql = NULL;
            switch (type)
            {
                case POI_TYPE_RECENT:
                    if (syncStatus == SYNC_STATUS_NONE)
                    {
                        sql = g_updateRecentSyncStatusWithServerId;
                    }
                    else
                    {
                        sql = g_updateRecentSyncStatus;
                    }
                    break;
                case POI_TYPE_FAVORITE:
                    if (syncStatus == SYNC_STATUS_NONE)
                    {
                        sql = g_updateFavoriteSyncStatusWithServerId;
                    }
                    else
                    {
                        sql = g_updateFavoriteSyncStatus;
                    }
                    break;
                default:
                    break;
            }
			DBCommand cmd = pConnection->CreateCommand( sql );
            LLM_ASSERT(cmd.IsValid());
			ret = cmd.BindIntParam("@locationid", locationId);
            CHECK_SQL_ERROR(ret);
            ret = cmd.BindIntParam("@syncStatus", syncStatus);
            CHECK_SQL_ERROR(ret);
            if (syncStatus == SYNC_STATUS_NONE)
            {
                ret = cmd.BindInt64Param("@serverid", serverId);
                CHECK_SQL_ERROR(ret);
            }
            ret = cmd.ExecuteNonQuery();
            CHECK_SQL_ERROR(ret);
            if ((type == POI_TYPE_FAVORITE) && (syncStatus == SYNC_STATUS_DELETED))
            {
                DBCommand cmd = pConnection->CreateCommand(g_updateFavoritePlacesUpdateOrderForRemove);
                LLM_ASSERT(cmd.IsValid());
                ret = cmd.BindIntParam( "@locationid", locationId );
                CHECK_SQL_ERROR(ret);
                ret = cmd.ExecuteNonQuery();
                CHECK_SQL_ERROR(ret);
            }
        } while ( 0 );
        m_dbmanager->ReleaseConnection(pConnection);
    }
    return ret;
}

PAL_DB_Error
POIDBDAO::UpdatePOITimeStamp(DBConnection* pConnection, uint32 locationId, DMPOIType type, uint32 timeStamp)
{
    nsl_assert(locationId != 0);
	PAL_DB_Error ret = SQLITE_ERROR;
    if ( pConnection != NULL )
    {
        do
        {
            const char* sql = NULL;
            switch (type)
            {
                case POI_TYPE_RECENT:
                    {
                        sql = g_updateRecentTimeStamp;
                    }
                    break;
                case POI_TYPE_FAVORITE:
                    {
                        sql = g_updateFavoriteTimeStamp;
                    }
                    break;
                default:
                    break;
            }
            DBCommand cmd = pConnection->CreateCommand( sql );
            LLM_ASSERT(cmd.IsValid());
            ret = cmd.BindInt64Param("@locationid", locationId);
            CHECK_SQL_ERROR(ret);
            ret = cmd.BindIntParam("@ts", timeStamp);
            CHECK_SQL_ERROR(ret);
            ret = cmd.ExecuteNonQuery();
        } while ( 0 );
        m_dbmanager->ReleaseConnection(pConnection);
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::UpdateFavoritePOIIndex(nbsearch::POI const*POI, uint32 index)
{
	PAL_DB_Error ret = SQLITE_ERROR;
    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if ( pConnection != NULL )
    {
        do
        {
            uint32 locationid = CheckDuplicatePOI(pConnection, POI, POI_TYPE_FAVORITE);
            if (locationid == 0)
            {
                ret = SQLITE_NOTFOUND;
                break;
            }
            ret = UpdateFavoritePOIIndex(pConnection, locationid, index);
        } while ( 0 );
        m_dbmanager->ReleaseConnection(pConnection);
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::UpdateFavoritePOIIndex(DBConnection* pConnection, uint32 locationId, uint32 index)
{
    nsl_assert(locationId != 0);
	PAL_DB_Error ret = SQLITE_ERROR;
    if ( pConnection != NULL )
    {
        do
        {
            const char* sql = NULL;
            const char* sql2 = NULL;
            uint32 currentIndex = UINT32_MAX;
            {
                uint32 max_index = 0;
                {
                    DBCommand cmd = pConnection->CreateCommand( g_getFavoritePlacesMaxOrder);
                    LLM_ASSERT(cmd.IsValid());
                    ret = cmd.ExecuteInt( (int*)&max_index );
                    if (ret != SQLITE_OK)
                    {
                        max_index = 0;
                    }
                }

                ret = GetFavoriteOrderIndex(pConnection, locationId, currentIndex);
                if (ret == SQLITE_OK)
                {
                    if (index > max_index)
                    {
                        index = max_index;
                    }
                    LLM_ASSERT(currentIndex < UINT32_MAX);
                    if (currentIndex != index)
                    {
                        if (currentIndex < index)
                        {
                            sql = g_updateFavoritePlacesMoveOrderIncrease;
                            sql2 = g_updateFavoritePlacesSyncStatusMoveOrderIncrease;
                        }
                        else
                        {
                            sql = g_updateFavoritePlacesMoveOrderDecrease;
                            sql2 = g_updateFavoritePlacesSyncStatusMoveOrderDecrease;
                        }

                        // update sync status if sync status is SYNC_STATUS_NONE
                        {
                            DBCommand cmd = pConnection->CreateCommand( sql2 );
                            if ( !cmd.IsValid() )
                            {
                                break;
                            }
                            ret = cmd.BindIntParam("@ordernew", index);
                            CHECK_SQL_ERROR(ret);
                            ret = cmd.BindIntParam( "@orderold", currentIndex);
                            CHECK_SQL_ERROR(ret);
                            ret = cmd.ExecuteNonQuery();
                            CHECK_SQL_ERROR(ret);
                        }

                        DBCommand cmd = pConnection->CreateCommand( sql );
                        if ( !cmd.IsValid() )
                        {
                            break;
                        }
                        ret = cmd.BindIntParam("@ordernew", index);
                        CHECK_SQL_ERROR(ret);
                        ret = cmd.BindIntParam( "@orderold", currentIndex);
                        CHECK_SQL_ERROR(ret);
                        ret = cmd.ExecuteNonQuery();
                        CHECK_SQL_ERROR(ret);
                    }
                }
            }
            sql = g_updateFavoriteOrder;
			DBCommand cmd = pConnection->CreateCommand( sql );
            LLM_ASSERT(cmd.IsValid());
			ret = cmd.BindIntParam("@locationid", locationId);
            CHECK_SQL_ERROR(ret);
			ret = cmd.BindIntParam( "@favOrder", index);
            CHECK_SQL_ERROR(ret);
            ret = cmd.ExecuteNonQuery();
            CHECK_SQL_ERROR(ret);

            // update sync status if the status is SYNC_STATUS_NONE
            DMSyncStatusType syncStatus = SYNC_STATUS_NONE;
            GetPOISyncStatus(pConnection, locationId, POI_TYPE_FAVORITE, syncStatus);
            if(syncStatus == SYNC_STATUS_NONE)
            {
                sql = g_updateFavoriteSyncStatus;
                DBCommand cmd = pConnection->CreateCommand( sql );
                LLM_ASSERT(cmd.IsValid());
                ret = cmd.BindIntParam("@locationid", locationId);
                CHECK_SQL_ERROR(ret);
                ret = cmd.BindIntParam("@syncStatus", SYNC_STATUS_PLACE_MODIFIED);
                CHECK_SQL_ERROR(ret);
                ret = cmd.ExecuteNonQuery();
                CHECK_SQL_ERROR(ret);
            }

        } while ( 0 );
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::SetFavoriteOrder(nbsearch::POI const* POI, uint32 index)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if ( pConnection != NULL )
    {
        do
        {
            uint32 locationid = CheckDuplicatePOI(pConnection, POI, POI_TYPE_FAVORITE);
            if (locationid == 0)
            {
                ret = SQLITE_NOTFOUND;
                break;
            }
            ret = SetFavoriteOrder(pConnection, locationid, index);
        } while ( 0 );
        m_dbmanager->ReleaseConnection(pConnection);
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::SetFavoriteOrder(DBConnection* pConnection, uint32 locationId, uint32 index)
{
    nsl_assert(locationId != 0);
	PAL_DB_Error ret = SQLITE_ERROR;
    if ( pConnection != NULL )
    {
        do
        {
            const char* sql = g_updateFavoriteOrder;
			DBCommand cmd = pConnection->CreateCommand( sql );
            LLM_ASSERT(cmd.IsValid());
			ret = cmd.BindIntParam("@locationid", locationId);
            CHECK_SQL_ERROR(ret);
			ret = cmd.BindIntParam( "@favOrder", index);
            CHECK_SQL_ERROR(ret);
            ret = cmd.ExecuteNonQuery();
            CHECK_SQL_ERROR(ret);
        } while ( 0 );
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::ExtractAPOI(DBReader *pReader, nbsearch::POIImpl *&result, uint32 &internalId, DMSyncStatusType& syncStatus, uint32& timeStamp, int64& serverId, std::map<std::string, std::string>& userData)
{
    userData.clear();
    result = NULL;
    if ( pReader == NULL )
    {
        return SQLITE_ERROR;
    }
    PAL_DB_Error ret = pReader->Read();
    if ( ret != SQLITE_ROW )
    {
        return ret;
    }
    result = new(std::nothrow) nbsearch::POIImpl();
    if ( result == NULL )
    {
        return SQLITE_NOMEM;
    }
    do
    {
        result->m_place = new nbsearch::PlaceImpl();
        result->m_place->m_location = new nbsearch::Location();

        nbsearch::Location* pLocation = result->m_place->m_location;

        char* pStringResult = NULL;

        double doubleResult = 0.0;
        pLocation->m_point = nbsearch::LatLonPoint(0.0, 0.0);
        ret = pReader->GetDouble( "Latitude", &doubleResult );
        CHECK_SQL_ERROR(ret);
        pLocation->m_point.m_latitude = doubleResult;

        ret = pReader->GetDouble( "Longitude", &doubleResult );
        CHECK_SQL_ERROR(ret);
        pLocation->m_point.m_longitude = doubleResult;

        int intResult = 0;
        ret = pReader->GetInt( "LocationType", &intResult );
        CHECK_SQL_ERROR(ret);
        pLocation->m_type = ( nbsearch::LocationType )intResult;

        ret = pReader->GetString( "City", &pStringResult );
        CHECK_SQL_ERROR(ret);
        pLocation->m_city = pStringResult;
        nsl_free( pStringResult );

        ret = pReader->GetString( "Airport", &pStringResult );
        CHECK_SQL_ERROR(ret);
        pLocation->m_airport = pStringResult;
        nsl_free( pStringResult );

        ret = pReader->GetString( "Area", &pStringResult );
        CHECK_SQL_ERROR(ret);
        pLocation->m_name = pStringResult;
        nsl_free( pStringResult );

        ret = pReader->GetString( "Country", &pStringResult );
        CHECK_SQL_ERROR(ret);
        pLocation->m_country = pStringResult;
        nsl_free( pStringResult );

        ret = pReader->GetString( "State", &pStringResult );
        CHECK_SQL_ERROR(ret);
        pLocation->m_state = pStringResult;
        nsl_free( pStringResult );

        ret = pReader->GetString( "Street1", &pStringResult );
        CHECK_SQL_ERROR(ret);
        pLocation->m_street = pStringResult;
        nsl_free( pStringResult );

        ret = pReader->GetString( "Street2", &pStringResult );
        CHECK_SQL_ERROR(ret);
        pLocation->m_crossStreet = pStringResult;
        nsl_free( pStringResult );

        ret = pReader->GetString( "StreetNumber", &pStringResult );
        CHECK_SQL_ERROR(ret);
        pLocation->m_number = pStringResult;
        nsl_free( pStringResult );

        ret = pReader->GetString( "County", &pStringResult );
        CHECK_SQL_ERROR(ret);
        pLocation->m_county = pStringResult;
        nsl_free( pStringResult );

        ret = pReader->GetString( "Postal", &pStringResult );
        CHECK_SQL_ERROR(ret);
        pLocation->m_zipcode = pStringResult;
        nsl_free( pStringResult );

        ret = pReader->GetString( "Name", &pStringResult );
        CHECK_SQL_ERROR(ret);
        result->m_place->m_name = pStringResult;
        nsl_free( pStringResult );

        ret = pReader->GetString( "SearchFilter", &pStringResult );
        CHECK_SQL_ERROR(ret);
        nbsearch::SearchFilter* sf = new nbsearch::SearchFilter();
        sf->m_serialized = pStringResult;
        result->m_searchfilter = sf;
        nsl_free( pStringResult );

        ret = pReader->GetInt( "LocationId", &intResult );
        CHECK_SQL_ERROR(ret);
        internalId = intResult;

        ret = pReader->GetInt( "SyncStatus", &intResult );
        CHECK_SQL_ERROR(ret);
        syncStatus = (DMSyncStatusType)intResult;

        ret = pReader->GetInt( "Timestamp", &intResult );
        CHECK_SQL_ERROR(ret);
        timeStamp = intResult;

        int64 int64result = 0;
        ret = pReader->GetInt64("serverId", &int64result);
        CHECK_SQL_ERROR(ret);
        serverId = int64result;

        void* blobResult = NULL;
        int size = 0;
        ret = pReader->GetBlob("ExData", &blobResult, &size);
        CHECK_SQL_ERROR(ret);
        if (size > 0 && blobResult != NULL)
        {
            std::map<std::string, std::string> exData;
            Deserialize<std::map<std::string, std::string> >(exData, blobResult, size);
            nsl_free(blobResult);
            SetExDataToPOIImpl(exData, result);
            blobResult = NULL;
            size = 0;
        }

        ret = pReader->GetBlob("UserData", &blobResult, &size);
        CHECK_SQL_ERROR(ret);
        if (size > 0 && blobResult != NULL)
        {
            Deserialize<std::map<std::string, std::string> >(userData, blobResult, size);
            nsl_free(blobResult);
        }

    return SQLITE_OK;
    } while ( 0 );
    delete result;
    result = NULL;
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::ExtractACategory(DBReader *pReader, nbsearch::POIImpl *POI, uint32 internalId)
{
    if ( pReader == NULL )
    {
        return SQLITE_ERROR;
    }
    PAL_DB_Error ret = pReader->Read();
    if ( ret != SQLITE_ROW )
    {
        return ret;
    }
    int id = 0;
    ret = pReader->GetInt( "LocationId", &id );
    if ( ret != SQLITE_OK )
    {
        return ret;
    }
    if (id != internalId)
    {
        return SQLITE_OK;
    }
    nbsearch::PlaceImpl* place = POI->m_place;
    // assert pPlace is not NULL
    if (place == NULL)
    {
        return SQLITE_ERROR;
    }
//    if ( place->categories.size() >= NB_PLACE_NUM_CAT )
//    {
//        return SQLITE_ERROR;
//    }
    nbsearch::CategoryImpl* category = new nbsearch::CategoryImpl();

    // get data
    char* pStringResult = NULL;
    ret = pReader->GetString( "Code", &pStringResult );
    if ( ret != SQLITE_OK )
    {
        return ret;
    }
    category->m_code = pStringResult;
    nsl_free( pStringResult );

    ret = pReader->GetString( "Name", &pStringResult );
    if ( ret != SQLITE_OK )
    {
        return ret;
    }
    category->m_name = pStringResult;
    nsl_free( pStringResult );

    ret = pReader->GetString( "ParentCode", &pStringResult );
    if ( ret != SQLITE_OK )
    {
        return ret;
    }
    if (nsl_strlen(pStringResult) >0 )
    {
        category->m_parent = new nbsearch::CategoryImpl();
        category->m_parent->m_code = pStringResult;
        nsl_free( pStringResult );

        ret = pReader->GetString( "ParentName", &pStringResult );
        if ( ret != SQLITE_OK )
        {
            return ret;
        }
        category->m_parent->m_name = pStringResult;
        nsl_free( pStringResult );
    }
    else
    {
        nsl_free(pStringResult);
    }

    place->m_categories.push_back(category);
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::ExtractAPhoneNumber(DBReader *pReader, nbsearch::POIImpl *POI, uint32 internalId)
{
    if ( pReader == NULL )
    {
        return SQLITE_ERROR;
    }
    PAL_DB_Error ret = pReader->Read();
    if ( ret != SQLITE_ROW )
    {
        return ret;
    }
    int id = 0;
    ret = pReader->GetInt( "LocationId", &id );
    if ( ret != SQLITE_OK )
    {
        return ret;
    }
    if (id != internalId)
    {
        return SQLITE_OK;
    }

    nbsearch::PlaceImpl* place = POI->m_place;
    // assert pPlace is not NULL

    if ( place == NULL )
    {
        return SQLITE_ERROR;
    }

    // get data
    nbsearch::PhoneImpl* phone = new nbsearch::PhoneImpl();
    char* pStringResult = NULL;
    ret = pReader->GetString( "CountryCode", &pStringResult );
    if ( ret != SQLITE_OK )
    {
        return ret;
    }
    phone->m_country = pStringResult;
    nsl_free( pStringResult );

    ret = pReader->GetString( "AreaCode", &pStringResult );
    if ( ret != SQLITE_OK )
    {
        return ret;
    }
    phone->m_area = pStringResult;
    nsl_free( pStringResult );

    ret = pReader->GetString( "LocalNumber", &pStringResult );
    if ( ret != SQLITE_OK )
    {
        return ret;
    }
    phone->m_number = pStringResult;
    nsl_free( pStringResult );

    int intResult = 0;
    ret = pReader->GetInt( "PhoneNumberType", &intResult );
    if ( ret != SQLITE_OK )
    {
        return ret;
    }
    phone->m_type = ( nbsearch::PhoneType )intResult;

    place->m_phones.push_back(phone);
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::GetAllMatchedCategories(DBConnection *pConnection, DMPOIType type, nbsearch::POIImpl *POI, uint32 internalId)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    // init command
    const char* pSql = NULL;
    switch ( type )
    {
        case POI_TYPE_RECENT:
            pSql = g_getRecentPlacesCategories;
            break;
        case POI_TYPE_FAVORITE:
            pSql = g_getFavoritePlacesCategories;
            break;
        default:
            break;
    }
    if ( pSql == NULL )
    {
        return ret;
    }
    DBCommand cmd = pConnection->CreateCommand( pSql );
    if ( !cmd.IsValid() )
    {
        return ret;
    }
    // read data
    DBReader* pReader = cmd.ExecuteReader();
    ret = ExtractACategory(pReader, POI, internalId);
    while ( ret == SQLITE_OK )
    {
        ret = ExtractACategory(pReader, POI, internalId);
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::GetAllMatchedPhoneNumbers(DBConnection *pConnection, DMPOIType type, nbsearch::POIImpl *POI, uint32 internalId)
{
     PAL_DB_Error ret = SQLITE_ERROR;
    // init command
    const char* pSql = NULL;
    switch ( type )
    {
        case POI_TYPE_RECENT:
            pSql = g_getRecentPlacesPhoneNumbers;
            break;
        case POI_TYPE_FAVORITE:
            pSql = g_getFavoritePlacesPhoneNumbers;
            break;
        default:
            break;
    }
    if ( pSql == NULL )
    {
        return ret;
    }
    DBCommand cmd = pConnection->CreateCommand( pSql );
    if ( !cmd.IsValid() )
    {
        return ret;
    }
    // read data
    DBReader* pReader = cmd.ExecuteReader();
    ret = ExtractAPhoneNumber(pReader, POI, internalId);
    while ( ret == SQLITE_OK )
    {
        ret = ExtractAPhoneNumber(pReader, POI, internalId);
    }
    return ret;
}


/* See header file for description */
uint32
POIDBDAO::CheckDuplicatePOI(DBConnection *pConnection, nbsearch::POI const*pPOI, DMPOIType type)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    nbsearch::Place const* pPlace = pPOI->GetPlace();
    nsl_assert(pPlace != NULL);
    uint32 locationID = 0;
    // init command
    do
    {
        const char* pSql = NULL;
        switch ( type )
        {
            case POI_TYPE_RECENT:
                pSql = g_checkRecentPlace;
                break;
            case POI_TYPE_FAVORITE:
                pSql = g_checkFavoritePlace;
                break;
            default:
                break;
        }
        LLM_ASSERT(pSql != NULL);
        DBCommand cmd = pConnection->CreateCommand( pSql );
        LLM_ASSERT(cmd.IsValid());
        nbsearch::Location const* pLocation = pPlace->GetLocation();
        // assert pLocation is not NULL
        LLM_ASSERT(pLocation != NULL);
        ret = cmd.BindStringParam( "@name", pPlace->GetName().c_str());
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@area", pLocation->GetName().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@streetnumber", pLocation->GetNumber().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@street1", pLocation->GetStreet().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@street2", pLocation->GetCrossStreet().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@city", pLocation->GetCity().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@state", pLocation->GetState().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@county", pLocation->GetCounty().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@postal", pLocation->GetZipCode().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@country", pLocation->GetCountry().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@airport", pLocation->GetAirport().c_str() );
        CHECK_SQL_ERROR(ret);
//        ret = cmd.BindDoubleParam( "@latitude", pLocation->GetCoordinates()->GetLatitude());
//        CHECK_SQL_ERROR(ret);
//        ret = cmd.BindDoubleParam( "@longitude", pLocation->GetCoordinates()->GetLongitude());
//        CHECK_SQL_ERROR(ret);
        ret = cmd.ExecuteInt( (int*)&locationID );
        LLM_ASSERT(ret == SQLITE_OK || ret == SQLITE_DONE);
    }while (0);
    return locationID;
}

uint32
POIDBDAO::CheckDuplicatePOI(DBConnection *pConnection, int64 serverid, DMPOIType type)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    uint32 locationID = 0;
    // init command
    do
    {
        const char* pSql = NULL;
        switch ( type )
        {
            case POI_TYPE_RECENT:
                pSql = g_checkRecentServerId;
                break;
            case POI_TYPE_FAVORITE:
                pSql = g_checkFavoriteServerId;
                break;
            default:
                break;
        }
        LLM_ASSERT(pSql != NULL);
        DBCommand cmd = pConnection->CreateCommand( pSql );
        LLM_ASSERT(cmd.IsValid());
        ret = cmd.BindInt64Param( "@serverid", serverid);
        CHECK_SQL_ERROR(ret);
        ret = cmd.ExecuteInt( (int*)&locationID );
        LLM_ASSERT(ret == SQLITE_OK || ret == SQLITE_DONE);
    }while (0);
    return locationID;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::SetAllPOIParam(DBCommand &cmd, nbsearch::POI const* pPOI, DMSyncStatusType syncStatus, uint32 timeStamp, const std::map<std::string, std::string>* userData)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    nbsearch::Place const* pPlace = pPOI->GetPlace();
    nbsearch::Location const* pLocation = pPlace->GetLocation();
    // assert pLocation is not NULL
    do
    {
        LLM_ASSERT(pLocation != NULL);
        ret = cmd.BindStringParam( "@area", pLocation->GetName().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@streetnumber", pLocation->GetNumber().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@street1", pLocation->GetStreet().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@street2", pLocation->GetCrossStreet().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@city", pLocation->GetCity().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@state", pLocation->GetState().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@county", pLocation->GetCounty().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@postal", pLocation->GetZipCode().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@country", pLocation->GetCountry().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@airport", pLocation->GetAirport().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindDoubleParam( "@latitude", pLocation->GetCoordinates().GetLatitude() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindDoubleParam( "@longitude", pLocation->GetCoordinates().GetLongitude() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindIntParam( "@locationtype", pLocation->GetType() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@name", pPlace->GetName().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@sf", pPOI->GetSearchFilter() != NULL ? pPOI->GetSearchFilter()->GetSerialized().c_str() :"" );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindIntParam( "@ts", timeStamp);
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindIntParam( "@syncstatus", syncStatus );
        CHECK_SQL_ERROR(ret);

        void* exdata = NULL;
        int exsize = 0;
        std::map<std::string, std::string> exDataMap = CreateExDataFromPOI(pPOI);
        Serialize<std::map<std::string, std::string> >(exDataMap, exdata, exsize);
        LLM_ASSERT(exdata != NULL && exsize != 0);
        ret = cmd.BindBlobParam("@exdata", exdata, exsize);
        CHECK_SQL_ERROR(ret);
        nsl_free(exdata);

        if (userData != NULL)
        {
            void* data = NULL;
            int size = 0;
            Serialize<std::map<std::string, std::string> >(*userData, data, size);
            LLM_ASSERT(data != NULL && size != 0);
            ret = cmd.BindBlobParam("@userdata", data, size);
            CHECK_SQL_ERROR(ret);
            nsl_free(data);
        }
    }while(0);
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::SetAllPhoneParam(DBCommand &cmd, uint32 locationID, const nbsearch::Phone *pPhone)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    do
    {
        LLM_ASSERT(pPhone);
        ret = cmd.BindIntParam( "@locationid", locationID );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindIntParam( "@phonenumbertype", pPhone->GetPhoneType() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@areacode", pPhone->GetArea().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@countrycode", pPhone->GetCountry().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@localnumber", pPhone->GetNumber().c_str() );
        CHECK_SQL_ERROR(ret);
        ret = cmd.BindStringParam( "@formattednumber", pPhone->GetFormattedNumber().c_str() );
        CHECK_SQL_ERROR(ret);
    } while (0);
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::SetAllCategoryParam(DBCommand &cmd, uint32 internalID, const nbsearch::Category *pCategory)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    if ( pCategory == NULL )
    {
        return ret;
    }
    ret = cmd.BindIntParam( "@locationid", internalID );
    if ( ret != SQLITE_OK )
    {
        return ret;
    }
    ret = cmd.BindStringParam( "@name", pCategory->GetName().c_str() );
    if ( ret != SQLITE_OK )
    {
        return ret;
    }
    ret = cmd.BindStringParam( "@code", pCategory->GetCode().c_str() );
    if ( ret != SQLITE_OK )
    {
        return ret;
    }
    std::string parentCode = "";
    std::string parentName = "";
    if (pCategory->GetParentCategory() != NULL)
    {
        parentCode = pCategory->GetParentCategory()->GetCode();
        parentName = pCategory->GetParentCategory()->GetName();
    }
    ret = cmd.BindStringParam( "@parentname", parentName.c_str() );
    if ( ret != SQLITE_OK )
    {
        return ret;
    }
    ret = cmd.BindStringParam( "@parentcode", parentCode.c_str() );

    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::UpdateAPOI(DBConnection *pConnection, uint32 locationId, nbsearch::POI const*pPOI, uint32 timeStamp,  DMPOIType type, DMSyncStatusType syncStatus, const std::map<std::string, std::string>* userData)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    // start transaction
    do
    {
        DBCommand cmd = pConnection->CreateCommand( g_startTransaction );
        LLM_ASSERT(cmd.IsValid());
        ret = cmd.ExecuteNonQuery();
        CHECK_SQL_ERROR(ret);
    }while (0);
    if ( ret != SQLITE_OK )
    {
        return ret;
    }
    do
    {
        // update location infomation
        {
            ret = SQLITE_ERROR;
            const char* pSql = NULL;
            switch ( type )
            {
                case POI_TYPE_RECENT:
                    if (userData == NULL)
                    {
                        pSql = g_updateRecentPlaceLocation;
                    }
                    else
                    {
                        pSql = g_updateRecentPlaceLocationWithUD;
                    }
                    break;
                case POI_TYPE_FAVORITE:
                    if (userData == NULL)
                    {
                        pSql = g_updateFavoritePlaceLocation;
                    }
                    else
                    {
                        pSql = g_updateFavoritePlaceLocationWithUD;
                    }
                    break;
                default:
                    break;
            }
            if ( pSql == NULL )
            {
                break;
            }
            DBCommand cmd = pConnection->CreateCommand( pSql );
            LLM_ASSERT(cmd.IsValid());
            ret = SetAllPOIParam( cmd, pPOI, syncStatus, timeStamp, userData);
            CHECK_SQL_ERROR(ret);
            ret = cmd.BindIntParam( "@locationid", locationId );
            CHECK_SQL_ERROR(ret);
            ret = cmd.ExecuteNonQuery();
            CHECK_SQL_ERROR(ret);
        }

        // update favorite place's PhoneNumber list, recent place need not update
        if ( type == POI_TYPE_FAVORITE )
        {
            // update PhoneNumbers
            DBCommand cmd = pConnection->CreateCommand(g_deleteFavoritePlacePhoneNumber);
            ret = cmd.BindIntParam( "@locationid", locationId );
            CHECK_SQL_ERROR(ret);
            ret = cmd.ExecuteNonQuery();
            CHECK_SQL_ERROR(ret);
            InsertPOIPhoneNumbers(pConnection, locationId, pPOI, POI_TYPE_FAVORITE);
        }

    } while ( 0 );

    if ( ret == SQLITE_OK )
    {
        // commit transaction
        DBCommand cmd = pConnection->CreateCommand( g_commitTransaction );
        nsl_assert(cmd.IsValid());
        if ( !cmd.IsValid() )
        {
            ret = SQLITE_ERROR;
        }
        else
        {
            ret = cmd.ExecuteNonQuery();
        }
    }
    else
    {
        // rollback transaction
        DBCommand cmd = pConnection->CreateCommand( g_rollbackTransaction );
        nsl_assert(cmd.IsValid());
        ret = cmd.ExecuteNonQuery();
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::InsertPOICategories(DBConnection *pConnection, uint32 locationId, nbsearch::POI const*pPOI, DMPOIType type)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    const char* pSql = NULL;
    switch ( type )
    {
        case POI_TYPE_RECENT:
            pSql = g_insertRecentPlaceCategory;
            break;
        case POI_TYPE_FAVORITE:
            pSql = g_insertFavoritePlaceCategory;
            break;
        default:
            break;
    }
    if ( pSql == NULL )
    {
        return ret;
    }
    int count = pPOI->GetPlace()->GetCategories().size();
    if ( count == 0 )
    {
        return SQLITE_OK;
    }
    for ( int i = 0; i < count; ++i )
    {
        nbsearch::Category const* pCategory = pPOI->GetPlace()->GetCategories().at(i);
        if ( pCategory != NULL )
        {
            DBCommand cmd = pConnection->CreateCommand( pSql );
            if ( !cmd.IsValid() )
            {
                ret = SQLITE_ERROR;
                break;
            }
            ret = SetAllCategoryParam( cmd, locationId, pCategory );
            if ( ret != SQLITE_OK )
            {
                break;
            }
            ret = cmd.ExecuteNonQuery();
            if ( ret != SQLITE_OK )
            {
                break;
            }
        }
        else
        {
            ret = SQLITE_ERROR;
            break;
        }
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::InsertPOIPhoneNumbers(DBConnection *pConnection,uint32 locationId, nbsearch::POI const*pPOI, DMPOIType type)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    const char* pSql = NULL;
    switch ( type )
    {
        case POI_TYPE_RECENT:
            pSql = g_insertRecentPlacePhoneNumber;
            break;
        case POI_TYPE_FAVORITE:
            pSql = g_insertFavoritePlacePhoneNumber;
            break;
        default:
            break;
    }
    do
    {
        LLM_ASSERT(pSql);
        int count = pPOI->GetPlace()->GetPhoneNumbers().size();
        if ( count == 0 )
        {
            return SQLITE_OK;
        }
        for ( int i = 0; i < count; ++i )
        {
            nbsearch::Phone const* pPhone = pPOI->GetPlace()->GetPhoneNumbers().at(i);
            if ( pPhone != NULL )
            {
                DBCommand cmd = pConnection->CreateCommand( pSql );
                LLM_ASSERT(cmd.IsValid());
                if ( !cmd.IsValid() )
                {
                    ret = SQLITE_ERROR;
                    break;
                }
                ret = SetAllPhoneParam( cmd, locationId, pPhone );
                CHECK_SQL_ERROR(ret);
                ret = cmd.ExecuteNonQuery();
                CHECK_SQL_ERROR(ret);
            }
            else
            {
                ret = SQLITE_ERROR;
                break;
            }
        }
    } while (0);
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::ExecuteOneLineCommand(DBConnection *pConnetion, const char *command)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    DBCommand cmd = pConnetion->CreateCommand( command );
    if ( !cmd.IsValid() )
    {
        return ret;
    }

    ret = cmd.ExecuteNonQuery();
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::GetFavoriteOrderIndex(DBConnection *pConnection, uint32 internalId, uint32 &index)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    if( pConnection != NULL)
    {
        do
        {
            DBCommand cmd = pConnection->CreateCommand( g_getFavoritePlacesOrder );
            LLM_ASSERT(cmd.IsValid());
            ret = cmd.BindIntParam( "@locationid", internalId );
            CHECK_SQL_ERROR(ret);
            char* result = NULL;
            ret = cmd.ExecuteString(&result);
            CHECK_SQL_ERROR(ret);
            if (nsl_strlen(result) == 0)
            {
                ret = SQLITE_DONE;
            }
            else
            {
                ret = cmd.ExecuteInt((int*)&index);
                CHECK_SQL_ERROR(ret);
            }
            nsl_free(result);
        } while( 0 );
    }
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::GetStringUserData(const char* key, std::string& data)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    if (key == NULL)
    {
        return ret;
    }

    int _size = 0;
    void *_result = NULL;
    ret = GetBinaryUserData(key, &_result, _size);
    if (ret != SQLITE_OK)
    {
        return ret;
    }

    if (_result == NULL)
    {
        return SQLITE_ERROR;
    }

    if (_result != NULL)
    {
        char* temp = new char[_size+1];
        memset(temp, 0, _size+1);
        memcpy(temp, _result, _size);
        data = temp;

        nsl_free(_result);
        _result = NULL;
        delete [] temp;
    }

    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::SetStringUserData(const char* key, const char* value)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    if (key == NULL || value == NULL)
    {
        return ret;
    }

    ret = SetBinaryUserData(key, (void*)value, strlen(value));
    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::GetBinaryUserData(const char* key, void **result, int& size)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    if (key == NULL || result == NULL)
    {
        return ret;
    }

    DBConnection* pConnection = m_dbmanager->GetConnection(DBT_POI);;
    if (pConnection == NULL)
    {
        return ret;
    }

    do
    {
        DBCommand cmd = pConnection->CreateCommand(g_getUserData);
        if (!cmd.IsValid())
        {
            break;
        }

        ret = cmd.BindStringParam("@key", key);
        if (ret != SQLITE_OK)
        {
            break;
        }

        ret = cmd.ExecuteBlob(result, &size);

    } while(0);

    m_dbmanager->ReleaseConnection(pConnection);

    return ret;
}

/* See header file for description */
PAL_DB_Error
POIDBDAO::SetBinaryUserData(const char* key, void* data, uint32 size)
{
    PAL_DB_Error ret = SQLITE_ERROR;
    if (key == NULL ||data == NULL)
    {
        return ret;
    }

    DBConnection* pConnection = m_dbmanager->GetConnection(DBT_POI);;
    if (pConnection == NULL)
    {
        return ret;
    }

    do
    {
        DBCommand cmd = pConnection->CreateCommand(g_insertUserData);
        if (!cmd.IsValid())
        {
            break;
        }

        ret = cmd.BindStringParam("@key", key);
        if (ret != SQLITE_OK)
        {
            break;
        }

        ret = cmd.BindBlobParam("@data", data, size);
        if (ret != SQLITE_OK)
        {
            break;
        }

        ret = cmd.ExecuteNonQuery();
    } while(0);

    m_dbmanager->ReleaseConnection(pConnection);

    return ret;
}

PAL_DB_Error
POIDBDAO::RemoveAllInUserData()
{
    PAL_DB_Error ret = SQLITE_ERROR;

    DBConnection* pConnection = m_dbmanager->GetConnection(DBT_POI);;
    if (pConnection == NULL)
    {
        return ret;
    }

    ret = pConnection->Excute(g_resetUserData);

    m_dbmanager->ReleaseConnection(pConnection);

    return ret;
}

PAL_DB_Error
POIDBDAO::UpdateFavoriteName(nbsearch::POI const* POI, DMPOIType type, std::string& name)
{
    PAL_DB_Error ret = SQLITE_ERROR;

    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if ( pConnection != NULL )
    {
        do
        {
            LLM_ASSERT(type == POI_TYPE_FAVORITE);
            uint32 locationid = CheckDuplicatePOI(pConnection, POI, type);
            if (locationid == 0)
            {
                ret = SQLITE_NOTFOUND;
                break;
            }

            //check if new place name already taken by other record
            uint32 checkLocation = 0;
            {
                // init command
                const char* pSql = g_checkFavoritePlace;
                DBCommand cmd = pConnection->CreateCommand( pSql );
                LLM_ASSERT(cmd.IsValid());
                nbsearch::Location const* pLocation = POI->GetPlace()->GetLocation();
                ret = cmd.BindStringParam( "@name", name.c_str());
                CHECK_SQL_ERROR(ret);
                ret = cmd.BindStringParam( "@area", pLocation->GetName().c_str() );
                CHECK_SQL_ERROR(ret);
                ret = cmd.BindStringParam( "@streetnumber", pLocation->GetNumber().c_str() );
                CHECK_SQL_ERROR(ret);
                ret = cmd.BindStringParam( "@street1", pLocation->GetStreet().c_str() );
                CHECK_SQL_ERROR(ret);
                ret = cmd.BindStringParam( "@street2", pLocation->GetCrossStreet().c_str() );
                CHECK_SQL_ERROR(ret);
                ret = cmd.BindStringParam( "@city", pLocation->GetCity().c_str() );
                CHECK_SQL_ERROR(ret);
                ret = cmd.BindStringParam( "@state", pLocation->GetState().c_str() );
                CHECK_SQL_ERROR(ret);
                ret = cmd.BindStringParam( "@county", pLocation->GetCounty().c_str() );
                CHECK_SQL_ERROR(ret);
                ret = cmd.BindStringParam( "@postal", pLocation->GetZipCode().c_str() );
                CHECK_SQL_ERROR(ret);
                ret = cmd.BindStringParam( "@country", pLocation->GetCountry().c_str() );
                CHECK_SQL_ERROR(ret);
                ret = cmd.BindStringParam( "@airport", pLocation->GetAirport().c_str() );
                CHECK_SQL_ERROR(ret);
//                ret = cmd.BindDoubleParam( "@latitude", pLocation->GetCoordinates()->GetLatitude());
//                CHECK_SQL_ERROR(ret);
//                ret = cmd.BindDoubleParam( "@longitude", pLocation->GetCoordinates()->GetLongitude());
//                CHECK_SQL_ERROR(ret);
                ret = cmd.ExecuteInt( (int*)&checkLocation );
                LLM_ASSERT(ret == SQLITE_OK || ret == SQLITE_DONE);
            }
            if (checkLocation != 0)
            {
                ret = SQLITE_DONE + 1;
                break;
            }

            DMSyncStatusType syncStatus = SYNC_STATUS_NONE;
            GetPOISyncStatus(pConnection, locationid, type, syncStatus);

            // if current sync status is ADDED then user must be editing non synched place. In this case we do not
            // have to change status of place as modified, as it will cause server to throw error 4017
            // for modified something which never existed on server.
            if(syncStatus != SYNC_STATUS_ADDED)
            {
                syncStatus = SYNC_STATUS_PLACE_MODIFIED;
            }

            const char* sql = g_pdateFavoriteName;

            DBCommand cmd = pConnection->CreateCommand( sql );
            LLM_ASSERT(cmd.IsValid());
            ret = cmd.BindInt64Param("@locationid", locationid);
            CHECK_SQL_ERROR(ret);
            ret = cmd.BindStringParam("@name", name.c_str());
            CHECK_SQL_ERROR(ret);
            ret = cmd.BindIntParam( "@syncstatus", syncStatus );
            CHECK_SQL_ERROR(ret);
            ret = cmd.ExecuteNonQuery();

        } while ( 0 );
        m_dbmanager->ReleaseConnection(pConnection);
    }
    return ret;
}

PAL_DB_Error
POIDBDAO::CheckPOIExist(nbsearch::POI const* POI, DMPOIType type)
{
    PAL_DB_Error ret = SQLITE_OK;

    DBConnection* pConnection = m_dbmanager->GetConnection( DBT_POI );
    if ( pConnection != NULL )
    {
        do
        {
            uint32 locationid = CheckDuplicatePOI(pConnection, POI, type);
            if (locationid == 0)
            {
                ret = SQLITE_NOTFOUND;
                break;
            }

        } while ( 0 );
        m_dbmanager->ReleaseConnection(pConnection);
    }
    return ret;
}

/*! @} */
