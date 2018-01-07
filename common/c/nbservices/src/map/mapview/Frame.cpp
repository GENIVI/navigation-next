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
    @file     Frame.cpp
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#include "Frame.h"
#include "NBGMViewController.h"
#include "NBUIConfig.h"
#include <math.h>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <iostream>
#include "TileKeyUtils.h"

extern "C"
{
#include "spatialvector.h"
#include "palmath.h"
}

using namespace nbmap;


// #define LOG_VERBOSE

// Tile size in pixel. @todo: Should be taken from configuration
//const int TILE_SIZE = 256;

/*
    We keep the raster tiles near the current zoom level for better visual appearance
 */
const int RASTER_NEAR_ZOOM_HIGH_BORDER = 1;     // Zoom levels above (higher number) the current zoom level
const int RASTER_NEAR_ZOOM_LOW_BORDER  = 2;     // Zoom levels below (lower number) the current zoom level


// Functor classes ..............................................................................

/*! Functor to return all TilePtr's which do NOT match a specific layer-ID. This negative-predicate is used for remove_copy_if(). */
struct TilePtrNotLayerIDFunctor
{
    TilePtrNotLayerIDFunctor(uint32 layerID) : m_layerID(layerID){};

    bool operator()(const TilePtr& tile)
    {
        return (tile->GetLayerID() != m_layerID);
    }

private:
    uint32 m_layerID;
};

/*! Functor to print out the content-ID for a TilePtr */
struct TilePtrPrintoutFunctor
{
    void operator()(const TilePtr& tile)
    {
        cout << "Tile: " << tile->GetContentID()->c_str() << endl;
    }
};

/*! Functor class to add TileKey shared pointers to a vector.

    The input is a set of TileKey's (not having shared pointers)

    This class can be passed to the std::for_each() algorithm.
*/
class SetToVectorFunctor
{
public:

    SetToVectorFunctor() : m_pOutputVector(NULL){};
    SetToVectorFunctor(vector<TileKeyPtr>* pOutputVector) : m_pOutputVector(pOutputVector){};

    /*! This operator gets called for every element */
    void operator()(const TileKey& tile)
    {
        // Create a new shared pointer of a TileKey and add it to the output vector
        m_pOutputVector->push_back(shared_ptr<TileKey>(new TileKey(tile)));
    }

private:
    vector<TileKeyPtr>* m_pOutputVector;
};

/*! Functor class used to sort the tile vector based on the distance of the tile to the center of the map.

    We want to prioritize the tiles closer to the center. This is used in the std::sort function.
*/
class SortVectorMyCenter : public binary_function<TileKeyPtr, TileKeyPtr, bool>
{
public:
    SortVectorMyCenter(){};
    SortVectorMyCenter(const Point& center) : m_center(center) {};

    /*! Compare operator used to compare tiles */
    bool operator()(const TileKeyPtr& leftSide, const TileKeyPtr& rightSide) const
    {
        /*
            Calculate the distance between the center of the map and the center of the two tiles.
            Compare the tiles based on that distance. Lowest distance from map center will result in the highest download priority.
         */

        /*
            The problem with this logic is that the distance for a tile from the map center will be calculated over and over again,
            since each tile will be compared multiple times during the sort() algorithm. To improve this, we could pre-calculate
            the distance and make a wrapper class which we would use for the sort() algorithm. But the downside is that we would
            have to convert the TileKey's into that wrapper class and vice versa. For now this should suffice.
         */

        double leftX = static_cast<double>(m_center.x - leftSide->m_x);
        double leftY = static_cast<double>(m_center.y - leftSide->m_y);
        double rightX = static_cast<double>(m_center.x - rightSide->m_x);
        double rightY = static_cast<double>(m_center.y - rightSide->m_y);

        double leftDistance = sqrt((leftX * leftX) + (leftY * leftY));
        double rightDistance = sqrt((rightX * rightX) + (rightY * rightY));

//        // !!!! test
//        static int count = 0;
//        printf("Map center[%d]: %d, %d, left Tile: %d, %d, right Tile: %d, %d, Left distance: %f, right distance: %f\r\n",
//               ++count,
//               m_center.x,
//               m_center.y,
//               leftSide->m_x,
//               leftSide->m_y,
//               rightSide->m_x,
//               rightSide->m_y,
//               leftDistance,
//               rightDistance);

        return leftDistance < rightDistance;
    }

private:

    // Map center
    Point m_center;
};


// TEST
class DumpVector
{
public:

    DumpVector(){};
    DumpVector(const Point& center) : m_center(center){};

    /*! This operator gets called for every element */
    void operator()(const TileKeyPtr& tile)
    {
        double tileX = static_cast <double>(m_center.x - tile->m_x);
        double tileY = static_cast <double>(m_center.y - tile->m_y);

        double leftDistance = sqrt((tileX * tileX) + (tileY * tileY));

        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelDebug, "Distance: %f, Tile: %d, %d, %d", leftDistance, tile->m_x, tile->m_y, tile->m_zoomLevel);
    }

private:
    Point m_center;
};
// END TEST


// TileInfo .............................................................................................................................

TileInfo::TileInfo()
    : m_animationTimestamp(0)
{
}

TileInfo::TileInfo(TilePtr tile)
    : m_tile(tile),
      m_key(*tile->GetTileKey()),
      m_animationTimestamp(tile->GetTimeStamp())
{
}

TileInfo::~TileInfo()
{
}

/* See header for description */
bool
TileInfo::operator<(const TileInfo& rightSide) const
{
    // Don't call the TileKey operator here, we need to also check for the timestamp, and since we check for equivalence (and not equality)
    // we can't call the TileKey operator here.

    // zoom
    if (m_key.m_zoomLevel < rightSide.m_key.m_zoomLevel)
    {
        return true;
    }
    else if (m_key.m_zoomLevel > rightSide.m_key.m_zoomLevel)
    {
        return false;
    }

    // X
    if (m_key.m_x < rightSide.m_key.m_x)
    {
        return true;
    }
    else if (m_key.m_x > rightSide.m_key.m_x)
    {
        return false;
    }

    // Y
    if (m_key.m_y < rightSide.m_key.m_y)
    {
        return true;
    }
    else if (m_key.m_y > rightSide.m_key.m_y)
    {
        return false;
    }

    // timestamp
    if (m_animationTimestamp < rightSide.m_animationTimestamp)
    {
        return true;
    }
    else if (m_animationTimestamp > rightSide.m_animationTimestamp)
    {
        return false;
    }

    // The value is NOT less
    return false;
}


// Frame class .............................................................................

