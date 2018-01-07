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
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#ifndef NLGUIDANCE_H
#define NLGUIDANCE_H

#include "IniFileProperties.h"
#include "NcdbTypes.h"
#include "UtfString.h"
#include "AutoArray.h"
#include "AutoSharedPtr.h"

class LaneGuidanceGen;

namespace Ncdb
{
enum NGType{
	NG_INFO_TRAFFICSIGN = 1,
    NG_INFO_TRAFFICLIGHT,
    NG_INFO_ENDOFROAD,
    NG_INFO_BRIDGEEND,
    NG_INFO_STREETCOUNT,
    NG_INFO_RAILWAYCROSS,
    NG_INFO_NGPOI,
    NG_INFO_NONE
};

enum RoadType
{
    RT_Normal = 1,
    RT_Highway = 2,
    RT_Destination = 3,
    RT_Total = 4
};

class NCDB_API ManeuverProperties
{
public:
    ManeuverProperties();
    ManeuverProperties(const char *path); 

    void Init(const char *path);

    int GetPrioTrafficlight()   { return m_inited ? m_PrioTrafficlight : 1; }
    int GetPrioStopsign()       { return m_inited ? m_PrioStopsign : 2;}
    int GetPrioEndofroad()      { return m_inited ? m_PrioEndoftheroad : 3; }
    int GetPrioBridge()         { return m_inited ? m_PrioBridge : 11; }
    int GetPrioStreetcount()    { return m_inited ? m_PrioStreetcount : 12; }
    int GetPrioNGPoint(int idx) { return m_inited ? m_PrioNGPoint[idx] : 4; }
    int GetPrioRailroad(int idx)    {return m_inited ? m_PrioRailroad[idx] : 4;}

    int GetPriority(NGType ngType, int idx = 0);
    int GetAreaSizeOfSpeed(int speed, RoadType roadType);

    float GetDisForSameRoad()   { return m_inited ? m_DisForSameRoad : (float)10.0; }
    float GetDisForStack()      { return m_inited ? m_DisForStack : (float)10000.0; }
    float GetTimeForStack()     { return m_inited ? m_TimeForStack : (float)10000.0; }
    float GetDisForStreetCnt()  { return m_inited ? m_DisForStreetCount : (float) 75.0; }
    float GetDisForMultiExitForEX() { return m_inited ? m_DisForMultiExitForEX : (float) 200.0; }
    float GetDegreeOfAngleForEndOfRoad()
    {
        return m_inited ? m_DegreeOfAngleForEndOfRoad : (float)75.0;
    }

    //
    // Maneuver configurations
    //
    // Calculate whether 2 links are horizontal or not
    float m_MinStraightAngle;   // default: 1   TURNANGLE_MIN_STRAIGHT    
    float m_MaxStraightAngle;   // default: 12  TURNANGLE_MAX_STRAIGHT
    float m_SplitStraightAngle; // default: 25  TURNANGLE_SPLIT_STRAIGHT
    float m_HalfStraightAngle;  // default: 6   TURNANGLE_HALF_STRAIGHT
    float m_VeryStraightAngle;  // default: 3   TURNANGLE_VERY_STRAIGHT

    // Turn related angles
    float m_TurnAngle;          // default: 36  TURNANGLE_MIN_TURN
    float m_HardTurnAngle;      // default: 51  TURNANGLE_HARD_TURN
    float m_MinSlightAngle;     // new maneuver, default: 20
    float m_MaxSlightAngle;     // new maneuver, default: 70
    float m_MinHardAngle;       // new maneuver, default: 110
    
    // Keep related angles      
    float m_MaxKeepAngle;       // default: 45  TURNANGLE_MAX_KEEP   
    float m_KeepLimitAngle;     // default: 20  TURNANGLE_MAX_LOW_ANGLE

