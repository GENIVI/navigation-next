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

 @file     myplacesutils.cpp
 */
/*
 (C) Copyright 2012 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

#include "myplacesutils.h"

static const char* const g_getRecentsFromDataBase   = "SELECT * FROM Recents a, RecentsLocation b, RecentsCategory c, RecentsPhone d WHERE a.placeId = b.placeId AND b.placeId = c.placeId AND c.placeId = d.placeId AND a.name = @name AND b.latitude = @lat AND b.longitude = @lon ORDER BY lastUpdateTime DESC;";
static const char* const g_getRecentById            = "SELECT * FROM Recents a, RecentsLocation b, RecentsCategory c, RecentsPhone d WHERE a.placeId = @placeId AND b.placeId = @placeId AND c.placeId = @placeId AND d.placeId = @placeId;";

static const char* const g_getFavoritesFromDataBase = "SELECT * FROM Favorites a, FavoritesLocation b, FavoritesCategory c, FavoritesPhone d WHERE a.placeId = b.placeId AND b.placeId = c.placeId AND c.placeId = d.placeId AND a.name = @name AND b.latitude = @lat AND b.longitude = @lon ORDER BY lastUpdateTime DESC;";
static const char* const g_getFavoriteById          = "SELECT * FROM Favorites a, FavoritesLocation b, FavoritesCategory c, FavoritesPhone d WHERE a.placeId = @placeId AND b.placeId = @placeId AND c.placeId = @placeId AND d.placeId = @placeId;";

PAL_Error PalDbErrorToPalError(PAL_DB_Error err)
{
    switch ( err )
    {
    case PAL_DB_OK:
    case PAL_DB_ROW:
    case PAL_DB_DONE:
      return PAL_Ok;
    case PAL_DB_ENOMEM:
      return PAL_ErrNoMem;
    default:
      return PAL_Failed;
    };
}

PAL_DB_Error GetPlace(PAL_DBConnect* connection, char* pFilter, PAL_MyPlace* pMyPlacesArray,
                      char* database, double latitude, double longitude, int id)
{
    PAL_DB_Error ret      = PAL_DB_OK;
    PAL_DB_Error tmpError = PAL_DB_OK;
    PAL_DBCommand* cmdGet = NULL;
    PAL_DBReader* pReader = NULL;

    const char* pDatabase = static_cast<const char*>(database);

    if (!pMyPlacesArray || ((strcmp(pDatabase,"recentsdb.db3") != 0) && (strcmp(pDatabase, "favoritesdb.db3") != 0)))
    {
        return PAL_DB_ERROR;
    }

    if (pFilter)
    {
        // If filter is specified, then user searches for place by keyword.
        if (strcmp(pDatabase, "recentsdb.db3") == 0)
        {
            ret = PAL_DBInitCommand(connection, g_getRecentsFromDataBase, &cmdGet);
        }
        else
        {
            ret = PAL_DBInitCommand(connection, g_getFavoritesFromDataBase, &cmdGet);
        }
        ret = ret ? ret : PAL_DBBindStringParam(cmdGet, "@name", pFilter);
        ret = ret ? ret : PAL_DBBindDoubleParam(cmdGet, "@lat", latitude);
        ret = ret ? ret : PAL_DBBindDoubleParam(cmdGet, "@lon", longitude);
    }
    else if (id)
    {
        // If id is specified, then we need to search for place by id.
        if (strcmp(pDatabase, "recentsdb.db3") == 0)
        {
            ret = PAL_DBInitCommand(connection, g_getRecentById, &cmdGet);
        }
        else
        {
            ret = PAL_DBInitCommand(connection, g_getFavoriteById, &cmdGet);
        }
        ret = ret ? ret : PAL_DBBindIntParam(cmdGet, "@placeId", id);
    }
    else
    {
        return PAL_DB_ERROR;
    }

    ret = ret ? ret : PAL_DBExecuteReader(cmdGet, &pReader);
    ret = ret ? ret : PAL_DBReaderRead(pReader);

    while (ret == PAL_DB_ROW)
    {
        int placeId         = 0;
        int placeType       = 0;
        int phoneType       = 0;
        char* name          = NULL;
        char* placeUrl      = NULL;
        char* placeImageUrl = NULL;
        char* areaname      = NULL;
        char* streetnum     = NULL;
        char* street1       = NULL;
        char* street2       = NULL;
        char* city          = NULL;
        char* county        = NULL;
        char* state         = NULL;
        char* postal        = NULL;
        char* country       = NULL;
        char* airport       = NULL;
        char* freeform      = NULL;
        char* code          = NULL;
        char* categoryName  = NULL;
        char* phoneCountry  = NULL;
        char* area          = NULL;
        char* number        = NULL;
        double lat          = 0.0;
        double lon          = 0.0;
        PAL_MyPlace myPlace = {0};

        ret = PAL_DBReaderGetInt(pReader, "placeId", &placeId);
        ret = ret ? ret : PAL_DBReaderGetInt(pReader, "type", &placeType);
        ret = ret ? ret : PAL_DBReaderGetInt(pReader, "phoneType", &phoneType);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "name", &name);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "url", &placeUrl);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "imageUrl", &placeImageUrl);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "areaname", &areaname);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "streetnum", &streetnum);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "street1", &street1);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "street2", &street2);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "city", &city);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "county", &county);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "state", &state);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "postal", &postal);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "country", &country);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "airport", &airport);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "freeform", &freeform);
        ret = ret ? ret : PAL_DBReaderGetdouble(pReader, "latitude", &lat);
        ret = ret ? ret : PAL_DBReaderGetdouble(pReader, "longitude", &lon);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "code", &code);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "categoryName", &categoryName);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "phoneCountry", &phoneCountry);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "area", &area);
        ret = ret ? ret : PAL_DBReaderGetstring(pReader, "number", &number);
        if (id)
        {
            tmpError = PAL_DBCommandFinalize(cmdGet, pReader);
            ret = ret ? ret : tmpError;
        }
        if (ret != PAL_DB_OK)
        {
            goto clean;
        }

        if (name)
            nsl_strlcpy(myPlace.name, name, sizeof(myPlace.name));
        if (placeUrl)
            nsl_strlcpy(myPlace.url, placeUrl, sizeof(myPlace.url));
        if (placeImageUrl)
            nsl_strlcpy(myPlace.imageUrl, placeImageUrl, sizeof(myPlace.imageUrl));
        if (areaname)
            nsl_strlcpy(myPlace.location.areaname, areaname, sizeof(myPlace.location.areaname));
        if (streetnum)
            nsl_strlcpy(myPlace.location.streetnum, streetnum, sizeof(myPlace.location.streetnum));
        if (street1)
            nsl_strlcpy(myPlace.location.street1, street1, sizeof(myPlace.location.street1));
        if (street2)
            nsl_strlcpy(myPlace.location.street2, street2, sizeof(myPlace.location.street2));
        if (city)
            nsl_strlcpy(myPlace.location.city, city, sizeof(myPlace.location.city));
        if (county)
            nsl_strlcpy(myPlace.location.county, county, sizeof(myPlace.location.county));
        if (state)
            nsl_strlcpy(myPlace.location.state, state, sizeof(myPlace.location.state));
        if (postal)
            nsl_strlcpy(myPlace.location.postal, postal, sizeof(myPlace.location.postal));
        if (country)
            nsl_strlcpy(myPlace.location.country, country, sizeof(myPlace.location.country));
        if (airport)
            nsl_strlcpy(myPlace.location.airport, airport, sizeof(myPlace.location.airport));
        if (freeform)
            nsl_strlcpy(myPlace.location.freeform, freeform, sizeof(myPlace.location.freeform));
        if (code)
            nsl_strlcpy(myPlace.category.code, code, sizeof(myPlace.category.code));
        if (categoryName)
            nsl_strlcpy(myPlace.category.name, categoryName, sizeof(myPlace.category.name));
        if (phoneCountry)
            nsl_strlcpy(myPlace.phone.country, phoneCountry, sizeof(myPlace.phone.country));
        if (area)
            nsl_strlcpy(myPlace.phone.area, area, sizeof(myPlace.phone.area));
        if (number)
            nsl_strlcpy(myPlace.phone.number, number, sizeof(myPlace.phone.number));
        myPlace.location.latitude = lat;
        myPlace.location.longitude = lon;
        myPlace.myPlaceID = placeId;
        myPlace.type = static_cast<PAL_MyPlacesType>(placeType);
        myPlace.phone.type = static_cast<PAL_MyPlacesPhoneType>(phoneType);

        if (pFilter)
        {
            nsl_memcpy(pMyPlacesArray, &myPlace, sizeof(PAL_MyPlace));
            pMyPlacesArray++;
        }
        else
        {
            *pMyPlacesArray = myPlace;
        }

    clean:
        nsl_free(name);
        nsl_free(placeUrl);
        nsl_free(placeImageUrl);
        nsl_free(areaname);
        nsl_free(streetnum);
        nsl_free(street1);
        nsl_free(street2);
        nsl_free(city);
        nsl_free(county);
        nsl_free(state);
        nsl_free(postal);
        nsl_free(country);
        nsl_free(airport);
        nsl_free(freeform);
        nsl_free(code);
        nsl_free(categoryName);
        nsl_free(phoneCountry);
        nsl_free(area);
        nsl_free(number);

        if (pFilter)
        {
            ret = ret ? ret : PAL_DBReaderRead(pReader);
        }
    } // while (ret == PAL_DB_ROW)

    ret = (ret == PAL_DB_DONE) ? PAL_DB_OK : ret;

    if (pFilter)
    {
        tmpError = PAL_DBCommandFinalize(cmdGet, pReader);
        ret = ret ? ret : tmpError;
    }

    return ret;
}

/*! @} */