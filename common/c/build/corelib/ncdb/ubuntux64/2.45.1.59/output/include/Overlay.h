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

@file     Overlay.h
@date     03/05/2009
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
#ifndef OVERLAY_H_ONCE
#define OVERLAY_H_ONCE


#include "NcdbTypes.h"
#include "AutoArray.h"
#include "AutoSharedPtr.h"

namespace Ncdb {

class RasterControl;

//! Map overlay for user defined features.
class NCDB_API Overlay
{
public:
	//! Constructor
	Overlay(void);
	//! Copy constructor
	Overlay(const Overlay& other);
	//! Copy operator
	void operator = (const Overlay& other);
	//! Destructor
	virtual ~Overlay(void);
	//! Update the the data needed for this overlay.
	virtual void update(RasterControl* control) = 0;
	//! Draw the overlay.
	virtual void draw(const WorldRect& wpRect, const PixelRect& pxRect) = 0;

protected:
	RasterControl* m_Control;

};


class NCDB_API OverlayList : public AutoArray<AutoSharedPtr<Overlay> >
{
public:
	OverlayList(void){}
	virtual ~OverlayList(void){}
};

};
#endif // OVERLAY_H_ONCE
/*! @} */