    // U-Turn releated angles
    float m_MaxUTurnAngle;      // default: 160 TURNANGLE_MAX_UTURN
    float m_UTurnLimitAngle;    // default: 150 TURNANGLE_MAX_UTURN_WITH_TURNLANE
    float m_UTurnSharpAngle;    // default: 120 TURNANGLE_MIN_SHARP
    float m_UTurnFilterAngle;   // default: 110 TURNANGLE_MAX_4ALT_WAY

    float m_MinPerpendicularAngle; // default: 40  TURNANGLE_MIN_PERPENDICULAR
    float m_MaxPerpendicularAngle; // default: 135 TURNANGLE_MAX_PERPENDICULAR

    float m_MinComplexUTurnAngle;  //default: 250 TURNANGLE_MIN_COMPLEX_UTRUN
    float m_MaxComplexUTurnAngle;  //default: 290 TURNANGLE_MAX_COMPLEX_UTRUN

    // Alternative angles and others
    float m_MaxAltAngle;        // default: 130 TURNANGLE_MAX_ALTERNATE
    float m_MinObviousAltAngle; // default: 75  TURNANGLE_MAX_4ALTERNATE
    float m_MaxObviousAltAngle; // default: 110 TURNANGLE_MAX_4ALT_WAY
    float m_MinBearAngle;       // default: 29  TURNANGLE_MIN_BEAR
    float m_MaxBearAngle;       // default: 40  TURNANGLE_MAX_BEAR

    // Max distance from turn lane to intersection, in meters
    float m_TurnlaneDistance;   // default: 90  MAX_TL_DIST_TO_INTR
    float m_SpecialTurnlaneDist;// default: 60  SPECIAL_TURNLANE_LENGTH
    float m_SplitTurnlaneDist;  // default: 100 SPLIT_TURNLANE_LENGTH
    float m_MaxDistForRT;       // default: 100 MAX_DISTANCE_FOR_RT

    float m_MaxDistToMergeAdjacentTurn; // default: 15.0f MIN_DISTANCE_FOR_ADJACENT_TURN
    float m_MinTunnelLengthToGenMan;    // default: 30.0f MIN_DISTANCE_FOR_TUNNEL
    float m_MaxLinkSumLengthToGenMan;   // default: 500.0f MAX_LINK_SUM_LENGTH

    unsigned int m_TrafficLightDelayTime;      // default: 30 seconds
    unsigned int m_StopSignDelayTime;          // default: 20 seconds

private:    
    bool SetPriorityList(const char *pPrio, int *list, int cnt);
    bool SetAreaSize(const char *pStr);

private:
    // NLGuidance configurations
    int m_PrioTrafficlight;
    int m_PrioStopsign;
    int m_PrioEndoftheroad;
    int m_PrioBridge;
    int m_PrioStreetcount;
    static const int m_NGPointCnt = 5;
    int m_PrioNGPoint[m_NGPointCnt];
    static const int m_RailroadCnt = 2;
    int m_PrioRailroad[m_RailroadCnt];

    static const int m_SpeedCnt = 7;
    int m_AreaSizeOfSpeed[m_SpeedCnt * RT_Total];

    float m_DisForStack;
    float m_TimeForStack;
    float m_DisForStreetCount;
    float m_DisForSameRoad;
    float m_DisForMultiExitForEX;
    float m_DegreeOfAngleForEndOfRoad;

