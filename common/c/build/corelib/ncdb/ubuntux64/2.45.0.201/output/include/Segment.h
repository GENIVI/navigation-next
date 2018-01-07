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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2014 by TeleCommunication Systems, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to TeleCommunication Systems, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of TeleCommunication Systems is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/
#pragma once
#ifndef NCDB_SEGMENT_H
#define NCDB_SEGMENT_H

#include "NcdbTypes.h"
#include "AutoArray.h"
#include "UtfString.h"
#include "AutoSharedPtr.h"

#define SEGMENT_VER     6

class Map;
class MapLink;
struct dtm_rec;
class EngineConfiguration;

namespace Ncdb
{

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 + Defines Nodes and Segments in DB
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


//! A node is uniquely identified by
//! Map ID     : !!!TBD somehow identify all the files that belong to this DB should be identified with this ID
//! Layer ID   : Uniquely identify the layer in this map. Example would be Layer 1, Layer 2 and T layer will be layer 3
//! Block ID   : Uniquely identify a block with in this layer ( There can be more than 65,000 blocks)
//! Node index : Unique Node index within this block. There is a way to identify Ref nodes in DB.
//!              the nodes are designated as Node1 and Node2 ( Node1 is the ref node). So a segment is always
//!              identified with it's ref node.

//! In order to uniquely identify a segment we need the link number connected to this node.
//! link_no    : link number connected to this node. If there are n links connected to this node, they will be numbered 1 to n

typedef unsigned char   MapIdType;
typedef unsigned char   LayerType;
typedef unsigned char   NodeLinkType;
typedef unsigned short  NodeType;
typedef unsigned int    MapBlockType;


struct NCDB_API NodeSegmentID
{
    typedef unsigned long long  id_a;

    id_a            m_Value;

                    NodeSegmentID() : m_Value( 0 ) {}
    explicit        NodeSegmentID( id_a value ) : m_Value( value ) {}
                    NodeSegmentID( LayerType l, NodeLinkType nl, NodeType n, MapBlockType mb );
                    NodeSegmentID( MapIdType m, LayerType l, NodeLinkType nl, NodeType n, MapBlockType mb );

    void            set( LayerType l, NodeLinkType nl, NodeType n, MapBlockType mb );
    void            set( MapIdType m, LayerType l, NodeLinkType nl, NodeType n, MapBlockType mb );

    void            set( id_a value );

    MapIdType       getMapId(void) const;

    LayerType       getLayer(void) const;

    NodeLinkType    getNodeLink(void) const;

    NodeType        getNode(void) const;

    MapBlockType    getMapBlock(void) const;

    void            get(LayerType &l, NodeLinkType &nl, NodeType &n, MapBlockType &mb) const;
    void            get(MapIdType &m, LayerType &l, NodeLinkType &nl, NodeType &n, MapBlockType &mb) const;

    void            setMapId(MapIdType m);


    bool            operator == ( const NodeSegmentID& o )    const { return m_Value == o.m_Value; }
    bool            operator != ( const NodeSegmentID& o )    const { return m_Value != o.m_Value; }

    bool            operator <  ( const NodeSegmentID& o )    const { return m_Value <  o.m_Value; }
    bool            operator >  ( const NodeSegmentID& o )    const { return m_Value >  o.m_Value; }
    bool            operator <= ( const NodeSegmentID& o )    const { return m_Value <= o.m_Value; }
    bool            operator >= ( const NodeSegmentID& o )    const { return m_Value >= o.m_Value; }

};

typedef AutoArray<NodeSegmentID>    RoadSegmentIDList;
#ifdef _MSC_VER
template class NCDB_API AutoArray<NodeSegmentID>;
template class NCDB_API AutoSharedPtr<RoadSegmentIDList>;
#endif

struct NCDB_API LaneInfoStruct
{
	unsigned int m_ID;
	long m_ID2;
	UtfString m_DirLinkData;
	UtfString m_LinkSeqData;
};

#define AR_FORMAT_UNDEFINED 0x0     // no addresses defined
#define AR_FORMAT_EVEN      0x1
#define AR_FORMAT_MIXED     0x2
#define AR_FORMAT_ODD       0x3

struct NCDB_API AddressRangeInfo
{
    int lFormat;
    int lFirst;
    int lLast;
    int rFormat;
    int rFirst;
    int rLast;
};


struct NCDB_API NodeSegmentWithCost
{
    NodeSegmentWithCost()
    {
    }

