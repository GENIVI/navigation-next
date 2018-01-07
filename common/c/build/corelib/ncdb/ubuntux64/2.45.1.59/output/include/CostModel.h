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

/*
 *
 * Copyright (c) 2007 Networks In Motion, Inc.
 * All rights reserved.  This file and associated materials are the
 * trade secrets, confidential information, and copyrighted works of
 * Networks In Motion, Inc.
 *
 * This intellectual property is for the internal use only by Networks
 * In Motion, Inc.  This source code contains proprietary information
 * of Networks In Motion, Inc., and shall not be used, copied by, or
 * disclosed to, anyone other than a Networks In Motion, Inc.,
 * certified employee that has written authorization to view or modify
 * said source code.
 */

#ifndef NCDB_COSTMODEL_H
#define NCDB_COSTMODEL_H
#include "NcdbTypes.h"

namespace Ncdb {
class RoutingOptions;
class RoutingProperties;
struct AngleCostPoint;
struct RoadFlagCost;

class  CostModel 
{
	public:
		//!Default constructor
		CostModel();

		/*! Constructor for CostModel.This is the only way public interface to create it.
		 @param[in] routingOptions
		 @param[in] routingProps
		 */
		void SetCostModel(const RoutingOptions* routingOptions,const RoutingProperties* routingProps);
		//!Accessors
        /*! Get the AStarMul.
		 @return -m_AStarMul
		 */
		float GetAStarMul()const;
         /*! Get the m_ASTARsubtractDist.
		 @return -m_ASTARsubtractDist
		 */
		float GetAStarSubtractDist()const;
		 /*! Get the FunctionalClass Cost.
		 @return -m_FunClassCost
		 */
		float* GetFunClassCost()const;
		 /*! Get the SpeedCatCost.
		 @return -m_SpeedCatCost
		 */
		float* GetSpeedCatCost()const;
		 /*! Get the Trun Cost Size.
         @param[in] fastest: flag indicates to use fastest or default turn cost
		 @return -m_TurnCostSize/m_FastestTurnCost
		 */
		int GetNumAnglePoints(bool fastest = false)const;
		 /*! Get the TurnCost.
         @param[in] fastest: flag indicates to use fastest or default turn cost
		 @return -m_TurnCost/m_FastestTurnCost
		 */
		const AngleCostPoint* GetTurnCost(bool fastest = false)const;     
        /*! Get the Turn Cost for shortest dist opt
        @return - m_ShortestDistTurnCost;
        */
		float GetShortestDistTurnCost()const;
        /*! Get the Turn Cost for Walk mode(pedestrian and bicycle currently
         @return - m_WalkModeTurnCost
	    */
		float GetWalkModeTurnCost()const;
        /*! Get the cost for crossing a border
             @return - m_Border
	    */
		float GetBorderCrossingCost()const;
		/*! Get the number of TransitionCosts
		  @return -m_BorderCost;
		*/
		int GetNumTransitionCosts()const;
		/*! Get the Transition Costs
		 @return -m_TransitionCosts
		 */
		const RoadFlagCost* GetTransitionCosts()const;
        /*! Get the number of Add Costs
		  @return -m_NumAdditiveCosts
		*/
		int GetNumAdditiveCosts()const;
		/*! Get the Additive Costs
		 @return -m_AdditiveFlagCosts
		 */
		const RoadFlagCost* GetAdditiveCosts()const;
		 /*! Get the number of Multiplicative Costs
		  @return -m_NumAdditiveCosts
		*/
		int GetNumMultiplicativeCosts()const;
        /*! Get the Multiplicative Costs
		 @return -m_AdditiveFlagCosts
		 */
        const RoadFlagCost* GetMultiplicativeCosts()const;
        /*! Get the pedestrian cost for non pedestrian roads
		 @return -m_AdditiveFlagCosts
		 */
		float GetWalkModeCostOnNOPedRoads()const;
        /*! Get the pedestrian cost factor for reversal
		 @return -m_WalkModeCostFactorOnReversal
		 */
		float GetWalkModeCostFactorOnReversal()const;
         /*! Get the cost for passing through a Gate
		 @return m_GateCost
		 */
		float GetGateCost()const;
        /*! Destructor
		*/
		~CostModel();
            
