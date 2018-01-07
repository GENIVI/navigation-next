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
#ifndef NCDB_USER_OPTIONS_H
#define NCDB_USER_OPTIONS_H

#include "NcdbTypes.h" 
#include "Segment.h"
#include "AutoSharedPtr.h"
#include <string>


namespace Ncdb 
{

enum VehicleType 
{
    Car = 0,
    Truck,
    Bicycle,
    Pedestrian
};

enum RouteOptimization 
{
    RO_Fastest = 0,
    RO_Shortest,
    RO_Easiest,
};

enum RoutingFlags_e
{
    AVOID_TOLL              = 1,
    AVOID_FERRY             = 2,
    AVOID_HWY               = 4,
    AVOID_UNPAVEDROADS      = 8,
    AVOID_UTURN             = 16,
    AVOID_HOV               = 32,
    AVOID_TRAFFICINCIDENTS  = 64,
    AVOID_CONSTRUCTIONAREA  = 128,
    AVOID_HOV_ALL_TIME      = 256,
    USE_TRAFFICINFO_REAL    = 1024, // REAL
    USE_TRAFFICINFO_HIST    = 2048, // HISTORICAL
    NO_MANEUVER_LIST        = 4096, // Suppress maneuver generation
};


/** Class used to keep track of all the user preferences in one place
 * This will be also used to set some of the Routing Engines Control parameters
 */
class NCDB_API RoutingOptions
{
public:
    //default constructor
    RoutingOptions();
	RoutingOptions(const RoutingOptions&);

    ~RoutingOptions();

    //!@brief Set Routingoptions given another RoutingOptions object
    void    SetRoutingOptions(const RoutingOptions& rteOpts);         

    //!Set the Vehicle Type,Car,truck or pedestrian
    //@param[in] vehType
    void    SetVehicleType(VehicleType vehType);

    //!Return the Vehicle Type
    //@return vehType
    VehicleType GetVehicleType()const;

    //!Set the Nature of the route fastest,shortest or simplest
    //@param[in] rtOpt
    void    SetRouteOptimization(RouteOptimization rtOpt);

    //!Get the nature of the Route fastest,shortest or simplest
    //@return rtOpt
    RouteOptimization GetRouteOptimization()const;

    //!Set/Get user language code
    void    SetUserLanguage(SupportedLanguages language) { m_userLanguage = language; };
    SupportedLanguages GetUserLanguage() const { return m_userLanguage; };

    //!Avoidances

    //!Get the Avoidance of Toll option
    //@return avoidToll
    bool    GetAvoidToll()                const { return 0 != (m_flags & AVOID_TOLL); }
    //!Get the Avoidance of Ferry option
    //@return avoidFerry
    bool    GetAvoidFerry()               const { return 0 != (m_flags & AVOID_FERRY); }
    //!Get the Avoidance of HOV roads option
    //@return avoidHOV option
    bool    GetAvoidHOV()                 const { return 0 != (m_flags & AVOID_HOV); }
    //!Get the Avoidance of HWY
    //@return avoidHwy
    bool    GetAvoidHwy()                 const { return 0 != (m_flags & AVOID_HWY); }
    //!Get the Avoidance of Unpaved roads option
    //@return avoidUnpavedRoads
    bool    GetAvoidUnpavedRoads()        const { return 0 != (m_flags & AVOID_UNPAVEDROADS); }
    //!Get the Avoidance of Uturn option
    //@return avoidUturn
    bool    GetAvoidUturn()               const { return 0 != (m_flags & AVOID_UTURN); }
    //!Get the Avoidance of Traffic incidents option
    //@return avoidTrafficIncidents
    bool    GetAvoidTrafficIncidents()    const { return 0 != (m_flags & AVOID_TRAFFICINCIDENTS); }
    //!Get the Avoidance of Construction area option
    //@return avoidConstructionArea
    bool    GetAvoidConstructionArea()    const { return 0 != (m_flags & AVOID_CONSTRUCTIONAREA); }

    bool    GetUseRealTraffic()           const { return 0 != (m_flags & USE_TRAFFICINFO_REAL); }
    bool    GetUseHistoricalTraffic()     const { return 0 != (m_flags & USE_TRAFFICINFO_HIST); }
    //!Get whether the HOV based on DTMs should be avoided at all time
	//@return avoidHOVatallTime 
	bool GetAvoidHOVAtAllTime() const { return 0!= (m_flags & AVOID_HOV_ALL_TIME);} 

    bool    GetNoManeuverList()           const { return 0 != (m_flags & NO_MANEUVER_LIST); }

