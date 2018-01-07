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

@file     NBMChunk.h
@date     11/01/2011
@defgroup MOBIUS_NBM NavBuilder Binary Model API
@author   Sevnsson Sun

*/
/*
(C) Copyright 2012 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

#pragma once

#include "NBMTypes.h"
#include <map>
#include <vector>

#define NBM_MEMORY_INCREACE_SIZE	(0x00000001 << 19)	// 512K bytes for one increace 

#define NBM_BASEFLAG_COMPRESSED 0x0001		//whether compressed or not:  
#define NBM_BASEFLAG_EXONYM     0x0002      // whether ExonymString;
#define NBM_INVALIDATE_INDEX	0xFFFF	//
#define NBM_INVALIDATE_POS		0xFFFFFFFF	//
#define NBM_INVALIDATE_VALUE	0xFFFFFFFF

class NBMFileHeader;

// NBM chunk header defines
struct NBMChunkHeader
{
	NBMChunkHeader(unsigned int id, unsigned int size, unsigned short flags, unsigned short count)
	{
		m_identifier = id;
		m_size = size;
		m_flags = flags;
		m_count = count;
	}
	unsigned int m_identifier;
	unsigned int m_size;
	unsigned short m_flags;
	unsigned short m_count;

	unsigned int ToBuffer(unsigned char* buf);
};

// All NBM chunks base define
class NBMChunk
{
public:
	NBMChunk(NBMFileHeader * pFileHeader, bool bMake, unsigned int u32ChunkIdentifier);
	virtual ~NBMChunk(void);

	// To  control whether use the same index for same data content item.
	// only used by make processing ,  or it will return false;
	bool SetAvoidDuplication(bool bAvoid);

	// Get flag whether current chunk is compressed or not
	bool IsCompressed();

	// Set compressed flag for current chunk
	bool SetCompressed(bool bComp);

	// Clear All
	void ClearAll();

    // Get chunk identifier
    unsigned int GetChunkIdentifier();
    
	// Get Item Count
	unsigned int GetCount();

	// Get Chunk Stream size
	unsigned int GetChunkStreamSize();
	
	// Serialize One chunks data to Buffer: 
	// pszBufStart is the buffer point, uLenLimited is the buffer maxSize, 
	// uSuggestChunkSize is the total chunk size(include chunk size if they exist).
	// return the real buffer used
	unsigned int SerializeToBuffer(const char * pszBufStart, unsigned int uLenLimited, unsigned int uSuggestChunkSize = -1);

	// Serialize One chunks data From Buffer: 
	// pszBufStart is the buffer point, uLenLimited is the buffer maxSize;
	// return the total chunk size(include sub chunks size),
	// Need call GetChunkStreamSize() to get this chunks's real chunk size.
	unsigned int SerializeFromBuffer(const char * pszBufStart,unsigned int uLenLimited);

	//function can be used when make flag opened
	static unsigned int Hash(void * pVoid , unsigned int uLen);
 
    // Get/Set associated flag
    bool GetAssociated();
    void SetAssociated(bool isAssociated);    

protected:

	//get the Buffer by Index 
	bool GetItemBufByIndex(unsigned short uIndex,unsigned char*& pBuf,unsigned int& uLen);

	//Add Mem data into the Object's memory
	NBMIndex MemAdd(void * pVoid , unsigned int uLen);

	//One Object Item Len return a lenth 
	virtual unsigned int GetOneItemLen(unsigned char*) = 0;

private:

	//Judge Whether Exist. return the Exist index, NBM_INVALIDATE_INDEX means not Exist;
	NBMIndex IsExist(void * pVoid , unsigned int uLen);

	//Add mapExist 
	void AddExist(void * pVoid , unsigned int uLen , unsigned short uIndex);

	//Get Next Item 
	bool GetNextItemBufByPos(unsigned int& uIndexPos,unsigned char*& pBuf,unsigned int& uLen);

	//Get Memory Buffer by Position ; Warning , do not Edit this memory.
	unsigned char* GetBufByPos(unsigned int uIndexPos);

	//Get Item 
	bool GetItemBufByPos(unsigned int uIndexPos,unsigned char*& pBuf,unsigned int& uLen);

	//Is Index is in the Content
	bool IsPosAvailable(unsigned int uIndex);

	//Get the index 's pos
	unsigned int GetPos(unsigned short uIndex);

	//Set the Pos for index;
	unsigned short SetPos(unsigned int uPos);

protected:
	unsigned int m_u32ChunkIdentifier;	
	unsigned int m_u32ChunkSize;
	unsigned short m_wFlag;
	unsigned short m_u16Count;

	NBMFileHeader * m_pFileHeader;

private:
	//MemoryManagment
	unsigned char * m_pMem;
	unsigned int m_uMemCurWritePos;
	unsigned int m_uMemMaxLen;

	bool m_bMaking;
	bool m_bAvoidDuplication;
	std::map<unsigned int,unsigned short> m_mapExist;
	std::vector<unsigned int> m_vecIndexToPos;

    // Flag indicates whether current chunk is already be associated by another chunk,
    // One associated chunk can be associated ONCE by another dynamic chunk.
    bool m_isAssociated;
};
