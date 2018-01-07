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

    @file nbgmdevicelocation.h
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
#ifndef _NBGM_DEVICE_LOCATION_H_
#define _NBGM_DEVICE_LOCATION_H_
#include <string>
#include "nbgmtypes.h"
#include "nbgmavatarsetting.h"
#include "nbgmbinarybuffer.h"

/*! \addtogroup NBGM_Manager
*  @{
*/

/*! Avatar instance shown on map view.
*/
class NBGM_DeviceLocation
{
public:
    /// Enable halo
    virtual void EnableHalo(bool showHalo) = 0;

    /// Set avatar location
    virtual void SetLocation(const NBGM_Location64& location) = 0;

    /// Set avatar texture
    virtual PAL_Error SetDirectionalTexture(const std::string& textureFilePath, const NBGM_Point2d& offset) = 0;
    virtual PAL_Error SetDirectionalTexture(const NBGM_BinaryBuffer& textureBuffer, const NBGM_Point2d& offset) = 0;
    virtual PAL_Error SetDirectionlessTexture(const NBGM_BinaryBuffer& textureBuffer, const NBGM_Point2d& offset) = 0;
    virtual PAL_Error SetDirectionlessTexture(const std::string& textureFilePath, const NBGM_Point2d& offset) = 0;

    /// Set halo Style
    virtual PAL_Error SetHaloStyle(const NBGM_Color& edgeColor, const NBGM_Color& fillColor, uint8 edgeWidth, NBGM_HaloEdgeStyle style) = 0;

    /// Set avatar state
    virtual void SetState(NBGM_AvatarState state) = 0;

    /// Get screen position of avatar
    virtual bool GetScreenPosition(float& screenX, float& screenY) const = 0;

    /*! Get if given screen position hit avatar. 
      @param x, screen point x
      @param y, screen point y
      @return is avatar hit
     */
    virtual bool SelectAndTrack(float x, float y) = 0;

    virtual const std::string& ID() const = 0;

protected:
    virtual ~NBGM_DeviceLocation(){}
};

/*! @} */
#endif
