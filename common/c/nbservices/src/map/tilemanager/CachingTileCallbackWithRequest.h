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

    @file       CachingTileCallbackWithRequest.h

    Class CachingTileCallbackWithRequest inherits
    from AsyncCallbackWithRequest interface. A
    CachingTileCallbackWithRequest object is a callback of requesting
    a tile by the caching tile manager. It is used to save the
    data of tile to the cache.
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

#ifndef CACHINGTILECALLBACKWITHREQUEST_H
#define CACHINGTILECALLBACKWITHREQUEST_H

/*!
    @addtogroup nbmap
    @{
*/

#include "AsyncCallback.h"
#include "Cache.h"
#include "TileImpl.h"
#include <string>
#include <map>
#include "datastream.h"

namespace nbmap
{

// Constants ....................................................................................


// Types ........................................................................................

/*! A CachingTileCallbackWithRequest object is a callback of requesting a tile by the caching tile manager */
class CachingTileCallbackWithRequest : public Base,
                                       public AsyncCallbackWithRequest<StringMapPtr, TilePtr>
{
public:
    // Public functions .........................................................................

    /*! CachingTileCallbackWithRequest constructor */
    CachingTileCallbackWithRequest(shared_ptr<string> contentId,   /*!< Content ID */
                                   shared_ptr<string> datasetId,   /*!< Dataset generation ID */
                                   shared_ptr<string> cachingType, /*!< Caching type of the tiles */
                                   nbcommon::CachePtr cache,            /*!< A Cache object to save the tiles */
                                   shared_ptr<bool> callbackValid,      /*!< Flag to identify if the callback is necessary to
                                                                             be called. This flag does not affect the logic of
                                                                             saving the downloaded data to the cache. */
                                   shared_ptr<AsyncCallbackWithRequest<StringMapPtr, TilePtr> > callback, /*!< A callback used to return template
                                                                                                            parameters and a Tile object */
                                   nbcommon::DataStreamPtr origData,
                                   AdditionalDataPtr addtionalData);

    /*! CachingTileCallbackWithRequest destructor */
    virtual ~CachingTileCallbackWithRequest();

    /* See description in AsyncCallback.h */
    virtual void Success(shared_ptr<map<string, string> > templateParameters,/*!< Parameters to request a tile */
                         TilePtr tile                                                   /*!< A Tile object to return */
                         );
    virtual void Error(shared_ptr<map<string, string> > templateParameters,  /*!< Parameters to request a tile */
                       NB_Error error                                                       /*!< An error to return */
                       );
    virtual bool Progress(int percentage    /*!< A value of percentage */
                         );


private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    CachingTileCallbackWithRequest(const CachingTileCallbackWithRequest& callback);
    CachingTileCallbackWithRequest& operator=(const CachingTileCallbackWithRequest& callback);

    // Private members ..........................................................................

    shared_ptr<string> m_contentId;    /*!< Content ID */
    shared_ptr<string> m_datasetId;    /*!< Dataset generation ID */
    shared_ptr<string> m_cachingType;  /*!< Caching type of the tiles */
    nbcommon::CachePtr m_cache;             /*!< A Cache object to save the tiles */
    shared_ptr<bool> m_callbackValid;       /*!< Flag to identify if the callback is necessary to
                                                 be called. This flag does not affect the logic of
                                                 saving the downloaded data to the cache. */
    shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> > m_callback; /*!< A callback used to return template
                                                                                                                     parameters and a Tile object */
    nbcommon::DataStreamPtr m_originalData;
    AdditionalDataPtr       m_additionalData;
};

};  // namespace nbmap

/*! @} */

#endif  // CACHINGTILECALLBACKWITHREQUEST_H
