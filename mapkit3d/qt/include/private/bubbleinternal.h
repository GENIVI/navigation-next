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
    @file bubbleinternal.h
    @date 08/21/2014
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
#ifndef __BUBBLE_IMPL_H__
#define __BUBBLE_IMPL_H__
#include "BubbleInterface.h"

namespace locationtoolkit
{
class PinParameters;
class Bubble;

class BubbleInternal : public nbmap::BubbleInterface
{
public:
    BubbleInternal(Bubble* bubble);
    ~BubbleInternal();

public:
    /*! Called when the bubble needs to be displayed.
    @param x screen coordinates x, unit is percentage
    @param y screen coordinates y, unit is percentage
    @param orientation, not used for now
    @return None
    */
    void Show(float x, float y, int orientation);

    /*! Called if the position of the bubble changes.
    @param x  screen coordinates x, unit is percentage
    @param y  screen coordinates y, unit is percentage
    @param orientation, not used for now
    @return None
    */
    void Update(float x, float y, int orientation);

   /*! Called when need to hide the bubble.
    @return None
    */
    void Hide();

    /*! Called when need to release the bubble.
    @return None
    */
    void Remove();

private:
    Bubble* mBubble;
};
}
#endif // __BUBBLE_IMPL_H__
