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

 @file sqlite_connection.cpp
 @date 9/13/10

 SQLite connection object

*/
/*
 (C) Copyright 2010 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret
 as defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly prescribing
 the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

#include <sqlite3.h>
#include "sqlite3x.h"

namespace sqlite3x {

sqlite3_connection::sqlite3_connection() : db(NULL) {}
sqlite3_connection::sqlite3_connection(const char *db) : db(NULL) { this->open(db); }
sqlite3_connection::~sqlite3_connection() { if(this->db) sqlite3_close(this->db); }
SQLITE_Error sqlite3_connection::open(const char *db) {
    return sqlite3_open(db, &this->db);
}
SQLITE_Error sqlite3_connection::close() {
    SQLITE_Error sqlite_result = SQLITE_OK;
    if(this->db) {
        sqlite_result = sqlite3_close(this->db);
        this->db=NULL;
    }
    return sqlite_result;
}
SQLITE_Error sqlite3_connection::insertid(long long& id) {
    if(!this->db) return SQLITE_MISUSE;
    id = sqlite3_last_insert_rowid(this->db);
    return SQLITE_OK;
}
SQLITE_Error sqlite3_connection::setbusytimeout(int ms) {
    if(!this->db) return SQLITE_MISUSE;
    return sqlite3_busy_timeout(this->db, ms);
}
SQLITE_Error sqlite3_connection::executenonquery(const char *sql) {
    if(!this->db) return SQLITE_MISUSE;
    sqlite3_command cmd(*this);
    SQLITE_Error errcode = cmd.prepare(sql);
    if (errcode == SQLITE_OK)
    {
        errcode = cmd.executenonquery();
    }
    return errcode;
}
SQLITE_Error sqlite3_connection::executenonquery(const std::string &sql) {
    if(!this->db) return SQLITE_MISUSE;
    sqlite3_command cmd(*this);
    SQLITE_Error errcode = cmd.prepare(sql);
    if (errcode == SQLITE_OK)
    {
        return cmd.executenonquery();
    }
    return errcode;
}
SQLITE_Error sqlite3_connection::executeint(const char *sql, int& result) {
    if(!this->db) return SQLITE_MISUSE;
    sqlite3_command cmd(*this);
    SQLITE_Error errcode = cmd.prepare(sql);
    if (errcode == SQLITE_OK)
    {
        return cmd.executeint(result);
    }
    return errcode;
}
SQLITE_Error sqlite3_connection::executeint(const std::string &sql, int& result) {
    if(!this->db) return SQLITE_MISUSE;
    sqlite3_command cmd(*this);
    SQLITE_Error errcode = cmd.prepare(sql);
    if (errcode == SQLITE_OK)
    {
        errcode = cmd.executeint(result);
    }
    return errcode;
}
SQLITE_Error sqlite3_connection::executeint64(const char *sql, long long& result) {
    if(!this->db) return SQLITE_MISUSE;
    sqlite3_command cmd(*this);
    SQLITE_Error errcode = cmd.prepare(sql);
    if (errcode == SQLITE_OK)
    {
        return cmd.executeint64(result);
    }
    return errcode;
}
SQLITE_Error sqlite3_connection::executeint64(const std::string &sql, long long& result) {
    if(!this->db) return SQLITE_MISUSE;
    sqlite3_command cmd(*this);
    SQLITE_Error errcode = cmd.prepare(sql);
    if (errcode == SQLITE_OK)
    {
        return cmd.executeint64(result);
    }
    return errcode;
}
SQLITE_Error sqlite3_connection::executedouble(const char *sql, double& result) {
    if(!this->db) return SQLITE_MISUSE;
    sqlite3_command cmd(*this);
    SQLITE_Error errcode = cmd.prepare(sql);
    if (errcode == SQLITE_OK)
    {
        return cmd.executedouble(result);
    }
    return errcode;
}
SQLITE_Error sqlite3_connection::executedouble(const std::string &sql, double& result) {
    if(!this->db) return SQLITE_MISUSE;
    sqlite3_command cmd(*this);
    SQLITE_Error errcode = cmd.prepare(sql);
    if (errcode == SQLITE_OK)
    {
        return cmd.executedouble(result);
    }
    return errcode;
}
SQLITE_Error sqlite3_connection::executestring(const char *sql, std::string& result) {
    if(!this->db) return SQLITE_MISUSE;
    sqlite3_command cmd(*this);
    SQLITE_Error errcode = cmd.prepare(sql);
    if (errcode == SQLITE_OK)
    {
        return cmd.executestring(result);
    }
    return errcode;
}
SQLITE_Error sqlite3_connection::executestring(const std::string &sql, std::string& result) {
    if(!this->db) return SQLITE_MISUSE;
    sqlite3_command cmd(*this);
    SQLITE_Error errcode = cmd.prepare(sql);
    if (errcode == SQLITE_OK)
    {
        return cmd.executestring(result);
    }
    return errcode;
}
SQLITE_Error sqlite3_connection::executeblob(const char *sql, std::string& result) {
    if(!this->db) return SQLITE_MISUSE;
    sqlite3_command cmd(*this);
    SQLITE_Error errcode = cmd.prepare(sql);
    if (errcode == SQLITE_OK)
    {
        return cmd.executeblob(result);
    }
    return errcode;
}
SQLITE_Error sqlite3_connection::executeblob(const std::string &sql, std::string& result) {
    if(!this->db) return SQLITE_MISUSE;
    sqlite3_command cmd(*this);
    SQLITE_Error errcode = cmd.prepare(sql);
    if (errcode == SQLITE_OK)
    {
        return cmd.executeblob(result);
    }
    return errcode;
}
SQLITE_Error sqlite3_connection::execute(const std::string &sql) {
    char* errmsg = NULL;
    if(!this->db)
        return SQLITE_MISUSE;
    SQLITE_Error errcode = sqlite3_exec(this->db, sql.c_str(), NULL, NULL, &errmsg);
    if (errcode != SQLITE_OK)
    {
        if (errmsg)
            sqlite3_free(errmsg);
    }
    return errcode;
}
const char* sqlite3_connection::getlasterror ()
{
    return (const char*)sqlite3_errmsg16(this->db);
}
}
