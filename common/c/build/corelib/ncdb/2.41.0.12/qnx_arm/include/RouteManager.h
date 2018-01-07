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

    @file     filename.h
    @date     06/06/2009
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

#ifndef NCDB_ROUTE_MANAGER_H
#define NCDB_ROUTE_MANAGER_H
#include "UserOptions.h"
#include "Route.h"
#include "CorridorRoute.h"
#include "Maneuver.h"
#include "ManeuverOptions.h"
#include "RouteGuide.h"
#include "RoutingHooks.h"

namespace Ncdb {

#ifdef _MSC_VER
    typedef unsigned __int64 timer_val;
#else
    typedef unsigned long long timer_val;
#endif // _MSC_VER

class RouteEngine; // internal implementation of RouteManager.
class Session;
class Address;


struct PerfomanceInfo
{
    timer_val m_GetOrigin;
    timer_val m_GetDestination;
    timer_val m_RouteCalculation;
    timer_val m_RouteBuilding;
    timer_val m_ManeuverGeneration;
    timer_val m_PolylineGeneration;
};

/*! @brief High Leve Interface API class for Route and Maneuver Generation
*/
class NCDB_API RouteManager
{
public:
	/*! @brief Constructor 
	*/
    RouteManager(Session& session);

    virtual ~RouteManager();
	/*!Initialise the Traffic Reader */
    ReturnCode InitializeTrafficReader();

	/*! Close the traffic reader */
    ReturnCode CloseTrafficReader();

	/*! Reset the traffic reader */
    ReturnCode ResetTrafficReader();

	/*! @brief Read in the historical db for a specific provider*/
	ReturnCode ReadHistoricTrafficByProvider(const char * provider,const char *pathToHistDB);

	/*! @brief Update Real Time Data*/
    ReturnCode UpdateRealTimeDataByProvider(const char * provider);
    
	/*! @brief Update Incident Data for a specific provider*/
    ReturnCode UpdateIncidentDataByProvider(const char * provider);
   
	/*! @brief Set the Maneuver Preferences
    */
    void SetManeuverOptions(const ManeuverOptions& optns /*!<[in] User/Application set preferences  */);

    void SetMultiRouteOptions(const MultiRouteOptions& optns /*!<[in] User/Application set preferences  */);

    /*! @brief Calculates a route given the Source and Destination lat lon and route options

        @return NCDB_OK         - Route found
                NCDB_INVALID    - Invalid input parameters
                NCDB_FAIL       - Route could not be found
    */
    ReturnCode CalculateRoute(const RoutingOptions &routeOpts,  /*!<[in] User/Application set preferences */
        const WorldPoint &originPt, /*!<[in] Starting point based on the GPS fix */
        const WorldPoint &destPt,   /*!<[in] Destination point as obtained from Geocoding */
        float heading,              /*!<[in] Direction of travel if the car is in motion */
		float speed,                /*!<[in] Speed with which the Client is travelling */
        Route &route                /*!<[out] Route Object with a list of RoadsegmentIDs,origin and destConnections which form the route */
        ); 

	/*! @brief Calculates Corridor Routes for  given the Source and Destination lat lon and prev calculated route 

        @return NCDB_OK         - Corridor Routes found
                NCDB_INVALID    - Invalid input parameters
                NCDB_FAIL       - C.Routes could not be found
    */
    CorridorRouteList CalculateCorridorRoutes(const RoutingOptions &routeOpts,  /*!<[in] User/Application routing preferences */
		const ManeuverOptions& manOptns,/*!<[in] Maneuver preferences  */
        const WorldPoint &originPt, /*!<[in] Starting point based on the GPS fix */
        const WorldPoint &destPt,   /*!<[in] Destination point as obtained from Geocoding */
        float heading,              /*!<[in] Direction of travel if the car is in motion */
		float speed,                /*!<[in] Speed with which the Client is travelling */
        const Route &route             /*!<[in/out]  route */
        ); 

    /*! @brief Calculates a route given the Source and Destination lat lon and route options

        @return NCDB_OK         - Route found
                NCDB_INVALID    - Invalid input parameters
                NCDB_FAIL       - Route could not be found
    */
    ReturnCode CalculateAlternateRoute(const Route &baseRoute,  /*!<[in] Original route with routing preferences */
        const WorldPoint &originPt,     /*!<[in] Starting point based on the GPS fix */
        float heading,                  /*!<[in] Direction of travel if the car is in motion */
        float speed,                    /*!<[in] Speed with which the Client is travelling */
        const AltRouteOptions &altOpts, /*!<[in] Option flags for alternate route */
        Route &route                    /*!<[out] Route Object with a list of RoadsegmentIDs,origin and destConnections which form the route */
        ); 

