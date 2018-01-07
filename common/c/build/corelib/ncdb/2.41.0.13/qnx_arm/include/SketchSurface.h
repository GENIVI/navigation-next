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

@file     SketchSurface.h
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
#ifndef SKETCHSURFACE_H_ONCE
#define SKETCHSURFACE_H_ONCE


#include "NcdbTypes.h"
#include "RgbColor.h"
#include "AutoSharedPtr.h"

namespace Ncdb {

class SketchEngine;
class Session;
class Sprite;

//! Draw Surface
class NCDB_API SketchSurface
{
public:
	//! Constructor
	SketchSurface(void);

	//! Destructor
	virtual ~SketchSurface(void);

	//! @brief Create the display buffer.
	//! @details Creates the screen buffers for display based
	//! on the input.
	//! @param[in] pixelWidth Back buffer width in pixels.
	//! @param[in] pixelHeight Back buffer height in pixels.
	//! @param[in] colorMode Number of colors in output image.
	virtual ReturnCode create(
		int pixelWidth, int pixelHeight,
		ColorMode colorMode = ColorModeBGR24) = 0;

	//! @brief Check if properly created.
	bool isCreated(void) { return (m_Created); }

	//! @brief Check if surface was resized.
	bool isResized(void) { return (m_Resized); }

	//! @brief Set the status of resize.
	void setResized(bool resized) { m_Resized = resized; }

	//! @brief Return the display engine.
	virtual SketchEngine& getEngine(void) = 0;

	virtual void bitBlit(void * hdc, int OffsetX=0, int OffsetY=0) = 0;

	virtual void moveArea(PixelRect& Source, int dx, int dy) = 0;

	virtual void clearArea(PixelRect& Rect, RgbColor Color) = 0;

	PixelRect& getBoundry(void) {return m_ScreenRect;}

	void getResolution(int& Width, int& Height);

	virtual void resize(int width, int height) = 0;

	virtual void copyArea(Sprite& sprite, PixelRect& destination, PixelRect& source) = 0;

	virtual void setColorMode(const ColorMode color);

	virtual ColorMode getColorMode(void) const ;

	virtual void setAntiAliased(const bool AntiAliased);

	virtual bool getAntiAliased();
protected:

	// Protected members.
	ColorMode m_ColorMode;
	PixelRect m_ScreenRect;
	bool m_Created;
	bool m_Resized;
	bool m_AntiAliased;

};

typedef AutoSharedPtr<SketchSurface> SharedSurface;
#ifdef _MSC_VER
template class NCDB_API AutoSharedPtr<SketchSurface>;
#endif

};
#endif // SKETCHSURFACE_H_ONCE
/*! @} */