    bool    m_inited;
};

enum LaneDividerMarkerType {
	LONG_DASHED   = 1,
	DOUBLE_SOLID  = 2,
	SINGLE_SOLID  = 3,
	SOLID_DASHED  = 4,
	DASHED_SOLID  = 5,
	SHORT_DASHED  = 6,
	SHADED_AREA   = 7,
	DASHED_BLOCKS = 8,
	PHYS_DIVIDER  = 9,
	DOUBLE_DASHED = 10,
	NONE          = 11,
	XING_ALERT    = 12,
	CTR_TURN_LANE = 13	
};
/*
enum LanePosIden {
    DEFAULT_LANE = 0,
	LEFT_LANE ,
	LEFT_2_LANES ,
	LEFT_3_LANES ,
	LEFT_4_LANES ,
    LEFT_5_LANES ,
    LEFT_6_LANES ,
    LEFT_7_LANES ,
    LEFT_8_LANES ,
    LEFT_9_LANES ,
	CENTER_LANE ,
	CENTER_LANES ,
	RIGHT_LANE ,
	RIGHT_2_LANES ,
	RIGHT_3_LANES ,
	RIGHT_4_LANES ,
    RIGHT_5_LANES
    RIGHT_6_LANES
    RIGHT_7_LANES
    RIGHT_8_LANES
    RIGHT_9_LANES
    ALL_LANES
};
*/
enum PrepPronoun {
	PREPRONOUN_NONE = 0,
	PREPRONOUN_THE = 1
};
// Bitmask for lane direction type
const unsigned int LD_STRAIGHT     = 1;
const unsigned int LD_SLIGHT_RIGHT = 2;
const unsigned int LD_RIGHT        = 4;
const unsigned int LD_HARD_RIGHT   = 8;
const unsigned int LD_UTURN_LEFT   = 16;
const unsigned int LD_HARD_LEFT    = 32;
const unsigned int LD_LEFT         = 64;
const unsigned int LD_SLIGHT_LEFT  = 128;
const unsigned int LD_MERGE_RIGHT  = 256;
const unsigned int LD_MERGE_LEFT   = 512;
const unsigned int LD_MERGE_LANES  = 1024;
const unsigned int LD_UTURN_RIGHT  = 2048;
const unsigned int LD_SECOND_RIGHT = 4096;
const unsigned int LD_SECOND_LEFT  = 8192;

// Bitmask for Lane Types
const unsigned short LT_REGULAR     = 1;
const unsigned short LT_HOV         = 2;
const unsigned short LT_REVERSIBLE  = 4;
const unsigned short LT_EXPRESS     = 8;
const unsigned short LT_ACCEL       = 16;
const unsigned short LT_DECEL       = 32;
const unsigned short LT_AUX         = 64;
const unsigned short LT_SLOW        = 128;
const unsigned short LT_PASSING     = 256;
const unsigned short LT_SHOULDER    = 512;
const unsigned short LT_REGULATED   = 1024;
const unsigned short LT_TURN        = 2048;
const unsigned short LT_CENTER_TURN = 4096;
const unsigned short LT_TRUCK_PARK  = 8192;
const unsigned short LT_TOLL_STRUCT = 16384;

typedef AutoArray< unsigned int > ArrowItemList;

class NCDB_API MnvrLaneInfo
{
    public:
        MnvrLaneInfo() 
            : m_LaneNumber(0)
            , m_LaneDirectionType(0)
            , m_LaneDirection(0)
            , m_LaneType(0)
            , m_DividerType(NONE)
            , m_CenterDividerType(NONE)
            , m_Highlight(false) 
        {}
		~MnvrLaneInfo(){}
		//!Gets the position number of the lane based on the Navteq numbering scheme.
        unsigned char  GetLaneNumber()const {return m_LaneNumber;}
        void           SetLaneNumber(unsigned char num) { m_LaneNumber = num; }
		//!Divider Type
        LaneDividerMarkerType GetDividerType()const {return m_DividerType;}
        void                  SetDividerType(LaneDividerMarkerType type) {m_DividerType = type;}
        //!Center Divider Type
        LaneDividerMarkerType GetCenterDividerType()const {return m_CenterDividerType;}
        void                  SetCenterDividerType(LaneDividerMarkerType type) { m_CenterDividerType = type; }
		//!Lane needs highlight if true
        bool IsHighlight()const {return m_Highlight;}
        void SetHighlight(bool bLight) {m_Highlight = bLight;}
        
