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
 @file     BubblelistenerInterface.h
 @defgroup nbmap
 
 */
/*
 (C) Copyright 2011 by TeleCommunication Systems, Inc.                
 
 The information contained herein is confidential, proprietary 
 to TeleCommunication Systems, Inc., and considered a trade secret as 
 defined in section 499C of the penal code of the State of     
 California. Use of this information by anyone other than      
 authorized employees of TeleCommunication Systems, is granted only    
 under a written non-disclosure agreement, expressly           
 prescribing the scope and manner of such use.                 
 
 ---------------------------------------------------------------------------*/

/*! @{ */

#ifndef __BUBBLE_LISTENER__
#define __BUBBLE_LISTENER__

#include "smartpointer.h"
#include <string>

namespace nbmap
{

class BubblelistenerInterface
{
public:
    /*! Callback function called when the pin is selected or the selected pin is moved from the outside to the screen

        This function is called in the render thread.

        @return None
    */
    virtual void NotifyShowBubble(shared_ptr<std::string> pinId, float x, float y, bool orientation) = 0;

    /*! Callback function called when the pin is unselected or the selected pin is moved outside the screen

        This function is called in the render thread.

        @return None
    */
    virtual void NotifyHideBubble(shared_ptr<std::string> pinId) = 0;

    /*! Callback function called when position of selected pin is changed

        This function is called in the render thread.

        @return None
    */
    virtual void NotifyUpdateBubble(shared_ptr<std::string> pinId, float x, float y, bool orientation) = 0;

    /*! Callback function called to show location bubble.

        This function is called in the render thread.

      @return None
    */
    virtual void NotifyShowLocationBubble(float x, float y, bool orientation,
                                          float latitude, float longitude) = 0;

    /*! Callback function called to hide location bubble.

        This function is called in the render thread.

      @return None
    */
    virtual void NotifyHideLocationBubble() = 0;


    /*! Callback function called when position of location disk changes

        This function is called in the render thread.

        @return None
    */
    virtual void NotifyUpdateLocationBubble(float x, float y, bool orientation,
                                            float latitude, float longitude) = 0;

    /*! Callback function called when the static poi is selected or the selected static poi is moved from the outside to the screen

        This function is called in the render thread.

        @return None
    */
    virtual void NotifyShowStaticPoiBubble(float x, float y, bool orientation) = 0;

    /*! Callback function called when the static poi is unselected or the selected static poi is moved outside the screen

        This function is called in the render thread.

        @return None
    */
    virtual void NotifyHideStaticPoiBubble() = 0;

    /*! Callback function called when position of selected static poi is changed

        This function is called in the render thread.

        @return None
    */
    virtual void NotifyUpdateStaticPoiBubble(float x, float y, bool orientation) = 0;
};

}; // namespace nbmap

#endif

/*! @} */
