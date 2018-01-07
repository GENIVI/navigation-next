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
@file     UnifiedLayer.cpp
@defgroup nbmap

NOTE: UnifiedLayer instance is shared in one NB_Context, this means there may
      be multiple LayerManagers that is requesting tiles with same layer
      instance. UnifiedLayer should not skip same requests from different
      client. This is ensured by storing requests into the callback resides in
      UnifiedLayerManager.
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
#include "UnifiedLayer.h"
#include "TileCallbackWithRequest.h"
#include "CachingTileManager.h"
#include "LayerAvailabilityMatrixLayer.h"
#include "StringUtility.h"
#include "ctype.h"
#include <sstream>
#include <algorithm>
#include "nbmacros.h"
#include <functional>

extern "C"
{
#include "nbqalog.h"
#include "cslquadkey.h"
}

//#define DLAYER
#ifdef DLAYER
#include <QDebug>
#endif

using namespace std;
using namespace nbcommon;
using namespace nbmap;

/*! Functor to help set parent for unified layer. */
class ParentLayerSetter
{
public:
    ParentLayerSetter(UnifiedLayer* parent)
            : m_parentLayer(parent) {}
    virtual ~ParentLayerSetter() {}

    void operator() (const UnifiedLayerPtr& childLayer)
    {
        if (childLayer)
        {
            childLayer->SetParentLayer(m_parentLayer);
            if (m_parentLayer)
            {
                m_parentLayer->AddChildrenLayer(childLayer);
            }
        }
    }

private:
    UnifiedLayer* m_parentLayer;
};

struct RemoveAllTilesFunctor
{
    void operator() (UnifiedLayerPtr& layer)
    {
        if (layer)
        {
            layer->RemoveAllTiles();
        }
    }
};

UnifiedLayer::UnifiedLayer(TileManagerPtr tileManager,
                           uint32 layerIdDigital,
                           NB_Context* context)
      : m_pCallback(new TileCallbackWithRequest(context)),
        m_pCachedOnlyCallback(new TileCallbackWithRequest(context)),
        m_pContext(context),
        m_tileManager(tileManager),
        m_isEnabled(true),
        m_isOptional(false),
        m_isAnimationLayer(false),
        m_cacheID(0),
        m_tileSize(0),
        m_downloadPriority(0),
        m_maxVersion(0),
        m_minVersion(0),
        m_layerAvailabilityMatrixIndex(INVALID_LAM_INDEX),
        m_IsInternal(false),
        m_tileLayerInfo(new TileLayerInfo),
        m_isOverlay(false),
        m_isRasterLayer(true),
        m_parentLayer(NULL),
        m_displayable(true)
{
    // @note: if we failed to create TileLayerInfo objct here, UnifiedLayerProvider will
    //        report this error to the caller. So we do not need to check m_tileLayerInfo
    //        every time in further operation.
    if (m_tileLayerInfo)
    {
        m_tileLayerInfo->layerID          = layerIdDigital;
        m_tileLayerInfo->minZoom          = DEFAULT_MIN_SUPPORTED_ZOOM_LEVEL;
        m_tileLayerInfo->maxZoom          = DEFAULT_MAX_SUPPORTED_ZOOM_LEVEL;
        m_tileLayerInfo->refTileGridLevel = INVALID_TILE_ZOOM_LEVEL;
    }
}

UnifiedLayer::~UnifiedLayer()
{
    // Reset all children layers.
    for_each (m_childrenLayers.begin(), m_childrenLayers.end(), ParentLayerSetter(NULL));
    m_childrenLayers.clear();

    m_mutexLayers.clear();
    m_deactivateLayers.clear();

}

/* See description in Layer.h */
uint32
UnifiedLayer::GetDrawOrder()
{
    return m_tileLayerInfo->drawOrder;
}

