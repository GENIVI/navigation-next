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

    @file     RoutingProperties.h
    @date     03/23/2009
    @defgroup MOBIUS_COMMON  Mobius Common API 

    RoutingProperties API class accessing routing INI-file properties.
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
#ifndef NCDB_ROUTINGPROPERTIES_H
#define NCDB_ROUTINGPROPERTIES_H


#include "NcdbTypes.h"
#include "RouteGuide.h"

namespace Ncdb {
class   RoutingPropertyData;
struct AngleCostPoint;
struct RoadFlagCost;
struct MultiRouteSettings;
struct ReturnToRouteSettings;
// =====================================================================================
//!     Routing Properties container class
// =====================================================================================
class NCDB_API RoutingProperties
{
public:
    RoutingProperties(const char *  mapConfigPath);
    ~RoutingProperties();

    //! Check if RoutingProperties object is valid.
    //bool              IsValid() const; 

	//! Get AStar subtract dist
	float GetDefaultAStarSubtractDist() const;
    //! Get Astar Dist Weight 
    float GetAStarDistWeight()  const;

	//! Get Astar Time Weight
	float GetAStarTimeWeight() const;

	//!Get Astar Major Weight
    float GetAStarMajorWeight() const;

	//!Get Astar Local Weight
    float GetAStarLocalWeight() const;
	 
	//!Get Astar Walk Weight
    float GetAStarWalkWeight() const;

	//!Get Default cost based on the functional class of roads
	const float* GetDefaultFuncClassCost()const;

    //!Get cost for faster route based on the functional class of roads
	const float* GetShortestTimeFuncClassCost()const;

	//!Get cost for faster route for Trucks based on functional class
	const float* GetShortestTimeTruckFuncClassCost()const;

	//!Get Default cost based on  speed cat of roads 
	const float* GetDefaultSpeedCatCost()const;

	//!Get  cost based on  speed cat of roads for Shortest time 
	const float* GetShortestTimeSpeedCatCost()const;

	//!Get Turn Cost
	const AngleCostPoint* GetDefaultTurnCost()const;

	//!Get TurnCost for Fastest Truck Routes

    const AngleCostPoint* GetTruckFastestTurnCost()const;

	//!Get TurnCost for Easiest Truck Routes

    const AngleCostPoint* GetTruckEasiestTurnCost()const;

	//!Get Turn Cost for Easiest Route 
	const AngleCostPoint* GetEasiestRouteTurnCost()const;

    //!Get turn factor for Stop Sign cost
    const AngleCostPoint* GetStopSignAngleFactor()const;

    //!Get turn factor for Traffic light cost
    const AngleCostPoint* GetTrafficLightAngleFactor() const;

	//!Get the turn cost for the shortest dist
	const float GetTurnCostForShortestDist()const;

	//!Get the turn Cost for Pedestrian mode
    const float GetTurnCostForPedestrianMode()const;
   
	//!Get the turn Cost for Bicyle mode
    const float GetTurnCostForBicycleMode()const;
	
	//!Get Number of Angle Points
    int GetDefaultNumAnglePoints()const;

    //!Get Number of Angle Points
    int GetAngleFactorPoints() const;

	//!Get Number of Transition Costs
	int GetNumTransitionCosts()const;

	//!Get the default Transition Cost
    float GetDefaultTransitionCost()const;
    
	//!Get the  Transition Cost for shortest time
	float GetShortestTimeTransitionCost()const;
    
	//!Get the  Transition Cost when we move from a controlled access link
	//!to a non controlled access link
	float GetControlledAccessTransitionCost()const;

	//!Get Number of Additive cost based on theroad flags
	int GetNumAdditiveFlagCosts()const;

	//!Get the   Cost for travelling on a segment where cars are not allowed for shortest time
	float GetShortestTimeNonAutoCost()const;

    //!Get the   Default Cost for travelling on a segment where cars are not allowed 
	float GetDefaultNonAutoCost()const;
    
