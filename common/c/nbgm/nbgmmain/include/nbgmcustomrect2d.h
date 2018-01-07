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

    @file nbgmcustomrect2d.h
*/
/*
    (C) Copyright 2014 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBGM_CUSTOM_RECT2_H_
#define _NBGM_CUSTOM_RECT2_H_

#include "nbgmtypes.h"

/*! \addtogroup NBGM_Manager
*  @{
*/

/*! Rect2d instance shown on map view.
*/
class NBGM_CustomRect2d
{
public:
    /// Set Rect2D center and heading angle
    virtual void Update(double mercatorX, double mercatorY, float heading) = 0;

    /// Set Rect2d size
    virtual void SetSize(float width, float height) = 0;

    /// Set Rect2d visible
    virtual void SetVisible(bool visible) = 0;

    /// Set Rect2d texture
    virtual void SetTexture(const NBGM_BinaryBuffer& buffer) = 0;

    /// Set Rect2d texture
    virtual void SetTexture(NBGM_TextureId textureId) = 0;

    /// Get Rect2d id
    virtual NBGM_Rect2dId ID() const = 0;

protected:
    virtual ~NBGM_CustomRect2d(){}
};

/*! @} */
#endif
