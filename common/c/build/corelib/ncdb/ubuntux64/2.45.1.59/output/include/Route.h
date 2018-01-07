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
#ifndef NCDB_ROUTE_H
#define NCDB_ROUTE_H



#include "NcdbTypes.h"

#include "Connection.h"
#include "CostModel.h"
#include "UserOptions.h"
#include "Maneuver.h"
#include "EnhancedNV.h"
#include "SpeedLimits.h"
#include "NLGuidance.h"
#include "SpecialRegion.h"
#include "GeocodeTypes.h"

#define ROUTE_VER     1


namespace Ncdb {

enum RouteHasFlags
{
    ROUTE_HAS_HWY     = 0x01,
    ROUTE_HAS_BRIDGE  = 0x02,
    ROUTE_HAS_TUNNEL  = 0x04,
    ROUTE_HAS_TOLL    = 0x08,
    ROUTE_HAS_FERRY   = 0x10,
    ROUTE_HAS_TMC     = 0x20,
    ROUTE_HAS_UNPAVED = 0x40,
    ROUTE_HAS_HOV     = 0x80, 
    ROUTE_HAS_PRIVATE = 0x100,
    ROUTE_HAS_COUNTRY = 0x200,
    ROUTE_HAS_GATE    = 0x400,
};


/*!
   RouteSegment is a  structure which stores the ID and speed information of 
   the segment which forms a part of the route

*/
struct NCDB_API RouteSegment
{
    RouteSegment();

    int     m_Dir;              //Is direction of travel from RefNode to Non-ref Node;
    float   m_mapDataSpeed;     //! Map data speed used in route calc in mps
    float   m_AdjustedSpeed;    //! Actual speed used in the route calc in mps
                                //! either real time or historic based on the options chosen
    float   m_Length;           //length of the RouteSegment in meters

	int                     m_EnhancedNV; // is in a SAR/MJO/ECM
    NodeSegmentID           m_SegmentID;

	int						m_cid;//country id

	bool isSpeedCat;
	bool hasCDM;

    //!TMC Codes on the segment in the direction of travel -Used by the Traffic Notify Traffic Incidents.
    AutoArray<UtfString>    TMCcodes;

	//!Poly line info , -1 is invalidate 
	int m_indexMan;				//which maneuver contained this segment's polyline .
	int m_indexManPolyStart;		//which point as the start point in this maneuver's poly line.
	int m_indexManPolyEnd;		//which point as the end point in this maneuver's poly line.

	int m_indexNormalPolyStart;	//which point as the start point in whole routing path poly line.
	int m_indexNormalPolyEnd;		//which point as the end point in whole routing path poly line.

};

typedef AutoArray<RouteSegment>     RouteSegmentList;
#ifdef _MSC_VER
template class NCDB_API AutoSharedPtr<RouteSegmentList>;
#endif


/*! RouteMapBlocks is a strucure that contains the list of all MapBlocksIndices 
    of the RouteSegments.
*/
typedef AutoArray<int> MapBlockIndexList;
#ifdef _MSC_VER
template class NCDB_API  AutoSharedPtr<MapBlockIndexList>;
#endif

class NCDB_API RouteMapBlocks
{
    public:
		RouteMapBlocks();
		~RouteMapBlocks(){};
		/*!@brief Get List of MapBlocks in the Local Layer
			*/
		AutoSharedPtr<MapBlockIndexList>       GetLocalLayerMapBlockIndices(); 

        /*!@brief Get List of MapBlocks in the FwyLayerFull Layer
		*/
        AutoSharedPtr<MapBlockIndexList>       GetFwyLayerFullMapBlockIndices();

        /*!@brief Get List of MapBlocks in the FwyThin Layer
		*/
        AutoSharedPtr<MapBlockIndexList>       GetFwyLayerThinMapBlockIndices();

    
    private:
         
		AutoSharedPtr<MapBlockIndexList>  m_LocalLayerMapBlockIndices;	

        AutoSharedPtr<MapBlockIndexList>  m_FwyLayerFullMapBlockIndices;

        AutoSharedPtr<MapBlockIndexList> m_FwyLayerThinMapBlockIndices;

		friend class   RouteEngine;
};
/*!
   RouteStartTime is a  structure which stores all the information about the exact time the route begins

*/
struct NCDB_API RouteStartTime
{
    float   m_Time; // in GMT = (float)hrs + ((float) Min)/60.F + ((float)Sec)/3600.F;
	int     m_DOW;  // 0 = Sunday to 6 = Saturday
	int     m_Day;  // day of the month
	int     m_Month;// month of the year
	int     m_Year; // year

