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

@file     NBMTypes.h
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char Char8_t;						//				1	8 bit ascii character
typedef signed   char Int8_t;             //              1   8 bit signed integer
typedef unsigned char Uint8_t;			//				1	8 bit unsigned integer
typedef unsigned char Byte_t;
typedef unsigned short	Uint16_t;			//				2	16 bit unsigned integer
typedef unsigned int	Uint32_t;			//				4	32 bit unsigned integer
typedef unsigned short Float16_t;			//				

typedef unsigned short  NBMIndex;
typedef unsigned short  NBMFloat3;

typedef struct pint_2
{
    unsigned int data[2];
}pint2;

struct pfix_double
{
    double m_lat;
    double m_lon;
};

#define NBM_FILE_IDENTIFIER_FILEHEADER	*(unsigned int*)"NBMF"
#define NBM_FILE_IDENTIFIER_DATATAIL    *(unsigned int*)"TAIL"

typedef enum
{
	NBM_CHUNCK_IDENTIFIER_INVALID = -1,		// Invalid ID

    NBM_CHUNCK_IDENTIFIER_LAYERS = 0,		//"LAYS", layer chunk as the first chunk since version 23

	NBM_CHUNCK_IDENTIFIER_TEXTUREIMAGE,	    //"IMGS"
	NBM_CHUNCK_IDENTIFIER_TEXTUREBITMAP,	//"BMAP"
	NBM_CHUNCK_IDENTIFIER_MATERIALS,		//"MTRL"
	NBM_CHUNCK_IDENTIFIER_TEXT,				//"TEXT"
	NBM_CHUNCK_IDENTIFIER_VERTEX,			//"VRTX"
	NBM_CHUNCK_IDENTIFIER_TRIANGLEGEOMETRY,	//"GEOM"
	NBM_CHUNCK_IDENTIFIER_MESH,				//"MESH"
	NBM_CHUNCK_IDENTIFIER_SPLINEGEOMETRY,	//"SPLN"
	NBM_CHUNCK_IDENTIFIER_TERRAINOBJECT,	//"TRRN"
	NBM_CHUNCK_IDENTIFIER_EXTERNALOBJECT,	//"OBJX"
	NBM_CHUNCK_IDENTIFIER_OBJECT,			//"OBJS"
	NBM_CHUNCK_IDENTIFIER_SCENE,			//"SEEN"

	NBM_CHUNCK_IDENTIFIER_POLYLINE,			//"PLIN"
	NBM_CHUNCK_IDENTIFIER_ROAD,				//"ROAD"
	NBM_CHUNCK_IDENTIFIER_AREA,				//"AREA"
	NBM_CHUNCK_IDENTIFIER_POINTS,			//"PNTS"
	NBM_CHUNCK_IDENTIFIER_TRACK,			//"TPTH"
	NBM_CHUNCK_IDENTIFIER_TEXTPATHSHIELD,	//"TPSH", text path shield sub chunk
	NBM_CHUNCK_IDENTIFIER_TEXTPATHARROW,	//"TPAR", text path arrow sub chunk
	NBM_CHUNCK_IDENTIFIER_VECTOR,			//"VCTR"
	NBM_CHUNCK_IDENTIFIER_POIS,				//"POIS"
    NBM_CHUNCK_IDENTIFIER_GPIN,				//"PINS"
	NBM_CHUNCK_IDENTIFIER_EIDS,				//"EIDS"
    NBM_CHUNCK_IDENTIFIER_DRAWORDER,
	NBM_CHUNCK_IDENTIFIER_ROAD2,				//"ROAD2"
	// DO NOT make this order change .  if any new type insert . please ask Sevnsson to do it. 
	NBM_CHUNK_TYPE_NUM
}NBMChunkID;

typedef enum
{
    NBM_NO_ERROR = 0,
    NBM_NO_DATA = 1,
    NBM_NO_MEMORY = 2,
}NbmErrorCode;

struct NBMChunkType
{
	unsigned int nType;
	const char* pType;
};

