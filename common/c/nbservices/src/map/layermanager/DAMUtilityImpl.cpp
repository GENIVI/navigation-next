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
   @file        DAMUtilityImpl.cpp
   @defgroup    nbmap

*/
/*
   (C) Copyright 2013 by TeleCommunications Systems, Inc.

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
#include "nbqalog.h"
}

#include "nbspatial.h"
#include "DAMUtilityImpl.h"
#include "TileServicesConfiguration.h"
#include "UnifiedLayerManager.h"
#include <algorithm>
#include <math.h>
#include <set>
#include "TileKeyUtils.h"
#include "AggregateLayerProvider.h"
using namespace nbmap;
using namespace std;

static const uint32 BITS_PER_BYTE = 8;
// The macro defines come from DAM ESD document
static const int DEFAULT_DAM_ZOOMLEVEL                  = 11;

DAMUtilityImpl::DAMUtilityImpl(NB_Context* context, LayerManagerPtr layerManager)
    : UnifiedLayerListener(context),
      m_pDamTileRequestcallback(new DamTileRequestCallback(this)),
      m_layerManager(layerManager),
      m_lamProcessor(new LamProcessor(context))
{
    shared_ptr<AggregateLayerProvider> aggregateLayerProvider =
            ContextBasedSingleton<AggregateLayerProvider>::getInstance(context);
    aggregateLayerProvider->RegisterUnifiedLayerListener(this, false);
}

DAMUtilityImpl::~DAMUtilityImpl()
{
    if (m_lamProcessor)
    {
        delete m_lamProcessor;
    }
    m_layerManager.reset();
    m_pDamTileRequestcallback.reset();
}
/*! see description in header. */
bool DAMUtilityImpl::IsDataAvailable(double latitude, double longitude, std::bitset<DI_TotalCount> damIndices)
{
    // if a combination of bits is asked then we return true if all bits are true, otherwise false
    int xTile = -1;
    int yTile = -1;
    ConvertLatLongToTile(latitude, longitude, xTile, yTile);

    for (size_t i = 0; i < DI_TotalCount; ++i)
    {
        if (damIndices[i])
        {
            if (!CheckDamIndex(xTile, yTile, i))
            {
                return false;
            }
        }
    }

    return true;
}

/*! see the description in header. */
NB_DamValue DAMUtilityImpl::IsDataAvailable(double latitudeTopLeft, double longitudeTopLeft,
                                            double latitudeBottomRight, double longitudeBottomRight,
                                            std::bitset<DI_TotalCount> damIndices)
{
    // if a combination of bits is asked then we return true if all bits are true, otherwise false
    int xTopLeft = -1;
    int yTopLeft = -1;
    int xBottomRight = -1;
    int yBottomRight = -1;
    ConvertLatLongToTile(latitudeTopLeft, longitudeTopLeft, xTopLeft, yTopLeft);
    ConvertLatLongToTile(latitudeBottomRight, longitudeBottomRight, xBottomRight, yBottomRight);

    return IsDataAvailable(xTopLeft, yTopLeft, xBottomRight, yBottomRight, damIndices);
}

/*! see the description in header. */
NB_DamValue DAMUtilityImpl::IsDataAvailable(const std::vector<TileKeyPtr>& tileKeys, std::bitset<DI_TotalCount> damIndices)
{
     set<TileKey> convertedTiles;
     for (vector<TileKeyPtr>::const_iterator it=tileKeys.begin(); it != tileKeys.end(); ++it)
     {
         if ((*it)->m_zoomLevel == DEFAULT_DAM_ZOOMLEVEL)
         {
             convertedTiles.insert(**it);
         }
         else
         {
            vector <TileKeyPtr> convertedKeys;
            ConvertTileKeyToOtherZoomLevel(*it, DEFAULT_DAM_ZOOMLEVEL, convertedKeys);
            vector<TileKeyPtr>::iterator iter = convertedKeys.begin();
            vector<TileKeyPtr>::iterator end  = convertedKeys.end();
            while (iter != end)
            {
                convertedTiles.insert(**iter);
                ++iter;
            }
         }
     }

    bitset<2> result;
    for (set<TileKey>::iterator it = convertedTiles.begin(); it != convertedTiles.end(); ++it)
    {
        for (size_t i = 0; i < DI_TotalCount; ++i)
        {
            if (damIndices[i])
            {
                CheckDamIndex((*it).m_x, (*it).m_y, i) ? result.set(0) : result.set(1);
            }
            if (result.count() == 2)
            {
                return NDV_Partial;
            }
        }
    }
    if (result[1])
    {
        return NDV_False;
    }
    return NDV_True;
}

