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
#ifndef NCDB_MANEUVER_H
#define NCDB_MANEUVER_H


#include "NcdbTypes.h"
#include "UtfString.h"
#include "AutoArray.h"
#include "AutoSharedPtr.h"
#include "Segment.h"
#include "CountryInfo.h"
#include "NLGuidance.h"
#include <vector>

class LaneGuidanceGen;
namespace Ncdb {


enum TurnType {
    UNKNOWN_MANEUVER,
    CONTINUE,
    U_TURN,
    OFFROUTE_U_TURN,
    TURN_RIGHT,
	TURN_HARD_RIGHT,
	TURN_SLIGHT_RIGHT,
    TURN_LEFT,
	TURN_HARD_LEFT,
	TURN_SLIGHT_LEFT,
    ENTER_HIGHWAY,
    ENTER_HIGHWAY_RIGHT,
    ENTER_HIGHWAY_LEFT,
    MERGE_INTO_TRAFFIC_RIGHT,
    MERGE_INTO_TRAFFIC_LEFT,
    EXIT_HIGHWAY_RIGHT,
    EXIT_HIGHWAY_LEFT,
    KEEP_TO_SPLIT_RIGHT,
    KEEP_TO_SPLIT_LEFT,
    KEEP_TO_SPLIT,
    KEEP_TO_SPLIT_HWY_RIGHT,
    KEEP_TO_SPLIT_HWY_LEFT,
    KEEP_TO_HWY_SPLIT,
	KEEP_TO_SPLIT_SUR_RIGHT,
    KEEP_TO_SPLIT_SUR_LEFT,
    KEEP_TO_SUR_SPLIT,
    STAY_TO_SPLIT_HWY_RIGHT,
    STAY_TO_SPLIT_HWY_LEFT,
    STAY_TO_HWY_SPLIT,
	STAY_TO_SPLIT_SUR_RIGHT,
    STAY_TO_SPLIT_SUR_LEFT,
    STAY_TO_SUR_SPLIT,
    NAME_CHANGE,
    CONTINUE_ON,
    ENTER_ROUNDABOUT_RIGHT,
    ENTER_ROUNDABOUT_LEFT,
    EXIT_ROUNDABOUT,
    ARRIVING_AT_DESTINATION_RIGHT,
    ARRIVING_AT_DESTINATION_LEFT,
    ARRIVING_AT_DESTINATION,
    ENTER_TUNNEL,
    ENTER_TUNNEL_RIGHT,
    ENTER_TUNNEL_LEFT,
    EXIT_TUNNEL,
    EXIT_TUNNEL_RIGHT,
    EXIT_TUNNEL_LEFT,
    ENTER_BRIDGE,
    ENTER_BRIDGE_RIGHT,
    ENTER_BRIDGE_LEFT,
    CATCH_A_FERRY, 
    CATCH_A_FERRY_RIGHT,
    CATCH_A_FERRY_LEFT,
    EXIT_FERRY,
    EXIT_FERRY_RIGHT,
    EXIT_FERRY_LEFT,
    ENTER_COUNTRY,
    DRIVE_STRAIGHT_THROUGH_ROTARY,
    CONTINUE_ROUTE_AS_PEDESTRIAN,
    CONTINUE_ROUTE_AS_PEDESTRIAN_RIGHT,
    CONTINUE_ROUTE_AS_PEDESTRIAN_LEFT,
    TAKE_RAMP_TOWARDS_ROADWAY,
    TAKE_RAMP_TOWARDS_ROADWAY_RIGHT,
    TAKE_RAMP_TOWARDS_ROADWAY_LEFT,
    ENTER_PRIVATE_ROADWAY,
    ENTER_PRIVATE_ROADWAY_RIGHT,
    ENTER_PRIVATE_ROADWAY_LEFT,
    EXIT_PRIVATE_ROADWAY,
    EXIT_PRIVATE_ROADWAY_RIGHT,
    EXIT_PRIVATE_ROADWAY_LEFT,
    ENTER_STAIRS,
    ENTER_STAIRS_RIGHT,
    ENTER_STAIRS_LEFT,
    ENTER_ESCALATOR,
    ENTER_ESCALATOR_RIGHT,
    ENTER_ESCALATOR_LEFT,
    ARRIVING_AT_INTERMEDIATE_POINT_LEFT,
    ARRIVING_AT_INTERMEDIATE_POINT_RIGHT,
	ARRIVING_AT_INTERMEDIATE_POINT,
    MAX_NO_TURN_TYPES
};
inline
const char* GetManAbbrevByType( TurnType turnType )
{
    const char* result = 0;

    switch( turnType )
    {
    case Ncdb::EXIT_HIGHWAY_RIGHT:              result = "EX.R";    break;
    case Ncdb::EXIT_HIGHWAY_LEFT:               result = "EX.L";    break;
    case Ncdb::OFFROUTE_U_TURN:                                     break;
    case Ncdb::U_TURN:                          result = "UT.";     break;
    case Ncdb::TURN_RIGHT:                      result = "TR.R";    break;
	case Ncdb::TURN_HARD_RIGHT:                 result = "TR.HR";   break;
	case Ncdb::TURN_SLIGHT_RIGHT:               result = "TR.SR";   break;
    case Ncdb::TURN_LEFT:                       result = "TR.L";    break;
    case Ncdb::TURN_HARD_LEFT:                  result = "TR.HL";   break;
	case Ncdb::TURN_SLIGHT_LEFT:                result = "TR.SL";   break;
    case Ncdb::KEEP_TO_SPLIT_RIGHT:             result = "KP.R";    break;
    case Ncdb::KEEP_TO_SPLIT_LEFT:              result = "KP.L";    break;
    case Ncdb::KEEP_TO_SPLIT:                   result = "KP.";     break;
	case Ncdb::KEEP_TO_SPLIT_HWY_RIGHT:         result = "KH.R";    break;    
    case Ncdb::KEEP_TO_SPLIT_HWY_LEFT:          result = "KH.L";    break;
    case Ncdb::KEEP_TO_HWY_SPLIT:               result = "KH.";    break;
	case Ncdb::KEEP_TO_SPLIT_SUR_RIGHT:         result = "KS.R";    break;
    case Ncdb::KEEP_TO_SPLIT_SUR_LEFT:          result = "KS.L";    break;
    case Ncdb::KEEP_TO_SUR_SPLIT:               result = "KS.";    break;
    case Ncdb::STAY_TO_SPLIT_HWY_RIGHT:         result = "SH.R";    break;
    case Ncdb::STAY_TO_SPLIT_HWY_LEFT:          result = "SH.L";    break;
    case Ncdb::STAY_TO_HWY_SPLIT:               result = "SH.";    break;
	case Ncdb::STAY_TO_SPLIT_SUR_RIGHT:         result = "SS.R";    break;
    case Ncdb::STAY_TO_SPLIT_SUR_LEFT:          result = "SS.L";    break;
    case Ncdb::STAY_TO_SUR_SPLIT:               result = "SS.";    break;
    case Ncdb::ARRIVING_AT_DESTINATION:         result = "DT.";     break;
    case Ncdb::ARRIVING_AT_DESTINATION_RIGHT:   result = "DT.R";    break;
    case Ncdb::ARRIVING_AT_DESTINATION_LEFT:    result = "DT.L";    break;
    case Ncdb::ENTER_HIGHWAY:                   result = "EN.";     break;
    case Ncdb::ENTER_HIGHWAY_RIGHT:             result = "EN.R";    break;
    case Ncdb::ENTER_HIGHWAY_LEFT:              result = "EN.L";    break;
    case Ncdb::MERGE_INTO_TRAFFIC_RIGHT:        result = "MR.R";    break;
    case Ncdb::MERGE_INTO_TRAFFIC_LEFT:         result = "MR.L";    break;
    case Ncdb::UNKNOWN_MANEUVER:                result = "UNKNOWN"; break;
    case Ncdb::CONTINUE:                        result = "CONTINUE";break;
    case Ncdb::NAME_CHANGE:                     result = "NC.";     break;
    case Ncdb::CONTINUE_ON:                     result = "CO.";     break;
    case Ncdb::ENTER_ROUNDABOUT_RIGHT:          result = "RE.R";    break;
    case Ncdb::ENTER_ROUNDABOUT_LEFT:           result = "RE.L";    break;    
    case Ncdb::EXIT_ROUNDABOUT:                 result = "RX.";     break;
    case Ncdb::ENTER_TUNNEL:                    result = "TE.";     break;
    case Ncdb::EXIT_TUNNEL:                     result = "TX.";     break;
    case Ncdb::ENTER_BRIDGE:                    result = "BE.";     break;
    case Ncdb::TAKE_RAMP_TOWARDS_ROADWAY:       result = "ER.";     break;
    case Ncdb::TAKE_RAMP_TOWARDS_ROADWAY_LEFT:  result = "ER.L";    break;
    case Ncdb::TAKE_RAMP_TOWARDS_ROADWAY_RIGHT: result = "ER.R";    break;
    case Ncdb::CATCH_A_FERRY:                   result = "FE.";     break;
    case Ncdb::EXIT_FERRY:                      result = "FX.";     break;
    case Ncdb::DRIVE_STRAIGHT_THROUGH_ROTARY:   result = "RT.";     break;
    case Ncdb::CONTINUE_ROUTE_AS_PEDESTRIAN:    result = "PE.";     break;
    case Ncdb::ENTER_PRIVATE_ROADWAY:           result = "NR.";     break;
    case Ncdb::ENTER_PRIVATE_ROADWAY_LEFT:      result = "NR.L";    break;
    case Ncdb::ENTER_PRIVATE_ROADWAY_RIGHT:     result = "NR.R";    break;
    case Ncdb::ENTER_COUNTRY:                   result = "EC.";     break;    
    case Ncdb::ENTER_STAIRS:                    result = "ES.";     break;
    case Ncdb::ENTER_STAIRS_RIGHT:              result = "ES.R";    break;
    case Ncdb::ENTER_STAIRS_LEFT:               result = "ES.L";    break;
    case Ncdb::ENTER_ESCALATOR:                 result = "EE.";     break;
    case Ncdb::ENTER_ESCALATOR_RIGHT:           result = "EE.R";    break;
    case Ncdb::ENTER_ESCALATOR_LEFT:            result = "EE.L";    break;  
    case Ncdb::ARRIVING_AT_INTERMEDIATE_POINT_LEFT:                 break;
    case Ncdb::ARRIVING_AT_INTERMEDIATE_POINT_RIGHT:                break;
    case Ncdb::ARRIVING_AT_INTERMEDIATE_POINT:                      break;
    default:
        result = "Invalid turn type";
        break;
    }

    return result;
}

inline
TurnType GetManTypeByAbbrevation( const char* abbrev )
{
     if( strcmp( abbrev, "EX.R" ) == 0 )
         return EXIT_HIGHWAY_RIGHT;
     else if( strcmp( abbrev, "EX.L" ) == 0 )
         return EXIT_HIGHWAY_LEFT;
     else if( strcmp( abbrev, "UT." ) == 0 )
         return U_TURN;
     else if( strcmp( abbrev, "TR.R" ) == 0 )
         return TURN_RIGHT;
	 else if( strcmp( abbrev, "TR.HR") == 0 )
		  return TURN_HARD_RIGHT;
	 else if( strcmp( abbrev, "TR.SR") == 0 )
		  return TURN_SLIGHT_RIGHT;
     else if( strcmp( abbrev, "TR.L" ) == 0 )
         return TURN_LEFT;
	 else if( strcmp( abbrev, "TR.HL") == 0 )
		  return TURN_HARD_LEFT;
	 else if( strcmp( abbrev, "TR.SL") == 0 )
		  return TURN_SLIGHT_LEFT;
     else if( strcmp( abbrev, "KP.R" ) == 0 )
         return KEEP_TO_SPLIT_RIGHT;
     else if( strcmp( abbrev, "KP.L" ) == 0 )
         return KEEP_TO_SPLIT_LEFT;
     else if( strcmp( abbrev, "KP." ) == 0 )
         return KEEP_TO_SPLIT;
	 else if( strcmp( abbrev,"KH.R" ) == 0 )
         return KEEP_TO_SPLIT_HWY_RIGHT;
	 else if( strcmp( abbrev,"KH.L" ) == 0 )
         return KEEP_TO_SPLIT_HWY_LEFT;
     else if( strcmp( abbrev,"KH." ) == 0 )
         return KEEP_TO_HWY_SPLIT;
	 else if( strcmp( abbrev,"KS.R" ) == 0 )
         return KEEP_TO_SPLIT_SUR_RIGHT;
     else if( strcmp( abbrev,"KS.L" ) == 0 )
         return KEEP_TO_SPLIT_SUR_LEFT;
     else if( strcmp( abbrev,"KS." ) == 0 )
         return KEEP_TO_SUR_SPLIT;
     else if( strcmp( abbrev,"SH.R" ) == 0 )
         return STAY_TO_SPLIT_HWY_RIGHT;
     else if( strcmp( abbrev,"SH.L" ) == 0 )
         return STAY_TO_SPLIT_HWY_LEFT;
     else if( strcmp( abbrev,"SH." ) == 0 )
         return STAY_TO_HWY_SPLIT;
	 else if( strcmp( abbrev, "SS.R" ) == 0 )
         return STAY_TO_SPLIT_SUR_RIGHT;
     else if( strcmp( abbrev, "SS.L" ) == 0 )
         return STAY_TO_SPLIT_SUR_LEFT;
     else if( strcmp( abbrev, "SS." ) == 0 )
         return STAY_TO_SUR_SPLIT;
     else if( strcmp( abbrev, "DT." ) == 0 )
         return ARRIVING_AT_DESTINATION;
     else if( strcmp( abbrev, "DT.R" ) == 0 )
         return ARRIVING_AT_DESTINATION_RIGHT;
     else if( strcmp( abbrev, "DT.L" ) == 0 )
         return ARRIVING_AT_DESTINATION_LEFT;
     else if( strcmp( abbrev, "EN." ) == 0 )
         return ENTER_HIGHWAY;
     else if( strcmp( abbrev, "EN.R" ) == 0 )
         return ENTER_HIGHWAY_RIGHT;
     else if( strcmp( abbrev, "EN.L" ) == 0 )
         return ENTER_HIGHWAY_LEFT;
     else if( strcmp( abbrev, "MR.R" ) == 0 )
         return MERGE_INTO_TRAFFIC_RIGHT;
     else if( strcmp( abbrev, "MR.L" ) == 0 )
         return MERGE_INTO_TRAFFIC_LEFT;
     else if( strcmp( abbrev, "CONTINUE" ) == 0 )
         return CONTINUE;
     else if( strcmp( abbrev, "NC." ) == 0 )
         return NAME_CHANGE;
     else if( strcmp( abbrev, "CO." ) == 0 )
         return CONTINUE_ON;
     else if( strcmp( abbrev, "RE.R" ) == 0 )
         return ENTER_ROUNDABOUT_RIGHT;
     else if( strcmp( abbrev, "RE.L" ) == 0 )
         return ENTER_ROUNDABOUT_LEFT;
     else if( strcmp( abbrev, "RX." ) == 0 )
         return EXIT_ROUNDABOUT;
     else if( strcmp( abbrev, "TE." ) == 0 )
         return ENTER_TUNNEL;
     else if( strcmp( abbrev, "TX." ) == 0 )
         return EXIT_TUNNEL;
     else if( strcmp( abbrev, "BE." ) == 0 )
         return ENTER_BRIDGE;
     else if( strcmp( abbrev, "ER." ) == 0 )
         return TAKE_RAMP_TOWARDS_ROADWAY;
     else if( strcmp( abbrev, "ER.L" ) == 0 )
         return TAKE_RAMP_TOWARDS_ROADWAY_LEFT;
     else if( strcmp( abbrev, "ER.R" ) == 0 )
         return TAKE_RAMP_TOWARDS_ROADWAY_RIGHT;
     else if( strcmp( abbrev, "FE." ) == 0 )
         return CATCH_A_FERRY;
     else if( strcmp( abbrev, "FX." ) == 0 )
         return EXIT_FERRY;
     else if( strcmp( abbrev, "RT." ) == 0 )
         return DRIVE_STRAIGHT_THROUGH_ROTARY;
     else if( strcmp( abbrev, "PE." ) == 0 )
         return CONTINUE_ROUTE_AS_PEDESTRIAN;
     else if( strcmp( abbrev, "NR." ) == 0 )
         return ENTER_PRIVATE_ROADWAY;
     else if( strcmp( abbrev, "NR.L" ) == 0 )
         return ENTER_PRIVATE_ROADWAY_LEFT;
     else if( strcmp( abbrev, "NR.R" ) == 0 )
         return ENTER_PRIVATE_ROADWAY_RIGHT;
     else if( strcmp( abbrev, "EC." ) == 0 )
         return ENTER_COUNTRY;
     else if( strcmp( abbrev, "ES." ) == 0 )
         return ENTER_STAIRS;
     else if( strcmp( abbrev, "ES.R" ) == 0 )
         return ENTER_STAIRS_RIGHT;
     else if( strcmp( abbrev, "ES.L" ) == 0 )
         return ENTER_STAIRS_LEFT;
     else if( strcmp( abbrev, "EE." ) == 0 )
         return ENTER_ESCALATOR;
     else if( strcmp( abbrev, "EE.R" ) == 0 )
         return ENTER_ESCALATOR_RIGHT;
     else if( strcmp( abbrev, "EE.L" ) == 0 )
         return ENTER_ESCALATOR_LEFT;

     return UNKNOWN_MANEUVER;
}


enum PhonemeType {
    PT_UNKNOWN        = -1,
    PT_PLAIN_TEXT     = 0,
    PT_TRANSLIT_TEXT  = 1,
    PT_SAMPA          = 2,
};

class  NCDB_API RoadInfo
{
public:

    RoadInfo() : 
      m_RouteNo( -1 ),
      m_UnNamed( true ),
      m_UnPaved( true ),
      m_bIsRefNode (false),
      m_bIsToll( false ),
      m_PronunType(PT_UNKNOWN),
      m_PlayTime( 0 )
      {}

    int                 getRouteNo()      const   { return m_RouteNo; }
    const UtfString&    getPrimaryName()  const   { return m_PrimaryName; }
    const UtfString&    getSecondaryName()const   { return m_SecondaryName; }
    UtfString           getPronunKey()    const   { return m_PronunKey; }
    NodeSegmentID       getNodeSegID()    const   { return m_NodeSegID; }
    const UtfString&    getRouteType()    const   { return m_RouteType; }
    const UtfString&    getRouteName()    const   { return m_RouteName; }
    const UtfString&    getRouteDir()     const   { return m_RouteDir; }
    void				SetIsRefNode(const bool b)	{m_bIsRefNode = b;}
    bool				GetIsRefNode() const		{return m_bIsRefNode;}
    void                SetIsCurTollRoad(const bool b){m_bIsToll = b;}
    bool                GetIsCurTollRoad()const     {return m_bIsToll;}
    UtfString           getPronunDial()   const   { return m_PronunDial; }
    int                 getPronunType()   const   { return m_PronunType; }
    int                 getPlayTime() const {return m_PlayTime;}
    UtfString           getPronunValue()const {return m_PronunKey;}
    UtfString           getTransLitTxt()const {return m_TransLitTxt;}

private:

    int             m_RouteNo;
    UtfString       m_PrimaryName;
    UtfString       m_SecondaryName;
    bool            m_UnNamed;
    bool            m_UnPaved;
    bool			m_bIsRefNode;
    bool            m_bIsToll;
    UtfString       m_PronunKey;
    NodeSegmentID   m_NodeSegID;
    UtfString       m_RouteType;
    UtfString       m_RouteName;
    UtfString       m_RouteDir;
    UtfString       m_PronunDial;
    UtfString       m_TransLitTxt;
    int             m_PronunType;
    int             m_PlayTime;
    friend class RouteEngine;
};

class NCDB_API Crossroad
{
    public:
        Crossroad();

        Crossroad( float distanceInMetersFromLastManeuver
                 , const UtfString& leftMostCrossStreetName
                 , const UtfString& rightMostCrossStreetName ); 
        float getDistanceInMetersFromLastManeuver() const;
        const UtfString& getLeftMostCrossStreetName() const;
        const UtfString& getRightMostCrossStreetName() const;
		//RoadSegmentIDList getCrossStreetIDList()const;
    private:
        float m_DistanceInMetersFromLastManeuver;
        UtfString   m_LeftMostCrossStreetName;
        UtfString   m_RightMostCrossStreetName;
};

typedef AutoArray< Crossroad > CrossroadList;
#ifdef _MSC_VER
template class NCDB_API AutoArray< Crossroad >;
#endif

class NCDB_API FirstManeuverCrossroad
{
public:
    FirstManeuverCrossroad();
    FirstManeuverCrossroad(
        double distanceInMeters,
        const UtfString& crossStreetName,
        const UtfString& pronunKey,
        const char * pronunDial = NULL,
        int pronunType = PT_UNKNOWN,
        int playTime = 0); 

