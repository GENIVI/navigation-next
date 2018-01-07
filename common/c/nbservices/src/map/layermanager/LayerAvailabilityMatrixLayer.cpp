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
  @file        LayerAvailabilityMatrixLayer.cpp
  @defgroup    nbmap

  Description: Implementation of LayerAvailabilityMatrixLayer.

*/
/*
  (C) Copyright 2012 by TeleCommunications Systems, Inc.

  The information contained herein is confidential, proprietary to
  TeleCommunication Systems, Inc., and considered a trade secret as defined
  in section 499C of the penal code of the State of California. Use of this
  information by anyone other than authorized employees of TeleCommunication
  Systems is granted only under a written non-disclosure agreement, expressly
  prescribing the scope and manner of such use.

  --------------------------------------------------------------------------*/

/*! @{ */

extern "C"
{
#include "palclock.h"
#include "nbqalog.h"
#include "nbcontextprotected.h"
}
#include "LayerAvailabilityMatrixLayer.h"
#include "UnifiedLayerWithLAM.h"
#include "LamProcessor.h"

using namespace nbmap;

// LAM tile is used to identify if layers contains content within each level 15 tile area.
// This is specified by SDS, but not represented by metadata, so we have to hardcode this.
static const int    LAM_IDENTIFIED_ZOOM_LEVEL = 15;

/* See description in header file. */
LayerAvailabilityMatrixLayer::LayerAvailabilityMatrixLayer(TileManagerPtr tileManager,
                                                           uint32 layerIdDigital,
                                                           NB_Context* context)
        : UnifiedLayer(tileManager, layerIdDigital, context)
{
    SetDisplayable(false);
    // Initialize m_pLamTileRequestCallback here.
    m_pLamTileRequestCallback = shared_ptr<LamTileRequestCallback> (
        new LamTileRequestCallback(this));
    m_lamProcessor = new LamProcessor(context);
}

/* See description in header file. */
LayerAvailabilityMatrixLayer::~LayerAvailabilityMatrixLayer()
{
    m_pLamTileRequestCallback->Reset();
    delete m_lamProcessor;
}

/* See description in header file. */

/* See description in header file. */
void LayerAvailabilityMatrixLayer::GetTiles(vector <TileKeyPtr> /*tileKey*/,
                                            AsyncTileRequestWithRequestPtr /*callback*/,
                                            uint32 /*priority*/)
{
    // @note: do nothing in this function. This function should not be called.
}

/* See description in header file. */
TileKey LayerAvailabilityMatrixLayer::ConvertTileKeyToLamKey(TileKeyPtr tileKey)
{
    TileKey lamKey(*tileKey);
    if (tileKey)
    {
        int lamLevel = (int)(m_tileLayerInfo->refTileGridLevel);
        int distance = tileKey->m_zoomLevel - lamLevel;
        if (distance > 0)
        {
            lamKey.m_x         = tileKey->m_x >> distance;
            lamKey.m_y         = tileKey->m_y >> distance;
            lamKey.m_zoomLevel = lamLevel;
        }
    }
    return lamKey;
}

/* See description in header file. */
LamCacheEntry* LayerAvailabilityMatrixLayer::FindUsableLamCacheEntry(const TileKey& lamKey)
{
    // Check whether we can find a cache entry who is READY or UnderGoing.
    map<TileKey, LamCacheEntry*> :: iterator iter = m_dirtyEntries.find(lamKey);
    if (iter != m_dirtyEntries.end())
    {
        nsl_assert(iter->second->m_tileKey == lamKey);
        return iter->second;
    }

    // Other wise, try to find a free entry or reuse the oldest READY one.
    LamCacheEntry* lastAccessEntry = NULL;
    time_t lastAccessTime = PAL_ClockGetUnixTime();
    for (int i = 0; i < LAMCONTAINER_CAPACITY; i++)
    {
        // If status of cache is free, use this one.
        if (m_pLamCache[i].m_state == NB_LAM_State_Free)
        {
            m_pLamCache[i].m_tileKey        = lamKey;
            m_pLamCache[i].m_lastAccessTime = PAL_ClockGetUnixTime();
            if (m_pLamCache[i].m_decodedData)
            {
                DestroyPNG(m_pLamCache[i].m_decodedData);
                m_pLamCache[i].m_decodedData = NULL;
            }
            m_dirtyEntries.insert(make_pair(lamKey, &m_pLamCache[i]));
            nsl_assert(m_dirtyEntries.size() <= LAMCONTAINER_CAPACITY);
            return &m_pLamCache[i];
        }

        // If entry is not free, but ready, then update local pointer to find the oldest one.
        if (m_pLamCache[i].m_lastAccessTime <= lastAccessTime &&
            m_pLamCache[i].m_state == NB_LAM_State_Ready)
        {
            lastAccessEntry = &m_pLamCache[i];
            lastAccessTime  = lastAccessEntry->m_lastAccessTime;
        }
    }

    // If we reached here, it means no free entry is available, and lastAccessEntry points
    // to the old entry whose status is NB_LAM_State_Ready. We need to reuse this one.
    if (lastAccessEntry)
    {
        m_dirtyEntries.erase(lastAccessEntry->m_tileKey);
        lastAccessEntry->Reset();
        lastAccessEntry->m_tileKey = lamKey;
        m_dirtyEntries.insert(make_pair(lamKey, lastAccessEntry));
        nsl_assert(m_dirtyEntries.size() <= LAMCONTAINER_CAPACITY);
    }

    return lastAccessEntry;
}
/* See description in header file. */
void LayerAvailabilityMatrixLayer::TileRequestSuccess(TileKeyPtr request, TilePtr response)
{
    if (request && response)
    {
        map<TileKey, LamCacheEntry*>::iterator entryIter = m_dirtyEntries.find(*request);
        if (entryIter != m_dirtyEntries.end())
        {
            LamCacheEntry* entry = entryIter->second;
            nsl_assert(entry != NULL);

            // Decode PNG data, and update statistics.
            entry->m_lastAccessTime = PAL_ClockGetUnixTime();
            entry->m_decodedData    = m_lamProcessor->DecodeLamFromTile(response);
            entry->m_state          = NB_LAM_State_Ready;
            ProcessRequestsInCacheEntry(entry);
            return;
        }
    }

    TileRequestError(request, NE_UNEXPECTED);
}

