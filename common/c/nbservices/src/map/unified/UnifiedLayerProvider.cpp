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
    @file     UnifiedLayerProvider.cpp
    @defgroup nbmap

*/
/*
(C) Copyright 2015 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems, is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

extern "C"
{
#include "palfile.h"
#include "nbcontextprotected.h"
#include "nbpersistentdata.h"
#include "nbpersistentdataprotected.h"
#include "nbqalog.h"
#include "cslqarecorddefinitions.h"
#include "cslnetwork.h"
}

#include "UnifiedLayerProvider.h"
#include "TpsAtlasbookProtocol.h"
#include "UnifiedTileManager.h"
#include "UnifiedTileType.h"
#include "LeastRecentlyUsedEvictionStrategy.h"
#include "Cache.h"
#include "CachingTileManager.h"
#include "TpsElement.h"
#include "NBProtocolMetadataSourceInformationSerializer.h"
#include "Tuple.h"
#include "contextbasedsingleton.h"
#include "RasterTileConverter.h"
#include "CommonMaterialLayer.h"
#include <cmath>
#include <string>
#include <limits>
#include "StringUtility.h"
#include "LayerAvailabilityMatrixLayer.h"
#include "FrameListLayer.h"
#include "AnimationLayer.h"
#include "CommonMaterialLayer.h"
#include "RefreshLayer.h"
#include "RouteLayer.h"
#include "UnifiedLayerWithLAM.h"
#include "AnimationLayer.h"
#include <algorithm>
#include "paldebuglog.h"
#include "nbmacros.h"
#include "MetadataConfiguration.h"
#include "UnifiedService.h"

#define DEFAULT_LANGUAGE_CODE       0
#define DEFAULT_TILE_REQUEST_RETRY_TIMES 3

//#define DLAYER
#ifdef DLAYER
#include <QDebug>
#endif

using namespace nbmap;
using namespace protocol;
using namespace nbcommon;

//static const uint32 MAX_PERSISTENT_METADATA_PATH_LENGTH = 512;

// Define in the metadata protocol.
static const char METADATA_PARAMETER_FORMAT[]            = "$fmt";
static const char METADATA_PARAMETER_FORMAT_VERSION[]    = "$vfmt";
static const char METADATA_PARAMETER_LOCALE[]            = "$loc";
static const char METADATA_PARAMETER_TOD[]               = "$tod";
//static const char METADATA_PARAMETER_Q[]                 = "$q";
static const char METADATA_PARAMETER_RESOLUTION[]        = "$res";
static const char METADATA_PARAMETER_SIZE[]              = "$sz";
static const char METADATA_PARAMETER_VERSION[]           = "$v";
//static const char METADATA_PARAMETER_X[]                 = "$x";
//static const char METADATA_PARAMETER_Y[]                 = "$y";
//static const char METADATA_PARAMETER_Z[]                 = "$z";
static const char METADATA_PARAMETER_APIKEY[]            = "$apikey";


// Define default parameter
// Set DEFAULT_LOCALE to empty for default so server will handle it automatically.
static const char METADATA_LOCALE_LOCAL[]                = "";
static const char METADATA_TOD_DAY[]                     = "day";
static const char METADATA_TOD_NIGHT[]                   = "night";
static const char METADATA_LOCALE_ENGLISH[]              = "en-US";
static const char METADATA_DEFAULT_RESOLUTION[]          = "192";
static const char METADATA_DEFAULT_TILE_SIZE[]           = "256";
static const char METADATA_DEFAULT_VERSION[]             = "1.0";
//static const unsigned int DEFALUT_RESOLUTION             = 192;

// Define image format
//static const char METADATA_IMAGE_FORMAT_PNG[]            = "PNG";
//static const char METADATA_IMAGE_FORMAT_PNG_LOWER_CASE[] = "png";
//static const char METADATA_IMAGE_FORMAT_GIF[]            = "GIF";
//static const char METADATA_IMAGE_FORMAT_JPG[]            = "JPG";
static const char METADATA_IMAGE_FORMAT_NBM[]            = "NBM";

// Name of the cache for saving tiles
// @todo: Temporary solution - use NBM folder as cache folder. This code should be removed after
//        logic for tiles loading from buffer will be implemented!!!
static const char* TILE_CACHING_NAME      = "NBM";
static const char* ANIMATION_CACHING_NAME = "ANIMATION";
//static const char TILE_CACHING_NAME[] = "TileCache";

// Time interval when metadata request failed, UNIT: milliseconds;
static const uint32 METADATA_RETRY_INTERVAL_MSEC  = 5000;
// Time interval between two Metadata request. UNIT: seconds.
//static const uint64 METADATA_REQUEST_INTERVAL_SEC = 3600;
// Unlimited metadata retry during navigation.

//@TODO: This value should be configured by MapServiceConfiguration.
static const uint32 MAX_ANIMATION_TILES_COUNT = 1000;
static const uint32 MAX_METADATA_RETRY_TIMES  = 3;

typedef map<string,string>   StringMap;

/*! Functor to remove specified content from a cache. */
class RemoveCacheFunctor
{
public:
    RemoveCacheFunctor(shared_ptr<string> type, shared_ptr<string> name)
            : m_name(name),
              m_type(type)
    {
    }

    virtual ~RemoveCacheFunctor()
    {
    }

    void operator() (const pair<string, CachePtr>& iter) const
    {
        if (iter.second)
        {
            iter.second->RemoveData(m_type, m_name);
        }
    }
private:
    shared_ptr<string> m_name;
    shared_ptr<string> m_type;
};

class FormatFinder
{
public:
    FormatFinder(uint32 nbgmVersion)
            : m_nbgmVersion(nbgmVersion) {}
    virtual ~FormatFinder(){}

    bool operator() (const shared_ptr<Format>& format) const
    {
        bool result = false;
        if (format)
        {
            uint32 maxNbgmVersion = static_cast<uint32>(format->GetMaxVersion());
            uint32 minNbgmVersion = static_cast<uint32>(format->GetMinVersion());
            result = (m_nbgmVersion >= minNbgmVersion) &&
                     (m_nbgmVersion <= maxNbgmVersion) &&
                     format->GetName() && !format->GetName()->empty();
        }
        return result;
    }

private:
    uint32 m_nbgmVersion;
};

class UnifiedLayerListenerFinder
{
public:
    UnifiedLayerListenerFinder(UnifiedLayerListener* listener)
            : m_pListener(listener)
    {
    }

    virtual ~UnifiedLayerListenerFinder()
    {
    }

    bool operator() (UnifiedLayerListener* listener) const
    {
        return (m_pListener == listener);
    }

