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

@file     NBMMeshChunk.h
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
#include <string>
using std::string;

class NBMTriangleGeometryChunk;
class NBMPointChunk;

#define BLDG_DEFAULT_FONTSIZE 12
#define BLDG_DEFAULT_FONTIDX  2

struct MeshBody
{
	unsigned char     m_flag;
	NBMIndex  m_materialIdx;
	NBMIndex  m_labelIdx;
	NBMIndex  m_geometryIdx;
	unsigned short    m_count;
	NBMIndex* m_pIdx;

	unsigned int size();

	void FromBuffer(char* buf);
	unsigned int ToBuffer(char* buf);

    MeshBody()
    {
        m_flag = 0;
        m_materialIdx = NBM_INVALIDATE_INDEX;
        m_labelIdx = NBM_INVALIDATE_INDEX;
        m_geometryIdx = NBM_INVALIDATE_INDEX;
        m_count = 0;
        m_pIdx = NULL;
    }
};

struct BuildingBody
{
    string m_label;
	POINT2 m_centroid;
	vector<NBMTriangle> m_vTriangles;
};

struct LandMarkGeometry
{
	unsigned int vertexCount;
	unsigned int textureCoordinatesCount;
	unsigned int trianglesCount;

	vector<float> vecPosition;  //<x,y,z> ...
	vector<float> vecNormal;    //<x,y,z> ...
	vector<float> vecTexture;   //<s,t> ...
	vector<unsigned> vecTriangle; //<v1,vt1,vn1,v2,vt2,vn2,v3,vt3,vn3 ...>

	LandMarkGeometry()
	{
		vertexCount = textureCoordinatesCount = trianglesCount = 0;
	}
};

struct LandMarkScene 
{
	unsigned char Ver;
	unsigned char Flag;
	float XMax;
	float XMin;
	float YMax;
	float YMin;

	float anchorX;
	float anchorY;
	float csvX; //point from a landmark csv file from navteq
	float csvY;

    string label;
	POINT2 centroid;
	POINT2 antiAnchor;

	int isGroupBldg;
	unsigned int textureImagesCount;
	unsigned int geometriesCount;
    vector<string> vecTexture;

	LandMarkGeometry Geometry;
	TextureBitMapBody textureBitMapBody;
};

enum TextureType
{
	FILE_PNG = 0,
	FILE_BMP,
	FILE_TGA,
	FILE_JPG
};

struct TextureInfo
{
	TextureInfo()
	{
		memset(filename, 0, sizeof(filename));
		pImage = NULL;
		imageSize = 0;
		width = 0;
		height = 0;
		type = FILE_PNG;
	}
	~TextureInfo()
	{
		if (imageSize && pImage)
		{
			delete[] pImage;
		}
	}
	void SetImage(char* name, char* p, int size);

	short type;
	char filename[256];
	char* pImage;
	unsigned int imageSize;
	unsigned int width;
	unsigned int height;
};

struct LandMarkObj
{
	TextureInfo m_texture;
    string m_label;
	POINT2 m_centroid;
	vector<NBMDoubleTriangle> m_vTri;
};

//////////////////////////////////////////////////////////////////////////
class NBMMeshChunk : public NBMChunk
{
public:
	NBMMeshChunk(NBMFileHeader* pFileHeader, bool bMake)
      : NBMChunk(pFileHeader, bMake, NBM_CHUNCK_IDENTIFIER_MESH)
	{
	}
	virtual ~NBMMeshChunk()
	{
	}

	NBMIndex SetData(BuildingBody& buildingBody, NBMTriangleGeometryChunk* geometryChunk, NBMPointChunk* pointChunk);
	bool GetData(NBMIndex uIndex, MeshBody& meshBody, GeometryBody& geometryBody, NBMTriangleGeometryChunk* geometryChunk);
	bool GetData(NBMIndex uIndex, BuildingBody& buildingBody, NBMTriangleGeometryChunk* geometryChunk, NBMPointChunk* pointChunk);
    
    // Set raster tile as texture bitmap, it's a special landmark scene
    NBMIndex SetData(TextureBitMapBody& textureBitmapBody, NBMTriangleGeometryChunk* geometryChunk);

	NBMIndex SetData(LandMarkScene& landMarkScene, NBMTriangleGeometryChunk* geometryChunk, NBMPointChunk* pointChunk, bool useAnchor = true);
	bool GetData(NBMIndex uIndex, LandMarkScene& landMarkScene);
	bool GetData(NBMIndex uIndex, LandMarkObj& landMarkObj, NBMTriangleGeometryChunk* geometryChunk, NBMPointChunk* pointChunk);

	unsigned int GetOneItemLen(unsigned char* pBuf);
};
