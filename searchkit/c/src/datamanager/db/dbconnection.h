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

	@file dbconnection.h
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
#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <string>
#include "dbcommand.h"

class DBConnection
{
private:
    //forbid copy constructor
    DBConnection( DBConnection& );

public:
    DBConnection();
    ~DBConnection();

    /*! Brief Open a connection to the specific path
    @return Returs SQLITE_OK if succeed.
    @see
    @see
    */
    PAL_DB_Error
    Open( struct PAL_Instance* pal,                 /*!< pal instance */
          const char* dbname                        /*!< database file path */
          );

    /*! Brief Close the connection
    @return Returs SQLITE_OK if succeed.
    @see
    @see
    */
    PAL_DB_Error
    Close();

    /*! Brief Create a DBCommand object.
    @return Returs NULL if failed.
    @see
    @see
    */
    DBCommand
    CreateCommand( const char* sql                    /*!< format string */
                   );

    /*! Brief To excute a sql command directly
    @return Returs SQLITE_OK if succeed.
    @see
    @see
    */
    PAL_DB_Error
    Excute( const char* sql                         /*!< sql command string */
            );

    /*! Brief Request the error code occured last time
    @return Returs the last error code.
    @see
    @see
    */
    int GetLastError();

    inline bool IsValid() const { return m_connection != NULL; }
    inline const char* GetPath() const { return m_path.c_str(); }
private:
    PAL_DBConnect* m_connection;
    int m_error;
    std::string m_path;
};

#endif


/*! @} */