    UnifiedLayerListener* m_pListener;
};

// Implementation of UnifiedLayerProvider.
UnifiedLayerProvider::UnifiedLayerProvider(NB_Context* context,
                                             uint32 nbgmVersion,
                                             uint32 maxTileRequestCountPerLayer,
                                             uint32 maxCachedTileCount,
                                             HybridMode mode)
        : m_pContext(context),
          m_nbgmVersion(nbgmVersion),
          m_maximumCachingTileCount(maxCachedTileCount),
          m_maximumTileRequestCountPerLayer(maxTileRequestCountPerLayer),
          m_minRasterZoomLevel(numeric_limits<uint32>::max()),
          m_maxRasterZoomLevel(numeric_limits<uint32>::min()),
          m_pCallback(new CheckMetadataChangesCallback<UnifiedLayerProvider> (
                          this, &UnifiedLayerProvider::MetadataUpdated)),
          m_pMaterialCallback(new CommonMaterialCallback(this)),
          m_preferredLanguageCode(DEFAULT_LANGUAGE_CODE),
          m_pMutableParameters(new map<string, string>()),
          m_metadataRetryCounter(0),
          m_clientGuid(CCC_NEW string),
          m_mapTheme(MVT_DAY)
{
    m_onoffBoardMode = mode;
    m_pMetadataProcessor = UnifiedService::GetMetadataProcessor(context, mode);
    m_contextCallback.callback     = &(UnifiedLayerProvider::MasterClear);
    m_contextCallback.callbackData = (void*)this;
    NB_ContextRegisterMasterClearCallback(m_pContext, &m_contextCallback);
    (*m_pMutableParameters)[METADATA_PARAMETER_LOCALE] = METADATA_LOCALE_LOCAL;
    (*m_pMutableParameters)[METADATA_PARAMETER_TOD] = METADATA_TOD_DAY;
    NB_NetworkNotifyEventClientGuid guidInfo = {NULL, 0};
    NB_Error error = NB_ContextGetClientGuid(context, &guidInfo);
    if (error == NE_OK)
    {
        // Convert the information of client guid to a string.
        m_clientGuid->assign(StringUtility::BinaryBufferToString(reinterpret_cast<const unsigned char*>(guidInfo.guidData), guidInfo.guidDataLength));
    }

    // Add the callback for event of client guid assigned.
    NB_NetworkNotifyEventCallback callback = {NULL, NULL};
    callback.callback = (NB_NetworkNotifyEventCallbackFunction) LayerProviderNetworkCallback;
    callback.callbackData = this;
    CSL_NetworkAddEventNotifyCallback(NB_ContextGetNetwork(context), &callback);
    
    (void) NB_ContextGetCredential(m_pContext, &m_credential);
}

UnifiedLayerProvider::~UnifiedLayerProvider()
{
    m_pCallback->SetInvalid();
    NB_ContextUnregisterMasterClearCallback(m_pContext, &m_contextCallback);
    if (m_credential != NULL)
    {
        nsl_free(m_credential);
    }
}

NB_Context* UnifiedLayerProvider::GetContext()
{
    return m_pContext;
}

/* See header file for description */
uint32 UnifiedLayerProvider::GetNbgmVersion()
{
    return m_nbgmVersion;
}

/* See header file for description */
shared_ptr<std::string> UnifiedLayerProvider::GetCachePath()
{
    return m_cachePath;
}

/* See header file for description */
uint32
UnifiedLayerProvider::GetMaximumCachingTileCount()
{
    return m_maximumCachingTileCount;
}

/* See header file for description */
uint32
UnifiedLayerProvider::GetMaximumTileRequestCountPerLayer()
{
    return m_maximumTileRequestCountPerLayer;
}

/* See header file for description */
NB_Error UnifiedLayerProvider::SetCachePath(shared_ptr<string> cachePath)
{
    m_cachePath = cachePath;
    return NE_OK;
}

/* See description in LayerProvider.h */
NB_Error
UnifiedLayerProvider::GetBackgroundRasterRange(uint32& minZoomLevel,
                                                uint32& maxZoomLevel)
{
    // It is invalid if minimum zoom level is greater than maximum zoom level.
    if (m_minRasterZoomLevel > m_maxRasterZoomLevel)
    {
        return NE_NOENT;
    }

    minZoomLevel = m_minRasterZoomLevel;
    maxZoomLevel = m_maxRasterZoomLevel;
    return NE_OK;
}

const vector<LayerPtr>& UnifiedLayerProvider::GetLayerPtrVector() const
{
    return m_pLayers;
}

const vector<LayerPtr>& UnifiedLayerProvider::GetCommonMaterialLayers() const
{
    return m_pCommonMaterialLayers;
}

void
UnifiedLayerProvider::GetLayers(shared_ptr<AsyncCallback<const vector<LayerPtr>& > > callback,
                                 shared_ptr<MetadataConfiguration> metadataConfig)
{
    if (!callback)
    {
        //No way to inform caller about the error,
        //since callback is null. So just return.
        return;
    }

    if (m_pContext == NULL || !m_pLayerIdGenerator)
    {
        callback->Error(NE_NOTINIT);
        return;
    }

    if (!m_pCallback)
    {
        callback->Error(NE_NOMEM);
    }
    else
    {
        m_callbackList.push_back(callback);
        m_pMetadataProcessor->CheckMetadataChanges(m_pCallback, metadataConfig, false, false);
    }
    return;
}

/* See description in LayerProvider.h */
void
UnifiedLayerProvider::ClearLayers()
{
    // Must cancel current metadata request.
    if (m_pMetadataProcessor)
    {
        m_pMetadataProcessor->Reset();
    }

    // Clear outstanding requests.
    if (!m_pLayers.empty())
    {
        vector<LayerPtr>::iterator iter = m_pLayers.begin();
        vector<LayerPtr>::iterator end  = m_pLayers.end();
        for (; iter != end; ++iter)
        {
            (*iter)->RemoveAllTiles();
        }
    }

    // Clear saved layers.
    m_pLayers.clear();
    m_pCommonMaterialLayers.clear();
    m_pLayerAvailabilityMatrix.reset();
    m_pDAMLayer.reset();
    m_pOptionalLayers.clear();

    // Reset min and max zoom levels of raster layers.
    m_minRasterZoomLevel = numeric_limits<uint32>::max();
    m_maxRasterZoomLevel = numeric_limits<uint32>::min();
}

const LayerPtr UnifiedLayerProvider::GetLayerAvailabilityMatrix() const
{
    return m_pLayerAvailabilityMatrix;
}

