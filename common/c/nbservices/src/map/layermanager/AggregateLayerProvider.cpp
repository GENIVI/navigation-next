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
  @file     AggregateLayerProvider.cpp
  @defgroup nbmap

*/
/*
  (C) Copyright 2012 by TeleCommunication Systems, Inc.

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
}

#include "MetadataConfiguration.h"
#include "AggregateLayerProvider.h"
#include "LayerFunctors.h"
#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>

//#define DLAYER
#ifdef DLAYER
#include <QDebug>
#endif

using namespace nbmap;
using namespace protocol;

// Implementation of AggregateLayerProvider.
AggregateLayerProvider::AggregateLayerProvider(NB_Context* context, uint32 /*nbgmVersion*/)
{
    m_pContext       = context;
    m_isInitialized  = false;
}

AggregateLayerProvider::~AggregateLayerProvider()
{
    if (m_pLayerCallback)
    {
        m_pLayerCallback->SetAggregateLayerProvider(NULL);
    }
}

void AggregateLayerProvider::Initialize()
{
    // Initialize layer callback.
    m_pLayerCallback = shared_ptr<LayerCallback>(new LayerCallback);
    if (!m_pLayerCallback)
    {
        return;
    }

    m_pLayerCallback->Initialize(this);
    m_isInitialized = true;
}

bool AggregateLayerProvider::IsInitialized()
{
    return m_isInitialized;
}

NB_Context* AggregateLayerProvider::GetContext()
{
    return m_pContext;
}

void AggregateLayerProvider::GetLayers(shared_ptr<AsyncCallback<const std::vector<LayerPtr>& > > callback,
                                       shared_ptr<MetadataConfiguration> metadataConfig)
{
    /* Reset the initialized flag.

       TRICKY: It is mainly used to reset the flag hasErrorOccured of
               LayerCallback. Because if GetLayers failed once, the
               callback of success could not be called again. See the
               function LayerCallback::Success for details.
    */
    if (m_isInitialized)
    {
        m_isInitialized = false;
        Initialize();
    }

    // Check if this object is initialized.
    if (!m_isInitialized)
    {
        if (callback)
        {
            callback->Error(NE_NOTINIT);
        }
        return;
    }

    // Clear previously stored contents.

    m_pLayers.clear();
    m_pCommonMaterialLayers.clear();
    m_pOptionalLayers.clear();
    m_pLayerAvailabilityMatrix.reset();
    m_pDAMLayer.reset();

    m_pCallbackFinal = callback;

    if (m_pLayerCallback && (m_pLayerCallback->CanStartRequest()))
    {
        m_pLayerCallback->SetCallbackCount((unsigned int)m_LayerProviders.size());

        std::vector<LayerProviderPtr>::iterator iter;
        for (iter = m_LayerProviders.begin(); iter != m_LayerProviders.end(); iter++)
        {
            (*iter)->GetLayers(m_pLayerCallback, metadataConfig);
        }

    }
}

/* See description in LayerProvider.h */
void
AggregateLayerProvider::ClearLayers()
{
    // Call function ClearLayers on each layer provider.
    vector<LayerProviderPtr>::iterator providerIterator = m_LayerProviders.begin();
    vector<LayerProviderPtr>::const_iterator providerEnd = m_LayerProviders.end();
    for (; providerIterator != providerEnd; ++providerIterator)
    {
        LayerProviderPtr layerProvider = *providerIterator;

        if (layerProvider)
        {
            layerProvider->ClearLayers();
        }
    }

    // Clear saved layers.
    m_pLayers.clear();
    m_pCommonMaterialLayers.clear();
    m_pLayerAvailabilityMatrix.reset();
    m_pDAMLayer.reset();

    // Reset callback.
    m_pLayerCallback.reset();
    m_pCallbackFinal.reset();

    // Reset initialized flag.
    if (m_isInitialized)
    {
        m_isInitialized = false;
        Initialize();
    }
}

vector<LayerProviderPtr> AggregateLayerProvider::GetLayerProviderVector()
{
    return m_LayerProviders;
}

const LayerPtr AggregateLayerProvider::GetLayerAvailabilityMatrix() const
{
    return m_pLayerAvailabilityMatrix;
}

const LayerPtr AggregateLayerProvider::GetDAMLayer() const
{
    return m_pDAMLayer;
}