	NodeSegmentWithCost(const NodeSegmentID& nodeSegmentId, const float& cost)
	{
		m_nodeSegmentId = nodeSegmentId;
		m_cost = cost;
	}

	NodeSegmentID	m_nodeSegmentId;
	float			m_cost;
};

typedef AutoArray<NodeSegmentWithCost>      NodeSegmentWithCostList;
#ifdef _MSC_VER
template class NCDB_API AutoArray<NodeSegmentWithCost>;
template class NCDB_API AutoSharedPtr<NodeSegmentWithCostList>;
#endif



class   Session;
class   LaneInfoGet;
struct  Segment;
struct  SegmentDetailStreetName;
struct  SegmentAddress;
struct  SegmentDetailInfo;
struct  SegmentBriefInfo;
struct  SegmentDirInfo;
struct  SegmentTMCInfo;
struct	CdmInfo;


typedef AutoSharedPtr< CdmInfo >				CdmInfoPtr;
typedef AutoArray< CdmInfoPtr >					CdmInfoList;
typedef AutoSharedPtr< CdmInfoList >			CdmInfoListPtr;
typedef AutoArray< CdmInfoListPtr >				CDMsGroup;

class NCDB_API SegInfoAccessor
{
public:
    SegInfoAccessor(Session & rSession);
	~SegInfoAccessor();

    ReturnCode GetSegInfo(const NodeSegmentID& rSegment, Segment& segment);

    ReturnCode IsLegalLink(const NodeSegmentID& nodeSegmentID, MapLink*& pMapLink);
    
    ReturnCode GetDetailStreetName(const NodeSegmentID& nodeSegmentID, SegmentDetailStreetName& seg_detail_stname);
    
    ReturnCode GetSegAddressInfo(const NodeSegmentID& rSegment, SegmentAddress& rSegmentAddrInfo,  bool fill_defaults = true);

    ReturnCode GetSegAddressRangeInfo (const NodeSegmentID& nodeSegmentID, AddressRangeInfo & rInfo);

    ReturnCode GetSegShapePoints(const NodeSegmentID& rSegment, WorldPointList& worldPointList);

    ReturnCode GetSegPointAddress(const NodeSegmentID& rSegment, WorldPointList& arrPointList, WorldPointList& dispPointList);

    ReturnCode GetSegDetailInfo( const NodeSegmentID& rSegment, SegmentDetailInfo& segDetailInfo);

    ReturnCode GetSegBriefInfo( const NodeSegmentID& rSegment, SegmentBriefInfo& rSegBriefInfo );

	ReturnCode GetCdmInfoList( const NodeSegmentID& rSegment, CdmInfoList& rCdmInfoList );

    ReturnCode GetLaneInfo( const NodeSegmentID& rSegment, LaneInfoStruct& rLaneInfoStruct );

	ReturnCode GetSegTMCInfo( const NodeSegmentID& rSegment, SegmentTMCInfo& rSegTMCInfo, int dir );

    ReturnCode GetSegidByPvid( const unsigned int& pvid, NodeSegmentID& rSegment );

	ReturnCode GetSegidByTTid( const unsigned long long& TTid, NodeSegmentID& rSegment );

	ReturnCode GetSegmentAlongDir(const WorldPoint& pt,float distanceInMeters,float heading, SegmentDirInfo& segInfo);

	void GetMapBBox(WorldRect & box) const;
	bool GetOriginalMapBBox(int mapId, WorldRect & box) const;
	void GetMapBlockBBox(bool local, int blkx, int blky, WorldRect & box);
	void GetMapBlockBinCoords(bool local, const WorldPoint & pt, int & blkx, int & blky);
	ReturnCode GetSegsAtNode( const NodeSegmentID& nodeSegmentID, bool atEnd, RoadSegmentIDList & segList);
	int GetCommonNode( const NodeSegmentID& nodeSegmentID1, bool atEnd1, const NodeSegmentID& nodeSegmentID2);
private:
    ReturnCode GetZoneName( int mapId, long lFilePos, char* name );