        /*!Lane type. A lane can be more than one type.
		//!Need to  use the bit mask on the return value to get all the types*/
        unsigned short GetLaneType()const {return m_LaneType;}
        void           SetLaneType(unsigned short type) { m_LaneType = type; }
        
        //!Lane direction -- forward or reverse
        unsigned char GetLaneDirection()const {return m_LaneDirection;}
        void          SetLaneDirection(unsigned char lanedirection) {m_LaneDirection = lanedirection;}
        
        //!Lane direction type -- Straight...
        void SetLaneDirectionType(unsigned char type) { m_LaneDirectionType = type; }
		unsigned char GetLaneDirectionType()const {return m_LaneDirectionType; }

		//!Quick functions to see if a lane belongs to  one of the said types
		
		//!Regular lane defines lanes that do not have a specific use.
        bool IsRegularLane()const {return (LT_REGULAR & m_LaneType) == LT_REGULAR;}
		//!CarPool
        bool IsHOVLane()const {return (LT_HOV & m_LaneType) == LT_HOV;}
		//!Reversible
        bool IsReversibleLane()const {return (LT_REVERSIBLE & m_LaneType) == LT_REVERSIBLE;}
		/*!Express Lane defines that a lane of a road is used for faster 
		moving traffic and has fewer accesses to exits/off ramps*/
        bool IsExpressLane()const {return (LT_EXPRESS & m_LaneType) == LT_EXPRESS;}
       /*!Acceleration Lane defines lanes, typically on the right side of a roadway, that lets a 
		vehicle increase its speed to where it can safely merge with ongoing traffic*/
        bool IsAccelarationLane()const {return (LT_ACCEL & m_LaneType) == LT_ACCEL;}
		/*!!Deceleration Lane defines lanes, typically on the right side 
		of a roadway, that lets a vehicle decrease its speed to where it can
		safely stop or turn*/
        bool IsDecelarationLane()const  {return (LT_DECEL & m_LaneType) == LT_DECEL;}
		/*!Auxiliary Lane defines lanes that run parallel along a motorway and connects the 
		entrance ramp/acceleration lane from one interchange exit ramp/deceleration lane
		of the next interchange*/
        bool IsAuxillaryLane()const  {return (LT_AUX & m_LaneType) == LT_AUX;}
		/*!Slow Lane defines a designated lane on long and/or steep uphill/downhill stretches of 
		high-speed roads to facilitate slow traffic */
        bool IsSlowLane()const  {return (LT_SLOW & m_LaneType) == LT_SLOW;}
		/*!Passing /Overtaking Lane defines a lane that can occur on steep mountain grades or other roads where 
		overtaking slow traffic needs to be regulated*/
        bool IsPassingLane()const  {return (LT_PASSING & m_LaneType) == LT_PASSING;}
		/*!Drivable Shoulder Lane defines a emergency lane that is used as an extra lane for traffic during 
		rush hours, either for buses only or for all vehicles*/
        bool IsShoulderLane()const  {return (LT_SHOULDER & m_LaneType) == LT_SHOULDER;}
        /*!Regulated Lane Access defines a lane designated as a holding zone, used to regulate traffic using 
		time intervals.*/
        bool IsRegulatedLane()const {return (LT_REGULATED & m_LaneType) == LT_REGULATED;}
		/*!Turn Lane identifies an extra lane that is specifically used for making a turn, either left or right, 
		to not disrupt ongoing traffic.*/
        bool IsTurnLane()const  {return (LT_TURN & m_LaneType) == LT_TURN;}
		/*!Center Turn Lane identifies a bidirectional turn lane located in the middle of the road surface that allows 
		traffic in both directions to turn left (right for left side driving countries)*/
        bool IsCenterTurnLane()const {return (LT_CENTER_TURN & m_LaneType) == LT_CENTER_TURN;}
		/*!Truck Parking Lane defines wide shoulder lanes that maybe used for truck
		parking as well as for emergency.*/
        bool IsTruckParkingLane()const  {return (LT_TRUCK_PARK & m_LaneType) == LT_TRUCK_PARK;}
		/*!Gets the Arrow types that needs to be drawn on the lane. 
		Consists of two elements. [0] consists of the arrow item that will not be highlighted.	
		 and [1] is the arrow type that needs to be highlighted
		A bit mask will have to be used to resolve the arrow items as each one could be  
		made up of one or more.*/

