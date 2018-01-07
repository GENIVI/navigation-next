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

@file     PixelPoint.h
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
#ifndef PIXELPOINT_H_ONCE
#define PIXELPOINT_H_ONCE



namespace Ncdb {


//! @brief Signed integer fixed point pixel location.
struct NCDB_API PixelPoint
{
	//! Horizontal distance.
	int x;
	//! Vertical distance.
	int y;

    PixelPoint()    // default constructor
        {  }

    PixelPoint(int px, int py)      // (x,y) constructor
        : x(px), y(py)
        { }

	//! Equal operator.
	bool operator == (const PixelPoint& p) const
	{
		return (x == p.x && y == p.y);
	}
	//! Not equal operator.
	bool operator != (const PixelPoint& p) const
	{
		return (x != p.x || y != p.y);
	}
};

//! PixelPointList class is an array containing a list of pixel points.
#ifdef _MSC_VER
template class NCDB_API  AutoArray<PixelPoint>; // export declaration for DLL
#endif
typedef AutoArray<PixelPoint>   PixelPointList;


};
#endif // PIXELPOINT_H_ONCE
/*! @} */