// General Rules:
//   1. m_receivedTiles should always be called in CCC thread, and no lock is needed.
//   2. m_layers should always be called in CCC thread, no lock is needed.
//   3. m_currentTiles should always be called in CCC thread, no lock is needed.
//   4. m_center will be called in UI and Rander thread, so lock is needed, but less of  the locks  would be better.
//   5. m_frustum will be called in UI and Rander thread, so lock is needed, but less of  the locks  would be better.
//   6. m_currentZoom will be called in UI and Rander thread, so lock is needed, but less of  the locks  would be better.
Frame::Frame()
    : Base(),
      m_lock(NULL),
      m_currentZoom(0.0),
      m_needsUpdate(true),                  // Update frame/tiles at the beginning
      m_zoomHasChanged(false),
      m_currentTilesUpdated(false),
      m_minRasterZoomLevel(0),
      m_maxRasterZoomLevel(0),
      m_profiling(false),                   // Important: Set to 'false' for production code! Only used for testing!
      m_frustumOverlapping(false),
      m_isFrustumValid(false)
{
    nsl_memset(m_frustum, 0, sizeof(m_frustum));
    nsl_memset(m_frustumRasterNear, 0, sizeof(m_frustumRasterNear));
}

Frame::~Frame()
{
    if (m_lock)
    {
        PAL_LockDestroy(m_lock);
    }
}

// Public Functions ....................................................................................................

bool
Frame::Initialize(PAL_Instance* palInstance)
{
    return (PAL_LockCreate(palInstance, &m_lock) == PAL_Ok);
}

/* See header for description */
void
Frame::SetLayers(const vector<LayerPtr>& layers)
{
    m_layers.clear();

    // Copy all the layers to our map. The map will be sorted by the layer-ID
    for (vector<LayerPtr>::const_iterator iterator = layers.begin();
         iterator != layers.end(); ++iterator)
    {
    	// Print out Layers received
    	static int count = 0;
    	NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "Frame::Layer Received[%d]: Type: %s", ++count, (*iterator)->GetTileDataType()->c_str());

        m_layers.insert(make_pair((*iterator)->GetID(), *iterator));
    }
}

void
Frame::SetFrustumValid(bool status)
{
    m_isFrustumValid = status;
}

/* See header for description */
bool
Frame::SetZoomLevelAndFrustum(double newZoom,
                              const vector<NBGM_Point2d64>& frustum,
                              const vector<NBGM_Point2d64>& frustumRasterNear)
{
    bool frustumChanged = false;
    bool zoomChanged = SetZoom(newZoom);

    if (!frustum.empty() && !frustumRasterNear.empty())
    {
        frustumChanged = SetFrustum(frustum, frustumRasterNear);
    }

    return zoomChanged | frustumChanged;
}

/* See header for description */
bool
Frame::SetFrustum(const vector<NBGM_Point2d64>& frustum,
                  const vector<NBGM_Point2d64>& frustumRasterNear)
{
    // All frustums have to have 4 points/corners
    const int FRUSTUM_SIZE = 4;

    // Validate input
    if ((frustum.size() != FRUSTUM_SIZE) || (frustumRasterNear.size() != FRUSTUM_SIZE))
    {
        nsl_assert(FALSE);
        return false;
    }

    // Copy frustum
    /*
        @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
        we can remove all locks. Until then ALL public interfaces have to be locked!
     */

    bool modified = false;
    Lock lock(m_lock);
    for (size_t i = 0; i < FRUSTUM_SIZE; ++i)
    {
        SetVector(frustum[i], m_frustum[i], modified);
        SetVector(frustumRasterNear[i], m_frustumRasterNear[i], modified);
    }

    if (modified)
    {
        SetFrustumOverlapping();
        m_needsUpdate = true;
    }

    return modified;
}

/* See header for description */
void
Frame::SetCenter(Point newCenter)
{
    /*
        @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
        we can remove all locks. Until then ALL public interfaces have to be locked!
     */
    Lock lock(m_lock);

    if ( ! (m_center == newCenter))
    {
        m_center = newCenter;
        m_needsUpdate = true;
    }
}

/* See header for description */
bool
Frame::SetZoom(double newZoom)
{
    /*
        @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
        we can remove all locks. Until then ALL public interfaces have to be locked!
     */
    Lock lock(m_lock);

    if (m_currentZoom != newZoom)
    {
        /*
            Only set the zoom-has-changed flag if the zoom switched to the next integer zoom level. Ignore small changes.
         */
        if ((uint32)m_currentZoom != (uint32)newZoom)
        {
            m_zoomHasChanged = true;
        }

        m_currentZoom = newZoom;
        m_needsUpdate = true;
        return true;
    }

    return false;
}

double
Frame::GetZoom() const
{
    /*
        @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
        we can remove all locks. Until then ALL public interfaces have to be locked!
     */
    Lock lock(m_lock);
    return m_currentZoom;
}

/* See header for description */
bool
Frame::UpdateTiles(vector<TileKeyPtr>& deltaTiles, vector<TileKeyPtr>& deltaTilesNonOverlay)
{
    if(!m_isFrustumValid)
    {
        return false;
    }

    int currentZoomInInt = 0;
    {
        /*
            @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
            we can remove all locks. Until then ALL public interfaces have to be locked!
         */
        Lock lock(m_lock);
        currentZoomInInt = (int)m_currentZoom;
    }

    NB_Vector frustum[4] = {{0}};
    {
        /*
            @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
            we can remove all locks. Until then ALL public interfaces have to be locked!
         */
        Lock lock(m_lock);
        for (size_t i = 0; i < 4; ++i)
        {
            frustum[i].x =  m_frustum[i].x;
            frustum[i].y =  m_frustum[i].y;
            frustum[i].z =  m_frustum[i].z;
        }
    }

    if ((currentZoomInInt == 0) || (! m_needsUpdate))
    {
        return false;
    }
    m_needsUpdate = false;

    if (RasterMode())
    {
        // Update both the raster tile levels. Get the higher zoom tiles first

        int higherZoom = currentZoomInInt - HIGHER_ZOOM_LEVEL_DIFFERENCE;
        if (higherZoom >= NBUIConfig::getMinZoom())
        {
            // The tiles for the higher zoom level should be made excluding the overlay layers (route, traffic, animation, ...)
            UpdateTilesFrustum(deltaTilesNonOverlay, frustum, higherZoom, m_currentTilesHigherZoomLevel);
        }
        UpdateTilesFrustum(deltaTiles, m_frustumRasterNear, currentZoomInInt, m_currentTiles);
    }
    else
    {
        // Vector mode
        UpdateTilesFrustum(deltaTiles, frustum, currentZoomInInt, m_currentTiles);
    }

#ifdef LOG_VERBOSE
    // !!!! TEST
    if (! deltaTiles.empty())
    {
        static int count = 0;
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelDebug, "UpdateTiles[%d]: Tiles to request: ", ++count);

        DumpVector dump(m_center);
        for_each(deltaTiles.begin(), deltaTiles.end(), dump);
    }
    if (! deltaTilesNonOverlay.empty())
    {
        static int count = 0;
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelDebug, "UpdateTiles[%d]: Tiles to request NON-OVERLAY: ", ++count);

        Point adjustedCenter(m_center);
        adjustedCenter.x >>= HIGHER_ZOOM_LEVEL_DIFFERENCE;
        adjustedCenter.y >>= HIGHER_ZOOM_LEVEL_DIFFERENCE;

        DumpVector dump(adjustedCenter);
        for_each(deltaTilesNonOverlay.begin(), deltaTilesNonOverlay.end(), dump);
    }
    // end TEST !!!!
#endif

    // Retun true to let MapView start download tiles if any of deltaTiles or
    // deltaTilesNonOverlay is not empty.
    return (!deltaTiles.empty() || !deltaTilesNonOverlay.empty());
}