const std::vector<LayerPtr>& AggregateLayerProvider::GetLayerPtrVector() const
{
    return m_pLayers;
}

const std::vector<LayerPtr>& AggregateLayerProvider::GetCommonMaterialLayers() const
{
    return m_pCommonMaterialLayers;
}

/* See description in header file. */
const std::vector <LayerPtr>&  AggregateLayerProvider::GetOptionalLayerPtrVector() const
{
    return m_pOptionalLayers;
}

void AggregateLayerProvider::CallbackSuccess()
{
    if (m_pCallbackFinal)
    {
        m_pCallbackFinal->Success(m_pLayers);
    }
}

void AggregateLayerProvider::CallbackError(NB_Error error)
{
    if (m_pCallbackFinal)
    {
        m_pCallbackFinal->Error(error);
    }
}

/*! Please refer LayerManager.h */
NB_Error AggregateLayerProvider::SetCachePath(shared_ptr<std::string> cachePath)
{
    std::vector<LayerProviderPtr>::iterator iter;
    for (iter = m_LayerProviders.begin(); iter != m_LayerProviders.end(); iter++)
    {
        (*iter)->SetCachePath(cachePath);
    }
    return NE_OK;
}

/* See description in LayerProvider.h */
NB_Error
AggregateLayerProvider::GetBackgroundRasterRange(uint32& minZoomLevel,
                                                 uint32& maxZoomLevel)
{
    // If there is no zoom levels specified, return NE_NOENT.
    NB_Error error = NE_NOENT;
    uint32 minZoomLevelResult = numeric_limits<uint32>::max();
    uint32 maxZoomLevelResult = numeric_limits<uint32>::min();
    vector<LayerProviderPtr>::iterator iterator = m_LayerProviders.begin();
    vector<LayerProviderPtr>::const_iterator end = m_LayerProviders.end();
    for (; iterator != end; ++iterator)
    {
        LayerProviderPtr& layerProvider = *iterator;
        if (!layerProvider)
        {
            continue;
        }

        // Get minimum and maximum zoom levels from layer provider.
        uint32 minZoomLevelGot = 0;
        uint32 maxZoomLevelGot = 0;
        NB_Error tempError = layerProvider->GetBackgroundRasterRange(minZoomLevelGot,
                                                           maxZoomLevelGot);
        if (tempError != NE_OK)
        {
            continue;
        }

        // Check minimum zoom level.
        if (minZoomLevelGot < minZoomLevelResult)
        {
            minZoomLevelResult = minZoomLevelGot;
        }

        // Check maximum zoom level.
        if (maxZoomLevelGot > maxZoomLevelResult)
        {
            maxZoomLevelResult = maxZoomLevelGot;
        }

        /* Set returned error to NE_OK. Because there are minimum and maximum
           zoom levels specified. */
        error = NE_OK;
    }

    if (error != NE_OK)
    {
        // Return the error.
        return error;
    }

    minZoomLevel = minZoomLevelResult;
    maxZoomLevel = maxZoomLevelResult;
    return NE_OK;
}

void AggregateLayerProvider::SetPreferredLanguageCode(uint8 languageCode)
{
    vector<LayerProviderPtr>::iterator iter = m_LayerProviders.begin();
    vector<LayerProviderPtr>::iterator end  = m_LayerProviders.end();
    for (; iter != end; ++iter)
    {
        if (*iter)
        {
            (*iter)->SetPreferredLanguageCode(languageCode);
        }
    }
}

void AggregateLayerProvider::SetTheme(MapViewTheme theme)
{
    vector<LayerProviderPtr>::iterator iter = m_LayerProviders.begin();
    vector<LayerProviderPtr>::iterator end  = m_LayerProviders.end();
    for (; iter != end; ++iter)
    {
        if (*iter)
        {
            (*iter)->SetTheme(theme);
        }
    }
}

void AggregateLayerProvider::SetMetadataRetryTimes(int number)
{
    std::vector<LayerProviderPtr>::iterator iter;
    for (iter = m_LayerProviders.begin(); iter != m_LayerProviders.end(); iter++)
    {
        (*iter)->SetMetadataRetryTimes(number);
    }
}