/* See description in header file. */
void LayerAvailabilityMatrixLayer::TileRequestError(TileKeyPtr tileKey, NB_Error error)
{
    if (!tileKey)
    {
        return;
    }

    map<TileKey, LamCacheEntry*>::iterator entryIter = m_dirtyEntries.find(*tileKey);
    LamCacheEntry* entry = NULL;
    if (entryIter != m_dirtyEntries.end())
    {
        entry = entryIter->second;
        nsl_assert(entry != NULL);

        vector<TileRequestEntry> requests;
        entry->m_requests.swap(requests);
        entry->Reset();
        m_dirtyEntries.erase(*tileKey);

        /* Report the error for each tile request. The layer's content SHALL be downloaded
           only if the LAM tile is available.
        */
        vector<TileRequestEntry>::iterator it  = requests.begin();
        vector<TileRequestEntry>::iterator end = requests.end();
        while (it != end)
        {
            const TileRequestEntry& request = *it;
            request.ReportError(error);
            ++it;
        }
    }
}

/* See description in header file. */
void LayerAvailabilityMatrixLayer::ProcessRequestsInCacheEntry(LamCacheEntry* entry)
{
    if (!entry || entry->m_requests.empty())
    {
        return;
    }

    vector<TileRequestEntry> requests;
    entry->m_requests.swap(requests);

    vector<TileRequestEntry>::iterator it  = requests.begin();
    vector<TileRequestEntry>::iterator end = requests.end();
    while (it != end)
    {
        const TileRequestEntry& request = *it;
        if (request.IsLayerAvaiable() && IsRequestDownloadable(entry->m_decodedData, request))
        {
            request.StartRealDownload();
        }
        ++it;
    }
}

/* See description in header file. */
bool LayerAvailabilityMatrixLayer::IsRequestDownloadable(PNG_Data* decodedData,
                                                         const TileRequestEntry& request)
{
    bool       result   = false;
    uint32     lamIndex = request.GetLayerAvailabilityMatrixIndex();
    TileKeyPtr tileKey  = request.GetDataTileKey();
    if (m_lamProcessor && decodedData && tileKey && lamIndex != INVALID_LAM_INDEX)
    {
        if (tileKey->m_zoomLevel == LAM_IDENTIFIED_ZOOM_LEVEL)
        {
            result = m_lamProcessor->CheckLamIndex(decodedData, tileKey->m_x, tileKey->m_y,
                                                   lamIndex);

            // Is QA logging enabled
            if (NB_ContextGetQaLog(m_pContext))
            {
                shared_ptr<string> layerType = request.GetTileDataType();
                if (layerType && !layerType->empty())
                {
                    NB_QaLogLayerAvailabilityMatrixCheckResult(m_pContext, tileKey->m_x,
                                                               tileKey->m_y, layerType->c_str(),
                                                               (nb_boolean)result);
                }
            }
        }
        else
        {
            vector<TileKeyPtr> convertedKeys;
            ConvertTileKeyToOtherZoomLevel(tileKey, LAM_IDENTIFIED_ZOOM_LEVEL,
                                           convertedKeys);
            vector<TileKeyPtr>::iterator iter = convertedKeys.begin();
            vector<TileKeyPtr>::iterator end  = convertedKeys.end();
            while (iter != end)
            {
                const TileKeyPtr& key = *iter;
                result |= m_lamProcessor->CheckLamIndex(decodedData, key->m_x, key->m_y,
                                                        lamIndex);
                // @todo: add qalog if really necessary!
                if (result)
                {
                    break;
                }
                ++iter;
            }
        }
    }
    else
    {
        result = true;
    }

    return result;
}