        float GetPreferFWYDist() const { return m_PreferFWYDist; }
        float GetEnlargeAstarDist() const { return m_EnlargeAstarDist; }
        float GetAstarMlpMlp() const { return m_AstarMlpMlp; }
        float GetFC4Multipler() const { return m_FC4Multipler; }
        float GetFC5Multipler() const { return m_FC5Multipler; }
        float GetMaxSpeedRamp() const { return  m_MaxSpeedRamp;}
        float GetUTurnLowCost() const { return m_UTurnLowCost; }
        float GetUTurnHighCost() const { return m_UTurnHighCost; }
        int GetAStarDynamicSpeed() const { return m_AStarDynamicSpeed; }
        float GetTrafficLightCost() const { return m_TrafficLightCost;}
        float GetStopSignCost() const { return m_StopSignCost;}
        float GetIntersectionCost() const { return m_IntersectionCost;}
        float* GetStopSignFCFactor() const { return m_StopSignFCFactor;}
        float* GetTrafficLightFCFactor() const { return m_TrafficLightFCFactor;}

        const AngleCostPoint* GetTrafficLightAngleFactor() const {return m_TrafficLightAngleFactor;}
        const AngleCostPoint* GetStopSignAngleFactor() const {return m_StopSignAngleFactor;}
        int GetAngleFactorPoints() const {return m_AngleFactorPoints;}
		
	private:
		
        //!Handle to the routing properties populated using the routing.ini file
		RoutingProperties* m_RoutingProps;
		 
		//!Facilitates a smoother route with fewer maneuvers by simulating a real life where turns at intersections are expensive
		//!This is mostly an additive cost.This is implemented using 
		//!An Array of (angle,cost) points that will be interpolated in the turn cost function
        int             m_TurnCostSize;     // Default turn costs size
        AngleCostPoint* m_TurnCost;         // Default turn costs

        int             m_FastestCostSize;  // Fastest turn costs size
        AngleCostPoint* m_FastestTurnCost;  // Fastest turn costs (backup turn costs for "Easiest" routing)
		
        const AngleCostPoint* m_TrafficLightAngleFactor;
        const AngleCostPoint* m_StopSignAngleFactor;

		float m_ShortestDistTurnCost;

		float m_WalkModeTurnCost;
        //!Multiplicative cost of Travel based on the functional class of the link.Initialised to 1.0.
		//!Could be used to avoid roads
		float *m_FunClassCost;
		//!Multiplicative cost of Travel based on the speed category of the link.Initialised to 1.0.
		//!Could be used to avoid roads
		float *m_SpeedCatCost;
		//!AStar pull which has a default value and is changed with the mode of travel and the route optimization chosen
		float m_AStarMul;

	    float m_ASTARsubtractDist;

        float m_PreferFWYDist;
        float m_EnlargeAstarDist;
        float m_AstarMlpMlp;
        float m_FC4Multipler;
        float m_FC5Multipler;
        float m_MaxSpeedRamp;
        int m_AStarDynamicSpeed;
		float m_BorderCost;
        float m_UTurnLowCost;
        float m_UTurnHighCost;
        //!Transition Costs.Penalties that are added when flags change from one link to the next.  
		//!Flags with the same cost can be grouped together in one FlagCost element 
		//!(e.g. if entering a private way and entering construction had the same cost then
		//!m_FlagCosts[0].OnFlag = LKF_DC_PRIVATE | LKF_CONSTRUCTION ?in this case the OffFlag would be NULL).  
		//!FlagCosts contain OnFlag and OffFlag so that you can add a penalty when a maplink flag changes from 0 to 1 or from 1 to 0.       
		int m_NumTransitionCosts;//!number of patterns of flags that have the same cost
        RoadFlagCost *m_TransitionCosts;// Additive Flag Penalties (should include ramp penalty)
        //!FlagCosts :Penalties that are added based on the flag itself (e.g. detour penalty or pedestrian way for a car).  
		//!This works similar to the transition cost in that you can specify if the penalty is levied if the flag is ON or OFF 
		//!based on whether you make the flag on in the OnFlag or the OffFlag.
        int m_NumAddFlagCosts;
        RoadFlagCost *m_AdditiveFlagCosts;
        //!Multiplicative Flag Penalties 
		// penalties imposed prior to other penalties being added ?except maybe turn cost) ?
		//!these could be things like ramps or tolls when no tolls is requested).  Again you can 
		//!specify if the penalty is applied when the flag is high or low by whether the flag is high in m_OnFlag or m_OffFlag
        int m_NumMultFlagCosts;
        //!The cost of travel by pedestrian on roads where walking is not allowed
		float m_WalkModeCostOnNOPedRoads;
		//!The cost factor of travel by pedestrian on one way roads but reversal
		float m_WalkModeCostFactorOnReversal;
        //!The cost of crossing a gate 
		float m_GateCost;
        RoadFlagCost *m_MultFlagCosts;

