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

@file     WorldRect.h
@date     04/21/2009
@defgroup MOBIUS_UTILS  Mobius utility classes
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
#ifndef WORLDRECT_H_ONCE
#define WORLDRECT_H_ONCE

#include <math.h>


namespace Ncdb {

struct  WorldPoint;

//! @brief Single precision floating point coordinate rectangle.
//! @note The WorldRect class has no constructor, destructor,
//! or initial value.  Initialize using set or rect = {0,0,0,0}.
struct  WorldRect
{
	//! Orginize so it's correct.
	void sanctify(void);
	//! Set the rectangle.
	void set (float left, float top, float right, float bottom);
	//! Set the rectangle.
	void set (const WorldPoint& p1, const WorldPoint& p2);
	//! Set the rectangle.
	void setMin(float x1, float y1, float x2, float y2);
	//! Set the rectangle.
	void setMin(const WorldPoint& p1, const WorldPoint& p2);
    //! Get the rectangle.
    void get (float& left, float& top, float& right, float& bottom) const;
	//! Clip the rectangle to another rectangle.
	void clip (const WorldRect& r);
    //! Return center of the rectangle
    WorldPoint center() const;
	//! Return the width
	float width(void) const;
	//! Return the height
	float height(void) const;
    //! Expand the rectangle by the offsets.
	void expand (float dx, float dy);
	//! Expand the rectangle by the offsets.
	void expand (const WorldPoint& p);
	//! Expand the rectangle around point p.
    //  size of rectangle is >= then previous size
    //  point p is center of rectangle
	void expandAround (const WorldPoint& p);
	//! Expand the rectangle to inclose point.
	void expandTo (const WorldPoint& p);
	//! Test to see if the point is within the rectangle.
	bool within (const float x, const float y) const;
	//! Test to see if the point is within the rectangle.
	bool within (const WorldPoint & wp) const;
	//! Test to see if one rectangle contains another.
	bool contains(float left, float top, float right, float bottom) const;
	//! Test to see if one rectangle contains another.
	bool contains(const WorldRect& r) const;
	//! Test to see if one rectangle overlaps another.
	bool overlaps(float left, float top, float right, float bottom) const;
    //! Test to see if the rectangle overlaps another.
    bool overlaps(const WorldRect& another) const;
    //! Test to see if segment [x1,y1]:[x2,y2] intersects the rectangle
    bool intersects(float x1, float y1, float x2, float y2) const;
	//! Return the coordinates of the top left corner of the rectangle.
	WorldPoint topLeft(void) const;
	//! Return the coordinates of the bottom right corner of the rectangle.
	WorldPoint bottomRight(void) const;
	//! Expands the rectangle to the bigger of the two.
	void maximize(const WorldRect& rect);
	//! Contracts the rectangle to the smaller of the two.
	void minimize(const WorldRect& rect);
	//! Checks if left and right are within the world boundry.
	void bind(void);
	//! Range check x value of left or right.
	void bind_x(float& x) const;
	//! Subtract x across 180 -180 threshold.
	float subtract_x(float x1, float x2) const;

	//! Move the rectangle by the pixel 
	void operator += (const WorldPoint& p);
	void operator -= (const WorldPoint& p);

