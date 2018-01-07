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
    @file     recents.cpp
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "recents.h"
#include "palstdlib.h"
#include "paldberror.h"
#include "palclock.h"
#include "myplacesutils.h"

#define RECENTS_DATABASE_NAME "recentsdb.db3"

static const char* const g_createRecentsTable  = "CREATE TABLE IF NOT EXISTS Recents ('placeId' INTEGER PRIMARY KEY NOT NULL, 'name' BLOB, 'type' INTEGER, 'url' BLOB, 'imageUrl' BLOB, 'lastUpdateTime' BIGINT);";
static const char* const g_createLocationTable = "CREATE TABLE IF NOT EXISTS RecentsLocation ('placeId' INTEGER, 'areaname' BLOB, 'streetnum' BLOB, 'street1' BLOB, 'street2' BLOB, 'city' BLOB, 'county' BLOB, 'state' BLOB, 'postal' BLOB, 'country' BLOB, 'airport' BLOB, 'freeform' BLOB, 'latitude' REAL, 'longitude' REAL, 'lastUpdateTime' BIGINT);";
static const char* const g_createCategoryTable = "CREATE TABLE IF NOT EXISTS RecentsCategory ('placeId' INTEGER, 'code' BLOB, 'categoryName' BLOB, 'lastUpdateTime' BIGINT);";
static const char* const g_createPhoneTable    = "CREATE TABLE IF NOT EXISTS RecentsPhone ('placeId' INTEGER, 'phoneType' INTEGER, 'phoneCountry' BLOB, 'area' BLOB, 'number' BLOB, 'lastUpdateTime' BIGINT);";

static const char* const g_checkRecents        = "SELECT placeId FROM (SELECT * FROM Recents a, RecentsLocation b, RecentsPhone c WHERE a.name = @name AND a.type = @type AND b.latitude = @lat AND b.longitude = @lon AND c.number = @num AND a.placeId = b.placeId AND b.placeId = c.placeId);";
static const char* const g_countRecents        = "SELECT COUNT(*) FROM (SELECT * FROM Recents a, RecentsLocation b WHERE a.name = @name AND b.latitude = @lat AND b.longitude = @lon AND a.placeId = b.placeId);";

static const char* const g_insertToRecents     = "INSERT INTO Recents (name, type, url, imageUrl, lastUpdateTime) VALUES (@name, @type, @url, @imageUrl, @timestamp);";
static const char* const g_insertToLocation    = "INSERT INTO RecentsLocation (placeId, areaname, streetnum, street1, street2, city, county, state, postal, country, airport, freeform, latitude, longitude, lastUpdateTime) VALUES (@id, @areaName, @streetNum, @street1, @street2, @city, @county, @state, @postal, @country, @airport, @freeform, @latitude, @longitude, @timestamp);";
static const char* const g_insertToCategory    = "INSERT INTO RecentsCategory (placeId, code, categoryName, lastUpdateTime) VALUES (@id, @categoryCode, @categoryName, @timestamp);";
static const char* const g_insertToPhone       = "INSERT INTO RecentsPhone (placeId, phoneType, phoneCountry, area, number, lastUpdateTime) VALUES (@id, @phoneType, @phoneCountry, @phoneArea, @phoneNumber, @timestamp);";
static const char* const g_updateRecents       = "UPDATE Recents SET lastUpdateTime = @timestamp, url = @url, imageUrl = @imageUrl WHERE placeId = @id;";
static const char* const g_updateLocation      = "UPDATE RecentsLocation SET lastUpdateTime = @timestamp, areaname = @areaName, streetnum = @streetNum, street1 = @street1, street2 = @street2, city = @city, county = @county, state = @state, postal = @postal, country = @country, airport = @airport, freeform = @freeform, latitude = @latitude, longitude = @longitude WHERE placeId = @id;";
static const char* const g_updateCategory      = "UPDATE RecentsCategory SET lastUpdateTime = @timestamp, code = @categoryCode, categoryName = @categoryName WHERE placeId = @id;";
static const char* const g_updatePhone         = "UPDATE RecentsPhone SET lastUpdateTime = @timestamp, phoneType = @phoneType, phoneCountry = @phoneCountry, area = @phoneArea, number = @phoneNumber WHERE placeId = @id;";

static const char* const g_getPlaceId          = "SELECT placeId FROM Recents WHERE name = @name AND type = @type AND url = @url AND imageUrl = @imageUrl AND lastUpdateTime = @timestamp;";

static const char* const g_clearRecents        = "DELETE FROM Recents;";
static const char* const g_clearLocation       = "DELETE FROM RecentsLocation;";
static const char* const g_clearCategory       = "DELETE FROM RecentsCategory;";
static const char* const g_clearPhone          = "DELETE FROM RecentsPhone;";

