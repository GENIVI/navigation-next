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

#pragma once
#ifndef MAP_LAYER_H_ONCE
#define MAP_LAYER_H_ONCE
#include "Sprite.h"
#include "AutoSharedPtr.h"
#include "AutoArray.h"
/*!--------------------------------------------------------------------------

@file     MapLayer.h
@date     01/18/2012
@brief	  to configure Options of drawing
@defgroup MOBIUS_MAP  Mobius Map API
@author   Sheng Wang



*/
/*
(C) Copyright 2011 by Networks In Motion, Inc.                

The information contained herein is confidential, proprietary 
to Networks In Motion, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of Networks In Motion is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
namespace Ncdb {

const unsigned int MAP_LAYER_POLYGON = 0x00000001;
const unsigned int MAP_LAYER_LINEAR = 0x00000002;
const unsigned int MAP_LAYER_ROAD = 0x00000004;
const unsigned int MAP_LAYER_AREA_NAME = 0x00000008;
const unsigned int MAP_LAYER_ROAD_NAME =   0x00000010;
const unsigned int MAP_LAYER_LINEAR_NAME = 0x00000020;
const unsigned int MAP_LAYER_POINT_NAME = 0x00000040;
const unsigned int MAP_LAYER_OVERLAY = 0x00000080;
const unsigned int MAP_LAYER_SATELLITE_FLAG = 0x00000100;
const unsigned int MAP_LAYER_WITHOUT_LABEL = MAP_LAYER_POLYGON|MAP_LAYER_LINEAR|MAP_LAYER_ROAD;
const unsigned int MAP_LAYER_ONLY_LABEL = MAP_LAYER_AREA_NAME|MAP_LAYER_LINEAR_NAME|MAP_LAYER_ROAD_NAME|MAP_LAYER_POINT_NAME;
const unsigned int MAP_LAYER_ALL = MAP_LAYER_WITHOUT_LABEL|MAP_LAYER_ONLY_LABEL|MAP_LAYER_OVERLAY;
const unsigned int MAP_LAYER_SATELLITE = MAP_LAYER_SATELLITE_FLAG|MAP_LAYER_POINT_NAME|MAP_LAYER_ROAD|MAP_LAYER_LINEAR|MAP_LAYER_ROAD_NAME|MAP_LAYER_LINEAR_NAME;
 
	class NCDB_API MapLayer
	{
	public:
		MapLayer();
// 		{
// 
// 		}
/*! @brief set DrawOption

    Set the options of drawing.

    @return	void
*/
		MapLayer(unsigned int nType);
// 		{
// 
// 		}
/*! @brief get DrawOption

    Get the options of drawing.

    @return	the draw option
*/
		unsigned int getLayerType() const;
// 		{
// 			return m_nType;
// 		}
		
		void setLayerType(unsigned int nType);

		bool getNext(Sprite& sprite);
// 		{
// 			bool ret = false;
// 			if (m_nIndex < m_Tiles.size())
// 			{
// 				sprite = m_Tiles[m_nIndex];
// 				++m_nIndex;
// 				ret = true;
// 			}
// 			return ret;
// 		}

		void rewind();
// 		{
// 			m_nIndex = 0;
// 		}
		//! @brief set the Transparency for the map layer tile
		//! @param[in] ucAlpha it represents the transparency(0 represents fully transparent , 255 represents fully opaque)
		void setTransparency(unsigned char ucAlpha);
		//! @brief get the Transparency for the map layer tile
		//! @return  it represents the transparency (0 represents fully transparent , 255 represents fully opaque)
		unsigned char getTransparency();
		MapLayer &operator = (const MapLayer &other);
// 		{
// 			this->m_nType = other.m_nType;
// 			this->m_nIndex = other.m_nIndex;
// 			return *this;
// 		}
	private:
//the type of MayLayer
//it can be the combination of different types. e.g. LAYER_LABELS|LAYER_ROADS
		unsigned int m_nType;
//the index for iterating the map tiles
		unsigned int m_nIndex;
//it stores the transparency(0 represents fully transparent, 255 represents fully opaque)
		unsigned char m_ucAlpha;
		AutoArray<Sprite> m_Tiles;
		friend class RasterTile;
	};
	typedef AutoSharedPtr<MapLayer>     MapLayerPtr;
	typedef AutoArray<MapLayerPtr>      MapLayerPtrList;

};

#endif