    double getDistanceInMeters() const;
    const UtfString& getCrossStreetName() const;
    const UtfString& getPronunKey() const;
    const UtfString& getPronunDial() const;
    int getPronunType() const;
    int getPlayTime() const;

private:
    double m_DistanceInMeters;
    UtfString m_CrossStreetName;
    UtfString m_PronunKey;
    UtfString m_PronunDial;
    int       m_PronunType;
    int       m_PlayTime;
};

typedef AutoArray< FirstManeuverCrossroad > FirstManeuverCrossroadList;
#ifdef _MSC_VER
template class NCDB_API AutoArray< FirstManeuverCrossroad >;
#endif

// Traffic Delay type
enum TrafficDelayType
{
    RouteFeature_None = 0,
    RouteFeature_StopSign = 1,
    RouteFeature_TrafficLight = 2,
};

// Traffic delay for StopSign & TrafficLight
class NCDB_API TrafficDelay
{
public:
    TrafficDelay():m_Type(RouteFeature_None), m_Position(0), m_DelayTime(0){}
    TrafficDelay(TrafficDelayType type, double position, unsigned int delayTime)
        :m_Type(type), m_Position(position), m_DelayTime(delayTime){}

    TrafficDelayType m_Type;        // delay type
    double           m_Position;    // delay position relative to the start of this maneuver in meters.
    unsigned int     m_DelayTime;   // delay time in seconds
};

typedef AutoArray<TrafficDelay> TrafficDelayList;
#ifdef _MSC_VER
template class NCDB_API AutoArray<TrafficDelay>;
#endif

// Route features for each maneuver
class NCDB_API RouteFeatures
{
public:
    RouteFeatures()
    : m_IsTolls(false), 
      m_IsHOV(false), 
      m_IsHighway(false), 
      m_IsUnpaved(false),
      m_IsFerry(false),
      m_IsPrivate(false),
      m_IsCountry(false),
      m_IsGate(false)
    {};
    ~RouteFeatures(){};

