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
@file     UnifiedLayerProvider.h
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
#ifndef __UNIFIED_LAYER_PROVIDER__
#define __UNIFIED_LAYER_PROVIDER__

extern "C"
{
#include "paltypes.h"
#include "pal.h"
#include "paltimer.h"
#include "palclock.h"
#include "nberror.h"
#include "nbcontextprotected.h"
}
#include "LayerProvider.h"
//#include "NBProtocolMetadataSourceInformation.h"
//#include "NBProtocolMetadataSourceParameters.h"
#include "UnifiedLayer.h"
#include "UnifiedLayerListener.h"
#include "LayerIdGenerator.h"
#include "UnifiedMetadataProcessor.h"
#include <list>

#include "Handler.h"

namespace nbmap
{

class LayerAvailabilityMatrixLayer;

/* @todo: Remove both declarations until the test functions CreateTestFrameListLayer and
          CreateTestAnimationLayer are removed.
*/
class FrameListLayer;
class AnimationLayer;
class CachingTileManager;
class UnifiedTileType;

/*! Implementation for Unified Layer Provider.

This implementation of layer provider will talk to the
server to get layer and tile information. Please refer
to header file LayerProvider.h for more details.
*/
class UnifiedLayerProvider : public LayerProvider,
                             public Base
{
public:
    UnifiedLayerProvider(NB_Context* context,                /*!< Pointer to current context */
                         uint32 nbgmVersion,                 /*!< NBGM version to format requesting URL. And
                                                                   it should be checked with minimum and
                                                                   maximum version from metadata protocol.
                                                              */
                         uint32 maxTileRequestCountPerLayer, /*!< Number of maximum Tile Request Count Per Layer */
                         uint32 maxCachedTileCount,          /*!< Number of maximum count of saved tiles in cache */
                         nbcommon::HybridMode mode
                         );

    ~UnifiedLayerProvider();

    /*See Description in LayerProvider.h */
    virtual void GetLayers(shared_ptr<AsyncCallback<const std::vector<LayerPtr>& > > callback,
                           shared_ptr<MetadataConfiguration> metadataConfig);
    virtual void ClearLayers();

    /*! See description in LayerManager.h */
    virtual NB_Error SetCachePath(shared_ptr<string> cachePath);
    virtual NB_Error GetBackgroundRasterRange(uint32& minZoomLevel,
                                              uint32& maxZoomLevel);
    virtual const std::vector<LayerPtr>& GetLayerPtrVector() const;
    virtual const std::vector<LayerPtr>& GetOptionalLayerPtrVector() const;
    virtual const std::vector<LayerPtr>& GetCommonMaterialLayers() const;
    virtual const LayerPtr  GetLayerAvailabilityMatrix() const;
    virtual const LayerPtr GetDAMLayer() const;
    virtual void SetPreferredLanguageCode(uint8 languageCode);
    virtual void SetTheme(MapViewTheme theme);
    virtual void SetMetadataRetryTimes(int number);
    virtual NB_Error SetPersistentMetadataPath(shared_ptr<string> metadataPath);
    virtual void CheckMetadataChanges(shared_ptr<AsyncCallback<bool> > callback, shared_ptr<MetadataConfiguration> config, bool skipTimeInterval, bool forceUpdate);
    NB_Context* GetContext();

    /*! Assign LayerIdGenerator to UnifiedLayerProvider instance.

        @return None.
    */
    void SetLayerIdGenerator(LayerIdGeneratorPtr idGenerator);

    /*! Get the value of NBGM version

        @return Value of NBGM version
    */
    uint32 GetNbgmVersion();

    /*! Get the cache path of tile data

        @return Cache path of tile data
    */
    shared_ptr<std::string> GetCachePath();

    /*! Get the maximum count of the tiles for caching

        @return Maximum count of the tiles for caching
    */
    uint32 GetMaximumCachingTileCount();

    /*! Get maximum count of requests to download tiles for each layer

        @return Maximum count of requests to download tiles for each layer
    */
    uint32 GetMaximumTileRequestCountPerLayer();

    /*! Parse returned metadata and create layers based on it.

      @return NB_OK if succeeded, or other error code to represent an error.
    */
    NB_Error
    UpdateLayersBasedOnMetadataResponse(protocol::MetadataSourceInformationSharedPtr response);

    /*! This function will be called when client calls NB_ContextSendMasterClear

        This is a callback function that will be registered with NB_context.
        When client calls NB_ContextSendMasterClear, NB_Context will call this function
        This function will clear the cache.
     */
    static void MasterClear(void* pObject /*!< void pointer to be converted to the class */
                            );

    /*! Handling metadata changes.

        This function gets called by UnifiedLayerProvider CheckMetadataChangesCallback.

        @return None.
    */
    void MetadataUpdated(bool changed, NB_Error err = NE_OK);

    void CopyLayersFrom(const UnifiedLayerProvider& layerProvider);

private:

    /*! When a layers category is background,
        the client should not displaying any other background layer with a lower draw order
        we will set layer enable is false for lower draw order layer

        @return None.
    */
    void ProcessBackgroundLayers();

    /*! "deactivating-layer" key/value pair be added to the metadata common characteristics
        to identify that a layer should not be displayed when another layer is active.
        e.g. DVA layer characteristics: key:"deactivating-layer" value:"satellite".
        It meaning that the DVA layer CAN NOT display when the satellite layer active.

        @return None.
    */
    void ProcessDeactiveLayers();

    /*! Creates one UnifiedLayer baed on UrlArgsTemplate.

      It calls CreateAndSpecifyLayer() to create type-specified layer, and then fills it
      with some generic information.

      @return shared_pointer of created layer.
    */
    shared_ptr<UnifiedLayer>
    CreateAndGroupUnifiedLayer(shared_ptr<string> baseURL,
                               shared_ptr<protocol::UrlArgsTemplate> urlArgsTemplate);

    /*! Creates and specify layer based on layer type.

      @return shared pointer of created layer.
    */
    shared_ptr<UnifiedLayer>
    CreateAndSpecifyLayer(const string& category,
                          shared_ptr<UnifiedTileType> tileType,
                          shared_ptr<string> format,
                          shared_ptr<protocol::UrlArgsTemplate> urlArgsTemplate);

    /*! Cleans up the created layers.

      This will make association that is missing while the layer is created, for example,
      the association between DWR and DWRFL.

      @return void
    */
    void PostCreateLayers();

    /*! Invoke all stored callbacks passed in when GetLayer() is called..

      @return void
    */
    void InvokeCallbacks(NB_Error error);

    /*! Updates availability of common materials.

        @return None.
    */
    void UpdateMaterialStatistics(uint32 layerId, NB_Error error);

    /*! Callback when network event changed in CSL_Network.

       @return None
     */
    static void
    LayerProviderNetworkCallback(NB_NetworkNotifyEvent event,  /*!< Notification event. */
                                 void* data,                   /*!< Event specific data. */
                                 UnifiedLayerProvider* layerProvider/*!< The LayerProvider object */
                                 );

    /*! Some error code need the special handle when its happening. Like the 4020, 4003 and so on.
        The error 4003 and 4020 is meaning that the server broken, so the client should NOT retry the request.

        @return true(Need special handle). false(opposite)
    */
    bool CheckErrorCode(NB_Error err);
    void AppendIfNotContain(std::vector<LayerPtr> &lhsLayers, const std::vector<LayerPtr> &rhsLayers);
    bool Contain(std::vector<LayerPtr> layers, LayerPtr layer);

    NB_Context* m_pContext;                     /*!< Pointer to current context */
    NB_ContextCallback m_contextCallback;       /*!< A context callback object */
    uint32 m_nbgmVersion;                       /*!< NBGM version to format requesting URL. And
                                                     it should be checked with minimum and
                                                     maximum version from metadata protocol. */
    uint32 m_maximumCachingTileCount;           /*!< Maximum count of the tiles for caching */
    uint32 m_maximumTileRequestCountPerLayer;   /*!< Maximum count of requests to download tiles
                                                     for each layer */

    uint32 m_minRasterZoomLevel;                /*!< Minimum zoom level of raster layers */
    uint32 m_maxRasterZoomLevel;                /*!< Maximum zoom level of raster layers */

    shared_ptr<std::string> m_cachePath;        /*!< Cache path of tile data */
    std::vector<LayerPtr> m_pLayers;                /*!< cached layer list */
    std::vector<LayerPtr> m_pOptionalLayers;        /*!< cached optional layer list */
    std::vector<LayerPtr> m_pCommonMaterialLayers;  /*!< cached common materials layer list */
    UnifiedLayerPtr      m_pLayerAvailabilityMatrix;/*!< Layer Availability Matrix Layer */
    UnifiedLayerPtr      m_pDAMLayer;               /*!< Data Availability Matrix Layer */
    UnifiedMetadataProcessorPtr m_pMetadataProcessor;      /*!< Pointer of MetadataProcessor instance */
    LayerIdGeneratorPtr  m_pLayerIdGenerator;        /*!< Idgenerator to generate layer id */
    list<UnifiedLayerListener*> m_pListeners;       /*!< Listeners register to this provider */

    /*!< List of all callback for GetLayers() function.
         UnifiedLayerProvider is context based singleton, so there may be multiple calls of
         GetLayer() at the same time. If other request is being processed, we just store the
         callback, and invoke it when GetLayers() is finished.
    */
    vector<shared_ptr<AsyncCallback<const vector<LayerPtr>& > > > m_callbackList;
    shared_ptr<CheckMetadataChangesCallback<UnifiedLayerProvider> > m_pCallback;  /*!< Callback used internally */

    map<string, shared_ptr<FrameListLayer> > m_frameListLayers; /*!< FrameList layers, dataType as Key. */
    vector<shared_ptr<AnimationLayer> > m_pAnimationLayers;     /*!< Animation layers vector  */
    nbcommon::CachePtr    m_pPersistentCache;/*!< Pointer of Persistent Cache used by  regular layers. */
    map<string, nbcommon::CachePtr> m_animationCaches; /*!< Caches for animation layers.
                                                            Each animation layer will have
                                                            its own cache. Use TileDataType as key. */

    enum TileDownloadStatus
    {
        TDS_NotStart = 0,
        TDS_Failed,
        TDS_Succeeded,
    };

    /*!< Availability of common materials. It uses layer ID as key */
    map<uint32, TileDownloadStatus> m_materialStatistics;

    /*! Special callback for downloading Common Materials.

      When metadata is returned from Server, we start to download common materials, and we
      will only report success to the caller after all materials download.

      @note: This class should be used by GenericLayerCallback internally only. As the common
      materials do not need tile key, the parameter `request` in Error() is not the real tile
      key, but used to indicate the layer id of which layer CommonMaterial failed to download.
    */
    class CommonMaterialCallback : public AsyncCallbackWithRequest<TileKeyPtr, TilePtr>,
                                   public Base
    {
    public:
        CommonMaterialCallback(UnifiedLayerProvider* layerProvider);
        virtual ~CommonMaterialCallback();
        virtual void Success(TileKeyPtr request,
                             TilePtr response);
        virtual void Error(TileKeyPtr request,
                           NB_Error error);
        virtual bool Progress(int percentage);
    private:
        UnifiedLayerProvider* m_pLayerProvider;
    };

    typedef shared_ptr<CommonMaterialCallback> CommonMaterialCallbackPtr;
    CommonMaterialCallbackPtr m_pMaterialCallback;
    shared_ptr<string> m_language;

    uint8 m_preferredLanguageCode;
    shared_ptr<map<string, string> > m_pMutableParameters;
    map<string, UnifiedLayerPtr> m_layerMap; /*!< Used to record layers internally, should
                                                 be cleared when all layers are ready. */

    /*!< Used to record background layers internally,
         should be cleared when all layers are ready.
         The key is draw order. the value is the layer list of belong to this draw order.
    */
    map<uint32, vector<UnifiedLayerPtr> > m_backgroundLayerMap;
    uint32 m_metadataRetryCounter;

    /*!< Used to record deactivatin layers internally, should be cleared when all layers are ready. */
    typedef map<string, vector<UnifiedLayerPtr> > DEACTIVATING_LAYER_MAP;
    DEACTIVATING_LAYER_MAP m_deactivatingLayerMap;

    shared_ptr<std::string> m_clientGuid; /*!< Identifier of Client. */
    MapViewTheme m_mapTheme;
    
    char *m_credential;
};

typedef shared_ptr<UnifiedLayerProvider> UnifiedLayerProviderPtr;

}

#endif

/*! @} */
