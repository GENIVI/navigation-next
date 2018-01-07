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

	@file     graphics.cpp
	@date     09/23/2010
*/
/*
	See description in header file.

	(C) Copyright 2010 by TeleCommunication Systems                

	The information contained herein is confidential, proprietary 
	to Networks In Motion, Inc., and considered a trade secret as 
	defined in section 499C of the penal code of the State of     
	California. Use of this information by anyone other than      
	authorized employees of Networks In Motion is granted only    
	under a written non-disclosure agreement, expressly           
	prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "graphics.h"

using namespace nb;


// Size class ........................................................................................................

Size::Size() 
	: width(0),height(0)
{
};

/*! Overload constructor */
Size::Size(int _width, int _height)
	: width(_width), height(_height) 
{
};

/*! Simple set function */
void 
Size::Set(int _width, int _height)
{
	width = _width;
	height = _height;
};

	
// Point class .......................................................................................................

Point::Point()
	: x(0), y(0)
{
};
	
/*! Overload constructor */
Point::Point(int _x, int _y)
	: x(_x), y(_y)
{
};

/*! Simple set function */	
void 
Point::Set(int _x, int _y)
{
	x = _x;
	y = _y;
};

/*! Addition operator */
void 
Point::operator+=(const Point& rightSide)
{
	x += rightSide.x;
	y += rightSide.y;
}

/*! Compare operator */
bool
Point::operator==(const Point& rightSide) const
{
    return ((x == rightSide.x) && (y == rightSide.y));
}

/*! The less-operator is used so that we can put a Point into a std::set */
bool 
Point::operator<(const Point& rightSide) const
{
    if (x < rightSide.x)
    {
        return true;
    }
    if (x > rightSide.x)
    {
        return false;
    }
    if (y < rightSide.y)
    {
        return true;
    }
    if (y > rightSide.y)
    {
        return false;
    }
    
	// Now they are considered equal
	return false;
}

	
// Rectangle class ...................................................................................................

Rectangle::Rectangle()
	: x(0), y(0), width(0), height(0)
{
};

/*! Overload constructor */
Rectangle::Rectangle(int _x, int _y, int _width, int _height)
	: x(_x), y(_y), width(_width), height(_height)
{
};

/*! Simple set function */
void 
Rectangle::Set(int _x, int _y, int _width, int _height)
{
	x = _x;
	y = _y;
	width = _width;
	height = _height;
};

/*! Returns 'true' if rectangle is empty */
bool 
Rectangle::IsEmpty() const
{
	return ((x == 0) && (y == 0) && (width == 0) && (height == 0));
}

/*! Return right side of rectangle */
int 
Rectangle::Right() const
{
	return (x + width - 1);
}

/*! Return bottom side of rectangle */
int 
Rectangle::Bottom() const
{
	return (y + height - 1);
}

/*! Get half width */
int 
Rectangle::HalfWidth() const
{
	return width / 2;
}

/*! Get half height */
int 
Rectangle::HalfHeight() const
{
	return height / 2;
}

/*! Return the center point */	
Point 
Rectangle::Center() const
{
	return Point(x + HalfWidth(), y + HalfHeight());
}

/*! Compare operator */
bool 
Rectangle::operator==(const Rectangle& rightSide) const
{
    if ((x == rightSide.x) && 
        (y == rightSide.y) &&
        (width == rightSide.width) &&
        (height == rightSide.height))
    {
        return true;
    }
    return false;
}

/*! Returns 'true' if the given point is inside the rectangle */
bool 
Rectangle::Contains(const Point& point) const
{
	if ((point.x >= x) &&
		(point.x <= Right()) &&
		(point.y >= y) &&
		(point.y <= Bottom()))
	{
		return true;
	}
	return false;
}


/*! @} */





