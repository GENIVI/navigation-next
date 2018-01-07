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

	@file dbcommand.h
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
#ifndef DB_COMMAND_H
#define DB_COMMAND_H

#include "dbreader.h"

class DBCommand
{
private:
    // forbid copy constructor
    //DBCommand( DBCommand& );

public:
    DBCommand( PAL_DBCommand* cmd );
    ~DBCommand();

    PAL_DB_Error Step();

    /*! Brief Format command string by data.
    @return Returs SQLITE_OK if succeed.
    @see
    @see
    */
    PAL_DB_Error 
    BindDoubleParam( const char* paramName,                             /*!< parameter name */
                     double data                                        /*!< value */
                     );
    PAL_DB_Error 
    BindIntParam( const char* paramName,                                /*!< parameter name */
                  int data                                              /*!< value */
                  );
    PAL_DB_Error
    BindInt64Param( const char* paramName,                              /*!< parameter name */
                    int64 data                                          /*!< value */
                    );
    PAL_DB_Error
    BindStringParam( const char* paramName,                             /*!< parameter name */
                     const char* data                                   /*!< the string */
                     );
    PAL_DB_Error
    BindBlobParam( const char* paramName,                               /*!< parameter name */
                   void* data,                                          /*!< buffer pointer */
                   int datalen                                          /*!< data size */
                   );

    /*! Brief Execute command and return the results as a reader.
    @return Returs NULL if failed.
    @see
    @see
    */
    DBReader* ExecuteReader();

    /*! Brief Execute the command.
    @return Returs SQLITE_OK if succeed.
    @see
    @see
    */
    PAL_DB_Error 
    ExecuteInt( int* result         /*!< the result */
                );
    PAL_DB_Error
    ExecuteInt64( int64* result     /*!< the result */
                  );
    PAL_DB_Error
    ExecuteDouble( double* result   /*!< the result */
                   );
    PAL_DB_Error
    ExecuteString( char** result    /*!< buffer pointer */
                   );
    PAL_DB_Error
    ExecuteBlob( void** result,     /*!< buffer pointer */
                 int* size          /*!< buffer lenth */
                 );
    PAL_DB_Error
    ExecuteNonQuery();

    inline bool IsValid() const { return m_pCmd != NULL; }
private:
    /*! Brief clean up the command.
    @return Returs SQLITE_OK if succeed.
    */
    PAL_DB_Error Finalize();

    PAL_DBCommand* m_pCmd;
    DBReader* m_pReader;
};

#endif

/*! @} */

