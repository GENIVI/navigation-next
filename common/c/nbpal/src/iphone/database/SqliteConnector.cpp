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
    @file     SqliteConnector.cpp
    @date     09/01/2010
    @defgroup PAL DB API
    APIs to access data and store data in SQLite.
    For our purposes, SQlite DB is set to handle all data in UTF-8.
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/
#include <sqlite3.h>
#include "sqlite3x.h"
#include "paldb.h"
#include "palfile.h"
using namespace sqlite3x;
struct PAL_DBConnect
{
    sqlite3_connection* m_Connection;

    PAL_File* m_File;
    PAL_Instance* m_Palinstance;

};
struct PAL_DBCommand
{
    sqlite3_command* m_Command;
};
struct PAL_DBReader
{
    sqlite3_reader* m_Reader;
};

/* See header file for description. */
PAL_DB_Error PAL_DBOpen(PAL_Instance* pal,
                        PAL_DB_Type dbtype,
                        const char* dbname,
                         PAL_DBConnect** dbconn
                         )
{
    if (!dbname)
    {
        return SQLITE_ERROR;
    }

    PAL_DB_Error error = SQLITE_OK;
    if (dbtype == PAL_DB_Sqlite)
    {
        PAL_DBConnect* _instance = (PAL_DBConnect*)nsl_malloc(sizeof(PAL_DBConnect));
        if (_instance)
        {
            // Init inner pinter
            _instance->m_Connection = NULL;
            _instance->m_File = NULL;
            _instance->m_Palinstance = NULL;

            sqlite3_connection* configConnection = new sqlite3_connection(dbname);
            if (configConnection)
            {
                _instance->m_Connection = configConnection;
                *dbconn = _instance;
            }
            else
            {
                nsl_free(_instance);
                error = SQLITE_NOMEM;
            }
        }
        else
        {
            error = SQLITE_NOMEM;
        }
    }
    else
    {
        error = PAL_ErrUnsupported;
    }

    return error;
}

PAL_DB_Error PAL_DBRead(PAL_DBConnect* conn,byte** data,uint32* dataLen)
{
    return PAL_ErrUnsupported;
}

PAL_DEC PAL_DB_Error
PAL_DB_Transaction_Begin(PAL_DBConnect* con)
{
    if(con == NULL || con->m_Connection == NULL)
    {
        return SQLITE_ERROR;
    }
    return con->m_Connection->executenonquery("begin;");
}

PAL_DEC PAL_DB_Error
PAL_DB_Transaction_Commit(PAL_DBConnect* con)
{
    if(con == NULL || con->m_Connection == NULL)
    {
        return SQLITE_ERROR;
    }
    return con->m_Connection->executenonquery("commit;");
}

PAL_DEC PAL_DB_Error
PAL_DB_Transaction_Rollback(PAL_DBConnect* con)
{
    if(con == NULL || con->m_Connection == NULL)
    {
        return SQLITE_ERROR;
    }
    return con->m_Connection->executenonquery("rollback;");
}

