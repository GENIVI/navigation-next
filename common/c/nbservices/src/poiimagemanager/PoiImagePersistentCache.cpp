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
   @file        PoiImagePersistentCache.cpp
   Implementation of PoiImagePersistentCache, refer to header file for more
   description.

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



#include "PoiImagePersistentCache.h"
#include "poiimagemanagerimplementation.h"
#include "datastreamimplementation.h"

using namespace nbsearch;

const char*  CACHE_THUMBNAIL       = "poi_thumbnail_images";
const char*  CACHE_LARGE           = "poi_large_images";
static const byte OBFUSCATE_KEY[]  = {0x01, 0x02, 0x03, 0xAA, 0xBB, 0xAF, 0xFA, 0x23, 0xA8, 0x99};
const int    MALLOC_MAX_RETRY_TIME      = 3;

PoiImagePersistentCache::PoiImagePersistentCache(PAL_Instance* pal,
                                                 PoiImageManager * manager,
                                                 PoiImageManagerConfigPtr configuration,
                                                 bool isThumbnail):
        PoiImageCache(pal, manager, configuration, isThumbnail)
{
    nsl_assert(pal != NULL);

    NB_Error error     = NE_OK;
    char*    cachePath = NULL;

    do {
        int pathLength     = m_configuration->m_cachePath.length() + \
                             MAX(nsl_strlen(CACHE_THUMBNAIL), nsl_strlen(CACHE_LARGE)) + 3;
        cachePath = new char[pathLength];
        if (cachePath == NULL)
        {
            error = NE_NOMEM;
            break;
        }
        nsl_memset(cachePath, 0, pathLength);
        nsl_memcpy(cachePath, m_configuration->m_cachePath.c_str(),
                   m_configuration->m_cachePath.length());

        PAL_Error palError = PAL_Ok;
        palError = PAL_FileAppendPath(pal, cachePath, pathLength,
                                      isThumbnail ? CACHE_THUMBNAIL : CACHE_LARGE);
        palError = palError ? palError : PAL_FileCreateDirectoryEx(pal, cachePath);
        if (palError != PAL_Ok)
        {
            error = NE_FSYS;
            break; // Just break, allocated resources will be cleaned at the end of loop.
        }

        m_CSLCache = CSL_CacheAlloc(pal, cachePath,
                                    MAX_ITEMS_MEMORY,
                                    isThumbnail? m_configuration->m_thumbnailImageCacheSize :
                                    m_configuration->m_largeImageCacheSize,
                                    OBFUSCATE_KEY, sizeof(OBFUSCATE_KEY),
                                    NULL, TRUE);
        if (!m_CSLCache)
        {
            error = NE_NOMEM;
            break;
        }

        CSL_CacheLoadIndex(m_CSLCache);
        CSL_CacheSetNotificationFunctions(m_CSLCache, CacheRemovedCallback,
                                          (void*)this);
        m_cachePath = string(cachePath);

    } while (0);

    if (cachePath)
    {
        delete[] cachePath;
    }
    if (error != NE_OK)
    {
        CSL_CacheDealloc(m_CSLCache);
        m_IsOk = false;
    }
}


PoiImagePersistentCache::~PoiImagePersistentCache()
{
    if (m_CSLCache)
    {
        CSL_CacheDealloc(m_CSLCache);
    }
}

bool PoiImagePersistentCache::ImageExistsInCache(PoiImageIdentifierPtr identifier)
{
    bool result = false;
    if (m_CSLCache)
    {
        shared_ptr<std::string> url = identifier->GetUrl();
        if (!url) // should not happen.
        {
            return false;
        }
        byte*  data   = NULL;
        size_t length = 0;
        NB_Error error = CSL_CacheFind(m_CSLCache, (byte*)(url->c_str()), url->length(),
                                       &data, &length, FALSE, FALSE, FALSE);
        if (error == NE_OK && length > 0)
        {
            result = true;
        }
    }
    return result;
}

NB_Error PoiImagePersistentCache::SaveImageToCache(PoiImageIdentifierPtr identifier,
                                                   nbcommon::DataStreamPtr stream)
{
    uint32 total = stream->GetDataSize(); // Invalid data.
    if (total == 0)
    {
        return NE_NET;
    }

    byte* data    = NULL;
    int   counter = 0;
    do {
        data = (byte*)nsl_malloc(total);
        counter++;
    } while (data == NULL && counter < MALLOC_MAX_RETRY_TIME);

    // return if failed to allocate memory for more than MALLOC_MAX_RETRY_TIME times.
    if (data == NULL)
    {
        return NE_NOMEM;
    }

    uint32 got      = 0;
    uint32 pos      = 0;
    uint32 required = total;
    do
    {
        got += stream->GetData(data, pos, required);
        pos = got;
        required -= got;
    } while (got < total && got != 0);

    shared_ptr<std::string> url = identifier->GetUrl();
    NB_Error error = CSL_CacheAdd(m_CSLCache, (byte*)(url->c_str()), url->length(), data, got,
                                FALSE, FALSE, 0);
    if (error == NE_OK) // Just return if failed to add content to cache.
    {
        // Image has been stored into cache, and even the following function calls fails,
        // image can still be read from cache, so just keep going.
        CSL_CacheSaveDirty(m_CSLCache, -1, NULL, true);
        CSL_CacheSaveIndex(m_CSLCache);
    }
    return error;;
}

NB_Error PoiImagePersistentCache::ClearCache()
{
    nsl_assert(m_pal != NULL);

    NB_Error error = NE_OK;
    if (m_CSLCache)
    {
        CSL_CacheDealloc(m_CSLCache);
    }
    m_CSLCache = NULL;

    if (!m_cachePath.empty())
    {
        PAL_Error palError = PAL_FileRemoveDirectory(m_pal, m_cachePath.c_str(), TRUE);
        if (palError != PAL_Ok)
        {
            error = NE_FSYS;
        }
    }

    return error;
}

void PoiImagePersistentCache::CacheRemovedCallback(CSL_Cache* cache, const byte* name,
                                                   size_t namelen, void * userData)
{
    if (userData)
    {
        PoiImagePersistentCache* me = (PoiImagePersistentCache*)userData;
        shared_ptr<std::string> url = shared_ptr<std::string> (new std::string((char*)name));
        if (url != NULL)
        {
            PoiImageIdentifierPtr identifier =
                    PoiImageIdentifierPtr(new
                                          PoiImageIdentifierImplementation(url, me->m_isThumbnail));

            ((PoiImageManagerImplementation*)(me->m_Manager))->ImageRemoved(identifier);
        }
    }
}

nbcommon::DataStreamPtr
PoiImagePersistentCache::GetImageData(PoiImageIdentifierPtr identifier)
{
    nbcommon::DataStreamPtr dataStream(new nbcommon::DataStreamImplementation());

    shared_ptr<std::string> url   = identifier->GetUrl();
    byte*    data   = NULL;
    size_t   length = 0;
    NB_Error err    = CSL_CacheFind(m_CSLCache, (byte*)(url->c_str()), url->length(),
                                    &data, &length, FALSE, FALSE, FALSE);
    if (err == NE_OK && length > 0)
    {
        dataStream->AppendData(data, length);
    }
    return dataStream;
}

