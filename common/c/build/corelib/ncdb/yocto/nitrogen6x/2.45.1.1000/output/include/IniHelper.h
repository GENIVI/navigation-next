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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2014 by TeleCommunication Systems, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to TeleCommunication Systems, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of TeleCommunication Systems is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/
#pragma once
#ifndef NCDB_INIHELPER_H
#define NCDB_INIHELPER_H

#include "NcdbTypes.h"
#include "UtfString.h"
#include "AutoSharedPtr.h"
#include "AutoPtr.h"
#include "FileName.h"

namespace Ncdb {

class IniLabelMap;
class IniGroupMap;

//! Ini file reader API utility class
class IniHelper
{
public:
	IniHelper();
	~IniHelper(void);
	
	//! @brief Load the ini file.
	//! @details Load the ini file into a memory buffer and parse into a lookup tree.
	//! Typically you should call this function only once per ini file as the
	//! buffer allocation, file read, and parse take some time.
	//! @param[in] fullname The full path name of the file.
	//! @return 
	//!			NCDB_OK - Successfully opened and parsed the file.
	//!         NCDB_BAD_POINTER - Pointer passed in was NULL.
	//!         NCDB_READ_ERR - fread returned an error.
	//!         NCDB_OPEN_ERR - fopen returned an error.
	ReturnCode loadFile(const FileNameCharType *fullname);

	//! @brief Get the ini Value.
	//! @details Get the string indexed by the group and label.
	//! @param[in] group The ini file group [section]
	//! @param[in] label The ini variable name.
	//! @return The variable value.
	//! @return Returns NULL pointer if not found.
	const char* getValue(const char *group, const char *label);

	const char* getValueByHash(IniHashCode group, IniHashCode label);

	//! Get the ini Value.
	const char* getValueByGroupCode(IniHashCode groupCode, const char *label);

	//! Return the number of labels within this group.
	int getLabelCount(IniHashCode group);
	
	//! Get next iteration.
	const char* getValueByIndex(IniHashCode groupCode, int index, IniHashCode& labelCode);
	
	//! Compute the hash code.
	static IniHashCode hash(const char* text);

	//! Get the ini file name.
	FileName& getFileName(void) {return m_FullName;}

	//! Get the group hash code.
	//! @return true if success false otherwise.
	IniHashCode getGroupByIndex(int index);

	//! Return the number of groups within this ini file.
	int getGroupCount(void);

    //! Return last error string
    const UtfString& GetLastErrorString();

	bool getVersion(float& version) const;
    
    void UpdateMap(IniHashCode groupId, IniHashCode labelId, const char* value);

private:

	AutoSharedPtr<IniGroupMap> m_Map;
	AutoArrayPtr<char> m_Data, m_IncData;
	FileName m_FullName;
	int m_DataSize, m_IncDataSize;
	void parse(bool isIncFile = false, int start = 0);
	void parseIncFile();
    UtfString m_LastError;
	float m_Version;
	bool m_VersionValid;
};



class NCDB_API IniHelperHandle : public AutoSharedPtr<IniHelper>
{
public:
	IniHelperHandle(void)
	: AutoSharedPtr<IniHelper>()
	{
	}
	IniHelperHandle(IniHelper* config)
	: AutoSharedPtr<IniHelper>(config)
	{
	}
};



};
#endif