const char*  NBMChunkID2String(unsigned int);
const unsigned int NBMChunkString2ID(const char*);

unsigned short NBM_Float2Half(float Value);
float NBM_Half2Float(unsigned short Value);

unsigned short NBM_Float2Half2(float Value, float AbsThreshold);
float NBM_Half22Float(unsigned short Value, float AbsThreshold);

#define NBM_FILE_MAJOR_VERSTION	23
#define NBM_FILE_MINOR_VERSION  5       // Geometry chunk becomes a dynamic chunk now

#define NBM_FILE_MAJOR_VERSTION_CUR 24
#define NBM_FILE_MINOR_VERSION_CUR  0

#define NBM_ROAD_CHUNK_VERSION 2

#define NBM_FILE_REFNAME_BASE				"BASE"
#define NBM_FILE_REFNAME_ROAD				"ROAD"
#define NBM_FILE_REFNAME_AREA				"AREA"
#define NBM_FILE_REFNAME_UNTEXTUREBUIDING	"3DUT"
#define NBM_FILE_REFNAME_LANDMARK			"3DLM"
#define NBM_FILE_REFNAME_POIS				"POIS"
#define NBM_FILE_REFNAME_ROUTE				"RUTE"
#define NBM_FILE_REFNAME_WL					"WLLP"
#define NBM_FILE_REFNAME_RL					"RLLP"
#define NBM_FILE_REFNAME_RASTERTILE			"RAST"
#define NBM_FILE_REFNAME_LABELTILE			"LBLT"
#define NBM_FILE_REFNAME_B2DTILE			"B2DT"
#define NBM_FILE_REFNAME_POI1TILE			"POI0"
#define NBM_FILE_REFNAME_POI2TILE			"POI1"
#define NBM_FILE_REFNAME_SATELLITE			"SATE"
#define NBM_FILE_REFNAME_MATERIALS          "MATS"
#define NBM_FILE_REFNAME_LBLT				"%02dLB"
#define NBM_FILE_REFNAME_RAST_D				"%02dRD"
#define NBM_FILE_REFNAME_RAST_N				"%02dRN"

#define NBM_LAYER_ID_INVALID            0xFFFF
#define NBM_LAYER_ID_ASSOCIATED			0
#define NBM_LAYER_ID_ROAD				1
#define NBM_LAYER_ID_AREA				2
#define NBM_LAYER_ID_UNTEXTUREONLY		3
#define NBM_LAYER_ID_UNTEXTURELANDMARK	4
#define NBM_LAYER_ID_LANDMARK			5
#define NBM_LAYER_ID_RASTER             6
//id 6 for LAYER_ID_ROAD_AREA //need to check
#define NBM_LAYER_ID_POIS				7
#define NBM_LAYER_ID_ROUTE				8
#define NBM_LAYER_ID_LABELPOINT         9
#define NBM_LAYER_ID_BASE				NBM_LAYER_ID_ROAD
#define NBM_LAYER_ID_SATELLITE          10
#define NBM_LAYER_ID_STATICPOI          11
#define NBM_LAYER_ID_MATERIAL           12
namespace NbmApi
{
	struct TileBuf
	{
		unsigned int tileLen;   /* tile buffer's length*/
		char* tBuf;             /* tile buffer*/
		TileBuf():tileLen(0),tBuf(NULL){}
		TileBuf(unsigned int len,const char* buf):tileLen(len)
		{
			tBuf = new char[len];
			memcpy(tBuf,buf,len);
		}
		~TileBuf(){ReleaseTileBuf();}
		TileBuf(const TileBuf& rhs)
		{
			tileLen = rhs.tileLen;
			tBuf = new char[tileLen];
			memcpy(tBuf,rhs.tBuf,tileLen);
		}
		TileBuf& operator=(const TileBuf& rhs)
		{
			if(this==&rhs)
				return *this;
			tileLen = rhs.tileLen;
			if(tBuf)
				delete[] tBuf;
			tBuf = new char[tileLen];
			memcpy(tBuf,rhs.tBuf,tileLen);
			return *this;
		}
		void ReleaseTileBuf()
		{
			tileLen = 0;
			if(tBuf)
			{
				delete []tBuf;
				tBuf = NULL;
			}
		}
	};
}
struct NBMRealPoint
{
	float x;
	float y;
	float z;