/*! see the description in header. */
void DAMUtilityImpl::ConvertLatLongToTile(double latitude, double longitude, int& xTile, int& yTile)
{
    double xMercator = 0.0;
    double yMercator = 0.0;
    NB_SpatialConvertLatLongToMercator(latitude, longitude, &xMercator, &yMercator);
    NB_SpatialConvertMercatorToTile(xMercator, yMercator, DEFAULT_DAM_ZOOMLEVEL, &xTile, &yTile);
}

/*! see description in header. */
void DAMUtilityImpl::StoreDamToCache(TileKeyPtr tileKey, TilePtr tile)
{
    DAMCacheEntry* entry = &m_damCacheEntry[0];
    for (size_t i = 0; i < DAMCONTAINER_CAPACITY; ++i)
    {
        if (m_damCacheEntry[i].tileKey && *(m_damCacheEntry[i].tileKey) == *tileKey)
        {
            m_damCacheEntry[i].lastAccessTime = PAL_ClockGetUnixTime();
            return;
        }
        else if (entry->lastAccessTime > m_damCacheEntry[i].lastAccessTime)
        {
            entry = &m_damCacheEntry[i];
        }
    }

    entry->tileKey = tileKey;
    entry->lastAccessTime = PAL_ClockGetUnixTime();
    if (entry->decodeData)
    {
        m_lamProcessor->DestroyPNGData(&entry->decodeData);
        entry->decodeData = NULL;
    }
    entry->decodeData = m_lamProcessor->DecodeLamFromTile(tile);
}

NB_Context* DAMUtilityImpl::GetContext()
{
    return m_pContext;
}

bool DAMUtilityImpl::CheckDamIndex(int x, int y, int index)
{
    if (!m_layerManager || GetDAMReferenceZoomLevel() < 0)
    {
        return true;
    }

    int damReferenceZoomLevel = GetDAMReferenceZoomLevel();
    int distance = DEFAULT_DAM_ZOOMLEVEL - damReferenceZoomLevel;
    shared_ptr<TileKey> damKey(new TileKey(x >> distance, y >> distance, damReferenceZoomLevel));
    if (damKey)
    {
        DAMCacheEntry* dam = GetDamTileFromCache(damKey);

        // Here we need check the dam cache twice to confirm that
        // the dam loaded from persistent cache success.
        if (dam == NULL)
        {
            dam = GetDamTileFromCache(damKey);
        }

        if (dam == NULL)
        {
            //DAM SHALL be considered an optimization layer in the sense that if there is a problem with downloading a DAM tile, the assumption SHALL be made that this area contains all necessary data.
            return true;
        }
        else
        {
            // The bit mapping should start from the LSB. But mobius had set from MSB, it means Bit0 of each pixel indicate the highest bit of pixel
            // So here we need to convert the index from lowest bit to highest bit.
            // @TODO: SDS is going to updated, and this part of code should be updated too.
            bool result = dam->decodeData ? m_lamProcessor->CheckLamIndex(dam->decodeData, x, y, (dam->decodeData->m_bitDepth * BITS_PER_BYTE - index - 1)) : true;

            // Is QA logging enabled
            if (NB_ContextGetQaLog(m_pContext))
            {
                NB_QaLogDAMCheckResult(m_pContext, x, y, (uint8)index, (nb_boolean)result);
            }

            return result;
        }
    }
    return true;
}

/*! see description in header. */
DAMCacheEntry* DAMUtilityImpl::GetDamTileFromCache(TileKeyPtr tileKey)
{
    for (int i = 0; i < DAMCONTAINER_CAPACITY; ++i)
    {
        if (m_damCacheEntry[i].tileKey && (*tileKey == *(m_damCacheEntry[i].tileKey)))
        {
            return &m_damCacheEntry[i];
        }
    }

    if (m_layerManager)
    {
        vector<TileKeyPtr> tileKeys;
        tileKeys.push_back(tileKey);
        UnifiedLayerManager* layerManager =
            static_cast<UnifiedLayerManager*>(m_layerManager.get());
        if (!layerManager)
        {
            return NULL;
        }

        layerManager->GetDAMTiles(tileKeys, m_pDamTileRequestcallback);
    }

    return NULL;
}

