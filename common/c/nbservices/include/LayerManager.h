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
  @file     LayerManager.h
  @defgroup nbmap
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

#ifndef __LAYER_MANAGER__
#define __LAYER_MANAGER__

#include "nbexp.h"
#include "paltypes.h"
#include "nbcontext.h"
#include "smartpointer.h"
#include <map>
#include <vector>
#include <string>
#include "AsyncCallback.h"
#include "TileKey.h"
#include "Layer.h"
#include "Tile.h"
#include "LayerManagerListener.h"

#define UNLIMITED_RETRY_TIME  -1


/*! Theme for mapview */
enum MapViewTheme
{
    MVT_DAY,                /*!<Set the theme to day>*/
    MVT_NIGHT,              /*!<Set the theme to night>*/
    MVT_MAX                 /*!<Max vaule for this enumeration>*/
};

namespace nbmap
{

class MetadataConfiguration;

/*! The primary responsibility of Layer Manager component is to encapsulate all necessary
    data and operations required to provide Tile instances to the MapView component. The
    MapView component interfaces with Layer Manager by providing a key and expecting a Tile
    stack in return.
*/
class LayerManager
{

public:

    LayerManager() {}

    virtual ~LayerManager() {}


    /*! Asynchronously get an array of tiles for each tile key.

      This function will return one tile for each layer for each requested tile key.

      Either the layerList or ignoreList will be populated, but they can not be populated at
      the same time.

      If the layerList is populated only those layers will be requested.
      If the ignoreList is populated only those layers will be ignored.
      If callback is not desired pass in a null pointer for callback.
    */
    virtual NB_Error GetTiles(const std::vector<TileKeyPtr>& tileKeys,        /*!< Vector of tile keys that have to be fetched. */
                              shared_ptr<AsyncCallback<TilePtr> > callback,   /*!< Callback objects for notifications */
                              const std::vector<LayerPtr>& layerList,         /*!< Layers to be downloaded. */
                              const std::vector<LayerPtr>& ignoreList,        /*!< Layers to be ignored. */
                              bool includeOverlay = true,
                              bool isPrefetch = false
                              ) = 0;

    /*! Deprecated Do not use

      Get Tiles and store them in cache.

      This function will get the tiles and store them in
      cache. Client is not notified after the tiles are fetched. Can
      be used by clients to get tiles so that they are available in
      cache when needed.

    */
    virtual NB_Error GetTiles(const std::vector<TileKeyPtr>& tileKeys,        /*!< Vector of tile keys that need to be fetched. */
                              std::vector<LayerPtr> const* ignoreList = NULL, /*!< Layers to be ignored. */
                              bool includeOverlay = true,                     /*!< Download tiles from overlay or not. */
                              bool isPrefetch = false) = 0;

    /*! Deprecated Do not use

      Asynchronously get an array of tiles for each tile key.

      This function will return one tile for each layer for each requested tile key.
      Callback will return an array of tileKeys and and an array of tiles that belong
      to those tile keys.
    */
    virtual NB_Error GetTiles(const std::vector<TileKeyPtr>& tileKeys,        /*!< Vector of tile keys that have to be fetched. */
                              shared_ptr<AsyncCallbackWithRequest<TileKeyPtr, TilePtr> > callback, /*!< Callback objects for notifications */
                              std::vector<LayerPtr> const* ignoreList = NULL, /*!< Layers to be ignored. */
                              bool includeOverlay = true,                      /*< Download tiles from overlay or not. */
                              bool isPrefetch = false) = 0;

    /*! Request the tiles for the specified animation layer

        This function requests the tiles by the tile keys and timestamps for the
        specified layer ID. An animation tile is identified by both the tile key
        and timestamp. One tile is related with one tile key, but it may contain
        multiple frames for different timestamps.

        @return NE_OK if success
    */
    virtual NB_Error
    GetAnimationTiles(LayerPtr layer,                               /*!< Specified the requested layer */
                      const std::vector<TileKeyPtr>& tileKeys,      /*!< Tile keys used to request tiles */
                      const std::vector<uint32>& timestamps,        /*!< Timestamps for each tile. If the vector is empty,
                                                                         all timestamps for each tile will be requested. */
                      shared_ptr<AsyncCallback<TilePtr> > callback  /*!< Callback used to return the animation tiles */
                      ) = 0;



    /*! Return tiles that are cached

       This function will return a vector of tile objects if and only if the tiles are in
       cache. For each tilekey function will check if tile is present in cache. If present
       in cache add it to the return vector otherwise just ignore.

       @return NE_OK if succeeded, and tiles will be returned in callback.
    */
    virtual NB_Error
    GetCachedTiles(const vector<TileKeyPtr>& tileKeys, /*!< Vector of tile keys to be fetched. */
                   shared_ptr<AsyncCallback<TilePtr> > callback /*!< callback used to return tile*/
                   ) = 0;


