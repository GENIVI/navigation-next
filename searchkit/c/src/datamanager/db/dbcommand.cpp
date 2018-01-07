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

    @file dbcommand.cpp
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

#include "dbcommand.h"

DBCommand::DBCommand( PAL_DBCommand* cmd )
    : m_pCmd( cmd )
    , m_pReader( NULL )
{
}

DBCommand::~DBCommand()
{
    Finalize();
}

/* See header file for description */
PAL_DB_Error 
DBCommand::Step()
{
    if ( !IsValid() )
    {
        return SQLITE_ERROR;
    }
    return PAL_DBCommandStep( m_pCmd, m_pReader == NULL ? NULL : m_pReader->GetHandle() );
}

/* See header file for description */
PAL_DB_Error 
DBCommand::BindDoubleParam( const char* paramName, double data )
{
    if ( !IsValid() || m_pReader != NULL )
    {
        return SQLITE_ERROR;
    }
    return PAL_DBBindDoubleParam( m_pCmd, paramName, data );
}

/* See header file for description */
PAL_DB_Error 
DBCommand::BindIntParam( const char* paramName, int data )
{
    if ( !IsValid() || m_pReader != NULL )
    {
        return SQLITE_ERROR;
    }
    return PAL_DBBindIntParam( m_pCmd, paramName, data );
}

/* See header file for description */
PAL_DB_Error 
DBCommand::BindInt64Param( const char* paramName, int64 data )
{
    if ( !IsValid() || m_pReader != NULL )
    {
        return SQLITE_ERROR;
    }
    return PAL_DBBindLongParam( m_pCmd, paramName, data );
}

/* See header file for description */
PAL_DB_Error 
DBCommand::BindStringParam( const char* paramName, const char* data )
{
    if ( !IsValid() || m_pReader != NULL )
    {
        return SQLITE_ERROR;
    }
    return PAL_DBBindStringParam( m_pCmd, paramName, data );
}

/* See header file for description */
PAL_DB_Error 
DBCommand::BindBlobParam( const char* paramName, void* data, int datalen )
{
    if ( !IsValid() || m_pReader != NULL )
    {
        return SQLITE_ERROR;
    }
    return PAL_DBBindBlobParam( m_pCmd, paramName, data, datalen );
}

/* See header file for description */
DBReader* 
DBCommand::ExecuteReader()
{
    if ( !IsValid() || m_pReader != NULL )
    {
        return NULL;
    }
    PAL_DBReader* pReader = NULL;
    if ( PAL_DBExecuteReader( m_pCmd, &pReader ) == SQLITE_OK )
    {
        m_pReader = new DBReader( pReader );
        return m_pReader;
    }
    return NULL;
}

/* See header file for description */
PAL_DB_Error 
DBCommand::ExecuteInt( int* result )
{
    if ( !IsValid() || m_pReader != NULL )
    {
        return SQLITE_ERROR;
    }
    return PAL_DBExecuteInt( m_pCmd, result );
}

/* See header file for description */
PAL_DB_Error 
DBCommand::ExecuteInt64( int64* result )
{
    if ( !IsValid() || m_pReader != NULL )
    {
        return SQLITE_ERROR;
    }
    return PAL_DBExecuteLong( m_pCmd, result );
}

/* See header file for description */
PAL_DB_Error 
DBCommand::ExecuteDouble( double* result )
{
    if ( !IsValid() || m_pReader != NULL )
    {
        return SQLITE_ERROR;
    }
    return PAL_DBExecuteDouble( m_pCmd, result );
}

/* See header file for description */
PAL_DB_Error 
DBCommand::ExecuteString( char** result )
{
    if ( !IsValid() || m_pReader != NULL )
    {
        return SQLITE_ERROR;
    }
    return PAL_DBExecuteString( m_pCmd, result );
}

/* See header file for description */
PAL_DB_Error 
DBCommand::ExecuteBlob( void** result, int* size )
{
    if ( !IsValid() || m_pReader != NULL )
    {
        return SQLITE_ERROR;
    }
    return PAL_DBExecuteBlobEx( m_pCmd, (byte**)result, size );
}

/* See header file for description */
PAL_DB_Error 
DBCommand::ExecuteNonQuery()
{
    if ( !IsValid() || m_pReader != NULL )
    {
        return SQLITE_ERROR;
    }
    return PAL_DBExecuteNonQueryCommand( m_pCmd );
}

/* See header file for description */
PAL_DB_Error 
DBCommand::Finalize()
{
    PAL_DB_Error ret = SQLITE_OK;
    if ( IsValid() )
    {
        if ( m_pReader == NULL )
        {
            ret = PAL_DBCommandFinalize( m_pCmd, NULL );
            if ( ret == SQLITE_OK )
            {
                m_pCmd = NULL;
            }
        }
        else
        {
            ret = PAL_DBCommandFinalize( m_pCmd, m_pReader->GetHandle() );
            if ( ret == SQLITE_OK )
            {
                delete m_pReader;
                m_pReader = NULL;
                m_pCmd = NULL;
            }
        }
    }
    return ret;
}

/*! @} */

