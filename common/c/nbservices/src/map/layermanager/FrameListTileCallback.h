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

    @file       FrameListTileCallback.h

    Class FrameListTileCallback inherits from AsyncCallbackWithRequest
    interface. A FrameListTileCallback object is a callback of
    requesting the frame list by the FrameListLayer.
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

#ifndef FRAMELISTTILECALLBACK_H
#define FRAMELISTTILECALLBACK_H

/*!
    @addtogroup nbmap
    @{
*/

#include "AsyncCallback.h"
#include "base.h"
#include "smartpointer.h"
#include "Tile.h"
#include <map>
#include <string>

namespace nbmap
{
// Types ........................................................................................

class FrameListLayer;

/*! A FrameListTileCallback object is a callback of requesting the frame list by the FrameListLayer */
class FrameListTileCallback : public Base,
                              public AsyncCallbackWithRequest<shared_ptr<std::map<std::string, std::string> >, TilePtr>
{
public:
    // Public functions .........................................................................

    /*! FrameListTileCallback constructor */
    FrameListTileCallback();

    /*! FrameListTileCallback destructor */
    virtual ~FrameListTileCallback();

    /* See description in AsyncCallback.h */
    virtual void Success(shared_ptr<std::map<std::string, std::string> > templateParameters,/*!< Parameters to request the frame list */
                         TilePtr tile                                                       /*!< A Tile object to return */
                         );
    virtual void Error(shared_ptr<std::map<std::string, std::string> > templateParameters,  /*!< Parameters to request the frame list */
                       NB_Error error                                                       /*!< An error to return */
                       );
    virtual bool Progress(int percentage    /*!< A value of percentage */
                         );

    /*! Set the frame list layer

        The FrameListLayer is used to set the tile of frame list. The FrameListLayer
        needs to call this function to set NULL when the FrameListLayer is destroyed.

        @return None
    */
    void
    SetFrameListLayer(FrameListLayer* frameListLayer);


private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    FrameListTileCallback(const FrameListTileCallback& callback);
    FrameListTileCallback& operator=(const FrameListTileCallback& callback);


    // Private members ..........................................................................

    FrameListLayer* m_frameListLayer;       /*!< Frame list layer to set the tile of frame list */
};

};  // namespace nbmap

/*! @} */

#endif  // FRAMELISTTILECALLBACK_H
