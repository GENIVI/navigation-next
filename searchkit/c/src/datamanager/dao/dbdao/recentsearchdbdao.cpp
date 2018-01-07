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

    @file recentsearchdbdao.cpp
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

#include "recentsearchdbdao.h"
#include "paldberror.h"
#include "palclock.h"
#include "dbmanager.h"
#include "dbdaofactory.h"
#include "SearchHistoryManager.h"
#include "offboard/SingleSearchOffboardInformation.h"

#define RECENTSEARCH_COUNT_LIMIT  100
// Transaction
static const char* g_beginTransaction = "BEGIN TRANSACTION;";
static const char* g_commitTransaction = "COMMIT TRANSACTION;";
static const char* g_rollbackTransaction = "ROLLBACK TRANSACTION;";

static const char* g_createRecentSearchTable = "CREATE TABLE IF NOT EXISTS 'RecentSearch' ('Id' INTEGER PRIMARY KEY NOT NULL, 'Type' INTEGER, 'Data' BLOB, 'TimeStamp' INTEGER);";
static const char* g_getAllRecentSearches = "SELECT * FROM RecentSearch ORDER BY Id DESC;";
static const char* g_getAllRecentSearchesCount = "SELECT COUNT(*) FROM RecentSearch;";
static const char* g_addRecentSearch = "INSERT INTO RecentSearch (Type, Data, TimeStamp) VALUES (@Type, @Data, @ts);";
static const char* g_removeRecentSearch = "DELETE FROM RecentSearch WHERE Id=@Id;";
static const char* g_removeRecentSearchMinId = "DELETE FROM RecentSearch WHERE Id= (SELECT MIN(Id) FROM RecentSearch);";
static const char* g_getLastInsertedRowID = "SELECT last_insert_rowid();";
static const char* g_removeAllRecentSearches = "DELETE FROM RecentSearch;";

static const char* k_SuggestionMatchSearchFilter = "SuggestionMatchSearchFilter";
static const char* k_SuggestionMatchLine1 = "SuggestionMatchLine1";
static const char* k_SuggestionMatchLine2 = "SuggestionMatchLine2";
static const char* k_SuggestionMatchLine3 = "SuggestionMatchLine3";
static const char* k_SuggestionMatchIconIdGroup = "SuggestionMatchIconID_%d";
static const char* k_SuggestionMatchType = "SuggestionMatchType";

inline std::map<std::string, std::string> CreateDataFromSuggestionMatch(const nbsearch::SuggestionMatch* suggestion)
{
    std::map<std::string, std::string> map;
    if (suggestion->GetSearchFilter() != NULL)
    {
        map[k_SuggestionMatchSearchFilter] = suggestion->GetSearchFilter()->GetSerialized();
    }
    map[k_SuggestionMatchLine1] = suggestion->GetLine1();
    map[k_SuggestionMatchLine2] = suggestion->GetLine2();
    map[k_SuggestionMatchLine3] = suggestion->GetLine3();
    char buffer[10] = "";
    nsl_sprintf(buffer, "%d", suggestion->GetMatchType());
    map[k_SuggestionMatchType] = buffer;

    for (int i = 0; i < (int)suggestion->GetIconIDs().size(); i++)
    {
        char key[50] = {0};
        nsl_sprintf(key, k_SuggestionMatchIconIdGroup, i);
        map[key] = suggestion->GetIconIDs().at(i);
    }
    return map;
}

inline void SetDataToSuggestionMatchImpl(const std::map<std::string, std::string>& exdata, nbsearch::SuggestionMatchImpl* suggestion)
{
    std::map<std::string, std::string>::const_iterator iter;
    //setting compact address
    iter = exdata.find(k_SuggestionMatchSearchFilter);
    if (iter != exdata.end())
    {
        nbsearch::SearchFilter*& sf = suggestion->m_searchFilter;
        if (sf == NULL)
        {
            sf = new nbsearch::SearchFilter();
        }
        sf->m_serialized = iter->second;
    }
    iter = exdata.find(k_SuggestionMatchLine1);
    if (iter != exdata.end())
    {
        suggestion->m_line1 = iter->second;
    }
    iter = exdata.find(k_SuggestionMatchLine2);
    if (iter != exdata.end())
    {
        suggestion->m_line2 = iter->second;
    }
    iter = exdata.find(k_SuggestionMatchLine3);
    if (iter != exdata.end())
    {
        suggestion->m_line3 = iter->second;
    }
    iter = exdata.find(k_SuggestionMatchType);
    if (iter != exdata.end())
    {
        suggestion->m_type = (nbsearch::SuggestionMatchType)nsl_atoi(iter->second.c_str());
    }
    int i = 0;
    while (1)
    {
        char key[50] = {0};
        nsl_sprintf(key, k_SuggestionMatchIconIdGroup, i++);
        iter = exdata.find(key);
        if (iter == exdata.end())
        {
            break;
        }
        suggestion->m_iconIDs.push_back(iter->second);
    }
}