/* See description in Layer.h */
void
UnifiedLayer::SetEnabled(bool isEnabled)
{
    if (m_isEnabled ^ isEnabled)
    {
        m_isEnabled = isEnabled;
        m_pCallback->SetEnabled(isEnabled);
        m_pCachedOnlyCallback->SetEnabled(isEnabled);
        PostLayerToggled();
    }
}

/* See description in Layer.h */
shared_ptr<string>
UnifiedLayer::GetTileDataType()
{
    return m_tileLayerInfo->tileDataType;
}

/* See description in Layer.h */
uint32
UnifiedLayer::GetMinZoom()
{
    return m_tileLayerInfo->minZoom;
}

/* See description in Layer.h */
uint32
UnifiedLayer::GetMaxZoom()
{
    return m_tileLayerInfo->maxZoom;
}

/* See description in Layer.h */
uint32
UnifiedLayer::GetReferenceTileGridLevel()
{
    return m_tileLayerInfo->refTileGridLevel;
}

/* See description in Layer.h */
uint32
UnifiedLayer::GetLayerAvailabilityMatrixIndex()
{
    return m_layerAvailabilityMatrixIndex;
}

/* See description in Layer.h */
uint32
UnifiedLayer::GetLabelDrawOrder()
{
    return m_tileLayerInfo->labelDrawOrder;
}

/* See description in Layer.h */
shared_ptr<string>
UnifiedLayer::GetProjection()
{
    return m_pProjection;
}

/* See description in Layer.h */
shared_ptr<string>
UnifiedLayer::GetCountry()
{
    return m_pCountry;
}

/* See description in Layer.h */
void UnifiedLayer::GetTile(TileKeyPtr tileKey, TileKeyPtr convertedTileKey,
                           AsyncTileRequestWithRequestPtr callback,
                           uint32 priority, bool cachedOnly)
{

    if (cachedOnly && !callback)
    {
        return;
    }

    NB_Error error = NE_OK;
    do
    {
        if (!m_tileManager)
        {
            error = NE_NOTINIT;
            break;
        }
        if (!tileKey)
        {
            error = NE_INVAL;
            break;
        }

        shared_ptr<map<string, string> > templateParameters;
        // Create the template parameters by a TileKey object.
        error = CreateTemplateParametersByTileKey(convertedTileKey,
                                                  templateParameters);

        if (error != NE_OK)
        {
            break;
        }

        shared_ptr<TileCallbackWithRequest> pCallback = cachedOnly ?
                                                        m_pCachedOnlyCallback : m_pCallback;
        // Create a TileCallbackWithRequest object to request a tile.
        if (callback)
        {
            TileRequestPtr request(new TileRequest(tileKey, convertedTileKey, callback));
            if (request && pCallback)
            {
                pCallback->StoreTileRequest(request);
            }
            else
            {
                error = NE_NOMEM;
                break;
            }
        }

        // QaLog
        if (CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(m_pContext)))
        {
            NB_QaLogTileRequest(m_pContext, convertedTileKey->m_x,
                                convertedTileKey->m_y, convertedTileKey->m_zoomLevel);
        }

        // Request a tile by the tile manager.
        m_tileManager->GetTile(templateParameters, pCallback, priority, cachedOnly);
    } while (0);

    if (callback && error != NE_OK)
    {
        callback->Error(tileKey, error);
    }
}

/* See description in Layer.h */
void
UnifiedLayer::RemoveAllTiles()
{
    if (m_tileManager)
    {
        m_tileManager->RemoveAllTiles();
    }
}

uint32 UnifiedLayer::GetID() const
{
    return m_tileLayerInfo->layerID;
}

shared_ptr<string> UnifiedLayer::GetBaseURL()
{
    return m_pBaseURL;
}

shared_ptr<string> UnifiedLayer::GetTemplate()
{
    return m_pTemplate;
}

shared_ptr<string> UnifiedLayer::GetTileStoreTemplate()
{
    return m_pTileStoreTempalte;
}

uint32 UnifiedLayer::GetDownloadPriority()
{
    return m_downloadPriority;
}