const LayerPtr UnifiedLayerProvider::GetDAMLayer() const
{
    return m_pDAMLayer;
}

void UnifiedLayerProvider::SetPreferredLanguageCode(uint8 languageCode)
{
    if (m_preferredLanguageCode != languageCode)
    {
        m_preferredLanguageCode = languageCode;
        if (!m_pMutableParameters)
        {
            m_pMutableParameters.reset(new map<string, string>);
            if (!m_pMutableParameters)
            {
                return;
            }
        }

        // According to current requirement, the client should set the 'loc' parameter
        // (defined as a variable in the URL template) to either 'en-us' or an empty
        // string based on the English/Local toggle.
        string locale(METADATA_LOCALE_ENGLISH);
        if (languageCode == DEFAULT_LANGUAGE_CODE)
        {
            locale = METADATA_LOCALE_LOCAL;
        }
        (*m_pMutableParameters)[METADATA_PARAMETER_LOCALE] = locale;
    }
}

void UnifiedLayerProvider::SetTheme(MapViewTheme theme)
{
    if (m_mapTheme != theme)
    {
        m_mapTheme = theme;
        if (!m_pMutableParameters)
        {
            m_pMutableParameters.reset(new map<string, string>);
            if (!m_pMutableParameters)
            {
                return;
            }
        }

        // According to current requirement, the client should set the 'tod' parameter
        // (defined as a variable in the URL template) to either 'day' or 'night'.
        string tod(METADATA_TOD_DAY);
        if (theme == MVT_NIGHT)
        {
            tod = METADATA_TOD_NIGHT;
        }
        (*m_pMutableParameters)[METADATA_PARAMETER_TOD] = tod;
    }
}

void UnifiedLayerProvider::SetMetadataRetryTimes(int number)
{
    m_pMetadataProcessor->SetMetadataRetryTimes(number);
}

NB_Error UnifiedLayerProvider::SetPersistentMetadataPath(shared_ptr<string> metadataPath)
{
    return m_pMetadataProcessor->SetPersistentMetadataPath(metadataPath);
}

void UnifiedLayerProvider::CheckMetadataChanges(shared_ptr<AsyncCallback<bool> > callback, shared_ptr<MetadataConfiguration> config, bool skipTimeInterval, bool forceUpdate)
{
    m_pMetadataProcessor->CheckMetadataChanges(callback, config, skipTimeInterval, forceUpdate);
}

void UnifiedLayerProvider::MasterClear(void* pTileManagerObject)
{
    UnifiedLayerProvider * pThis = (UnifiedLayerProvider*) pTileManagerObject;

    if (pThis == NULL)
    {
        return;
    }

    // Get context and PAL instance.
    if (pThis->m_pMetadataProcessor)
    {
        pThis->m_pMetadataProcessor->Reset();
    }

    shared_ptr<string> emptyPointer;
    if (pThis->m_pPersistentCache)
    {
        pThis->m_pPersistentCache->RemoveData(emptyPointer, emptyPointer);
    }

    RemoveCacheFunctor functor(emptyPointer, emptyPointer);
    for_each (pThis->m_animationCaches.begin(), pThis->m_animationCaches.end(), functor);

    // Reset members.
    pThis->m_pLayerIdGenerator->Reset();
    pThis->m_pMetadataProcessor->Reset();
}

NB_Error
UnifiedLayerProvider::UpdateLayersBasedOnMetadataResponse(MetadataSourceInformationSharedPtr response)
{
    // Check if the NB_Context and NB_PalInstance is valid.
    if (!m_pContext || (NB_ContextGetPal(m_pContext) == NULL))
    {
        return NE_NOTINIT;
    }

    // Check if the responsed metadata source information is NULL.
    if (!response)
    {
        return NE_UNEXPECTED;
    }

    // Check it again to make sure this response is valid.
    if (!response || !response->GetContentSourceArray() ||
        response->GetContentSourceArray()->empty())
    {
        return NE_UNEXPECTED;
    }

    /* @todo: 1. All tile requests are removed from the tile managers when the metadata is
       updated. I think the navigation should add requests again when the GPS is
       updated.
       2. Should I remove the cached data if the layer is removed from the new
       metadata? These cached data can also be removed when it is least recently
       used.
    */

    m_pCommonMaterialLayers.clear();
    m_pLayers.clear();
    m_pOptionalLayers.clear();
    m_materialStatistics.clear();

    // Create and initialize a persistent cache for all tiles.
    if (!m_cachePath)
    {
        return NE_INVAL;
    }
 	shared_ptr<string> cachingName(new string(TILE_CACHING_NAME));
	m_pPersistentCache.reset(new Cache(m_pContext, m_maximumCachingTileCount, cachingName, m_cachePath));
    shared_ptr<string> citySummaryDisplay(new string(*(response->GetCitySummaryDisplay())));
    if (!citySummaryDisplay)
    {
        return NE_NOMEM;
    }

    // Initialize min zoom level with max integer and max zoom level with min integer.
    shared_ptr<vector<shared_ptr<ContentSource> > > contentSourceArray = response->GetContentSourceArray();
    vector<shared_ptr<ContentSource> >::iterator       it  = contentSourceArray->begin();
    vector<shared_ptr<ContentSource> >::const_iterator end = contentSourceArray->end();
    for (; it != end; ++it)
    {
        //Each Content Source will have a country and projection
        shared_ptr<ContentSource> contentSource = *it;

        shared_ptr<string> country(new string(*(contentSource->GetCountry())));
        shared_ptr<string> projection(new string(*(contentSource->GetProjection())));
        shared_ptr<string> baseURL(new string(*(contentSource->GetUrl()->GetValue())));
        if (!country || !projection || !baseURL)
        {
            return NE_NOMEM;
        }

        bool isInternalSource = contentSource->GetInternalSource();

        //Each contentSource Will contain Multiple Args Template.
        shared_ptr<vector< shared_ptr<UrlArgsTemplate> > > urlArgsArray = contentSource->GetUrlArgsTemplateArray();

        if (!urlArgsArray || urlArgsArray->empty())
        {
            continue;
        }

        // Add to QaLog.
        NB_QaLogMetadataSourceResponse(m_pContext,
                                       citySummaryDisplay->c_str(),
                                       baseURL->c_str(),
                                       projection->c_str(),
                                       urlArgsArray->size());

        vector<shared_ptr<UrlArgsTemplate> >::const_iterator iter = urlArgsArray->begin();
        vector<shared_ptr<UrlArgsTemplate> >::const_iterator end  = urlArgsArray->end();
        for (; iter != end; ++iter)
        {
            const shared_ptr<UrlArgsTemplate>& urlArgsTemplate = *iter;
            if (!urlArgsTemplate)
            {
                continue;
            }

//            if (urlArgsTemplate->GetType()->compare("DAM") == 0)
//            {
//                continue;
//            }

            shared_ptr<UnifiedLayer> unifiedLayer =
                    CreateAndGroupUnifiedLayer(baseURL, urlArgsTemplate);
            if (!unifiedLayer)
            {
                continue;
            }

            // Update necessary information.
            unifiedLayer->SetCountry(country);
            unifiedLayer->SetProjection(projection);
            unifiedLayer->SetIsInternal(isInternalSource);

            shared_ptr<string> type = unifiedLayer->GetTileDataType();
            if (type)
            {
                m_layerMap.insert(map<string, UnifiedLayerPtr> ::value_type(*type, unifiedLayer));
            }
        }
    }

    PostCreateLayers();
    return NE_OK;
}