Recents::Recents(PAL_Instance* pal)
{
    m_pal = pal;
}

Recents::~Recents()
{

}

PAL_Error Recents::Initialize()
{
    PAL_DB_Error result = PAL_DB_OK;

    result = PAL_DBOpen(m_pal, PAL_DB_Sqlite, RECENTS_DATABASE_NAME, &m_connection);
    if (m_connection && result == PAL_DB_OK)
    {
            result = PAL_DBExecuteNonQuery(m_connection, g_createRecentsTable);
            result = result ? result : PAL_DBExecuteNonQuery(m_connection, g_createLocationTable);
            result = result ? result : PAL_DBExecuteNonQuery(m_connection, g_createCategoryTable);
            result = result ? result : PAL_DBExecuteNonQuery(m_connection, g_createPhoneTable);
    }

    return PalDbErrorToPalError(result);
}

PAL_Error
Recents::RecentsGet(char* pFilter, PAL_MyPlace** ppMyPlacesArray, int* pMyPlaceCount, PAL_SortType sortType, double latitude, double longitude)
{
    PAL_DB_Error ret       = PAL_DB_OK;
    PAL_DB_Error tmpError  = PAL_DB_OK;
    PAL_MyPlace* pMyPlaces = NULL;
    PAL_MyPlace* pStore    = NULL;
    int count              = 0;

    if (!pFilter || !pMyPlaceCount || !ppMyPlacesArray)
    {
        return PAL_ErrBadParam;
    }

    PAL_DBCommand* cmdCount = NULL;
    ret = PAL_DBInitCommand(m_connection, g_countRecents, &cmdCount);
    ret = ret ? ret : PAL_DBBindStringParam(cmdCount, "@name", pFilter);
    ret = ret ? ret : PAL_DBBindDoubleParam(cmdCount, "@lat", latitude);
    ret = ret ? ret : PAL_DBBindDoubleParam(cmdCount, "@lon", longitude);
    ret = ret ? ret : PAL_DBExecuteInt(cmdCount, &count);
    ret = (ret == PAL_DB_DONE || ret == PAL_DB_ROW) ? PAL_DB_OK : ret;
    tmpError = PAL_DBCommandFinalize(cmdCount, NULL);
    ret = ret ? ret : tmpError;

    if (!count)
    {
        return PAL_Failed;
    }

    pMyPlaces = static_cast<PAL_MyPlace*>(nsl_malloc(sizeof(PAL_MyPlace)*count));
    if (!pMyPlaces)
    {
        return PAL_ErrNoMem;
    }
    pStore = pMyPlaces;

    ret = ret ? ret : GetPlace(m_connection, pFilter, pStore, RECENTS_DATABASE_NAME, latitude, longitude, 0);

    *ppMyPlacesArray = pMyPlaces;
    *pMyPlaceCount   = count;

    return PalDbErrorToPalError(ret);
}

