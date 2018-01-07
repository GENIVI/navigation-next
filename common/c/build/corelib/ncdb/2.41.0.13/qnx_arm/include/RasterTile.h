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

@file     RasterTile.h
@date     06/05/2009
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
#ifndef RASTERTILE_H_ONCE
#define RASTERTILE_H_ONCE

#include "NcdbTypes.h"
#include "RasterControl.h"
#include "Sprite.h"
#include "AutoArray.h"
#include "MapLayer.h"
#include "GeoTransform.h"

namespace Ncdb {

class Session;
class RasterTileImpl;

//! Raster tile creation class.
class NCDB_API RasterTile
{
public:
	//! Raster tile constructor.
	RasterTile(Session& session);

	//! Raster tile class destructor
	~RasterTile(void);

	//! Set the tile to extract.
    void setTile(unsigned int x, unsigned int y, unsigned int z);

    //! Extract the specified tile.
    ReturnCode extract();

	//! Set the tile pixel size.
	void setTileSize(int width, int height);

    //! Set the color display mode.
	void setColorMode(ColorMode Mode);

	//! Set drawing mode with anti-aliased or aliased.
	void setAntiAliased(const bool AntiAliased);

	//! @brief Update the map projection mode.
	void setProjection(ProjectionMode mode);

	//! Set the display configuration.
	void setConfiguration(const char * name);

	//! Set screen dpi.
	void setResolution(const char * dpi);

	//! Set Rotation.
	void setRotation(float angle);
	
	//! Set the center coordinates of the displayed map.
    void setCoordCenter(double Longitude, double Latitude);

	//! Set the scale in meters per pixel at the screen center.
	void setCenterScale(float MetersPerPixel);

	//! Set the label mode.
    void setLabelMode(LabelMode mode);

    //! Add an overlay to the map display.
	void addOverlay(AutoSharedPtr< Overlay > overlay);

	//! Remove all overlays added to the map display.
    void clearOverlays(void);

	//! Set map layer.
    void setMapLayer(MapLayer* MapLayer);

    //! Get the raster draw transformer.
    const GeoTransform* GetTransform();
    
	//! Return sprite.
	bool getTile(Sprite& sprite);
    //! Set the global locale.
    void SetLocale(LocalId lcid);

	//! (Deprecated)Get the raster draw controller.
	RasterControl& getControl(void);
	//! (Deprecated : backward compatibility)Set the number of tiles.
    void setTileMatrix(int rows, int columns) {};
	//! (Deprecated : backward compatibility)Get the specified tiles.
	ReturnCode buildTiles(void);
	//! (Deprecated : backward compatibility)Return each sprite.
	bool getNext(Sprite& sprite);

private:
    RasterTileImpl* m_RasterTileImpl;
};

};
#endif // RASTERTILE_H_ONCE
/*! @} */