void UnifiedLayer::SetID(shared_ptr<string> id)
{
    m_id = id;
}

void UnifiedLayer::SetCacheID(uint32 cacheID)
{
    m_cacheID = cacheID;
}

/**/
void UnifiedLayer::SetTileStoreTemplate(shared_ptr<string> tileStoreTemplate)
{
    m_pTileStoreTempalte = tileStoreTemplate;
}

void UnifiedLayer::SetLabelDrawOrder(uint32 LabelDrawOrder)
{
    m_tileLayerInfo->labelDrawOrder = LabelDrawOrder;
}

void UnifiedLayer::SetTileSize(uint32 tileSize)
{
    m_tileSize = tileSize;
}

void UnifiedLayer::SetDrawOrder(uint32 drawOrder)
{
    m_tileLayerInfo->drawOrder = drawOrder;
}

void UnifiedLayer::SetMinZoom(uint32 minZoom)
{
    m_tileLayerInfo->minZoom = minZoom;
}

void UnifiedLayer::SetMaxZoom(uint32 maxZoom)
{
    m_tileLayerInfo->maxZoom = maxZoom;
}

void UnifiedLayer::SetLayerAvailabilityMatrixIndex(uint32 layerAvailabilityMatrixIndex)
{
    m_layerAvailabilityMatrixIndex = layerAvailabilityMatrixIndex;
}

void UnifiedLayer::SetTemplate(shared_ptr<string> ptemplate)
{
    m_pTemplate = ptemplate;
}

void UnifiedLayer::SetDatasetGenerationID(shared_ptr<string> datasetGenerationID)
{
    m_datasetGenerationID = datasetGenerationID;
}

void UnifiedLayer::SetDownloadPriority(uint32 downloadPriority)
{
    m_downloadPriority = downloadPriority;
}

void UnifiedLayer::SetType(shared_ptr<string> type)
{
    m_tileLayerInfo->tileDataType = type;
}

void UnifiedLayer::SetReferenceTileGridLevel(uint32 refTileGridLevel)
{
    m_tileLayerInfo->refTileGridLevel = refTileGridLevel;
}

void UnifiedLayer::SetCharacteristics(const string& key, const string& value)
{
    m_characteristics[key] = value;
}

void UnifiedLayer::SetCharacteristics(const map<string, string>& characteristicMap)
{
    m_characteristics.insert(characteristicMap.begin(), characteristicMap.end());
}

shared_ptr<string> UnifiedLayer::GetCharacteristics(const string key) const
{
    shared_ptr<string> value;
    map<string, string>::const_iterator iter = m_characteristics.find(key);
    if (iter != m_characteristics.end())
    {
        value = shared_ptr<string>(new string(iter->second));
    }

    return value;
}

void UnifiedLayer::SetCountry(shared_ptr<string> country)
{
    m_pCountry = country;
}

void UnifiedLayer::SetProjection(shared_ptr<string> projection)
{
    m_pProjection = projection;
}

void UnifiedLayer::SetBaseURL(shared_ptr<string> baseURL)
{
    m_pBaseURL = baseURL;
}

void UnifiedLayer::SetIsInternal(bool isInternal)
{
    m_IsInternal = isInternal;
}

void UnifiedLayer::SetFormatName(shared_ptr<string> formatName)
{
    m_pFormatName = formatName;

    //@note: Seems no better way to decide if tiles of this layer are vector or not except
    //       this hard-coded "NBM" format.

    //TODO: This change only for NextGen Demo, Later we should remove this logic.
    //Function: Stretching tiles when zoom level > 18 in satellite layer
    //-------For demo begin-------
    shared_ptr<string> type = GetTileDataType();
    if(type && (!type->compare("satellite2")))
    {
        m_isRasterLayer = false;
        return;
    }
    //-------For demo end-------

    if (m_pFormatName && *m_pFormatName == "NBM")
    {
        m_isRasterLayer = false;
    }
}