/* See header for description */
bool
Frame::TileReceived(TilePtr tile, vector<TilePtr>& tilesToUnload)
{
    bool status = true;
    do
    {
        /*
            This is being called from the CCC thread
         */

        TileKeyPtr key = tile->GetTileKey();

    #ifdef LOG_VERBOSE
        static int count = 0;
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "Tile received[%d]: %s, Timestamp: %d", ++count, tile->GetContentID()->c_str(), tile->GetTimeStamp());
    #endif

        // Not sure what the best way is to ignore material files. They all have -1, -1, -1 as keys.
        if ((key->m_x == -1) && (key->m_y == -1) && (key->m_zoomLevel == -1))
        {
            break;
        }

        // Check if the tile is still needed
        if (! IsTileStillNeeded(tile))
        {
            status = false;
            break;
        }

        /*
            Check if the tile already exists. Put it in a layer bucket to keep track of it.
         */

        uint32 layerID = tile->GetLayerID();
        TileInfo info(tile);

        // Check if we already have a layer-bucket with that layer-ID
        BUCKET_MAP::iterator layerBucket = m_receivedTiles.find(layerID);
        if (layerBucket == m_receivedTiles.end())
        {
            LAYER_MAP::const_iterator layer = m_layers.find(layerID);
            if (layer == m_layers.end())
            {
                NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "ERROR: Can't find layer for type: %s, ID: %u!", tile->GetDataType()->c_str(), tile->GetLayerID());
                status = false;
                break;
            }

            // The bucket does not yet exist, create it
            LayerBucket newBucket(layer->second->GetMinZoom(),
                                  layer->second->GetMaxZoom(),
                                  layer->second->GetReferenceTileGridLevel(),
                                  layer->second->IsRasterLayer());

            // add the bucket to our bucket list
            layerBucket = m_receivedTiles.insert(m_receivedTiles.begin(), make_pair(layerID, newBucket));
        }

        /* If the reference tile grid zoom level is invalid, only the tiles of the current zoom level
           should be displayed.
        */
        {
            /*
                @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
                we can remove all locks. Until then ALL public interfaces have to be locked!
             */
            Lock lock(m_lock);
            if (layerBucket->second.m_isRasterLayer &&
                (floor(m_currentZoom) != key->m_zoomLevel))
            {
                // If this is not vector layer, raster tiles should only be loaded when the
                // zoomLevel of this Tile is the same as current zoom level.
                status = false;
                break;
            }
        }

        /*
             We check if we have the tile already. The old code accepted the tile even if we already
             had it, since NBGM discards duplicate tiles. But this is a huge overhead, especially since we now request
             cached tiles first.
             Also, we do retry tiles, if they fail to load into NBGM, so I think there is no reason to accept duplicate
             tiles anymore.
         */
        if (layerBucket->second.m_tiles.find(info) == layerBucket->second.m_tiles.end())
        {
            layerBucket->second.m_tiles.insert(info);
        }
        else
        {
            static int count = 0;
            NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "Received duplicate tile, ignore[%d]: %s", ++count, tile->GetContentID()->c_str());
            status = false;
        }

        //@todo: I don't think this code is useful, check again and remove it if so.
        // Check if we need to unload any raster tiles in response to the newly received tile. We unload raster tiles if they are now covered by the new tile!
        set<TileInfo> receivedInfo;
        receivedInfo.insert(tile);
        CheckRasterTilesToUnload(receivedInfo, tilesToUnload);
    }while(0);

    return status;
}

/* See header for description */
bool
Frame::CheckForTilesToUnload(vector<TilePtr>& tiles)
{
    /*
        @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
        we can remove all locks. Until then ALL public interfaces have to be locked!
     */
    // Check for tiles to be unloaded based on zooming or panning
    CheckZoomTilesToUnload(tiles);
    CheckPanningTilesToUnload(tiles);

    return ! tiles.empty();
}

/* See header for description */
void
Frame::ForceCleanCurrentFrame()
{
    m_needsUpdate  = true;
    m_currentTiles.clear();
    m_currentTilesHigherZoomLevel.clear();
}

/* See header for description */
void
Frame::SetRasterZoomRange(uint32 minZoomLevel, uint32 maxZoomLevel)
{
    /*
        @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
        we can remove all locks. Until then ALL public interfaces have to be locked!
     */

    m_minRasterZoomLevel = minZoomLevel;
    m_maxRasterZoomLevel = maxZoomLevel;
}

/* See header file for description */
void
Frame::UnloadAllTiles(vector<TilePtr>& unloadedTiles)
{
    // Add all received tiles to return.
    BUCKET_MAP::iterator layerIterator = m_receivedTiles.begin();
    BUCKET_MAP::iterator layerEnd = m_receivedTiles.end();
    for (; layerIterator != layerEnd; ++layerIterator)
    {
        set<TileInfo>& tilesOfLayer = layerIterator->second.m_tiles;

        set<TileInfo>::iterator tileIterator = tilesOfLayer.begin();
        set<TileInfo>::iterator tileEnd = tilesOfLayer.end();
        for (; tileIterator != tileEnd; ++tileIterator)
        {
            TilePtr tile = tileIterator->m_tile;
            if (!tile)
            {
                continue;
            }
            unloadedTiles.push_back(tile);
        }
    }

    // Clear all received tiles.
    m_receivedTiles.clear();
}

/* See header file for description */
void
Frame::UnloadTilesOfLayer(uint32 layerId,
                          vector<TilePtr>& unloadedTiles)
{
    // Find received tiles by layer ID.
    BUCKET_MAP::iterator layerIterator = m_receivedTiles.find(layerId);
    if (layerIterator != m_receivedTiles.end())
    {
        // Add found received tiles to return.
        set<TileInfo>& tilesOfLayer = layerIterator->second.m_tiles;

        set<TileInfo>::iterator tileIterator = tilesOfLayer.begin();
        set<TileInfo>::iterator tileEnd = tilesOfLayer.end();
        for (; tileIterator != tileEnd; ++tileIterator)
        {
            TilePtr tile = tileIterator->m_tile;
            if (!tile)
            {
                continue;
            }

            unloadedTiles.push_back(tile);
        }

        // Remove received tiles of this layer.
        m_receivedTiles.erase(layerIterator);
    }
}