    Map* m_pMap;

    Session* m_pSession;

    LaneInfoGet *m_pLaneInfoGet;
};

enum NCDB_API TravelDirection
{
    TD_Unknown = 0,
    TD_Forward = 1,
    TD_Backward = 2,
    TD_Both = 3
};

enum NCDB_API FunctionClass
{
    FunClassUnknown = 0,
    FunClass1 = 1,
    FunClass2 = 2,
    FunClass3 = 3,
    FunClass4 = 4,
    FunClass5 = 5,
};

//! Virtual Connection Type
enum NCDB_API VirtConnType
{
    VCT_ELEVATOR        = 0,
    VCT_ESCALATOR       = 1,
    VCT_LEVEL           = 2,
    VCT_PEDESTRIAN_RAMP = 3,
    VCT_STAIRS          = 4,
	VCT_EXT_XING        = 5
};

//! Virtual Connection Access Restriction
enum NCDB_API VirtConnAccessRestr
{
    VCAR_TO_DUSK        = 0,
    VCAR_DUSK_TO_DAWN   = 1,
    VCAR_NONE           = 2
};

//! Virtual Connection Grade
enum NCDB_API VirConnGrade
{
    VCG_NOT_APPLICABLE  = 0,
    VCG_TO_REF          = 1,
    VCG_FROM_REF        = 2,
	VCG_NONE            = 3
};

//! Virtual Connection Location
enum NCDB_API VirtConnLoc
{
    VCL_NOT_APPLICABLE  = 0,
    VCL_BUILDING        = 1,
    VCL_PARK_FREE       = 2,
    VCL_PARK_FEE        = 3,
    VCL_PLAZA           = 4,
    VCL_STREET          = 5
};

//! Virtual Connection Stair Traversal
enum NCDB_API VirtConnStairTrav
{
    VCST_NOT_APPLICABLE = 0,
    VCST_TRUE           = 1,
    VCST_FALSE          = 2
};

enum NCDB_API CdmType
{
	CDM_TYPE_NONE = 0,
	CDM_TYPE_TOLL_STRUCTURE,
	CDM_TYPE_EXTRA_TYPE,
	CDM_TYPE_CONSTRUCTION_CLOSED,
	CDM_TYPE_GATE,
	CDM_TYPE_DIRTRAVEL,
	CDM_TYPE_RDM,
	CDM_TYPE_ACCESS_RESTRICTION,
	CDM_TYPE_SPEED_INFO,
};

enum NCDB_API CrosswalkType
{
	CDM_CROSSWALK_NONE = 0,
	CDM_CROSSWALK_ZEBRA,
	CDM_CROSSWALK_PARALLEL,
	CDM_CROSSWALK_TUNNEL,
	CDM_CROSSWALK_BRIDGE,
	CDM_CROSSWALK_NO_MARKINGS,
	CDM_CROSSWALK_PRIORITY_MANEUVER,
	CDM_CROSSWALK_ELECTRONIC_TOLL,
};
enum CondSpeedType {
	RDM_ADVISORY_SPEED_TYPE = 0, // NOT POPULATED NOW
	RDM_DEP_SPEED_TYPE_SCHOOL = 1,
	RDM_DEP_SPEED_TYPE_DTM        = 2,
	RDM_DEP_SPEED_TYPE_SEASONAL   = 3,// NOT POPULATED NOW
	RDM_SPEED_TYPE_NONE = 4,
};  
enum DirectionalInfo {
	BOTH_WAY = 0,//Applies in both directions 
	REVERSE_ONLY = 1,//Applies in reverse direction
	FORWARD_ONLY = 2,//Applies in forward direction
};
// sub types of GATE ,given by Andy
enum CDM_EXTRA_SUBTYPE {
	EXST_CROSSWALK = 0,
	EXST_TRAFFIC_LIGHT = 1,
	EXST_TRAFFIC_SIGN = 2,
	EXST_RAILWAY_CROSS = 3, 
	EXST_COUNTRY_BORDER_CROSS = 4
};

enum TrafficSignTypes {
    TST_UNKNOWN                         = 0,
    TST_START_NO_OVERTAKING             = 1,
    TST_END_NO_OVERTAKING               = 2,
    TST_PROTECTED_OVERTAKING_LANE       = 3,
    TST_PROTECTED_OVERTAKING_LANE_RIGHT = 4,
    TST_PROTECTED_OVERTAKING_LANE_LEFT  = 5,
    TST_LANE_MERGE_RIGHT                = 6,
    TST_LANE_MERGE_LEFT                 = 7,
    TST_LANE_MERGE_CENTER               = 8,
    TST_RR_XING_PROTECTED               = 9,
    TST_RR_XING_UNPROTECTED             = 10,
    TST_ROAD_NARROWS                    = 11,
    TST_SHARP_CURVE_LEFT                = 12,
    TST_SHARP_CURVE_RIGHT               = 13,
    TST_WINDING_ROAD_LEFT               = 14,
    TST_WINDING_ROAD_RIGHT              = 15,
    TST_START_NO_OVERTAKING_TRUCKS      = 16,
    TST_END_NO_OVERTAKING_TRUCKS        = 17,
    TST_STEEP_HILL_UPWARDS              = 18,
    TST_STEEP_HILL_DOWNWARDS            = 19,
    TST_STOP                            = 20,
    TST_LATERAL_WIND                    = 21,
    TST_GENERAL_WARNING_SIGN            = 22,
    TST_RISK_OF_GROUNDING               = 23,
    TST_GENERAL_CURVE                   = 24,
    TST_END_ALL_RESTRICTIONS            = 25,
    TST_GENERAL_HILL                    = 26,
    TST_ANIMAL_CROSSING                 = 27,
    TST_ICY_CONDITIONS                  = 28,
    TST_SLIPPERY_ROAD                   = 29,
    TST_FALLING_ROCKS                   = 30,
    TST_SCHOOL_ZONE                     = 31,
    TST_TRAMWAY_CROSSING                = 32,
    TST_CONGESTION_HAZARD               = 33,
    TST_ACCIDENT_HAZARD                 = 34,
    TST_PRIORITY_OVER_ONCOMING_TRAFFIC  = 35,
    TST_YIELD_TO_ONCOMING_TRAFFIC       = 36,
    TST_CROSSING_PRIORITY_FROM_RIGHT    = 37,
    TST_PEDESTRIAN_CROSSING             = 41,
    TST_YIELD                           = 42,
    TST_NO_ENGINE_BRAKE                 = 53,
    TST_END_NO_ENGINE_BRAKE             = 54,
    TST_NO_IDLING                       = 55,
    TST_TRUCK_ROLLOVER                  = 56,
    TST_LOW_GEAR                        = 57,
    TST_END_LOW_GEAR                    = 58,
};
enum LinkEndTypes{
	CDMF_LINKEND_NA       = 0,
	CDMF_LINKEND_REF      = 1,
	CDMF_LINKEND_NONREF   = 2,
	CDMF_LINKEND_BOTH	  = 3,
};
// sub types of GATE ,given by Andy

struct NCDB_API ParamValItem
{
	UtfString	parameter;
	UtfString	value;
};

typedef AutoArray< ParamValItem >	ParamValList;
#ifdef _MSC_VER
template class NCDB_API AutoArray< ParamValItem >;
#endif

enum DTM_Types {
    DTM_DATE_RANGE,
    DTM_DOM,
    DTM_DOW_WOM,
    DTM_DOW_WOY,
    DTM_WOM,
    DTM_MOY,
    DTM_DOM_MOY,
    DTM_DAYS_OF_WEEK,
    DTM_EXTERNAL,
    DTM_DAWN_TO_DUSK,
    DTM_DUSK_TO_DAWN,
    DTM_INVALID,
};

struct NCDB_API DateTimeModifier
{
    DateTimeModifier();

