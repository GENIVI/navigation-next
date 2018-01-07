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

    @file       CachingTileCallbackWithRequest.cpp

    See header file for description.
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

extern "C"
{
}

#include "CachingTileCallbackWithRequest.h"
#include "datastreamimplementation.h"
#include "StringUtility.h"
#include "CacheOperationCallback.h"

/*! @{ */

using namespace std;
using namespace nbmap;
using namespace nbcommon;

// Local Constants ..............................................................................

#define BUFFER_SIZE 4096


// Public functions .............................................................................

/* See header file for description */
CachingTileCallbackWithRequest::CachingTileCallbackWithRequest(shared_ptr<string> contentId,
                                                               shared_ptr<string> datasetId,
                                                               shared_ptr<string> cachingType,
                                                               CachePtr cache,
                                                               shared_ptr<bool> callbackValid,
                                                               shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> > callback,
                                                               DataStreamPtr origData,
                                                               AdditionalDataPtr
                                                               additionalData)
        : AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr>(),
          m_contentId(contentId),
          m_datasetId(datasetId),
          m_cachingType(cachingType),
          m_cache(cache),
          m_callbackValid(callbackValid),
          m_callback(callback),
          m_originalData(origData),
          m_additionalData(additionalData)
{
}

/* See header file for description */
CachingTileCallbackWithRequest::~CachingTileCallbackWithRequest()
{
}

/* See description in AsyncCallback.h */
void
CachingTileCallbackWithRequest::Success(shared_ptr<map<string, string> > templateParameters,
                                        TilePtr tile)
{
    //NB_Error  error    = NE_OK;
    TileImpl* tileImpl = NULL;
    // Check if the pointer to the tile is NULL.
    if (!tile ||
        (tileImpl = static_cast<TileImpl*>(tile.get())) == NULL)
    {
        if (m_callback && m_callbackValid && (*m_callbackValid))
        {
            m_callback->Error(templateParameters, NE_UNEXPECTED);
        }
        return;
    }

    // Check if the content ID is empty.
    shared_ptr<string> cachingName = tileImpl->GetCachingName();
    if ((!cachingName) || (cachingName->empty()))
    {
        if (m_callback && m_callbackValid && (*m_callbackValid))
        {
            m_callback->Error(templateParameters, NE_UNEXPECTED);
        }
        return;
    }

    // Add the dataset ID to the additional data of the tileImpl.
    shared_ptr<map<string, shared_ptr<string> > > additionalData = tileImpl->GetAdditionalData();
    if (m_datasetId)
    {
        if (!additionalData)
        {
            additionalData.reset(new map<string, shared_ptr<string> >());
            if (!additionalData)
            {
                if (m_callback && m_callbackValid && (*m_callbackValid))
                {
                    m_callback->Error(templateParameters, NE_NOMEM);
                }
                return;
            }
        }

        // Add the dataset ID.
        (*additionalData)[TILE_ADDITIONAL_KEY_DATASET_ID] = m_datasetId;

        // Set the additional data to tileImpl.
        tileImpl->SetAdditionalData(additionalData);
    }

    // Save the data of tileImpl to the cache.
    if (m_cachingType && m_cache)
    {
        // Build the caching path. It is used by NBGM library to read the data of the caching tileImpl.
        shared_ptr<string> cachingPath = m_cache->BuildCachingPath(m_cachingType, cachingName);
        if (!cachingPath)
        {
            if (m_callback && m_callbackValid && (*m_callbackValid))
            {
                m_callback->Error(templateParameters, NE_UNEXPECTED);
            }
            return;
        }

        // Set the full path of the caching tileImpl.
        tileImpl->SetCachingPath(cachingPath);

        DataStreamPtr dataStream = tileImpl->GetData();
        if ((!dataStream) || (dataStream->GetDataSize() == 0))
        {
            // @note: Server may retured 304, needs to read from cache.
            if (!m_originalData)
            {
                m_cache->RemoveData(m_cachingType, cachingName);

                if (m_callback && m_callbackValid && (*m_callbackValid))
                {
                    m_callback->Error(templateParameters, NE_UNEXPECTED);
                }
                return;
            }

            // Merge the caching additional data to the additional data of tileImpl.
            if (m_additionalData && (!(m_additionalData->empty())))
            {
                if (!additionalData)
                {
                    additionalData.reset(new map<string, shared_ptr<string> >());
                    if (!additionalData)
                    {
                        if (m_callback && m_callbackValid && (*m_callbackValid))
                        {
                            m_callback->Error(templateParameters, NE_NOMEM);
                        }
                        return;
                    }
                }

                additionalData->insert(m_additionalData->begin(), m_additionalData->end());
            }

            // Protect caching item if it is common material.
            if ((*m_cachingType) == TILE_TYPE_COMMON_MATERIALS
                ||(*m_cachingType) == TILE_TYPE_DATA_AVAILABILITY_MATRIX)
            {
                (*additionalData)[CACHE_ADDITIONAL_KEY_PROTECTED] = shared_ptr<string> (new string("true"));
            }
            // Save the additional data of key and value to cache (dataset ID and ETAG).
            m_cache->SetAdditionalData(m_cachingType, cachingName, additionalData);

            // Set the additional data of tileImpl.
            tileImpl->SetAdditionalData(additionalData);

            if (m_callback && m_callbackValid && (*m_callbackValid))
            {
                /* If the generic download manager returns NE_HTTP_NOT_MODIFIED,
                   data returned from cached should be stored as m_originalData;
                   read from cache.
                */
                if ((!m_originalData) || (m_originalData->GetDataSize() == 0))
                {
                    m_callback->Error(templateParameters, NE_UNEXPECTED);
                    return;
                }

                // Set the tileImpl data.
                tileImpl->SetData(m_originalData);
            }
        }
        else if (!tileImpl->IsDuplicatedTile())
        {
            m_cache->RemoveData(m_cachingType, cachingName);

            // Save data to cache.
            m_cache->SaveData(m_cachingType, cachingName, dataStream, additionalData,
                                      CacheOperationCallbackPtr());
        }
    }

    if (m_callback && m_callbackValid && (*m_callbackValid))
    {
        m_callback->Success(templateParameters, tile);
    }
}

