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

 @file shortcutdbdao.cpp
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
#include "shortcutdbdao.h"

static const char* g_beginTransaction = "BEGIN TRANSACTION;";
static const char* g_commitTransaction = "COMMIT TRANSACTION;";
static const char* g_rollbackTransaction = "ROLLBACK TRANSACTION;";

static const char* g_createShortcutTable = "CREATE TABLE IF NOT EXISTS 'Shortcut' ('Id' INTEGER PRIMARY KEY NOT NULL, 'Data' TEXT, 'TimeStamp' INTEGER);";
static const char* g_getAllShortcuts = "SELECT * FROM Shortcut ORDER BY Id ASC;";
static const char* g_addShortcut = "INSERT INTO Shortcut (Data, TimeStamp) VALUES (@Data, @ts);";
static const char* g_removeAllShortcuts = "DELETE FROM Shortcut;";

ShortcutDBDAO::ShortcutDBDAO(const shared_ptr<DBManager>& manager): m_dbmanager(manager)
{
}

ShortcutDBDAO::~ShortcutDBDAO()
{
}

PAL_DB_Error ShortcutDBDAO::Initialize()
{
    PAL_DB_Error result = PAL_DB_ERROR;

    // Get a connection from the pool.
    DBConnection* connection = m_dbmanager->GetConnection(DBT_Shortcut);
    if (connection)
    {
        do
        {
            DBCommand command = connection->CreateCommand(g_createShortcutTable);
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

PAL_DB_Error ShortcutDBDAO::UpdateAll(const std::vector<ShortcutData>& allData)
{
    if (allData.empty())
    {
        return PAL_DB_OK;
    }

    PAL_DB_Error result = PAL_DB_ERROR;
    const ShortcutData& data = allData[0];

    // Get a connection from the pool.
    DBConnection* connection = m_dbmanager->GetConnection(DBT_Shortcut);
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
                // This table has one and only field that packages all suggest matches in Explore screen.
                // So it's an easy way to remove all fields before add/update one.
                {
                    DBCommand command = connection->CreateCommand(g_removeAllShortcuts);
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

                // Insert a suggest list including all suggest matches in explore screen.
                {
                    DBCommand command = connection->CreateCommand(g_addShortcut);
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

PAL_DB_Error ShortcutDBDAO::RemoveAll()
{
    PAL_DB_Error result = PAL_DB_ERROR;

    // Get a connection from the pool.
    DBConnection* connection = m_dbmanager->GetConnection(DBT_Shortcut);
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
                DBCommand command = connection->CreateCommand(g_removeAllShortcuts);
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

PAL_DB_Error ShortcutDBDAO::GetAll(std::vector<ShortcutData>& allData)
{
    PAL_DB_Error result = PAL_DB_ERROR;
    std::vector<ShortcutData> tempShortcuts;

    // Get a connection from the pool.
    DBConnection* connection = m_dbmanager->GetConnection(DBT_Shortcut);
    if (connection)
    {
        do
        {
            DBCommand command = connection->CreateCommand(g_getAllShortcuts);
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

                ShortcutData shortcut;
                shortcut.m_id = id;
                shortcut.m_text = textData;
                shortcut.m_timestamp = timestamp;

                tempShortcuts.push_back(shortcut);

                result = reader->Read();
            }

            result = (result == PAL_DB_DONE) ? PAL_DB_OK : result;

            allData = tempShortcuts;

        } while (0);

        // Put connection to the pool.
        m_dbmanager->ReleaseConnection(connection);
        connection = NULL;
    }

    return result;
}

/*! @} */
