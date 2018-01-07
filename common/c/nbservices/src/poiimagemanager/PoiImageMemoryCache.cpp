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
   @file        PoiImageMemoryCache.cpp

   Implementation of PoiImageMemoryCache, refer to header for more
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



#include "PoiImageMemoryCache.h"
#include "datastreamimplementation.h"

using namespace nbsearch;

PoiImageMemoryCache::PoiImageMemoryCache(PAL_Instance* pal,
                                         PoiImageManager * manager,
                                         PoiImageManagerConfigPtr configuration,
                                         bool isThumbnail):
        PoiImageCache(pal, manager, configuration, isThumbnail)
{
}

PoiImageMemoryCache::~PoiImageMemoryCache()
{
    ClearCache();
}

//@todo: overload operator of PoiImageIdentifier;
bool PoiImageMemoryCache::IsIdentical(PoiImageIdentifierPtr identifier1,
                                PoiImageIdentifierPtr identifier2)
{
    return *(identifier1->GetUrl()) == *(identifier2->GetUrl()) &&
            identifier1->IsThumbnail() == identifier1->IsThumbnail();
}
bool PoiImageMemoryCache::ImageExistsInCache(PoiImageIdentifierPtr identifier)
{
    bool result = false;
    unsigned int i = 0;
    for (i = 0; i < m_memoryCache.size(); i++)
    {
        if (IsIdentical(identifier, m_memoryCache[i].m_identifier))
        {
            result = true;
            break;
        }
    }
    return result;;
}

NB_Error PoiImageMemoryCache::SaveImageToCache(PoiImageIdentifierPtr identifier,
                                               nbcommon::DataStreamPtr stream)
{
    // Clear the first entry if limit reached.
    size_t size  = m_memoryCache.size();
    if (size >= MAX_ITEMS_MEMORY)
    {
        m_memoryCache.erase(m_memoryCache.begin());
    }

    PoiImageMemoryCacheEntry entry = {identifier, stream};
    m_memoryCache.push_back(entry);
    return NE_OK;
}

NB_Error PoiImageMemoryCache::ClearCache()
{
    m_memoryCache.clear();
    return NE_OK;
}

nbcommon::DataStreamPtr
PoiImageMemoryCache::GetImageData(PoiImageIdentifierPtr identifier)
{
    for (unsigned int i = 0; i < m_memoryCache.size(); i++)
    {
        if (IsIdentical(identifier, m_memoryCache[i].m_identifier))
        {
            return m_memoryCache[i].m_data;
        }
    }
    return nbcommon::DataStreamPtr(new nbcommon::DataStreamImplementation());
}
