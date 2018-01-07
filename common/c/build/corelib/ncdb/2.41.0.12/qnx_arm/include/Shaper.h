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

@file     Shaper.h
@date     03/05/2009
@defgroup MOBIUS_MAP  Mobius Map Draw API
@author   John Efseaff



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
#ifndef SHAPER_H_ONCE
#define SHAPER_H_ONCE

#include "NcdbTypes.h"
#include "GeoTransform.h"


namespace Ncdb {

class SketchSurface;
class RasterStyleHandle;
class RoadShape;
class AreaShape;
class LinearShape;
class PointShape;
class PoiShape;
class LabelShape;
class RoadLabel;
class HighwayShield;
class LinearLabel;
class PointLabel;
class PoiLabel;
class AreaLabel;
class Sprite;
class UtfString;


//! Map Shaper Base Class
class NCDB_API Shaper
{
public:
	//! Constructor
	Shaper(void);
	//! Destructor
	virtual ~Shaper(void);

	virtual void begin(const PixelRect &ClipArea, const WorldRect &WorldArea, bool isBkTransp) = 0;
	virtual void end(const unsigned char* shrlfData = 0) = 0;

	virtual void drawRoadFeature(RoadShape& Shape) = 0;
	virtual void drawLinearFeature(LinearShape& Shape) = 0;
	virtual bool drawAreaFeature(AreaShape& Shape) = 0;
    virtual bool drawCoverageFeature(AreaShape& Shape, bool isIndiaVersion = false) = 0;
	virtual void drawPointFeature(PointShape& Shape) = 0;
	virtual void drawPoiFeature(PoiShape& Shape) = 0;
    virtual void drawSatelliteFeature() = 0;

	virtual void drawRoadLabel(RoadLabel* shape) = 0;
	virtual void drawLinearName(LinearLabel* shape) = 0;
	virtual void drawAreaName(AreaLabel* shape) = 0;
	virtual void drawPointName(PointLabel* shape, bool bDrawIcon = false) = 0;

    // Log drawn area and point labels
    virtual void setLogLabels(bool logLabels) = 0;
    virtual void getAreaLabels(AutoArray<AreaLabel*>& labels) = 0;
    virtual void getPointLabels(AutoArray<PointLabel*>& labels) = 0;

	virtual const RasterStyleHandle&    getStyle(void) const = 0;
	virtual void                        setStyle(const RasterStyleHandle& Handle)= 0;
	virtual bool NeedForceDraw(const GeoTransform&) { return false; }

	virtual bool GetRoadWidth(RoadShape& Shape, double& Width) = 0;

	virtual ReturnCode attachSurface(SketchSurface* Surface) = 0;

	const GeoTransform* getTransform(void) const
	{
		return &m_Transform;
	}

	void pasteTransform(const GeoTransform& transform)
	{
		m_Transform = transform;
	}

protected:

	GeoTransform    m_Transform;
	PixelRect       m_ClipArea;
	WorldRect       m_WorldArea;

};


};
#endif // SHAPER_H_ONCE
/*! @} */