	//!Get the   Cost for travelling on a segment meant to be detoured around for shortest time
	float GetShortestTimeDetourCost()const;

    //!Get the   Default Cost for travelling on a segment meant to be detoured around 
	float GetDefaultDetourCost()const;

	//!Get the   Cost for travelling on a segment where trucks are to be avoided
	float GetShortestTimeAvoidTruckCost()const;

    //!Get the   Default Cost for travelling on a segment  where trucks are to be avoided
	float GetDefaultAvoidTruckCost()const;
	
	//!Get the number of multiplicative costs based on the road flaga
	int GetNumMultiplicativeFlagCosts()const;

	//!Get the default multiplicative cost for travelling on a ramp
    float GetDefaultRampCost()const;

    //!Get the Cost Divider used for favouring roads with certain flags or attributes
	float GetCostDividerforFavoring()const;
 
	//!Get the Cost Multiplier used of avoiding roads with certain flags or attributes
	float GetCostMultiplierforAvoidance()const;
	
	//! Get the pedestrian cost for non pedestrian roads
	float GetWalkModeCostOnNOPedRoads()const;

	//! Get the pedestrian cost factor for reversal
	float GetWalkModeCostFactorOnReversal()const;

	//! Get Gate Costs
	float GetGateCost()const;
	
	//!Get the ProjectionMultiplication Factor
	float GetProjectionMultFactor()const;

	//!Get the ProjectionAddFactor
	float GetProjectionAddFactor()const;

	//!Get Valid Speed
    float GetValidSpeedForProjection()const;

	//!Get Lunatic Speed
	float GetLunaticSpeed()const;

    //!Get MaxSpeedAllowedOnRamps(Km/hr)
    float GetMaxSpeedOnRamp()const;

	//!To use or Not to use Virtual connections
	int GetUseVirtualConnections()const;

    //! Get underConstructionOffset
    float GetUnderConstructionOffset() const;

	//! Get minDestinationOffset
	float GetMinDestinationOffset() const;

    //! Get multi dest lookup radius
    float GetMultiDestLookupRadius() const;

	//! Ignore Destination Number
	int GetIgnoreAddressNumber() const;

    //! Light weight goecoding
    int GetUseLightWeightGeocode() const;

    //! Dest Street Name for link selection
    int GetUseDestStreetName() const;

    float GetPreferFWYDist() const;
    float GetEnlargeAstarDist() const;
    float GetAstarMlpMlp() const;
    float GetFC4Multipler() const;
    float GetFC5Multipler() const;
    float GetUTurnHighCost() const;
    float GetUTurnLowCost() const;

    int GetUseStopSignTrafficLight() const;
    float GetETAStopSign() const;
    float GetETATrafficLight() const;
    float GetETAIntersection() const;
    float GetStopSignCost() const;
    float GetTrafficLightCost() const;
    float GetIntersectionCost() const;

    const float* GetStopSignFCFactor() const;
    const float* GetTrafficLightFCFactor() const;

	//!Get the penalty for crossing a country border.
    float GetBorderCrossingCost() const;

    //!Get mutli-route settings
	const MultiRouteSettings* GetMultiRouteSettings()const;

    //!Get return to route settings
	const ReturnToRouteSettings* GetReturnToRouteSettings()const;

	//!Get ETA speed adjustment factors based on functional class
	const float* GetETASpeedFactors(int cc = -1)const;
    //! Enable use of dynamic speed in AStar heuristic
    int GetAStarDynamicSpeed() const;
    
	//!Get route guides
	const RouteGuideList* GetRouteGuideList()const;

	//!Use route guides as primary (base) route
	int GetUseRouteGuideAsPrimary()const;

	int GetRetainSDBlocks()const;

    RoutingPropertyData * GetRoutingPropertyData() {return m_RoutingPropsData;}
private:  
    RoutingPropertyData*   m_RoutingPropsData;          // pointer to Routing Properties data container
};

};

#endif // NCDB_ROUTINGPROPERTIES_H
/*! @} */
