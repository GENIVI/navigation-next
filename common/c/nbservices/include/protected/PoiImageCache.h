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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
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

 /*!--------------------------------------------------------------------------
   @file        PoiImageCache.h

   Base class for Cache Management used by PoiImageManager, a factory is also
   provided.

   Upper layer makes strict limit of the use for cache. Initially,
   caches are written to local disk. But user may choose to disable this kind
   of caching, in this case, but because the PoiIMageManager and
   PoiImageListener are communicated asynchronously, we need to provide a
   simple cache for PoiImageManager even if cache is disabled (i.e. should not
   store cached files into disk). Therefor, an abstracted class is used here
   to provide interfaces needed by PoiImageManager. A light weight memory
   cache and a persistent cache based on CSL_Cache are implemented to provide
   cache function.

   Note:
   Cache should be configured before creating PoiImageManager, but not be
   changed during a PoiImageManager's life cycle.

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of Networks In
   Motion is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/



#ifndef _POIIMAGECACHE_H_
#define _POIIMAGECACHE_H_

extern "C"
{
#include "nbcontext.h"
#include "nbcontextprotected.h"
#include "palfile.h"
}

#include "palmath.h"
#include "poiimagemanager.h"
#include "datastream.h"
#include "poiimageidentifierimplementation.h"

namespace nbsearch
{

const unsigned int MAX_ITEMS_MEMORY      = 6;
const unsigned int MALLOC_MAX_RETRY_TIME = 3;

/*! Base class for cache used by poiimagemanager */
class PoiImageCache
{
    /*! Default constructor. */
public:
    PoiImageCache(PAL_Instance* pal,                      /*!<  pal instance. */
                  PoiImageManager * manager,              /*!<  PoiIMageManager  instance*/
                  PoiImageManagerConfigPtr configuration, /*!<  configuration of cache.  */
                  bool isThumbnail                        /*!< flag to indicate whether this is
                                                               used to cache thumbnail or not. */
                  )
    {
        m_pal = pal;
        m_Manager = manager;
        m_configuration = configuration;
        m_IsOk = true;
        m_isThumbnail = isThumbnail;
    }

    /*! Deconstructor */
    virtual ~PoiImageCache() { }

    /*! Save image data into cache.

        @return NE_OK if succeeded.
    */
    virtual NB_Error
    SaveImageToCache(PoiImageIdentifierPtr identifier,  /*!< Identifier of Poi Image */
                     nbcommon::DataStreamPtr stream     /*!< datastream pointer containing image */
                     ) = 0;

    /*! Clear all cached data, and remove necessary directory.

        @return NE_OK if succeeded.
    */
    virtual
    NB_Error ClearCache() = 0;

    /*! Return Binary data to the caller.

      @return nbcommon::DataStreamPtr containing image data.
    */
    virtual nbcommon::DataStreamPtr
    GetImageData(PoiImageIdentifierPtr identifier /*!< identifier of Poi Image */
                 ) = 0;

    /*! Return status of this cache.

        @return true if it is initialized correctly.
    */
    bool IsOk();


    /*! Check whether there's image in cache.

      @return true if so.
    */
    virtual bool
    ImageExistsInCache(PoiImageIdentifierPtr identifier
                       ) = 0;


protected:

    PoiImageManagerConfigPtr m_configuration; /*!< Configuration used for POI image manager */
    PAL_Instance* m_pal;
    bool m_IsOk;
    bool m_isThumbnail;
    PoiImageManager* m_Manager;
};

typedef shared_ptr<PoiImageCache> PoiImageCachePtr;

/*! Cache factory. */
class PoiImageCacheFactory {
public:
    /*! Default constructor */
    PoiImageCacheFactory(PAL_Instance* pal,                     /*!< PAL instance */
                         PoiImageManager* manager,              /*!< PoiImageManager instance */
                         PoiImageManagerConfigPtr configuration /*!< configuration of cache. */
                         );
    /*! Deconstructor. */
    virtual ~PoiImageCacheFactory();

    /*! Creates a PoiImageCach instance based on configuration.

       @return Shared pointer of PoiImageCache.
    */
    PoiImageCachePtr
    CreateInstance(bool isThumbnail = true);
private:
    PoiImageManagerConfigPtr m_configuration; /*!< Configuration used for POI image manager */
    PAL_Instance* m_pal;                      /*!< PAL instance. */
    PoiImageManager* m_Manager;               /*!< PoiImageManager instance*/
};

};
#endif /* _POIIMAGECACHE_H_ */
