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

@file     RouteGuide.h
@date     09/17/2012
@defgroup MOBIUS_UTILS  Mobius utility classes
@author   Darrell Mathis



*/
/*
(C) Copyright 2012 by Networks In Motion, Inc.                

The information contained herein is confidential, proprietary 
to Networks In Motion, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of Networks In Motion is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */
#pragma once
#ifndef ROUTEGUIDE_H_ONCE
#define ROUTEGUIDE_H_ONCE

#include "WorldPoint.h"
#include "WorldRect.h"

namespace Ncdb {

//! @brief Guided connection between rectangle regions.
struct  RouteGuide
{
    RouteGuide ()
    {
        m_OrigRect.set (0.0f, 0.0f, 0.0f, 0.0f);
        m_DestRect.set (0.0f, 0.0f, 0.0f, 0.0f);
    }
	//! Set the route guide.
	void set (WorldRect & origRect, WorldRect & destRect, WorldPointList & chain);
    //! Get the rectangle.
    void get (WorldRect & origRect, WorldRect & destRect, WorldPointList & chain) const;
	//! Return the orig rectangle.
	WorldRect getOrigRect(void) const;
	//! Return the dest rectangle.
	WorldRect getDestRect(void) const;
	//! Return the chain betweem orig and dest.
	WorldPointList getChain(void) const;

	//! Orig rectangle
	WorldRect m_OrigRect;
	//! Dest rectangle
	WorldRect m_DestRect;
	//! Chain of via points
	WorldPointList m_Chain;
};

inline void RouteGuide::set(WorldRect & origRect, WorldRect & destRect, WorldPointList & chain)
{
	m_OrigRect = origRect;
	m_DestRect = destRect;
	m_Chain = chain;
}

inline void RouteGuide::get(WorldRect & origRect, WorldRect & destRect, WorldPointList & chain) const
{
	origRect = m_OrigRect;
	destRect = m_DestRect;
	chain = m_Chain;
}

inline WorldRect RouteGuide::getOrigRect() const
{
	return m_OrigRect;
}

inline WorldRect RouteGuide::getDestRect() const
{
	return m_DestRect;
}

inline WorldPointList RouteGuide::getChain() const
{
	return m_Chain;
}

typedef AutoArray<RouteGuide> RouteGuideList;
#ifdef _MSC_VER
template class NCDB_API  AutoArray<RouteGuide>;         // export declaration for DLL
template class NCDB_API  AutoArray<RouteGuideList>;     // export declaration for DLL
#endif

};
#endif // ROUTEGUIDE_H_ONCE
/*! @} */