    DateTimeModifier(const dtm_rec* fromEngine);

    ~DateTimeModifier();

	void ToStringList( ParamValList& vParamList ) const;

    UtfString ToString() const;
    int GetDtmType () const;
    bool IsExcludeDate () const;
    bool IsFromEnd () const;
    // For DTM_DATE_RANGE
    void GetDate (bool start, int & year, int & month, int & day) const;
    // For DTM_DOW_WOM, DTM_DOW_WOY, and DTM_DOM_MOY
    void GetDate (bool start, int & day, int & week_month) const;
    // For DTM_DOM, DTM_WOM, DTM_MOY
    void GetDate (bool start, int & day_week_month) const;
    // For DTM_DAYS_OF_WEEK
    void GetDate (bool start, bool dow [8]) const;
    void GetDate (bool dow [8]) const;
    void GetTime (bool start, int & hour, int & min) const;
    bool IsApplicable (float hoursFromMidnight, int dayOfWeek, int dayOfMonth, int month, int year) const;

private:
    DateTimeModifier(const DateTimeModifier&);

    DateTimeModifier& operator =(const DateTimeModifier&);
private:
    dtm_rec* m_DTM;
};

#ifdef _MSC_VER
template class NCDB_API AutoArray< AutoSharedPtr< DateTimeModifier > >;
template class NCDB_API AutoArray< AutoSharedPtr< CdmInfo > >;
template class NCDB_API AutoArray< AutoSharedPtr< CdmInfoList > >;
#endif

struct NCDB_API Node
{
    WorldPoint      m_NodeLocation;
    unsigned char   m_No_of_Links;
    unsigned char   m_ZLevel;
    bool            m_IsBndNode;
    bool            m_IsMergedNode;
    bool            m_IsLocal;
};

typedef AutoSharedPtr< DateTimeModifier >       DateTimeModifierPtr;
typedef AutoArray< DateTimeModifierPtr >        DtmList;

struct NCDB_API Segment
{
    Node nStart;
    Node nFinish;