/*! see description in header. */
int DAMUtilityImpl::GetDAMReferenceZoomLevel()
{
    if (m_layerManager)
    {
        UnifiedLayerManager* layerManager =
            static_cast<UnifiedLayerManager*>(m_layerManager.get());
        if (layerManager)
        {
            LayerPtr damLayer = layerManager->GetLayerProvider()->GetDAMLayer();
            if (damLayer)
            {
                return damLayer->GetReferenceTileGridLevel();
            }
        }
    }

    return -1;
}

/*! see description in header. */
void DAMUtilityImpl::UnifiedLayerUpdated()
{
    for (int i = 0; i < DAMCONTAINER_CAPACITY; ++i)
    {
        m_damCacheEntry[i].tileKey.reset();
        m_damCacheEntry[i].lastAccessTime = 0;
        if (m_damCacheEntry[i].decodeData)
        {
            m_lamProcessor->DestroyPNGData(&m_damCacheEntry[i].decodeData);
            m_damCacheEntry[i].decodeData = NULL;
        }
    }
}

/*! see description in header. */
NB_DamValue DAMUtilityImpl::IsDataAvailable(int xTopLeft, int yTopLeft, int xBottomRight, int yBottomRight,
                                            std::bitset<DI_TotalCount> damIndices)
{
    // bit[0] indicate true
    // bit[1] indicate false
    bitset<2> result;
    int xMax = max(xTopLeft, xBottomRight);
    int xMin = min(xTopLeft, xBottomRight);
    int yMax = max(yTopLeft, yBottomRight);
    int yMin = min(yTopLeft, yBottomRight);
    if (xTopLeft > xBottomRight)
    {
        // The bounding box now cross the boundary of the map.
        // The boundary divide the boundingbox to two part.
        int xMaxBoundary = static_cast<int>(pow(2.0, DEFAULT_DAM_ZOOMLEVEL) - 1.0);
        for (int y = yMin; y <= yMax; ++y)
        {
            for (int x = 0; x <= xMin; ++x)
            {
                for (int i = 0; i < DI_TotalCount; ++i)
                {
                    if (damIndices[i])
                    {
                        CheckDamIndex(x, y, i) ? result.set(0) : result.set(1);
                    }
                    if (result.count() == 2)
                    {
                        return NDV_Partial;
                    }
                }
            }
            for (int x = xMax; x <= xMaxBoundary; ++x)
            {
                for (int i = 0; i < DI_TotalCount; ++i)
                {
                    if (damIndices[i])
                    {
                        CheckDamIndex(x, y, i) ? result.set(0) : result.set(1);
                    }
                    if (result.count() == 2)
                    {
                        return NDV_Partial;
                    }
                }
            }
        }
    }
    else
    {
        for (int x = xMin; x <= xMax; ++x)
        {
            for (int y = yMin; y <= yMax; ++y)
            {
                for (int i = 0; i < DI_TotalCount; ++i)
                {
                    if (damIndices[i])
                    {
                        CheckDamIndex(x, y, i) ? result.set(0) : result.set(1);
                    }
                    if (result.count() == 2)
                    {
                        return NDV_Partial;
                    }
                }
            }
        }
    }
    if (result[1])
    {
        return NDV_False;
    }
    return NDV_True;
}

/*! see description in header. */
DamTileRequestCallback::DamTileRequestCallback(DAMUtilityImpl* damUtility)
    : m_damUtility(damUtility)
{
}

/*! see description in header. */
void DamTileRequestCallback::Success(TileKeyPtr request, TilePtr response)
{
    if (request && response)
    {
        m_damUtility->StoreDamToCache(request, response);
    }
}

/*! see description in header. */
void DamTileRequestCallback::Error(TileKeyPtr /*request*/, NB_Error /*error*/)
{
}

/*! @} */
