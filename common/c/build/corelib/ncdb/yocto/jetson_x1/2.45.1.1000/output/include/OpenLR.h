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

    @file     OpenLR.h
    @date     10/16/2014
    

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
#ifndef OPENLR_H_IS_INCLUDED
#define OPENLR_H_IS_INCLUDED

#include "NcdbTypes.h"
#include "AutoArray.h"
#include "Segment.h"

typedef enum
{
    FRC_UNDEFINED = -1,     // No defined, ignore FRC
    FRC_MAIN_ROAD = 0,
    FRC_FIRST_CLASS = 1,
    FRC_SECOND_CLASS = 2,
    FRC_THIRD_CLASS = 3,
    FRC_FOURTH_CLASS = 4,
    FRC_FIFTH_CLASS = 5,
    FRC_SIXTH_CLASS = 6,
    FRC_OTHER = 7,      
    FRC_MAX
}FunctionalRoadClass;

typedef enum
{
    FOW_UNDEFINED = 0,      // No defined, ignore FOW
    FOW_MOTORWAY = 1,
    FOW_MULTIPLE_CARRIAGEWAY = 2,
    FOW_SINGLE_CARRIAGEWAY = 3,
    FOW_ROUNDABOUT = 4,
    FOW_TRAFFIC_SQUARE = 5,
    FOW_SLIPROAD = 6,
    FOW_OTHER = 7,
    FOW_MAX
}FormOfWay;

#define OPENLR_INVALID_LATLON  999.0f
#define OPENLR_INVALID_HEADING 777.0f

namespace Ncdb 
{

class  Session;
class  RouteEngine;
struct ConnInfo;

//////////////////////////////////////////////////////////////////////////
struct NCDB_API OpenLRPoint
{
    float m_latitude;
    float m_longitude;
    FunctionalRoadClass m_frc;
    FormOfWay m_fow;
    float m_bearing;
    FunctionalRoadClass m_lowfrc;
    float m_distance;

    OpenLRPoint()
    {
        m_latitude = OPENLR_INVALID_LATLON;
        m_longitude = OPENLR_INVALID_LATLON;
        m_frc = FRC_UNDEFINED;
        m_fow = FOW_UNDEFINED;
        m_bearing = OPENLR_INVALID_HEADING;
        m_lowfrc = FRC_UNDEFINED;
        m_distance = 0;
    }
};
typedef AutoArray<OpenLRPoint> OpenLRPointList;
#ifdef _MSC_VER
template class NCDB_API AutoArray<OpenLRPoint>; // export declaration for DLL
#endif

//////////////////////////////////////////////////////////////////////////
class NCDB_API OpenLRSegment
{
public:
	OpenLRSegment()
	{    
		m_beginOffset = 0;
		m_endOffset = 0;
	};

	virtual ~OpenLRSegment(){};

    const OpenLRPointList& GetPoints() const;    
    float GetBeginOffset() const;
    float GetEndOffset() const;

    ReturnCode appendPoint(const OpenLRPoint &point);
    ReturnCode setBeginOffset(float offset);
    ReturnCode setEndOffset(float offset);

private:
    OpenLRPointList m_points;
    float m_beginOffset;
    float m_endOffset;
};

typedef AutoArray<OpenLRSegment> OpenLRSegmentList;
typedef AutoArray<NodeSegmentID> NodeSegmentIDList;
#ifdef _MSC_VER
template class NCDB_API AutoArray<OpenLRSegment>;
template class NCDB_API AutoArray<NodeSegmentID>;
#endif

//////////////////////////////////////////////////////////////////////////
class NCDB_API OpenLR
{
public:
    OpenLR(Session& session);
    ~OpenLR();

    ReturnCode GetSegmentIDListByOpenLRSegment(NodeSegmentIDList& segmentIDList, 
                                               const OpenLRSegment& openLRSegment);

private:
     Session*     m_Session;     
     RouteEngine* m_RouteEngine;
};


}

#endif //  OPENLR_H_IS_INCLUDED