    /*! @brief get multi-route default options

        @return MiltiRouteOptions
    */
    MultiRouteOptions GetMultiRouteDefaultOptions () const;

    /*! @brief Calculates a set of routes given the Source and Destination lat lon and route options

        @return NCDB_OK         - Routes found
                NCDB_INVALID    - Invalid input parameters
                NCDB_FAIL       - Routes could not be found
    */
    ReturnCode CalculateMultiRoutes (
        const RoutingOptions &routeOpts,    /*!<[in] User/Application routing preferences */
        const WorldPoint &originPt,         /*!<[in] Starting point based on the GPS fix */
        const WorldPoint &destPt,           /*!<[in] Destination point as obtained from Geocoding */
        float heading,                      /*!<[in] Direction of travel if the car is in motion */
        float speed,                        /*!<[in] Speed with which the Client is travelling */
        AutoArray<Route> &routes            /*!<[out] Route Objects */
        ); 

    /*! @brief Calculates a set of routes given the Source and Destination lat lon and route options

        @return NCDB_OK         - Routes found
                NCDB_INVALID    - Invalid input parameters
                NCDB_FAIL       - Routes could not be found
                NCDB_NO_ALTERNATE - No alternate route found
    */
    ReturnCode CalculateDetourRoutes (
        const Route &prevRoute,             /*!<[in] Previous route */
        const RoadSegmentIDList & detourSegs,     /*!<[in] Detour segments */
        const WorldPoint &originPt,         /*!<[in] Starting point based on the GPS fix */
        float heading,                      /*!<[in] Direction of travel if the car is in motion */
        float speed,                        /*!<[in] Speed with which the Client is travelling */
        AutoArray<Route> &routes            /*!<[out] Route Objects */
        ); 

    /*! @brief Calculates ETA for a set of Destinations given a common Source

        @return NCDB_OK         - Routes found
                NCDB_INVALID    - Invalid input parameters
    */
    ReturnCode CalculateFastETA (
        const RoutingOptions &routeOpts,    /*!<[in] User/Application routing preferences */
        const WorldPoint &origPt,           /*!<[in] Starting point based on the GPS fix */
        float heading,                      /*!<[in] Direction of travel if the car is in motion */
        float speed,                        /*!<[in] Speed with which the Client is travelling */
        const RouteStartTime stime,         /*!<[In] Start time */
        AutoArray<FastETA>& fastETAList);   /*!<[in/out] Dest points/ETA */

    /*! @brief Calculates a route given the Source and Destination Connections and route options

        @return NCDB_OK         - Route found,
                NCDB_INVALID    - Invalid input parameters,
                NCDB_FAIL       - Route could not be found.
    */
    ReturnCode CalculateRoute(const RoutingOptions &routeOpts,  /*!< [in]User /Application set preferences */
        const Connection &originConnection, /*!<[in]Origin Connection*/
        const Connection &destConnection,   /*!<[in] Destination Connection */
        float heading,                      /*!<[in]Direction of travel if the car is in motion */
		float speed,                        /*!<[in] Speed with which the Client is travelling */
        Route &route                        /*!<[out]Route Object with a list of RoadsegmentIDs which form the route */
        );

	/*! @brief Calculates a route given the Source and DestinationCrossroad lat lon and route options

        @return NCDB_OK         - Route found
                NCDB_INVALID    - Invalid input parameters
                NCDB_FAIL       - Route could not be found
    */
    ReturnCode CalculateRouteToCrossroad(const RoutingOptions &routeOpts,  /*!<[in] User/Application set preferences */
        const WorldPoint &originPt, /*!<[in] Starting point based on the GPS fix */
        const WorldPoint &destPt,   /*!<[in] Destination point as obtained from Geocoding */
        float heading,              /*!<[in] Direction of travel if the car is in motion */
		float speed,                /*!<[in] Speed with which the Client is travelling */
        Route &route                /*!<[out] Route Object with a list of RoadsegmentIDs,origin and destConnections which form the route */
        ); 

    /*! @brief Return the limited segment count according to WndSize
    */
    int GetLimitSegmentCnt();

    /*! @brief Calculates the WorldPointList for a  given route.

        @return NCDB_OK
    */
    ReturnCode GenerateRoutePolyline(Route &route,  /*!< [in]Calculated route */
        WorldPointList& poly                        /*!< [out]Binary polyline structure */
        );

