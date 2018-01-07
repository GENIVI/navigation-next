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
  @file        LocalLayerProvider.cpp
  @defgroup    map

  Description: Implementation of LocalLayerProvider.
*/
/*
  (C) Copyright 2012 by TeleCommunications Systems, Inc.

  The information contained herein is confidential, proprietary to
  TeleCommunication Systems, Inc., and considered a trade secret as defined
  in section 499C of the penal code of the State of California. Use of this
  information by anyone other than authorized employees of TeleCommunication
  Systems is granted only under a written non-disclosure agreement, expressly
  prescribing the scope and manner of such use.

  --------------------------------------------------------------------------*/
/*! @{ */

#include "LocalLayerProvider.h"
#include "StringUtility.h"
#include "PinLayerImpl.h"
#include "LayerFunctors.h"


using namespace nbmap;
using namespace std;


LocalLayerProvider::LocalLayerProvider(NB_Context* context,
                                       AggregateLayerProvider* parentLayerProvider)
{
    m_pContext = context;
    m_parentLayerProvider = parentLayerProvider;
}

LocalLayerProvider::~LocalLayerProvider()
{
}

void
LocalLayerProvider::GetLayers(shared_ptr <AsyncCallback<const vector<LayerPtr>& > > callback,
                              shared_ptr<MetadataConfiguration> /*metadataConfig*/)
{
    callback->Success(m_pLayers);
}

/* See description in LayerProvider.h */
void
LocalLayerProvider::ClearLayers()
{
    // @todo: Should we reset pin layers too?
}

const vector <LayerPtr>& LocalLayerProvider::GetLayerPtrVector() const
{
    return m_pLayers;
}

// Following functions exists to make his parent happy, will do nothing.
const vector <LayerPtr>& LocalLayerProvider::GetCommonMaterialLayers() const
{
    return m_dummyCommonMaterialLayers;
}

/* See description in header file. */
const vector <LayerPtr>&  LocalLayerProvider::GetOptionalLayerPtrVector() const
{
    return m_pOptionalLayers;
}

const LayerPtr LocalLayerProvider::GetLayerAvailabilityMatrix() const
{
    return m_dummyLayer;
}

const LayerPtr LocalLayerProvider::GetDAMLayer() const
{
    return m_dummyLayer;
}

NB_Error LocalLayerProvider::SetCachePath(shared_ptr < string > /*cachePath*/)
{
    return NE_OK;
}

void LocalLayerProvider::SetMetadataRetryTimes(int /*number*/)
{
}

NB_Error LocalLayerProvider::SetPersistentMetadataPath(shared_ptr<string> /*metadataPath*/)
{
    return NE_OK;
}

/* See description in LayerProvider.h */
NB_Error LocalLayerProvider::GetBackgroundRasterRange(uint32& /*minZoomLevel*/,
                                             uint32& /*maxZoomLevel*/)
{
    // There is no this zoom level specified.
    return NE_NOENT;
}
void LocalLayerProvider::SetPreferredLanguageCode(uint8 /*languageCode*/)
{
}
void LocalLayerProvider::SetTheme(MapViewTheme /*theme*/)
{
}

NB_Context* LocalLayerProvider::GetContext()
{
    return m_pContext;
}

/* See header file for description */
bool
LocalLayerProvider::AddLayer(LayerPtr layer)
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
    if(m_parentLayerProvider)
    {
        m_parentLayerProvider->AddLayer(layer);
    }
    return true;
}

/* See header file for description */
bool
LocalLayerProvider::RemoveLayer(LayerPtr layer)
{
    if (!layer)
    {
        return false;
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

    if (!m_parentLayerProvider)
    {
        // Need to update layers.
        return true;
    }

    return m_parentLayerProvider->RemoveLayer(layer);
}

vector<LayerPtr> LocalLayerProvider::GetLayerPtrVectorFromParent()
{
    return m_parentLayerProvider->GetLayerPtrVector();
}

/* See description in header file. */
vector < LayerPtr > LocalLayerProvider::GetOptionalLayerPtrVectorFromParent()
{
    return m_parentLayerProvider->GetOptionalLayerPtrVector();
}


/*! @} */