inline void RemoveInternalKeyItems(std::map<std::string, std::string>& exdata)
{
    std::map<std::string, std::string>::iterator iter;
    //setting compact address
    iter = exdata.find(k_SuggestionMatchSearchFilter);
    if (iter != exdata.end())
    {
        exdata.erase(iter);
    }
    iter = exdata.find(k_SuggestionMatchLine1);
    if (iter != exdata.end())
    {
        exdata.erase(iter);
    }
    iter = exdata.find(k_SuggestionMatchLine2);
    if (iter != exdata.end())
    {
        exdata.erase(iter);
    }
    iter = exdata.find(k_SuggestionMatchLine3);
    if (iter != exdata.end())
    {
        exdata.erase(iter);
    }
    iter = exdata.find(k_SuggestionMatchType);
    if (iter != exdata.end())
    {
        exdata.erase(iter);
    }
    int i = 0;
    while (1)
    {
        char key[50] = {0};
        nsl_sprintf(key, k_SuggestionMatchIconIdGroup, i++);
        iter = exdata.find(key);
        if (iter == exdata.end())
        {
            break;
        }
        exdata.erase(iter);
    }
}

// Public functions ......................................................

RecentSearchDBDAO::RecentSearchDBDAO(shared_ptr<DBManager> manager)
{
    m_dbmanager = manager;
}

RecentSearchDBDAO::~RecentSearchDBDAO()
{
}

/* See description in "recentsearchdao.h" */
PAL_DB_Error
RecentSearchDBDAO::Initialize()
{
    PAL_DB_Error result = PAL_DB_ERROR;

    // Get a connection from the pool.
    DBConnection* connection = m_dbmanager->GetConnection(DBT_SearchHistory);
    if (connection)
    {
        do
        {
            DBCommand command = connection->CreateCommand(g_createRecentSearchTable);
            if (!(command.IsValid()))
            {
                result = PAL_DB_ERROR;
                break;
            }

            result = command.ExecuteNonQuery();
            if (result != PAL_DB_OK)
            {
                break;
            }
        } while (0);

        // Put connection to the pool.
        m_dbmanager->ReleaseConnection(connection);
        connection = NULL;
    }

    return result;
}

