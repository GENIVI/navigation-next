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

 @file     paldb.h
 @date     09/01/2010
 @defgroup PAL DB API

 @brief    Platform-independent DataBase API

 */
/*
 (C) Copyright 2010 by Networks In Motion, Inc.

 The information contained herein is confidential, proprietary
 to Networks In Motion, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of Networks In Motion is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/

#ifndef PALDB_H
#define PALDB_H


#include "pal.h"
#include "paltypes.h"
#include "palstdlib.h"

typedef enum
{
    PAL_DB_Sqlite,          // using sqlite store data
    PAL_DB_File             // using file store data
}PAL_DB_Type;

typedef int    PAL_DB_Error;
typedef struct PAL_DBConnect PAL_DBConnect;
typedef struct PAL_DBCommand PAL_DBCommand;   // using when PAL_DB_Type is PAL_DB_Sqlite
typedef struct PAL_DBReader PAL_DBReader;     // using when PAL_DB_Type is PAL_DB_File

/*! Open a database and initialize a db connection.

 The connection can be maintained in the application for as long as you want
 to use the DB. Multiple connections can also be obtained on the same DB.
 The caller is responsible to call PAL_DBClose to close DB which
 will then release DB resources

 A valid object must be destroyed by PAL_DBClose.
 @return 0 when success, otherwise error code.
 */
PAL_DEC PAL_DB_Error
PAL_DBOpen(PAL_Instance* pal,                       /*!<The pal instance*/
           PAL_DB_Type type,                        /*!<Specify the store type in pal layer */
           const char* name,                        /*!<Name of the database which you want to open */
           PAL_DBConnect** connection               /*!<connection on success, a new created PAL_DBConnect object; NULL otherwise */
           );


/*! Close the database and free allocated memory.
 @return 0 when success, otherwise error code.
 */
PAL_DEC PAL_DB_Error
PAL_DBClose(PAL_DBConnect* connection               /*!<A PAL_DBConnect object createed with PAL_DBOpen.*/
            );

/*! Read the data from file,only using for PAL_DB_Type is PAL_DB_File
 @return 0 when success, otherwise error code
 */
PAL_DEC PAL_DB_Error
PAL_DBRead(PAL_DBConnect* connection,           /*!<A PAL_DBConnect object createed with PAL_DBOpen.*/
           byte** data,                             /*!<The buffer to read data into*/
           uint32* length                           /*!<The number of bytes read into the buffer*/
           );

/*! Begin a transaction, must call commit or rollback after finish the job*/
PAL_DEC PAL_DB_Error
PAL_DB_Transaction_Begin(PAL_DBConnect* con);

/*! commit a transaction. */
PAL_DEC PAL_DB_Error
PAL_DB_Transaction_Commit(PAL_DBConnect* con);

/*! rollback a transaction. */
PAL_DEC PAL_DB_Error
PAL_DB_Transaction_Rollback(PAL_DBConnect* con);

/*! Write the data from file,only using for PAL_DB_Type is PAL_DB_File
 @return 0 when success, otherwise error code
 */
PAL_DEC PAL_DB_Error
PAL_DBWrite(PAL_DBConnect* connection,              /*!<A PAL_DBConnect object createed with PAL_DBOpen.*/
            byte* data,                             /*!<The buffer to write data from */
            uint32 length                           /*!<The number of bytes write into file*/
            );

// below function using when PAL_DB_Type is PAL_DB_Sqlite
/*! Get the last error code from PAL_DB.
 @return 0 when success, otherwise error code.
 */
PAL_DEC PAL_DB_Error
PAL_DBGetLastError(PAL_DBConnect* connection,       /*!<A PAL_DBConnect object createed with PAL_DBOpen.*/
                   int* error                       /*!<The last error message.*/
                   );

/*! Execute the SQL command without return.

 Used for INSERT, UPDATE, DELETE statements.
 @return 0 when success, otherwise error code.
 */
PAL_DEC PAL_DB_Error
PAL_DBExecuteNonQuery(PAL_DBConnect* connection,    /*!<A PAL_DBConnect object createed with PAL_DBOpen.*/
                      const char* command           /*!<The SQL command which need to execute.*/
                      );

/*! Execute the SQL command without return.

 Used for INSERT, UPDATE, DELETE statements.
 @return 0 when success, otherwise error code.
 */
PAL_DEC PAL_DB_Error
PAL_DBExecuteNonQueryCommand(PAL_DBCommand* command /*!<sqlCommand instance On success, a new created PAL_DBCommand object; NULL otherwise.*/
                             );

/*! Prepare a SQL statement and return a command object.

 It requires a valid connection object.
 The command object can be retained to run the same prepared SQL many times.
 The caller has to release the command object by calling the PAL_DBCommandFinalze.

 A valid object must be destroyed by PAL_DBCommandFinalize.
 @return 0 when success, otherwise error code.
 */
PAL_DEC PAL_DB_Error
PAL_DBInitCommand(PAL_DBConnect* connection,        /*!<A PAL_DBConnect object createed with PAL_DBOpen.*/
                  const char* sqlCommand,           /*!<The SQL command which need to execute.*/
                  PAL_DBCommand** command           /*!<sqlCommand instance On success, a new created PAL_DBCommand object; NULL otherwise.*/
                  );



/*! Prepare for the next statement.

 Used to prepare the next statement if a series of SQL statements have been passed when
 initializing command object.

 @return 0 when success, otherwise error code.
 */
PAL_DEC PAL_DB_Error
PAL_DBCommandStep(PAL_DBCommand* command,           /*!<A PAL_DBCommand object createed with PAL_DBInitCommand.*/
                  PAL_DBReader* reader              /*!<A PAL_DBReader object createed with PAL_DBExecuteReader.*/
                  );


