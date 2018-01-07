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

/*
   (C) Copyright 2016 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/
#ifndef HYBRIDLAYERPROVIDER_H
#define HYBRIDLAYERPROVIDER_H
#include "HybridStrategy.h"
#include "LayerProvider.h"
#include "HybridManager.h"

using namespace nbcommon;
namespace nbmap {

class HybridLayerProviderCallback;
class UnifiedLayerProvider;
class AggregateLayerProvider;

class HybridLayerProvider : public LayerProvider, public HybridModeListener
{
public:
    HybridLayerProvider(NB_Context* context,
                        uint32 nbgmVersion = 24,
                        uint32 maxTileRequestCountPerLayer = 0,
                        uint32 maxCachedTileCount = 0);
    ~HybridLayerProvider();

public:
    NB_Context *GetContext();
    void OnSuccessed(HybridMode mode);
    void OnError(NB_Error error, HybridMode mode);

    // HybridModeListener interface
public:
    virtual void HybridModeChanged(HybridMode mode);

    // LayerProvider interface
private:
    virtual void GetLayers(shared_ptr<AsyncCallback<const std::vector<LayerPtr>& > > callback,
                              shared_ptr<MetadataConfiguration> metadataConfig);
    virtual void ClearLayers();
    virtual const LayerPtr GetLayerAvailabilityMatrix() const;
    virtual const std::vector<LayerPtr> &GetLayerPtrVector() const;
    virtual const std::vector<LayerPtr> &GetOptionalLayerPtrVector() const;
    virtual const std::vector<LayerPtr> &GetCommonMaterialLayers() const;
    virtual const LayerPtr GetDAMLayer() const;
    virtual NB_Error GetBackgroundRasterRange(uint32 &minZoomLevel, uint32 &maxZoomLevel);
    virtual NB_Error SetCachePath(shared_ptr<std::string> cachePath);
    virtual void SetPreferredLanguageCode(uint8 languageCode);
    virtual void SetTheme(MapViewTheme theme);
    virtual void SetMetadataRetryTimes(int number);
    virtual NB_Error SetPersistentMetadataPath(shared_ptr<string> metadataPath);
    virtual void CheckMetadataChanges(shared_ptr<AsyncCallback<bool> > callback, shared_ptr<MetadataConfiguration> config, bool skipTimeInterval, bool forceUpdate);

private:
    const shared_ptr<UnifiedLayerProvider> GetCurrentProvider() const;
    void NotifyGotLayer();

private:
    enum GetLayerStatus
    {
        GLS_NotStart = 0,
        GLS_Failed,
        GLS_Succeeded,
    };

private:
    NB_Context* m_pContext;
    shared_ptr<UnifiedLayerProvider> m_onBoardLayerProvider;
    shared_ptr<UnifiedLayerProvider> m_offBoardLayerProvider;
    shared_ptr<HybridLayerProviderCallback > m_onBoardCallback;
    shared_ptr<HybridLayerProviderCallback > m_offBoardCallback;
    shared_ptr<AsyncCallback<const std::vector<LayerPtr> &> > m_finalCallback;
    shared_ptr<AggregateLayerProvider> m_pAggregateLayerProvider;
    shared_ptr<HybridManager> m_HybridManager;
    HybridMode m_HybridMode;
    GetLayerStatus m_OnboardStatus;
    GetLayerStatus m_OffboardStatus;
};

class HybridLayerProviderCallback : public AsyncCallback<const std::vector<LayerPtr> &>
{
public:
    HybridLayerProviderCallback(HybridLayerProvider* hybridLayerProvider, HybridMode mode);
    ~HybridLayerProviderCallback();
    virtual void Success(const std::vector<LayerPtr>& layers);
    virtual void Error(NB_Error error);
    void SetLayerProvider(HybridLayerProvider* hybridLayerProvider);
    const HybridMode& GetOnOffBoardMode() const;

private:
    HybridLayerProvider*  m_pHybridLayerProvider; /*!< Instance of HybridLayerProvider*/
    HybridMode m_mode;
};

}
#endif // HYBRIDLAYERPROVIDER_H
