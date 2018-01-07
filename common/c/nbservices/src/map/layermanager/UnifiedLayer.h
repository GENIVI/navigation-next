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
@file     UnifiedLayer.h
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

#ifndef __UNIFIED_LAYER__
#define __UNIFIED_LAYER__

extern "C"
{
#include "nbcontext.h"
}

#include "nbexp.h"
#include "paltypes.h"
#include "smartpointer.h"
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "base.h"
#include "Layer.h"
#include "TileKeyUtils.h"
#include "AsyncCallback.h"
#include "Tile.h"
#include "TileManager.h"
#include "Cache.h"
#include "contextbasedsingleton.h"
#include "LayerIdGenerator.h"

using namespace std;

namespace nbmap
{

// Default values of minimum and maximum supported zoom levels for layer
#define DEFAULT_MIN_SUPPORTED_ZOOM_LEVEL 0
#define DEFAULT_MAX_SUPPORTED_ZOOM_LEVEL 30

#define LAYER_DIGITAL_ID_INVALID       ((uint32)-1)

/*! Invalid value of LAM index is described in SDS:

   If the LAM index is set to 255, the layer's content is assumed to be always
   available within a tile area, and is therefore not represented in the LAM.

   @see GetLayerAvailabilityMatrixIndex
*/
#define INVALID_LAM_INDEX 0xFF

typedef shared_ptr<AsyncCallbackWithRequest<TileKeyPtr, TilePtr> > AsyncTileRequestWithRequestPtr;

class TileCallbackWithRequest;

/*! Layer within the unified map model.

    Contains both the internal details necessary
    for layer manager and a getter for the external interface layer.
    Internal interface not exposed to clients of LayerManager.
*/

class UnifiedLayer;
typedef shared_ptr<UnifiedLayer> UnifiedLayerPtr;

class UnifiedLayer : public Base,
                     public Layer
{
public:
    // Public functions ........................................................................
    UnifiedLayer(TileManagerPtr tileManagerPtr, uint32 layerIdDigital, NB_Context* context);
    UnifiedLayer(const UnifiedLayer& layer);

    virtual ~UnifiedLayer();

    /* See description in Layer.h */
    virtual uint32 GetDrawOrder();
    virtual shared_ptr<string> GetTileDataType();
    virtual uint32 GetMinZoom();
    virtual uint32 GetMaxZoom();
    virtual void GetTiles(vector<TileKeyPtr> tileKey,    /*!< Tile key to request */
                          AsyncTileRequestWithRequestPtr callback,   /*!< Callback of tile request */
                          uint32 priority        /*!< Priority of tile request */
                          );
    virtual NB_Error GetCachedTiles(const vector <TileKeyPtr>& tileKeys,
                                    AsyncTileRequestWithRequestPtr callback);
    virtual void RemoveAllTiles();
    virtual uint32 GetID() const;

    // @todo: remove functions in the following group if they are useless.
    virtual uint32 GetReferenceTileGridLevel();
    virtual uint32 GetLayerAvailabilityMatrixIndex();
    virtual bool IsEnabled() const;
    virtual uint32 GetLabelDrawOrder();
    virtual bool IsOverlay();
    virtual bool IsRasterLayer();

    /*! Clone this layer.

      New layer will be assigned with ID generated using IdGenerator which is context-based
      singleton.
     */
    virtual shared_ptr<UnifiedLayer> Clone();


    /*! Make this layer displayable

        @return None.
    */
    virtual void SetEnabled(bool isEnabled);

    shared_ptr<string> GetProjection();

    shared_ptr<string> GetCountry();

    shared_ptr<string> GetBaseURL();

    shared_ptr<string> GetTemplate();

    shared_ptr<string> GetTileStoreTemplate();

    uint32 GetDownloadPriority();

    TileLayerInfoPtr GetTileLayerInfo();

    bool IsOptional() const;

    bool IsAnimationLayer() const;

    /*! Return name of this layer. */
    shared_ptr<string> GetLayerName() const;

    virtual string className() const { return "UnifiedLayer"; }
    virtual string str() const;

    /*! Return name of parent layer. */
    shared_ptr<string> GetParentLayerType() const;

    /*! Get all children layers of this layer.

      @return vector of children layers.
    */
    const vector<shared_ptr<UnifiedLayer> >& GetAllChildrenLayers() const;

    /*! Check if tiles from this layer can be displayed.

      It should return false from LAM and DAM layers.

      @return true if tiles should be displayed.
    */
    bool IsDisplayable() const;

/*All setter functions*/


    // Actually, all these setters should be only be accessed by LayerProvder, and should be
    // removed from interfaces!
    void SetID(shared_ptr<string> id);

    void SetCacheID(uint32 cacheID);

    void SetTileStoreTemplate(shared_ptr<string> tileStoreTemplate);

    void SetLabelDrawOrder(uint32 LabelDrawOrder);

    void SetTileSize(uint32 tileSize);

    void SetDrawOrder(uint32 drawOrder);

    void SetMinZoom(uint32 minZoom);

    void SetMaxZoom(uint32 maxZoom);

    void SetLayerAvailabilityMatrixIndex(uint32 layerAvailabilityMatrixIndex);

    void SetTemplate(shared_ptr<string> ptemplate);

    void SetDatasetGenerationID(shared_ptr<string> datasetGenerationID);

    void SetDownloadPriority(uint32 downloadPriority);

    void SetType(shared_ptr<string> type);

    void SetReferenceTileGridLevel(uint32 refTileGridLevel);

    void SetCharacteristics(const string& key, const string& value);

    void SetCharacteristics(const map<string, string>& characteristicMap);

    void SetProjection(shared_ptr<string> projection);

    void SetCountry(shared_ptr<string> country);

    void SetBaseURL(shared_ptr<string> baseURL);

    void SetIsInternal(bool isInternal);

    void SetFormatName(shared_ptr<string> formatName);

    void SetMinVerison(uint32 minVersion);

    void SetMaxVersion(uint32 maxVersion);

    /*! Process characteristics of this layer.

      It should handle some characteristics shared by all layers, then call function virtual
      function: ProcessSpecialCharacteristicsOfLayer() which should be implemented by
      children of this class to handle some layer-type dependent characteristics.
     */
    void ProcessCharacteristics();

    /*! Return name of this layer. */
    void SetParentLayer(const UnifiedLayer* layer);

    /*! Set specified layer as children.

      @return None.
    */
    void AddChildrenLayer(const shared_ptr<UnifiedLayer>& layer);

    /*! Set specified layer as mutex.

      @return None.
    */
    void AddMutexLayer(UnifiedLayer* layer);
    set<UnifiedLayer*> GetMutexLayers();

    /*! Set displayable property of layer.

      If a layer is not displayable, it should not be exposed to MapView. This property will
      be checked before it is exposed.

      @return None.
    */
    void SetDisplayable(bool displayable);

    /*! Set mutable parameters of layer.

      Some parameters may get changed, these mutable parameters need to be updated during
      runtime.

      @return None
    */
    void SetMutableParameters(shared_ptr<map<string, string> > mutableParameters);

    void SetSubDrawOrder(uint32 subDrawOrder)
    {
        m_tileLayerInfo->subDrawOrder = subDrawOrder;
    }


    void AddDeactivatingLayer(UnifiedLayer* layer)
    {
        if (layer)
        {
            m_deactivateLayers.insert(layer);
        }
    }

    bool IsBackgroundRasterLayer();

protected:
    // Private functions ........................................................................

    /* See source file for description */
    virtual void GetTile(TileKeyPtr tileKey, TileKeyPtr convertedTileKey,
                         AsyncTileRequestWithRequestPtr callback,
                         uint32 priority, bool cachedOnly = false);

    /*! Actions that was needed to handle layer disable or enable
     *
     * Subclasses can do some clean up here.
     *
     * @return void
     */
    virtual void PostLayerToggled();

    NB_Error
    CreateTemplateParametersByTileKey(const TileKeyPtr& convertedTileKey,
                                      shared_ptr<map<string, string> >& templateParameters
                                      );
    /*! Add tileType specific TemplateParameters into template parameters map.

        @return None
    */
    virtual void
    AppendSpecialTemplateParamters(shared_ptr<map<string, string> > templateParameters);

    /*! Process some layer specific characteristics.

      Should be implemented by Children of UnifiedLayer.
     */
    virtual void
    ProcessSpecialCharacteristicsOfLayer();

    /*! Get characteristic specified by `key` */
    shared_ptr<string> GetCharacteristics(const string key) const ;

    template <typename T>
    shared_ptr<UnifiedLayer> CloneWithTemplate (const T* layer)
    {
        // Caller should make sure T is a subclass of UnifiedLayer!
        shared_ptr<UnifiedLayer> newLayer;
        uint32 layerId = LayerIdGenerator::InvalidId;
        LayerIdGeneratorPtr idGenerator =
                ContextBasedSingleton<LayerIdGenerator>::getInstance(m_pContext);
        if (layer && idGenerator &&
            ((layerId = idGenerator->GenerateId()) != LayerIdGenerator::InvalidId))
        {
            newLayer.reset(new T(*layer));
            if (newLayer)
            {
                newLayer->m_tileLayerInfo->layerID = layerId;
            }
        }

        return newLayer;
    }

    shared_ptr<TileCallbackWithRequest> m_pCallback;
    shared_ptr<TileCallbackWithRequest> m_pCachedOnlyCallback;

    NB_Context*        m_pContext;            /*!< Pointer to NB_Context instance */
    TileManagerPtr     m_tileManager;         /*!<Please refer to TileManager.h >*/
    bool               m_isEnabled;           /*!<Is the Layer enabled>*/
    bool               m_isOptional;          /*!<Is the layer Optional>*/
    bool               m_isAnimationLayer;    /*!<Is the layer an animation layer>*/
    shared_ptr<string> m_id;                  /*!<Layer ID>*/
    vector<uint32>     m_supportedZoomLevels; /*!<Array of supported zoom levels>*/
    uint32             m_cacheID;             /*!<Number that is used by the client to see
                                                  if it needs to flush the cache. It will be
                                                  changed by the server whenever the tile
                                                  source changes. The client should cache
                                                  the last number received and flush the
                                                  cache if the new cache-id is different. >*/
    shared_ptr<string> m_pProjection;         /*!<Map projection type. The client can use
                                                  this value to determine, if it needs to
                                                  convert the TCS coordinates to match the
                                                  coordinate system used by the map source
                                                  server. Supported types : 'mercator',
                                                  'transverse-mercator',
                                                  'universal-transverse-mercator', or
                                                  'web-mercator'.>*/
    shared_ptr<string> m_pCountry;            /*!<Country code ISO 3166-1 alpha 3. Set to an
                                                  empty string if the content is not
                                                  specific to a country.>*/
    shared_ptr<string> m_pBaseURL;            /*!<Base URL for all url-args-template elements>*/
    shared_ptr<string> m_pFormatName;         /*!<Data formats in which the content can be
                                                  retrieved (e.g. PNG or NBM). The client
                                                  must specify one of these formats when
                                                  requesting the data.>*/
    uint32             m_tileSize;            /*!<The tile size to use for this server,
                                               based on the screen size and resolution
                                               provided in the maptile-source request.
                                               This parameter will be hardcoded in the
                                               URL, but the number is passed to the
                                               client to use in calculations.>*/
    uint32             m_downloadPriority;    /*!<Priority to download from the content
                                               source server relative to the other layers.
                                               Valid range is 0 (for download first) to 255
                                               (for download last).>*/
    shared_ptr<string> m_datasetGenerationID; /*!<Dataset Generation ID. This ID uniquely
                                               specifies the generation (build) of
                                               the map content for the layer.>*/
    uint32             m_maxVersion;          /*!<Maximum supported version of the format>*/
    uint32             m_minVersion;          /*!<Minimum supported version of the format>*/
    uint32             m_layerAvailabilityMatrixIndex;  /*!<Index (bit offset) within the
                                                         LAM (layer availability matrix)
                                                         bitmask for this layer.>*/
    bool               m_IsInternal;          /*!<If true, the map source is owned by TCS>*/
    shared_ptr<string> m_pTileStoreTempalte;  /*!<The template for the tile cache. After the
                                                  application replaces the parameters with
                                                  real values, the resulting string can be
                                                  used as a key for the cache. The template
                                                  is designed such that tiles coming from the
                                                  same store will have the same key
                                                  regardless of the resulting URL used to
                                                  fetch them.>*/
    shared_ptr<string> m_pTemplate;

    TileLayerInfoPtr   m_tileLayerInfo;       /*!<Pointer to information that shared by all tiles
                                                  of this layer.*/

    bool               m_isOverlay;           /*!< Whether this is overlay layer or not. */
    bool               m_isRasterLayer;       /*!< Flag to indicate if this is raster Layer. */

private:
    /*! Check if this tile key is duplicated.

      @return true if so.
    */
    bool IsDuplicatedKey(TileKeyPtr tileKey);

    /*! Check if this layer is active.
     *
     *  We should not download tiles from this layer if it is not effective.
     */
    bool IsEffective();

    // Private variables ....
    set<TileKey>       m_filter;         /*!< a filter to filter out duplicated requests. */

    map<string,string> m_characteristics; /*!<This element specifies the characteristics of
                                              a set of data. The set of characteristics may
                                              differ based on data type. >*/
    /*! Relationship between layers:

      A layer may have one parent layer, and multiple children layers. Children layers are
      active only when its parent layer is active.
    */
    const UnifiedLayer* m_parentLayer;   /*!< Parent of this layer. Do not use shared_ptr here,
                                              it will cause circular reference. */
    vector<shared_ptr<UnifiedLayer> >  m_childrenLayers;  /*!< Children of this layer */
    shared_ptr<string>       m_parentLayerType; /*!< Name of parent layer, it should be released
                                                 when all layers are initialized, and then we
                                                 should use m_parentLayer to specify parent
                                                 layer. */
    shared_ptr<string> m_layerName;     /*!< User readable name of this layer. */
    bool               m_displayable;   /*!< Flag to indicate if tiles from this layer
                                             should be displayed by MapView */

    set<UnifiedLayer*> m_mutexLayers; // If this layer is enabled, then all layers in
                                      // m_mutexLayers need to be disabled.

    set<UnifiedLayer*>      m_deactivateLayers; // If one of m_deactivateLayers is enabled,
                                               // then this layer should be treat as disabled.

    shared_ptr<map<string, string> >  m_pMutableParameters;
};

}
#endif
/*! @} */
