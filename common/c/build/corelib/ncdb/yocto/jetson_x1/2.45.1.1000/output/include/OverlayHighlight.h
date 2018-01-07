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

@file     OverlayHighlight.h
@date     02/06/2010
@defgroup MOBIUS_MAP  Mobius Map Draw API
@author   John Efseaff



*/
/*
(C) Copyright 2010 by Networks In Motion, Inc.                

The information contained herein is confidential, proprietary 
to Networks In Motion, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of Networks In Motion is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */
#pragma once
#ifndef OVERLAYHIGHLIGHT_H_ONCE
#define OVERLAYHIGHLIGHT_H_ONCE

#include "Overlay.h"
#include "AscString.h"

namespace Ncdb
{

class Route;

class NCDB_API HighlightPolyline
{
	friend class OverlayHighlight;
	WorldPointList m_Polyline;
	RgbColor m_Color;
	int m_Width;

public:
	//! Default Constructor
	HighlightPolyline(void);
	//! Constructor
	HighlightPolyline(int width, RgbColor color,
		const WorldPointList& polyline);
	//! Destructor
	~HighlightPolyline(void);
	//! Return the width of the highlight.
	int getWidth(void) const;
	//! Return the color of the highlight.
	const RgbColor& getColor(void) const;
	//! Set the width.
	void setWidth(int width);
	//! Set the color.
	void setColor(const RgbColor& color);

};

#ifdef _MSC_VER
template class NCDB_API AutoArray < HighlightPolyline >;
#endif

class NCDB_API OverlayHighlight : public Overlay
{
	AutoArray < HighlightPolyline > m_Highlights;

public:
	//! Set each highlight.  Same name replaces old one.
	void add(const HighlightPolyline& highlight);
	//! Draw the overlay
	virtual void draw(const WorldRect& wpRect, const PixelRect& pxRect);
	//! Update to the new coordinates.
	virtual void update(RasterControl* control);
	//! Clear all added highlights.
	void reset(void);

};




};
#endif // OVERLAYHIGHLIGHT_H_ONCE
/*! @} */


