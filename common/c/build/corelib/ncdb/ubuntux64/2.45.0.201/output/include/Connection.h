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
#ifndef NCDB_CCONNECTION_H
#define NCDB_CCONNECTION_H



#include "NcdbTypes.h"
#include "Segment.h"
#include "AutoSharedPtr.h"

namespace Ncdb {


//! This defines the connection for a Src, Destination 
class NCDB_API Connection
{
    public:
		//!Default Constructor
		Connection();
        ~Connection(){};        

        bool operator == (const Connection& rhs) const;
		
		ReturnCode  SetConnection(NodeSegmentID segmentId, const WorldPoint& projectedPoint,StreetSide sideofstreet,int shapeIndex);
		/*! Make the current Connection object a copy of the input argument
		 @param[in] connection
		 @return  NCDB_OK      -  succesfull
                   NCDB_FAIL - Failed. 
        */
		ReturnCode SetConnection(const Connection &connection);
     
       /*!Get the Id of the closest segment.
          @return    SegmentID 
       */
		NodeSegmentID  getSegmentID()const;
       /*!Get the location co-ordinates of the connection.
          @param[out] location - wolrd coordinates of the projected point on the closest segment.
          @return    NCDB_OK      -  succesfull
                   NCDB_FAIL - Failed. 
        */
		ReturnCode  getPointLocation(WorldPoint &location)const;
        /*!Get the side of the street as specified.
          @return    StreetSide StreetSide_Unknown = 0,
                     1 = StreetSide_Left,
                     2 =StreetSide_Right,
                     3 =StreetSide_Both     
        */
		StreetSide getSideOfStreet()const;
         /*!Get the Index of the closest shapePoint.
           @return  m_ShapeIdx 

		  */
		int getShapePointIndex()const;

    private:
	    //! The id of the segment that is closest to the origin point (obtained from the first GPS fix)
		//!or to the destination point(obtained from geocoding the destination address
	   NodeSegmentID	m_SegmentID;		
	    //! projected position of the address on destination segment in Lat/Long
		// !OR projected position of the mapmatched position onto the starting segment
	    WorldPoint	m_Location;
	    //! Side of the segment. Side is decided based on walking the segments along shapepoints from 0...to ...n
	    //! incase of 0 shapepoints then it will be node with smallest lat is 0th shapepoint. If lat are same then
	    //! smallest lon is the 0th shapepoint.
	    StreetSide m_Side;
		//!The index of the closest shapepoint to the projected position
		//!For Origin it will the index of the first shapepoint that occurs in the direction of
		//!the to-end of the segment after m_Location.For Destination it will be the closest one
		//!that occurs before the projected point
	    int m_ShapeIdx;
};
typedef AutoArray<Connection> ConnectionList;
#ifdef _MSC_VER
template class NCDB_API AutoSharedPtr<ConnectionList>;
#endif
};
#endif // NCDB_ROUTESRCDESTGEN_H
