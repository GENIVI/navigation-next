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
    @file     sqlite3_command.cpp
    @date     3/18/2009
    @defgroup SQLITE Clock Functions
    SQLite command support.
*/
/*
    See file description in header file.
    (C) Copyright 2012 by TeleCommunication Systems, Inc.
    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include <sqlite3.h>
#include "sqlite3x.h"
#include <algorithm>
#include <string.h>
#include <ctype.h>
#include "palstdlib.h"

namespace sqlite3x
{
    sqlite3_command::sqlite3_command(sqlite3_connection &con)
        : con(con)
        , stmt(NULL)
        , refs(0)
        , argc(0)
        , tail(NULL)
        , column_map(NULL)
        , column_map16(NULL)
		{
		}

    sqlite3_command::~sqlite3_command()
	{
        if ( tail )
        {
            delete [] tail;
            tail = NULL;
        }
        if (column_map)
        {
            delete column_map;
            column_map = NULL;
        }
        if (column_map16)
        {
            delete column_map16;
            column_map16 =  NULL;
        }
        sqlite3_finalize(stmt);
    }

	SQLITE_Error sqlite3_command::prepare(const char *sql)
	{
		const char* pztail = NULL;
		SQLITE_Error sqlite_result = SQLITE_OK;
		sqlite_result = sqlite3_prepare(con.db, sql, -1, &stmt, &pztail);
		if (sqlite_result != SQLITE_OK || pztail == NULL)
		{
			return sqlite_result;
		}

		int len = nsl_strlen(pztail);
		if(tail)
		{
			delete [] tail;
			tail = NULL;
		}
		tail = new char[len + 1];
		if(!tail)
		{
			return SQLITE_NOMEM;
		}
		nsl_memset(tail, 0, len + 1);
		nsl_strncpy (tail, pztail, len);
		argc=sqlite3_column_count(stmt);
		// build a hastable with column names and indexes to use when retrieving data from the reader

		if (column_map)
		{
			delete column_map;
			column_map = NULL;
		}
		column_map = new std::map<std::string, int>();
		if (!column_map)
		{
			return SQLITE_NOMEM;
		}
		for (int i = 0; i < argc; i++)
		{
			std::string s = sqlite3_column_name(stmt, i);
			to_lower (s);
			column_map->insert(std::pair<std::string, int>(s, i));
		}
		return sqlite_result;
	}

	SQLITE_Error sqlite3_command::prepare(const std::string &sql)
	{
		const char* pztail = NULL;
		SQLITE_Error sqlite_result = sqlite3_prepare(con.db, sql.data(), (int)sql.length(), &stmt, &pztail);
		if (sqlite_result != SQLITE_OK || pztail == NULL)
		{
			return sqlite_result;
		}

		int len = nsl_strlen(pztail);
		if(tail)
		{
			delete [] tail;
			tail = NULL;
		}
		tail = new char[len + 1];
		if(!tail)
		{
			return SQLITE_NOMEM;
		}
		nsl_memset(tail, 0, len + 1);
		nsl_strncpy (tail, pztail, len);

		argc=sqlite3_column_count(stmt);
		// build a hastable with column names and indexes to use when retrieving data from the reader
		if (column_map)
		{
			delete column_map;
			column_map = NULL;
		}
		column_map = new std::map<std::string, int>();
		if (!column_map)
		{
			return SQLITE_NOMEM;
		}
		for (int i = 0; i < argc; i++)
		{
			std::string s = sqlite3_column_name(stmt, i);
			to_lower (s);
			column_map->insert(std::pair<std::string, int>(s, i));
		}
		return sqlite_result;
	}

	SQLITE_Error sqlite3_command::bind(int index)
	{
		return sqlite3_bind_null(stmt, index);
	}

	SQLITE_Error sqlite3_command::bind(int index, int data)
	{
		return sqlite3_bind_int(stmt, index, data);
	}

	SQLITE_Error sqlite3_command::bind(int index, long long data)
	{
		return sqlite3_bind_int64(stmt, index, data);
	}

	SQLITE_Error sqlite3_command::bind(int index, double data)
	{
		return sqlite3_bind_double(stmt, index, data);
	}

	SQLITE_Error sqlite3_command::bind(int index, const char *data, int datalen)
	{
		return sqlite3_bind_text(stmt, index, data, datalen, SQLITE_TRANSIENT);
	}

	SQLITE_Error sqlite3_command::bind(int index, const void *data, int datalen)
	{
		return sqlite3_bind_blob(stmt, index, data, datalen, SQLITE_TRANSIENT);
	}

	SQLITE_Error sqlite3_command::bind(int index, const std::string &data)
	{
		return sqlite3_bind_text(stmt, index, data.data(), (int)data.length(), SQLITE_TRANSIENT);
	}

	sqlite3_reader sqlite3_command::executereader()
	{
		return sqlite3_reader(this);
	}

	SQLITE_Error sqlite3_command::executenonquery()
	{
		executereader().read();
		return SQLITE_OK;
	}

	SQLITE_Error sqlite3_command::executeint(int& result)
	{
		sqlite3_reader reader = executereader();
		SQLITE_Error errcode = reader.read();
		if (errcode == SQLITE_ROW)
		{
			return reader.getint(0, result);
		}
		return errcode;
	}

	SQLITE_Error sqlite3_command::executeint64(long long& result)
	{
		sqlite3_reader reader = executereader();
		SQLITE_Error errcode = reader.read();
		if (errcode == SQLITE_ROW)
		{
			return reader.getint64(0, result);
		}
		return errcode;
	}

	SQLITE_Error sqlite3_command::executedouble(double& result)
	{
		sqlite3_reader reader = executereader();
		SQLITE_Error errcode = reader.read();
		if (errcode == SQLITE_ROW)
		{
			return reader.getdouble(0, result);
		}
		return errcode;
	}

	SQLITE_Error sqlite3_command::executestring(std::string& result)
	{
		sqlite3_reader reader = executereader();
		SQLITE_Error errcode = reader.read();
		if (errcode == SQLITE_ROW)
		{
			return reader.getstring(0, result);
		}
		return errcode;
	}

	SQLITE_Error sqlite3_command::executestring16(wchar_t* result,int size)
	{
		sqlite3_reader reader = executereader();
		SQLITE_Error errcode = reader.read();
		if (errcode == SQLITE_ROW)
		{
			return reader.getstring16(0, result, size);
		}
		return errcode;
	}

	SQLITE_Error sqlite3_command::executeblob(std::string& result)
	{
		sqlite3_reader reader = executereader();
		SQLITE_Error errcode = reader.read();
		if (errcode == SQLITE_ROW)
		{
			return reader.getblob(0, result);
		}
		return errcode;
	}

	int sqlite3_command::get_parameter_index (const char* name)
	{
		return sqlite3_bind_parameter_index (stmt, name);
	}

	/// Map column name to an index
	int sqlite3_command::map_column_index (std::string colname)
	{
		if (column_map == NULL) return -1;
		std::map<std::string, int>::iterator iter;
		to_lower (colname);
		if ((iter = column_map->find (colname)) == column_map->end()) return -1;
		return iter->second;
	}

	int sqlite3_command::map_column_index16 (std::wstring colname)
	{
		if (column_map16 == NULL) return -1;
		std::map<std::wstring, int>::iterator iter;
		if ((iter = column_map16->find (colname)) == column_map16->end()) return -1;
		return iter->second;
	}

	sqlite3_connection& sqlite3_command::get_connection ()
	{
		return con;
	}

	SQLITE_Error sqlite3_command::stepnext()
	{
		const char* pztail = NULL;
		int sqlite_result = SQLITE_OK;
		if (tail)
		{
			sqlite3_finalize(stmt);
			sqlite_result = sqlite3_prepare(con.db, (const char*)tail, -1, &stmt, &pztail);
			if (sqlite_result != SQLITE_OK || pztail == NULL)
			{
				return sqlite_result;
			}

			int len = nsl_strlen(pztail);

			delete [] tail;
			tail = NULL;

			tail = new char[len + 1];
			if(!tail)
			{
				return SQLITE_NOMEM;
			}
			nsl_memset(tail, 0, len + 1);
			nsl_strncpy (tail, pztail, len);

			argc = sqlite3_column_count(stmt);
			// build a hastable with column names and indexes to use when retrieving data from the reader
			if (column_map)
			{
				delete column_map;
				column_map = NULL;
			}
			column_map = new std::map<std::string, int>();
			if (!column_map)
			{
				return SQLITE_NOMEM;
			}
			for (int i = 0; i < argc; i++)
			{
				std::string s = sqlite3_column_name(stmt, i);
				to_lower (s);
				column_map->insert(std::pair<std::string, int>(s, i));
			}
		}

		return sqlite_result;
	}

	/// Convert string to lower case
	void sqlite3_command::to_lower(std::string& s)
	{
		std::string::iterator pos;
		for (pos= s.begin(); pos != s.end(); ++pos)
		{
			char a = *pos;
			if (isalpha (a))
			*pos = tolower(a);
		}
	}

}
