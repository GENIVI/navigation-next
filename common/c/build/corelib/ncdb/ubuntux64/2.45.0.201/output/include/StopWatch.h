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

@file     StopWatch.h
@date     05/13/2009
@defgroup MOBIUS_UTILS  Mobius utility classes 

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

#ifndef STOPWATCH_H_ONCE
#define STOPWATCH_H_ONCE

#include "NcdbTypes.h"

namespace Ncdb {

#ifdef _MSC_VER
	typedef __int64 WatchTime;
#elif __linux__
	typedef long long WatchTime;
#else
	typedef long long WatchTime;
#endif

class NCDB_API StopWatch
{
public:

	typedef enum {
		RenderPolygons=0,
		ShapePolygons,
		RenderRoads,
		ShapeRoads,
		RenderLinears,
		ShapeLinears,
		RenderLabels,
		RenderMap,
		ShaperEnd,
		ShaperBegin,
		Present,
		GeocodeTotal,
		GeocodeQueryTotal,
		GeocodeQueryStep1,
		GeocodeQueryStep2,
		RouteCalc,
		SegmentIDFromLinkId,
		ManeuverGen,
		PolylineGen,
		DrawAreaName,
		DrawRoadLabel,
		DrawLinearName,
		DrawPointName,
		VectorTile,
		CodeTestTimer,
		Max,
	} WatchName;

	static int Counter;	

	static void start(WatchName name);
	static void stop(WatchName name);
    static void resetAll();
	static void reset(WatchName name);
	static WatchTime getTime(WatchName name);
	static WatchTime getCount(WatchName name);
	static WatchTime getFrequency(void);

protected:
	StopWatch(void);
	~StopWatch(void);

};



};
#endif // STOPWATCH_H_ONCE
/*! @} */


