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

@file     VectorTile.h
@date     03/28/2009
@defgroup MOBIUS_MAP  Mobius Map Draw API
@author   John Efseaff

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
#ifndef VECTORTILE_H_ONCE
#define VECTORTILE_H_ONCE


#include "NcdbTypes.h"
#include "RasterControl.h"
#include "AutoSharedPtr.h"
#include "AutoArray.h"
#include "WorldPoint.h"
#include "UtfString.h"
#include "AscString.h"
#include "GeoMath.h"
#include "Shaper.h"

#define MAX_ZLEVEL_NUM 37 //9*4+1

#define ROAD_NONE		0x0000
#define LIMITED_ACCESS	0x0001
#define ARTERIAL    	0x0002
#define LOCAL          	0x0004
#define TERMINAL   		0x0008
#define ROTARY       	0x0010
#define RAMP            0x0020
#define BRIDGE         	0x0040
#define TUNNEL        	0x0080
#define SKYWAY      	0x0100
#define FERRY           0x0200
#define UNPAVED			0x0400

namespace Ncdb {

class Session;

#ifdef _MSC_VER
template class NCDB_API AutoArray < AscString >;
#endif

class NCDB_API VectorRoad
{
public:
    VectorRoad(void);
    ~VectorRoad(void);
    AutoArray < WorldPointList > & getSegments(void) { return m_Segments; }
    UtfString & getName(void) { return m_Name; }
    unsigned int getNameIdentity(void) { return m_NameIdentity; }
    void setNameIdentity(unsigned int iNameIdentity) { m_NameIdentity = iNameIdentity; }
    int getRank(void) { return m_Rank; }
    void setRank(int iRank) { m_Rank = iRank; }
    int getRouteType(void) { return m_RouteType; }
    void setRouteType(int iRouteType) { m_RouteType = iRouteType; }
    LinkDirection getDirection(void) { return m_Direction; }
    void setDirection(LinkDirection direction) { m_Direction = direction; }
    LineUsage getLineUsage(void) { return m_Usage; }
    void setLineUsage(LineUsage usage) { m_Usage = usage; }
    AutoArray< AscString >& getForwardTMCs(void) { return m_ForwardTMCs; }
    AutoArray< AscString >& getBackwardTMCs(void) { return m_BackwardTMCs; }
private:
    UtfString m_Name;
    unsigned int m_NameIdentity;
    LineUsage m_Usage;
    int m_Rank;
    int m_RouteType;
    LinkDirection m_Direction;
    AutoArray < AscString > m_ForwardTMCs;
    AutoArray < AscString > m_BackwardTMCs;
    AutoArray < WorldPointList > m_Segments;
};

#ifdef _MSC_VER
template class NCDB_API AutoArray < VectorRoad >;
#endif

class NCDB_API VectorCenterPoint
{
public:
    VectorCenterPoint(void);
    ~VectorCenterPoint(void);
    int getPointType(void) { return m_PointType; }
	WorldPoint & getPoints(void) { return m_Location; }
	const UtfString&    getName(void) const { return m_Name; }
    UtfString&          getName(void)       { return m_Name; }
	int					getFlag(void){return iFlag;}
	void				setFlag(int flag){iFlag = flag;}
	void				setOrder(int order){iOrder = order;}
	int				    getOrder(){return iOrder;}
	void				setIndex(unsigned short index, int levelIdx/*0,1,2*/){ m_iIndex[levelIdx] = index;}
	unsigned short		getIndex(int levelIdx/*0,1,2*/){return m_iIndex[levelIdx];}
    AreaStyle           getAreaStyle(void) const { return m_Style; }
    void                setAreaStyle(AreaStyle style) { m_Style =  style; }
    AreaUsage           getAreaUsage(void) const { return m_Usage; }
    void                setAreaUsage(AreaUsage usage) { m_Usage = usage; }
private:
	int iFlag;
    int m_PointType;
	UtfString m_Name;
	WorldPoint m_Location;
	AreaStyle   m_Style;
	AreaUsage   m_Usage;
	int iOrder;
	unsigned short m_iIndex[3];
};

#ifdef _MSC_VER
template class NCDB_API AutoArray < VectorCenterPoint >;
#endif

class NCDB_API VectorArea
{
public:
    VectorArea(void);
    ~VectorArea(void);

    const AutoArray< WorldPointList >&  getPolygons(void) const { return m_Polygons; }
    AutoArray< WorldPointList >&        getPolygons(void)       { return m_Polygons; }