    bool m_IsTolls;         // Flag indicates that the route makes use of tolls.
    bool m_IsHOV;           // Flag indicates that the route makes use of HOV lanes.
    bool m_IsHighway;       // Flag indicates that the route makes use of highways.
    bool m_IsUnpaved;       // Flag indicates that the route makes use of unpaved roads.
    bool m_IsFerry;         // Flag indicates that the route makes use of ferry.    
    bool m_IsPrivate;       // Flag indicates that the route makes use of private road.
    bool m_IsCountry;       // Flag indicates that the route makes use of new country.
    bool m_IsGate;          // Flag indicates that the route makes use of gate.
};

struct NCDB_API SARInfo
{
	UtfString SARID;
	UtfString PortraitSARID;
	UtfString SARType;
	UtfString DataSetID;
	UtfString Version;
};

/*! A Single Maneuver or Instruction 
*/
class NCDB_API Maneuver
{
public:
    Maneuver()
        : m_TurnType(UNKNOWN_MANEUVER)
        , m_StackAdvise(false)
        , m_isComplete(true)
        , m_Length(0)
        , m_fMaxDistance(0)
        , m_fEstimatedTime(0.f)
        , m_fHeading(0)
        , m_iExitNumber( -1 )
        , m_NextPaving( UnknownPaving )
    {		
        m_Point.Clear();
    }

