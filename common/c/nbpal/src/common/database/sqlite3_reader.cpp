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
 
 @file sqlite3_reader.cpp
 @date 9/13/10
 
 SQLite reader
 
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
#include <wchar.h>
#include "sqlite3x.h"

namespace sqlite3x {

sqlite3_reader::sqlite3_reader() : cmd(NULL) {}

sqlite3_reader::sqlite3_reader(const sqlite3_reader &copy) : cmd(copy.cmd) {
	if(this->cmd) ++this->cmd->refs;	
}

sqlite3_reader::sqlite3_reader(sqlite3_command *cmd) : cmd(cmd) {
	++cmd->refs;
}

sqlite3_reader::~sqlite3_reader() {
	this->close();
}

sqlite3_reader& sqlite3_reader::operator=(const sqlite3_reader &copy) {
	this->close();

	this->cmd=copy.cmd;
	if(this->cmd) ++this->cmd->refs;

	return *this;
}

SQLITE_Error sqlite3_reader::read() {
	if(!this->cmd) return SQLITE_MISUSE;

	return sqlite3_step(this->cmd->stmt);
}

SQLITE_Error sqlite3_reader::reset() {
    if(!this->cmd) return SQLITE_MISUSE;

	return sqlite3_reset(this->cmd->stmt);
}

void sqlite3_reader::close() {
	if(this->cmd) {
		if(--this->cmd->refs==0) sqlite3_reset(this->cmd->stmt);
		this->cmd=NULL;
	}
}

SQLITE_Error sqlite3_reader::getint(int index, int& result) {
	if(!this->cmd) return SQLITE_MISUSE;
	if((index)>(this->cmd->argc-1)) return SQLITE_RANGE;
	result = sqlite3_column_int(this->cmd->stmt, index);
    return SQLITE_OK;
}

SQLITE_Error sqlite3_reader::getint64(int index, long long& result) {
	if(!this->cmd) return SQLITE_MISUSE;
	if((index)>(this->cmd->argc-1)) return SQLITE_RANGE;
	result = sqlite3_column_int64(this->cmd->stmt, index);
    return SQLITE_OK;
}

SQLITE_Error sqlite3_reader::getdouble(int index, double& result) {
    if(!this->cmd) return SQLITE_MISUSE;
	if((index)>(this->cmd->argc-1)) return SQLITE_RANGE;
	result = sqlite3_column_double(this->cmd->stmt, index);
    return SQLITE_OK;
}

SQLITE_Error sqlite3_reader::getstring(int index, std::string& result) {
	if(!this->cmd) return SQLITE_MISUSE;
	if((index)>(this->cmd->argc-1)) return SQLITE_RANGE;	
    result = std::string((const char*)sqlite3_column_text(this->cmd->stmt, index), sqlite3_column_bytes(this->cmd->stmt, index));
    return SQLITE_OK;
}
    
//SQLITE_Error sqlite3_reader::getstring16(int index, std::wstring& result) {
//
//	if(!this->cmd) return SQLITE_MISUSE;
//
//	if((index)>(this->cmd->argc-1)) return SQLITE_RANGE;
//
//    result = std::wstring((const wchar_t*)sqlite3_column_text16(this->cmd->stmt, index), sqlite3_column_bytes16(this->cmd->stmt, index)/2);
//
//    return SQLITE_OK;
//
//}

SQLITE_Error sqlite3_reader::getstring16(int /*index*/, wchar_t* /*result*/, int /*result_len*/) {
    return SQLITE_ERROR;
	//if(!this->cmd) return SQLITE_MISUSE;
	//if((index)>(this->cmd->argc-1)) return SQLITE_RANGE;
    //int src_len = sqlite3_column_bytes16(this->cmd->stmt, index)/2;
    //wcsncpy_s (result, result_len, (const wchar_t*)sqlite3_column_text16(this->cmd->stmt, index), src_len);
    //return SQLITE_ERROR;
}

SQLITE_Error sqlite3_reader::getblob(int index, std::string& result) {
	if(!this->cmd) return SQLITE_MISUSE;
	if((index)>(this->cmd->argc-1)) return SQLITE_RANGE;
	const char* data = (const char*)sqlite3_column_blob(this->cmd->stmt, index);	
    if (data == NULL)
        return SQLITE_DONE;
    
    result = std::string(data, sqlite3_column_bytes(this->cmd->stmt, index));
    return SQLITE_OK;
    
}

SQLITE_Error sqlite3_reader::getcolname(int index, std::string& result) {
	if(!this->cmd) return SQLITE_MISUSE;
	if((index)>(this->cmd->argc-1)) return SQLITE_RANGE;
	result = sqlite3_column_name(this->cmd->stmt, index);
    return SQLITE_OK;
}

//SQLITE_Error sqlite3_reader::getcolname16(int index, std::wstring& result) {
//
//	if(!this->cmd) return SQLITE_MISUSE;
//
//	if((index)>(this->cmd->argc-1)) return SQLITE_RANGE;
//
//	result = (const wchar_t*)sqlite3_column_name16(this->cmd->stmt, index);
//
//    return SQLITE_OK;
//
//}

SQLITE_Error sqlite3_reader::getcolindex(std::string colname, int& index) {	
	if(!this->cmd) return SQLITE_MISUSE;
    if(((index = (this->cmd->map_column_index(colname))) == -1) ||
	    ((index)>(this->cmd->argc-1)))
        return SQLITE_RANGE;
	return SQLITE_OK;
}

SQLITE_Error sqlite3_reader::getcolindex16(std::wstring colname, int& index) {
	if(!this->cmd) return SQLITE_MISUSE;
	if(((index = (this->cmd->map_column_index16(colname))) == -1) ||
	    ((index)>(this->cmd->argc-1)))
        return SQLITE_RANGE;
	return SQLITE_OK;
}

//get columndata by name

SQLITE_Error sqlite3_reader::getint(std::string colname, int& result) {		
    if(!this->cmd) return SQLITE_MISUSE;
    int index = -1;
	SQLITE_Error sqlite_result = getcolindex (colname, index);

    if (sqlite_result == SQLITE_OK)
    {
	    result = sqlite3_column_int(this->cmd->stmt, index);
    }
    return sqlite_result;
}

SQLITE_Error sqlite3_reader::getint64(std::string colname, long long& result) {
	if(!this->cmd) return SQLITE_MISUSE;
	int index = -1;
    SQLITE_Error sqlite_result = getcolindex (colname, index);
    if (sqlite_result == SQLITE_OK)
    {
	    result = sqlite3_column_int64(this->cmd->stmt, index);
    }
    return sqlite_result;
}

SQLITE_Error sqlite3_reader::getdouble(std::string colname, double& result) {
	if(!this->cmd) return SQLITE_MISUSE;
	int index = -1;
    SQLITE_Error sqlite_result = getcolindex (colname, index);
    if (sqlite_result == SQLITE_OK)
    {
	    result = sqlite3_column_double(this->cmd->stmt, index);
    }
    return sqlite_result;
}

SQLITE_Error sqlite3_reader::getstring(std::string colname, std::string& result) {
	if(!this->cmd) return SQLITE_MISUSE;
	int index = -1;
    SQLITE_Error sqlite_result = getcolindex (colname, index);
	const char* s = (const char*)sqlite3_column_text(this->cmd->stmt, index);
	if (s == NULL)
    {
        result = std::string("");
    }
    else
    {
	    result = std::string(s, sqlite3_column_bytes(this->cmd->stmt, index));
    }
    return sqlite_result;
}

//SQLITE_Error sqlite3_reader::getstring16(std::string colname, std::wstring& result) {
//
//	if(!this->cmd) return SQLITE_MISUSE;
//
//    int index = -1;
//
//	SQLITE_Error sqlite_result = getcolindex (colname, index);
//
//	const wchar_t* s = (const wchar_t*)sqlite3_column_text16(this->cmd->stmt, index);
//
//	if ( s == NULL)
//
//    {
//
//        result = std::wstring(L"");
//
//    }
//
//    else
//
//    {
//
//	    result = std::wstring(s, sqlite3_column_bytes16(this->cmd->stmt, index)/2);
//
//    }
//
//    return sqlite_result;
//
//}

SQLITE_Error sqlite3_reader::getstring16(std::string colname, wchar_t* result, int result_len) {
    
    return SQLITE_ERROR;
    
    // TODOSQLITE_ERROR
	if(!this->cmd || !result) return SQLITE_MISUSE;
    int index = -1;
	SQLITE_Error sqlite_result = getcolindex (colname, index);
	const wchar_t* s = (const wchar_t*)sqlite3_column_text16(this->cmd->stmt, index);
	if ( s == NULL)
    {
        wcscpy (result, L"");
    }
    else
    {        
        int src_len = sqlite3_column_bytes16(this->cmd->stmt, index)/2;        
        if (src_len >= result_len)
        {
            //leave room for null terminator
            src_len = result_len-1;
        }
        //wcsncpy_s (result, result_len, s, src_len);
    }
    return sqlite_result;
}

SQLITE_Error sqlite3_reader::getblob(std::string colname, std::string& result) {
	if(!this->cmd) return SQLITE_MISUSE;
	int index = -1;
    SQLITE_Error sqlite_result = getcolindex (colname, index);
	const char* s = (const char*)sqlite3_column_blob(this->cmd->stmt, index);
	if (s == NULL)		
    {
        result = std::string("");
    }
    else
    {
	    result = std::string(s, sqlite3_column_bytes(this->cmd->stmt, index));
    }
    return sqlite_result;
}

}