    //AccessType
    bool bIsCarpool;
    bool bIsNotThroughTraffic;
    bool bIsAccessAuto;
    bool bIsAccessTruck;
    bool bIsParkingLot;
    bool bIsAccessEmergency;
	bool bIsAccessPedPref;
    bool bIsAccessTaxi;
    bool bIsAccessBus;
    bool bIsPedestriansAllowed;

    //DisplayType
    bool bIsRamp;
    bool bIsControlledAccess;
    bool bIsRoundAbout;
    bool bIsIntersectionInternal;
    bool bIsSpecialExplication;
    bool bIsTurnLane;
    bool bIsBoatFerry;
    bool bIsToll;
    bool bIsTunnel;
    bool bIsBridge;
    bool bIsPrivate;
    bool bIsUnpaved;
	bool bIsDivloc;

	bool bIsMPH;
	unsigned char iBoundaryType;
	unsigned char iMergeType;

    bool bIsVirtualConn;
    VirtConnType        vctType;
    VirtConnAccessRestr vcrAccessRestr;
    VirConnGrade        vcgGrade;
    VirtConnLoc         vclLocation;
    VirtConnStairTrav   vcsStairsTraversal;

    float                                       fLengthInMeters;
    TravelDirection                             tdDirection;
    FunctionClass                               functionClass;
	int											iSpeedLimitedPrecise;
	short										iIsPreciseSpeed;
	unsigned char								uZLevelrefNode;
	unsigned char								uZLevelnonrefNode;
	unsigned char								uLaneNumfromrefNode;
	unsigned char								uLaneNumtorefNode;
	unsigned char                               uSpeedCat;
	unsigned long long							ulSegmentId;
	//links64.bin
	unsigned long long                          uiTTid;
	unsigned int								uiPvid;
    unsigned int                                uiLfoFpos;

	UtfString                                   sZLevels;

    AutoArray<UtfString>                        sFwdTMCcodes;
    AutoArray<UtfString>                        sBckwdTMCcodes;

