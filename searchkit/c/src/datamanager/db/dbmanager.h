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
	@file dbmanager.h
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

#pragma once
#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <string>
#include <list>
#include <map>
#include "datamanagerdefs.h"
#include "dbconnection.h"
#include "nberror.h"
#include "nbcontext.h"
#include "pallock.h"

#define MAX_DB_CONNECTION_NUM 1

class ControllerManager;

/*! Brief Manage DBConnection objects;
*/
class DBManager
{
public:
    DBManager(NB_Context* context);
    ~DBManager();
    /*!
     *  @brief To create a connection to a specific path
     *  @param type database type id
     *  @param database file path
     */
    void SetupDB( DatabaseType type, const char* name);
    /*!
     *  @brief Get the DBConnection object by database type id
     *  @param type database type
     *  @return Returs NULL if failed.
     */
    DBConnection* GetConnection( DatabaseType type);
    /*! 
     *  @brief DisConnect object by database type id
     *  @param type database type
     *  @return Returs NULL if failed.
     */
    void Disconnect( DatabaseType type);
    /*!
     *  @brief Release the DBConnection to the pool
     *  @see DBConnection::Close
     *  @param pConnection pointer to the DBConnection to be released
     */
    void ReleaseConnection( DBConnection* pConnection);

    /*!
     *  @brief Delete all database files
     */
    void MasterClear();


    inline PAL_Instance* GetPalInstance() { return m_palInstance; }
    NB_Context* GetContext() const { return m_context; }

private:
    DBConnection* m_DBConnections[DBT_Number][MAX_DB_CONNECTION_NUM];
    uint32 m_DBConnectionNum[DBT_Number];
    std::list<DBConnection*> m_DBConnectionPool[DBT_Number];
    std::map<DBConnection*, DatabaseType> m_DBConnectionInUse;
    std::string m_DBNames[DBT_Number];
    PAL_Lock* m_pLock;
    PAL_Instance* m_palInstance;
    NB_Context* m_context;
};

#endif


/*! @} */