        //for now, we didn't check which Arrow need to highlight. will do it later
		ArrowItemList GetArrowType()const { return m_ArrowType; }
        void SetArrowType (ArrowItemList &list) { m_ArrowType = list; }

        bool Straight()const  {return (LD_STRAIGHT & m_LaneDirectionType) == LD_STRAIGHT;}
        bool SlightRight()const {return (LD_SLIGHT_RIGHT & m_LaneDirectionType) == LD_SLIGHT_RIGHT;}
        bool Right()const  {return (LD_RIGHT & m_LaneDirectionType) == LD_RIGHT;}
        bool HardRight()const {return (LD_HARD_RIGHT & m_LaneDirectionType) == LD_HARD_RIGHT;}
        bool UTurnLeft()const  {return (LD_UTURN_LEFT & m_LaneDirectionType) == LD_UTURN_LEFT;}
        bool HardLeft()const {return (LD_HARD_LEFT & m_LaneDirectionType) == LD_HARD_LEFT;}
        bool Left()const {return (LD_LEFT & m_LaneDirectionType) == LD_LEFT;}
        bool SlightLeft()const  {return (LD_SLIGHT_LEFT & m_LaneDirectionType) == LD_SLIGHT_LEFT;}
        bool MergeRight()const  {return (LD_MERGE_RIGHT & m_LaneDirectionType) == LD_MERGE_RIGHT;}
        bool MergeLeft()const {return (LD_MERGE_LEFT & m_LaneDirectionType) == LD_MERGE_LEFT;}
        bool MergeLanes()const  {return (LD_MERGE_LANES & m_LaneDirectionType) == LD_MERGE_LANES;}
        bool UTurnRight()const  {return (LD_UTURN_RIGHT & m_LaneDirectionType) == LD_UTURN_RIGHT;}
        bool SecondRight()const {return (LD_SECOND_RIGHT & m_LaneDirectionType) == LD_SECOND_RIGHT;}
        bool SecondLeft()const  {return (LD_SECOND_LEFT & m_LaneDirectionType) == LD_SECOND_LEFT;}

 
    private:
	    unsigned char	       m_LaneNumber;
	    unsigned char	       m_LaneDirectionType;
        unsigned char          m_LaneDirection;
        unsigned short         m_LaneType;
        LaneDividerMarkerType  m_DividerType;//Optional Attribute
        LaneDividerMarkerType  m_CenterDividerType;//Optional Attribute
        ArrowItemList          m_ArrowType;//Optional Attribute
        bool                   m_Highlight;
		friend class RouteEngine;
        friend class LaneGuidanceGen;

};

typedef AutoArray< MnvrLaneInfo > MnvrLaneInfoList;
#ifdef _MSC_VER
template class NCDB_API AutoArray< MnvrLaneInfo >;
#endif

class NCDB_API GuidancePoint
{
    public:
        GuidancePoint() : m_Prepare(false), m_Turn(false), m_offset(0), m_type(NG_INFO_NONE), m_maxTurnDistance(0){}
		~GuidancePoint(){}
		const WorldPoint&   getPoint() const       {return m_Point;}
        UtfString           getPronunKey() const   {return m_PronunKey;}
        UtfString           getPreposition() const {return m_Preposition;}

        float               getOffset() const      {return m_offset;}

		bool IsPrepare()const {return m_Prepare;}
		bool IsTurn()const {return m_Turn;}