    float                                       timeZoneIndex;
	bool                                        isDstFollowed;
    bool                                        isRightDriving;
    bool                                        bIsGhostLink;
    int                                         iRouteType;
    AutoArray< long >                           m_vAltRouteTypes;

    int                                         shapeCount;

	WorldPointList								pointList;
    bool                                        bIsEdgeLink;
    bool                                        bIsLongLink;
    bool                                        inSAR;
    bool                                        inMJO;
    bool                                        inECM;

    int                                         globalBinX;
    int                                         globalBinY;
};


struct NCDB_API SegmentBriefInfo
{
    UtfString   		sSegmentName;
    bool        		bIsGhostLink;
	TravelDirection		tdDirection;
};

struct NCDB_API SegmentDirInfo
{
    NodeSegmentID    rSegmentID;
    TravelDirection  tdDirection;
};

struct NCDB_API SegmentDetailInfo
{
    UtfString           sSegOrig;
    UtfString           sSegType;
    UtfString           sSegPrefix;
    UtfString           sSegSuffix;
};

struct NCDB_API AddressRange
{
    unsigned int uFirstNumber;
    unsigned int uLastNumber;
};

struct NCDB_API Side
{
    UtfString sCity;
    UtfString sState;
    UtfString sZone;
    UtfString sZipCode;
    AddressRange addressRange;
	UtfString sCounty;
	UtfString sNamedArea;
};

struct NCDB_API SegmentSign
{
    UtfString sSegsign;
    UtfString sBranchSign;
    UtfString sTowardSign;
    UtfString sDestSign;
};

struct NCDB_API SegmentDetailStreetName
{
    UtfString               sSegOrig;
    UtfString               sSegType;
    UtfString               sSegPrefix;
    UtfString               sSegSuffix;
};

struct NCDB_API SegmentAddress
{
    UtfString               sSegmentName;
    UtfString               sSegmentPronunKey;
    UtfString               sCountry;
    int                     iCountryId;
    AutoArray<UtfString>    sAlternativeNames;
	AutoArray< int >        bAlternativeOnSign;

    bool                    bSegmentOnSign;

    Side rightSide;
    Side leftSide;
    UtfString               sSegmentPronunDial;
    int                     iSegmentPronunType;
    int                     iSegmentPhPlayTime;
    AutoArray<UtfString>    sSegLangID; // As Jiayu said,if add this argument after "UtfString sSegmentPronunKey;", may cause server error 
};

#define CDMINFO_SPEED_TYPE_MASK	0x0180
#define CDMINFO_SPEED_MASK		0x3E00
#define CDMINFO_DIRECTION_MASK	0xC000

struct NCDB_API CdmInfo
{
	CdmInfo()
	{
		m_cdmType = CDM_TYPE_NONE;
		m_bAccessAuto = false;
		m_bAccessTruck = false;
		m_bAccessCarpool = false;
		m_bAccessDelivery = false;
		m_bAccessEmergency = false;
		m_bAccessTaxi = false;
		m_bAccessBus = false; 
		m_bAccessThruTraffic = false;
		m_bAccessPedestrian = false;
		m_crosswalkType = CDM_CROSSWALK_NONE;
		m_condSpeedType = RDM_SPEED_TYPE_NONE;
		m_speedLimitValue = 0;//kph
		m_directionalInfo = BOTH_WAY;
		isMPH = false;	
		CDM = 0;
		m_extraSubtype = EXST_CROSSWALK; // will indicate EXST_CROSSWALK if used
		m_trafficSignType = TST_UNKNOWN;
		m_linkEndType = CDMF_LINKEND_NA;

	}

	CdmType 		m_cdmType;
	bool			m_bAccessAuto;
	bool			m_bAccessTruck;
	bool			m_bAccessCarpool;
	bool			m_bAccessDelivery;
	bool			m_bAccessEmergency;
	bool			m_bAccessTaxi;
	bool			m_bAccessBus;
	bool			m_bAccessThruTraffic;
	bool			m_bAccessPedestrian;
	CrosswalkType	m_crosswalkType;
	DtmList			m_dtms;

