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
    @file     Frame.h
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

#ifndef _FRAME_
#define _FRAME_

#include "base.h"
#include "graphics.h"
#include "TileKey.h"
#include "TileInternal.h"
#include "Layer.h"
#include "Lock.h"
#include "MapViewProfiling.h"
#include <set>
#include <vector>
#include "nbgmtypes.h"
#include "nbspatial.h"
#include "MapViewAnimationLayer.h"

using namespace nb;
using namespace std;

namespace nbmap
{

/*! Difference between the current zoom level and the zoom level we retrieve tiles to fill entire screen. */
const int HIGHER_ZOOM_LEVEL_DIFFERENCE = 2;

/*! Class containing TilePtr and TileKey for sorting. */
class TileInfo: public Base
{
public:

    TileInfo();
    TileInfo(TilePtr tile);
    virtual ~TileInfo();

    /*! Less operator so that we can use it in a set and/or map */
    bool operator<(const TileInfo& rightSide) const;

    TilePtr m_tile;

    // The tile key is redundant information, but we want to avoid retrieving it over and over again from the Tile object
    TileKey m_key;

    /*! Timestamp for animation tiles. In GPS time. This is '0' for non-animation tiles */
    uint32 m_animationTimestamp;
};

/*! The Layer bucket class is used for internal Frame calculation.

    We have layer buckets for each tile type. Once the zoom level changes then we check if the current zoom level is still
    within the min/max range. If not, then we unload the tiles from NBGM.

    The buckets are also used to check if we need to load any received tiles to NBGM.
 */
class LayerBucket : public Base
{
public:
    LayerBucket()
    : m_minZoomLevel(0),
      m_maxZoomLevel(0),
      m_zoomLevel(INVALID_TILE_ZOOM_LEVEL),
      m_isRasterLayer(true)
    {}

    LayerBucket(uint32 min, uint32 max, uint32 zoom, bool isRasterLayer)
    : m_minZoomLevel(min),
      m_maxZoomLevel(max),
      m_zoomLevel(zoom),
      m_isRasterLayer(isRasterLayer)
    {}

    uint32 m_minZoomLevel;
    uint32 m_maxZoomLevel;

    // Zoom level for that layer.
    uint32 m_zoomLevel;
    bool   m_isRasterLayer;
    set<TileInfo> m_tiles;
};

/*! Frame class used in map view.

    The frame class is responsible to calculate all tiles needed for the current screen. It is owned by the map view.
*/
class Frame : public Base
{
public:
    Frame();
    virtual ~Frame();

    // Public Functions ................................................................................................

    /*! Initialize has to be called before calling any other function.

        @return 'true' if successful, 'false' otherwise
    */
    bool Initialize(PAL_Instance* palInstance);

    /*! Sets all the layers to the frame class. The layers are needed for the layer buckets. */
    void SetLayers(const vector<LayerPtr>& layers);

    /* Update the Zoom and Frustum

       Adding this function to ensure that the zoom level and frustum are updated together, otherwise sometimes the calculation of request tile is wrong

       @return 'true' if the zoom level or frustums changed, 'false' if the frustums and zoom level didn't change
     */
    bool SetZoomLevelAndFrustum(double newZoom,                 /*!< New zoom value */
                                const vector<NBGM_Point2d64>& frustum,              /*!< Frustum used for vector */
                                const vector<NBGM_Point2d64>& frustumRasterNear     /*!< Frustum used for raster */
                                );

    /*! Set new map center in tile coordinates. */
    void SetCenter(Point newCenter);

    /*! Update the zoom.

        Call UpdateTiles() after calling this function.
    
        @return 'true' if the zoom was changed, 'false' otherwise
    */
    bool SetZoom(double newZoom                 /*!< New zoom value */
                 );

    double GetZoom() const;

    /*! Calculate the new tiles needed for the current screen.

        This function calculates the new tiles needed since the last call of this function. If new tiles are available,
        then it returns the delta tiles since the last call.

        The returned vector is sorted by tile priority; highest priority tile is the first tile in the vector, lowest
        priority tile is at the end of the vector.

        SetViewSize() has to be called at least once before calling this function.
        Call SetCenter()/SetZoom() to update the center/zoom between calls.

        @return 'true' if new tiles are available, 'false' otherwise
    */
    bool UpdateTiles(vector<TileKeyPtr>& deltaTiles,            /*!< On return the new tiles (delta) since the last call of this function */
                     vector<TileKeyPtr>& deltaTilesNonOverlay   /*!< On return new tiles for layers excluding overlay layers. The tile
                                                                     request for those tiles have to exclude all overlay layers. */
                     );

    /*! Inform the Frame class that the given tile was received.

        This function checks, if the tile is still needed for rendering or not. It also checks if the tile was already received,
        in which case it also can be ignored. See return description for more details.

        This function also checks if any tiles need to be unloaded in response to receiving the new tile. This can happen in
        raster mode.

        @return 'true' if the tile is a new tile and should be loaded to NBGM. 'false' if the tile is no longer needed for the 
                current frame, or if the tile was already received before and therefore can be discarded as well.
     */
    bool TileReceived(TilePtr tile,                     /*!< New received tile */
                      vector<TilePtr>& tilesToUnload    /*!< On return any tiles that need to be unloaded */
                      );

