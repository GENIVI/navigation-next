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
 
 @file sqlite3x.h
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

#ifndef __SQLITE3X_H__
#define __SQLITE3X_H__

#include <string>
#include <map>
#include "paltypes.h"

typedef int SQLITE_Error;

namespace sqlite3x {
	class sqlite3_connection {
	private:
		friend class sqlite3_command;
		friend class database_error;

		struct sqlite3 *db;

	public:
		sqlite3_connection();
		sqlite3_connection(const char *db);		
		~sqlite3_connection();

		SQLITE_Error open(const char *db);		
		SQLITE_Error close();

		SQLITE_Error insertid(int64& id);
		SQLITE_Error setbusytimeout(int ms);

		SQLITE_Error executenonquery(const char *sql);		
		SQLITE_Error executenonquery(const std::string &sql);		

		SQLITE_Error executeint(const char *sql, int& result);		
		SQLITE_Error executeint(const std::string &sql, int& result);		

		SQLITE_Error executeint64(const char *sql, long long& result);		
		SQLITE_Error executeint64(const std::string &sql, long long& result);		

		SQLITE_Error executedouble(const char *sql, double& result);
		SQLITE_Error executedouble(const std::string &sql, double& result);
		
        SQLITE_Error executestring(const char *sql, std::string& result);
		SQLITE_Error executestring(const std::string &sql, std::string& result);
		
        SQLITE_Error executeblob(const char *sql, std::string& result);
		SQLITE_Error executeblob(const std::string &sql, std::string& result);

        SQLITE_Error execute(const std::string &sql);
		const char* getlasterror ();
	};

	class sqlite3_transaction {
	private:
		sqlite3_connection &con;
		bool intrans;

	public:
		sqlite3_transaction(sqlite3_connection &con, bool start=true);
		~sqlite3_transaction();

		void begin();
		void commit();
		void rollback();
	};

    struct sqlite3_column_item
    {
        std::string name;
        int index;
    };
    
    class sqlite3_reader;
    class sqlite3_command {
	private:
		friend class sqlite3_reader;

		sqlite3_connection &con;
		struct sqlite3_stmt *stmt;
		unsigned int refs;
		int argc;
		char *tail;

		/// CR - adding hashtable to store column names and indexes
		/// this will be used to access the reader by column name. 
		std::map<std::string, int>* column_map;
		std::map<std::wstring, int>* column_map16;
        
	public:
		sqlite3_command(sqlite3_connection &con);		
		~sqlite3_command();

        SQLITE_Error prepare(const char *sql);
        SQLITE_Error prepare(const std::string& sql);

		SQLITE_Error bind(int index);
		SQLITE_Error bind(int index, int data);
		SQLITE_Error bind(int index, long long data);
		SQLITE_Error bind(int index, double data);
		SQLITE_Error bind(int index, const char *data, int datalen);
		SQLITE_Error bind(int index, const wchar_t *data, int datalen);
		SQLITE_Error bind(int index, const void *data, int datalen);
		SQLITE_Error bind(int index, const std::string &data);
		SQLITE_Error bind(int index, const std::wstring &data);

		sqlite3_reader executereader();
		SQLITE_Error executenonquery();
		SQLITE_Error executeint(int& result);
		SQLITE_Error executeint64(long long& result);
		SQLITE_Error executedouble(double& result);
        SQLITE_Error executestring(std::string& result);        
        SQLITE_Error executestring16(wchar_t* result,int size);
        SQLITE_Error executeblob(std::string& result);
		int get_parameter_index (const char* name);
		sqlite3_connection& get_connection ();		
		SQLITE_Error stepnext();
        SQLITE_Error execute(const char* sql);

	private:
		int map_column_index (std::string col);
		int map_column_index16 (std::wstring colname);
		void to_lower(std::string& s);
	};

	class sqlite3_reader {
	private:
		friend class sqlite3_command;

		sqlite3_command *cmd;

		sqlite3_reader(sqlite3_command *cmd);

	private:
		SQLITE_Error getcolindex(std::string colname, int& index);
		SQLITE_Error getcolindex16(std::wstring colname, int& index);

	public:
		sqlite3_reader();
		sqlite3_reader(const sqlite3_reader &copy);
		~sqlite3_reader();

		sqlite3_reader& operator=(const sqlite3_reader &copy);

		SQLITE_Error read();
		SQLITE_Error reset();
		void close();

		SQLITE_Error getint(int index, int& result);
		SQLITE_Error getint64(int index, long long& result);
		SQLITE_Error getdouble(int index, double& result);
        SQLITE_Error getstring(int index, std::string& result);        
        SQLITE_Error getstring16(int index, std::wstring& result);
        SQLITE_Error getstring16(int index, wchar_t* result, int result_len);
        SQLITE_Error getblob(int index, std::string& result);

        SQLITE_Error getcolname(int index, std::string& result);
        SQLITE_Error getcolname16(int index, std::wstring& result);

		SQLITE_Error getint(std::string colname, int& result);
		SQLITE_Error getint64(std::string colname, long long& result);
		SQLITE_Error getdouble(std::string colname, double& result);
        SQLITE_Error getstring(std::string colname, std::string& result);
        SQLITE_Error getstring16(std::string colname, std::wstring& result);
        SQLITE_Error getstring16(std::string colname, wchar_t* result, int result_len);
        SQLITE_Error getblob(std::string colname, std::string& result);
	};
}

#endif