void UnifiedLayer::SetMinVerison(uint32 minVersion)
{
    m_minVersion = minVersion;
}

void UnifiedLayer::SetMaxVersion(uint32 maxVersion)
{
    m_maxVersion = maxVersion;
}

TileLayerInfoPtr UnifiedLayer::GetTileLayerInfo()
{
    return m_tileLayerInfo;
}

/*! Create the template parameters by a TileKey object

    @return NE_OK if success
*/
NB_Error
UnifiedLayer::CreateTemplateParametersByTileKey( const TileKeyPtr& tileKey,         /*!< Converted tile key related with reference tile
                                                                                                         grid level. This reference is used to return. */
                                                shared_ptr<map<string, string> >& templateParameters/*!< Template parameters to create. This reference
                                                                                                         is used to return. */
                                                )
{
    if (!tileKey)
    {
        return NE_INVAL;
    }

    map<string, string>* newTemplateParameters =
            m_pMutableParameters.get() ? new map<string, string>(*m_pMutableParameters) :
            new map<string, string>();
    if (!newTemplateParameters)
    {
        return NE_NOMEM;
    }

    // Transform a TileKey object to the template parameters.
    newTemplateParameters->insert(pair<string, string>(
        PARAMETER_KEY_TILE_X, StringUtility::NumberToString(tileKey->m_x)));

    newTemplateParameters->insert(pair<string, string>(
        PARAMETER_KEY_TILE_Y, StringUtility::NumberToString(tileKey->m_y)));

    newTemplateParameters->insert(pair<string, string>(
        PARAMETER_KEY_ZOOM_LEVEL, StringUtility::NumberToString(tileKey->m_zoomLevel)));

    char quadkey[CSL_MAXIMUM_QUADKEY_LENGTH] = {0};
    (void) CSL_QuadkeyConvertFromXYZ(tileKey->m_x,
                                     tileKey->m_y,
                                     (uint8)(tileKey->m_zoomLevel),
                                     quadkey);
    newTemplateParameters->insert( pair<string, string>(PARAMETER_KEY_QUAD, quadkey));

    templateParameters = shared_ptr<map<string, string> >(newTemplateParameters);

    // Append other TileTypeSpecific parameters (if necessary).
    AppendSpecialTemplateParamters(templateParameters);

    return NE_OK;
}

void UnifiedLayer::AppendSpecialTemplateParamters(shared_ptr<map<string, string> > /*templateParameters*/)
{
    // Nothing to do for generic unified layers.
}

UnifiedLayer::UnifiedLayer(const UnifiedLayer& layer)
        : m_pCallback(new TileCallbackWithRequest(layer.m_pContext)),
          m_pCachedOnlyCallback(new TileCallbackWithRequest(layer.m_pContext)),
          m_pContext(layer.m_pContext),
          m_isEnabled(layer.m_isEnabled),
          m_isOptional(layer.m_isOptional),
          m_isAnimationLayer(layer.m_isAnimationLayer),
          m_id(layer.m_id),
          m_supportedZoomLevels(layer.m_supportedZoomLevels),
          m_cacheID(layer.m_cacheID),
          m_pProjection(layer.m_pProjection),
          m_pCountry(layer.m_pCountry),
          m_pBaseURL(layer.m_pBaseURL),
          m_pFormatName(layer.m_pFormatName),
          m_tileSize(layer.m_tileSize),
          m_downloadPriority(layer.m_downloadPriority),
          m_datasetGenerationID(layer.m_datasetGenerationID),
          m_maxVersion(layer.m_maxVersion),
          m_minVersion(layer.m_minVersion),
          m_layerAvailabilityMatrixIndex(layer.m_layerAvailabilityMatrixIndex),
          m_IsInternal(layer.m_IsInternal),
          m_pTileStoreTempalte(layer.m_pTileStoreTempalte),
          m_pTemplate(layer.m_pTemplate),
          m_tileLayerInfo(new TileLayerInfo(*(layer.m_tileLayerInfo))),
          m_isOverlay(layer.m_isOverlay),
          m_isRasterLayer(layer.m_isRasterLayer),
          m_characteristics(layer.m_characteristics),
          m_parentLayer(layer.m_parentLayer),
          m_parentLayerType(layer.m_parentLayerType),
          m_layerName(layer.m_layerName),
          m_displayable(layer.m_displayable),
          m_mutexLayers(layer.m_mutexLayers),
          m_deactivateLayers(layer.m_deactivateLayers)