NB_Error AggregateLayerProvider::SetPersistentMetadataPath(shared_ptr<string> metadataPath)
{
    std::vector<LayerProviderPtr>::iterator iter;
    for (iter = m_LayerProviders.begin(); iter != m_LayerProviders.end(); iter++)
    {
        (*iter)->SetPersistentMetadataPath(metadataPath);
    }
    return NE_OK;
}

void AggregateLayerProvider::CheckMetadataChanges(shared_ptr<AsyncCallback<bool> > callback, shared_ptr<MetadataConfiguration> config, bool skipTimeInterval, bool forceUpdate)
{
    std::vector<LayerProviderPtr>::iterator iter;
    for (iter = m_LayerProviders.begin(); iter != m_LayerProviders.end(); iter++)
    {
        (*iter)->CheckMetadataChanges(callback, config, skipTimeInterval, forceUpdate);
    }
}

NB_Error AggregateLayerProvider::AddLayerProvider(LayerProviderPtr layerProvider)
{
    if (!layerProvider)
    {
        return NE_INVAL;
    }

    m_LayerProviders.push_back(layerProvider);

    // It is possible that there already some layers existed in the added layer provider,
    // add those layers to correspond vectors.
    UpdateLayersFromSingleProvider(layerProvider);

    return NE_OK;
}

/* See description in header file. */
void AggregateLayerProvider::UpdateLayersFromSubProviders()
{
    // Clear all cached layers.
    m_pLayers.clear();
    m_pCommonMaterialLayers.clear();
    m_pOptionalLayers.clear();
    m_pLayerAvailabilityMatrix.reset();
    m_pDAMLayer.reset();

    vector<LayerProviderPtr>::const_iterator iter = m_LayerProviders.begin();
    vector<LayerProviderPtr>::const_iterator end  = m_LayerProviders.end();
    for (; iter != end; ++iter)
    {
        UpdateLayersFromSingleProvider(*iter);
    }
}

/* See header file for description */
bool
AggregateLayerProvider::AddLayer(LayerPtr layer)
{
    if (!layer)
    {
        return false;
    }

    // Check if this layer already exists.
    LayerFindByPointerFunctor functor(layer);
    vector<LayerPtr>::iterator layerEnd = m_pLayers.end();
    vector<LayerPtr>::iterator layerIterator = find_if(m_pLayers.begin(), layerEnd, functor);
    if (layerIterator != layerEnd)
    {
        return false;
    }

    m_pLayers.push_back(layer);

#ifdef DLAYER
    qWarning() << "AggregateLayerProvider.AddLayer(" << layer->str().c_str() << "," << m_pLayers.size() << ")";
#endif

    /* Check if function GetLayers is called in progress.

       @todo: I ignore the logic to check if 'm_pCallbackFinal' is empty. */
    if (m_pLayerCallback && (!(m_pLayerCallback->CanStartRequest())))
    {
        // Not need to update layers. Because layers will be updated when GetLayers finishes.
        return false;
    }

    return true;
}

/* See header file for description */
bool
AggregateLayerProvider::RemoveLayer(LayerPtr layer)
{
    if (!layer)
    {
        return false;
    }

    /* Remove this layer in vector of callback. Because this layer may be already returned when
       function GetLayers is called in progress. */
    if (m_pLayerCallback)
    {
        m_pLayerCallback->RemoveLayer(layer);
    }

    // Check if this layer exists.
    LayerFindByPointerFunctor functor(layer);
    vector<LayerPtr>::iterator layerEnd = m_pLayers.end();
    vector<LayerPtr>::iterator layerIterator = find_if(m_pLayers.begin(), layerEnd, functor);
    if (layerIterator == layerEnd)
    {
        return false;
    }

    m_pLayers.erase(layerIterator);

#ifdef DLAYER
    qWarning() << "AggregateLayerProvider.RemoveLayer(" << layer->str().c_str() << "," << m_pLayers.size() << ")";
#endif

    /* Check if function GetLayers is called in progress.

       @todo: I ignore the logic to check if 'm_pCallbackFinal' is empty. */
    if (m_pLayerCallback && (!(m_pLayerCallback->CanStartRequest())))
    {
        // Not need to update layers. Because layers will be updated when GetLayers finishes.
        return false;
    }

    return true;
}


