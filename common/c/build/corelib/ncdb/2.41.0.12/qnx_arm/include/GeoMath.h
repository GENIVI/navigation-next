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

@file     GeoMath.h
@date     04/03/2009
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
#ifndef GEOMATH_H_ONCE
#define GEOMATH_H_ONCE

#include "NcdbTypes.h"
#include <math.h>
#include <stdlib.h>


namespace geomath
{
	static const double pi = 3.14159265358979323846;
	static const double MM_PER_INCH = 25.4;
	static const double INCH_PER_METER = (1.0/0.0254);
	static const double METERS_PER_FOOT = 0.3048;
	static const double METERS_PER_MILE = 1609.344;
	static const int FEET_PER_MILE = 5280;
	static const int METERS_PER_KILOMETER = 1000;
	static const double RADIUS_EARTH_METERS_ = 6378137.0;
	static const double METERS_PER_DEGREE = (2 * pi * RADIUS_EARTH_METERS_ / 360.0);
	static const int NIM_COORD_UNITS = 0x020000;

	template<typename T> T Max(T a, T b) { return (a > b) ? a : b; }
	template<typename T> T Min(T a, T b) { return (a < b) ? a : b; }
	template<typename T>
	inline T degree(T r) { return r * (T)180.0 / (T)3.14159265358979323846; }
	template<typename T>
	inline T radian(T d) { return d * (T)3.14159265358979323846 / (T)180.0; }

	static const double EarthRadius = 6378137.0;
	static const double MetersPerDegree = 2 * pi * EarthRadius / 360.0;

	template <typename T> class rect_tag { public: T left, top, right, bottom; };
	template <typename T> class rect : public rect_tag<T>	{};
	template <typename T> class worldrect : public rect_tag<T> {};
	template <typename T> class point_tag { public: T x, y; };
	template <typename T> class point : public point_tag<T> {};
	template <typename T> class worldpoint : public point_tag<T> {};
	template <typename T> class line { public: point_tag<T> p1, p2; };
	template <typename T> class mercator { };
    template <typename T> class equidistant { };

	template <typename W, typename P>
	struct mercator_pixel
	{
		W m_Latitude;
		W m_Longitude;
		P m_OffsetX;
		P m_OffsetY;
		W m_DotsPerDegree;
		W m_DotsPerRadian;
		W m_LatitudeMercator;
		W m_LongitudeMercator;
		W m_Sin;
		W m_Cos;
		//! Assignment constructor.
		mercator_pixel(W CenterLatitude, W CenterLongitude, P OffsetX, P OffsetY, W MetersPerPixel, float Angle=0)
				: m_Latitude(CenterLatitude)
				, m_Longitude(CenterLongitude)
				, m_OffsetX(OffsetX)
				, m_OffsetY(OffsetY)
		{
			m_LongitudeMercator = radian(CenterLongitude);
			double rad = radian(CenterLatitude);
			m_LatitudeMercator = (W)( log( tan(rad) + 1.0/cos(rad) ) );

			m_DotsPerDegree = (W)(MetersPerDegree / MetersPerPixel);
			m_DotsPerRadian = degree(m_DotsPerDegree);

			if (Angle == 0)
			{
				m_Sin = 0;
				m_Cos = 1;
			}
			else
			{
				rad = radian(Angle);
				m_Sin = (W)sin(rad);
				m_Cos = (W)cos(rad);
			}

		}		
		
		//! Default constructor.
		mercator_pixel(void){}
	};