/* See header file for description */
vector<TilePtr>
Frame::UnloadTilesFromFrame(const vector<TileKeyPtr>& tileKeys,
                            uint32 layerId)
{
    vector<TilePtr> tiles;

    /*
      @todo: We should ensure that the Frame class always gets called from the same thread.
      Once that's ensured then we can remove all locks. Until then ALL public interfaces
      have to be locked!
    */
    Lock lock(m_lock);

    /*
      This logic doesn't work for animation tiles, it would only unload the first tile with
      that key. But I don't think this gets called for animation tiles, since they get
      refreshed when the frame list changes.
    */

    // Find received tiles by layer ID.
    BUCKET_MAP::iterator layerIterator = m_receivedTiles.find(layerId);

    if (layerIterator == m_receivedTiles.end() || tileKeys.empty())
    {
        return tiles;
    }

    LayerBucket&       bucket = layerIterator->second;
    vector<TileKeyPtr> keys   = tileKeys;

    // Convert tileKeys to TileKey of ref-tile-grid-level if necessary.
    if (!bucket.m_isRasterLayer)
    {
        set<TileKey> convertedKeys;
        vector<TileKeyPtr>::const_iterator iter = tileKeys.begin();
        vector<TileKeyPtr>::const_iterator end  = tileKeys.end();
        for (; iter != end; ++iter)
        {
            if ((uint32)(*iter)->m_zoomLevel >= bucket.m_minZoomLevel &&
                (uint32)(*iter)->m_zoomLevel <= bucket.m_maxZoomLevel)
            {
                ConvertTileKeyToOtherZoomLevel(*iter, bucket.m_zoomLevel,
                                               convertedKeys);
            }
        }
        if (!convertedKeys.empty())
        {
            keys.clear();
            for_each(convertedKeys.begin(), convertedKeys.end(),
                     SetToVectorFunctor(&keys));
        }
    }

    // Find the tile by tile key.
    TileInfo tileInfo;
    vector<TileKeyPtr>::const_iterator iter = keys.begin();
    vector<TileKeyPtr>::const_iterator end  = keys.end();
    for (; iter != end; ++iter)
    {
        const TileKeyPtr& tileKey = *iter;
        if (!tileKey)
        {
            continue;
        }

        tileInfo.m_key = *tileKey;
        set<TileInfo>& tilesOfLayer = layerIterator->second.m_tiles;
        set<TileInfo>::iterator tileIterator = tilesOfLayer.find(tileInfo);
        if (tileIterator != tilesOfLayer.end())
        {
            tiles.push_back(tileIterator->m_tile);
            tilesOfLayer.erase(tileIterator);
        }
    }
    return tiles;
}

/* See header file for description */
void
Frame::GetCurrentTiles(vector<TileKeyPtr>& tiles) const
{
    /* Create vector of TileKeyPtr's from our set of current tiles */
    SetToVectorFunctor functor(&tiles);
    for_each(m_currentTiles.begin(), m_currentTiles.end(), functor);
}


/* See header file for description */
bool Frame::LayerHasTileLoaded(uint32 layerId)
{
    bool result = false;
    BUCKET_MAP::iterator layerBucket = m_receivedTiles.find(layerId);
    if (layerBucket != m_receivedTiles.end())
    {
        result = !layerBucket->second.m_tiles.empty();
    }

    return result;
}

// Animation layer functions ..........................................................................................

/*
    These funcitons have to be called in the context of the render thread in order to avoid synchronization issues.
 */

/* See header file for description */
void
Frame::AddAnimationLayer(uint32 layerID)
{
    // This gets called from the render thread.
    // No lock necessary

    // Create new animation layer and add it to our map
    shared_ptr<MapViewAnimationLayer> newLayer(new MapViewAnimationLayer);
    if (newLayer)
    {
        // Set to currently set animation parameters. The application can update it at any time.
        newLayer->SetAnimationParameters(m_animationLayerParameters);
        m_animationLayers.insert(make_pair(layerID, newLayer));
    }
}

/* See header file for description */
void
Frame::RemoveAnimationLayer(uint32 layerID)
{
    // This gets called from the render thread.
    // No lock necessary

    /*
        The tiles for that layer get removed by UnloadTilesOfLayer(). All we have to do here is to remove the actual layer object
     */

    // Remove the animation layer from our map. That will delete any associated data with it
    m_animationLayers.erase(layerID);
}

/* See header file for description */
void
Frame::UpdateAnimationFrameList(uint32 layerID,
                                const vector<uint32>& sortedFrameList,
                                vector<shared_ptr<string> >& tilesToUnload)
{
    // This gets called from the render thread.
    // No lock necessary

    if (sortedFrameList.empty())
    {
        return;
    }

    ANIMATION_MAP::iterator layer = m_animationLayers.find(layerID);
    if (layer != m_animationLayers.end())
    {
        // Update the animation layer
        layer->second->UpdateFrameList(sortedFrameList, tilesToUnload);
    }
}

/* See header file for description */
bool
Frame::HasAnimationLayers() const
{
    // This gets called from the render thread.
    // No lock necessary

    return !m_animationLayers.empty();
}

/* See header file for description */
bool
Frame::HasAnimationTiles() const
{
    /* Check all the layers if they have any tiles */

    ANIMATION_MAP::const_iterator layerIterator = m_animationLayers.begin();
    for (; layerIterator != m_animationLayers.end(); ++layerIterator)
    {
        if (layerIterator->second->HasAnyTiles())
        {
            return true;
        }
    }
    return false;
}

/* See header file for description */
bool
Frame::GetAnimationFrameData(int interval, map<uint32, ANIMATION_DATA>& data, uint32& timestamp, bool& animationCanPlay)
{
    // This gets called from the render thread.
    // No lock necessary

    bool result = false;

    // Get all the frame data from all animation layers (most likely it will only be one)
    ANIMATION_MAP::iterator end = m_animationLayers.end();
    for (ANIMATION_MAP::iterator layerIterator = m_animationLayers.begin(); layerIterator != end; ++layerIterator)
    {
        MapViewAnimationLayer& layer = *layerIterator->second;

        // Update the timestamp and check if we have new data
        if (layer.HasNewFrameData(interval))
        {
            // Insert an empty pair for that layer id
            ANIMATION_DATA newPair = make_pair(0, vector<shared_ptr<string> >());
            pair<map<uint32, ANIMATION_DATA>::iterator, bool> newEntry = data.insert(make_pair(layerIterator->first, newPair));

            // Call that layer to fill in the frame data
            if (layer.GetFrameData(newEntry.first->second, timestamp, animationCanPlay))
            {
                result = true;
            }
        }
    }

    return result;
}

/* See header file for description */
void
Frame::AnimationTileReceived(TilePtr tile)
{
    // This gets called from the render thread.
    // No lock necessary

    ANIMATION_MAP::iterator layer = m_animationLayers.find(tile->GetLayerID());
    if (layer != m_animationLayers.end())
    {
        layer->second->TileReceived(tile);
    }
}

/* See header file for description */
void
Frame::AnimationTilesRemoved(const vector<TilePtr>& tiles)
{
    // This gets called from the render thread.
    // No lock necessary

    if (m_animationLayers.empty())
    {
        return;
    }

    /*
        Go through all animation layers (most likely one) and remove the tiles for those layers.
     */

    ANIMATION_MAP::iterator end = m_animationLayers.end();
    for (ANIMATION_MAP::iterator layerIterator = m_animationLayers.begin(); layerIterator != end; ++layerIterator)
    {
        /* Filter out all tiles which match the layer-ID of the animation layer

            We can't use "copy_if" because it was only introduced with C++11
         */
        vector<TilePtr> animationTiles;
        remove_copy_if(tiles.begin(), tiles.end(), back_inserter(animationTiles), TilePtrNotLayerIDFunctor(m_animationLayers.begin()->first));

        if (! animationTiles.empty())
        {
            // Remove the tiles from the animation layer. This doesn't unload any tiles, it just removes them from the internal lists
            layerIterator->second->TilesRemoved(animationTiles);
        }
    }
}