    //!Set the Avoidance of Toll option(true or false)
    //@param[in] avoidToll
    void    SetAvoidToll( bool v )              { h_set_flag( AVOID_TOLL, v ); }
    //!Set the Avoidance of Ferry option(true or false)
    //@param[in] avoidFerry
    void    SetAvoidFerry( bool v )             { h_set_flag( AVOID_FERRY, v ); }
    //!Set the Avoidance of Ferry option(true or false)
    //@param[in] avoidHOV
    void    SetAvoidHOV( bool v )               { h_set_flag( AVOID_HOV, v ); }
    //!Set the Avoidance of HWY(use local roads only)
    //@param[in] avoidHwy
    void    SetAvoidHwy( bool v )               { h_set_flag( AVOID_HWY, v ); }
    //!Set the Avoidance of Unpaved roads option(true or false)
    //@param[in] avoidUnpavedRoads
    void    SetAvoidUnpavedRoads( bool v )      { h_set_flag( AVOID_UNPAVEDROADS, v ); }
    //!Set the Avoidance of Uturn option(true or false)
    //@param[in] avoidUturn
    void    SetAvoidUturn( bool v )             { h_set_flag( AVOID_UTURN, v ); }
    //!Set the Avoidance of Traffic incidents option(true or false)
    //@param[in] avoidTrafficIncidents
    void    SetAvoidTrafficIncidents( bool v )  { h_set_flag( AVOID_TRAFFICINCIDENTS, v ); }
    //!Set the Avoidance of Construction area option(true or false)
    //@param[in] avoidConstructionArea
    void    SetAvoidConstructionArea( bool v )  { h_set_flag( AVOID_CONSTRUCTIONAREA, v ); }

    void    SetUseRealTraffic( bool v )         { h_set_flag( USE_TRAFFICINFO_REAL, v ); }
    void    SetUseHistoricalTraffic( bool v )   { h_set_flag( USE_TRAFFICINFO_HIST, v ); }
	/*! Set the Avoidance of HOV at all times 
	    Some lanes are HOV only during certain hours of the day. Set this to true if
	    travel on them at any time is to be avoided.
		Default is True...HOV (DTM based )lanes will  be avoided during the times
		they are NOT HOV. 
	   *** IMP AvoidHOV still needs to be set to true.
	  */
	void SetAvoidHOVAtAllTime(bool v){h_set_flag( AVOID_HOV_ALL_TIME, v );}

    void    SetNoManeuverList( bool v )   { h_set_flag( NO_MANEUVER_LIST, v ); }

    void    SetTrafficProvider( const char* provider );

    const 
    char*   GetTrafficProvider() const;

    //!Store the list of Ids of segments with cost that have to be avoided while calculating the route
    ReturnCode SetAvoidSegmentsWithCost(const AutoSharedPtr<NodeSegmentWithCostList> nodeSegmentList);

    //!Return the list of Ids RoadSegments to be avoided
    const AutoSharedPtr<NodeSegmentWithCostList> 
            GetAvoidSegmentsWithCost() const;

    //!Store the list of Ids of segments that have to be avoided while calculating the route
    ReturnCode SetAvoidRoadSegments(const AutoSharedPtr<RoadSegmentIDList> roadSegmentsList);

    //!Return the list of Ids RoadSegments to be avoided
    const AutoSharedPtr<RoadSegmentIDList> 
            GetAvoidRoadSegments() const;


    //! Maximum distance to stack neighbour maneuvers
    void    SetMaxDistToStack( float fDistance );

    float   GetMaxDistToStack() const;

    //! Equal operator
    bool operator == ( const RoutingOptions& optns ) const;

	//! copy operator
	RoutingOptions& operator = ( const RoutingOptions& optns );

private:
    void h_set_flag( int flag, bool v )
    {
        if( v ) m_flags |= flag;
        else    m_flags &= ~flag;
    }

    //!Vehicle type
    VehicleType m_vehicletype;

    //!Nature of the route(Route Optimization)
    RouteOptimization m_routeopt;

    //!User language for routing
    SupportedLanguages m_userLanguage;

    int         m_flags;

    //!For Explication
    float       m_fMaxDistToStack;


    char*       m_TrafficProvider;
    
    //!List of segments to be avoided 
    AutoSharedPtr<RoadSegmentIDList>   mp_AvoidRoadSegmentIDsList; 

    AutoSharedPtr<NodeSegmentWithCostList>   mp_AvoidSegmentWithCostList; 

