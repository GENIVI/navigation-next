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

    @file       AnimationTileCallback.h

    Class AnimationTileCallback inherits from AsyncCallbackWithRequest
    interface. An AnimationTileCallback object is a callback of
    requesting the animation tiles by the AnimationLayer.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef ANIMATIONTILECALLBACK_H
#define ANIMATIONTILECALLBACK_H

/*!
    @addtogroup nbmap
    @{
*/

#include "AsyncCallback.h"
#include "base.h"
#include "smartpointer.h"
#include "Tile.h"
#include "TileKey.h"
#include <map>
#include <string>

namespace nbmap
{
// Types ........................................................................................

class AnimationLayer;

/*! An AnimationTileCallback object is a callback of requesting the animation tiles by the AnimationLayer */
class AnimationTileCallback : public Base,
                              public AsyncCallbackWithRequest<shared_ptr<std::map<std::string, std::string> >, TilePtr>
{
public:
    // Public functions .........................................................................

    /*! AnimationTileCallback constructor */
    AnimationTileCallback(AnimationLayer* animationLayer,                                       /*!< Animation layer to return the
                                                                                                     animation tiles */
                          shared_ptr<bool> animationLayerValid,                                 /*!< Flag identified if the AnimationLayer
                                                                                                     object is valid. It is set to false
                                                                                                     in the destructor of class
                                                                                                     AnimationLayer. */
                          TileKeyPtr tileKey,                                                   /*!< Tile key of the request */
                          shared_ptr<AsyncCallback<TilePtr> > callback,  /*!< Callback of the request */
                          uint32 timeStamp                                                      /*!< time stamp of requested tile. */
                          );

    /*! AnimationTileCallback destructor */
    virtual ~AnimationTileCallback();

    /* See description in AsyncCallback.h */
    virtual void Success(shared_ptr<std::map<std::string, std::string> > templateParameters,/*!< Parameters to request the animation tile */
                         TilePtr tile                                                       /*!< A Tile object to return */
                         );
    virtual void Error(shared_ptr<std::map<std::string, std::string> > templateParameters,  /*!< Parameters to request the animation tile */
                       NB_Error error                                                       /*!< An error to return */
                       );
    virtual bool Progress(int percentage    /*!< A value of percentage */
                         );


private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    AnimationTileCallback(const AnimationTileCallback& callback);
    AnimationTileCallback& operator=(const AnimationTileCallback& callback);


    // Private members ..........................................................................

    AnimationLayer* m_animationLayer;        /*!< Animation layer to return the animation tiles */
    shared_ptr<bool> m_animationLayerValid;  /*!< Flag identified if the AnimationLayer
                                                  object is valid. It is set to false in the
                                                  destructor of class AnimationLayer. */
    TileKeyPtr m_tileKey;                    /*!< Tile key of the request */
    uint32     m_timeStamp;                  /*!< TimeStamp of requested tile */

    shared_ptr<AsyncCallback<TilePtr> > m_callback;  /*!< Callback of the request */
};

};  // namespace nbmap

/*! @} */

#endif  // ANIMATIONTILECALLBACK_H