/* See description in header file. */
void AggregateLayerProvider::UpdateLayersFromSingleProvider(LayerProviderPtr layerProvider)
{
    if (!layerProvider)
    {
        return;
    }

    // Call the function AddLayer to add unique layers.
    const vector<LayerPtr>& rLayers = layerProvider->GetLayerPtrVector();
    if (!rLayers.empty())
    {
        for_each(rLayers.begin(), rLayers.end(),
                 bind1st(mem_fun(&AggregateLayerProvider::AddLayer), this));
    }

    sort (m_pLayers.begin(), m_pLayers.end());

    // Update common material layers.
    const vector<LayerPtr>& rCommonMatterialLayers = layerProvider->GetCommonMaterialLayers();
    copy(rCommonMatterialLayers.begin(), rCommonMatterialLayers.end(),
         back_inserter(m_pCommonMaterialLayers));

    // Optional layers
    const std::vector<LayerPtr>& rOptionalLayers = layerProvider->GetOptionalLayerPtrVector();
    copy(rOptionalLayers.begin(), rOptionalLayers.end(),
         back_inserter(m_pOptionalLayers));

    //@todo: Is it possible to have multiple LAM Layer ?
    LayerPtr rLamLayer = layerProvider->GetLayerAvailabilityMatrix();
    if (rLamLayer && !m_pLayerAvailabilityMatrix)
    {
        m_pLayerAvailabilityMatrix = rLamLayer;
    }
    else
    {
        // Place holder, should not happen if there's only one LAM layer by design.
    }

    // Data Availability Matrix layer.
    LayerPtr damLayer = layerProvider->GetDAMLayer();
    if (damLayer && !m_pDAMLayer)
    {
        m_pDAMLayer = damLayer;
    }
}

// Implementation of LayerCallback.

LayerCallback::LayerCallback()
 : m_pAggregateLayerProvider(NULL),
   m_callbackCount(0),
   m_remainedCallbackCount(0),
   hasErrorOccured(false)
{
    // Nothing to do here.
}

LayerCallback::~LayerCallback()
{
    // Nothing to do here.
}

void LayerCallback::Success(const vector<LayerPtr>& /*response*/)
{
    if (hasErrorOccured)
    {
        return;
    }

    // @tricky: hybrid layer provider may notice aggregate layer provide twice:
    // one for on-board layers, and the other for off-board layers
    // therefore m_remainedCallbackCount may become to a very big value due to overflow
    if(m_remainedCallbackCount>0)
    {
        --m_remainedCallbackCount;
    }
    //if remained callback Count is 0 we are not expecting any more callbacks
    if (m_remainedCallbackCount == 0 && m_pAggregateLayerProvider)
    {
        m_pAggregateLayerProvider->CallbackSuccess();
    }
}

void LayerCallback::Error(NB_Error error)
{
    hasErrorOccured         = true;
    m_callbackCount         = 0;
    m_remainedCallbackCount = 0;
    if (m_pAggregateLayerProvider)
    {
        m_pAggregateLayerProvider->CallbackError(error);
    }
}

void LayerCallback::Initialize(AggregateLayerProvider* aggregateLayerProvider)
{
    m_pAggregateLayerProvider = aggregateLayerProvider;
    m_callbackCount = 0;
    m_remainedCallbackCount = 0;
    hasErrorOccured = false;
}

void LayerCallback::SetAggregateLayerProvider(AggregateLayerProvider* aggregateLayerProvider)
{
    m_pAggregateLayerProvider = aggregateLayerProvider;
}

/* See header file for description */
bool
LayerCallback::CanStartRequest()
{
    return m_remainedCallbackCount == 0 ? true : false;
}

/* See header file for description */
void
LayerCallback::SetCallbackCount(unsigned int number)
{
    m_callbackCount = number;
    m_remainedCallbackCount = number;
}

/* See header file for description */
void
LayerCallback::RemoveLayer(LayerPtr layer)
{
    // Check if this layer exists.
    LayerFindByPointerFunctor functor(layer);
    vector<LayerPtr>::iterator layerEnd = m_pLayers.end();
    vector<LayerPtr>::iterator layerIterator = find_if(m_pLayers.begin(), layerEnd, functor);
    if (layerIterator != layerEnd)
    {
        m_pLayers.erase(layerIterator);
    }
}

/*! @} */

