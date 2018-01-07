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

    @file       poiimagelistener.h

    Class PoiImageListener should be inherited by the user of PoiImageManager.
    User could receive the events of image downloaded and removed by the
    listener. The event of image downloaded is called when the image has been
    downloaded or there is an error occurred during downloading. The event of
    image removed is called when the cache is full and the specified image is
    removed from the cache.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary to
    TeleCommunication Systems, Inc., and considered a trade secret as defined
    in section 499C of the penal code of the State of California. Use of this
    information by anyone other than authorized employees of Networks In
    Motion is granted only under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

#ifndef POIIMAGEMANAGERLISTENER_H
#define POIIMAGEMANAGERLISTENER_H

/*!
    @addtogroup poiimagemanager
    @{
*/

#include "smartpointer.h"
#include "poiimageidentifier.h"

namespace nbsearch
{
/*! The POI image listener */
class NB_DLL PoiImageListener
{
public:
    // Public functions ........................................................................

    /*! Default constructor. */
    PoiImageListener()
    {
    }
    /*! PoiImageListener destructor */
    virtual ~PoiImageListener()
    {
        // Nothing to do here.
    }

    /*! Notify that a image has been downloaded.

        @return None
    */
    virtual void
    ImageDownloaded(NB_Error result,                       /*!< Download result */
                    const PoiImageIdentifierPtr identifier /*!< Identifier of the image
                                                                downloaded. */
                    ) = 0;

    /*! Notify a image removed from cache

        @return None
    */
    virtual void
    ImageRemoved(const PoiImageIdentifierPtr identifier /*!< Identifier of the image removed. */
                 ) = 0;
};

typedef shared_ptr<PoiImageListener> PoiImageListenerPtr;

};

/*! @} */

#endif