/* See description in header file. */
void
LayerAvailabilityMatrixLayer::GetTileWithLamOptimized(const TileRequestEntry& request)
{
    TileKey        lamKey = ConvertTileKeyToLamKey(request.GetDataTileKey());
    LamCacheEntry* entry  = NULL;
    if (!m_lamProcessor ||
        ((entry = FindUsableLamCacheEntry(lamKey)) == NULL))
    {
        request.ReportError(NE_NOMEM);
        return;
    }

    entry->m_requests.push_back(request);
    switch (entry->m_state)
    {
        case NB_LAM_State_Ready: // Most of cases, it would be READY.
        {
            ProcessRequestsInCacheEntry(entry);
            break;
        }
        case NB_LAM_State_Free: // Or it may be FREE.
        {
            // Download LAM, parse it and start real download.
            TileKeyPtr lamKeyPtr(new TileKey(lamKey));
            entry->m_state = NB_LAM_State_Undergoing;
            GetTile(lamKeyPtr, lamKeyPtr, m_pLamTileRequestCallback,
                    HIGHEST_TILE_REQUEST_PRIORITY);
            break;
        }
        case NB_LAM_State_Undergoing: // Store tile key and layer and callback.
        {
            break;
        }
        default:
        {
            break;
        }
    }
}

// Implementation of TileRequestEntry.

/* See description in header file. */
TileRequestEntry::TileRequestEntry(TileKeyPtr tileKey,
                                   TileKeyPtr convertedTileKey,
                                   UnifiedLayerWithLAM* layer,
                                   AsyncTileRequestWithRequestPtr callback,
                                   uint32 priority,
                                   shared_ptr<bool> valid)
        : m_tileKey(tileKey),
          m_dataTileKey(convertedTileKey),
          m_layer(layer),
          m_callback(callback),
          m_priority(priority),
          m_isValid(valid)
{
}

/* See description in header file. */
TileRequestEntry::~TileRequestEntry()
{
}

/* See description in header file. */
void TileRequestEntry::StartRealDownload() const
{
    if (m_tileKey && IsLayerAvaiable())
    {
        m_layer->StartRealDownload(m_tileKey, m_dataTileKey, m_callback, m_priority);
    }
}

/* See description in header file. */
void TileRequestEntry::ReportError(NB_Error error) const
{
    if (m_tileKey && m_callback)
    {
        m_callback->Error(m_tileKey, error);
    }
}

/* See description in header file. */
uint32 TileRequestEntry::GetLayerAvailabilityMatrixIndex() const
{
    return   IsLayerAvaiable() ? m_layer->GetLayerAvailabilityMatrixIndex() : INVALID_LAM_INDEX;
}

/* See description in header file. */
shared_ptr<string> TileRequestEntry::GetTileDataType() const
{
    return m_layer && m_isValid && *m_isValid ? m_layer->GetTileDataType() : shared_ptr<string>();
}

/* See description in header file. */
TileKeyPtr TileRequestEntry::GetDataTileKey() const
{
    return m_dataTileKey;
}

/* See description in header file. */
bool TileRequestEntry::IsLayerAvaiable() const
{
    return (m_layer &&  m_isValid && *m_isValid) ? true : false;
}

// Implementation of LamTileRequestCallback.

/* See description in header file. */
LamTileRequestCallback::LamTileRequestCallback(LayerAvailabilityMatrixLayer* lamLayer)
        : m_pLamLayer(lamLayer)
{
}

/* See description in header file. */
LamTileRequestCallback::~LamTileRequestCallback()
{
}

/* See description in header file. */
void LamTileRequestCallback::Success(TileKeyPtr request, TilePtr response)
{
    if (m_pLamLayer)
    {
        m_pLamLayer->TileRequestSuccess(request, response);
    }
}

/* See description in header file. */
void LamTileRequestCallback::Error(TileKeyPtr request, NB_Error error)
{
    if (m_pLamLayer)
    {
        m_pLamLayer->TileRequestError(request, error);
    }
}

/* See description in header file. */
void LamTileRequestCallback::Reset()
{
    m_pLamLayer = NULL;
}

/* See description in header file. */
LamCacheEntry::LamCacheEntry()
        : m_tileKey(),
          m_lastAccessTime(0),
          m_state(NB_LAM_State_Free),
          m_decodedData(NULL)
{
}

/* See description in header file. */
LamCacheEntry::~LamCacheEntry()
{
    Reset();
}

/* See description in header file. */
void LamCacheEntry::Reset()
{
    m_requests.clear();
    if (m_decodedData)
    {
        DestroyPNG(m_decodedData);
    }
    m_decodedData = NULL;
    m_state       = NB_LAM_State_Free;
    m_tileKey     = TileKey();
}

/*! @} */
