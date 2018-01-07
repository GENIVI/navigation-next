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
   @file        PoiImagePersistentCache.h

   Inherits from PoiImageCache, used CSL_Cache to provide function to cache
   Poi Images to local disk. Refer to PoiImageCache for more information.
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



#ifndef _POIIMAGEPERSISTENTCACHE_H_
#define _POIIMAGEPERSISTENTCACHE_H_

#include "PoiImageCache.h"

namespace nbsearch
{
class PoiImagePersistentCache : public PoiImageCache
{
public:
    /*! Refer to PoiImageCache.h for more description. */
    PoiImagePersistentCache(PAL_Instance *pal,
                            PoiImageManager* manager,
                            PoiImageManagerConfigPtr configuration,
                            bool isThumbnail);;

    /*! Refer to PoiImageCache.h for more description. */
    virtual ~PoiImagePersistentCache();

    /*! Refer to PoiImageCache.h for more description. */
    virtual bool ImageExistsInCache(PoiImageIdentifierPtr identifier);

    /*! Refer to PoiImageCache.h for more description. */
    virtual NB_Error
    SaveImageToCache(PoiImageIdentifierPtr identifier,
                     nbcommon::DataStreamPtr stream
                     );

    /*! Refer to PoiImageCache.h for more description. */
    virtual NB_Error ClearCache();

    /*! Refer to PoiImageCache.h for more description. */
    virtual nbcommon::DataStreamPtr
    GetImageData(PoiImageIdentifierPtr identifier);

private:
    /*! Callback when a previously cached item is removed.

        @return None.
    */
    static void
    CacheRemovedCallback(CSL_Cache* cache, const byte* name, size_t namelen, void* userData);

    CSL_Cache* m_CSLCache;   /*!< The CSL cache for thumbnail images */

    string m_cachePath;      /*!< Path of cached files.. */
};

};

#endif /* _POIIMAGEPERSISTENTCACHE_H_ */