PAL_DB_Error PAL_DBWrite(PAL_DBConnect* conn,byte* data,uint32 datalen)
{
    return PAL_ErrUnsupported;
}
/* See header file for description. */
PAL_DB_Error PAL_DBClose(PAL_DBConnect* dbconn)
{
    PAL_DB_Error errCode = PAL_Ok;

    if (dbconn != NULL)
    {
        if (dbconn->m_Connection != NULL)
        {
            errCode = (PAL_DB_Error)dbconn->m_Connection->close();
            delete dbconn->m_Connection;
        }
        else
        {
            errCode =PAL_ErrUnsupported;
        }
        nsl_free(dbconn);
    }

    return errCode;
}
/* See header file for description. */
PAL_DB_Error PAL_DBGetLastError(PAL_DBConnect* dbconn,
                                 int* error
                                 )
{
    const char* errorDes = dbconn->m_Connection->getlasterror();

    if(errorDes!= NULL)
    {
        *error = SQLITE_ERROR;
    }
    else
    {
        *error = SQLITE_OK;
    }

    return SQLITE_OK;
}
/* See header file for description. */
PAL_DB_Error PAL_DBExecuteNonQuery(PAL_DBConnect* dbconn,
                                    const char* command)
{

    if (dbconn == NULL)
    {
        return SQLITE_ERROR;
    }
    return dbconn->m_Connection->execute(command);
}
/* See header file for description. */
PAL_DB_Error PAL_DBExecuteNonQueryCommand(PAL_DBCommand* sqlCommand)
{
    if (sqlCommand == NULL)
    {
        return SQLITE_ERROR;
    }
    return sqlCommand->m_Command->executenonquery();
}
/* See header file for description. */
PAL_DB_Error PAL_DBInitCommand(PAL_DBConnect* dbconn,
                               const char* command,
                               PAL_DBCommand** sqlCommand
                               )
{
    if (dbconn == NULL)
    {
        return SQLITE_ERROR;
    }
    PAL_DBCommand* _sqlCommand =(PAL_DBCommand*)nsl_malloc(sizeof(PAL_DBCommand));
    nsl_memset( _sqlCommand, 0, sizeof(PAL_DBCommand) );
    if (_sqlCommand == NULL)
    {
        return SQLITE_NOMEM;
    }
    sqlite3_command* tempCommand = new sqlite3_command (*(dbconn->m_Connection));

    if (tempCommand == NULL)
    {
        nsl_free(_sqlCommand);
        return SQLITE_NOMEM;
    }
    else
    {
        _sqlCommand->m_Command = tempCommand;
        *sqlCommand = _sqlCommand;
    }
    SQLITE_Error err = _sqlCommand->m_Command->prepare(command);
    if ( err != SQLITE_OK )
    {
        nsl_free(_sqlCommand);
        delete tempCommand;
        *sqlCommand = NULL;
    }
    return err;
}
/* See header file for description. */
PAL_DB_Error PAL_DBCommandStep(PAL_DBCommand* command,
                                       PAL_DBReader* rdr
                                       )
{
    if (command == NULL)
    {
        return SQLITE_ERROR;
    }
    // Close the reader to reduce the refence count on the command object by one.
    // StepNext fill follow with an execute command, and the execute commands
    // automatically increment the reference count of the cmd object
    if (rdr != NULL)
    {
        rdr->m_Reader->close();
    }

    return command->m_Command->stepnext();
}
/* See header file for description. */
PAL_DB_Error PAL_DBCommandFinalize(PAL_DBCommand* command,
                                   PAL_DBReader* rdr
                                   )
{
    PAL_DB_Error errorCode = SQLITE_OK;
    if (rdr != NULL)
    {
        errorCode = PAL_DBResetReader(rdr);
        if (errorCode == SQLITE_OK)
        {
            delete rdr->m_Reader;
            nsl_free(rdr);
        }
    }
    if(command != NULL)
    {
        delete command->m_Command;
        nsl_free(command);
    }

    return errorCode;
}
/* See header file for description. */
PAL_DB_Error PAL_DBBindDoubleParam(PAL_DBCommand* command,
                                           const char* paramName,
                                           double data
                                           )
{
    int index = command->m_Command->get_parameter_index(paramName);
    return command->m_Command->bind(index, data);
}
/* See header file for description. */
PAL_DB_Error PAL_DBBindIntParam(PAL_DBCommand* command,
                                        const char* paramName,
                                        int data
                                        )
{
    int index = command->m_Command->get_parameter_index(paramName);
    return command->m_Command->bind(index, data);
}
/* See header file for description. */
PAL_DB_Error PAL_DBBindLongParam(PAL_DBCommand* command,
                                         const char* paramName,
                                         int64 data
                                         )
{
    int index = command->m_Command->get_parameter_index(paramName);
    return command->m_Command->bind(index, data);
}
/* See header file for description. */
PAL_DB_Error PAL_DBBindStringParam(PAL_DBCommand* command,
                                           const char* paramName,
                                           const char* data
                                           )
{
    int index = command->m_Command->get_parameter_index(paramName);

    if (data == NULL)
    {
        return command->m_Command->bind(index);
    }
    else
    {
        return command->m_Command->bind(index, data,strlen(data));
    }
}
/* See header file for description. */
PAL_DB_Error PAL_DBBindBlobParam(PAL_DBCommand* command,
                                         const char* paramName,
                                         void* data,
                                         int datalen)
{
    int index = command->m_Command->get_parameter_index(paramName);
    return command->m_Command->bind (index, data, datalen);
}
/* See header file for description. */
PAL_DB_Error PAL_DBExecuteDouble(PAL_DBCommand* command,
                                         double* result
                                         )
{
    if (command == NULL)
    {
        return SQLITE_ERROR;
    }

    return command->m_Command->executedouble(*result);
}
/* See header file for description. */
PAL_DB_Error PAL_DBExecuteInt(PAL_DBCommand* command,
                                      int* result
                                      )
{
    if (command == NULL)
    {
        return SQLITE_ERROR;
    }

    return command->m_Command->executeint(*result);
}

/* See header file for description. */
PAL_DB_Error PAL_DBExecuteLong(PAL_DBCommand* command,
                                       int64* result
                                       )
{
    if (command == NULL)
    {
        return SQLITE_ERROR;
    }

    return  command->m_Command->executeint64(*result);
}