        void SetPoint(WorldPoint &p)    {m_Point = p;}
        void SetPoint(float x, float y)          {m_Point.x = x; m_Point.y = y;}
        void SetOffset(float offset)    {m_offset = offset;}

        void SetPrepare(bool b)         {m_Prepare = b;}
        void SetTurn(bool b)            {m_Turn = b;}

        void SetDesc(UtfString utfStr)  {m_Desc = utfStr;}
        UtfString GetDesc() const       {return m_Desc;}
        void SetPreposition(UtfString str)  {m_Preposition = str;}
        void SetPronunKey(UtfString str)    {m_PronunKey = str;}

        void SetNGType(NGType type)     {m_type = type;}
        NGType GetType()  const         {return m_type;}
        void SetNGMaxTurnDistance(float len) {m_maxTurnDistance = len;}
        float GetMaxTurnDistance() const     {return m_maxTurnDistance;}

    private:
	    //! Position for the guidance point
	    WorldPoint      m_Point;
		//!Pronunciation Key for the guidance point
		UtfString       m_PronunKey;
		//!If true this point can be referenced in the Prepare instruction
		bool    m_Prepare;
        //!If true this guidance point can be referenced in the Turn instruction
		bool    m_Turn;
        //!Description of the Guidance point
		UtfString       m_Desc;
        //!Preposition for the Guidance point
		UtfString       m_Preposition;
		//!Distance from the Maneuverpoint. ==0  If Guidance point is past man.pt .
        float           m_offset;
        //!RoadType
        NGType          m_type;
        float           m_maxTurnDistance;
};

typedef AutoArray< GuidancePoint > GuidancePointList;
#ifdef _MSC_VER
template class NCDB_API AutoArray< GuidancePoint >;
#endif

class NCDB_API LaneGuidanceInfo
{
    public:
		LaneGuidanceInfo():m_NumLanes(0),
			               m_position(0),
		                   m_PrepPronoun(PREPRONOUN_NONE),
                           m_LanePosIdentifier()
		                   {}
        ~LaneGuidanceInfo(){};
        unsigned short  GetNumLanesUsedInManeuver()const {return m_NumLanes;}
        void            SetNumLanesUsedInManeuver(unsigned short num) {m_NumLanes = num;}
        unsigned short  GetStartPosition()const {return m_position;}
        void            SetStartPosition(unsigned short pos) {m_position = pos;}
        const MnvrLaneInfoList&  GetLaneInfoList()const {return m_LaneInfoList;}
        void			SetLaneInfoList(MnvrLaneInfoList& laneInfoList) {m_LaneInfoList = laneInfoList;};

        UtfString       GetPrepPronun() const;
        UtfString       GetPronun() const;
        void            SetPronun();

    private:
        UtfString    GetLanePosIdentifier(Ncdb::UtfString posIden) const;
        bool CheckNonPron() const;
        void CreateLanePosIdentifier();

        //!Number of lanes used in the maneuver
        unsigned short      m_NumLanes;
        //!The position of the lanes
        unsigned short      m_position;
        //!Lane information for each lane populated as a list
        MnvrLaneInfoList        m_LaneInfoList;
        PrepPronoun m_PrepPronoun;
        UtfString m_LanePosIdentifier;
        friend class RouteEngine;
        friend class LaneGuidanceGen;
};   


class NCDB_API ExitNumber
{
public:
    ExitNumber() {}
    UtfString GetExitNumber() const { return m_ExitNumber; }
    UtfString GetPronun() const { return m_Pronun; }   
    void SetExitNumber(UtfString &number) { m_ExitNumber = number; }
    void SetPronun(UtfString &pronun) { m_Pronun = pronun; }
private:
    UtfString m_ExitNumber;
    UtfString m_Pronun;
   
};
};//namespace Ncdb
#endif //#ifndef NLGUIDANCE_H
