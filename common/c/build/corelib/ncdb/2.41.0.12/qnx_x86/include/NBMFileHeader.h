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

@file     NBMFileHeader.h
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
#include "NBMLayerChunk.h"
#include <vector>
#include <map>
#include <string>

struct DynamicChunkInfo;

class NBMFileHeader
{
public:
	NBMFileHeader(void);
	~NBMFileHeader(void);

	//FilePath MaxLen = 1024;
	void SetFilesPath(const char * pszPath);

	void SetRefInfo(const char * psz);

	bool SetIndex(unsigned int indexX, unsigned int indexY, unsigned int level=15);

	// Longitude Latitude float value
	bool IsIncludePoint(float x, float y);

	// Longitude Latitude to index
	void PointToTile(float lon, float lat,unsigned int& indexX, unsigned int& indexY);

    // Convert between degree and Mercator Offset
    void SwitchDegreeToMercatorOffset(double x, double y, unsigned short &offsetx, unsigned short &offsety, bool isHighPrecision = false, bool bMercator=false);
    void SwitchDegreeToMercatorOffset(double x, double y, float &offsetx, float &offsety, bool bMercator=false);
    void SwitchDegreeToMercatorOffset(double x, double y, double &offsetx, double &offsety, bool bMercator=false);
    void SwitchMercatorOffsetToDegree(unsigned short offsetx, unsigned short offsety, float &x, float &y, bool isHighPrecision = false);
    void SwitchMercatorOffsetToDegree(unsigned short offsetx, unsigned short offsety, double &x, double &y, bool isHighPrecision = false);
    void SwitchMercatorOffsetToDegree(float offsetx, float offsety, double &x, double &y);
    void SwitchMercatorOffsetToDegree(double offsetx, double offsety, double &x, double &y);

	// Convert between meter and Mercator Offset
	void SwitchMeterToMercatorOffset(float fMeter, unsigned short& f16Mercator);
    void SwitchMeterToMercatorOffset(float fMeter, float& fMercator);
    void SwitchMercatorOffsetToMeter(float fMercator, double& fMeter);
	void SwitchMercatorOffsetToMeter(unsigned short f16Mercator, double& dMeter);
    	
	void SetMaking();

	NBMChunk* GetChunk(unsigned int uChunkIdentifier);

	bool AccessFile();

	bool LoadFromFile();

	bool SaveToFile();

	bool FileBuffer(const char *&pBuf, unsigned int &uBufLen);

	bool GetTileBuffer(const char *&pBuf, unsigned int &uBufLen);

	bool SetTileByBuffer(const char *pBuf, unsigned int uBufLen);
	
    float meterToMercator();

	static void ReleaseBuffer(const char * pBuf);
	static const char * AllocBuffer(unsigned int uBufLen);

    static double meterToMercator(double lat);
    static double meterToLonDegree(double lat);
    static double meterToLatDegree();

	void GetBoundry(float& left, float& top, float& right, float& bottom)
	{
		left   = m_worldRectLeft;
		top    = m_worldRectTop;
		right  = m_worldRectRight;
		bottom = m_worldRectBottom;
	};

    void GetBoundryInMeractor(double& left, double& top, double& right, double& bottom)
    {
        left   = m_mercatorCenterX - m_mercatorWidth / 2;
        top    = m_mercatorCenterY + m_mercatorHeight / 2;
        right  = m_mercatorCenterX + m_mercatorWidth / 2;
        bottom = m_mercatorCenterY - m_mercatorHeight / 2;
    };
    void SetExonym(bool bExonym);
    bool IsExonym(); 
    
protected:
	void ClearChunks();
	bool AccessFile(char *pszFileName);

private:
	NBMChunk * CreateNewChunk(unsigned int uChunkIdentifier);
	void GetFileName(char* pszFileName);

//#pragma region New added functions

public:

	// Create a chunk by chunk identifier,
	// for dynamic chunk, need set LayerInfo and optional Associated Layer, parent chunk
	NBMChunk* CreateChunk(unsigned int chunkIdentifier, NBMLayerInfo* layerInfo = NULL, 
							   NBMChunk* associatedChunk = NULL, NBMChunk* parentChunk = NULL);

	// Get dynamic chunks by chunk identifier and layerType,
	// NBM_CHUNCK_IDENTIFIER_INVALID for get all dynamic chunks types,
	// NBM_LAYER_ID_INVALID for all layer type.
	void GetDynamicChunks(std::vector<NBMChunk*>& chunks, 
						  unsigned int chunkIdentifier = NBM_CHUNCK_IDENTIFIER_INVALID, 
						  unsigned short layerType = NBM_LAYER_ID_INVALID);