/* See description in "recentsearchdao.h" */
PAL_DB_Error
RecentSearchDBDAO::GetAllRecentSearches(std::vector<RecentSearchData *> &recentSearches)
{
    PAL_DB_Error result = PAL_DB_ERROR;
    std::vector<RecentSearchData*> tempRecentSearches;

    // Get a connection from the pool.
    DBConnection* connection = m_dbmanager->GetConnection(DBT_SearchHistory);
    if (connection)
    {
        do
        {
            DBCommand command = connection->CreateCommand(g_getAllRecentSearches);
            if (!(command.IsValid()))
            {
                result = PAL_DB_ERROR;
                break;
            }

            // Read the data from database.
            DBReader* reader = command.ExecuteReader();
            if (!reader)
            {
                result = PAL_DB_ERROR;
                break;
            }

            // Read each recent search.
            result = reader->Read();
            while (result == PAL_DB_ROW)
            {
                uint32 id = 0;
                result = reader->GetInt("Id", (int*)&id);
                if (result != PAL_DB_OK)
                {
                    break;
                }

                uint32 type = 0;
                result = reader->GetInt("Type", (int*)&type);
                if (result != PAL_DB_OK)
                {
                    break;
                }

                void* data = NULL;
                int datasize = 0;
                result = reader->GetBlob("Data", &data, &datasize);
                if (result != PAL_DB_OK)
                {
                    break;
                }

                RecentSearchData* recentSearch = new(std::nothrow) RecentSearchData();
                recentSearch->m_dataType = (RecentSearchDataType)type;
                recentSearch->m_id = id;
                switch (recentSearch->m_dataType)
                {
                    case RSDT_KeyWordText:
                    {
                        recentSearch->m_keyWord = (char*)data;
                    }
                        break;
                    case RSDT_SuggestionData:
                    {
                        nbsearch::SuggestionMatchImpl* suggestion = new nbsearch::SuggestionMatchImpl();
                        std::map<std::string, std::string> map;
                        Deserialize<std::map<std::string, std::string> >(map, data, datasize);
                        SetDataToSuggestionMatchImpl(map, suggestion);
                        RemoveInternalKeyItems(map);
                        recentSearch->m_map = map;
                        recentSearch->m_suggestion = suggestion;
                    }
                        break;
                    case RSDT_KeyValueMap:
                    {
                        Deserialize<std::map<std::string, std::string> >(recentSearch->m_map, data, datasize);
                    }
                    default:
                        break;
                }
                nsl_free(data);
                if (!recentSearch)
                {
                    result = PAL_DB_ERROR;
                    break;
                }
                tempRecentSearches.push_back(recentSearch);

                result = reader->Read();
            }

            result = (result == PAL_DB_DONE) ? PAL_DB_OK : result;

        } while (0);

        // Put connection to the pool.
        m_dbmanager->ReleaseConnection(connection);
        connection = NULL;
    }

    if (result == PAL_DB_OK)
    {
        recentSearches.insert(recentSearches.end(),
                              tempRecentSearches.begin(),
                              tempRecentSearches.end());
    }
    else
    {
        std::vector<RecentSearchData*>::const_iterator iter = tempRecentSearches.begin();
        std::vector<RecentSearchData*>::const_iterator end = tempRecentSearches.end();
        for (; iter != end; ++iter)
        {
            RecentSearchData* recentSearch = *iter;

            if (!recentSearch)
            {
                delete (recentSearch);
            }
        }
    }
    return result;
}

