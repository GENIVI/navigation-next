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

@file     SketchEngine.h
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
#ifndef SKETCHENGINE_H_ONCE
#define SKETCHENGINE_H_ONCE


#include "NcdbTypes.h"
#include <utility>

namespace Ncdb {

class Sprite;
class SketchSurface;
class UniString;
class Font;
class FileName;
class RgbColor;
class PixelPath;

typedef unsigned int FontName;
typedef unsigned int SpriteName;

class NCDB_API SketchEngine
{
public:

	SketchEngine(void);

	virtual ~SketchEngine(void);

	virtual void begin(void) = 0;

	virtual void end(void) = 0;

	virtual void setClipRect(const PixelRect& Clip) = 0;

	virtual void drawRectangle(const PixelRect& rectangle, const RgbColor& color) = 0;

	virtual void drawPolyline(int width, const PixelPointList& polyline, const RgbColor& color) = 0;

	virtual void drawPolygon(void) = 0;

	virtual void drawSprite(const PixelPoint& p, const Sprite* sprite) = 0;

	virtual void drawSprite(const PixelPoint& p, SpriteName sprite) = 0;

	virtual void drawText(const PixelPoint& p, const UniString& uni, FontName name) = 0;
	
	virtual void pathText(const PixelPath& p, double start, int direction, const UniString& s, FontName name) = 0;

	virtual void angleText(const PixelPoint& p, double rotation, const UniString& uni, FontName name) = 0;

	virtual SketchSurface* getSurface(void) = 0;

	virtual int getTextWidth(const UniString& uni, FontName name) const = 0;

	virtual int getTextHeight(const UniString& uni, FontName name) const = 0;

	virtual FontName addFont(const FileName& fn, Font& font) = 0;

    virtual void ConvertTextCase(UniString& uni, FontName fn) = 0;

    virtual void getFonts(AutoArray<std::pair<const Font*, const FileName*> >& fonts) const = 0;

	virtual int getSpriteWidth(SpriteName name) const = 0;

	virtual int getSpriteHeight(SpriteName name) const = 0;

	virtual SpriteName addSprite(const FileName& fn, Sprite& sprite) = 0;

	virtual void textBounds(AutoArray<PixelRect>& rects, const PixelPoint& p, const UniString& uni, FontName name) = 0;

protected:

};

};
#endif // SKETCHENGINE_H_ONCE
/*! @} */
