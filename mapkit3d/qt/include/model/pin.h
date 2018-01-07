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
    @file pin.h
    @date 08/22/2014
    @defgroup mapkit3d
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
 *
 */
#ifndef __PIN_H__
#define __PIN_H__

#include "coordinate.h"
namespace locationtoolkit
{
class Pin
{
public:
    virtual ~Pin(){}

    /*! Gets the position.
      @return The geographic position of the pin.
     */
    virtual const Coordinates& GetPosition() = 0;

    /*! Sets the position.
     @deprecated This method has not been supported yet.
     @param position The geographic position of the pin.
     @return None.
     */
    virtual void SetPosition(Coordinates& position) = 0;

    /*! Checks weather the pin is visible.
     @deprecated This method has not been supported yet.
     @return  True if the pin is visible, false otherwise.
     */
    virtual bool IsVisible() = 0;

    /*! Sets the visibility.
     @deprecated This method has not been supported yet.
     @param visible True to make the pin visible;false to make the pin invisible.
     */
    virtual void SetVisible(bool visible) = 0;

    /*! Selects the pin.
     @param selected True to make the pin selected;false to make the pin unselected.
     @return None.
     */
    virtual void SetSelected(bool selected) = 0;
};
}
#endif // __PIN_H__