    /*! Get all layers regardless of location

    Get all the layers regardless of the location.
    getCached will allow for fast load. If true only chached layers will be returned.
    if false network call will be made if necessary.

     Psuedocode
     For each UrlArgsTemplate
        Create Tile manager
        Create UnifiedLayer and Pass in Tile Manager
     End For.
     Pass Layer stack back to caller

    @param callback for notification that will be used to notify the availability of layers to its caller.
    */
    virtual NB_Error GetLayers(shared_ptr<AsyncCallback<const std::vector<LayerPtr>& > > callback,
                               shared_ptr<MetadataConfiguration> metadataConfig
                               ) = 0;

    /*! Set the cache path for tile data storage

       Lets the caller set the path on the filesystem where the tile data
       will be stored.
       @param cachePath Cache path of tile data
    */
    virtual NB_Error SetCachePath(shared_ptr<std::string> cachePath) = 0;

    /*! Set the full path of persistent metadata for metadata storage

        @todo: Fix the path without filename. The filename is specified
               in the internal.

        The parameter 'metadataPath' contains both path and filename.

        @return NE_OK if success
    */
    virtual NB_Error
    SetPersistentMetadataPath(shared_ptr<std::string> metadataPath  /*!< Full path of persistent metadata. It
                                                                         contains both path and filename. */
                              ) = 0;

    /*! Set maximum retry time when metadata request failed.

        Set it to UNLIMITED_RETRY_TIME(-1) means the retry time is unlimited.

        @return None.
    */
    virtual void
    SetMetadataRetryTimes(int number /*!< Maximum retry time to be set. */
                          ) = 0;

    /*! Get minimum and maximum zoom levels of raster layers

        @return NE_OK if success,
                NE_NOENT if there is no zoom levels specified.
    */
    virtual NB_Error
    GetBackgroundRasterRange(uint32& minZoomLevel,    /*!< On return minimum zoom level for all raster layers */
                             uint32& maxZoomLevel     /*!< On return maximum zoom level for all raster layers */
                   ) = 0;

    /*! Remove all Outstanding tile requestsand clear cache if requested.

        Indicate to Layer manager that the client is no longer interested in any previously requested tiles.
        Will cancel all the previously request tiles.

        @return None
    */
    virtual void
    RemoveAllTiles() = 0;

    /*! Remove all raster tiles requestsand clear cache if requested.

     Indicate to Layer manager that the client is no longer interested in any previously raster tiles.
     Will cancel all the previously request raster tiles.

     @return None
     */
    virtual void
    RemoveBackgroundRasterTiles() = 0;

    /*! Gets all the raster layers.

     @return all the raster layers.
     */
    virtual vector<LayerPtr>
    GetBackgroundRasterLayers() = 0;

    /*! Asynchronously get required common materials.

        This function will download all common materials. It is called automatically when
        metadata request is finished, and also can be called by the caller to download
        common materials other than other tiles.

        @param callback Asynchronous callback.
        @param type shared pointer to common material type. If is NULL, then all types of
               common materials will be downloaded.
        @return NB_OK if succeeded.
    */
    virtual NB_Error
    GetCommonMaterials(shared_ptr<AsyncCallbackWithRequest<TileKeyPtr, TilePtr> > callback,
                       shared_ptr<std::string> type)  = 0;

    /*! This function will register a listener

        Layer Manager will add the listener to local
        vector of listeners. When an update to tiles is required
        all the registered listeners will be notified.

        @param listener that needs to registered.
        @return NE_OK if success, NE_EXIST if this listener already exists.
    */
    virtual NB_Error
    RegisterListener(LayerManagerListener* listener) = 0;

    /*! This function will remove the listener specified from the notification list

        This function will indicate to the Layer Manager that specified listener no longer wishes
        to receive notifications from layer manager

        @param listener that needs to unregistered.
        @return None
    */
    virtual void
    UnregisterListener(LayerManagerListener* listener) = 0;

    /*! Set preferred language code for MapView.

      @param preferredLanguageCode Preferred language code for labels. 0 means local
      language. See NBM format doc for full code list.

      @return None
    */
    virtual void SetPreferredLanguageCode(uint8 languageCode) = 0;

   /*! Set day& night theme for MapView.

      @param theme for mapview.

      @return None
    */
    virtual void SetTheme(MapViewTheme theme) = 0;
};

typedef shared_ptr<LayerManager> LayerManagerPtr;

}

#endif  // __LAYER_MANAGER__

/*! @} */