	template <typename W, typename P>
    struct equidistant_pixel
	{
		W m_CenterLatitude;
		W m_CenterLongitude;
		P m_OffsetX;
		P m_OffsetY;
		W m_DotsPerDegreeX;
		W m_DotsPerDegreeY;
		W m_CosineLatitude;
		W m_Sin;
		W m_Cos;
		//! Assignment constructor.
		equidistant_pixel(W CenterLatitude, W CenterLongitude, P OffsetX, P OffsetY, W MetersPerPixel, float Angle=0)
		        : m_CenterLatitude(CenterLatitude)
		        , m_CenterLongitude(CenterLongitude)
		        , m_OffsetX(OffsetX)
		        , m_OffsetY(OffsetY)
	    {
		    double rad = radian(CenterLatitude);
		    m_CosineLatitude = (W)cos(rad);
		    m_DotsPerDegreeX = (W)(m_CosineLatitude * MetersPerDegree / MetersPerPixel);
		    m_DotsPerDegreeY = (W)(MetersPerDegree / MetersPerPixel);
		    if (Angle == 0)
		    {
			    m_Sin = 0;
			    m_Cos = 1;
		    }
		    else
		    {
			    rad = radian(Angle);
			    m_Sin = (W)sin(rad);
			    m_Cos = (W)cos(rad);
		    }

	    }
		//! Default constructor.
		equidistant_pixel(void){}
	};


	template<typename T>
	inline void swap(point_tag<T>& p)
	{
		T a;
		a = p.x;
		p.x = p.y;
		p.y = a;
	}

	template <typename R, typename P>
	inline void add(rect_tag<R>& r, const point_tag<P>& p)
	{
		r.left += p.x;
		r.right += p.x;
		r.top += p.y;
		r.bottom += p.y;
	}

	template <typename P>
	inline void add(point_tag<P>& p, P x, P y)
	{
		p.x += x;
		p.y += y;
	}

	template <typename P>
	inline void add(point_tag<P>& p1, point_tag<P>& p2)
	{
		p1.x += p2.x;
		p1.y += p2.y;
	}

	template <typename R, typename P>
	inline void sub(rect_tag<R>& r, const point_tag<P>& p)
	{
		r.left -= p.x;
		r.right -= p.x;
		r.top -= p.y;
		r.bottom -= p.y;
	}

	template <typename P>
	inline void sub(point_tag<P>& p, P x, P y)
	{
		p.x -= x;
		p.y -= y;
	}

	template <typename P>
	inline void sub(point_tag<P>& p1, point_tag<P>& p2)
	{
		p1.x -= p2.x;
		p1.y -= p2.y;
	}

	template <typename R1, typename R2>
	inline void clip(rect_tag<R1>& r1, const rect_tag<R2>& r2)
	{
		if (r1.left < r2.left) r1.left = r2.left;
		if (r1.right > r2.right) r1.right = r2.right;
		if (r1.top < r2.top) r1.top = r2.top;
		if (r1.bottom > r2.bottom) r1.bottom = r2.bottom;
	}

	//! Copy one point to another.
	template <typename P1, typename P2>
	inline void copy(point_tag<P1>& p1, point_tag<P2> p2)
	{
		p1.x = (P1)p2.x;
		p1.y = (P1)p2.y;
	}

	//! Copy one rectangle to another.
	template <typename R1, typename R2>
	inline void copy(rect_tag<R1>& r1, rect_tag<R2> r2)
	{
		r1.top = (R1)r2.top;
		r1.left = (R1)r2.left;
		r1.bottom = (R1)r2.bottom;
		r1.right = (R1)r2.right;
	}

	//! Assign the point from two values.
	template <typename T1, typename T2>
	inline void assign(point_tag<T1>& p, T2 p1, T2 p2)
	{
		p.x = (T1)p1;
		p.y = (T1)p2;
	}

	//! Assign the rectangle from two points.
	template <typename R, typename P>
	inline void assign(rect_tag<R>& r, const point_tag<P> p1, const point_tag<P> p2)
	{
		r.top = (R)p1.y;
		r.left = (R)p1.x;
		r.bottom = (R)p2.y;
		r.right = (R)p2.x;
	}

	//! Assign the rectangle to individual values.
	template <typename R, typename T>
	inline void assign(rect_tag<R>& r, const T top, const T left, const T bottom, const T right)
	{
		r.left = (R)left;
		r.right = (R)right;
		r.top = (R)top;
		r.bottom = (R)bottom;
	}

