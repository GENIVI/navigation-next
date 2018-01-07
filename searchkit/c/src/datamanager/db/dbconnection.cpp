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

    @file dbconnection.cpp

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

#include "dbconnection.h"

DBConnection::DBConnection()
    : m_connection( NULL )
    , m_error( SQLITE_OK )
    , m_path()
{
}

DBConnection::~DBConnection()
{
    if ( IsValid() )
    {
        Close();
    }
}

/* See header file for description */
PAL_DB_Error 
DBConnection::Open( struct PAL_Instance* pal, const char* dbname )
{
    m_path = dbname;
    return PAL_DBOpen( pal, PAL_DB_Sqlite, dbname, &m_connection );
}

/* See header file for description */
PAL_DB_Error 
DBConnection::Close()
{
    if ( IsValid() )
    {
        PAL_DB_Error ret = PAL_DBClose( m_connection );
        if ( ret == SQLITE_OK )
        {
            m_connection = NULL;
        }
        return ret;
    }
    return SQLITE_OK;
}

/* See header file for description */
DBCommand 
DBConnection::CreateCommand( const char* sql )
{
    if ( !IsValid() )
    {
        return DBCommand( NULL );
    }

    PAL_DBCommand* pCmd = NULL;
    PAL_DB_Error err = PAL_DBInitCommand( m_connection, sql, &pCmd );
    if ( err == SQLITE_OK )
    {
        return DBCommand( pCmd );
    }
    return DBCommand( NULL );
}

/* See header file for description */
PAL_DB_Error 
DBConnection::Excute( const char* sql )
{
    if ( IsValid() )
    {
        return PAL_DBExecuteNonQuery( m_connection, sql );
    }
    return SQLITE_ERROR;
}

/* See header file for description */
int 
DBConnection::GetLastError()
{
    m_error = SQLITE_OK;
    if ( IsValid() )
    {
        PAL_DBGetLastError( m_connection, &m_error );
    }
    return m_error;
}

/*! @} */

