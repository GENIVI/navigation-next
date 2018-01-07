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

@file     PixelRect.h
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
#ifndef PIXELRECT_H_ONCE
#define PIXELRECT_H_ONCE


#include  "AutoArray.h"
namespace Ncdb {

struct PixelPoint;

//! @brief Signed integer fixed point pixel rectangle
//! @note The PixelPoint class has no constructor, destructor,
//! or initial value.  Initialize using set or rect = {0,0,0,0}.
struct PixelRect
{
public:
	//! Set the rectangle individual values.
	void set (int x1, int y1, int x2, int y2);
	//! Get the rectangle individual values.
	void get (int& x1, int& y1, int& x2, int& y2);
	//! Clip the rectangle to another rectangle.
	void clip (const PixelRect& r);
	//! Move the rectangle by the PixelPoint.
	void operator += (const PixelPoint& p);
	//! Move the rectangle by the PixelPoint.
	void operator -= (const PixelPoint& p);
	//! Move the rectangle by the PixelPoint.
	void operator += (const int& i);
	//! Move the rectangle by the PixelPoint.
	void operator -= (const int& i);
	//! Compare two rectangles.
	bool operator == (const PixelRect& p);
	//! Compare two rectangles.
	bool operator != (const PixelRect& p);
	//! Get the width.
	int width(void) const;
	//! Get the height.
	int height(void) const;
	//! Test to see if the point is within the rectangle.
	bool within (const int x, const int y) const;
	//! Test to see if the rectangle is within the rectangle.
	bool within(const PixelRect& r) const;
	bool overlaps(int ileft, int itop, int iright, int ibottom) const;
	bool overlaps(const PixelRect& another) const;
    //! Inflate rect sizes by dx and dy (Height += 2*dy, Width += 2*dx)
    void inflate( int dx, int dy );
    //! Move the rectangle by the deltas.
    void move( int dx, int dy );
	//! Get the rectangle center.
	void center (PixelPoint& p) const;
public:
	//! Top most y coordinate
	int top;
	//! Left most x coordinate
	int left;
	//! Bottom most y coordinate
	int bottom;
	//! Right most x coordinate
	int right;
};

inline void PixelRect::center (PixelPoint& p) const
{
	p.x = (left + right) / 2;
	p.y = (top + bottom) / 2;

    // same as floor
    if (p.x * 2 > left + right)
        p.x--;
    if (p.y * 2 > top + bottom)
        p.y--;
}

inline void PixelRect::set(int x1, int y1, int x2, int y2)
{
	left = x1;
	top = y1;
	right = x2;
	bottom = y2;
}

inline void PixelRect::get(int& x1, int& y1, int& x2, int& y2)
{
	x1 = left;
	y1 = top;
	x2 = right;
	y2 = bottom;
}

inline void PixelRect::clip(const PixelRect& r)
{
	if (left < r.left) left = r.left;
	if (right > r.right) right = r.right;
	if (top < r.top) top = r.top;
	if (bottom > r.bottom) bottom = r.bottom;
}

inline void PixelRect::operator += (const PixelPoint& p)
{
	left += p.x;
	right += p.x;
	top += p.y;
	bottom += p.y;
}

inline void PixelRect::operator -= (const PixelPoint& p)
{
	left -= p.x;
	right -= p.x;
	top -= p.y;
	bottom -= p.y;
}

inline void PixelRect::operator += (const int& i)
{
	left -= i;
	right += i;
	top -= i;
	bottom += i;
}

inline void PixelRect::operator -= (const int& i)
{
	*this += -i;
}

inline bool PixelRect::operator == (const PixelRect& r)
{
	return ( left == r.left && right == r.right && top == r.top && bottom == r.bottom );
}

inline bool PixelRect::operator != (const PixelRect& r)
{
	return ( left != r.left || right != r.right || top != r.top || bottom != r.bottom );
}

inline int PixelRect::width(void) const
{
	return (right - left);
}

inline int PixelRect::height(void) const
{
	return (bottom - top);
}

inline bool PixelRect::within(const int x, const int y) const
{
	return ( y >= top &&
	         y <= bottom &&
	         x >= left &&
	         x <= right );
}

inline bool PixelRect::within(const PixelRect& r) const
{
	return ( within(r.left, r.top) && within(r.right, r.bottom) );
}

inline bool PixelRect::overlaps(int ileft, int itop, int iright, int ibottom) const
{
    assert( ileft <= iright );
    assert( itop <= ibottom );

    bool overlapsOnY = false;

    if( itop < top )
    {
        if( top <= ibottom )
        {
            overlapsOnY = true;
        }
    }
    else
    {
        if( itop <= bottom )
        {
            overlapsOnY = true;
        }
    }

    bool overlapsOnX = false;

    if( overlapsOnY )
    {
        if( ileft < left )
        {
            if( left <= iright )
            {
                overlapsOnX = true;
            }
        }
        else
        {
            if( ileft <= right )
            {
                overlapsOnX = true;
            }
        }
    }

    return overlapsOnY && overlapsOnX;
}

inline bool PixelRect::overlaps(const PixelRect& another) const
{
    return overlaps( another.left, another.top, another.right, another.bottom );
}

inline void PixelRect::inflate( int dx, int dy )
{
    left   -= dx;
    right  += dx;
    top    -= dy;
    bottom += dy;
}

inline void PixelRect::move( int dx, int dy )
{
    left   += dx;
    right  += dx;
    top    += dy;
    bottom += dy;
}
#ifdef _MSC_VER
template class NCDB_API  AutoArray<PixelRect>; 
#endif
};

#endif // PIXELRECT_H_ONCE
/*! @} */
