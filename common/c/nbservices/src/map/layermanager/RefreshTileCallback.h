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

    @file       RefreshTileCallback.h

    Class RefreshTileCallback inherits from AsyncCallbackWithRequest
    interface. A RefreshTileCallback object is a callback of requesting
    a tile of the layers need to refresh in an interval.
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

#ifndef REFRESHTILECALLBACK_H
#define REFRESHTILECALLBACK_H

/*!
    @addtogroup nbmap
    @{
*/

#include "AsyncCallback.h"
#include "TileKey.h"
#include "Tile.h"
#include "base.h"
#include <vector>

namespace nbmap
{

// Types ........................................................................................

typedef shared_ptr<AsyncCallbackWithRequest<TileKeyPtr, TilePtr> > AsyncTileRequestWithRequestPtr;

class RefreshLayer;

/*! A RefreshTileCallback object is a callback of requesting a tile of the layers need to refresh in an interval. */
class RefreshTileCallback : public AsyncCallbackWithRequest<TileKeyPtr, TilePtr>,
                            public Base
{
public:
    // Public functions .........................................................................

    /*! RefreshTileCallback constructor */
    RefreshTileCallback(AsyncTileRequestWithRequestPtr callback,/*!< A delegated callback */
                        shared_ptr<bool> layerValid,    /*!< The flag identified if the pointer to the refresh layer is valid */
                        RefreshLayer* layer             /*!< A pointer to a refresh layer */
                        );

    /*! RefreshTileCallback destructor */
    virtual ~RefreshTileCallback();

    /* See description in AsyncCallback.h */
    virtual void Success(TileKeyPtr tileKey,    /*!< The requested tile key */
                         TilePtr tile           /*!< A tile to return */
                         );
    virtual void Error(TileKeyPtr tileKey,      /*!< Parameters to request a tile */
                       NB_Error error           /*!< An error to return */
                       );
    virtual bool Progress(int percentage        /*!< A value of percentage */
                          );


private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    RefreshTileCallback(const RefreshTileCallback& callback);
    RefreshTileCallback& operator=(const RefreshTileCallback& callback);


    // Private members ..........................................................................

    AsyncTileRequestWithRequestPtr m_callback;  /*!< A delegated callback */
    shared_ptr<bool> m_layerValid;      /*!< The flag identified if the pointer to the refresh layer is valid */
    RefreshLayer* m_layer;              /*!< A pointer to a refresh layer. Need to check the flag m_layerValid
                                             before using this pointer. */
};

typedef shared_ptr<RefreshTileCallback> RefreshTileCallbackPtr;

};  // namespace nbmap

/*! @} */

#endif  // REFRESHTILECALLBACK_H
