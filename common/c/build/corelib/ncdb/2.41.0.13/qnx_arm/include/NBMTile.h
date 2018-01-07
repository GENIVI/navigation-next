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

@file     NBMTile.h
@date     11/01/2011
@defgroup MOBIUS_MAP  Mobius Map NBMFILE API
@author   Sevnsson Sun

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

#pragma once
#ifndef NBMTILE_H_ONCE
#define NBMTILE_H_ONCE
#define NBM_LABELPOINT_INDEX 0x8003;
#include "NcdbTypes.h"
#include "UtfString.h"
#include "VectorTile.h"

#define ROAD_PASTER_API

class NBMFileHeader;

namespace Ncdb {
class Session;
struct LayerInfo
{
    float  highScale;        // Near Visibility: Highest scale in meters per pixel.
    float  lowScale;        // Far Visibility: Lowest scale in meters per pixel.
};

class NCDB_API VectorLabelPoint
{
    friend class NBMTile;
public:
    VectorLabelPoint(void){}
    ~VectorLabelPoint(void){}
    int getPointType(void) { return m_PointType; }
    WorldPoint & getPoints(void) { return m_Location; }
    const UtfString&    getName(void) const { return m_Name; }
    UtfString&          getName(void)       { return m_Name; }
    PointStyle           getPointStyle(void) const { return m_Style; }
    PointUsage           getPointUsage(void) const { return m_Usage; }
private:
    int m_PointType;
    UtfString m_Name;
    WorldPoint m_Location;
    PointStyle   m_Style;
    PointUsage   m_Usage;


};
#ifdef _MSC_VER
template class NCDB_API AutoArray < VectorLabelPoint >;
#endif

typedef enum  
{
    NBM_FILTER_ANY=0,
    NBM_FILTER_SKIPLEN=1 ,
    NBM_FILTER_SKIPLEN1=2
}NbmFilterType;

class NCDB_API NBMTile
{
public:
    typedef enum FileTypesEnum
    {
        NBM_DVR = 0,
        NBM_GVR,
        NBM_DVA,
        NBM_GVA,
        NBM_DVR_DVA,
        NBM_GVR_GVA,
        NBM_3DUT,
        NBM_3DLM,
        NBM_MDAY,
        NBM_MNIGHT,
        NBM_PNGLAM,
        NBM_DAMPNG,
        NBM_WL,
        NBM_RL,
        NBM_RAST,
		NBM_RAST_NIGHT,
        NBM_LBLT,
        NBM_B2D,
        NBM_SATELLITE,
        NBM_POIS,
        NBM_MPOI,
		NBM_MPOIBMP,
        NBM_MBASEBMP,
		NBM_MDAYTHEME,
		NBM_MNIGHTTHEME,
        NBM_MSATELLITETHEME,
    }NbmFileTypes;

	enum {
		LAYER_ID_ROAD = 0,              //NBM_DVR
		LAYER_ID_AREA = 2,              //NBM_DVA
		LAYER_ID_UNTEXTUREONLY = 6,	    //NBM_3DUT	
		LAYER_ID_LANDMARK = 7,          //NBM_3DLM
		LAYER_ID_ROAD_AREA = 4,         //NBM_DVR_DVA
		COMMON_MATERIAL_DAY = 8,        //NBM_MDAY
		COMMON_MATERIAL_NIGHT = 9,      //NBM_MNIGHT
		LAYER_NBM_LAM_PNG = 10,         //NBM_PNGLAM
		LAYER_NBM_DAM_PNG = 11,         //NBM_DAMPNG
        LAYER_ID_RAST = 14,             //NBM_RAST
		LAYER_ID_RAST_NIGHT = 15,       //NBM_RAST_NIGHT,
        LAYER_ID_LBLT = 16,             //NBM_LBLT
        LAYER_ID_2DBULD = 17,            //NBM_B2D
        LAYER_ID_SATELLITE = 18,         //NBM_SATELLITE
        LAYER_ID_POIS = 19               //NBM_POIS
	} NBMLayerTypeEnum ;