	unsigned int	CDM;

	CDM_EXTRA_SUBTYPE  m_extraSubtype;
	TrafficSignTypes   m_trafficSignType;
	LinkEndTypes	   m_linkEndType;

	CondSpeedType   m_condSpeedType;
	unsigned int	m_speedLimitValue;//kph
	DirectionalInfo m_directionalInfo;

	bool			isMPH;
	void			FillParameters( unsigned char type, unsigned short flags, bool limitMPH );
	void            FillParameters( MapLink* pLink, const unsigned int* cdm, bool limitMPH );
};

struct NCDB_API SegmentTMCInfo
{
    TravelDirection                             tdDirection;
    unsigned char                               uSpeedCat;
	float                                       fLengthInMeters;
    AutoArray<UtfString>                        TMCcodes;
};
}
////////////////////////////////////////////////////////////////////////////////////////////////////

inline
void Ncdb::NodeSegmentID::set(LayerType l, NodeLinkType nl, NodeType n, MapBlockType mb)
{
    m_Value =
          ( (unsigned long long)l&0x7) |
          (((unsigned long long)nl&0xf)<<3) |
          (((unsigned long long)n&0xffff)<<7) |
          (((unsigned long long)mb&0x1ffffff)<<23);
}

inline
void Ncdb::NodeSegmentID::set(MapIdType m, LayerType l, NodeLinkType nl, NodeType n, MapBlockType mb)

{
    m_Value =
          ( (unsigned long long)l&0x7) |
          (((unsigned long long)nl&0xf)<<3) |
          (((unsigned long long)n&0xffff)<<7) |
          (((unsigned long long)mb&0x1ffffff)<<23) |
          (((unsigned long long)m&0xf)<<48);
}

inline
void Ncdb::NodeSegmentID::set( id_a value )
{
    m_Value = value;
}

inline
Ncdb::NodeSegmentID::NodeSegmentID( LayerType l, NodeLinkType nl, NodeType n, MapBlockType mb )
{
    set( l, nl, n, mb );
}

inline
Ncdb::NodeSegmentID::NodeSegmentID( MapIdType m, LayerType l, NodeLinkType nl, NodeType n, MapBlockType mb )
{
    set( m, l, nl, n, mb );
}

inline
Ncdb::MapIdType Ncdb::NodeSegmentID::getMapId(void) const
{
    return (NodeLinkType)((m_Value>>48)&0x0f);
}

inline
Ncdb::LayerType Ncdb::NodeSegmentID::getLayer(void) const

{
    return (LayerType)(m_Value&0x07);
}

inline
Ncdb::NodeLinkType Ncdb::NodeSegmentID::getNodeLink(void) const
{
    return (NodeLinkType)((m_Value>>3)&0x0f);
}

inline
Ncdb::NodeType Ncdb::NodeSegmentID::getNode(void) const
{
    return (NodeType)((m_Value>>7)&0xffff);
}

inline
Ncdb::MapBlockType Ncdb::NodeSegmentID::getMapBlock(void) const
{
    return (MapBlockType)((m_Value>>23)&0x1ffffff);
}

inline
void Ncdb::NodeSegmentID::get(LayerType &l, NodeLinkType &nl, NodeType &n, MapBlockType &mb) const
{
    l = getLayer();
    nl = getNodeLink();
    n = getNode();
    mb = getMapBlock();
}

inline
void Ncdb::NodeSegmentID::get(MapIdType &m, LayerType &l, NodeLinkType &nl, NodeType &n, MapBlockType &mb) const
{
    m = getMapId();
    l = getLayer();
    nl = getNodeLink();
    n = getNode();
    mb = getMapBlock();
}

inline
void Ncdb::NodeSegmentID::setMapId(MapIdType m)
{
    LayerType l;
    NodeLinkType nl;
    NodeType n;
    MapBlockType mb;

    get (l, nl ,n, mb);
    set (m, l, nl, n, mb);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

inline
Ncdb::DateTimeModifier::DateTimeModifier()
{
}

#endif // NCDB_SEGMENT_H