	//! Assign the line from two points.
	template <typename L, typename P>
	inline void assign(line<L>& r, const point_tag<P> p1, const point_tag<P> p2)
	{
		r.p1.x = (L)p1.y;
		r.p1.y = (L)p1.x;
		r.p2.x = (L)p2.y;
		r.p2.y = (L)p2.x;
	}

	//! Assign the line to individual values.
	template <typename L, typename T>
	inline void assign(line<L>& r, const T x1, const T y1, const T x2, const T y2)
	{
		r.p1.x = (L)x1;
		r.p1.y = (L)y1;
		r.p2.x = (L)x2;
		r.p2.y = (L)y2;
	}

	//! Translate the rect to PixelRect.
	template <typename T>
	Ncdb::PixelRect translate(rect<T>& in)
	{
		Ncdb::PixelRect out;
		out.left = (T)in.left;
		out.right = (T)in.right;
		out.top = (T)in.top;
		out.bottom = (T)in.bottom;
		return out;
	}

	//! Translate the worldrect to WorldRect.
	template <typename T>
	Ncdb::WorldRect translate(worldrect<T>& in)
	{
		Ncdb::WorldRect out;
		out.m_Left = (T)in.left;
		out.m_Right = (T)in.right;
		out.m_Top = (T)in.top;
		out.m_Bottom = (T)in.bottom;
		return out;
	}

	//! Translate the degree to ncu.
	template <typename T>
	inline void deg2ncu(int& out, const T in)
	{
		out = ( (int) (in * (T)(NIM_COORD_UNITS / 360.0) + (T)((in) >= 0.0 ? 0.5 : -0.5)) );
	}

	//! Translate the degree to ncu.
	template <typename T>
	inline void ncu2deg(T& out, const int in)
	{
		out = (T)in * (T)(360.0 / NIM_COORD_UNITS);
	}

	//! Translate the point from ncu to degree.
	template <typename T>
	inline void ncu2deg(worldpoint<T>& out, const point<int>& in)
	{
		ncu2deg(out.x, in.x);
		ncu2deg(out.y, in.y);
	}

	//! Translate the point from degree to ncu.
	template <typename T>
	inline void deg2ncu(point<int>& out, const worldpoint<T>& in)
	{
		deg2ncu(out.x , in.x);
		deg2ncu(out.y, in.y);
	}

	//! Translate the rectangle from ncu to degree.
	template <typename T>
	inline void ncu2deg(worldrect<T>& out, const rect<int>& in)
	{
		ncu2deg(out.left, in.left);
		ncu2deg(out.top, in.top);
		ncu2deg(out.right, in.right);
		ncu2deg(out.bottom, in.bottom);
	}

	//! Translate the rectangle from degree to ncu.
	template <typename T>
	inline void deg2ncu(rect<int>& out, const worldrect<T>& in)
	{
		deg2ncu(out.left, in.left);
		deg2ncu(out.top, in.top);
		deg2ncu(out.right, in.right);
		deg2ncu(out.bottom, in.bottom);
	}

	//! Test if a point is inside a rectangle.
	template < typename T >
	inline bool within (const worldrect<T> &r, T x, T y)
	{
		if( y <= r.top )
		{
			if( y >= r.bottom )
			{
				T tl, tr, tx;
				tl = r.left;
				tr = r.right;
				tx = x;
				if (tl > tr)
				{
					tr += 360;
				}
				if (tx < tl)
				{
					tx += 360;
				}
				if (tx >= tl && tx <= tr)
				{
					return true;
				}
			}
		}
		return false;
	}

	//! Test if a point is inside a rectangle.
	template < typename T >
	inline bool within (const rect<T> &r, T x, T y)
	{
		return ( y >= r.top &&
				 y <= r.bottom &&
				 x >= r.left &&
				 x <= r.right );
	}