/* See header file for description */
void
Frame::SetAnimationParameters(const AnimationLayerParameters& parameters)
{
    // This gets called from the render thread.
    // No lock necessary
    // Update parameters for current animation layers
    ANIMATION_MAP::iterator end = m_animationLayers.end();
    for (ANIMATION_MAP::iterator layerIterator = m_animationLayers.begin(); layerIterator != end; ++layerIterator)
    {
        layerIterator->second->SetAnimationParameters(parameters);
    }
}

// Private Functions ...................................................................................................

/*! Check if the given tile is still needed for drawing.

    @return 'true' if the tile is still needed, 'false' otherwise
*/
bool
Frame::IsTileStillNeeded(TilePtr tile) const
{
    uint32 currentZoomInInt = 0;
    {
        /*
            @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
            we can remove all locks. Until then ALL public interfaces have to be locked!
         */
        Lock lock(m_lock);
        currentZoomInInt = (uint32)m_currentZoom;
    }
    const TileKey& key(*tile->GetTileKey());

    // Find the layer based on the layer-ID of the tile. We should always find it.
    LAYER_MAP::const_iterator layer = m_layers.find(tile->GetLayerID());
    if (layer == m_layers.end())
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "ERROR: Can't find layer for type: %s, ID: %u!", tile->GetDataType()->c_str(), tile->GetLayerID());
        return false;
    }

    if(!layer->second->IsEnabled())
    {
        return false;
    }

    // Check if our current zoom level is within the zoom level range of the given tile. If
    // it is not then the tile is no longer needed
    if ((currentZoomInInt < layer->second->GetMinZoom()) ||
        (currentZoomInInt > layer->second->GetMaxZoom()))
    {
        // Make exception for raster tiles. Since we request higher zoom level tiles we have
        // to allow them here
        if (RasterMode() && (key.m_zoomLevel == (int)(currentZoomInInt - HIGHER_ZOOM_LEVEL_DIFFERENCE)))
        {
            // The tile is a raster tile and it is one of our "higher zoom level" tiles. Accept it.
            return true;
        }

        return false;
    }

    // If the returned tile has the same zoom level as our current zoom level then we just
    // need to check if we can find the tile in our current tile list
    if (key.m_zoomLevel == (int)currentZoomInInt)
    {
        if (m_currentTiles.find(key) == m_currentTiles.end())
        {
            // Tile is not in the current frame, discard it
            return false;
        }
    }

    // Check if the tile is inside the frustum
    return IsTileIntersectWithFrustum(key);
}

/*! Check if we need to unload any tiles in response to a zoom change

    Subfunction of CheckForTilesToUnload()

    @see CheckForTilesToUnload
*/
void
Frame::CheckZoomTilesToUnload(vector<TilePtr>& tiles       /*!< In/Out: On return tiles to unload */
                              )
{
    /*
        When the zoom level changes then we check if all our tile 'buckets' are still needed in the new zoom level. If not then we
        remove all those tiles from that layer.
     */
    if (! m_zoomHasChanged)
    {
        return;
    }

    m_zoomHasChanged = false;

    bool isRasterMode = RasterMode();
    uint32 currentZoomInInt = 0;
    {
        /*
            @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
            we can remove all locks. Until then ALL public interfaces have to be locked!
         */
        Lock lock(m_lock);
        currentZoomInInt = (uint32)m_currentZoom;
    }
    BUCKET_MAP::iterator layers = m_receivedTiles.begin();

    // Go through all our layers
    while (layers != m_receivedTiles.end() /* :TRICKY: Don't precalculate the end iterator since it might change during the loop */)
    {
        bool needToDeleteLayer = false;
        LayerBucket& bucket = layers->second;

        // Is it an animation layer bucket
        if ((! bucket.m_tiles.empty()) && (bucket.m_tiles.begin()->m_animationTimestamp > 0))
        {
            // I don't think there is anything to do here for animation layers.
            ++layers;
        }
        // Non-animation layer bucket
        else
        {
            /* Special handling for raster tiles to improve visual appearance. We don't want to unload
               the raster layers close to  the current zoom level.
            */
            if (isRasterMode && IsRasterBucket(bucket))
            {
                // Keep higher raster tiles
                if (! IsHigherRasterZoomBucket(bucket))
                {
                    // Only delete it if it is outside our range of layers to keep
                    if (((bucket.m_maxZoomLevel + RASTER_NEAR_ZOOM_HIGH_BORDER) < currentZoomInInt) ||
                        (((bucket.m_minZoomLevel - RASTER_NEAR_ZOOM_LOW_BORDER) > currentZoomInInt) &&
                         bucket.m_minZoomLevel != 0))
                    {
                        needToDeleteLayer = true;
                    }
                }
            }
            /* If the min/max range for this layer list is outside the current zoom level then we want to
               unload it from NBGM.
            */
            else if ((currentZoomInInt < bucket.m_minZoomLevel) || (currentZoomInInt > bucket.m_maxZoomLevel))
            {
                needToDeleteLayer = true;
            }

            if (needToDeleteLayer)
            {
                set<TileInfo>::const_iterator tilesToDelete = bucket.m_tiles.begin();
                set<TileInfo>::const_iterator tilesEnd = bucket.m_tiles.end();

                // Add all the tiles for this bucket to the unload vector
                for (; tilesToDelete != tilesEnd; ++tilesToDelete)
                {
                    tiles.push_back(tilesToDelete->m_tile);
                }

                // Remove the bucket from the layer map
                m_receivedTiles.erase(layers++);
            }
            else
            {
                /* There is no need to check the zoom levels of overlay tiles (traffic and route) with
                   current zoom level here. This logic is implemented in the function
                   CheckPanningTilesToUnload. Because current tiles are changed when current zoom
                   level is changed.
                */

                ++layers;
            }
        }
    }

    /*
        Special handling for raster tiles. We have to check if any of the current zoom level tiles get obscured through
        smaller tiles which have a higher draw order.
     */

    const LayerBucket* pCurrentZoomBucket = NULL;

    // Find the layer for the current zoom level (if we have one)
    for (layers = m_receivedTiles.begin(); layers != m_receivedTiles.end(); ++layers)
    {
        if (layers->second.m_zoomLevel == currentZoomInInt)
        {
            pCurrentZoomBucket = &(layers->second);
            break;
        }
    }

    if (pCurrentZoomBucket)
    {
        CheckRasterTilesToUnload(pCurrentZoomBucket->m_tiles, tiles);
    }
}

/*! Check if we need to unload any tiles in response to panning

    Subfunction of CheckForTilesToUnload()

    @see CheckForTilesToUnload
 */