{
    // Create new TiltLayerInfo and TileManager.
    CachingTileManager* tileManager =
            static_cast<CachingTileManager*>(layer.m_tileManager.get());
    CachingTileManager* newManager  = NULL;
    if (tileManager)
    {
        newManager = tileManager->CloneWithLayerInfo(m_tileLayerInfo);
    }

    m_tileManager = TileManagerPtr(newManager);


    // If this layer has children, clone all of them and update relation ship of them.

    vector<UnifiedLayerPtr>::const_iterator iter = layer.m_childrenLayers.begin();
    vector<UnifiedLayerPtr>::const_iterator end  = layer.m_childrenLayers.end();
    for (; iter != end; ++iter)
    {
        UnifiedLayerPtr child;
        if ((*iter) && (child = (*iter)->Clone()))
        {
            child->SetParentLayer(this);
            AddChildrenLayer(child);
        }
    }

    for_each(m_mutexLayers.begin(), m_mutexLayers.end(),
             bind2nd(mem_fun(&UnifiedLayer::AddMutexLayer), this));
    for_each(m_deactivateLayers.begin(), m_deactivateLayers.end(),
             bind2nd(mem_fun(&UnifiedLayer::AddDeactivatingLayer), this));

    // We don't need to update layer->m_parentLayer since this member field should be
    // set by parent of layer.
}

bool UnifiedLayer::IsEnabled() const
{
    if (!m_isEnabled) // If not enabled, just return false;
    {
        return false;
    }

    if (m_parentLayer && !m_parentLayer->IsEnabled()) // return false if parent not enabled
    {
        return false;
    }

    // return true otherwise.
    return true;
}

bool UnifiedLayer::IsOverlay()
{
    return m_isOverlay;
}

bool UnifiedLayer::IsRasterLayer()
{
    return m_isRasterLayer;
}

void UnifiedLayer::GetTiles(vector<TileKeyPtr> tileKeys,
                            AsyncTileRequestWithRequestPtr callback,
                            uint32 priority)
{
    if (!IsEffective())
    {
        return;
    }
    m_filter.clear();

    vector<TileKeyPtr>::iterator iter = tileKeys.begin();
    vector<TileKeyPtr>::iterator end  = tileKeys.end();
    uint32 localPriority = priority;
    for (; iter != end; ++iter)
    {
        const TileKeyPtr& key = *iter;
        if (key->m_zoomLevel < (int)m_tileLayerInfo->minZoom ||
            key->m_zoomLevel > (int)m_tileLayerInfo->maxZoom)
        {
            continue;
        }


        if (m_isRasterLayer) // Start download directly if this is raster layer.
        {
            if (!IsDuplicatedKey(key))
            {
                TileKeyPtr convertedKey = key;
                GetTile(key, key, callback, localPriority++);
            }
        }
        else // Convert to ref-tile-grid-level to start download.
        {
            vector <TileKeyPtr> convertedKeys;
            ConvertTileKeyToOtherZoomLevel(key, m_tileLayerInfo->refTileGridLevel,
                                           convertedKeys);
            vector<TileKeyPtr>::iterator iter = convertedKeys.begin();
            vector<TileKeyPtr>::iterator end  = convertedKeys.end();

            while (iter != end)
            {
                const TileKeyPtr& convertedKey = *iter;

                if (convertedKey && !IsDuplicatedKey(convertedKey))
                {
                    GetTile(key, convertedKey, callback, localPriority++);
                }
                ++ iter;
            }
        }
    }

    m_filter.clear();
}