    enum RoadPaving
    {
        UnknownPaving,
        Paved,
        Unpaved
    };

	const SARInfo& GetSARInfo() const
	{
		return m_SARInfo;
	}

    RoadPaving GetRoadPaving( ) const { return m_NextPaving; }

    const WorldPoint&       getPoint() const          {return m_Point;}
    const RoadInfo&         getCurRoadInfo() const    {return m_CurRoadInfo;}
    const RoadInfo&         getTurnRoadInfo() const   {return m_TurnRoadInfo;}
    TurnType                getTurnType() const       {return m_TurnType;}
    float                   getLength() const         {return m_Length;}
    bool                    isStacked() const         {return m_StackAdvise;}
    void                    setStacked(bool stack)    {m_StackAdvise = stack;}
    bool                    isComplete() const        {return m_isComplete;}
    float                   getMaxDistance() const    {return m_fMaxDistance;}
    float                   getHeading() const        {return m_fHeading;}
    float                   getEstimatedTime() const  {return m_fEstimatedTime;}
    const WorldPointList&   getPolyline() const       {return m_ManPoly;}
    const CrossroadList&    getCrossroads() const;
    const FirstManeuverCrossroadList& getFirstManeuverCrossroadList() const {return m_FirstManeuverCrossroads;}
    void                    SetTurnType( TurnType turn ){ m_TurnType = turn; }
    int                     GetExitNumber() const       { return m_iExitNumber; }