    /*! Checks if we need to unload any tiles from NBGM.

        @return 'true' if any tiles need to be unloaded, 'false' otherwise
     */
    bool CheckForTilesToUnload(vector<TilePtr>& tiles       /*!< In/Out: On return tiles to unload */
                               );

    /*! Set min/max zoom level where we should display raster layers */
    void SetRasterZoomRange(uint32 minZoomLevel, uint32 maxZoomLevel);

    /*! Unload all tiles

        @return None
    */
    void UnloadAllTiles(std::vector<TilePtr>& unloadedTiles      /*!< On return all unloaded tiles */
                        );

    /*! Unload all raster tiles

        @return None
    */
    void UnloadAllRasterTiles(std::vector<TilePtr>& unloadedTiles      /*!< On return all unloaded tiles */
                        );

    /*! Unload tiles of specified layer ID

        @return None
    */
    void UnloadTilesOfLayer(uint32 layerId,                      /*!< A layer ID to unload tiles */
                            std::vector<TilePtr>& unloadedTiles  /*!< On return unloaded tiles of the layer */
                            );

    /*! Get loaded tiles of specified layer.

        @return void
    */
    void GetLoadedTilesOfLayer(uint32 layerId,                      /*!< A layer ID to unload tiles */
                               std::vector<TilePtr>& unloadedTiles  /*!< On return loaded tiles of the layer */
                               );

    /*! Force clear tile keys for current frame.

            This function should be called then current frame coordinates isn't really changed but tile type has changed.
            For example route(traffic) layer was activated.

            This function should be used before UpdateTiles() function.

            @return none
        */
    void ForceCleanCurrentFrame();

    /*! Get the tile keys for the current frame */
    void GetCurrentTiles(vector<TileKeyPtr>& tiles  /*!< On return the tiles of the current frame */
                         ) const;

    /*! Check if tiles are loaded layer specified by layer Id. */
    bool LayerHasTileLoaded(uint32 layerId);

    // Animation layer functions .......................................................................................
    /*
        These funcitons have to be called in the context of the render thread in order to avoid synchronization issues.
     */
    void AddAnimationLayer(uint32 layerID);
    void RemoveAnimationLayer(uint32 layerID);
    void UpdateAnimationFrameList(uint32 layerID,
                                  const vector<uint32>& sortedFrameList,        /*!< New frame list, containing sorted timestamps */
                                  vector<shared_ptr<string> >& tilesToUnload    /*!< On return the tiles (if any) to unload from NBGM */
                                  );

    /*! Check if we have any animation layers active */
    bool HasAnimationLayers() const;

    /*! Check if the animation layers have any tiles */
    bool HasAnimationTiles() const;

    /*! Get all the animation frame data for all the animation layers.

        This function only returns frame data if it has changed since the last call.

        @return 'true' if we have any layer data, 'false' otherwise.
     */
    bool GetAnimationFrameData(int interval,                        /*!< Time elapesed in milliseconds since last call */
                               map<uint32, ANIMATION_DATA>& data,   /*!< The key to the outer map is a layerID. */
                               uint32& timestamp,                   /*!< On return: Timestamp of current frame index. If we have multiple animation
                                                                         layers then it returns the timestamp of the last layer (we don't display multiple
                                                                         timestamps in the UI) */
                               bool& animationCanPlay               /*!< Indicate if the animation can play */
                               );

    /*! Informs the animation layers that a tile was received */
    void AnimationTileReceived(TilePtr tile);

    /*! Informs all animation layer that tiles have been removed.

        This call doesn't actually unload the tiles. It just makes sure that the animation layer can be kept in sync for its animation.
     */
    void AnimationTilesRemoved(const vector<TilePtr>& tiles);

    /*! Set animation parameters for all animation layers */
    void SetAnimationParameters(const AnimationLayerParameters& parameters);

    /*! Switch animation layers to the first frame. Currently only doppler layers use this function. */
    void SwitchAnimationToFirstFrame();

    /*! Switch animation layers to the next frame. Currently only doppler layers use this function. */
    void SwitchAnimationToNextFrame();


    /*! Unload tiles from layer.

      @return vector of tiles should be unloaded.
    */
    vector<TilePtr>
    UnloadTilesFromFrame(const vector<TileKeyPtr>& tileKeys, /*!< TileKeys to unload tiles */
                         uint32 layerId                      /*!< A layer ID to unload a tile */
                         );

    /*! Remove animation tiles from layerbucket.

       Should only be called in CCC thread.
     */
    void   RemoveInvalidAnimationTiles(uint32 layerID,      /*!< ID of target layer */
                                       uint32 minTimestamp, /*!< min Timestamp of valid tiles */
                                       uint32 maxTimestamp  /*!< max Timestamp of valid tiles */
                                       );
    void SetFrustumValid(bool status);

private:

    // Private Functions ...............................................................................................

    /* See source file for description */
    void UpdateHigherZoomLevelTiles(vector<TileKeyPtr>& deltaTiles);
    void CalculateNeededTiles(const nb::Rectangle& rectangle, int zoom, set<TileKey>& currentTiles, vector<TileKeyPtr>& deltaTiles);
    bool IsTileStillNeeded(TilePtr tile) const;

    void CheckZoomTilesToUnload(vector<TilePtr>& tiles);
    void CheckPanningTilesToUnload(vector<TilePtr>& tiles);
    void CheckRasterTilesToUnload(const set<TileInfo>& tilesToCheck, vector<TilePtr>& tilesToUnload);
    void CheckTilesToUnloadFromBucketFrustum(LayerBucket& bucket, vector<TilePtr>& tiles);

    bool RasterMode() const;
    bool IsRasterBucket(const LayerBucket& bucket) const;
    bool IsHigherRasterZoomBucket(const LayerBucket& bucket) const;

    /* Functions for frustum calculation. See source for description */
    void UpdateTilesFrustum(vector<TileKeyPtr>& deltaTiles, const NB_Vector* frustum, int zoomLevel, set<TileKey>& currentTiles);
    void GetMinMaxTile(double x, double y, int zoomLevel, Point& minTile, Point& maxTile) const;
    void ConvertTileToMercator(TileKey key, NB_Vector tileCorners[4]) const;
    void SetVector(const NBGM_Point2d64& input, NB_Vector& output, bool& modified) const;
    bool IsTileIntersectWithFrustum(const TileKey& key) const;
    void SetFrustumOverlapping();

    /*! Set frustum

      @return 'true' if the frustums changed, 'false' if the frustums didn't change
    */
    bool SetFrustum(const vector<NBGM_Point2d64>& frustum,              /*!< Frustum used for vector */
                    const vector<NBGM_Point2d64>& frustumRasterNear     /*!< Frustum used for raster */
                    );

    // Private Members .................................................................................................

    /*! Lock for thread synchronization. */
    PAL_Lock* m_lock;

    /*! Current map center in tile coordinates.

        Used to prioritize tile requests based on the distance to the current center
     */
    Point m_center;

    double m_currentZoom;

    /*! Flag to indicate if we need to re-calculate the frame (and therefore the tiles). */
    bool m_needsUpdate;

    /*! Flag to indicate if the zoom level has changed since the last update */
    bool m_zoomHasChanged;

    /*! Flag to indicate that m_currentTiles were updated. We use this flag to check for tiles to unload */
    bool m_currentTilesUpdated;

    /* Minimum/maximum zoom levels for raster tiles */
    uint32 m_minRasterZoomLevel;
    uint32 m_maxRasterZoomLevel;

    /*! Tile keys for the current frame. These are the tiles we request based on the current zoom level. These tiles are different
     then the actually received tiles */
    set<TileKey> m_currentTiles;
    set<TileKey> m_currentTilesHigherZoomLevel;

    /*! Tiles which are currently loaded to NBGM. These tiles might be different then the requested tiles above due to the fact that
        the returned tiles can have a different reference zoom level then the actually requested tiles. Also, this only contains the
        already received tiles and not any still outstanding tiles.

        We have lists (buckets) sorted by layer-ids. When the zoom level changes then we check all the buckets of layers and throw out
        (unload from NBGM) any layers which are not required for the current zoom level.
     */
    typedef map<uint32, LayerBucket> BUCKET_MAP;
    BUCKET_MAP m_receivedTiles;

    /*! All layers. We need them to get the min/max zoom levels for our layer buckets. The key is the layer-ID */
    typedef map<uint32, LayerPtr> LAYER_MAP;
    LAYER_MAP m_layers;

    /*! Profiling info. Only used if enabled */
    MapViewProfiling m_profiling;

    /*! Frustum in mercator coordinates. The z-value is always zero. */
    NB_Vector m_frustum[4];
    NB_Vector m_frustumRasterNear[4];   // Used for raster calculation for tiles near the camera

    /*! Flag to indicate if the frustum is overlapping the west/east boundary. Gets set when the frustum gets updated */
    bool m_frustumOverlapping;

    /*! Currently displayed animation layers. The key in the map is the layer-ID

        Important: Only access from render thread!
     */
    typedef map<uint32, shared_ptr<MapViewAnimationLayer> > ANIMATION_MAP;
    ANIMATION_MAP m_animationLayers;

    /*! Animation layer parameters.

        We have to buffer the animation parameters and opacity, in case they are set while no animation layer is enabled, yet.
        When a new animation layer is added, then we use these settings as default.

        Important: Only access from render thread!
    */
    AnimationLayerParameters m_animationLayerParameters;
    bool m_isFrustumValid;
};

}

#endif

/*! @} */