/* See description in header file. */
NB_Error UnifiedLayer::GetCachedTiles(const vector <TileKeyPtr>& tileKeys,
                                      AsyncTileRequestWithRequestPtr callback)
{
    if (!IsEnabled())
    {
        return NE_NOENT;
    }

    m_filter.clear();

    vector<TileKeyPtr>::const_iterator iter = tileKeys.begin();
    vector<TileKeyPtr>::const_iterator end  = tileKeys.end();
    vector<TilePtr> tiles;

    for (; iter != end; ++iter)
    {
        const TileKeyPtr& key = *iter;
        if (key->m_zoomLevel < (int)m_tileLayerInfo->minZoom ||
            key->m_zoomLevel > (int)m_tileLayerInfo->maxZoom)
        {
            continue;
        }

        // @note: GetCachedTiels() are designed to fetch old raster tiles only.
        if (key->m_zoomLevel == (int)m_tileLayerInfo->refTileGridLevel)
        {
            if (!IsDuplicatedKey(key))
            {
                GetTile(key, key, callback, 0, true);
            }
        }
    }

    m_filter.clear();

    // Since we are getting tiles from cache, all tiles will be filled into the vector
    // resided in CachedOnlyCallbackWithRequest.

    return NE_OK;
}

/* See description in header file. */
bool UnifiedLayer::IsDuplicatedKey(TileKeyPtr tileKey)
{
    bool duplicated = true;
    if (tileKey)
    {
        pair<set<TileKey>::iterator, bool> result = m_filter.insert(*tileKey);
        duplicated = !result.second;
    }
    return duplicated;
}

/* See description in header file. */
void UnifiedLayer::PostLayerToggled()
{
    if (!m_isEnabled) // From Enabled to Disabled.
    {
        RemoveAllTiles();
        for_each (m_childrenLayers.begin(), m_childrenLayers.end(), RemoveAllTilesFunctor());
    }
}

/* See description in header file. */
shared_ptr <UnifiedLayer> UnifiedLayer::Clone()
{
    return CloneWithTemplate<UnifiedLayer>(this);
}

/* See description in header file. */
bool UnifiedLayer::IsOptional() const
{
    return m_isOptional;
}

/* See description in header file. */
bool UnifiedLayer::IsAnimationLayer() const
{
    return m_isAnimationLayer;
}

/* See description in header file. */
void UnifiedLayer::ProcessCharacteristics()
{
    // Processing common characteristics here.

    // Name
    shared_ptr<string> value = GetCharacteristics(TILE_ADDITIONAL_KEY_NAME);
    if (value && !value->empty())
    {
#ifdef DLAYER
        if (*value == "POI1")
        {
            qWarning() << "Got POI1 Layer";
        }

        if (*value == "BR15")
        {
            qWarning() << "Got BR15 Layer";
        }
#endif
        m_layerName = value;
    }

    // Check layer is optional, we treat this layer as optional whenever key "optional"
    // exits in characteristics.
    value = GetCharacteristics(TILE_ADDITIONAL_KEY_OPTIONAL);
    if (value)
    {
        m_isOptional = true;

        transform(value->begin(), value->end(), value->begin(), ::tolower);
        if (value->compare(OPTIONAL_DEFAULT_ON))
        {
            m_isEnabled  = false;
        }
        else
        {
            m_isEnabled = true;
        }
    }

    // Parent layer, store it into m_parentLayerType, and m_parentLayer will be filled based on this.
    value = GetCharacteristics(TILE_ADDITIONAL_KEY_PARENT_LAYER);
    if (value)
    {
        m_parentLayerType = value;
    }

    if (!m_tileLayerInfo->materialCategory)
    {
        value = GetCharacteristics(LAYER_CATEGORY_MATERIAL);
        if (value)
        {
            m_tileLayerInfo->materialCategory.reset(CCC_NEW string(*value));
        }
        else // always use default MAT otherwise.
        {
            m_tileLayerInfo->materialCategory.reset(CCC_NEW string("MAT"));
        }
    }

    // At last, dedicate processing to my children.
    ProcessSpecialCharacteristicsOfLayer();
}