    const CountryInfo&      GetCurCountry() const       { return m_CurCountry; }
    const CountryInfo&      GetTurnCountry() const      { return m_TurnCountry; }
    
    void					SetIsCurRefNode(const bool b)	{m_CurRoadInfo.SetIsRefNode(b);}
    void					SetIsTurnRefNode(const bool b)	{m_TurnRoadInfo.SetIsRefNode(b);}
    void                    SetIsCurTollRoad(const bool b) {m_TurnRoadInfo.SetIsCurTollRoad(b);}
	//!Main API to return the lane guidance information 
    const ExitNumber&             GetMvrExitNumber() const {return m_MvrExitNumber;}
	const GuidancePointList&      GetGuidancePointList()const {return m_GuidancePointList;}
    const LaneGuidanceInfo&       GetMvrLaneGuidance()const {return m_MvrLaneGuidanceInfo;}
	void	SetMvrLaneGuidance(LaneGuidanceInfo& laneGuidanceInfo) {m_MvrLaneGuidanceInfo = laneGuidanceInfo;};
    void    SetGuidancePointList(GuidancePointList &list) {m_GuidancePointList = list;}
    void    SetMvrExitNumber(ExitNumber &ExitNum) {m_MvrExitNumber = ExitNum;}

    const GuidancePointList&      TestGuidancePointList()const {return m_TestGuidancePointList;}
    void  SetTestGuidancePointList(GuidancePointList &list) {m_TestGuidancePointList = list;}

    // Get/Set basic route features & traffic delay list
    const RouteFeatures& GetRouteFeatures() const { return m_RouteFeatures; } 
    void  SetRouteFeatures(const RouteFeatures& features) { m_RouteFeatures = features; }
    const TrafficDelayList& GetTrafficDelayList() const { return m_trafficDelayList; }
    void  SetTrafficDelayList(const TrafficDelayList& delayList) { m_trafficDelayList = delayList; }

private:
    
	SARInfo m_SARInfo;

    ExitNumber              m_MvrExitNumber;
	LaneGuidanceInfo		m_MvrLaneGuidanceInfo;
	GuidancePointList       m_GuidancePointList;
    GuidancePointList       m_TestGuidancePointList;