    /*! @brief Calculates the Bounding Box for a Route Polyline

        @return NCDB_OK
                NCDB_INVALID    - Route's poliline is empty
    */
    ReturnCode CalculateRoutePolylineBoundingBox(const Route &route/*!< [in]Route for which the BBX needs to be calculated */,
        WorldRect& boundBox/*!< [out]Bounding box */
        );

    /*! @brief

        @return NCDB_OK
    */
    ReturnCode GetOriginDestAddress(const Route &route,
        Address& address,
        bool isOrigin );

    /*! @brief Calculates the route passing through a list of points with the origin being 
        the first point and destination the last

        @return NCDB_OK         - Route found
                NCDB_INVALID    - Invalid input parameters
                NCDB_FAIL       - Route could not be found
    */
    ReturnCode CalculateRoutePassingThroughIntermediatePoints(const RoutingOptions &routeOpts/*!<[in] User /Application set preferences */,
        const WorldPoint &originPt,/*!<[in]Startingpoint obtained from GPS fix */
        const WorldPoint &destPt,/*!<[in] Destination point as obtained from Geocoding */
        const WorldPointList& pointList,/*!<[in] Points that need to lie along the route */
        float heading,/*!< [in]Direction of travel if the car is in motion */
		float speed, /*!<[in] Speed with which the Client is travelling */
        Route &route,  /*!< [out]Route Object with a list of RoadsegmentIDs which form the route */
        AutoArray<double>& dstList );
     /*! @brief Calculates the route by excluding segments along a previously calculated route 

        @return NCDB_OK         - Route found
                NCDB_INVALID    - Invalid input parameters
                NCDB_FAIL       - Route could not be found
    */

    ReturnCode CalculateRoutePassingThroughIntermediatePoints(const RoutingOptions &routeOpts/*!<[in] User /Application set preferences */,
        const WorldPoint &originPt,/*!<[in]Startingpoint obtained from GPS fix */
        const WorldPoint &destPt,/*!<[in] Destination point as obtained from Geocoding */
        const WorldPointList& pointList,/*!<[in] Points that need to lie along the route */
        float heading,/*!< [in]Direction of travel if the car is in motion */
		float speed, /*!<[in] Speed with which the Client is travelling */
        Route &route  /*!< [out]Route Object with a list of RoadsegmentIDs which form the route */);
     /*! @brief Calculates the route by excluding segments along a previously calculated route 

        @return NCDB_OK         - Route found
                NCDB_INVALID    - Invalid input parameters
                NCDB_FAIL       - Route could not be found
    */

    ReturnCode CalculateRouteExcludingPartOfARoute(const WorldPoint &originPt,/*!<[in]CurrentLocation of the car obtained from GPS fix */
		float heading,/*!< [in]Direction of travel if the car is in motion */
		RoutingOptions &routeOpts,/*!<[in] User /Application set preferences */
        float distFromPrevOrigin,/*!<[in]Distance from Startingpoint of the previous route where the detour begins*/
        float lengthOfDetour,/*!<[in] Length of the detour*/
        const Route &prevRoute, /*!< [in]Route Object with a list of RoadsegmentIDs which form the route */
		Route &Route/*![out] New Route generated excluding a portion of prevRoute*/);

	/*! @brief Calculates the route by excluding one or more Maneuevrs of a previously calculated route
	    @return NCDB_OK         - Route found
                NCDB_INVALID    - Invalid input parameters
                NCDB_FAIL       - Route could not be found
    */
	ReturnCode CalculateRouteExcludingManeuvers(const WorldPoint &originPt,/*!<[in]CurrentLocation of the car obtained from GPS fix */
		float heading,/*!< [in]Direction of travel if the car is in motion */
		RoutingOptions &routeOpts,/*!<[in] User /Application set preferences */
        ManeuverList& blockedManeuvers,/*!<[in] List of Maneuvers to be avoided*/
        const Route &prevRoute, /*!< [in]Route Object with a list of RoadsegmentIDs which form the route */
		Route &Route/*![out] New Route generated excluding a portion of prevRoute*/);

    RouteGuideList GetRouteGuideList () const;

    void SetRouteGuideList ( RouteGuideList & routeGuideList);
    
	/*! @brief Setup call back functions for route calculation debugging
	    @return NCDB_OK         - On success
    */
    void SetRoutingHooks (const RoutingHooks * hooks);

    int SetTimeLimit (int limit);

protected:
    /*! @brief RouteManager internal implementation. */
    RouteEngine *m_RouteEngine;
};

};

#endif
/*! @} */