/* See description in AsyncCallback.h */
void
CachingTileCallbackWithRequest::Error(shared_ptr<map<string, string> > templateParameters,
                                      NB_Error error)
{
    // Save the error code of NE_HTTP_NO_CONTENT in cache to avoid downloading again.
    if (error == NE_HTTP_NO_CONTENT)
    {
        // Check if the cache is valid.
        if (m_contentId && (!(m_contentId->empty())) &&
            m_cachingType && (!(m_cachingType->empty())) &&
            m_cache)
        {
            // Create the additional data to save the error code and dataset ID.
            AdditionalDataPtr  additionalData(new AdditionalData());
            if (!additionalData)
            {
                if (m_callback && m_callbackValid && (*m_callbackValid))
                {
                    m_callback->Error(templateParameters, NE_NOMEM);
                }
                return;
            }

            // Save the error code.
            shared_ptr<string> errorString = StringUtility::NumberToStringPtr(static_cast<int>(error));
            if ((!errorString) || (errorString->empty()))
            {
                if (m_callback && m_callbackValid && (*m_callbackValid))
                {
                    m_callback->Error(templateParameters, NE_UNEXPECTED);
                }
                return;
            }
            (*additionalData)[TILE_ADDITIONAL_KEY_ERROR] = errorString;

            // Save the dataset ID.
            if (m_datasetId && (!(m_datasetId->empty())))
            {
                (*additionalData)[TILE_ADDITIONAL_KEY_DATASET_ID] = m_datasetId;
            }

            // Remove previous saved caching data if the caching item exists.
            if (m_cache->IsItemExisting(m_cachingType, m_contentId))
            {
                m_cache->RemoveData(m_cachingType, m_contentId);
            }

            // Save the additional data to cache.
            m_cache->SaveData(m_cachingType,
                              m_contentId,
                              DataStreamPtr(),
                              additionalData);
        }
    }

    if (m_callback && m_callbackValid && (*m_callbackValid))
    {
        m_callback->Error(templateParameters, error);
    }
}

/* See description in AsyncCallback.h */
bool
CachingTileCallbackWithRequest::Progress(int percentage)
{
    if (m_callback && m_callbackValid && (*m_callbackValid))
    {
        return m_callback->Progress(percentage);
    }

    return false;
}

/*! @} */
