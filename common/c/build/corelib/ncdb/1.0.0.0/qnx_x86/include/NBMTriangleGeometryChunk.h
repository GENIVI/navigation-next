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

@file     NBMTriangleGeometryChunk.h
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

#include "NBMChunk.h"
#include <vector>

using std::vector;

//Geometry Flags
// 	Vertex Type
// 	0x0 = Position Only
// 	0x1 = Position and Normal
// 	0x2 = Position and Texture Coordinate
// 	0x3 = Position and Normal and Texture Coordinate
//Array Size
// 	The Array Size is the length of each array Position, Normal, and TexCoord.

const char VTX_POSITION_ONLY = 0x0;
const char VTX_POSITION_NORMAL = 0x1;
const char VTX_POSITION_TEXTCOORD = 0x2;
const char VTX_POSITION_ALL = 0x3;

struct GeometryBody
{
	unsigned char  m_flag;
	unsigned short m_itemCount;

    // Values by half(float16), only use for NBMPrecision_half.
	NBMPosition*      m_position;
	NBMNormal*        m_normal;
	NBMTextureCord*   m_textureCord;

    // Values by float, only use for NBMPrecision_float.
    NBMRealPoint*     m_realPosition;
    NBMRealNormal*    m_realNormal;
    NBMRealTextCoord* m_realTextureCord;

    GeometryBody() : 
      m_flag(0), 
      m_itemCount(0), 
      m_position(NULL), 
      m_normal(NULL), 
      m_textureCord(NULL),
      m_realPosition(NULL),
      m_realNormal(NULL),
      m_realTextureCord(NULL),
      m_precision(NBMPrecision_half)
    {
    }
    ~GeometryBody()
    {
        Cleanup();
    }
	unsigned int HeaderLen()
	{
		return sizeof(m_flag) + sizeof(m_itemCount);
	}
	unsigned int size()
	{
		unsigned int Len = HeaderLen();

        unsigned int positionSize = 0;
        unsigned int normalSize = 0;
        unsigned int textcoordSize = 0;
        if (m_precision == NBMPrecision_float)
        {
            NBMRealPoint position;
            positionSize = position.size();
            NBMRealNormal normal;
            normalSize = normal.size();
            NBMRealTextCoord textureCord;
            textcoordSize = textureCord.size();            
        }
        else
        {
            NBMPosition position;
            positionSize = position.size();
            NBMNormal normal;
            normalSize = normal.size();
            NBMTextureCord textureCord;
            textcoordSize = textureCord.size();
        }

		Len += m_itemCount * positionSize;

		if (m_flag & VTX_POSITION_NORMAL)
		{			
			Len += m_itemCount * normalSize;
		}
		if (m_flag & VTX_POSITION_TEXTCOORD)
		{			
			Len += m_itemCount * textcoordSize;
		}
		return Len;
	}

	unsigned int ToBuffer(char* buf);
	void FromBuffer(char* buf);

    // Get/Set data precision
    void SetDataPrecision(NBMDataPrecision precision){ m_precision = precision; }
    NBMDataPrecision GetDataPrecision(){ return m_precision; }

private:
    void Cleanup();

    // Data precision control use NBMPosition/NBMNormal/NBMRealTextCoord or
    // NBMRealPoint/NBMRealNormal/NBMRealTextCoord values.
    NBMDataPrecision m_precision;
};

class NBMTriangleGeometryChunk : public NBMChunk
{
public:
	NBMTriangleGeometryChunk(NBMFileHeader * pFileHeader,bool bMake);

	virtual ~NBMTriangleGeometryChunk()
	{
	}

    // Set/Get data precision for GeometryBody,
    // Current we save the data precision by Chunk flag.
    // Add this to fix some small Statue(landmark) will lost people half body issue.
    void SetDataPrecision(NBMDataPrecision precision){ m_wFlag = (unsigned short)precision; }
    NBMDataPrecision GetDataPrecision(){ return (NBMDataPrecision)m_wFlag; }

	// for building
	NBMIndex SetData(vector<NBMTriangle>& vTri, NBMIndex* pIndex);

	// for landmark
	NBMIndex SetData(unsigned char* buf, unsigned int Len);

	bool GetData(NBMIndex uIndex, GeometryBody& gb);

	NBMIndex GetData(NBMIndex, unsigned char* buf);

	unsigned int GetOneItemLen(unsigned char* pBuf);
};