	//!Options for Corridor Routing
	bool m_WantCorridorRouting;
};

// Options used for computing alternate routes from a base route.
struct NCDB_API AltRouteOptions
{
    AltRouteOptions() {
        m_AvoidTraffic = false;
        m_AvoidHwy = false;
        m_AvoidToll = false;
        m_AvoidBridge = false;
        m_AvoidTunnel = false;
        m_AvoidFerry = false;
        m_OtherAltRoute = false;
        m_UseRouteGuide = false;
        m_MaxCostFactor = 0.0f;
        m_MaxTimeCost = 0.0f;
        m_TmcSpeedFactor = 0.0f;
        m_MaxCommonLength = 1.0;
        m_OtherFraction = 0.5f;
        m_HwyFraction = 1.0f;
        m_MaxFraction = 1.0f;
        m_UseVarCost = true;
        m_MaxDistFact = 0.0f;
        m_MaxTimeFact = 0.0f;
        m_MinSimFact = 0.0f;
        m_MinBaseLeng = 0.0f;
        m_MaxDistDiff = 0.0f;
        m_MaxTimeDiff = 0.0f;
    };

    ~AltRouteOptions(){};

    bool m_AvoidTraffic;    // Avoid traffic along base route
    bool m_AvoidHwy;        // Avoid highways in base route
    bool m_AvoidToll;       // Avoid toll roads in base route
    bool m_AvoidBridge;     // Avoid bridges in base route
    bool m_AvoidTunnel;     // Avoid tunnel in base route
    bool m_AvoidFerry;      // Avoid ferry in  base route
    bool m_OtherAltRoute;   // Avoid most of base route
    bool m_UseRouteGuide;   // Follow a route guide if possible

    // Cost limits on alternate route is the minimum of the following:

    // Upper bound on alt route cost based on multiplicative factor of
    // cost estimate of base route.
    // If <= 1.0 no limit.
    float m_MaxCostFactor;

    // Upper bound on alt route cost based on addition to base route cost
    // expressed in time units (minutes).
    // If <= 0.0 no limit.
    float m_MaxTimeCost;

    // If Avoid traffic, then avoid links when TMC speed is less than cost
    // factor of normal link speed.
    // If <= 0.0 don't avoid, If >= 1.0 always avoid.
    float m_TmcSpeedFactor;

    // Acceptance criteria for an alternate route is difference from base route.

    // Max ratio of base route length that can match a route and be accepted.
    // If <= 0.0, no overlap, if >=1.0 no limit.
    float m_MaxCommonLength;

    // Fraction of route used for OtherAltRoute option.
    float m_OtherFraction;
    // Fraction of hwy segs used for AvoidHwy and AvoidToll options.
    float m_HwyFraction;
    // Fraction of detour cost for max variable cost.
    float m_MaxFraction;
    // Use variable cost avoidance for OtherAltRoute, AvoidHwy, AvoidToll.
    bool m_UseVarCost;
    // Max distance factor relative to base length (0 is none).
    float m_MaxDistFact;
    // Max time factor relative to base time (0 is none).
    float m_MaxTimeFact;
    // Min similarity ratio (0 is none).
    float m_MinSimFact;
    // Min base length in miles required for alternates (0 is none).
    float m_MinBaseLeng;
    // Max distance increase in miles for alternates (0 is none).
    float m_MaxDistDiff;
    // Max time increase in minutes for alternates (0 is none).
    float m_MaxTimeDiff;
};


// Algorithm priority values to determine what actions to apply and in what order.
enum MR_Priority {
    MR_EXCLUDE  = 0 ,   // Don't use
    MR_PRI_1,           // 1st priority
    MR_PRI_2,           // 2nd priority
    MR_PRI_3,           // ...
    MR_PRI_4,
    MR_PRI_5,
    MR_PRI_6,
    MR_PRI_7,
    MR_PRI_8,
    MR_PRI_MAX
};

struct NCDB_API MultiRouteOptions
{
    MultiRouteOptions()
    {
        m_MaxNumRoutes = 2;
        m_MaxNumDetours = 2;
        m_UseEngineDefaults = true;
        m_AvoidTraffic  = MR_PRI_1;      // Try these avoid options in this order
        m_OtherAltRoute = MR_PRI_2;
        m_AvoidToll     = MR_PRI_3;
        m_AvoidTunnel   = MR_PRI_4;
        m_AvoidBridge   = MR_PRI_5;
        m_AvoidFerry    = MR_PRI_6;
        m_AvoidHwy      = MR_PRI_7;
        m_UseRouteGuide = MR_EXCLUDE;
        m_MaxCostFactor = -1.0f;         // No limit
        m_MaxTimeCost = -1.0f;           // No limit
        m_TmcSpeedFactor = 0.0f;         // No limit
        m_MaxCommonLength = 1.0;         // No limit
        m_OtherFraction = 0.5f;
        m_HwyFraction = 1.0f;
        m_MaxFraction = 1.0f;
        m_UseVarCost = true;
        m_TryRelaxAvoids = false;
        m_TryOtherDefault = false;
        m_UseTrafficAvoids = false;
        m_MaxDistFact = 0.0f;
        m_MaxTimeFact = 0.0f;
        m_MinSimFact = 0.0f;
        m_MinBaseLeng = 0.0f;
        m_MaxDistDiff = 0.0f;
        m_MaxTimeDiff = 0.0f;
        m_DetourFraction = 0.25f;
        m_DetourLength = 10.0f;
        m_LeadTime = 30.0f;
    };