void
Frame::CheckPanningTilesToUnload(vector<TilePtr>& tiles       /*!< In/Out: On return tiles to unload */
                                 )
{
    /*
        We currently check if a tile doesn't cover the current frustum anymore, if so then we unload it.
        We could have a more sophisticated method in the future and unload tiles more leasurely. The old raster tile map
        had that approach. It always kept some old tiles around.
     */

    // Did the tiles for the frame change
    if (! m_currentTilesUpdated)
    {
        return;
    }
    m_currentTilesUpdated = false;

    /*
        Go through all layer buckets and check if there are any obsolete tiles
     */

    BUCKET_MAP::iterator layers = m_receivedTiles.begin();
    BUCKET_MAP::const_iterator layersEnd = m_receivedTiles.end();

    for (;layers != layersEnd; ++layers)
    {
        CheckTilesToUnloadFromBucketFrustum(layers->second, tiles);
    }
}

/*! Check if we need to unload any additional raster tiles.

    Special handling for raster tiles. If we receive the tiles from the current zoom level then we want to make sure to
    unload any tiles from lower zoom levels, if they are completely obscured by the tiles of the current zoom level.

    This function also gets called during zoom to ensure that the new zoom level tiles don't get obscured by smaller tiles.

    This avoids showing any "small" raster tiles.
*/
void
Frame::CheckRasterTilesToUnload(const set<TileInfo>& tilesToCheck,      /*!< Tiles to check against. This can be either newly received tiles,
                                                                             or the tiles of the current zoom level. The zoom level for all those tiles
                                                                             has to match */
                                vector<TilePtr>& tilesToUnload          /*!< On return any tiles which need to be unloaded */
                                )
{
    // Only check when in raster mode
    if (! RasterMode())
    {
        return;
    }
    uint32 currentZoomInInt = 0;
    {
        /*
            @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
            we can remove all locks. Until then ALL public interfaces have to be locked!
         */
        Lock lock(m_lock);
        currentZoomInInt = (uint32)m_currentZoom;
    }
    // Only proceed if we receive tiles for the current zoom level. Assume all the tiles-to-check have the same zoom level
    if ((tilesToCheck.empty()) || (tilesToCheck.begin()->m_key.m_zoomLevel != (int)currentZoomInInt))
    {
        return;
    }

    // Go through all our layer lists.

    for (BUCKET_MAP::iterator layers = m_receivedTiles.begin(); layers != m_receivedTiles.end(); ++layers)
    {
        LayerBucket& bucket = layers->second;

        /* Is the bucket a raster bucket */
        if (IsRasterBucket(bucket))
        {
            // Only check zoom levels which have a higher (more zoomed in) zoom level than the current zoom level
            if (bucket.m_zoomLevel > currentZoomInInt)
            {
                uint32 zoomDifference = bucket.m_zoomLevel - currentZoomInInt;

                // For all tiles to check
                set<TileInfo>::const_iterator iteratorToCheckEnd = tilesToCheck.end();
                for (set<TileInfo>::const_iterator iteratorToCheck = tilesToCheck.begin(); iteratorToCheck != iteratorToCheckEnd; ++iteratorToCheck)
                {
                    const TileKey& keyToCheck = iteratorToCheck->m_key;

                    /*
                        Get the area which is covered by the tile-to-check. If tiles are completely covered by the
                        tile then we unload them.
                     */
                    int xMin = keyToCheck.m_x << zoomDifference;
                    int xMax = ((keyToCheck.m_x + 1) << zoomDifference) - 1;

                    int yMin = keyToCheck.m_y << zoomDifference;
                    int yMax = ((keyToCheck.m_y + 1) << zoomDifference) - 1;

                    set<TileInfo>::iterator tileInfos = bucket.m_tiles.begin();
                    set<TileInfo>::const_iterator tileEnd = bucket.m_tiles.end();

                    // Go through all tiles in this bucket
                    while (tileInfos != tileEnd)
                    {
                        const TileKey& key = tileInfos->m_key;

                        /*
                            Check if the tile is completely covered by the received tile
                         */
                        if ((key.m_x >= xMin) && (key.m_x <= xMax) && (key.m_y >= yMin) && (key.m_y <= yMax))
                        {
                            // Tile is no longer needed, add to unload vector
                            tilesToUnload.push_back(tileInfos->m_tile);

                            // Remove from bucket
                            bucket.m_tiles.erase(tileInfos++);
                        }
                        else
                        {
                            ++tileInfos;
                        }
                    }
                }
            }
        }
    }
}


/*! Check which tiles to unload from the given bucket.

 This function is only used for frustum calculation.

 @see CheckPanningTilesToUnload
 */
void
Frame::CheckTilesToUnloadFromBucketFrustum(LayerBucket& bucket,         /*!< Bucket to check */
                                           vector<TilePtr>& tiles       /*!< In/Out: On return tiles to unload */
                                           )
{
    set<TileInfo>::iterator tileInfos = bucket.m_tiles.begin();
    set<TileInfo>::const_iterator tileEnd = bucket.m_tiles.end();

    /*
        @todo: This function can be optimized for animation tiles. Animation tiles have multiple tiles with the same
               tile key but different timestamps. This function checks every frame tile if it intersects the frustum.
               It would be better to just check every tile key once for every frame.
     */

     // Go through all tiles in this bucket
    int currentZoomInInt = 0;
    {
        /*
            @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
            we can remove all locks. Until then ALL public interfaces have to be locked!
         */
        Lock lock(m_lock);
        currentZoomInInt = (int)floor(m_currentZoom);
    }
    while (tileInfos != tileEnd)
    {
        // Check if the tile does not intersect with the frustum
        if (!IsTileIntersectWithFrustum(tileInfos->m_key) ||
            (bucket.m_isRasterLayer &&
             (tileInfos->m_key.m_zoomLevel != currentZoomInInt)))
        {
            // Tile is no longer needed, add to unload vector
            tiles.push_back(tileInfos->m_tile);
            // Remove from bucket
            bucket.m_tiles.erase(tileInfos++);
        }
        else
        {
            ++tileInfos;
        }
    }
}

/*! Checks if we are currently in raster display */
bool
Frame::RasterMode() const
{
    /*
        @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
        we can remove all locks. Until then ALL public interfaces have to be locked!
     */
    Lock lock(m_lock);
    return ((unsigned)m_currentZoom >= m_minRasterZoomLevel) && ((unsigned)m_currentZoom <= m_maxRasterZoomLevel);
}

/*! Checks if the given bucket is a raster bucket

    @return 'true' if it is a raster bucket.
 */
bool
Frame::IsRasterBucket(const LayerBucket& bucket) const
{
    /* Treat bucket as RasterBucket when:
       1. Min-Zoom and Max-Zoom of a bucket is equal, and
       2. Min-Zoom and Max-Zoom of bucket belongs to [minRasterZoomLevel, maxRasterZoomLevel];
     */
    return ((bucket.m_minZoomLevel == bucket.m_maxZoomLevel) &&
            (bucket.m_minZoomLevel >= m_minRasterZoomLevel) &&
            (bucket.m_minZoomLevel <= m_maxRasterZoomLevel) &&
            (bucket.m_maxZoomLevel >= m_minRasterZoomLevel) &&
            (bucket.m_maxZoomLevel <= m_maxRasterZoomLevel));
}

/*! Check if the layer bucket is the higher zoom bucket for the raster tile layers

    @return 'true' if it is the higher raster tile zoom bucket, 'false' otherwise
 */