    const UtfString&    getName(void) const { return m_Name; }
    UtfString&          getName(void)       { return m_Name; }
    AreaStyle           getAreaStyle(void) const { return m_Style; }
    void                setAreaStyle(AreaStyle style) { m_Style =  style; }
    AreaUsage           getAreaUsage(void) const { return m_Usage; }
    void                setAreaUsage(AreaUsage usage) { m_Usage = usage; }
    int                 getAreaZ(void)     const { return m_Z; }
    void                setAreaZ(int iZ)  { m_Z = iZ; }
	void				setPoint(VectorCenterPoint& tPoint){ m_Point = tPoint;}
	VectorCenterPoint&	getPoint(void){return m_Point;}
	float				getAreaSize(void){return m_fAreaSize;}
    void				setAreaSize(float fAreaSize){ m_fAreaSize = fAreaSize;}
    void                setAreaB2DType(int type){m_AreaB2DType = type;}
    int                 getAreaB2DType(){return m_AreaB2DType;}
private:
    AreaStyle   m_Style;
    AreaUsage   m_Usage;
    int         m_Z;   
    UtfString   m_Name;
    AutoArray< WorldPointList > m_Polygons;
	VectorCenterPoint m_Point;
	float m_fAreaSize;
    int         m_AreaB2DType;//0- is not 2dbuilding,1- complex 2dbuilding,2- simple 2dbuilding
};

typedef AutoArray < VectorArea >    VectorAreaList;
#ifdef _MSC_VER
template class NCDB_API AutoArray < VectorArea >;
#endif

class NCDB_API VectorLinear
{
public:
    VectorLinear(void);
    ~VectorLinear(void);
    AutoArray < WorldPointList > & getSegments(void) { return m_Segments; }
    UtfString & getName(void) { return m_Name; }
    LineUsage getLineUsage(void) { return m_Usage; }
    void setLineUsage(LineUsage usage) { m_Usage = usage; }
    LineStyle getLineStyle(void) { return m_Style; }
    void setLineStyle(LineStyle style) { m_Style = style; }
private:
    UtfString m_Name;
    LineUsage m_Usage;
    LineStyle m_Style;
    AutoArray < WorldPointList > m_Segments;
};

#ifdef _MSC_VER
template class NCDB_API AutoArray < VectorLinear >;
#endif

class IVectorTile;
//! @brief Vector tile extraction and container class.
class NCDB_API VectorTile
{
public:
    //! Constructor
    VectorTile(Session& session);
    //! Desctructor
    ~VectorTile(void);
    //! Set default values for reuse
    void setDefaults();
    //! Set the tile to extract.
    void setTile(unsigned int x, unsigned int y, unsigned int z);
    //! Set the geographic boundry in standard coordinates.
    void setRectangle(WorldRect& rect);
    //! Extract the specified tile.
    ReturnCode extract();

    //! Get the road list.
    AutoArray < VectorRoad > & getRoadList(void);
    //! Get the polygon area list.
    AutoArray < VectorArea > & getAreaList(void);
    //! Get the list of railroads and rivers.
    AutoArray < VectorLinear > & getLinearList(void);

    //! Enable/Disable the type of objects for extraction.
    void setEnablement(bool doRoads, bool doAreas, bool doLinears);
    //! Enable/Disable the type of objects for extraction.
    void setBuildings(bool doBuilding);
    //! Set the maximum road rank to extract.
    //! @note  Setting 1-3 takes all data from the thin layer and setting
    //! 4-5 takes all data from the detail layer.  Use API setHighDetail()
    //! to override the detail level.
    void setPriority(int rank);
	//! Set the tile pixel size.
	void setTileSize(int width, int height);
	//! Set the display configuration.
	void setConfiguration(const char * name);
	//! Set screen dpi.
	void setResolution(const char * dpi);
    //! Set to combine links into one road.
    void setCombineLinks(bool flag);
    //! Set a polygon clip region.  Must be clockwise convex.
    void setPolygonClip(WorldPoint p1, WorldPoint p2, WorldPoint p3, WorldPoint p4);
    //! Set true to force data from the detail file when selecting priority 1-3.
    //! @note This API does nothing if setPriority to rank 4 or 5.
    void setForceHighDetail(bool flag);
    //! Set the maximum polygon rank to extract.
    //! @note set 0 for DVA, and 2 for GVA
    void setPolygonPriority(int rank);
    //! Set the  triangle thresh value.It is used for thinning out polyline
    //! @note default is 0.0, if triangleThresh!=0.F, filter point, Otherwise don't filter.
    void setTriangleThresh(float triangleThresh);
	//! Enable exonym string
    void enableExonymString(bool bExonymString=false);
    //! Get implement class pointer
    IVectorTile* getVectorTielImpl();
	//! set flag for clip road.
	void setRoadClip(bool bclip);
	//! set flag for enable paster.
	void setEnablePaster(bool flag);
	//! Get flag for enable paster.
	bool getEnablePaster();

private:
    IVectorTile* m_pVectorTileImpl;
    int m_Width;
	int m_Height;
    char m_Configuration[16], m_Resolution[16];

};

//! @brief Traffic tile extraction and container class.
class NCDB_API TrafficTile
{
public:
    //! Constructor
    TrafficTile(Session& session);
    //! Desctructor
    ~TrafficTile(void);
    //! Set the tile to extract.
    void setTile(unsigned int x, unsigned int y, unsigned int z);
    //! Set the geographic boundry in standard coordinates.
    void setRectangle(WorldRect& rect);
    //! Extract the specified tile.
    ReturnCode extract();

    //! Get the road list.
    AutoArray < VectorRoad > & getRoadList(void);

    //! Set the maximum road rank to extract.
    //! @note  Setting 1-3 takes all data from the thin layer and setting
    //! 4-5 takes all data from the detail layer.  Use API setHighDetail()
    //! to override the detail level.
    void setPriority(int rank);
	//! Set the tile pixel size.
	void setTileSize(int width, int height);
	//! Set the display configuration.
	void setConfiguration(const char * name);
	//! Set screen dpi.
	void setResolution(const char * dpi);
    //! Set to combine links into one road.
    void setCombineLinks(bool flag);
    //! Set true to force data from the detail file when selecting priority 1-3.
    //! @note This API does nothing if setPriority to rank 4 or 5.
    void setForceHighDetail(bool flag);
private:
    IVectorTile* m_pVectorTileImpl;
    int m_Width;
	int m_Height;
    char m_Configuration[16], m_Resolution[16];
};

} // namespace Ncdb

#endif // VECTORTILE_H_ONCE
/*! @} */
