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

@file     GeoTransform.h
@date     04/01/2009
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
#ifndef GEOTRANSFORM_H_ONCE
#define GEOTRANSFORM_H_ONCE

#include "NcdbTypes.h"
#include "GeoMath.h"

#ifdef _MSC_VER
template struct NCDB_API geomath::equidistant_pixel<float, int>;
template struct NCDB_API geomath::mercator_pixel<float, int>;

template struct NCDB_API geomath::equidistant_pixel<float, double>;
template struct NCDB_API geomath::mercator_pixel<float, double>;

template struct NCDB_API geomath::equidistant_pixel<double, double>;
template struct NCDB_API geomath::mercator_pixel<double, double>;
template class  NCDB_API geomath::point<double>;
#endif

namespace Ncdb {

//! @brief Map display geometry transform class.
//! @details This class controls the the projection, scale and rotation
//! transforms for map display.
class NCDB_API GeoTransform
{
public:

	//! @brief Default constructor.
	GeoTransform(void);

	//! @brief Default destructor.
	~GeoTransform(void);

    //! @brief Transform rect coords and return minimal rect, what contain transformed rect
    template< class T>
    void px2llBoundingT( Ncdb::WorldRect& to, T x1, T y1, T x2, T y2 ) const;

    //! @brief Transform rect coords and return minimal rect, what contain transformed rect
    //  tolerance -- expand source rect to 'tolerance'
    void px2llBounding( Ncdb::WorldRect& to, const PixelRect& from, int tolerance ) const;

    //! @brief Transform rect coords and return minimal rect, what contain transformed rect
    void px2llBounding( Ncdb::WorldRect& to, const PixelRect& from ) const;



    //! @brief Transform rect coords and return minimal rect, what contain transformed rect
    template< class T>
    void ll2pxBoundingT( T x1, T y1, T x2, T y2, Ncdb::PixelRect& to ) const;

    //! @brief Transform rect coords and return minimal rect, what contain transformed rect
    void ll2pxBounding( const Ncdb::WorldRect& from, Ncdb::PixelRect& to ) const;

    //! @brief Transform rect coords and return minimal rect, what contain transformed rect
    void ll2pxBounding( const Ncdb::WorldRect& from, Ncdb::PixelRect& to, float tolerance ) const;


	//! @brief Convert a coordinate to a screen pixel.
	void ll2px(float lon, float lat, int& x, int& y) const;

	//! @brief Convert a coordinate to a screen pixel.
	void ll2px(float lon, float lat, float& x, float& y) const;

	//! @brief Convert a coordinate to a screen pixel.
    PixelPoint ll2px(const WorldPoint& point) const;

	//! @brief Convert a coordinate to a screen pixel.
    void ll2px(const WorldPointList& points, Ncdb::PixelPointList& result) const;

	//! @brief Convert a screen pixel to a coordinate.
	void px2ll(float& lon, float& lat, int x, int y) const;

	//! @brief Convert a screen pixel to a coordinate.
	void px2ll(float& lon, float& lat, float x, float y) const;

	//! @brief One pixel square as coordinates.
	float getOnePixelArea(void) const;

	//! @brief Returns the pixels per degree with no projection.
	float getDotsPerDegree(void) const;

	//! @brief Set the rotation of the displayed map in degrees.
	//! @param[in] Angle Rotation angle in degrees zero north.
	//! @note The rotation is typically set by the vehicle heading.
	void setRotation(float Angle);

	//! @brief Get the rotation in degrees previously set.
	float getRotation(void) const
	{
		return m_RotationAngle;
	}

	//! @brief Set the scale in meters per pixel at the screen center.
	void setCenterScale(float MetersPerPixel);

	//! @brief Get the center scale in meters per pixel previously set.
	float getCenterScale(void) const
	{
		return m_MetersPerPixel;
	}

	//! @brief Set the scale in meters per pixel at the equator.
	void setEquitorialScale(float MetersPerPixel);

	//! @brief Get the equitorial scale in meters per pixel previously set.
	float getEquitorialScale(void) const
	{
		return m_EquitorialMetersPerPixel;
	}

	//! @brief Explicitly set the center coordinates of the displayed map.
	//! @note The center may be offset by the Center Pixel and the Center Offset.
	void setCoordCenter(double Longitude, double Latitude);

	//! @brief Return the coordinate center previously set.
	const WorldPoint& getCoordCenter(void) const
	{
		return m_CenterLocation;
	}

	//! @brief Explicitly set the center of the map.  The center is typically
	//! the location of the vehicle icon.
	//! @note On 3D maps you may want to set to 1/3 the screen height and
	//! on 2D maps you may want to set to 1/2 the screen height.
	void setCenterPixel(int x, int y);

	//! @brief Explicitly get the center of the map.  The center is typically
	//! the location of the vehicle icon.
    const Ncdb::PixelPoint& getCenterPixel() const { return m_CenterPoint; }

	//! @brief Explicitly set the center offset.
	//! @note The center offset is typically used for panning.  When using
	//! Equidistant map projection its critical to use this offset to move
	//! the map to prevent projection errors.
	void setCenterOffset(int x, int y);

	//! @brief Get the currently set center pixel offset.
	void getCenterOffset(WorldPoint& p) const
	{
		p = m_CenterOffset;
	}

	//! @brief Move the center offset for panning.
	//! @note The center offset is typically used for panning.  When using
	//! Equidistant map projection its critical to use this offset to move
	//! the map to prevent projection errors.
	void addCenterOffset(int x, int y);

	//! @brief Explicitly set the map projection mode.
	void setProjection(ProjectionMode mode);

	//! @brief Return the map projection mode.
	ProjectionMode getProjection(void) const;

	//! @brief Compute the tile location.
	static void getTile(int& tx, int& ty, float tz, float mx, float my);

	//! @brief Comute the tile bounds.
	static void tileBounds(int tx, int ty, float tz, double& left, double& top, double& right, double& bottom);

    //! @brief Comute the tile bounds.
	static void tileBounds(int tx, int ty, float tz, WorldRect& rect);

	//! @brief Comute the tile center.
	static void tileCenterAndScale(int tx, int ty, int tz, int pixels, double& dLon, double& dLat, float& fScale);

	geomath::equidistant_pixel<double, double>      m_Equidistant;
	geomath::mercator_pixel<double, double>         m_Mercator;

private:
	float           m_RotationAngle;
	float           m_MetersPerPixel;
	float           m_EquitorialMetersPerPixel;
	WorldPoint      m_CenterLocation;
    geomath::point<double> m_CenterLonLat;
	PixelPoint      m_CenterPoint;
	WorldPoint      m_CenterOffset;
	ProjectionMode  m_Projection;
	float           m_PixelArea;

    void        updateTransform(void);
};


};

////////////////////////////////////////////////////////////////////////////////////////////////////

inline
Ncdb::PixelPoint Ncdb::GeoTransform::ll2px(const WorldPoint& point) const
{
    PixelPoint result(0,0);

    ll2px( point.x, point.y, result.x, result.y );

    return result;
}

inline
void Ncdb::GeoTransform::ll2px(const WorldPointList& points, Ncdb::PixelPointList& result) const
{
    assert( 0 == result.size( ) );

    result.reserve( points.size( ) );

    for( int n = 0; n != points.size( ); ++ n )
    {
        result.add( ll2px( points[ n ] ) );
    }
}

#endif // GEOTRANSFORM_H_ONCE
/*! @} */