bool
Frame::IsHigherRasterZoomBucket(const LayerBucket& bucket) const
{
    if (IsRasterBucket(bucket))
    {
        // Is it the higher zoom level raster level
        /*
            @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
            we can remove all locks. Until then ALL public interfaces have to be locked!
         */
        Lock lock(m_lock);
        if (((uint32)m_currentZoom == (bucket.m_maxZoomLevel + HIGHER_ZOOM_LEVEL_DIFFERENCE)) &&
            ((uint32)m_currentZoom == (bucket.m_minZoomLevel + HIGHER_ZOOM_LEVEL_DIFFERENCE)))
        {
            return true;
        }
    }

    return false;
}

/*! Update tiles using frustum calculation

    Works the same as UpdateTiles() except that it uses the frustum for calculation. See description for UpdateTiles().
 */
void
Frame::UpdateTilesFrustum(vector<TileKeyPtr>& deltaTiles,   /*!< In/Out: Tiles to request */
                          const NB_Vector* frustum,         /*!< frustum to use for calculation */
                          int zoomLevel,                    /*!< Zoom level to use for calculation */
                          set<TileKey>& currentTiles        /*!< In/Out: Current tiles to work on */
                          )
{
    set<TileKey> newTiles;
    set<TileKey> deltaTilesSet;

    /*
        We first get the rectangular area which contains the frustum. Then we go through this rectangular area and check each tile
        if it intersects the frustum polygon to determine if the tile is needed to fill the frustum.
     */

    Point minTile(0x7FFFFFFF, 0x7FFFFFFF);
    Point maxTile;

    /*
        Get rectangular area based on the frustum
     */
    GetMinMaxTile(frustum[0].x, frustum[0].y, zoomLevel, minTile, maxTile);
    GetMinMaxTile(frustum[1].x, frustum[1].y, zoomLevel, minTile, maxTile);
    GetMinMaxTile(frustum[2].x, frustum[2].y, zoomLevel, minTile, maxTile);
    GetMinMaxTile(frustum[3].x, frustum[3].y, zoomLevel, minTile, maxTile);

    /*
      Now go through the rectangular area and check which tile actually intersects with the
      frustum.

      Additional check is added to ensure the generated TileKey is valid.
     */

    int maxValue = static_cast<int>(nsl_pow(2, zoomLevel));
    for (int x = minTile.x; x <= maxTile.x; ++x)
    {
        for (int y = minTile.y; y <= maxTile.y; ++y)
        {
            // Convert tile corners to mercator coordinates
            NB_Vector tileCorners[4] = {{0}};
            ConvertTileToMercator(TileKey(x, y, zoomLevel), tileCorners);

            // Check if the tile intersects with the frustum
            if (vector_polygons_intersect(tileCorners, 4, TRUE, frustum, 4, FALSE))
            {
                if (m_frustumOverlapping)
                {
                    // Wrap around logic
                    int xTile(x);
                    int yTile(y);

                    while (xTile < 0)
                    {
                        xTile += maxValue;
                    }
                    while (yTile < 0)
                    {
                        yTile += maxValue;
                    }
                    while (xTile >= maxValue)
                    {
                        xTile -= maxValue;
                    }
                    while (yTile >= maxValue)
                    {
                        yTile -= maxValue;
                    }

                    newTiles.insert(TileKey(xTile, yTile, zoomLevel));
                }
                else
                {
                    // Only add it if it is in range
                    if ((x >= 0) && (x < maxValue) && (y >= 0) && (y < maxValue))
                    {
                        newTiles.insert(TileKey(x, y, zoomLevel));
                    }
                }
            }
        }
    }

    // Calculate difference between last and current frame
    set_difference(newTiles.begin(), newTiles.end(),
                   currentTiles.begin(), currentTiles.end(),
                   inserter(deltaTilesSet, deltaTilesSet.end()));

    if (!deltaTilesSet.empty() || currentTiles.size() != newTiles.size())
    {
        // Save tiles for next request
        currentTiles.swap(newTiles);

        // Create shared pointers for the TileKeys and move them to the delta vector. See
        // functor for more details. We can't use the std::copy() functions since we have to
        // create the shared pointers.
        if (!deltaTilesSet.empty())
        {
            SetToVectorFunctor setToVectorFunctor(&deltaTiles);
            for_each(deltaTilesSet.begin(), deltaTilesSet.end(), setToVectorFunctor);
        }

        // Sort vector based on distance to the map center. We want to download the center
        // tiles first.
        // See functor class for more details To further improve this we could take the tilt
        // angle into consideration and download the tiles closer to the front first.
        {
            /*
               @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
               we can remove all locks. Until then ALL public interfaces have to be locked!
             */
            Lock lock(m_lock);
            SortVectorMyCenter sortFunctor(m_center);
            sort(deltaTiles.begin(), deltaTiles.end(), sortFunctor);
        }

        // Set flag so that we can check for any tiles to be unloaded
        m_currentTilesUpdated = true;
    }

    // @TODO: This is used for testing.
    // If the new tile size > 64, there must be something wrong.
    if (newTiles.size() > 64)
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo,"UPDATE_TILES_ERROR: size:%d", newTiles.size());
    }
}

/*! Helper function to calculate the minimum/maximum tile index based on a given mercator coordinate.

    @see UpdateTilesFrustum
 */
void
Frame::GetMinMaxTile(double x,          /*!< x coordinate in mercator */
                     double y,          /*!< y coordinate in mercator */
                     int zoomLevel,
                     Point& minTile,    /*!< In/Out: minimum tile coordinate */
                     Point& maxTile     /*!< In/Out: maximum tile coordinate */
                     ) const
{
    // Convert the mercator coordinate into a tile index and then update the minimum/maximum tile index.
    Point tempTile;
    NB_SpatialConvertMercatorToTile(x, y, zoomLevel, &(tempTile.x), &(tempTile.y));

    minTile.x = min(minTile.x, tempTile.x);
    minTile.y = min(minTile.y, tempTile.y);
    maxTile.x = max(maxTile.x, tempTile.x);
    maxTile.y = max(maxTile.y, tempTile.y);
}

/*! Convert a tile key to mercator coordinates */
void
Frame::ConvertTileToMercator(TileKey key,               /*!< Tile key to convert */
                             NB_Vector tileCorners[4]   /*!< On return the tile corners in mercator coordinates */
                             ) const
{
    // Convert top-left and bottom-right corners
    NB_SpatialConvertTileToMercator(key.m_x, key.m_y, key.m_zoomLevel, 0, 0, &tileCorners[0].x, &tileCorners[0].y);
    NB_SpatialConvertTileToMercator(key.m_x, key.m_y, key.m_zoomLevel, 1.0, 1.0, &tileCorners[2].x, &tileCorners[2].y);

    // Set top-right and bottom-left corners
    tileCorners[1].x = tileCorners[2].x;
    tileCorners[1].y = tileCorners[0].y;
    tileCorners[3].x = tileCorners[0].x;
    tileCorners[3].y = tileCorners[2].y;
}

/*! Helper function to set a vector.
 */
