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

    @file dbmanager.cpp
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

#include "dbmanager.h"
#include "nbcontextprotected.h"
#include "palfile.h"

DBManager::DBManager(NB_Context* context):m_pLock(NULL),m_context(context)
{
    nsl_memset( m_DBConnections, 0, sizeof( m_DBConnections ) );
    nsl_memset( m_DBConnectionNum, 0, sizeof( m_DBConnectionNum ) );
    m_palInstance = NB_ContextGetPal(context);
    PAL_LockCreate(m_palInstance, &m_pLock);
}

DBManager::~DBManager()
{
    for ( int i = 0; i < DBT_Number; ++i )
    {
        for ( uint32 j = 0; j < m_DBConnectionNum[i]; ++j )
        {
            if ( m_DBConnections[i][j] != NULL )
            {
                m_DBConnections[i][j]->Close();
                delete m_DBConnections[i][j];
            }
        }
    }
    PAL_LockDestroy(m_pLock);
}

/* See header file for description */
void 
DBManager::SetupDB( DatabaseType type, const char* name )
{
    // check type valid
    if ( type < 0 || type > DBT_Number ) /* check param */
    {
        return;
    }
    PAL_LockLock(m_pLock);
    if ( name == NULL )
    {
        m_DBNames[type].clear();
    }
    else
    {
        m_DBNames[type] = name;
    }
    PAL_LockUnlock(m_pLock);
}

/* See header file for description */
DBConnection* 
DBManager::GetConnection( DatabaseType type )
{
    // check type valid
    if ( type < 0 || type > DBT_Number )
    {
        return NULL;
    }
    PAL_LockLock(m_pLock);
    // find a existing DBConnection object
    std::list<DBConnection*>::iterator iter = m_DBConnectionPool[type].begin();
    while ( iter != m_DBConnectionPool[type].end() )
    {
        DBConnection* pConnection = *iter;
        // check path
        if ( m_DBNames[type].compare( pConnection->GetPath() ) == 0 )
        {
            m_DBConnectionPool[type].erase( iter );
            m_DBConnectionInUse.insert( std::pair<DBConnection*, DatabaseType>( pConnection, type ) );
            PAL_LockUnlock(m_pLock);
            return pConnection;
        }
        // try to close if path not match
        if ( pConnection->Close() == SQLITE_OK )
        {
            //open new path
            if ( pConnection->Open( m_palInstance, m_DBNames[type].c_str() ) == SQLITE_OK )
            {
                m_DBConnectionPool[type].erase( iter );
                m_DBConnectionInUse.insert( std::pair<DBConnection*, DatabaseType>( pConnection, type ) );
                PAL_LockUnlock(m_pLock);
                return pConnection; 
            }
            else 
            {
                PAL_LockUnlock(m_pLock);
                return NULL;
            }

        }
        iter ++;
    }

    // no matched existing DBConnection object
    // try to create new
    if ( m_DBConnectionNum[type] < MAX_DB_CONNECTION_NUM )
    {
        DBConnection* &rpConnection = m_DBConnections[type][m_DBConnectionNum[type]];
        rpConnection = new DBConnection();
        if ( rpConnection != NULL )
        {
            ++m_DBConnectionNum[type];

            // try to open the path
            if ( rpConnection->Open( m_palInstance, m_DBNames[type].c_str() ) == SQLITE_OK )
            {
                m_DBConnectionInUse.insert( std::pair<DBConnection*, DatabaseType>( rpConnection, type ) );
                PAL_LockUnlock(m_pLock);
                return rpConnection;
            }
            else
            {
                m_DBConnectionPool[type].push_front( rpConnection );
            }
        }
    }
    PAL_LockUnlock(m_pLock);
    return NULL;
}

/* See header file for description */
void
DBManager::Disconnect( DatabaseType type )
{
    // check type valid
    if ( type < 0 || type > DBT_Number )
    {
        return ;
    }

    PAL_LockLock(m_pLock);
    // find a existing DBConnection object
    std::list<DBConnection*>::iterator iter = m_DBConnectionPool[type].begin();
    while ( iter != m_DBConnectionPool[type].end() )
    {
        DBConnection* pConnection = *iter;
        // check path
        if ( m_DBNames[type].compare( pConnection->GetPath() ) == 0 )
        {
            // remove connection from m_DBConnectionPool
            m_DBConnectionPool[type].erase( iter );
            --m_DBConnectionNum[type];
            // close connection
            pConnection->Close();
            break;
        }
        iter ++;
    }
    PAL_LockUnlock(m_pLock);
}

/* See header file for description */
void 
DBManager::ReleaseConnection( DBConnection* pConnection )
{
    // check pointer valid
    PAL_LockLock(m_pLock);
    std::map<DBConnection*, DatabaseType>::iterator iter = m_DBConnectionInUse.find( pConnection );
    if ( iter == m_DBConnectionInUse.end() )
    {
        PAL_LockUnlock(m_pLock);
        return;
    }
    DatabaseType type = iter->second;
    m_DBConnectionInUse.erase( iter );
    m_DBConnectionPool[type].push_front( pConnection );
    PAL_LockUnlock(m_pLock);
}

/* See header file for description */
void
DBManager::MasterClear()
{
    PAL_LockLock(m_pLock);
    for (int type = 0; type < DBT_Number; ++type)
    {
        if (!m_DBNames[type].empty() && PAL_FileExists(m_palInstance, m_DBNames[type].c_str()) == PAL_Ok)
        {
            PAL_FileRemove(m_palInstance, m_DBNames[type].c_str());
        }
    }
    PAL_LockUnlock(m_pLock);
}

/*! @} */

