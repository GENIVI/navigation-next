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

@file     FileName.h
@date     05/13/2009
@defgroup MOBIUS_UTILS  Mobius utility classes 

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
/*! @{ */
#pragma once

#ifndef FILENAME_H_ONCE
#define FILENAME_H_ONCE

#include "NcdbTypes.h"

namespace Ncdb {

typedef char FileNameCharType;

class NCDB_API FileName
{
public:
	FileName(void);
	~FileName(void);
	
    //! Maximum file name length.
	static const int MaxName = 128;
	
    //! Maximum path length.
	static const int MaxPath = 256;
	
    //! Maximum full path with file name.
	static const int MaxPathAndName = MaxName + MaxPath;
	
    //! Return the full path with file name.
	const FileNameCharType* getPathAndName(void) const;
	
    //! Return the file name.
	const FileNameCharType* getName(void) const;
	
    //! Return the path.
	const FileNameCharType* getPath(void) const;
	
    //! Get the file name suffix below the period. like "txt", "cpp", etc.
	const FileNameCharType* getNameSuffix(void) const;
	
    //! Set the full path and file name.
	void setPathAndName(const FileNameCharType* str);
	
    //! Set the full path and file name.
	void setPathAndName(const FileNameCharType* path, const FileNameCharType* name);
	
    //! Set the file name.
	void setName(const FileNameCharType* name);
	
    //! Set the full path.
	void setPath(const FileNameCharType* path);
	
    //! Concatenate the relative path.
	void addPath(const FileNameCharType* path);

	//! Set the name and suffix.  prefix.suffix
	void setNameSuffix(const FileNameCharType* suffix);

	//! Set the file name prefix    prefix.
	void setNamePrefix(const FileNameCharType* prefix);

	//! Equal operator.
	bool operator == (class FileName const & fn) const ;

	//! Unequal operator.
	bool operator != (class FileName const & fn) const ;

    //bool Ncdb::FileName::operator ==(Ncdb::FileName &)

private:
	FileNameCharType m_FileName[MaxName+2];
	FileNameCharType m_FilePath[MaxPath+2];
	
    mutable FileNameCharType m_FilePathAndName[MaxPathAndName+2];

    void updatePathAndName();
};


};

#endif // FILENAME_H_ONCE

/*! @} */