void UnifiedLayerProvider::SetLayerIdGenerator(LayerIdGeneratorPtr idGenerator)
{
    m_pLayerIdGenerator = idGenerator;
}

bool
UnifiedLayerProvider::CheckErrorCode(NB_Error err)
{
    bool isNeedSpecialHandle = false;
    switch (err)
    {
        case NESERVERX_PROCESSING_ERROR:
        case NESERVERX_UNSUPPORTED_ERROR:
        case NESERVERX_INTERNAL_ERROR:
        case NE_UNEXPECTED:
        {
            isNeedSpecialHandle = true;
            break;
        }

        default:
            break;
    }
    return isNeedSpecialHandle;
}

void UnifiedLayerProvider::MetadataUpdated(bool changed, NB_Error err)
{
    /*PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
                "%s: changed = %d, sizeof(m_layers) = %u",
                __FUNCTION__, (int)changed, m_pLayers.size());
      */
    NB_Error error = NE_OK;
    bool invokeCallback = true;
    bool isNeedSpecialHandle = CheckErrorCode(err);
    error = isNeedSpecialHandle ? err : error;

    if ((!isNeedSpecialHandle) && (changed || m_pLayers.empty()))
    {
        MetadataSourceInformationSharedPtr information =
                m_pMetadataProcessor->GetMetadataSourceInformation();
        error = UpdateLayersBasedOnMetadataResponse(information);
        if (error == NE_OK)
        {
            // DownloadMaterialLayers.
            vector<LayerPtr>::iterator iter = m_pCommonMaterialLayers.begin();
            vector<LayerPtr>::iterator end  = m_pCommonMaterialLayers.end();
            for (; iter != end; ++iter)
            {
                nsl_assert(iter->get() != NULL);
                CommonMaterialLayer* cLayer = static_cast<CommonMaterialLayer*>((*iter).get());
                if (cLayer)
                {
                    cLayer->GetMaterial(m_pMaterialCallback, HIGHEST_TILE_REQUEST_PRIORITY);
                    invokeCallback = false;
                }
            }

            //@TODO: Create special DAMLayer and hide TileKey conversions.
            // Start downloading DAM tiles.
            UnifiedLayer* damLayer = static_cast<UnifiedLayer*>(m_pDAMLayer.get());
            if (damLayer)
            {
                vector<TileKeyPtr> tileKeys;
                int damReferenceGridLevel = damLayer->GetReferenceTileGridLevel();
                int tileCount = static_cast<int>(pow(2.0, damReferenceGridLevel));
                for (int x = 0; x < tileCount; ++x)
                {
                    for (int y = 0; y < tileCount; ++y)
                    {
                        TileKeyPtr tileKey(new TileKey(x, y, damReferenceGridLevel));
                        tileKeys.push_back(tileKey);
                    }
                }
                damLayer->GetTiles(tileKeys,
                                   shared_ptr<AsyncCallbackWithRequest<TileKeyPtr, TilePtr> >(),
                                   0xFFFFFF00);
            }
        }
        if (invokeCallback)
        {
            InvokeCallbacks(error);
        }
    }
}