	// Get Associated Chunk
	NBMChunk* GetAssociatedChunk(NBMChunk* dynamicChunk);

    // Get dynamic chunk's all associated chunks
    void GetAssociatedChunks(NBMChunk* dynamicChunk, std::vector<NBMChunk*>& associatedChunks);

    // Get dynamic chunk's layer info
    NBMLayerInfo* GetDynamicChunkLayerInfo(NBMChunk* chunk);

    //! Set the z Level, for DVR:set level=15; GVR:set level=8
    //! @param[in] level - zoom level,make sure to set valid level
    void SetZLevel(unsigned int level);

    //! Get the z Level
    unsigned int GetZLevel()
	{
        return m_zLevel;
    }
	unsigned int GetXindex(){return m_xIndex;}
	unsigned int GetYindex(){return m_yIndex;}

	
	void setNBMVersion(unsigned short NBMVersion); 
	unsigned short getNBMVersion() {return m_wVersion;}
	unsigned short getNBMMajorVersion() {return (m_wVersion & 0xff00) >> 8;}
	unsigned short getNBMMinorVersion() {return m_wVersion & 0x00ff;}

	void setMaterialOffset(unsigned short offset) {m_wFlags = ((m_wFlags & 0xff00) | offset);}
	unsigned short getMaterialOffset() {return m_wFlags & 0x00ff;}

	bool hasBuildingBBox() {return (m_wFlags>>15 == 1);}
	void setBuildingBBox(TILEBOUND bound);
	TILEBOUND getBuildingBox();

private:
	
	// Get layer chunk's related dynamic chunks info
	void GetDynamicChunksInfo(NBMLayerChunk* layerChunk, std::map<unsigned int, LayerBody>& info);

	// Clean all empty dynamic chunks
	void CleanupEmptyChunks();

	// Create layer chunk before need get all chunks buffer
	bool CreateLayerChunk();

	// Dynamic chunk/index convert
	int DynamicChunk2Index(NBMChunk* dynamicChunk);
	NBMChunk* DynamicIndex2Chunk(int index);

	// Get NBM file Header length
	unsigned int GetFileHeaderLength();

	// Get NBM file header chunk size
	// The chunk size contains the length of data contained AFTER the chunk size variable.
	void CalculateFileHeaderChunkSize();

	// Get dynamic chunk's info
	DynamicChunkInfo* GetDynamicChunkInfo(NBMChunk* chunk);

	// Get dynamic chunk size(include it's sub chunks size too)
	unsigned int GetDynamicChunkSize(NBMChunk* chunk);

	// Save dynamic chunk to buffer
	unsigned int SaveDynamicChunk(const char* buffer, unsigned int limitedLength, NBMChunk* chunk);

	// Read all static chunks from buffer
	bool ReadStaticChunks(const char *buffer, unsigned int totalSize, unsigned int &usedSize, 
						  std::map<unsigned int, LayerBody> &layerBodys);

	// Read dynamic chunks from buffer
	bool ReadDynamicChunk(const char *buffer, unsigned int totalSize, unsigned int &usedSize, 
						  std::map<unsigned int, LayerBody> &layerBodys, NBMChunk* parentChunk);

//#pragma endregion

protected:
	//static const unsigned int c_arrChunkInnerIndex[NBM_CHUNK_TYPE_NUM];

	unsigned int m_u32FileIdentifier;
	unsigned int m_u32ChunkSize;
	unsigned short m_wVersion;
	unsigned short m_wFlags;

	unsigned int m_xIndex;
	unsigned int m_yIndex;
    unsigned int m_zLevel;

	//Tile's rectangle by degree
    //(Use basic date types to avoid include Ncdb head files)
    float  m_worldRectLeft;
    float  m_worldRectTop;
    float  m_worldRectRight;
    float  m_worldRectBottom;
	
	//Tile's rectangle by mercator rectangle	
    double m_mercatorWidth;
    double m_mercatorHeight;

	//Tile's center by mercator points
    double m_mercatorCenterX;
    double m_mercatorCenterY;

	// 3D building bounding box
	TILEBOUND m_buildingBBox;

	NBMChunk* m_ppChunkBase[NBM_CHUNK_TYPE_NUM];   // All statics chunks, in the fixed order
    std::vector<DynamicChunkInfo> m_dynamicChunks;      // All dynamic chunks

	bool m_bMaking;
    bool m_bExonym;

	char m_szFilePath[1024];
	char m_szFileRefInfo[128];
};
