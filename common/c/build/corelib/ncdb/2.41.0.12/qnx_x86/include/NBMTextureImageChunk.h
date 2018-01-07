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

@file     NBMTextureImage.h
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
#include "NBMChunk.h"

struct TextureImageBody
{
	unsigned char namesize;
	char namebuf[256];

	TextureImageBody()
	{
		namesize = 0;
		namebuf[0] = 0;
	}
	~TextureImageBody()
	{
	}
	unsigned int Size()
	{
		return sizeof(namesize) + namesize;
	}
	void SetName(const char* name)
	{
		namesize = (unsigned char)strlen(name);
		//assert namesize < sizeof(namebuf)
		memset(namebuf, 0, sizeof(namebuf));
		memcpy(namebuf, name, namesize);
	}
	void FromBuf(char* buf);
	unsigned int ToBuf(char* buf);
};

struct TextureBitMapBody
{
	unsigned char namesize;
	char namebuf[256];
	unsigned int filesize;
	char*  filebuf;

	TextureBitMapBody()
	{
		namesize = 0;
		namebuf[0] = 0;
		filesize = 0;
		filebuf  = NULL;
	}
	~TextureBitMapBody()
	{
		if (filebuf)
		{
			delete[] filebuf;
			filebuf = NULL;
		}
	}
	unsigned int Size(char* buf);
	unsigned int Size()
	{
		return sizeof(namesize) + namesize + sizeof(filesize) + filesize;
	}
	void SetName(const char* name)
	{
		namesize = (unsigned char)strlen(name);
		//assert namesize < sizeof(namebuf)
		memset(namebuf, 0, sizeof(namebuf));
		memcpy(namebuf, name, namesize);
	}
	void FromBuf(char* buf);
	unsigned int ToBuf(char* buf);

	unsigned int LoadFromFile(FILE* fp);
    unsigned int LoadBmpBuffer(char* bmpBuffer, unsigned int fileSize);
};

class NBMTextureImageChunk : public NBMChunk
{
public:
	NBMTextureImageChunk(NBMFileHeader * pFileHeader, bool bMake)
        :NBMChunk(pFileHeader, bMake, NBM_CHUNCK_IDENTIFIER_TEXTUREIMAGE)
	{
	}
	virtual ~NBMTextureImageChunk()
	{
	}

	NBMIndex SetData(const char* psz);
	bool GetData(NBMIndex uIndex, char* psz);
	unsigned int GetOneItemLen(unsigned char* pBuf);
};

class NBMTextureBitmapChunk : public NBMChunk
{
public:
	NBMTextureBitmapChunk(NBMFileHeader * pFileHeader, bool bMake)
        : NBMChunk(pFileHeader, bMake, NBM_CHUNCK_IDENTIFIER_TEXTUREBITMAP)
	{
	}
	virtual ~NBMTextureBitmapChunk()
	{
	}

	NBMIndex SetData(TextureBitMapBody& tbb);
	bool GetData(NBMIndex uIndex, TextureBitMapBody& tbb);
	unsigned int GetOneItemLen(unsigned char* pBuf);
};