    unsigned int size()
    {
        return sizeof(x) + sizeof(y) + sizeof(z);
    }
};

struct NBMRealNormal
{
	float x;
	float y;
	float z;

    unsigned int size()
    {
        return sizeof(x) + sizeof(y) + sizeof(z);
    }
};

struct NBMRealTextCoord
{
	float x;
	float y;

    unsigned int size()
    {
        return sizeof(x) + sizeof(y);
    }
};

//
struct NBMDoublePoint
{
    double x;
    double y;
    double z;

    unsigned int size()
    {
        return sizeof(x) + sizeof(y) + sizeof(z);
    }
};

struct NBMDoubleNormal
{
    double x;
    double y;
    double z;

    unsigned int size()
    {
        return sizeof(x) + sizeof(y) + sizeof(z);
    }
};

struct NBMDoubleTextCoord
{
    double x;
    double y;

    unsigned int size()
    {
        return sizeof(x) + sizeof(y);
    }
};
//

struct NBMTriangle
{
	NBMRealPoint Point[3];
};

struct NBMRealTriangle
{
	NBMRealPoint Point[3];
	NBMRealNormal Normol[3];
	NBMRealTextCoord TextureCoord[3];
};

struct NBMDoubleTriangle
{
    NBMDoublePoint Point[3];
    NBMDoubleNormal Normol[3];
    NBMDoubleTextCoord TextureCoord[3];
};

struct NBMPosition
{
	NBMFloat3 x;
	NBMFloat3 y;
	NBMFloat3 z;
    
    NBMPosition():x(0),y(0),z(0){}

	unsigned int size()
	{
		return sizeof(x) + sizeof(y) + sizeof(z);
	}
};

struct NBMNormal
{
	NBMFloat3 x;
	NBMFloat3 y;
	NBMFloat3 z;

    NBMNormal():x(0),y(0),z(0){}

	unsigned int size()
	{
		return sizeof(x) + sizeof(y) + sizeof(z);
	}
};

struct NBMTextureCord
{
	NBMFloat3 x;
	NBMFloat3 y;

    NBMTextureCord():x(0),y(0){}

	unsigned int size()
	{
		return sizeof(x) + sizeof(y);
	}
};

typedef struct point_float
{
	float x;
	float y;
}POINT2;

typedef struct tile_int
{
    int x;
    int y;
}TILEXY;

typedef struct tile_Bound
{
    float fleft;
    float ftop;
    float fright;
    float fbottom;

	tile_Bound()
	{
		fleft = 0.0f;
		ftop = 0.0f;
		fright = 0.0f;
		fbottom = 0.0f;
	}

	tile_Bound(float left, float top, float right, float bottom) : fleft(left), ftop(top), fright(right), fbottom(bottom) {}
}TILEBOUND;
struct NBMLayerInfo
{
	NBMLayerInfo()
	{
		layerType = NBM_LAYER_ID_INVALID;	
		highScale = 0;		
		lowScale  = 0;		
	}

	unsigned short layerType;		// The type of layer.
	float  highScale;		// Near Visibility: Highest scale in meters per pixel.
	float  lowScale;		// Far Visibility: Lowest scale in meters per pixel.
};

// NBMVertex & GeometryBody need this flag to save data by different type.
typedef enum 
{
    NBMPrecision_half = 0,      // Current float16 way 
    NBMPrecision_half2 = 1,     // New high precision float16
    NBMPrecision_float = 2,     // Float = normal float number
    NBMPrecision_pint = 3       // Packed format described in NBM document
}NBMDataPrecision;

struct DataFileTailBody
{
	unsigned int fileHash;

	unsigned int length;
	unsigned int identifer;
};

#define NEW_FEATURE_THEME_MATERIAL