	//! Test if a point is inside a rectangle.
	template < typename T >
	inline bool within (const rect_tag<T> &r, const point_tag<T> &p)
	{
		return (within(r, p.x, p.y));
	}

	//! Test if rectangles overlap.
	template < typename T >
	inline bool overlaps (const rect_tag<T> &r, T left, T top, T right, T bottom)
	{
		bool overlaps = false;
		if( top > r.top )
		{
			if( r.top >= bottom )
			{
				overlaps = true;
			}
		}
		else
		{
			if( top >= r.bottom )
			{
				overlaps = true;
			}
		}
		if( overlaps )
		{
			if( left < r.left )
			{
				if( r.left <= right )
				{
					return true;
				}
			}
			else
			{
				if( left <= r.right )
				{
					return true;
				}
			}
		}
		return false;
	}

	//! Test if rectangles overlap.
	template < typename T >
	inline bool overlaps (const rect_tag<T> &r1, const rect_tag<T> &r2)
	{
		return overlaps(r1, r2.left, r2.top, r2.right, r2.bottom);
	}

	//! Test if two rectangles intersect.
	template < typename T >
	inline bool intersects(const rect_tag<T>& r, T x1, T y1, T x2, T y2)
	{
		T left      = ( x1 < x2 ) ? x1 : x2;
		T top       = ( y1 > y2 ) ? y1 : y2;
		T right     = ( x1 > x2 ) ? x1 : x2;
		T bottom    = ( y1 < y2 ) ? y1 : y2;
		return overlaps(r, left, top, right, bottom );
	}

	//! Test if two rectangles intersect.
	template < typename T >
	inline bool intersect(const rect_tag<T>& r1, const rect_tag<T>& r2)
	{
		return intersects( r1, r2.left, r2.top, r2.right, r2.bottom );
	}

	//! Test if lines intersect.
	template < typename T >
	inline bool intersect(const T x1_min, const T x1_max, const T x2_min, const T x2_max)
	{
		if (x1_min <= x2_min && x1_max >= x2_max)
			return true;
		else if (x1_max > x2_min && x1_max <= x2_max)
			return true;
		else if (x1_min >= x2_min && x1_min < x2_max)
			return true;
		else
			return false;
	}

	//! Compute the intersecting point between two lines.
	template < typename P, typename L >
	inline bool intersect(point_tag<P>& p, const line<L>& line1, const line<L>& line2)
	{
		L denominator = ( (line2.p2.y - line2.p1.y)*(line1.p2.x - line1.p1.x) -
			(line2.p2.x - line2.p1.x)*(line1.p2.y - line1.p1.y) );
		L y13 = line1.p1.y - line2.p1.y;
		L x13 = line1.p1.x - line2.p1.x;
		L numerator_a = (line2.p2.x - line2.p1.x) * y13 - (line2.p2.y - line2.p1.y) * x13;
		L numerator_b = (line1.p2.x - line1.p1.x) * y13 - (line1.p2.y - line1.p1.y) * x13;

		if (denominator == (L)0)
			return false; // parallel lines

		if (numerator_a == (L)0 && numerator_b == (L)0 && denominator == (L)0)
			return false; // coincident lines

        L slope = 0;    // made an invalid value to compile, but cause run-time error [AV]
		L ua = numerator_a / slope;
		L ub = numerator_b / slope;

		p.x = (P)( line1.p1.x + ua * (line1.p2.x - line1.p1.x) );
		p.y = (P)( line1.p1.y + ua * (line1.p2.y - line1.p1.y) );

		return (ua >= (L)0 && ua <= (L)1 && ub >= (L)0 && ub <= (L)1);
	}

	//! Compute the spherical distance.
	template < typename T, typename O >
	inline O distance (const worldpoint<T> &p1, const worldpoint<T> &p2)
	{
	}

	//! Compute the euclidian distance.
	template < typename T, typename O >
	inline O distance (const point<T> &p1, const point<T> &p2)
	{
	}

