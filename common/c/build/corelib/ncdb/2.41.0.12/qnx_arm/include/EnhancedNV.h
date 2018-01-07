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

#ifndef file__NCDB_VXOBJECT_H
#define file__NCDB_VXOBJECT_H

#include "NcdbTypes.h"      // LinkDirectionOW
#include "WorldPoint.h"
#include "AutoArray.h"      
#include "UtfString.h"      
#include "AutoSharedPtr.h"



// legend:
// 
// MJO - Motorway Junction Object
// cite: Motorway Junction Objects is a 3D application which displays 
//       animated cues to help the driver make better driving decisions. 
//       For example, when a busy junctions approaching, the system can 
//       turn from 2D display to 3D mode, to highlight the correct lane 
//       and the signs which follow. Simple put, missing a highway exit 
//       while using Motorway Junction Objects will be entirely your fault, 
//       as the system has no way of getting it wrong.
// ECM - 3D city maps
// SAR - Signs As Real (photos of road signs)

namespace Ncdb
{

// actually, Servlet don't need SplineData, they just need a buffer
// the content of this buffer is described by SplineData
// struct SplineData
// {
//     unsigned int                fromLinkPVID;
//     unsigned int                toLinkPVID;
//     LinkDirection               fromLinkDirection;
//     LinkDirection               toLinkDirection;
// 
//     unsigned int                vertexCount;
// 
//     float*                      positionsArray;     // [0..vertexCount)
//     float*                      outtangentsArray;   // [0..vertexCount)
//     float*                      intangentsArray;    // [0..vertexCount)
// };
struct SplineBinary
{
 	int bufferLength;
	AutoSharedPtr< char > dataBuffer;
};
// ------------------------------------------------------------------ SplineObject {}
class NCDB_API SplineObject
{
public:

    SplineObject()
    {
    }
    ~SplineObject()
    {
    }

	const UtfString& GetID() const
	{
		return m_strID;
	}
	WorldPoint  GetStartCoord() const
	{
		return m_startCoord;
	}
	WorldPoint  GetEndCoord() const
	{
		return m_endCoord;
	}
	//const SplineData&  GetSplineData() const
	//{
	//	return m_splineData;
	//}
	const SplineBinary&  GetSplineBinary() const
	{
		return m_splineBinary;
	}

private:
	UtfString m_strID;
	WorldPoint m_startCoord;
	WorldPoint m_endCoord;
	//! spline data
	//SplineData m_splineData;
	//! spline data, binary format
	SplineBinary m_splineBinary;
	friend class RouteEngine;
};

typedef AutoArray<SplineObject>		SplineObjectList;
#ifdef _MSC_VER
template class NCDB_API				AutoArray<SplineObject>;
template class NCDB_API				AutoSharedPtr<SplineObjectList>;
#endif

struct ProjectionParam
{
	double originLat;
	double originLon;
	double zOffset;
	double scaleFactor;		//currently, always be 0.9996
	double falseEasting;	//currently, always be 500000.0
	double falseNorthing;	//currently, always be 10000000.0
	UtfString strDatum;		//currently, always be "WGS84"
};
class NCDB_API EnNVObject
{
public:
	EnNVObject();

	const UtfString&    GetID() const;
	const UtfString&    GetType() const;
    const UtfString&    GetVersion() const;
	const UtfString&    GetDataSetID() const;
	const UtfString&	GetProjection() const;
	unsigned int        GetStartManeuverIndex() const;
	unsigned int        GetEndManeuverIndex() const;
    double              GetStartManeuverOffset() const;
	double              GetEndManeuverOffset() const;

	const ProjectionParam&	GetProjectionParam() const;
	const WorldRect&	GetBoundingBox() const ;

    AutoSharedPtr<SplineObjectList>  GetSplineObjectList() const;

private:
	UtfString		m_strID;
	UtfString		m_strType;
	UtfString		m_strVersion;
	UtfString		m_strDatasetID;
	UtfString		m_strProjection;
	unsigned int	m_startManIndex;
	unsigned int	m_endManIndex;
	double			m_startManOffset;
	double			m_endManOffset;

	ProjectionParam m_pparams;
	WorldRect       m_boundingBox;

	AutoSharedPtr<SplineObjectList> m_SplineObjectList;

	friend class RouteEngine;
};

typedef AutoArray<EnNVObject>     EnNVObjectList;
#ifdef _MSC_VER
template class NCDB_API AutoArray<EnNVObject>;
template class NCDB_API AutoSharedPtr<EnNVObjectList>;
#endif

} // namespace Ncdb

#endif // file__NCDB_VXOBJECT_H