bool UnifiedLayerProvider::Contain(std::vector<LayerPtr> layers, LayerPtr layer)
{
#ifdef DLAYER
    qWarning() << "UnifiedLayerProvider::Contain(" << layer->str().c_str() << ")";
#endif
    for(unsigned int i = 0; i < layers.size(); i++)
    {
        if(layer && layers[i])
        {
            shared_ptr<string> lhs = layer->GetLayerName();
            shared_ptr<string> rhs = layers[i]->GetLayerName();
            if (lhs && rhs)
            {
                if (*lhs == *rhs)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void UnifiedLayerProvider::AppendIfNotContain(std::vector<LayerPtr>& lhsLayers, const std::vector<LayerPtr>& rhsLayers)
{
    for(unsigned int i = 0; i < rhsLayers.size(); i++)
    {
        if(!Contain(lhsLayers, rhsLayers[i]))
        {
            lhsLayers.push_back(rhsLayers[i]);
        }
    }
}

void UnifiedLayerProvider::CopyLayersFrom(const UnifiedLayerProvider &layerProvider)
{
    std::vector<LayerPtr> optionalLayers = layerProvider.GetOptionalLayerPtrVector();
    AppendIfNotContain(m_pOptionalLayers, optionalLayers);
}

/* See description in header file. */
const std::vector <LayerPtr>&  UnifiedLayerProvider::GetOptionalLayerPtrVector() const
{
    return m_pOptionalLayers;
}

/* See description in header file. */
void
UnifiedLayerProvider::ProcessBackgroundLayers()
{
    map<uint32, vector<UnifiedLayerPtr> >::iterator backgroundLayerIter = m_backgroundLayerMap.begin();
    map<uint32, vector<UnifiedLayerPtr> >::iterator backgroundLayerEnd  = m_backgroundLayerMap.end();
    for (; backgroundLayerIter != backgroundLayerEnd; ++backgroundLayerIter)
    {
        vector<UnifiedLayerPtr>& lowDrawOrderLayers = backgroundLayerIter->second;
        for(size_t i = 0; i<lowDrawOrderLayers.size(); ++i)
        {
            for(size_t j = 0; j<lowDrawOrderLayers.size(); ++j)
            {
                if(i != j)
                {
                    lowDrawOrderLayers[i]->AddMutexLayer(lowDrawOrderLayers[j].get());
                }
            }
        }
    }

    m_backgroundLayerMap.clear();
}

/* See description in header file. */
void
UnifiedLayerProvider::ProcessDeactiveLayers()
{
    DEACTIVATING_LAYER_MAP::iterator iter = m_deactivatingLayerMap.begin();
    DEACTIVATING_LAYER_MAP::iterator end  = m_deactivatingLayerMap.end();
    for (; iter != end; ++iter)
    {
       const string&             dataType = iter->first;
        vector<UnifiedLayerPtr>& layers   = iter->second;
        if (layers.empty())
        {
            continue;
        }

        map<string, UnifiedLayerPtr>::iterator layerIter = m_layerMap.find(dataType);
        if (layerIter == m_layerMap.end() || !layerIter->second)
        {
            continue;
        }

        UnifiedLayerPtr& deactiveLayer = layerIter->second;
        {
            vector<UnifiedLayerPtr>::const_iterator iter = layers.begin();
            vector<UnifiedLayerPtr>::const_iterator end  = layers.end();
            for (; iter != end; ++iter)
            {
                (*iter)->AddDeactivatingLayer(deactiveLayer.get());
            }
        }
    }

    m_deactivatingLayerMap.clear();
}

/* See description in header file. */
void UnifiedLayerProvider::PostCreateLayers()
{
    // Associate Animation layers with their frame list layer.
    vector<AnimationLayerPtr>::iterator iter = m_pAnimationLayers.begin();
    vector<AnimationLayerPtr>::iterator end  = m_pAnimationLayers.end();
    for (; iter != end; ++iter)
    {
        AnimationLayerPtr& animationLayer = *iter;
        if (!animationLayer)
        {
            continue;
        }

        shared_ptr<string> frameListLayerId = animationLayer->GetFrameListLayerID();
        if (!frameListLayerId || frameListLayerId->empty())
        {
            continue;
        }

        map<string, FrameListLayerPtr>::iterator result =
                m_frameListLayers.find(*frameListLayerId);
        if (result == m_frameListLayers.end() || !result->second)
        {
            continue;
        }

        animationLayer->SetFrameListLayer(result->second);
    }

    m_pAnimationLayers.clear();
    m_frameListLayers.clear();

    ProcessBackgroundLayers();
    ProcessDeactiveLayers();

    // Find parent layer based on name of parent layer.
    map<string, UnifiedLayerPtr>::iterator layerIter = m_layerMap.begin();
    map<string, UnifiedLayerPtr>::iterator layerEnd  = m_layerMap.end();
    for (; layerIter != layerEnd; ++layerIter)
    {
        UnifiedLayerPtr layer = layerIter->second;
        if (!layer)
        {
            continue;
        }

        shared_ptr<string> parentLayerType = layer->GetParentLayerType();
        if (!parentLayerType)
        {
            // Tiles from some layers should not be passed to MapView and NBGM, don't add
            // these kinds layers into m_pLayers or m_pOptionalLayers.
            if (layer->IsDisplayable())
            {
                // Only layers that are not children will be recorded here, children layers
                // will be added to MapView dynamically when parent layer is enabled by
                // LayerAgents.
                if (layer->IsOptional())
                {
                    m_pOptionalLayers.push_back(layer);
                }
                else
                {
                    m_pLayers.push_back(layer);
                }
            }
            continue;
        }

        map<string, UnifiedLayerPtr>::iterator parentIter = m_layerMap.find(*parentLayerType);
        if (parentIter == layerEnd || !(parentIter->second))
        {
            continue;
        }
        UnifiedLayerPtr& parentLayer = parentIter->second;
        layer->SetParentLayer(parentLayer.get());
        if(parentLayer->IsOptional())
        {
            parentLayer->AddChildrenLayer(layer);
        }
    }

    // Clear m_layerMap to release memory.
    m_layerMap.clear();
}

// @todo: There are lots of hard-coded data types in this function, all of these should be
//        removed after server can return information in the characteristics of layers.
/* See description in header file. */
shared_ptr <UnifiedLayer>
UnifiedLayerProvider::CreateAndSpecifyLayer(const string& category,
                                             UnifiedTileTypePtr tileType,
                                             shared_ptr<string> format,
                                             shared_ptr<UrlArgsTemplate> urlArgsTemplate)
{
    uint32             layerId  = m_pLayerIdGenerator->GenerateId();
    shared_ptr<string> dataType = tileType ? tileType->GetDataType() : shared_ptr<string>();
    if (layerId == LayerIdGenerator::InvalidId || !dataType ||
        dataType->empty() ||  !urlArgsTemplate)
    {
        return UnifiedLayerPtr();
    }

    //Create a caching tile manager here.
    CachingTileManagerPtr cachingTileManager(new CachingTileManager());
    if (!cachingTileManager)
    {
        return UnifiedLayerPtr();
    }

    // Set the common parameters for the tile manager.
    shared_ptr<map<string, string> > commonParameters(new map<string, string>());
    if (!commonParameters)
    {
        return UnifiedLayerPtr();
    }

    string nbgmVersionString = StringUtility::NumberToString(m_nbgmVersion);
    commonParameters->insert(make_pair(METADATA_PARAMETER_FORMAT_VERSION,
                                       nbgmVersionString));
    commonParameters->insert(make_pair(METADATA_PARAMETER_FORMAT, *format));

    if (m_credential != NULL)
    {
        commonParameters->insert(make_pair(METADATA_PARAMETER_APIKEY, string(m_credential)));
    }

    UnifiedLayerPtr  layer;

    CachePtr cache          = m_pPersistentCache;
    bool     clearFlag      = false;

    // Generally, tiles of non-NBM format needs to be converted to NBM format.
    bool     needsConvert   = format->compare(METADATA_IMAGE_FORMAT_NBM) != 0;

    // Generally, only Tiles of NBM format is compressed by server.
    bool     compressed     = !needsConvert;
    uint32   lamIndex       = urlArgsTemplate->GetLamIndex();
    uint32   tileRetryDelay = 0;

#ifdef DLAYER
    qWarning() << "Category: " << category.c_str() << ", format: " << (*format).c_str() << ", type: " << (*dataType).c_str();
#endif

    if (!category.empty()) // Category is specified, create layer for this category.
    {
        if (!category.compare(LAYER_CATEGORY_MATERIAL)) // material layer.
        {
            layer.reset(new CommonMaterialLayer(cachingTileManager, layerId, m_pContext));
            if (layer)
            {
                m_pCommonMaterialLayers.push_back(layer);
                m_materialStatistics.insert(make_pair(layerId, TDS_NotStart));
                tileRetryDelay = METADATA_RETRY_INTERVAL_MSEC;
            }
        }
        else if (!category.compare(LAYER_CATEGORY_ROUTE)) // route layers.
        {
            layer.reset(new RouteLayer(cachingTileManager, layerId, m_pContext));
            clearFlag = true;
        }
        else if (!category.compare(LAYER_CATEGORY_TRAFFIC)) // traffic layers.
        {
            layer.reset(new RefreshLayer(cachingTileManager, layerId, m_pContext));
            clearFlag = true;
        }
        else if (!category.compare(LAYER_CATEGORY_WEATHER)) // weather layers
        {
            if (dataType->find(TILE_TYPE_WEATHER_FRAME_LIST) != string::npos)
            {
                FrameListLayerPtr frameListLayer(new FrameListLayer(m_pContext, layerId,
                                                                    cachingTileManager));
                if (frameListLayer && frameListLayer->GetTileLayerInfo())
                {
                    cache        = CachePtr();
                    layer        = frameListLayer;
                    needsConvert = false;
                    // Should be unique, so use a map instead of vector.
                    m_frameListLayers.insert(make_pair(*dataType, frameListLayer));
                }
            }
            else
            {
                AnimationLayerPtr animationLayer(new AnimationLayer(m_pContext, layerId,
                                                                    cachingTileManager));
                if (animationLayer && animationLayer->GetTileLayerInfo())
                {
                    // Each type of animation layer will have its own cache. Here we first
                    // try to find if we can find one, if not, we need to create new one and
                    // add it to m_animationCaches.
                    cache = CachePtr();
                    map<string, CachePtr>::iterator iter = m_animationCaches.find(*dataType);
                    if (iter == m_animationCaches.end())
                    {
                        shared_ptr<string> cachingName(new string(ANIMATION_CACHING_NAME));
                        if (cachingName && m_cachePath && !m_cachePath->empty())
                        {
                            cache.reset(new Cache(m_pContext,
                                                  MAX_ANIMATION_TILES_COUNT,
                                                  cachingName,
                                                  m_cachePath));
                            if (cache && cache->IsInitialized())
                            {
                                //Remove all animation tiles in cache.
                                shared_ptr<string> emptyPointer;
                                cache->RemoveData(emptyPointer, emptyPointer);
                                m_animationCaches.insert(make_pair(*dataType, cache));
                            }
                        }
                    }
                    else
                    {
                        cache = iter->second;
                    }

                    clearFlag = true;
                    animationLayer->SetMapLegend(urlArgsTemplate->GetMapLegned());
                    animationLayer->SetAnimationInfo(urlArgsTemplate->GetAnimationInfo());

                    // @note: do not put it into OptionalLayerList, it will be put there
                    //        when it is fully initialized.
                    m_pAnimationLayers.push_back(animationLayer);
                    layer = animationLayer;
                }
            }
        }
//        else // Preserved for other layers that will be added in future.
//        {
//            PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
//                        "%s: category: %s, dataType: %s",
//                        __FUNCTION__, category.c_str(), dataType->c_str());
//        }
    }

    if (!layer)
    {
        if (*dataType == TILE_TYPE_LAYER_AVAILABILITY_MATRIX)       // LAM layer
        {
            layer.reset(new LayerAvailabilityMatrixLayer(cachingTileManager, layerId, m_pContext));
            if (layer)
            {
                m_pLayerAvailabilityMatrix = layer;
                tileRetryDelay = METADATA_RETRY_INTERVAL_MSEC;
                compressed     = true;
                needsConvert   = false;
            }
        }
        else if (*dataType == TILE_TYPE_DATA_AVAILABILITY_MATRIX)       // DAM layer
        {
            layer.reset(new UnifiedLayer(cachingTileManager, layerId, m_pContext));
            if (layer)
            {
                m_pDAMLayer  = layer;
                m_pDAMLayer->SetDisplayable(false);
                compressed   = true;
                needsConvert = false;
            }
        }
        //@note: should be removed when all servers are updated to the latest!
        else if (*dataType == TILE_TYPE_COMMON_MATERIALS || *dataType == "NMAT" ||
                 *dataType == "DMAT" || *dataType == "PMAT") // Common material layer
        {
            layer.reset(new CommonMaterialLayer(cachingTileManager, layerId, m_pContext));
            if (layer)
            {
                m_pCommonMaterialLayers.push_back(layer);
                m_materialStatistics.insert(make_pair(layerId, TDS_NotStart));
                tileRetryDelay = METADATA_RETRY_INTERVAL_MSEC;
            }
        }
        // Then, decide layer to create based on lam index.
        else if (lamIndex == INVALID_LAM_INDEX) // Layers with out LAM.
        {
            layer.reset(new UnifiedLayer(cachingTileManager, layerId, m_pContext));
        }
        else  // Layers with Lam
        {
            layer.reset(new UnifiedLayerWithLAM(cachingTileManager, layerId, m_pContext,
                                                &m_pLayerAvailabilityMatrix));
        }
    }

    TileLayerInfoPtr tileLayerInfo;
    if (layer && (tileLayerInfo = layer->GetTileLayerInfo()))
    {
        // While creating unified layer, the associated TileLayerInfo which pack all
        // information that can be shared by all tiles of the same layer.
        // If associated TileLayerInfo can not be returned, treat as error.
        // Do not reset or free this TileLayerInfo, it will be used to initialize
        // TileManageres later in this function.

        if (cache && clearFlag)
        {
            cache->RemoveData(dataType, shared_ptr<string>());
        }
        tileLayerInfo->cache = cache;
        layer->SetLayerAvailabilityMatrixIndex(lamIndex);
        layer->SetType(dataType);

        // All layers shared the same mutable parameters, so that when client changed
        // something like language code, all layers can be updated automatically.
        layer->SetMutableParameters(m_pMutableParameters);
    }

    UnifiedTileManager* unifiedTileManager = UnifiedService::GetTileManager(m_pContext, m_onoffBoardMode);
    if (!unifiedTileManager)
    {
        return UnifiedLayerPtr();
    }
    TileConverterPtr tileConverter;
    if (needsConvert)
    {
        tileConverter.reset(new RasterTileConverter());
        if (!tileConverter)
        {
            return UnifiedLayerPtr();
        }

        // Fill some common parameters for Non-NBM format tile requests.
        commonParameters->insert(pair<string, string>(METADATA_PARAMETER_VERSION,
                                                      METADATA_DEFAULT_VERSION));
        commonParameters->insert(pair<string, string>(METADATA_PARAMETER_RESOLUTION,
                                                      METADATA_DEFAULT_RESOLUTION));
        commonParameters->insert(pair<string, string>(METADATA_PARAMETER_SIZE,
                                                      METADATA_DEFAULT_TILE_SIZE));
        unifiedTileManager->SetTileConverter(tileConverter);
    }

    NB_Error error = unifiedTileManager->Initialize(m_pContext,
                                             urlArgsTemplate,
                                             compressed,
                                             tileRetryDelay,
                                             m_maximumTileRequestCountPerLayer,
                                             tileType,
                                             layer->GetTileLayerInfo(),
                                             m_clientGuid);

    TileManagerPtr tileManager;
    tileManager.reset(unifiedTileManager);
    // Initialize the caching tile manager.
    error = (error != NE_OK) ? error :
            cachingTileManager->Initialize(m_pContext, tileType, tileManager, dataType,
                                           urlArgsTemplate->GetDsGenId(),
                                           layer->GetTileLayerInfo());

    error = (error != NE_OK) ? error :
            cachingTileManager->SetCommonParameterMap(commonParameters);
    if (error != NE_OK)
    {
        layer.reset();
    }

    return layer;
}

/* See description in header file. */
shared_ptr <UnifiedLayer>
UnifiedLayerProvider::CreateAndGroupUnifiedLayer(shared_ptr<string> baseURL,
                                                  shared_ptr <UrlArgsTemplate> urlArgsTemplate)
{
    UnifiedLayerPtr    unifiedLayer;
    do
    {
        // We begin this function with bunch of sanity checks. Results of those checks may
        // not be used right after the check, but will be used later. All checks are grouped
        // here to avoid unnecessary memory allocation/deallocation.

        // If format array is present, then check if NBGM can handle the provided format.
        shared_ptr<string> formatName;
        uint32 maxNbgmVersion = 0;
        uint32 minNbgmVersion = 0;

        shared_ptr<vector<shared_ptr<Format> > > formatArray =
                urlArgsTemplate->GetFormatArray();
        if (formatArray)
        {
            FormatFinder functor(m_nbgmVersion);
            vector<shared_ptr<Format> >::iterator iter =
                    find_if (formatArray->begin(), formatArray->end(), functor);
            if (iter != formatArray->end())
            {
                formatName = (*iter)->GetName();
                maxNbgmVersion = static_cast<uint32>((*iter)->GetMaxVersion());
                minNbgmVersion = static_cast<uint32>((*iter)->GetMinVersion());
            }
            /* None of format contains current NBGM version (between minimum and
               maximum). And we do not initialize this layer to download tiles. Because
               NBGM could not display tiles of this layer. */
            if (!formatName)
            {
                break;
            }
        }

        shared_ptr<string> stringTemplate(new string(*(urlArgsTemplate->GetStringTemplate())));
        if(!stringTemplate)
        {
            break;
        }

        // Combine the base URL and URL template.
        shared_ptr<string> entireUrlTemplate(new string((*baseURL) + (*stringTemplate)));
        if (!entireUrlTemplate)
        {
            break;
        }

        // Check if data type is present.
        shared_ptr<string> type(new string(*(urlArgsTemplate->GetType())));
        if (!type)
        {
            break;
        }

        shared_ptr<string> tileStoreTemplate(
            new string(*(urlArgsTemplate->GetTileStoreTemplate())));
        if(!tileStoreTemplate)
        {
            break;
        }

        // Create a UnifiedTileType object.
        UnifiedTileTypePtr tileType(new UnifiedTileType(type, entireUrlTemplate,
                                                          tileStoreTemplate));
        if(!tileType)
        {
            break;
        }

        //*********************************************************************************
        // Get Characteristics, and create layer based on its category.
        shared_ptr<Characteristics> characteristics = urlArgsTemplate->GetCharacteristics();
        map<string, string> characteristicMap;
        if(characteristics)
        {
            shared_ptr<vector<shared_ptr<Pair> > > pairArray = characteristics->GetPairArray();
            if(pairArray)
            {
                for (unsigned int l=0; l <pairArray->size(); ++l)
                {
                    shared_ptr<string> key   = pairArray->at(l)->GetKey();
                    shared_ptr<string> value = pairArray->at(l)->GetValue();
                    if (key && value)
                    {
                        characteristicMap.insert(make_pair(*key, *value));
                    }
                }
            }
        }

        string category;
        if (characteristicMap.find(LAYER_CATEGORY_IDENTIFIER) != characteristicMap.end())
        {
            category = characteristicMap[LAYER_CATEGORY_IDENTIFIER];
        }

        if (characteristicMap.find(TILE_ADDITIONAL_KEY_NAME) == characteristicMap.end())
        {
            characteristicMap[TILE_ADDITIONAL_KEY_NAME] = *type;
        }

        // Create this layer, and fill some type-specified fields.
        unifiedLayer = CreateAndSpecifyLayer(category, tileType, formatName, urlArgsTemplate);
        if (!unifiedLayer)
        {
            break;
        }

        string level;
        uint32 layerRefGrid = 0;
        if (characteristicMap.find(LAYER_LEVEL_IDENTIFIER) != characteristicMap.end())
        {
            level = characteristicMap[LAYER_LEVEL_IDENTIFIER];
            if (level.compare(LAYER_LEVEL_BACKGROUND) == 0)
            {
                layerRefGrid = urlArgsTemplate->GetRefTileGridLevel();

                m_backgroundLayerMap[layerRefGrid].push_back(unifiedLayer);
                DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_INFO, ("***********Insert backgroundLayerMap[layerRefGrid:%d]=[%s]***********", layerRefGrid, type->c_str()));
            }
        }

        StringMap::iterator iter = characteristicMap.find(LAYER_DEACTIVATING_IDENTIFIER);
        if (iter != characteristicMap.end())
        {
            m_deactivatingLayerMap[iter->second].push_back(unifiedLayer);
        }

        // Now layer is created, fill up some generic fields.
        unifiedLayer->SetTileStoreTemplate(tileStoreTemplate);
        unifiedLayer->SetLabelDrawOrder(urlArgsTemplate->GetLabelDrawOrder());
        unifiedLayer->SetTileSize(urlArgsTemplate->GetTileSize());
        unifiedLayer->SetDrawOrder(urlArgsTemplate->GetBaseDrawOrder());
        unifiedLayer->SetLayerAvailabilityMatrixIndex(urlArgsTemplate->GetLamIndex());
        unifiedLayer->SetTemplate(stringTemplate);
        unifiedLayer->SetDatasetGenerationID(urlArgsTemplate->GetDsGenId());
        unifiedLayer->SetDownloadPriority(urlArgsTemplate->GetDownloadPriority());
        unifiedLayer->SetReferenceTileGridLevel(urlArgsTemplate->GetRefTileGridLevel());
        unifiedLayer->SetCacheID(urlArgsTemplate->GetCacheId());
        unifiedLayer->SetCharacteristics(characteristicMap);
        unifiedLayer->SetMaxVersion(maxNbgmVersion);
        unifiedLayer->SetMinVerison(minNbgmVersion);
        unifiedLayer->SetFormatName(formatName);
        unifiedLayer->SetBaseURL(baseURL);
        unifiedLayer->ProcessCharacteristics();

        uint32 minZoomLevel = urlArgsTemplate->GetMinZoom();
        uint32 maxZoomLevel = urlArgsTemplate->GetMaxZoom();

        unifiedLayer->SetMinZoom(minZoomLevel);
        unifiedLayer->SetMaxZoom(maxZoomLevel);

        // Check with string 'BR' to confirm if it is a layer of raster.
        if (type && (type->compare(0, nsl_strlen(TILE_TYPE_BR), TILE_TYPE_BR) == 0))
        {
            // Get minimum and maximum zoom levels of raster layers.
            if (m_minRasterZoomLevel > minZoomLevel)
            {
                m_minRasterZoomLevel = minZoomLevel;
            }
            if (m_maxRasterZoomLevel < maxZoomLevel)
            {
                m_maxRasterZoomLevel = maxZoomLevel;
            }
        }


        // Add QaLog
        MetadataSharedPtrList metadataArray = urlArgsTemplate->GetMetadataArray();
        uint32 sizeOfMetadata =static_cast<uint32>(metadataArray->size());
        for (uint32 k = 0; k < sizeOfMetadata; k++)
        {
            MetadataSharedPtr metadata = metadataArray->at(k);
            BoxSharedPtr box = metadata->GetBox();
            ProjectionParametersSharedPtr projectionParameters =
                    metadata->GetProjectionParameters();
            NB_QaLogMetadataSourceTemplate(m_pContext,
                                           type->c_str(),
                                           stringTemplate->c_str(),
                                           urlArgsTemplate->GetCacheId(),
                                           metadata->GetId()->c_str(),
                                           metadata->GetManifestVersion()->c_str(),
                                           metadata->GetSize(),
                                           metadata->GetDisplayName()->c_str(),
                                           box->GetLeftCornerPoint()->GetLat(),
                                           box->GetLeftCornerPoint()->GetLon(),
                                           box->GetRightCornerPoint()->GetLat(),
                                           box->GetRightCornerPoint()->GetLon(),
                                           projectionParameters->GetDatum()->c_str(),
                                           projectionParameters->GetFalseEasting(),
                                           projectionParameters->GetFalseNorthing(),
                                           projectionParameters->GetOriginLatitude(),
                                           projectionParameters->GetOriginLongitude(),
                                           projectionParameters->GetScaleFactor(),
                                           projectionParameters->GetZOffset());
        }

    } while (0);

    return unifiedLayer;
}

/* See description in header file. */
void UnifiedLayerProvider::InvokeCallbacks(NB_Error error)
{
    vector<shared_ptr<AsyncCallback<const vector<LayerPtr>& > > > callbackList;
    callbackList.swap(m_callbackList);
    size_t size = callbackList.size();
    for (size_t i = 0; i < size; ++i)
    {
        const shared_ptr<AsyncCallback<const std::vector<LayerPtr>& > >&
                callback = callbackList.at(i);
        if (callback)
        {
            if (error == NE_OK)
            {
                callback->Success(m_pLayers);
            }
            else
            {
                callback->Error(error);
            }
        }
    }
}

/* See description in header file. */
void UnifiedLayerProvider::UpdateMaterialStatistics(uint32 layerId, NB_Error error)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

   // PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo,
   //             "%s: error = %d", __FUNCTION__, (int)error);
    if ((error == NE_HTTP_BAD_REQUEST || error == NE_HTTP_RESOURCE_NOT_FOUND) &&
        (++m_metadataRetryCounter <= MAX_METADATA_RETRY_TIMES))
    {
        m_pMetadataProcessor->CheckMetadataChanges(m_pCallback,
                                                   shared_ptr<MetadataConfiguration>(), true, false);
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "Request metadata-source again");
        return;
    }

    m_materialStatistics[layerId] = error == NE_OK ? TDS_Succeeded : TDS_Failed;

    NB_Error result = NE_OK;

    map<uint32, TileDownloadStatus>::const_iterator iter = m_materialStatistics.begin();
    map<uint32, TileDownloadStatus>::const_iterator end  = m_materialStatistics.end();
    for (; iter != end; ++iter)
    {
        if (iter->second == TDS_NotStart)
        {
            return; // return if some tiles are not downloaded yet.
        }
        else if (iter->second == TDS_Failed)
        {
            result = NE_NOTINIT;
        }
    }

    InvokeCallbacks(result);
}

// Implementation of UnifiedLayerProvider::CommonMaterialCallback, refer to header file for descriptions.
UnifiedLayerProvider::CommonMaterialCallback::CommonMaterialCallback(UnifiedLayerProvider* provider)
        : m_pLayerProvider(provider)
{
}

UnifiedLayerProvider::CommonMaterialCallback::~CommonMaterialCallback()
{
}

void UnifiedLayerProvider::CommonMaterialCallback::Success(TileKeyPtr request, TilePtr response)
{
    if (response)
    {
        m_pLayerProvider->UpdateMaterialStatistics(response->GetLayerID(), NE_OK);
    }
    else
    {
        Error(request, NE_INVAL);
    }
}

void UnifiedLayerProvider::CommonMaterialCallback::Error(TileKeyPtr request, NB_Error error)
{
    if (m_pLayerProvider)
    {
        uint32 layerId = 0;
        if (request)
        {
            // ZoomLevel of this tile is replaced by layer id in CommonMaterialLayer, in this
            // way, we can know the failed layer.
            layerId = static_cast<uint32>(request->m_zoomLevel);
        }

        m_pLayerProvider->UpdateMaterialStatistics(layerId, error);
    }
}

bool UnifiedLayerProvider::CommonMaterialCallback::Progress(int /*percentage*/)
{
    return true;
}

/* See description in header file. */
void UnifiedLayerProvider::LayerProviderNetworkCallback(NB_NetworkNotifyEvent event,
                                                         void* data,
                                                         UnifiedLayerProvider* layerProvider)
{
    if ((event == NB_NetworkNotifyEvent_ClientGuidAssigned) && layerProvider)
    {
        NB_NetworkNotifyEventClientGuid* guidInfo = (NB_NetworkNotifyEventClientGuid*) data;
        if (guidInfo)
        {
            // Convert the information of client guid to a string.
            layerProvider->m_clientGuid->assign(StringUtility::BinaryBufferToString(reinterpret_cast<const unsigned char*>(guidInfo->guidData), guidInfo->guidDataLength));
        }
    }
}

/*! @} */