PAL_Error
Recents::RecentsAdd(PAL_MyPlace* pMyPlace)
{
    PAL_DB_Error ret      = PAL_DB_OK;
    PAL_DB_Error tmpError = PAL_DB_OK;
    int placeId      = 0;

    if (!pMyPlace)
    {
        return PAL_ErrBadParam;
    }

    // Check if the recent is already existing.
    // If item with given name, type, lat, lon and phone number already in our database,
    // then we will just update other fields, we won't duplicate this item in the DB.
    PAL_DBCommand* cmdCheck = NULL;
    ret = PAL_DBInitCommand(m_connection, g_checkRecents, &cmdCheck);
    ret = ret ? ret : PAL_DBBindStringParam(cmdCheck, "@name", pMyPlace->name);
    ret = ret ? ret : PAL_DBBindIntParam(cmdCheck, "@type", pMyPlace->type);
    ret = ret ? ret : PAL_DBBindDoubleParam(cmdCheck, "@lat", pMyPlace->location.latitude);
    ret = ret ? ret : PAL_DBBindDoubleParam(cmdCheck, "@lon", pMyPlace->location.longitude);
    ret = ret ? ret : PAL_DBBindStringParam(cmdCheck, "@num", pMyPlace->phone.number);
    ret = ret ? ret : PAL_DBExecuteInt(cmdCheck, &placeId);
    ret = (ret == PAL_DB_DONE || ret == PAL_DB_ROW) ? PAL_DB_OK : ret;
    tmpError = PAL_DBCommandFinalize(cmdCheck, NULL);
    ret = ret ? ret : tmpError;

    // Update the recents if it is existing or add the recent.
    const char* pSqlLocation  = NULL;
    const char* pSqlCategory  = NULL;
    const char* pSqlPhone     = NULL;

    nb_unixTime currentTime = PAL_ClockGetUnixTime();

    if (placeId == 0)
    {
        pSqlLocation  = g_insertToLocation;
        pSqlCategory  = g_insertToCategory;
        pSqlPhone     = g_insertToPhone;

        PAL_DBCommand* cmdRecents = NULL;
        ret = ret ? ret : PAL_DBInitCommand(m_connection, g_insertToRecents, &cmdRecents);
        ret = ret ? ret : PAL_DBBindStringParam(cmdRecents, "@name", pMyPlace->name);
        ret = ret ? ret : PAL_DBBindIntParam(cmdRecents, "@type", pMyPlace->type);
        ret = ret ? ret : PAL_DBBindStringParam(cmdRecents, "@url", pMyPlace->url);
        ret = ret ? ret : PAL_DBBindStringParam(cmdRecents, "@imageUrl", pMyPlace->imageUrl);
        ret = ret ? ret : PAL_DBBindIntParam(cmdRecents, "@timestamp", currentTime);
        ret = ret ? ret : PAL_DBExecuteNonQueryCommand(cmdRecents);
        ret = (ret == PAL_DB_DONE || ret == PAL_DB_ROW) ? PAL_DB_OK : ret;
        tmpError = PAL_DBCommandFinalize(cmdRecents, NULL);
        ret = ret ? ret : tmpError;

        // Get placeId from Recents table.
        PAL_DBCommand* cmdGetId = NULL;
        ret = ret ? ret : PAL_DBInitCommand(m_connection, g_getPlaceId, &cmdGetId);
        ret = ret ? ret : PAL_DBBindStringParam(cmdGetId, "@name", pMyPlace->name);
        ret = ret ? ret : PAL_DBBindIntParam(cmdGetId, "@type", pMyPlace->type);
        ret = ret ? ret : PAL_DBBindStringParam(cmdGetId, "@url", pMyPlace->url);
        ret = ret ? ret : PAL_DBBindStringParam(cmdGetId, "@imageUrl", pMyPlace->imageUrl);
        ret = ret ? ret : PAL_DBBindIntParam(cmdGetId, "@timestamp", currentTime);
        ret = ret ? ret : PAL_DBExecuteInt(cmdGetId, &placeId);
        ret = (ret == PAL_DB_DONE || ret == PAL_DB_ROW) ? PAL_DB_OK : ret;
        tmpError = PAL_DBCommandFinalize(cmdGetId, NULL);
        ret = ret ? ret : tmpError;
    }
    else
    {
        pSqlLocation  = g_updateLocation;
        pSqlCategory  = g_updateCategory;
        pSqlPhone     = g_updatePhone;

        PAL_DBCommand* cmdUpdate = NULL;
        ret = ret ? ret : PAL_DBInitCommand(m_connection, g_updateRecents, &cmdUpdate);
        ret = ret ? ret : PAL_DBBindStringParam(cmdUpdate, "@url", pMyPlace->url);
        ret = ret ? ret : PAL_DBBindStringParam(cmdUpdate, "@imageUrl", pMyPlace->imageUrl);
        ret = ret ? ret : PAL_DBBindIntParam(cmdUpdate, "@timestamp", currentTime);
        ret = ret ? ret : PAL_DBBindIntParam(cmdUpdate, "@id", placeId);
        ret = ret ? ret : PAL_DBExecuteNonQueryCommand(cmdUpdate);
        ret = (ret == PAL_DB_DONE || ret == PAL_DB_ROW) ? PAL_DB_OK : ret;
        tmpError = PAL_DBCommandFinalize(cmdUpdate, NULL);
        ret = ret ? ret : tmpError;
    }

    // Insert into or update RecentsLocation table
    PAL_DBCommand* cmdLocation = NULL;
    ret = ret ? ret : PAL_DBInitCommand(m_connection, pSqlLocation, &cmdLocation);
    ret = ret ? ret : PAL_DBBindIntParam(cmdLocation, "@id", placeId);
    ret = ret ? ret : PAL_DBBindStringParam(cmdLocation, "@areaName", pMyPlace->location.areaname);
    ret = ret ? ret : PAL_DBBindStringParam(cmdLocation, "@streetNum", pMyPlace->location.streetnum);
    ret = ret ? ret : PAL_DBBindStringParam(cmdLocation, "@street1", pMyPlace->location.street1);
    ret = ret ? ret : PAL_DBBindStringParam(cmdLocation, "@street2", pMyPlace->location.street2);
    ret = ret ? ret : PAL_DBBindStringParam(cmdLocation, "@city", pMyPlace->location.city);
    ret = ret ? ret : PAL_DBBindStringParam(cmdLocation, "@county", pMyPlace->location.county);
    ret = ret ? ret : PAL_DBBindStringParam(cmdLocation, "@state", pMyPlace->location.state);
    ret = ret ? ret : PAL_DBBindStringParam(cmdLocation, "@postal", pMyPlace->location.postal);
    ret = ret ? ret : PAL_DBBindStringParam(cmdLocation, "@country", pMyPlace->location.country);
    ret = ret ? ret : PAL_DBBindStringParam(cmdLocation, "@airport", pMyPlace->location.airport);
    ret = ret ? ret : PAL_DBBindStringParam(cmdLocation, "@freeform", pMyPlace->location.freeform);
    ret = ret ? ret : PAL_DBBindDoubleParam(cmdLocation, "@latitude", pMyPlace->location.latitude);
    ret = ret ? ret : PAL_DBBindDoubleParam(cmdLocation, "@longitude", pMyPlace->location.longitude);
    ret = ret ? ret : PAL_DBBindIntParam(cmdLocation, "@timestamp", currentTime);
    ret = ret ? ret : PAL_DBExecuteNonQueryCommand(cmdLocation);
    ret = (ret == PAL_DB_DONE || ret == PAL_DB_ROW) ? PAL_DB_OK : ret;
    tmpError = PAL_DBCommandFinalize(cmdLocation, NULL);
    ret = ret ? ret : tmpError;

    // Insert into or update RecentsCategory table
    PAL_DBCommand* cmdCategory = NULL;
    ret = ret ? ret : PAL_DBInitCommand(m_connection, pSqlCategory, &cmdCategory);
    ret = ret ? ret : PAL_DBBindIntParam(cmdCategory, "@id", placeId);
    ret = ret ? ret : PAL_DBBindStringParam(cmdCategory, "@categoryCode", pMyPlace->category.code);
    ret = ret ? ret : PAL_DBBindStringParam(cmdCategory, "@categoryName", pMyPlace->category.name);
    ret = ret ? ret : PAL_DBBindIntParam(cmdCategory, "@timestamp", currentTime);
    ret = ret ? ret : PAL_DBExecuteNonQueryCommand(cmdCategory);
    ret = (ret == PAL_DB_DONE || ret == PAL_DB_ROW) ? PAL_DB_OK : ret;
    tmpError = PAL_DBCommandFinalize(cmdCategory, NULL);
    ret = ret ? ret : tmpError;

    // Insert into or update RecentsPhone table
    PAL_DBCommand* cmdPhone = NULL;
    ret = ret ? ret : PAL_DBInitCommand(m_connection, pSqlPhone, &cmdPhone);
    ret = ret ? ret : PAL_DBBindIntParam(cmdPhone, "@id", placeId);
    ret = ret ? ret : PAL_DBBindIntParam(cmdPhone, "@phoneType", pMyPlace->phone.type);
    ret = ret ? ret : PAL_DBBindStringParam(cmdPhone, "@phoneCountry", pMyPlace->phone.country);
    ret = ret ? ret : PAL_DBBindStringParam(cmdPhone, "@phoneArea", pMyPlace->phone.area);
    ret = ret ? ret : PAL_DBBindStringParam(cmdPhone, "@phoneNumber", pMyPlace->phone.number);
    ret = ret ? ret : PAL_DBBindIntParam(cmdPhone, "@timestamp", currentTime);
    ret = ret ? ret : PAL_DBExecuteNonQueryCommand(cmdPhone);
    ret = (ret == PAL_DB_DONE || ret == PAL_DB_ROW) ? PAL_DB_OK : ret;
    tmpError = PAL_DBCommandFinalize(cmdPhone, NULL);
    ret = ret ? ret : tmpError;

    return PalDbErrorToPalError(ret);
}

PAL_Error
Recents::RecentsGetPlaceById(uint32 id, PAL_MyPlace* myPlace)
{
    PAL_DB_Error ret = PAL_DB_OK;

    PAL_MyPlace result = {0};

    ret = GetPlace(m_connection, NULL, &result, RECENTS_DATABASE_NAME, 0, 0, id);

    *myPlace = result;

    return PalDbErrorToPalError(ret);
}

PAL_Error
Recents::RecentsClear()
{
    PAL_DB_Error result = PAL_DB_ERROR;

    result = PAL_DBExecuteNonQuery(m_connection, g_clearRecents);
    result = result ? result : PAL_DBExecuteNonQuery(m_connection, g_clearLocation);
    result = result ? result : PAL_DBExecuteNonQuery(m_connection, g_clearCategory);
    result = result ? result : PAL_DBExecuteNonQuery(m_connection, g_clearPhone);

    return PalDbErrorToPalError(result);
}

void
Recents::RecentsDestroy()
{
    if (m_connection)
    {
        PAL_DBClose(m_connection);
    }
}

/*! @} */