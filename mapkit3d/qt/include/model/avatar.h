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
    @file avatar.h
    @date 08/21/2014
    @addtogroup mapkit3d
*/
/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#ifndef __AVATAR_H__
#define __AVATAR_H__

#include "location.h"

namespace locationtoolkit
{
class Avatar
{
public:
    /*! Avatar mode.
     The avatar has multiple styles of appearance suiting for various scenarios.
     Try and select one mode for your own requirement.
     */
    enum AvatarMode
    {
        AM_NONE,
        AM_MAP,
        AM_ARROW,
        AM_CAR
    };
    Avatar(){}
    virtual ~Avatar(){}
public:
    /*!
     Sets the location of the avatar.
     @param avatarLocation The location of the avatar.
     @return None
     */
    virtual void SetLocation(Location avatarLocation) = 0;

    /*!
     Gets the location of the avatar.
     @return Location
     */
    virtual Location GetLocation() = 0;

    /*!
     Sets the mode of the avatar.
     @param AvatarMode. Default mode is AVATAR_MODE_MAP.
     @return None
     */
    virtual void SetMode(AvatarMode mode) = 0;

    /*! Gets the mode of the avatar.
     @return AvatarMode
     */
    virtual AvatarMode GetMode() = 0;

    /*!
     Sets the scale value of the avatar.
     @param scale The scale value. Default scale value is 1.0.
     @return None
     */
    virtual void SetScale(float scale) = 0;

    /*!
     Gets the scale value of the avatar.
     @return The scale value.
     */
    virtual float GetScale()= 0;
};
}
#endif // __AVATAR_H__

/*! @} */