    RouteStartTime()
    {
        Clear();
    }

    void Clear()
    {
        m_Time = -1.0f;
        m_DOW = m_Day = m_Month = m_Year = 0;
    }

    void SetTime(float time, int dow, int day, int month, int year)
    {
        m_Time = time; m_DOW = dow; m_Day = day; m_Month = month; m_Year = year;
    }
};

/*! Route is the least cost path between two points subject to cost model and constraints
  * Contains Source ,Destination Connection and Address and a list of road segments which form the route
  * Has a state ,unique ID and length
*/

class NCDB_API Route
{
public:
	//!Default Constructor
	Route();

	//!With a server generated ID.
	Route(long routeID);

    //!A Route objected is created by the RouteEngine Class 
    // @param[in] originConnection
    // @param[in] destConnection
    // @param[in] RouteID
    // @param[in] routeSegmentsList
    //Route( const Connection& originConnection
    //     , const Connection& destConnection
    //     , long RouteID
    //     , AutoSharedPtr<const RouteSegmentList> routeSegmentsList );

    ~Route();

	void  Reset ();

    //! Set the destination Address no. Used to determine the destination arrival maneuver (left or right)
	// @param[in] addrNum
	// @return ReturnCode -NCDB_OK
	//                    -NCDB_FAIL
	ReturnCode          SetDestAddressNumber(int addrNum);

    //! Get the destination Address no. Used to determine the destination arrival maneuver (left or right)
	// @return  m_AddrNumber
	int                 GetDestAddressNumber() const;

    //! New feature: support use Address as destination
    ReturnCode          SetDestAddress(const Ncdb::InAddress& address);
    const Ncdb::InAddress&    GetDestAddress() const;

    //!Set Start Time
	// @param[in] startTime
	// @return ReturnCode -NCDB_OK
	//                    -NCDB_FAIL
    ReturnCode          SetStartTime( const RouteStartTime& startTime );
    
    //!Get Start Time
	// @return RouteStartTime -m_RteStartTime
    const
    RouteStartTime&     GetStartTime() const;

	//!Bool to set generation of the mapblock Indices for a route

	void  SetWantMapBlockIndices(bool wantMBIndices);


	//!Get length of the Route in mts
	float               GetRouteLength() const;

	//!Set length of the Route in mts
    void                SetRouteLength (float length);

    //!@brief Get total number of segments
	// @return NumberOfSegments
	int                 GetNoOfRoadSegmentsinRoute() const;

	//!@brief Get the segment List
	// @return - AutoSharedPtr<RoadSegmentIDList> -pointer to list of RouteSegments
	AutoSharedPtr<const RouteSegmentList> GetRouteSegmentList() const;
    void SetRouteSegmentList(const AutoSharedPtr<RouteSegmentList> segList);

    void                    SetRouteSegment( int idx, const RouteSegment& seg );
    const RouteSegment&     GetRouteSegment( int idx ) const;

	//!Get the shapePoint List
    WorldPointList&         GetRoutePolyline()          { return mp_RtePoly; }
    const WorldPointList&   GetRoutePolyline()    const { return mp_RtePoly; }

    //! Get a point suitable for placing a route label (different from other
    // routes)
    WorldPoint&             GetLabelPoint()          { return m_LabelPoint; }
    const WorldPoint&       GetLabelPoint() const    { return m_LabelPoint; }

    //! Get a bounding box for portion of route that is relevent to the label point
    // (part that is different from other routes and used to determine label point)
    void                    GetLabelBoundingBox(WorldRect & rect) const;

    //!Set the route polyline
	ReturnCode                          SetRoutePolyline( const WorldPointList& rtePoly );

    //!Get the maneuver List
	AutoSharedPtr<ManeuverList>         GetManeuverList();

    //!Get the maneuver List
	AutoSharedPtr<const ManeuverList>   GetManeuverList() const;

    //!Get the Intermediate Point List
    WorldPointList &                    GetIntermediatePointList();

    //!Get the Intermediate connections
    AutoSharedPtr<const ConnectionList> GetIntermediateConnections() const;

    //!Get Route ID
    long            GetRouteID() const;

    //!Get the Estimated travel time in Sec
    double          GetEstimatedTravelTime() const;

    //!Set the Estimated travel time in Sec
    void            SetEstimatedTravelTime (float time);

    //!Get the unadjusted travel time in Sec
    double          GetRawTravelTime() const;

