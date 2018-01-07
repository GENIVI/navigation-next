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

/*!------------------------------------------------------------------

@file     MercatorTile.h
@date     01/1/2010
@brief    Some math behind Mercator projection

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

---------------------------------------------------------------------*/

#ifndef _MERCATOR_TILE_H_INCLUDED_
#define _MERCATOR_TILE_H_INCLUDED_

#include <math.h>

namespace nbm
{

typedef unsigned int uint32_t;

struct MERCATOR_TILE_INFO
{
  double lat;
  double lon;
  double width;
};

void get_mercator_tile_info(int x, int y, int z, MERCATOR_TILE_INFO* ti);

class Mercator
{
public:
  Mercator(uint32_t tile_size);

  void convert_ll_to_meters(double lat, double lon,
			    double& mx, double& my);
  void convert_meters_to_ll(double mx, double my,
			    double& lat, double& lon);

  void convert_ll_to_pixels(double lat, double lon, uint32_t z,
			    uint32_t& x, uint32_t& y);

  void convert_pixels_to_meters(double px, double py, uint32_t z,
				double& mx, double& my);
  void convert_meters_to_pixels(double mx, double my, uint32_t z,
				double& px, double& py);

  void get_tile_for_pixel(double px, double py,
			  uint32_t& tx, uint32_t& ty);
  void convert_pixels_to_raster(double px, double py, uint32_t z,
				double& rx, double& ry);
  void convert_meters_to_tile(double mx, double my, uint32_t z,
			      uint32_t& nx, uint32_t& ny);

  void get_tile_bounds_meters(uint32_t nx, uint32_t ny, uint32_t z,
			      double& minx, double& miny, double& maxx, double& maxy);
  void get_tile_bounds_ll(uint32_t nx, uint32_t ny, uint32_t z,
			  double& minlat, double& minlon, double& maxlat, double& maxlon);

  void get_dts_coordinates(uint32_t tx, uint32_t ty, uint32_t z,
			   uint32_t& nx, uint32_t& ny);
  void get_tms_coordinates(uint32_t nx, uint32_t ny, uint32_t z,
			   uint32_t& tx, uint32_t& ty);

private:
  uint32_t m_tile_size;
  double m_origin_shift;
};

};

#endif //_MERCATOR_TILE_H_INCLUDED_