	//! Project the point on the line.
	template< typename T >
	inline point_tag<T>& project (const line<T>& line1, const line<T>& line2)
	{
		point_tag<T> out;
		out.x = 0;
		out.y = 0;
		return (out);
	}

	//! @brief Translate the coordinates to a equidistant projected pixel.
	template < typename W >
	void forward (const equidistant<W> &m, W& x, W& y)
	{
        y = radian( y );
        x = radian( x ) * cos( y );
	}

	//! Translate mercator radian coordinate to WGS84 degree coordinate.
	template < typename W >
	inline void reverse (const equidistant<W> &m, W& x, W& y)
	{
        W cos_ = cos( y );

        if( 0 != cos_ )
        {
            x = degree( x / cos_ );
        }
        else
            x = 0; // any value.

        y = degree( y );
	}





	//! @brief Translate the coordinates to a equidistant projected pixel.
	template < typename W >
	inline point<int> forward (const equidistant_pixel<W, int> &r, const worldpoint<W> &p)
	{
		point<int> out;

		W dx = p.x - r.m_CenterLongitude;
		W dy = p.y - r.m_CenterLatitude;

		dx =  dx * r.m_DotsPerDegreeX;
		dy = -dy * r.m_DotsPerDegreeY;

		W x = dx * r.m_Cos + dy * -r.m_Sin;
		W y = dx * r.m_Sin + dy * r.m_Cos;

		out.x = r.m_OffsetX + (int) ( (x>=0)?(x+0.5):(x-0.5) );
		out.y = r.m_OffsetY + (int) ( (y>=0)?(y+0.5):(y-0.5) );

		return out;
	}

	//! @brief Translate the coordinates to a equidistant projected pixel.
	template < typename W, typename P >
	inline point<P> forward (const equidistant_pixel<W, P> &r, const worldpoint<W> &p)
	{
		point<P> out;

		W dx = p.x - r.m_CenterLongitude;
		W dy = p.y - r.m_CenterLatitude;

		dx =  dx * r.m_DotsPerDegreeX;
		dy = -dy * r.m_DotsPerDegreeY;

		W x = dx * r.m_Cos + dy * -r.m_Sin;
		W y = dx * r.m_Sin + dy * r.m_Cos;

		out.x = r.m_OffsetX + (P)x;
		out.y = r.m_OffsetY + (P)y;

		return out;
	}

	//! Translate the coordinates from equidistant pixel to WGS84 coordinates.
	template < typename W, typename P >
	inline worldpoint<W> reverse (const equidistant_pixel<W, P> &r, const point<P> &p)
	{
		worldpoint<W> out;

		P x = p.x - r.m_OffsetX;
		P y = p.y - r.m_OffsetY;

		W dx = (W)x *  r.m_Cos + (W)y * r.m_Sin;
		W dy = (W)x * -r.m_Sin + (W)y * r.m_Cos;

		dx =  dx / r.m_DotsPerDegreeX;
		dy = -dy / r.m_DotsPerDegreeY;

		out.x = dx + r.m_CenterLongitude;
		//if (out.x > (W) 180) out.x -= (W)180*2;
		//if (out.x < (W)-180) out.x += (W)180*2;
		out.y = dy + r.m_CenterLatitude;

		return (out);
	}



	//! Translate coordinates to a mercator scaled pixel.
	template < typename W >
	inline point<int> forward (const mercator_pixel<W, int> &r, const worldpoint<W> &p)
	{
		point<int> out;

		W dx = radian(p.x);
		dx = dx - r.m_LongitudeMercator;

		double ry = radian(p.y);
		W dy = (W)( ( log( tan(ry) + 1.0/cos(ry) ) ) - r.m_LatitudeMercator );

		dx =  dx * r.m_DotsPerRadian;
		dy = -dy * r.m_DotsPerRadian;

		W x = dx * r.m_Cos + dy * -r.m_Sin;
		W y = dx * r.m_Sin + dy * r.m_Cos;

		out.x = r.m_OffsetX + (int) ( (x>=0)?(x+0.5):(x-0.5) );
		out.y = r.m_OffsetY + (int) ( (y>=0)?(y+0.5):(y-0.5) );

		return out;
	}