    //!Check if bridges occur in route
    //   @return bool
    bool            IsBridgeInRoute() const;

    //!Check if tunnels occur in route
    //   @return bool
    bool            IsTunnelInRoute() const;

    //!Check if tollroads occur in route
    //   @return bool
    bool            IsTollRoadInRoute() const;

    //!Check if highways occur in route
    //   @return bool
    bool            IsHwyInRoute() const;

    //!Check if ferrys occur in route
    //   @return bool
    bool            IsFerryInRoute() const;

    //!Check if TMC occur in route
    //   @return bool
    bool            IsTMCInRoute() const;

    //!Check if unpaved occur in route
    //   @return bool
    bool            IsUnpavedInRoute() const;

    //!Check if HOV occur in route
    //   @return bool
    bool            IsHOVInRoute() const;

    //!Check if private occur in route
    //   @return bool
    bool            IsPrivateRoadInRoute() const;

    //!Check if new country occur in route
    //   @return bool
    bool            IsEnteringCountryInRoute() const;

    //!Check if Gate occur in route
    //   @return bool
    bool            IsGateAccessInRoute() const;

    //!Get hasFlags for route
    //   @return bool
    int             GetHasFlags() const;
    void            SetHasFlags(int flags);

    //!Get internal route cost
    //   @return float
    float           GetInternalCost() const {return m_InternalCost;};

    //!Set internal route cost
    void            SetInternalCost(float cost) {m_InternalCost=cost;};

    //!Get coomon lentgh with bsae route
    //   @return float
    float           GetCommonLengthRatio() const {return m_CommLengRatio;};

    //!@brief Set the user Options used for calculating the route
    // @return - ReturnCode -NCDB_OK
    //                      -NCDB_FAIL
    ReturnCode      SetRoutingOptions( const RoutingOptions& rteOptions );  

    //!@brief Get the user Options used for calculating the route
    // @return - RoutingOptions 
    const RoutingOptions & GetRoutingOptions()const;  


	//!@brief Set the user Options used for Maneuver generate
	// @return - ReturnCode -NCDB_OK
	//                      -NCDB_FAIL
	ReturnCode SetManeuverProperties(const ManeuverProperties &maneuverProperties);

	//!@brief Get the user Options used for calculating the route
	// @return - ManeuverProperties *
	ManeuverProperties * GetManeuverProperties();


    //!@brief Set the user alternate route options used for calculating the route
    // @return - ReturnCode -NCDB_OK
    //                      -NCDB_FAIL
    ReturnCode      SetAltRouteOptions( const AltRouteOptions& altOpts );  

    //!@brief Get the user alternate route options used for calculating the route
    // @return - AltRouteOptions 
    AltRouteOptions  GetAltRouteOptions()const;  

    //! Set the Origin Connection
    // @param[in] connection
    // @return - ReturnCode -NCDB_OK 
    //                      -NCDB_FAIL
    ReturnCode      SetOriginConnection( const Connection &connection );

    //! Get the Origin Connection
    //  @return - Connection
    const
    Connection&     GetOriginConnection() const;

    //!Set the Destination Connection
    // @param[in] connection
    // @return - ReturnCode -NCDB_OK 
    //                      -NCDB_FAIL
    ReturnCode      SetDestConnection( const Connection& connection );

    //!Get the Destination Connection
    // @return-Const Connection
    const 
    Connection&     GetDestinationConnection() const;

    //!Check if the StartTime is Set
    //   @return bool
    bool            IsTimeSet() const;

    //! Check equality of routes
    // @param[in] route1
    // @param[in] route2
    // @return True-Same,False-not equal
    //bool            RouteCompare( const Route& route1, const Route& route2 );

    //! To merge 2 routes 
    // @param[in] route1
    // @param[in] route2
    // @param[out] resroute
    // @return -ReturnCode-NCDB_OK
    //                    -NCDB_FAIL
    //ReturnCode      MergeRoutes( const Route& route1, const Route& route2, Route& resroute );

    //!Append a route route = route + route1
    // @param[in] route1
    // @return -ReturnCode-NCDB_OK
    //                    -NCDB_FAIL
    // Has bugs in filling RoadInfo structure and vxData. 
    // Use method RouteEngine::AppendRoute instead
    ReturnCode      AppendRoute( const Route &route1, bool recalcPolicy );

    //!Prepend a Route route1 = route2+route1
    // @param[in] route1
    // @param[in] route2
    // @return -ReturnCode-NCDB_OK
    //                    -NCDB_FAIL
    //ReturnCode      PrependRoute( Route& route1, const Route& route2 );