/*! Destroy the PAL_DBCommand object and release memory.

 @return 0 when success, otherwise error code.
 */
PAL_DEC PAL_DB_Error
PAL_DBCommandFinalize(PAL_DBCommand* command,       /*!<A PAL_DBCommand object createed with PAL_DBInitCommand.*/
                      PAL_DBReader* reader          /*!<A PAL_DBReader object createed with PAL_DBExecuteReader.*/
                      );


/*! Bind data to parameter in SQL.

 The parameter name must be a valid name in the query.
 Bind methods are available for the data types supported in SQLite.

 @param sqlCommand A PAL_DBCommand object createed with PAL_DBInitCommand.
 @param paramName The parameter name used in the table.
 @param data The data need to bind to parameter in SQL.
 @param datalen The length of the data.
 @return 0 when success, otherwise error code.
 */
PAL_DEC PAL_DB_Error
PAL_DBBindDoubleParam(PAL_DBCommand* command,
                      const char* paramName,
                      double data
                      );

PAL_DEC PAL_DB_Error
PAL_DBBindIntParam(PAL_DBCommand* command,
                   const char* paramName,
                   int data
                   );

PAL_DEC PAL_DB_Error
PAL_DBBindLongParam(PAL_DBCommand* command,
                    const char* paramName,
                    int64 data
                    );

PAL_DEC PAL_DB_Error
PAL_DBBindStringParam(PAL_DBCommand* command,
                      const char* paramName,
                      const char* data
                      );

PAL_DEC PAL_DB_Error
PAL_DBBindBlobParam(PAL_DBCommand* command,
                    const char* paramName,
                    void* data,
                    int length
                    );


/*! Return a single value from the database.

  Call this method if you want the value from the first cell in the resultset.
  Please note these are a C interface, and all allocated memory should be allocated by
  calling nsl_malloc(), and be freed by calling nsl_free(). Mismatched nsl_malloc/nsl_free
  and new/delete will lead to disaster.

  @param sqlCommand A PAL_DBCommand object createed with PAL_DBInitCommand.
  @param result The value of the first cell in the resultset when success.
  @param result_len The length of the result when success.
  @return 0 when success, otherwise error code.
*/
PAL_DEC PAL_DB_Error
PAL_DBExecuteDouble(PAL_DBCommand* command,
                    double* result
                    );

PAL_DEC PAL_DB_Error
PAL_DBExecuteInt(PAL_DBCommand* command,
                 int* result
                 );

PAL_DEC PAL_DB_Error
PAL_DBExecuteLong(PAL_DBCommand* command,
                  int64* result
                  );

PAL_DEC PAL_DB_Error
PAL_DBExecuteString(PAL_DBCommand* command,
                    char** result
                    );

PAL_DEC PAL_DB_Error
PAL_DBExecuteBlobEx(PAL_DBCommand* command,
                    byte** result,
                    int* length
                    );

/*! Sqlite_ExecuteReader provides a reader object to step through the resultant rowset
 @return 0 when success, otherwise error code.
 */
PAL_DEC PAL_DB_Error
PAL_DBExecuteReader (PAL_DBCommand* command,        /*!<A PAL_DBCommand object createed with PAL_DBInitCommand.*/
                     PAL_DBReader** reader          /*!<rdr instance On success, a new created PAL_DBReader object; NULL otherwise.*/
                     );

/*! Sqlite_ResetReader resets the reader to it's start position.
 Instead of calling InitCommand again to execute the same query,
 you can reset the reader and directly step into it to get the data.
 New parameter values can be bound to the SQL statement by calling
 ResetReader first.
 @return 0 when success, otherwise error code.
 */
PAL_DEC PAL_DB_Error
PAL_DBResetReader (PAL_DBReader* reader             /*!<A PAL_DBReader object createed with PAL_DBExecuteReader.*/
                   );



/*! Reader the next result in resultset. For the first time, need call this function before call PAL_DBReaderGetXXX function
 @return PAL_SQLITE_ROW if has another row ready, SQLITE_DONE if has finished executing.other ways error code.
 */
PAL_DEC PAL_DB_Error
PAL_DBReaderRead (PAL_DBReader* reader              /*!<A PAL_DBReader object createed with PAL_DBExecuteReader.*/
                  );

/*! Return a single value from the database.

 Call this method if you want the value with the column specified in the resultset.
 Please note these are a C interface, and all allocated memory should be allocated by
 calling nsl_malloc(), and be freed by calling nsl_free(). Mismatched nsl_malloc/nsl_free
 and new/delete will lead to disaster.

 @param rdr A PAL_DBReader object createed with PAL_DBExecuteReader.
 @param colname The name of column in the resultset when success.
 @param result The value of the result when success.
 @return 0 when success, otherwise error code.
 */

PAL_DEC PAL_DB_Error
PAL_DBReaderGetInt(PAL_DBReader* reader,
                   const char* columName,
                   int* result
                   );

PAL_DEC PAL_DB_Error
PAL_DBReaderGetLong(PAL_DBReader* reader,
                    const char* columName,
                    int64* result
                    );

PAL_DEC PAL_DB_Error
PAL_DBReaderGetdouble(PAL_DBReader* reader,
                      const char* columName,
                      double* result
                      );

PAL_DEC PAL_DB_Error
PAL_DBReaderGetstring(PAL_DBReader* reader,
                      const char* columName,
                      char** result
                      );

PAL_DEC PAL_DB_Error
PAL_DBReaderGetblob(PAL_DBReader* reader,
                    const char* columName,
                    byte** result,
                    int* length
                    );

/*! @} */

#endif    // PALDB_H
