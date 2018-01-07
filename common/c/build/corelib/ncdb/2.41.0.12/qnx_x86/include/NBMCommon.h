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

@file     NBMCommon.h
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

#include "NBMTextChunk.h"
#include "NBMMaterialChunk.h"
#include "NBMObjectChunk.h"
#include "NBMTextureImageChunk.h"
#include "NBMTriangleGeometryChunk.h"
#include "NBMLayerChunk.h"
#include "NBMSceneChunk.h"
#include "NBMFileHeader.h"
#include "NBMVertexChunk.h"
#include "NBMMeshChunk.h"
#include "NBMRoadChunk.h"
#include "NBMRoad2Chunk.h"
#include "NBMPolyLineChunk.h"
#include "NBMVectorChunk.h"
#include "NBMAreaChunk.h"
#include "NBMTrackChunk.h"
#include "NBMTextPathShieldChunk.h"
#include "NBMTextPathArrowChunk.h"
#include "NBMPointChunk.h"
#include "NBMPOIChunk.h"
#include "NBMPinChunk.h"
#include "NBMIdChunk.h"
#include "NBMDrawOrderChunk.h"

class BufferConverter
{
public:
	BufferConverter(char* buf);

private:
	int m_offset;
	char* m_buf;

public:
	void push(unsigned char);
    void push(signed char);
	void push(unsigned short);
	void push(unsigned int);
	void push(float);
	void push(const char* buf, int len);

	void pop(unsigned char*);
    void pop(signed char*);
	void pop(unsigned short*);
	void pop(unsigned int*);
	void pop(float*);
	void pop(char* buf, int len);

	unsigned int BufLen()
	{
		return m_offset;
	}
};

struct LandmarkInfo
{
	string landmarkName; //src file name
	string label;		 //for display name
	float anchorLat;
	float anchorLon;
	float centerLat;
	float centerLon;
	float antiAnchorLat;
	float antiAnchorLon;
	float anchorLatFromCSV;
	float anchorLonFromCSV;
};