	//! Left most longitude
	float m_Left;
	//! Top most latitude
	float m_Top;
	//! Right most longitude
	float m_Right;
	//! Bottom most latitude
	float m_Bottom;
};

inline void WorldRect::bind_x(float&) const
{
	//if (x > 180) x -= 360;
	//if (x < -180) x += 360;
}

inline float WorldRect::subtract_x(float x1, float x2) const
{
	float t = x1 - x2;
	if (t > 360) t -= 360;
	if (t < -360) t += 360;
	return t;
}

inline void WorldRect::maximize(const WorldRect& rect)
{
	float t;
	t = subtract_x(m_Left, rect.m_Left);
	if (t > 0) m_Left = rect.m_Left;
	t = subtract_x(m_Right, rect.m_Right);
	if (t < 0) m_Right = rect.m_Right;
	if (m_Top < rect.m_Top) m_Top = rect.m_Top;
	if (m_Bottom > rect.m_Bottom) m_Bottom = rect.m_Bottom;
}

inline void WorldRect::minimize(const WorldRect& rect)
{
	float t;
	t = subtract_x(m_Left, rect.m_Left);
	if (t < 0) m_Left = rect.m_Left;
	t = subtract_x(m_Right, rect.m_Right);
	if (t > 0) m_Right = rect.m_Right;
	if (m_Top > rect.m_Top) m_Top = rect.m_Top;
	if (m_Bottom < rect.m_Bottom) m_Bottom = rect.m_Bottom;
}

inline WorldPoint WorldRect::topLeft(void) const
{
	WorldPoint p;
	p.x = m_Left;
	p.y = m_Top;
	return p;
}

inline WorldPoint WorldRect::bottomRight(void) const
{
	WorldPoint p;
	p.x = m_Right;
	p.y = m_Bottom;
	return p;
}

inline void WorldRect::bind(void)
{
	/*if (m_Left > 180) m_Left -= 360;
	if (m_Left < -180) m_Left += 360;
	if (m_Right > 180) m_Right -= 360;
	if (m_Right < -180) m_Right += 360;*/
}

inline void WorldRect::sanctify(void)
{
	bind();
	if (m_Bottom > m_Top)
	{
		float t;
		t = m_Top;
		m_Top = m_Bottom;
		m_Bottom = t;
	}
}

inline void WorldRect::set(float left, float top, float right, float bottom)
{
	m_Left = left;
	m_Top = top;
	m_Right = right;
	m_Bottom = bottom;
}

inline void WorldRect::set(const WorldPoint& p1, const WorldPoint& p2)
{
	m_Left   = p1.x;
	m_Top    = p1.y;
	m_Right  = p2.x;
	m_Bottom = p2.y;
}

inline void WorldRect::setMin(float x1, float y1, float x2, float y2)
{
    if( x2 > x1 )
    {
    	m_Left  = x1;
	    m_Right = x2;
    }
    else
    {
    	m_Left  = x2;
	    m_Right = x1;
    }

    if( y2 > y1 )
    {
	    m_Top    = y1;
	    m_Bottom = y2;
    }
    else
    {
	    m_Top    = y2;
	    m_Bottom = y1;
    }

    bind();
}

inline void WorldRect::setMin(const WorldPoint& p1, const WorldPoint& p2)
{
    setMin( p1.x, p1.y, p2.x, p2.y );
}

inline void WorldRect::get(float& left, float& top, float& right, float& bottom) const
{
	left = m_Left;
	top = m_Top;
	right = m_Right;
	bottom = m_Bottom;
}

inline void WorldRect::clip(const WorldRect& r)
{
	float t;
	t = subtract_x (m_Left, r.m_Left);
	if (t < 0) m_Left = r.m_Left;
	if (m_Top < r.m_Top) m_Top = r.m_Top;
	t = subtract_x (m_Right, r.m_Right);
	if (t > 0) m_Right = r.m_Right;
	if (m_Bottom > r.m_Bottom) m_Bottom = r.m_Bottom;
}

inline WorldPoint WorldRect::center() const
{
    WorldPoint center;

	float t = subtract_x(m_Right, m_Left);
    center.x = m_Left + t / 2;
	bind_x(center.x);
    center.y = ( m_Bottom + m_Top ) / 2;

    return center;
}

inline void WorldRect::expand(float dx, float dy)
{
	m_Left   -= dx;
	m_Top    += dy;
	m_Right  += dx;
	m_Bottom -= dy;
	bind();
}

inline void WorldRect::expand(const WorldPoint& p)
{
    expand( p.x, p.y );
}

inline void WorldRect::expandAround (const WorldPoint& p)
{
    const float dx1 = (float)fabs(subtract_x(p.x, m_Left));
    const float dx2 = (float)fabs(subtract_x(p.x, m_Right));
    const float dy1 = (float)fabs(p.y - m_Top);
    const float dy2 = (float)fabs(p.y - m_Bottom);

    const float dx = dx1 > dx2 ? dx1 : dx2;
    const float dy = dy1 > dy2 ? dy1 : dy2;

    m_Left   = p.x - dx;
    m_Right  = p.x + dx;
    m_Top    = p.y - dy;
    m_Bottom = p.y + dy;
	bind();
}

inline void WorldRect::expandTo (const WorldPoint& p)
{
    if (p.x < m_Left)
    {
        m_Left = p.x;
    }
    else if (p.x > m_Right)
    {
        m_Right = p.x;
    }
    if (p.y < m_Top)
    {
        m_Top = p.y;
    }
    else if (p.y > m_Bottom)
    {
        m_Bottom = p.y;
    }
	bind();
}

inline void WorldRect::operator += (const WorldPoint& p)
{
	m_Left      += p.x;
	m_Top       += p.y;
	m_Right     += p.x;
	m_Bottom    += p.y;
	bind();
}

inline void WorldRect::operator -= (const WorldPoint& p)
{
	m_Left      -= p.x;
	m_Top       -= p.y;
	m_Right     -= p.x;
	m_Bottom    -= p.y;
	bind();
}

inline bool WorldRect::within(const float x, const float y) const
{
	if( y <= m_Top )
	{
		if( y >= m_Bottom )
		{
			float tl, tr, tx;
			tl = m_Left;
			tr = m_Right;
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

inline bool WorldRect::within(const WorldPoint &wp) const
{
    return within(wp.x, wp.y);
}

inline bool WorldRect::contains(float left, float top, float right, float bottom) const
{
	float t1, t2;
	bind_x(left);
	t1 = subtract_x(left, m_Left);
	bind_x(right);
	t2 = subtract_x(right, m_Right);
	return ( top <= m_Top &&
			 bottom >= m_Bottom &&
			 t1 >= 0 &&
			 t2 <= 0 );
}

inline bool WorldRect::contains(const WorldRect& r) const
{
	return contains( r.m_Left, r.m_Top, r.m_Right, r.m_Bottom );
}

inline bool WorldRect::overlaps(float left, float top, float right, float bottom) const
{

	bool overlapsOnY = false;

	float y = top - m_Top;
	if( y > 0 )
	{
		y = m_Top - bottom;
		if( y >= 0 )
		{
			overlapsOnY = true;
		}
	}
	else
	{
		y = top - m_Bottom;
		if( y >= 0 )
		{
			overlapsOnY = true;
		}
	}

	bool overlapsOnX = false;

	if( overlapsOnY )
	{
		float tl, tr, c1, c2, w1, w2, f1, f2;
		tl = m_Left;
		tr = m_Right;
		if (tl > tr)
		{
			tr += 360;
		}
		if (left > right)
		{
			right += 360;
		}
		if (right < tl)
		{
			left += 360;
			right += 360;
		}

		c1 = (tr + tl) / 2;
		w1 = tr - tl;

		c2 = (right + left) / 2;
		w2 = right - left;

		f1 = (w2 + w1)/2;
		f2 = (c2 > c1) ? (c2 - c1) : (c1 - c2);
		if (f2 <= f1)
		{
			overlapsOnX = true;
		}
		else if (f1 > 180)
		{
			overlapsOnX = true;
		}
		else
		{
			overlapsOnX = false;
		}
	}
	return overlapsOnY && overlapsOnX;
}

inline bool WorldRect::overlaps(const WorldRect& another) const
{
    return overlaps( another.m_Left, another.m_Top, another.m_Right, another.m_Bottom );
}

inline bool WorldRect::intersects(float x1, float y1, float x2, float y2) const
{
	float t;
	t = subtract_x(x1, x2);
    float left      = ( t < 0 ) ? x1 : x2;
    float top       = ( y1 > y2 ) ? y1 : y2;
    float right     = ( t > 0 ) ? x1 : x2;
    float bottom    = ( y1 < y2 ) ? y1 : y2;

    return overlaps( left, top, right, bottom );
}

inline float WorldRect::width(void) const
{
	float t = m_Right - m_Left;
	if (m_Left > m_Right) t += 360;
	return t;
}

inline float WorldRect::height(void) const
{
	return (m_Bottom - m_Top);
}

static const WorldRect WorldLimit = {-180,90,180,-90};

};
#endif // WORLDRECT_H_ONCE
/*! @} */
