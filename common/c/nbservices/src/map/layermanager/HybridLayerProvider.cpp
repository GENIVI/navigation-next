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

#include "HybridLayerProvider.h"
#include "UnifiedLayerProvider.h"
#include "AggregateLayerProvider.h"
#include "nbcontext.h"

using namespace nbcommon;
namespace nbmap {
#define ON_BOARD_WORKFOLDER "/on_board"
#define OFF_BOARD_WORKFOLDER "/off_board"
#define ON_BOARD_METADATA_SOURCE "/on_board_metadata_source"
#define OFF_BOARD_METADATA_SOURCE "/off_board_metadata_source"

HybridLayerProvider::HybridLayerProvider(NB_Context* context, uint32 nbgmVersion, uint32 maxTileRequestCountPerLayer, uint32 maxCachedTileCount)
    :HybridModeListener(context)
{
    m_pContext = context;
    LayerIdGeneratorPtr layerIdGenerator = ContextBasedSingleton<LayerIdGenerator>::getInstance(context);
    m_onBoardLayerProvider.reset(new UnifiedLayerProvider(context,
                                                          nbgmVersion,
                                                          maxTileRequestCountPerLayer,
                                                          maxCachedTileCount,
                                                          HBM_ONBOARD_ONLY));
    m_offBoardLayerProvider.reset(new UnifiedLayerProvider(context,
                                                           nbgmVersion,
                                                           maxTileRequestCountPerLayer,
                                                           maxCachedTileCount,
                                                           HBM_OFFBOARD_ONLY));
    m_onBoardLayerProvider->SetLayerIdGenerator(layerIdGenerator);
    m_offBoardLayerProvider->SetLayerIdGenerator(layerIdGenerator);

    m_HybridManager = HybridManager::GetInstance(context);
    m_HybridMode = m_HybridManager->GetMode();
    m_onBoardCallback = shared_ptr<HybridLayerProviderCallback>(new HybridLayerProviderCallback(this, HBM_ONBOARD_ONLY));
    m_offBoardCallback = shared_ptr<HybridLayerProviderCallback>(new HybridLayerProviderCallback(this, HBM_OFFBOARD_ONLY));
    m_pAggregateLayerProvider = ContextBasedSingleton<AggregateLayerProvider>::getInstance(context);
    m_OnboardStatus = GLS_NotStart;
    m_OffboardStatus = GLS_NotStart;
}

HybridLayerProvider::~HybridLayerProvider()
{
    m_onBoardCallback->SetLayerProvider(NULL);
    m_offBoardCallback->SetLayerProvider(NULL);
}

void HybridLayerProvider::GetLayers(shared_ptr<AsyncCallback<const std::vector<LayerPtr> &> > callback, shared_ptr<MetadataConfiguration> metadataConfig)
{
    m_OffboardStatus = GLS_NotStart;
    m_OnboardStatus = GLS_NotStart;
    m_finalCallback = callback;
    m_onBoardLayerProvider->GetLayers(m_onBoardCallback, metadataConfig);
    m_offBoardLayerProvider->GetLayers(m_offBoardCallback, metadataConfig);
}

void HybridLayerProvider::ClearLayers()
{
    m_onBoardLayerProvider->ClearLayers();
    m_offBoardLayerProvider->ClearLayers();
}

const LayerPtr HybridLayerProvider::GetLayerAvailabilityMatrix() const
{
    return GetCurrentProvider()->GetLayerAvailabilityMatrix();
}

const std::vector<LayerPtr> &HybridLayerProvider::GetLayerPtrVector() const
{
    return GetCurrentProvider()->GetLayerPtrVector();
}

const std::vector<LayerPtr> &HybridLayerProvider::GetOptionalLayerPtrVector() const
{
    return GetCurrentProvider()->GetOptionalLayerPtrVector();
}

const std::vector<LayerPtr> &HybridLayerProvider::GetCommonMaterialLayers() const
{
    return GetCurrentProvider()->GetCommonMaterialLayers();
}

const LayerPtr HybridLayerProvider::GetDAMLayer() const
{
    return GetCurrentProvider()->GetDAMLayer();
}

NB_Error HybridLayerProvider::GetBackgroundRasterRange(uint32 &minZoomLevel, uint32 &maxZoomLevel)
{
    // If there is no zoom levels specified, return NE_NOENT.
    NB_Error error = NE_NOENT;
    uint32 minZoomLevelResult = 0XFFFFFFFF;
    uint32 maxZoomLevelResult = 0;

    // Get minimum and maximum zoom levels from layer provider.
    uint32 minZoomLevelGot = 0;
    uint32 maxZoomLevelGot = 0;
    NB_Error tempError = m_onBoardLayerProvider->GetBackgroundRasterRange(minZoomLevelGot, maxZoomLevelGot);
    if (tempError == NE_OK)
    {
        minZoomLevelResult = minZoomLevelGot < minZoomLevelResult? minZoomLevelGot : minZoomLevelResult;
        maxZoomLevelResult = maxZoomLevelGot > maxZoomLevelResult? maxZoomLevelGot : maxZoomLevelResult;
        error = NE_OK;
    }

    tempError = m_offBoardLayerProvider->GetBackgroundRasterRange(minZoomLevelGot, maxZoomLevelGot);
    if (tempError == NE_OK)
    {
        minZoomLevelResult = minZoomLevelGot < minZoomLevelResult? minZoomLevelGot : minZoomLevelResult;
        maxZoomLevelResult = maxZoomLevelGot > maxZoomLevelResult? maxZoomLevelGot : maxZoomLevelResult;
        error = NE_OK;
    }

    if (error != NE_OK)
    {
        return error;
    }

    minZoomLevel = minZoomLevelResult;
    maxZoomLevel = maxZoomLevelResult;
    return NE_OK;
}

NB_Error HybridLayerProvider::SetCachePath(shared_ptr<std::string> cachePath)
{
    shared_ptr<string> onBoardPath(new string((*cachePath) + ON_BOARD_WORKFOLDER));
    shared_ptr<string> offBoardPath(new string((*cachePath) + OFF_BOARD_WORKFOLDER));
    m_onBoardLayerProvider->SetCachePath(onBoardPath);
    m_offBoardLayerProvider->SetCachePath(offBoardPath);
    return NE_OK;
}

void HybridLayerProvider::SetPreferredLanguageCode(uint8 languageCode)
{
    m_onBoardLayerProvider->SetPreferredLanguageCode(languageCode);
    m_offBoardLayerProvider->SetPreferredLanguageCode(languageCode);
}

void HybridLayerProvider::SetTheme(MapViewTheme theme)
{
    m_onBoardLayerProvider->SetTheme(theme);
    m_offBoardLayerProvider->SetTheme(theme);
}

void HybridLayerProvider::SetMetadataRetryTimes(int number)
{
    m_onBoardLayerProvider->SetMetadataRetryTimes(number);
    m_offBoardLayerProvider->SetMetadataRetryTimes(number);
}

NB_Error HybridLayerProvider::SetPersistentMetadataPath(shared_ptr<string> metadataPath)
{
    shared_ptr<string> onBoardPath(new string((*metadataPath) + ON_BOARD_METADATA_SOURCE));
    shared_ptr<string> offBoardPath(new string((*metadataPath) + OFF_BOARD_METADATA_SOURCE));
    m_onBoardLayerProvider->SetPersistentMetadataPath(onBoardPath);
    m_offBoardLayerProvider->SetPersistentMetadataPath(offBoardPath);
    return NE_OK;
}

void HybridLayerProvider::CheckMetadataChanges(shared_ptr<AsyncCallback<bool> > callback, shared_ptr<MetadataConfiguration> config, bool skipTimeInterval, bool forceUpdate)
{
    m_HybridMode = m_HybridManager->GetMode();
    if(m_HybridMode == HBM_ONBOARD_ONLY)
    {
        m_onBoardLayerProvider->CheckMetadataChanges(callback, config, skipTimeInterval, forceUpdate);
    }
    else if(m_HybridMode == HBM_OFFBOARD_ONLY)
    {
        m_offBoardLayerProvider->CheckMetadataChanges(callback, config, skipTimeInterval, forceUpdate);
    }
}

NB_Context *HybridLayerProvider::GetContext()
{
    return m_pContext;
}

void HybridLayerProvider::HybridModeChanged(HybridMode mode)
{
    if(m_HybridMode != mode)
    {
        m_HybridMode = mode;
    }
}

void HybridLayerProvider::OnSuccessed(HybridMode mode)
{
    if(mode == HBM_ONBOARD_ONLY)
    {
        m_OnboardStatus = GLS_Succeeded;
        if(m_HybridMode == HBM_ONBOARD_ONLY)
        {
            NotifyGotLayer();
        }
    }
    else if(mode == HBM_OFFBOARD_ONLY)
    {
        m_OffboardStatus = GLS_Succeeded;
        if(m_HybridMode == HBM_OFFBOARD_ONLY)
        {
            NotifyGotLayer();
        }
    }

    if(m_OnboardStatus == GLS_Succeeded && m_OffboardStatus == GLS_Succeeded)
    {
        m_onBoardLayerProvider->CopyLayersFrom(*m_offBoardLayerProvider);
        if(m_HybridMode == HBM_ONBOARD_ONLY)
        {
            NotifyGotLayer();
        }
    }
}

void HybridLayerProvider::OnError(NB_Error error, HybridMode mode)
{
    if(mode == HBM_ONBOARD_ONLY)
    {
        m_OnboardStatus = GLS_Failed;
    }
    else if(mode == HBM_OFFBOARD_ONLY)
    {
        m_OffboardStatus = GLS_Failed;
    }
    if(mode == HBM_OFFBOARD_ONLY && m_HybridMode == HBM_ONBOARD_ONLY)
    {
        NotifyGotLayer();
    }
    if(mode == m_HybridMode && m_finalCallback)
    {
        m_finalCallback->Error(error);
    }
}

void HybridLayerProvider::NotifyGotLayer()
{
    if(m_finalCallback)
    {
        m_pAggregateLayerProvider->UpdateLayersFromSubProviders();
        NotifyLayersUpdated(GetCurrentProvider()->GetLayerPtrVector());
        m_finalCallback->Success(GetCurrentProvider()->GetLayerPtrVector());
    }
}

const shared_ptr<UnifiedLayerProvider> HybridLayerProvider::GetCurrentProvider() const
{
    shared_ptr<UnifiedLayerProvider> layerProvider;
    if(m_HybridMode == HBM_ONBOARD_ONLY)
    {
        layerProvider = m_onBoardLayerProvider;
    }
    else
    {
        layerProvider = m_offBoardLayerProvider;
    }
    return layerProvider;
}

HybridLayerProviderCallback::HybridLayerProviderCallback(HybridLayerProvider *hybridLayerProvider, HybridMode mode)
    :m_pHybridLayerProvider(hybridLayerProvider)
    ,m_mode(mode)
{
}

HybridLayerProviderCallback::~HybridLayerProviderCallback()
{
}

void HybridLayerProviderCallback::Success(const std::vector<LayerPtr> &/*layers*/)
{
    if(m_pHybridLayerProvider)
    {
        m_pHybridLayerProvider->OnSuccessed(m_mode);
    }
}

void HybridLayerProviderCallback::Error(NB_Error error)
{
    if(m_pHybridLayerProvider)
    {
        m_pHybridLayerProvider->OnError(error, m_mode);
    }
}

void HybridLayerProviderCallback::SetLayerProvider(HybridLayerProvider *hybridLayerProvider)
{
    m_pHybridLayerProvider = hybridLayerProvider;
}

const HybridMode& HybridLayerProviderCallback::GetOnOffBoardMode() const
{
    return m_mode;
}

}
