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

 @file suggestlistdbdao.cpp
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

#include "paldberror.h"
#include "palclock.h"
#include "dbmanager.h"
#include "dbdaofactory.h"
#include "suggestlistdbdao.h"

static const char* g_beginTransaction = "BEGIN TRANSACTION;";
static const char* g_commitTransaction = "COMMIT TRANSACTION;";
static const char* g_rollbackTransaction = "ROLLBACK TRANSACTION;";

static const char* g_createSuggestListTable = "CREATE TABLE IF NOT EXISTS 'SuggestList' ('Id' INTEGER PRIMARY KEY NOT NULL, 'Data' TEXT, 'TimeStamp' INTEGER);";
static const char* g_getAllSuggestList = "SELECT * FROM SuggestList ORDER BY Id ASC;";
static const char* g_addSuggestList = "INSERT INTO SuggestList (Data, TimeStamp) VALUES (@Data, @ts);";
static const char* g_removeAllSuggestList = "DELETE FROM SuggestList;";

SuggestListDBDAO::SuggestListDBDAO(const shared_ptr<DBManager>& manager): m_dbmanager(manager)
{
}

SuggestListDBDAO::~SuggestListDBDAO()
{
}

PAL_DB_Error SuggestListDBDAO::Initialize()
{
    PAL_DB_Error result = PAL_DB_ERROR;

    // Get a connection from the pool.
    DBConnection* connection = m_dbmanager->GetConnection(DBT_SuggestList);
    if (connection)
    {
        do
        {
            DBCommand command = connection->CreateCommand(g_createSuggestListTable);
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

PAL_DB_Error SuggestListDBDAO::UpdateAll(const std::vector<SuggestListData>& allData)
{
    if (allData.empty())
    {
        return PAL_DB_OK;
    }

    PAL_DB_Error result = PAL_DB_ERROR;

    // Get a connection from the pool.
    DBConnection* connection = m_dbmanager->GetConnection(DBT_SuggestList);
    if (connection)
    {
        do
        {
            // Begin transaction.
            {
                DBCommand command = connection->CreateCommand(g_beginTransaction);
                if (!command.IsValid())
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
                {
                    DBCommand command = connection->CreateCommand(g_removeAllSuggestList);
                    if (!command.IsValid())
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

                for (std::size_t i = 0; i < allData.size(); ++i)
                {
                    const SuggestListData& data = allData[i];

                    DBCommand command = connection->CreateCommand(g_addSuggestList);
                    if (!(command.IsValid()))
                    {
                        result = PAL_DB_ERROR;
                        break;
                    }

                    result = command.BindStringParam("@Data", data.m_text.c_str());
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

            } while (0);

            if (result == PAL_DB_OK)
            {
                // Commit transaction.
                DBCommand command = connection->CreateCommand(g_commitTransaction);
                if (!command.IsValid())
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
                if (!command.IsValid())
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

PAL_DB_Error SuggestListDBDAO::RemoveAll()
{
    PAL_DB_Error result = PAL_DB_ERROR;

    // Get a connection from the pool.
    DBConnection* connection = m_dbmanager->GetConnection(DBT_SuggestList);
    if (connection)
    {
        do
        {
            // Begin transaction.
            {
                DBCommand command = connection->CreateCommand(g_beginTransaction);
                if (!command.IsValid())
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

            // Transaction in progress.
            do
            {
                DBCommand command = connection->CreateCommand(g_removeAllSuggestList);
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

            // End transaction.
            if (result == PAL_DB_OK)
            {
                DBCommand command = connection->CreateCommand(g_commitTransaction);
                if (!command.IsValid())
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
                DBCommand command = connection->CreateCommand(g_rollbackTransaction);
                if (!command.IsValid())
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

        } while (0);

        // Put connection to the pool.
        m_dbmanager->ReleaseConnection(connection);
        connection = NULL;
    }
    
    return result;
}

PAL_DB_Error SuggestListDBDAO::GetAll(std::vector<SuggestListData>& allData)
{
    PAL_DB_Error result = PAL_DB_ERROR;
    std::vector<SuggestListData> tempSuggestList;

    // Get a connection from the pool.
    DBConnection* connection = m_dbmanager->GetConnection(DBT_SuggestList);
    if (connection)
    {
        do
        {
            DBCommand command = connection->CreateCommand(g_getAllSuggestList);
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

                char* data = NULL;
                result = reader->GetString("Data", &data);
                if (result != PAL_DB_OK)
                {
                    break;
                }
                nsl_assert(data);
                std::string textData(data);
                nsl_free(data);

                uint32 timestamp = 0;
                result = reader->GetInt("TimeStamp", (int*)&timestamp);
                if (result != PAL_DB_OK)
                {
                    break;
                }

                SuggestListData suggestListData;
                suggestListData.m_id = id;
                suggestListData.m_text = textData;
                suggestListData.m_timestamp = timestamp;

                tempSuggestList.push_back(suggestListData);

                result = reader->Read();
            }

            result = (result == PAL_DB_DONE) ? PAL_DB_OK : result;

            allData = tempSuggestList;

        } while (0);
        
        // Put connection to the pool.
        m_dbmanager->ReleaseConnection(connection);
        connection = NULL;
    }
    
    return result;
}

/*! @} */