	//! Translate coordinates to a mercator scaled pixel.
	template < typename W, typename P >
	inline point<P> forward (const mercator_pixel<W, P> &r, const worldpoint<W> &p)
	{
		point<P> out;

		W dx = radian(p.x);
		dx = dx - r.m_LongitudeMercator;

		double ry = radian(p.y);
		W dy = (W)( log( tan(ry) + 1.0/cos(ry) ) ) - (W)r.m_LatitudeMercator;

		dx =  dx * r.m_DotsPerRadian;
		dy = -dy * r.m_DotsPerRadian;

		W x = dx * r.m_Cos + dy * -r.m_Sin;
		W y = dx * r.m_Sin + dy * r.m_Cos;

		out.x = r.m_OffsetX + (P)x;
		out.y = r.m_OffsetY + (P)y;

		return out;
	}

	//! Translate mercator scaled pixel to coordinates.
	template < typename W, typename P >
	inline worldpoint<W> reverse (const mercator_pixel<W, P> &r, const point<P> &p)
	{
		worldpoint<W> out;

		P x = p.x - r.m_OffsetX;
		P y = p.y - r.m_OffsetY;

		W dx = (W)x *  r.m_Cos + (W)y * r.m_Sin;
		W dy = (W)x * -r.m_Sin + (W)y * r.m_Cos;

		dx =  dx / (W)r.m_DotsPerRadian;
		dy = -dy / (W)r.m_DotsPerRadian;

		out.x = (W)degree(dx + r.m_LongitudeMercator);
		//if (out.x > (W) 180) out.x -= (W)180*2;
		//if (out.x < (W)-180) out.x += (W)180*2;
		out.y = (W)degree( atan( sinh(dy + r.m_LatitudeMercator) ) );

		return (out);
	}

	//! Translate WGS84 degree coordinate to a mercator radian coordinate.
	template < typename W >
	inline void forward (const mercator<W> &m, W& x, W& y)
	{
		x = radian(x);

        if( y > 85 )
        {
            y = W(85); //W(pi/2);
        }
        else if( y < -85 )
        {
            y = W(-85); // W(-pi/2); 
        }
        
        {
            const W rlat = radian(y);
            const W sin_ = sin(rlat);
            const W expr = (1+sin_)/(1-sin_);
            y = (W)( log( expr ) ) / 2;
        }
	}

	//! Translate mercator radian coordinate to WGS84 degree coordinate.
	template < typename W >
	inline void reverse (const mercator<W> &m, W& x, W& y)
	{
		x = degree(x);
		y = degree<W>(atan(sinh(y)));
	}

	//! Translate WGS84 degree coordinate to a mercator radian coordinate.
	template < typename W >
	inline void forward (const mercator<W> &m, worldpoint<W> &p)
	{
        forward<W>( m, p.x, p.y );
	}

	//! Translate mercator radian coordinate to WGS84 degree coordinate.
	template < typename W >
	inline void reverse (const mercator<W> &m, worldpoint<W> &p)
	{
        reverse<W>( m, p.x, p.y );
	}

	//! Translate WGS84 degree coordinate to a mercator radian coordinate.
	template < typename W >
	inline void forward (const mercator<W> &m, worldrect<W> &r)
	{
        forward<W>( m, r.left, r.top );
        forward<W>( m, r.right, r.bottom );
	}

	//! Translate mercator radian coordinate to WGS84 degree coordinate.
	template < typename W >
	inline void reverse (const mercator<W> &m, worldrect<W> &r)
	{
        reverse<W>( m, r.left, r.top );
        reverse<W>( m, r.right, r.bottom );
	}
}


#endif // GEOMATH_H_ONCE
/*! @} */