    //!Finding the closest point on the current route to current location. 
    // @param[in] curPoint
    // @param[in] ptOnRoute - point on route which is closest to the current point
    // @return -ReturnCode-NCDB_OK
    //                    -NCDB_FAIL
    //ReturnCode      GetClosestPointToRoute( const WorldPoint& curPoint, WorldPoint& ptOnRoute ) const;


    AutoSharedPtr<EnNVObjectList> GetEnNVObjectList() const
    {
        return mp_EnNVObjectList;
    }
    AutoSharedPtr<EnNVObjectList> GetEnNVObjectList()
    {
        return mp_EnNVObjectList;
    }

    AutoSharedPtr<SpeedRegionList> GetSpeedRegionList() const
    {
        return mp_SpeedRegionList;
    }
	AutoSharedPtr<SpeedRegionList> GetSpeedRegionList()
	{
		return mp_SpeedRegionList;
	}

    AutoSharedPtr<SpecialRegionList> GetSpecialRegionList() const
    {
        return mp_SpecialRegionList;
    }
    AutoSharedPtr<SpecialRegionList> GetSpecialRegionList()
    {
        return mp_SpecialRegionList;
    }

    RouteMapBlocks GetMapBlocksForRoute()
	{
		return m_RouteMapBlock ;
	}

    void SetDestStreetName (const UtfString & destName)
    {
        m_destStreetName = destName;
    }

    UtfString GetDestStreetName () const
    {
        return m_destStreetName;
    }

	//!Returns the first major Road of the route
	ReturnCode   GetFirstMajorRoad(RoadInfo &mjrRd,CountryInfo &cInfo);

private:

    Connection      m_origConnection;   	//Source Info
    Connection      m_destConnection;       //Destination Info

    UtfString       m_destStreetName;       // For dest validation

    //!Destination Address (currently mobius does not support alpha numeric)
    int             m_AddrNumber; 

    //! Destination Address
    Ncdb::InAddress       m_Address;    

	bool            m_WantMapBlockIndices;
    //!List of the ShapePoints which  forms the Route
    WorldPointList  mp_RtePoly;

    WorldPointList  mp_IntermediatePointsList;

    WorldPoint      m_LabelPoint;
    WorldRect       m_LabelRegion;
    
    //!List of the RouteSegments
    AutoSharedPtr<RouteSegmentList>     mp_RouteSegmentsList;  
    
    AutoSharedPtr<SpecialRegionList>    mp_SpecialRegionList;

    //!List of Maneuvers
    AutoSharedPtr<ManeuverList>         mp_ManeuverList;

    //Intermediate Point Connection List
    AutoSharedPtr<ConnectionList>       mp_IntermediateConnections;
	//! list of MJO/ECM
	AutoSharedPtr<EnNVObjectList>       mp_EnNVObjectList;
	//! list of SpeedLimits
	AutoSharedPtr<SpeedRegionList>      mp_SpeedRegionList;
   
	//! Structure which holds the list of MapBlockIndices 
	RouteMapBlocks  m_RouteMapBlock;

    //Attributes
    float               m_RouteLength;
    int                 m_NoOfRdSegsinRoute;
    long                m_RouteID;
    float               m_TotalTimeInSec;       // Travel time in secs
    float               m_AdjTotalTimeInSec;    // Adjusted travel time in secs

    //state info
    RoutingOptions      m_RteOpts;
	ManeuverProperties  m_ManeuverProperties;
    RouteStartTime      m_RteStartTime;
    AltRouteOptions     m_altOpts;
    float               m_InternalCost;

    int                 m_HasFlags;
    float               m_CommLengRatio;
    int                 m_nRetry;
     
    friend class   RouteEngine;

}; // class Route

typedef AutoArray<Route> RouteList;
#ifdef _MSC_VER
template class NCDB_API AutoSharedPtr<RouteList>;
#endif

// Interface structure for fast ETA calculation of multiple destinations
struct NCDB_API FastETA
{
    FastETA() : m_len (0.0f), m_eta (-1.0f), m_HasFlags(0), m_RetCode(NCDB_FAIL){}

    WorldPoint  m_loc;  // Input destination location

    float       m_len;  // Output route length
    float       m_eta;  // Output route ETA
    int         m_HasFlags;
    AutoSharedPtr<RouteSegmentList> mp_RouteSegmentList;
    ReturnCode  m_RetCode;

};

} // namespace Ncdb

#endif // NCDB_ROUTE_H