void
Frame::SetVector(const NBGM_Point2d64& input,  /*!< input vector */
                 NB_Vector& output,            /*!< vector to set */
                 bool& modified                /*!< In/Out: set to 'true' if the vector has changed */
                 ) const
{
    if (input.x != output.x)
    {
        output.x = input.x;
        modified = true;
    }
    if (input.y != output.y)
    {
        output.y = input.y;
        modified = true;
    }
    // Z-value is ignored
    output.z = 0.0;
}

/*! Check if the tile represented by tile key intersects with current frustum.

    @return true if so.
 */
bool
Frame::IsTileIntersectWithFrustum(const TileKey& key) const
{
    NB_Vector tileCorners[4] = {{0}};
    TileKey tileKey(key);

    int currentZoomLevel = 0;
    {
        /*
           @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
           we can remove all locks. Until then ALL public interfaces have to be locked!
          */
        Lock lock(m_lock);
        currentZoomLevel = static_cast<int>(floor(m_currentZoom));
    }

    // If zoom level of tile is larger than current zoom level, we need to convert it to
    // the corresponding tile key of current zoom level to calculate if they intersects
    // or not.
    if (key.m_zoomLevel > currentZoomLevel)
    {
        int distance        = key.m_zoomLevel - currentZoomLevel;
        tileKey.m_x         = key.m_x >> distance;
        tileKey.m_y         = key.m_y >> distance;
        tileKey.m_zoomLevel = currentZoomLevel;
    }

    ConvertTileToMercator(tileKey, tileCorners);


    {
        /*
            @todo: We should ensure that the Frame class always gets called from the same thread.
             Once that's ensured then we can remove all locks. Until then ALL public interfaces
             have to be locked!
          */
        Lock lock(m_lock);
        if (vector_polygons_intersect(tileCorners, 4, TRUE, m_frustum, 4, FALSE))
        {
            return true;
        }
    }

    // Special check for west-east wrap around case. If the frustum is passed -180/180
    // longitude then we check if the tile is a "wrap-around tile.
    if (m_frustumOverlapping)
    {
        int maxValue = static_cast<int>(nsl_pow(2, tileKey.m_zoomLevel));

        // Is the tile closer to the west border
        if (tileKey.m_x < (maxValue / 2))
        {
            // This will result in an out-of-range value, but that is exactly what we want
            tileKey.m_x += maxValue;
        }
        // The tile is closer to the east border
        else
        {
            // This will result in an out-of-range value, but that is exactly what we want
            tileKey.m_x -= maxValue;
        }

        // Now check again
        ConvertTileToMercator(tileKey, tileCorners);
        {
            /*
                @todo: We should ensure that the Frame class always gets called from the same thread. Once that's ensured then
                we can remove all locks. Until then ALL public interfaces have to be locked!
             */
            Lock lock(m_lock);
            if (vector_polygons_intersect(tileCorners, 4, TRUE, m_frustum, 4, FALSE))
            {
                return true;
            }
        }
    }

    return false;
}

/*! Check if the frustum is overlapping the west/east longitude boundary.

    Sets the overlapping flag.

    @return none
 */
void
Frame::SetFrustumOverlapping()
{
    //locked by caller
    for (int i = 0; i < 4; ++i)
    {
        // For now just check the east/west boundary. I don't think we need to check
        if ((m_frustum[i].x < -PI) || (m_frustum[i].x > PI))
        {
            m_frustumOverlapping = true;
            return;
        }
    }

    m_frustumOverlapping = false;
}

/* See description in header file. */
void
Frame::RemoveInvalidAnimationTiles(uint32 layerID, uint32 minTimestamp, uint32 maxTimestamp)
{
    // Find the animation bucket
    BUCKET_MAP::iterator bucket = m_receivedTiles.find(layerID);
    if (bucket != m_receivedTiles.end())
    {
        set<TileInfo>& tiles = bucket->second.m_tiles;

        /* Remove all tiles which are outside the timestamp range given by the frame list */

        // For all tiles in that bucket
        for (set<TileInfo>::iterator tile = tiles.begin();
             tile != tiles.end() /* Don't precalculate */; /* Don't increment here! */)
        {
            uint32 timestamp = tile->m_tile->GetTimeStamp();
            if ((timestamp < minTimestamp) || (timestamp > maxTimestamp))
            {
                // Remove the element. :TRICKY: Use post-iterator.
                tiles.erase(tile++);
            }
            else
            {
                ++tile;
            }
        }
    }
}

/* See description in header file. */
void Frame::GetLoadedTilesOfLayer(uint32 layerId, std::vector<TilePtr>& unloadedTiles)
{
    // Find received tiles by layer ID.
    BUCKET_MAP::iterator layerIterator = m_receivedTiles.find(layerId);
    if (layerIterator != m_receivedTiles.end())
    {
        // Add found received tiles to return.
        set<TileInfo>&          tilesOfLayer = layerIterator->second.m_tiles;
        set<TileInfo>::iterator tileIterator = tilesOfLayer.begin();
        set<TileInfo>::iterator tileEnd      = tilesOfLayer.end();
        for (; tileIterator != tileEnd; ++tileIterator)
        {
            TilePtr tile = tileIterator->m_tile;
            if (tile)
            {
                unloadedTiles.push_back(tile);
            }
        }
    }
}

/* See header file for description */
void
Frame::SwitchAnimationToFirstFrame()
{
    // This gets called from the render thread.
    // No lock necessary

    // Get all the frame data from all animation layers (most likely it will only be one)
    ANIMATION_MAP::iterator layerIterator = m_animationLayers.begin();
    ANIMATION_MAP::iterator end = m_animationLayers.end();
    for (; layerIterator != end; ++layerIterator)
    {
        MapViewAnimationLayer& layer = *layerIterator->second;
        layer.SwitchToFirstFrame();
    }
}

/* See header file for description */
void
Frame::SwitchAnimationToNextFrame()
{
    // This gets called from the render thread.
    // No lock necessary

    // Get all the frame data from all animation layers (most likely it will only be one)
    ANIMATION_MAP::iterator layerIterator = m_animationLayers.begin();
    ANIMATION_MAP::iterator end = m_animationLayers.end();
    for (; layerIterator != end; ++layerIterator)
    {
        MapViewAnimationLayer& layer = *layerIterator->second;
        layer.SwitchToNextFrame();
    }
}


/* See header file for description */
void
Frame::UnloadAllRasterTiles(vector<TilePtr>& unloadedTiles)
{
    // Find raster layers to unload tiles.
    BUCKET_MAP::iterator layerIterator = m_receivedTiles.begin();
    for (; layerIterator != m_receivedTiles.end();)
    {
        bool isRaster = IsRasterBucket(layerIterator->second);
        if(isRaster)
        {
            set<TileInfo>& tilesOfLayer = layerIterator->second.m_tiles;
            set<TileInfo>::iterator tileIterator = tilesOfLayer.begin();
            set<TileInfo>::iterator tileEnd = tilesOfLayer.end();
            for (; tileIterator != tileEnd; ++tileIterator)
            {
                TilePtr tile = tileIterator->m_tile;
                if (tile)
                {
                    unloadedTiles.push_back(tile);
                }
            }
            m_receivedTiles.erase(layerIterator++);
        }
        else
        {
            ++layerIterator;
        }
    }
}

/*! @} */