    typedef enum ZoomEnum 
    {
        NBM_ZOOM_LEVEL_GVR = 8,
        NBM_ZOOM_LEVEL = 15,
    } NbmZoomLevel;

    //! Constructor
    NBMTile(Session& session,bool bCompress);
    //! Desctructor
    ~NBMTile(void);

    void setTile(unsigned int x, unsigned int y, unsigned int z);
    //! Extract the specified tile.
    ReturnCode extract( int layerID);

	//! set draw paster or not
	void setRoadPaster(bool bRoadPaster);

	//! get draw paster or not
	bool isRoadPaster();

    //! Get the Layer ID list.  only can be returned the 3dlm and 3dut  // this interface may no useful.
    AutoArray < int > & getLayerIDList(unsigned short iMajorVersion=23);

    //! Get the extracted buffer.
    void getTileBuffer(const char *& pBuf , int& iLen);

    //! return a 32bit number for version of this layer.
    ReturnCode getLayerVersion(int layerID, int &i32);
    //! Set the maximum road rank to extract.
    //! @note  Setting 1-3 takes all data from the thin layer and setting
    //! 4-5 takes all data from the detail layer.
    void setPriority(int rank);
    //! Set the  triangle thresh value.It is used for thinning out polyline
    //! @note default is 0.0, if triangleThresh != 0.F, then filter point. Otherwise don't filter.
    //! reference value is 0.00001F. It will filter more point if the value increase.
    void setTriangleThresh(float triangleThresh);

    ReturnCode AdjustBuildingHeight(const char*& buf,unsigned int& buflen, int x, int y, const char* ptype);
    //! set enable LOD or not 
    //! bEnable = true, enable LOD; bEnable = false disable
    
    void EnableLOD(bool bEnable){m_bEnableLOD=bEnable;}

    void SetExonym(bool bExonym);

    //! Set the global locale.
    void SetLocale(LocalId lcid);

	void setNBMVersion(unsigned short iMajor, unsigned short iMinor);
	unsigned short getNBMVersion() {return m_NBMVersion;}

	int setProductandRes(const char* product, const char* res);

protected:
    //! Extract the Common Material NBM file;
    bool extractMaterial(int iLayerID);
    bool extractBase(int iLayerID);
	bool extractNBM3DChunk(int iLayerID);

private:
     LocalId m_lcid;
     unsigned int m_xIndex;
     unsigned int m_yIndex;
     Session& m_Session;
     RasterControlHandle m_RasterControl;
     const char * m_pBuf;
     UtfString m_pstylename;
     UtfString m_pSatellitStyleName;
     unsigned int  m_iBufLen;
     unsigned int m_izoomLevel;
     int m_Rank;
     AutoArray < int > m_arrLayerIDs;
     UtfString m_strDataPath;
//   bool m_bNBMAddition;
     bool m_bCompress;
     void * m_pCommonMaterialCfg;
     float  m_fTriangleThresh;
     AutoArray<VectorLabelPoint> m_LabelPoints[3];
     AutoArray<PixelRect>        m_pxRextList;
     //! Default is set to false, enable LOD
     bool m_bEnableLOD;
     bool m_bExonym;

	 //! Default is set to true, draw paster
	 bool m_bRoadPaster;
		
     unsigned short m_NBMVersion;

     void ExtractLabelPoint(int iLayerID);
     void BuildLabelPoint(Session& session,NBMFileHeader &ubsFile, void *pCommonMaterialCfg , int id );
     bool dispartPointLayer(int & layer  , int level ,PointLabel label);
     bool BuildRasterTile();
     bool BuildSatelliteTile();
     void BuildLabelTile(Session& session,NBMFileHeader &ubsFile,VectorTile& tiler, void *pCommonMaterialCfg);
	 void setMode(int iMode);
	 
	 void BuildVectorArea(Session& session,NBMFileHeader &ubsFile, VectorTile& tiler, void *pCommonMaterialCfg , int iLayerID);
};


};


#endif //NBMTILE_H_ONCE