/* See description in header file. */
void UnifiedLayer::ProcessSpecialCharacteristicsOfLayer()
{
    //@note: Do nothing here, should be implemented by children when necessary.
}

/* See description in header file. */
shared_ptr<string> UnifiedLayer::GetLayerName() const
{
    return m_layerName;
}

string UnifiedLayer::str() const
{
    char buf[33];
    string s = "(";

    s += className();
    s += " ";
    if (GetLayerName())
    {
        s += *GetLayerName();
    }

    s += " ";
    sprintf(buf, "%d", GetID());
    s += buf;

    if (m_tileManager)
    {
        s += " ";
        s += m_tileManager->str();
    }
    s += ")";

    return s;
}

/* See description in header file. */
shared_ptr<string> UnifiedLayer::GetParentLayerType() const
{
    return m_parentLayerType;
}

/* See description in header file. */
void UnifiedLayer::SetParentLayer(const UnifiedLayer* layer)
{
    m_parentLayer = layer;
}

/* See description in header file. */
void UnifiedLayer::AddChildrenLayer(const shared_ptr <UnifiedLayer>& layer)
{
    // If this layer is not optional, it should not have children layers: it is meaningless
    // to have children layers if it is not going to be enabled/disabled. And only optional
    // layers can be toggled by design.
    nsl_assert(m_isOptional);

    if (layer) // @todo: should make layer unique!
    {
        m_childrenLayers.push_back(layer);
    }
}

/* See description in header file. */
const vector <shared_ptr <UnifiedLayer> >& UnifiedLayer::GetAllChildrenLayers() const
{
    return m_childrenLayers;
}

/* See description in header file. */
bool UnifiedLayer::IsDisplayable() const
{
    return m_displayable;
}

/* See description in header file. */
void UnifiedLayer::SetDisplayable(bool displayable)
{
    m_displayable = displayable;
}

/* See description in header file. */
void UnifiedLayer::SetMutableParameters(shared_ptr <map <string, string> > mutableParameters)
{
    m_pMutableParameters = mutableParameters;
}

/* See description in header file. */
void UnifiedLayer::AddMutexLayer(UnifiedLayer* layer)
{
    m_mutexLayers.insert(layer);
}

/* See description in header file. */
set<UnifiedLayer*> UnifiedLayer::GetMutexLayers()
{
    return m_mutexLayers;
}

bool UnifiedLayer::IsBackgroundRasterLayer()
{
    shared_ptr<string> layerCategory = GetCharacteristics(LAYER_CATEGORY_IDENTIFIER);

    bool isBaseMap = false;
    if(layerCategory)
    {
        isBaseMap = (*layerCategory == string(LAYER_CATEGORY_BASEMAP));
    }

    bool isMaxZoomLevelEqualMin = false;
    if(m_tileLayerInfo)
    {
        isMaxZoomLevelEqualMin = (m_tileLayerInfo->maxZoom == m_tileLayerInfo->minZoom);
    }

    if(isBaseMap && isMaxZoomLevelEqualMin)
    {
        return true;
    }

    return false;
}

bool UnifiedLayer::IsEffective()
{
    if (!IsEnabled())
    {
        return false;
    }


    // return false if one of m_deactivateLayers is enabled.
    if (!m_mutexLayers.empty())
    {
        set<UnifiedLayer*>::const_iterator iter = m_mutexLayers.begin();
        set<UnifiedLayer*>::const_iterator end  = m_mutexLayers.end();
        for (; iter != end; ++iter)
        {
            UnifiedLayer* mutexLayer = *iter;
            if (mutexLayer->IsEnabled() && mutexLayer->GetDrawOrder() > GetDrawOrder())
            {
                return false;
            }
        }
    }

    return true;
}
/*! @} */