        float m_TrafficLightCost;
        float m_StopSignCost;
        float m_IntersectionCost;

        int   m_AngleFactorPoints;
        float *m_TrafficLightFCFactor;
        float *m_StopSignFCFactor;

	    /*!Set the turnCost
		  @param[in] costs: data array
		  @param[in] size: array size
          @param[in] fastest: flag to set "fastest" or "default" turn costs
		  @return - ReturnCode -NCDB_OK 
		                      -NCDB_FAIL
	    */
		ReturnCode SetTurnCost(const AngleCostPoint* costs, int size, bool fastest = false);

		/*! Set the Turn Cost for shortest dist opt
		 @param[in] turnCost
         @return - ReturnCode -NCDB_OK 
		                      -NCDB_FAIL
	    */
		ReturnCode SetShortestDistTurnCost(float turnCost);
        /*! Set the Turn Cost for Walk mode(pedestrian and bicycle currently
		 @param[in] turnCost
         @return - ReturnCode -NCDB_OK 
		                      -NCDB_FAIL
	    */
		ReturnCode SetWalkModeTurnCost(float turnCost);
		/*!Set the FunClassCost
		  @param[in] funClassCost
		  @param[in] no_functionalClass
		  @return - ReturnCode -NCDB_OK 
		                      -NCDB_FAIL
	    */
		ReturnCode SetFunctionClassCost(const float *funClassCost);
        /*!Set the SpeedCategory Cost
		  @param[in] funClassCost
		  @param[in] no_functionalClass
		  @return - ReturnCode -NCDB_OK 
		                      -NCDB_FAIL
	    */
        ReturnCode SetSpeedCatCost(const float *speedCatCost);
        /*!Set the A* Pull factor
		  @param[in] AStarMul
		  @param[in] AStarSubractDist
		  @return - ReturnCode -NCDB_OK 
		                      -NCDB_FAIL
	    */
		ReturnCode SetAStarPullFactor(float AStarMul,const float AStarSubtractDist );
        /*!Set the Transition Cost
		  @param[in] numTransitionCosts
		  @param[in] transitionCost
		  @return - ReturnCode -NCDB_OK 
		                      -NCDB_FAIL
	    */
		ReturnCode SetTransitionCost(int numTransitionCosts, const RoadFlagCost *transitionCost);
        /*!Set the Additive Flag Cost
		  @param[in] numAdditiveFlagCosts
		  @param[in] addFlagCost
		  @return - ReturnCode -NCDB_OK 
		                      -NCDB_FAIL
	    */
		ReturnCode SetAdditiveFlagCosts(int numAdditiveFlagCosts, const RoadFlagCost *addFlagCost);
        /*!Set the Additive Flag Cost
		  @param[in] numMulFlagCosts
		  @param[in] mulFlagCost
		  @return - ReturnCode -NCDB_OK 
		                      -NCDB_FAIL
	    */
		ReturnCode SetMultiplicativeFlagCosts(int numMulFlagCosts, const RoadFlagCost *mulFlagCost);
        /*!Set the Cost of travel for pedestrians on roads not allowing them
		  @param[in] noPedCost
		  @return - ReturnCode -NCDB_OK 
		                      -NCDB_FAIL
	    */
		ReturnCode SetWalkModeCostOnNOPedRoads(float noPedCost);
		/*!Set the Cost of Crossing a Gate
		  @param[in] gateCost
		  @return - ReturnCode -NCDB_OK 
		                      -NCDB_FAIL
	    */
		ReturnCode SetGateCost(float gateCost);
        /*!Set the Cost of Crossing a country border
		  @param[in] bCost
		  @return - ReturnCode -NCDB_OK 
		                      -NCDB_FAIL
	    */
		ReturnCode SetBorderCrossingCost(float bCost);

        /*!Set the FCFactor for traffic light/stop sign
        @param[in] FCFactorDest
        @param[in] FCFactorSrc
        @return - ReturnCode -NCDB_OK 
        -NCDB_FAIL
        */
        ReturnCode SetFCFactor(float* &FCFactorDest, const float *FCFactorSrc);
};


};
#endif