    FirstManeuverCrossroadList m_FirstManeuverCrossroads;
    CrossroadList   m_Crossroads;
	//! Point for the turn
	WorldPoint      m_Point;
	//!Information about the road on which the turn begins
	RoadInfo        m_CurRoadInfo;
	//!Information about the road on which the turn ens
	RoadInfo        m_TurnRoadInfo;
	//!Description of the turn
	TurnType        m_TurnType;
	//!Present if stacking tothe next maneuver is advised
	bool            m_StackAdvise; //!what is expected ?TODO
    //!Present if this maneuver has full display segment.
    bool            m_isComplete;
    //! Length of the Maneuver in meters
	float           m_Length;
    //! Maximum distance at which to instruct  the user 
	float           m_fMaxDistance;
	//!	m_turnmap; Not sure of the output expectedTODO
	//	!m_crossstreet;Not sureTODO
	// ! WorldPointList for the maneuver TODO
	WorldPointList  m_ManPoly;

    float           m_fEstimatedTime; // hours

    float           m_fHeading;

    //! Contains exit # for EXIT_ROUNDABOUT maneuver and -1 for any other maneuver
    int             m_iExitNumber;

    CountryInfo     m_CurCountry;
    CountryInfo     m_TurnCountry;

    RoadPaving      m_NextPaving;

    RouteFeatures    m_RouteFeatures;     // Basic route features
    TrafficDelayList m_trafficDelayList;  // Traffic delay list 

    friend class RouteEngine;
    friend class LaneGuidanceGen;
};

typedef AutoArray<Maneuver> ManeuverList;
#ifdef _MSC_VER
template class NCDB_API AutoSharedPtr<ManeuverList>;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

inline
Crossroad::Crossroad() : m_DistanceInMetersFromLastManeuver( 0 )
{
}

inline
Crossroad::Crossroad(float distanceInMetersFromLastManeuver,
                           const UtfString& leftMostCrossStreetName,
                           const UtfString& rightMostCrossStreetName
						   ) :
    m_DistanceInMetersFromLastManeuver( distanceInMetersFromLastManeuver ),
    m_LeftMostCrossStreetName( leftMostCrossStreetName ),
    m_RightMostCrossStreetName( rightMostCrossStreetName )
{
}

inline
float Crossroad::getDistanceInMetersFromLastManeuver() const
{
    return m_DistanceInMetersFromLastManeuver;
}

inline
const UtfString& Crossroad::getLeftMostCrossStreetName() const
{
    return m_LeftMostCrossStreetName;
}

inline
const UtfString& Crossroad::getRightMostCrossStreetName() const
{
    return m_RightMostCrossStreetName;
}

inline
FirstManeuverCrossroad::FirstManeuverCrossroad() :
    m_DistanceInMeters(0),
    m_PronunType (PT_UNKNOWN),
    m_PlayTime( 0 )
{
}

inline
FirstManeuverCrossroad::FirstManeuverCrossroad(
    double distanceInMeters,
    const UtfString& crossStreetName,
    const UtfString& pronunKey,
    const char * pronunDial,
    int pronunType, int playtime)
{
    m_DistanceInMeters = distanceInMeters;
    m_CrossStreetName = crossStreetName;
    m_PronunKey = pronunKey;
    if (pronunDial)
    {
        m_PronunDial = pronunDial;
    }
    m_PronunType = pronunType;
    m_PlayTime = playtime;
}

inline
double FirstManeuverCrossroad::getDistanceInMeters() const
{
    return m_DistanceInMeters;
}

inline
const UtfString& FirstManeuverCrossroad::getCrossStreetName() const
{
    return m_CrossStreetName;
}

inline
const UtfString& FirstManeuverCrossroad::getPronunKey()const
{
    return m_PronunKey;
}

inline
const UtfString& FirstManeuverCrossroad::getPronunDial()const
{
    return m_PronunDial;
}

inline
int FirstManeuverCrossroad::getPronunType()const
{
    return m_PronunType;
}
inline
int FirstManeuverCrossroad::getPlayTime()const
{
    return m_PlayTime;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

inline
const CrossroadList& Maneuver::getCrossroads() const
{
    return m_Crossroads;
}
};
#endif // NDBR_MANEUVER_H