    ~MultiRouteOptions(){};

    int m_MaxNumRoutes;            // Max numnber of routes to compute
    int m_MaxNumDetours;           // Max numnber of detours to compute
    bool m_UseEngineDefaults;      // Use engine defined default setting
    MR_Priority m_AvoidTraffic;    // Avoid traffic along base route
    MR_Priority m_AvoidHwy;        // Avoid highways in base route
    MR_Priority m_AvoidToll;       // Avoid toll roads in base route
    MR_Priority m_AvoidBridge;     // Avoid bridges in base route
    MR_Priority m_AvoidTunnel;     // Avoid tunnel in base route
    MR_Priority m_AvoidFerry;      // Avoid ferry in  base route
    MR_Priority m_OtherAltRoute;   // Avoid most of base route
    MR_Priority m_UseRouteGuide;   // Follow a route guide if possible

    // Cost limits on alternate route is the minimum of the following:

    // Upper bound on alt route cost based on multiplicative factor of
    // cost estimate of base route.
    // If <= 1.0 no limit.
    float m_MaxCostFactor;

    // Upper bound on alt route cost based on addition to base route cost
    // expressed in time units (minutes).
    // If <= 0.0 no limit.
    float m_MaxTimeCost;

    // If Avoid traffic, then avoid links when TMC speed is less than cost
    // factor of normal link speed.
    // If <= 0.0 don't avoid, If >= 1.0 always avoid.
    float m_TmcSpeedFactor;

    // Acceptance criteria for an alternate route is difference from base route.

    // Max ratio of base route length that can match a route and be accepted.
    // If <= 0.0, no overlap, if >=1.0 no limit.
    float m_MaxCommonLength;

    // Fraction of route used for OtherAltRoute option.
    float m_OtherFraction;
    // Fraction of hwy segs used for AvoidHwy and AvoidToll options.
    float m_HwyFraction;
    // Fraction of detour cost for max variable cost.
    float m_MaxFraction;
    // Use variable cost avoidance for OtherAltRoute, AvoidHwy, AvoidToll.
    bool m_UseVarCost;
    // Try to relax user avoids if needed to get an alternate route.
    bool m_TryRelaxAvoids;
    // Try to use OtherAltRoute as a default if needed to get remaning alternates.
    bool m_TryOtherDefault;
    // Add avoid penalty for traffic on base route.
    bool m_UseTrafficAvoids;
    // Max distance factor relative to base length (0 is none).
    float m_MaxDistFact;
    // Max time factor relative to base time (0 is none).
    float m_MaxTimeFact;
    // Min similarity ratio (0 is none).
    float m_MinSimFact;
    // Min base length in miles required for alternates (0 is none).
    float m_MinBaseLeng;
    // Max distance increase in miles for alternates (0 is none).
    float m_MaxDistDiff;
    // Max time increase in minutes for alternates (0 is none).
    float m_MaxTimeDiff;
    // Max initial fraction of route to avoid for detours.
    float m_DetourFraction;
    // Max initial length of route to avoid for detours.
    float m_DetourLength;
    // Lead time for start of detour on current route.
    float m_LeadTime;
};

struct NCDB_API ReturnToRouteOptions
{
    ReturnToRouteOptions()
    {
        m_MinLength = 2000;
        m_ProjLength = 1000;
        m_DestDist = 3000;
        m_MaxRetry = 2;
        m_UseEngineDefaults = true;
    };

    ~ReturnToRouteOptions(){};

    bool m_UseEngineDefaults;      // Use engine defined default settings

    // Minimum length of route from closest segment to destination
    // expressed in meters
    // Otherwise just compute a new route
    float m_MinLength;

    // Distance from closest route polyline along route to join segment
    // expressed in meters.
    float m_ProjLength;

    // Distance from origin to destination expressed in meters.
    float m_DestDist;

    // Max calls to RTR for a given route.
    int m_MaxRetry;     // non hwy
    int m_MaxRetry2;    // hwy
};


};

#endif
