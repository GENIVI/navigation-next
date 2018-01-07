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
    @file     UnifiedLayerManager.h
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

#ifndef __UNIFIED_LAYER_MANAGER__
#define __UNIFIED_LAYER_MANAGER__

#include "nbcontext.h"
#include "LayerProvider.h"
#include "TileManager.h"
#include "LayerManager.h"
#include "nbcontextprotected.h"
#include "OffboardMetadataProcessor.h"
#include "GenericTileCallback.h"
#include "Pin.h"
#include "UnifiedMetadataProcessor.h"
#include "UnifiedLayerListener.h"
#include "HybridManager.h"
#include <set>

namespace nbmap
{
/*
  Implementation of LayerManager interface for the Unified Map Component.
  This implementation needs a LayerProvider injected in the constructor.
*/
class UnifiedLayerManager : public LayerManager,
                            public UnifiedLayerListener,
                            public Base,
                            public nbcommon::HybridModeListener
{
public:

    // Public functions .........................................................................

    /* See description in LayerManager.h */
    virtual NB_Error GetTiles(const std::vector<TileKeyPtr>& tileKeys,
                              shared_ptr<AsyncCallback<TilePtr> > callback,
                              const std::vector<LayerPtr>& layerList,
                              const std::vector<LayerPtr>& ignoreList,
                              bool includeOverlay = true,
                              bool isPrefetch = false);
    virtual NB_Error GetTiles(const vector<TileKeyPtr>& tileKeys,
                              vector<LayerPtr> const * ignoreList = NULL,
                              bool includeOverlay = true,
                              bool isPrefetch = false);
    virtual NB_Error GetTiles(const vector<TileKeyPtr>& tileKeys,
                              shared_ptr<AsyncCallbackWithRequest<TileKeyPtr, TilePtr> > callback,
                              vector<LayerPtr> const * ignoreList = NULL,
                              bool includeOverlay = true,
                              bool isPrefetch = false);
    virtual NB_Error GetAnimationTiles(LayerPtr layer,
                                       const vector<TileKeyPtr>& tileKeys,
                                       const vector<uint32>& timestamps,
                                       shared_ptr<AsyncCallback<TilePtr> > callback);
    virtual NB_Error GetCachedTiles(const vector<TileKeyPtr>& tileKeys,
                                    shared_ptr<AsyncCallback<TilePtr> > callback);
    virtual NB_Error GetLayers(shared_ptr<AsyncCallback<const vector<LayerPtr>& > > callback,
                               shared_ptr<MetadataConfiguration> metadataConfig);
    virtual NB_Error SetCachePath(shared_ptr<string> cachePath);
    virtual NB_Error SetPersistentMetadataPath(shared_ptr<string> metadataPath);

    virtual void SetMetadataRetryTimes(int number);
    virtual NB_Error GetBackgroundRasterRange(uint32& minZoomLevel,
                                    uint32& maxZoomLevel);
    virtual void RemoveAllTiles();
    virtual void RemoveBackgroundRasterTiles();
    virtual vector<LayerPtr> GetBackgroundRasterLayers();

    virtual NB_Error GetCommonMaterials(shared_ptr<AsyncCallbackWithRequest<TileKeyPtr, TilePtr> > callback,
                                        shared_ptr<string> type);
    virtual NB_Error RegisterListener(LayerManagerListener* listener);
    virtual void     UnregisterListener(LayerManagerListener* listener);
    virtual void     SetPreferredLanguageCode(uint8 languageCode);
    virtual void     SetTheme(MapViewTheme theme);

    void GetDAMTiles(vector<TileKeyPtr>& tileKeys,
                     shared_ptr<AsyncCallbackWithRequest<TileKeyPtr, TilePtr> > callback);

    /* See description in UnifiedLayerListener.h */
    virtual void UnifiedLayerUpdated();

    /*Constructor for LayerManager*/
    UnifiedLayerManager(LayerProviderPtr layerProvider, NB_Context* context);

    /*destructor for LayerManager*/
    virtual ~UnifiedLayerManager();

    /*! Callback for regular tiles when download failed.

        This function will walk through all the callbacks registered for a tileKey and layer,
        do further process according to tile type.

        @return None.
    */
    void
    TileRequestError(NB_Error error       /*!< Error code returned by TileManeger. */
                     );

    /*! This function will be called when client calls NB_ContextSendMasterClear

        This is a callback function that will be registered with NB_context.
        When client calls NB_ContextSendMasterClear, NB_Context will call this function
        This function will clear the cache.
     */
    static void MasterClear(void* pObject /*!< void pointer to be converted to the class*/
                            );

    /*! This function returns the Layer Provider Associated with the Layer Manager

        This function will return the Layer Provider that is associated with the Layer Manager
    */
    LayerProviderPtr GetLayerProvider();

    /*! Tell listeners that layers are updated.

        @return None.
    */
    void UpdateLayers(const vector<LayerPtr>& layers);

    void
    NotifyAnimationLayerAdded(shared_ptr<Layer> layer/*!<Shared pointer to identify the animation layer>*/
                              );

    void
    NotifyAnimationLayerUpdated(shared_ptr<Layer> layer, /*!<Shared pointer to identify the animation layer>*/
                                const vector<uint32 >& frameList /*!<Each int represents a time stamp>*/
                                );

    void
    NotifyAnimationLayerRemoved(shared_ptr<Layer> layer /*!<Shared pointer to identify the animation layer>*/);

    /*! Tell listeners that new layers available.

      @return void
    */
    void NotifyLayersAdded(const vector<LayerPtr>& layers);

    /*! Tell listeners that some layers are removed..

      @return void
    */
    void NotifyLayersRemoved(const vector<LayerPtr>& layers);

    /*! Tell listeners that some pins are removed.

     @return void
    */
    void NotifyPinsRemoved(shared_ptr<vector<PinPtr> > pins);

    /*! Tell listeners whether to display map legend or not.

      @return void
    */
    void
    NotifyShowMapLegend(bool enabled, shared_ptr <MapLegendInfo> mapLegend);

    /*! Clear layers

        This function clears all layers and persistent information about layers.

        @return None
    */
    void ClearLayers();

    /*! Invoke registered listeners to update all tiles.

        @return None
    */
    void
    RefreshAllTiles();

    /*! Invoke registered listeners to update tiles of specifed layer.

        @return None
    */
    void
    RefreshTilesOfLayer(LayerPtr layer      /*!< Specified layer to refresh tiles */
                        );

    /*! Invoke registered listeners to reload tiles of specifed layer.

      @return None
    */
    void
    ReloadTilesOfLayer(LayerPtr layer      /*!< Specified layer to refresh tiles */
                       );


    /*! Invoke registered listeners to update tiles of specified layer.

        @return None
    */
    void
    RefreshTiles(const vector<TileKeyPtr>& tileKeys,   /*!< Tile keys to refresh */
                 LayerPtr layer                        /*!< Specified layer to refresh tiles */
                 );


    /*! Handling metadata changes.

        This function gets called by CheckMetadataChangesCallback.

        @return None.
    */
    void MetadataUpdated(bool changed, NB_Error error = NE_OK);

    /*! Notify layer request success.

        @return None.
    */
    void LayerRequestSuccess(const vector<LayerPtr>& layers);

    /*! Notify layer request error.

        @return None.
    */
    void LayerRequestError(NB_Error error);


    /*! Notify listener to add/remove/active Material.  */
    void NotifyMaterialAdded(const ExternalMaterialPtr& material);
    void NotifyMaterialRemoved(const ExternalMaterialPtr& material);
    void NotifyMaterialActived(const ExternalMaterialPtr& material);

    /*! hybrid mode changed callback  */
    void HybridModeChanged(nbcommon::HybridMode mode);

private:
    // Private functions ...................................................................

    /*! Real function to download tiles.

      @return NB_Error
    */
    NB_Error GetTilesInternal(const vector<TileKeyPtr>& tileKeys,
                              shared_ptr<TileRequestCallback> callback,
                              const vector<LayerPtr>& layerList,
                              bool includeOverlay,
                              bool isPrefetch = false);

    /*! Get priority of this download request.

      @return download priority
    */
    uint32  GetBatchPriority();

    /*! Checks if metadata changed or not.

      @return None
    */
    void CheckMetadataChanges(bool forceUpdate);

    /*! Callback which will be added to event queue to notify listeners about new layers.

        @return None.
    */
    static void RetrivedLayersAndNotifyListener(PAL_Instance* pal, void* userData);

    /*! Callback when network event changed in CSL_Network.

       @return None
     */
    static void
    LayerManagerNetworkCallback(NB_NetworkNotifyEvent event,      /*!< Notification event. */
                                void* data,                       /*!< Event specific data. */
                                UnifiedLayerManager* layerManager /*!< The UnifiedLayerManager object */
                                );

   /*! Callback when metadata needs update.

        @return None.
    */
    static void CheckMetadataCallback(PAL_Instance* pal, void* userData);

    /*! Update metadate if it changes.

      @return None
    */
    void UpdateMetadata(bool forceUpdate);

    // Private members ..........................................................................

    uint32 m_currentBatchPriority;  /*!< Number of current BatchPriority. Lower number has
                                         higher priority BatchPriority is used to
                                         distinguish tile requests generated by different
                                         call of LayerManager::GetTiles(). This number get
                                         increased each time when LayerManager::GetTiles()
                                         is called, so that requests generated later will
                                         have higher priority than requests generated
                                         before. */

    NB_ContextCallback m_contextCallback;   /*!< A context callback object */

    LayerProviderPtr m_layerProviderPtr;    /*!< is called to provide layer list */
    uint32           m_retriedTimes;        /*!< Counter of how many download requests ended
                                                 with 404 error. */
    nb_unixTime      m_thresholdTimestamp;  /*!< timestamp for when the time threshold starts */

    shared_ptr<MetadataConfiguration> m_pMetadataConfig;

    bool m_errorHandlingInProgress;
    vector<LayerManagerListener*> m_pListeners;    /*!< Registered listeners of layer manager */

    /*! Temporary callback provided by client. It is stored here for two reasons:
        1. To notify client about the new layers after metadata and materials donwloaded.
        2. To check if the metadata is being requested or not.
     */
    shared_ptr<AsyncCallback<const vector<LayerPtr>& > > m_pGetLayersCallback;
    LayerManagerWrapperForCallbackPtr m_pManagerWrapper; /*!< LayerManagerWrapper used to
                                                              generate all kinds of callbacks. */
    TileRequestCallbackPtr m_tileReqeustCallback;

    vector<LayerPtr> m_animationLayers; /*!< This list will contain all the animation layers>*/

    struct LayerManagerTaskParameter
    {
        LayerManagerTaskParameter(UnifiedLayerManager* manager)
                : m_manager(manager),
                  m_taskId(0)        {}
        ~LayerManagerTaskParameter() {}

        UnifiedLayerManager* m_manager;
        uint32               m_taskId;
        bool                 m_forceUpdateMetadata;
    };

    LayerManagerTaskParameter* m_taskParam;
    LayerManagerTaskParameter* m_taskParamForMetadata;
};

typedef shared_ptr<UnifiedLayerManager> UnifiedLayerManagerPtr;

}
#endif
/*! @} */