/* See description in "recentsearchdao.h" */
PAL_DB_Error
RecentSearchDBDAO::AddRecentSearch(RecentSearchData *recentSearch)
{
    PAL_DB_Error result = PAL_DB_ERROR;

    // Get a connection from the pool.
    DBConnection* connection = m_dbmanager->GetConnection(DBT_SearchHistory);
    if (connection)
    {
        do
        {
            // Begin transaction.
            {
                DBCommand command = connection->CreateCommand(g_beginTransaction);
                if (!(command.IsValid()))
                {
                    result = PAL_DB_ERROR;
                    break;
                }
                result = command.ExecuteNonQuery();
                if (result != PAL_DB_OK)
                {
                    break;
                }
            }

            do
            {
                // Add the recent search.
                {
                    DBCommand command = connection->CreateCommand(g_addRecentSearch);
                    if (!(command.IsValid()))
                    {
                        result = PAL_DB_ERROR;
                        break;
                    }

                    result = command.BindIntParam("@Type", recentSearch->m_dataType);
                    if (result != PAL_DB_OK)
                    {
                        break;
                    }

                    void* data = NULL;
                    int dataSize = 0;
                    switch (recentSearch->m_dataType)
                    {
                        case RSDT_KeyWordText:
                        {
                            dataSize = nsl_strlen(recentSearch->m_keyWord.c_str()) + 1;
                            data = nsl_malloc(dataSize);
                            nsl_memcpy(data, recentSearch->m_keyWord.c_str(), dataSize);
                        }
                            break;
                        case RSDT_SuggestionData:
                        {
                            std::map<std::string, std::string> map = CreateDataFromSuggestionMatch(recentSearch->m_suggestion);
                            for (std::map<std::string, std::string>::iterator iter = recentSearch->m_map.begin();
                                 iter != recentSearch->m_map.end(); iter++)
                            {
                                map[iter->first] = iter->second;
                            }
                            Serialize<std::map<std::string, std::string> >(map, data, dataSize);
                        }
                            break;
                        case RSDT_KeyValueMap:
                        {
                            Serialize(recentSearch->m_map, data, dataSize);
                        }
                            break;
                        default:
                            break;
                    }
                    result = command.BindBlobParam("@Data", data, dataSize);
                    nsl_free(data);
                    if (result != PAL_DB_OK)
                    {
                        break;
                    }

                    result = command.BindIntParam("@ts", PAL_ClockGetUnixTime());
                    if (result != PAL_DB_OK)
                    {
                        break;
                    }

                    result = command.ExecuteNonQuery();
                    if (result != PAL_DB_OK)
                    {
                        break;
                    }
                }

                /*  Get the ID of last added recent search and set this ID to the parameter
                 'DMRecentSearch' object.
                 */
                {
                    DBCommand command = connection->CreateCommand(g_getLastInsertedRowID);
                    if (!(command.IsValid()))
                    {
                        result = PAL_DB_ERROR;
                        break;
                    }

                    int id = 0;
                    result = command.ExecuteInt(&id);
                    if (result != PAL_DB_OK)
                    {
                        break;
                    }
                    recentSearch->m_id = id;
                }
            } while (0);

            {
                DBCommand command = connection->CreateCommand(g_getAllRecentSearchesCount);
                if (!(command.IsValid()))
                {
                    result = PAL_DB_ERROR;
                    break;
                }
                int count = 0;
                result = command.ExecuteInt(&count);
                if (result != PAL_DB_OK)
                {
                    break;
                }
                if (count > RECENTSEARCH_COUNT_LIMIT)
                {
                    DBCommand command2 = connection->CreateCommand(g_removeRecentSearchMinId);
                    if (!(command2.IsValid()))
                    {
                        result = PAL_DB_ERROR;
                        break;
                    }
                    result = command2.ExecuteNonQuery();
                }
            }

            if (result == PAL_DB_OK)
            {
                // Commit transaction.
                DBCommand command = connection->CreateCommand(g_commitTransaction);
                if (!(command.IsValid()))
                {
                    result = PAL_DB_ERROR;
                    break;
                }

                result = command.ExecuteNonQuery();
                if (result != PAL_DB_OK)
                {
                    break;
                }
            }
            else
            {
                // Try to rollback transaction. And do not set the 'result' we want to return.
                DBCommand command = connection->CreateCommand(g_rollbackTransaction);
                if (!(command.IsValid()))
                {
                    break;
                }

                command.ExecuteNonQuery();
            }
        } while (0);

        // Put connection to the pool.
        m_dbmanager->ReleaseConnection(connection);
        connection = NULL;
    }

    return result;
}

/* See description in "recentsearchdao.h" */
PAL_DB_Error
RecentSearchDBDAO::RemoveRecentSearch(int recentSearchId)
{
    PAL_DB_Error result = PAL_DB_ERROR;

    // Get a connection from the pool.
    DBConnection* connection = m_dbmanager->GetConnection(DBT_SearchHistory);
    if (connection)
    {
        do
        {
            DBCommand command = connection->CreateCommand(g_removeRecentSearch);
            if (!(command.IsValid()))
            {
                result = PAL_DB_ERROR;
                break;
            }

            // Bind parameter 'Id' of SQL.
            result = command.BindIntParam("@Id", recentSearchId);
            if (result != PAL_DB_OK)
            {
                break;
            }

            result = command.ExecuteNonQuery();
            if (result != PAL_DB_OK)
            {
                break;
            }
        } while (0);

        // Put connection to the pool.
        m_dbmanager->ReleaseConnection(connection);
        connection = NULL;
    }

    return result;
}

/* See description in "recentsearchdao.h" */
PAL_DB_Error
RecentSearchDBDAO::RemoveAllRecentSearches()
{
    PAL_DB_Error result = PAL_DB_ERROR;

    // Get a connection from the pool.
    DBConnection* connection = m_dbmanager->GetConnection(DBT_SearchHistory);
    if (connection)
    {
        do
        {
            DBCommand command = connection->CreateCommand(g_removeAllRecentSearches);
            if (!(command.IsValid()))
            {
                result = PAL_DB_ERROR;
                break;
            }

            result = command.ExecuteNonQuery();
            if (result != PAL_DB_OK)
            {
                break;
            }
        } while (0);

        // Put connection to the pool.
        m_dbmanager->ReleaseConnection(connection);
        connection = NULL;
    }

    return result;
}

/*! @} */
