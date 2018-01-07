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

	@file     graphics.h
	@date     09/23/2010

	Common graphics related classes.

*/
/*
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

#ifndef _NB_GRAPHICS_H_
#define _NB_GRAPHICS_H_

namespace nb 
{

/*
	We currently have similar C-structures in NBSC_Defines.h which are used for the C-Interface. 
*/

/*! Simple size class */
class Size
{
public:
	Size();
	Size(int _width, int _height);

	/* See source for description */
	void Set(int _width, int _height);

	// Public members
	int width;
	int height;
};

/*! Simple Point class */
class Point 
{
public:
	Point();
	Point(int _x, int _y);
	
	/* See source for description */
	void Set(int _x, int _y);

	/* See source for description */
	void operator+=(const Point& rightSide);
    bool operator==(const Point& rightSide) const;
    bool operator<(const Point& rightSide) const;
	
	// Public members
	int x;
	int y;
};

/*! Simple rectangle class */
class Rectangle
{
public:
	Rectangle();
	Rectangle(int _x, int _y, int _width, int _height);
	
	/* See source for description */
	void Set(int _x, int _y, int _width, int _height);
	bool IsEmpty() const;
	
	/* See source for description */
	int Right() const;
	int Bottom() const;
	
	/* See source for description */
	int HalfWidth() const;
	int HalfHeight() const;
	
	/* See source for description */
	Point Center() const;
	bool Contains(const Point& point) const;
	
	/* See source for description */
    bool operator==(const Rectangle& rightSide) const;
    
	// Public members
	int x;
	int y;
	int width;
	int height;
};
    
}

/*! @} */

#endif