/* See header file for description. */
PAL_DB_Error PAL_DBExecuteString(PAL_DBCommand* command,
                                 char** result)
{
    if ((command == NULL)||(result == NULL))
    {
        return SQLITE_ERROR;
    }

    std::string str;
    SQLITE_Error errorCode = command->m_Command->executestring(str);

    if (errorCode == SQLITE_OK)
    {
        char* tempResult = nsl_strdup(str.c_str());

        if (tempResult == NULL)
        {
            errorCode = SQLITE_NOMEM;
        }
        else
        {
            *result = tempResult;
            errorCode = SQLITE_OK;
        }
    }

    return errorCode;
}

/* See header file for description. */
PAL_DB_Error PAL_DBExecuteBlobEx(PAL_DBCommand* command,
                                 byte** result,
                                 int* result_len)
{
    *result_len = 0;
    byte* data = NULL;
    std::string tempData;

    if (command == NULL)
    {
        return SQLITE_ERROR;
    }

    SQLITE_Error errcode = command->m_Command->executeblob(tempData);

    if (errcode == SQLITE_OK)
    {
        if (!tempData.empty())
        {
            data = (byte*) nsl_malloc(tempData.size() * sizeof(byte));
            memcpy(data, tempData.c_str(), tempData.size());
            *result = data;
            *result_len = tempData.size();
        }
    }
    return errcode;
}
/* See header file for description. */
PAL_DB_Error PAL_DBExecuteReader (PAL_DBCommand* command,
                                          PAL_DBReader** rdr
                                          )
{
    if (command == NULL)
    {
        return SQLITE_ERROR;
    }

    PAL_DBReader* tempReader = (PAL_DBReader*)nsl_malloc(sizeof(PAL_DBReader));

    if (tempReader == NULL)
    {
        return SQLITE_NOMEM;
    }

    sqlite3_reader* reader = new sqlite3_reader();
    *reader = command->m_Command->executereader ();

    if (reader == NULL)
    {
        nsl_free(tempReader);
        return SQLITE_NOMEM;
    }
    else
    {
        tempReader->m_Reader = reader;
        *rdr = tempReader;
        return SQLITE_OK;
    }
}
/* See header file for description. */
PAL_DB_Error PAL_DBResetReader (PAL_DBReader* rdr)
{
    if (rdr != NULL)
    {
      return rdr->m_Reader->reset();
    }

    return SQLITE_OK;
}
PAL_DB_Error PAL_DBReaderRead (PAL_DBReader* rdr)
{
    if (rdr == NULL)
    {
        return SQLITE_ERROR;
    }
    else
    {
        return rdr->m_Reader->read();
    }
}
/* See header file for description. */
PAL_DB_Error PAL_DBReaderGetInt(PAL_DBReader* rdr,
                                        const char* colname,
                                        int* result)
{

    if (rdr == NULL)
    {
        return SQLITE_ERROR;
    }

    int port;
    SQLITE_Error errcode = rdr->m_Reader->getint (colname, port);
    *result = port;
    return errcode;
}
PAL_DB_Error PAL_DBReaderGetLong(PAL_DBReader* rdr,
                                         const char* colname,
                                         int64* result)
{
    if (rdr == NULL)
    {
        return SQLITE_ERROR;
    }

    int64 port;
    SQLITE_Error errcode = rdr->m_Reader->getint64(colname, port);
    *result = port;
    return errcode;
}
PAL_DB_Error PAL_DBReaderGetdouble(PAL_DBReader* rdr,
                                           const char* colname,
                                           double* result)
{
    if (rdr == NULL)
    {
        return SQLITE_ERROR;
    }

    double port;
    SQLITE_Error errcode = rdr->m_Reader->getdouble(colname, port);
    *result = port;
    return errcode;
}
PAL_DB_Error PAL_DBReaderGetstring(PAL_DBReader* rdr,
                                           const char* colname,
                                           char** result)
{
    if (rdr == NULL)
    {
        return SQLITE_ERROR;
    }

    std::string strTemp;

    SQLITE_Error errcode = rdr->m_Reader->getstring(colname, strTemp);

    if (errcode == SQLITE_OK)
    {
        char* resultTemp = nsl_strdup(strTemp.c_str());

        if (resultTemp != NULL)
        {
            *result = resultTemp;
        }
        else
        {
            *result = NULL;
            return SQLITE_NOMEM;
        }
    }
    return errcode;
}

PAL_DB_Error PAL_DBReaderGetblob(PAL_DBReader* rdr,
                                         const char* colname,
                                         byte** result,
                                         int* len)
{
    if (rdr == NULL)
    {
        return SQLITE_ERROR;
    }

    byte* data = NULL;
    std::string dataTemp;

    SQLITE_Error errcode = rdr->m_Reader->getblob(colname, dataTemp);

    if (errcode == SQLITE_OK)
    {
        if (!dataTemp.empty())
        {
            data = (byte*) nsl_malloc(dataTemp.size() * sizeof(byte));
            nsl_memcpy(data, dataTemp.c_str(), dataTemp.size());
            *result = data;
            *len = dataTemp.size();
        }
    }
    return errcode;
}
