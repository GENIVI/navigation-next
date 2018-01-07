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

    @file       CustomLayerManagerImpl.cpp

    See header file for description.
*/
/*
    (C) Copyright 2014 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "CustomLayerManagerImpl.h"
#include "LayerProvider.h"
#include "LocalLayerProvider.h"
#include "CustomLayer.h"
#include "UnifiedLayerManager.h"
#include "LayerIdGenerator.h"
#include <vector>
#include "smartpointer.h"
#include "StringUtility.h"

/*! @{ */

using namespace std;
using namespace nbmap;
using namespace nbcommon;

// Public functions .............................................................................

static const string CUSTOM_LAYER_ID_PREFIX("CUSTOM_LAYER");

/* See declaration for description */
CustomLayerManagerImpl::CustomLayerManagerImpl(NB_Context* context,
                                               LayerProviderPtr layerProvider,
                                               LayerManagerPtr layerManager,
                                               shared_ptr<LayerIdGenerator> idGenerator)
        : m_currentLayerIdDigital(0),
          m_pContext(context),
          m_pLayerManager(layerManager),
          m_layerProvider(layerProvider),
          m_pLayerIdGenerator(idGenerator)
{
}

/* See declaration for description */
CustomLayerManagerImpl::CustomLayerManagerImpl(NB_Context* context)
        : m_currentLayerIdDigital(0),
          m_pContext(context)
{
}

/* See declaration for description */
CustomLayerManagerImpl::~CustomLayerManagerImpl()
{
}

void
CustomLayerManagerImpl::LoadTile(uint32 layerID, CustomNBMTilePtr tile)
{
    std::map<uint32, shared_ptr<CustomLayer> >::iterator it = m_layerMap.find(layerID);
    if(it != m_layerMap.end())
    {
        shared_ptr<CustomLayer>& customLayerPtr = it->second;
        if(customLayerPtr)
        {
            customLayerPtr->LoadTile(tile);
        }
    }
}

bool
CustomLayerManagerImpl::GetMarkerID(const std::string& internalMarkerID, int& userMarkerID)
{
    std::map<uint32, shared_ptr<CustomLayer> >::const_iterator iter = m_layerMap.begin();
    std::map<uint32, shared_ptr<CustomLayer> >::const_iterator end  = m_layerMap.end();
    for (; iter != end; ++iter)
    {
        if(iter->second)
        {
            shared_ptr<CustomLayer> customLayer = iter->second;
            bool ret = customLayer->GetMarkerID(internalMarkerID, userMarkerID);
            if(ret)
            {
                return true;
            }
            continue;
        }
    }
    return false;
}

/* See description in CustomLayerManager.h */
uint32
CustomLayerManagerImpl::AddCustomLayer(TileProviderPtr provider, int mainOrder, int subOrder,
        int refZoom, int minZoom, int maxZoom, bool visible)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if (!m_pLayerIdGenerator)
    {
        return 0;
    }

    uint32 layerIdDigital = m_pLayerIdGenerator->GenerateId();
    shared_ptr<string> layerID(new string(CUSTOM_LAYER_ID_PREFIX +
                                            StringUtility::NumberToString(layerIdDigital)));

    //Create a CustomLayer object for the Layer
    CustomLayer *customLayer = new CustomLayer(m_pContext, layerID,
                                               m_materialConfigPath,
                                               layerIdDigital,
                                               this,
                                               provider, mainOrder, subOrder, refZoom,
                                               minZoom, maxZoom, visible);
    //Check if memory allocation fails.
    if(!customLayer)
    {
        //return a null pointer if memory allocation fails
        return 0;
    }

    shared_ptr<CustomLayer> customLayerPtr(customLayer);
    m_layerMap.insert(std::make_pair(customLayerPtr->GetID(),customLayerPtr));

    //Add the Layer to the Layer Provider
    LocalLayerProvider* customLayerProvider = static_cast<LocalLayerProvider*>(m_layerProvider.get());
    if (customLayerProvider)
    {
        bool needToUpdate = customLayerProvider->AddLayer(customLayerPtr);
        if (needToUpdate)
        {
            UnifiedLayerManager* layerManager =
                    static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
            if (layerManager)
            {
                vector<LayerPtr> layers;
                layers.push_back(customLayerPtr);
                layerManager->NotifyLayersAdded(layers);
            }
        }
    }

    return layerIdDigital;
}

void
CustomLayerManagerImpl::RemoveCustomLayer(int layerID)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // Find custom layer to remove.
    map<uint32, shared_ptr<CustomLayer> >::iterator layerIterator = m_layerMap.find(layerID);
    if (layerIterator == m_layerMap.end())
    {
        return;
    }
    shared_ptr<CustomLayer> removedLayer = layerIterator->second;

    // Remove this custom layer from map of custom layer manager.
    m_layerMap.erase(layerIterator);

    // Remove this layer from layer provider.
    LocalLayerProvider* layerProvider = dynamic_cast<LocalLayerProvider*>(m_layerProvider.get());
    if (layerProvider)
    {
        bool needToUpdate = layerProvider->RemoveLayer(removedLayer);
        if (needToUpdate)
        {
            UnifiedLayerManager* layerManager =
                    static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
            if (layerManager)
            {
                vector<LayerPtr> layers;
                layers.push_back(removedLayer);
                layerManager->NotifyLayersRemoved(layers);
            }
        }
    }
}

/* See description in PinManager.h */
void
CustomLayerManagerImpl::SetMaterialConfigPath(shared_ptr<string> materialConfigPath)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    m_materialConfigPath = materialConfigPath;

    map<uint32, shared_ptr<CustomLayer> >::iterator layerIterator = m_layerMap.begin();
    map<uint32, shared_ptr<CustomLayer> >::iterator layerEnd = m_layerMap.end();
    for (; layerIterator != layerEnd; ++layerIterator)
    {
        shared_ptr<CustomLayer>& layer = layerIterator->second;
        if (layer)
        {
            layer->SetMaterialConfigPath(materialConfigPath);
        }
    }
}

/* See description in CustomLayerManager.h */
shared_ptr<vector<NB_LatitudeLongitude> >
CustomLayerManagerImpl::GetAllPinCoordinates()
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // Check if there are pin layers in the pin manager.
    if (m_layerMap.empty())
    {
        return shared_ptr<vector<NB_LatitudeLongitude> >();
    }

    // Create a vector to return the coordinates.
    shared_ptr<vector<NB_LatitudeLongitude> > allPinCoordinates(new vector<NB_LatitudeLongitude>());
    if (!allPinCoordinates)
    {
        return shared_ptr<vector<NB_LatitudeLongitude> >();
    }

    // Get all pin coordinates from each pin layer.
    map<uint32, shared_ptr<CustomLayer> >::const_iterator layerIterator = m_layerMap.begin();
    map<uint32, shared_ptr<CustomLayer> >::const_iterator layerEnd = m_layerMap.end();
    for (; layerIterator != layerEnd; ++layerIterator)
    {
        shared_ptr<CustomLayer> customLayer = layerIterator->second;
        if (customLayer && customLayer->IsEnabled())
        {
            shared_ptr<vector<NB_LatitudeLongitude> > layerCoordinates = customLayer->GetAllPinCoordinates();
            if (layerCoordinates && (!(layerCoordinates->empty())))
            {
                // Add the coordinates of the layer to the result.
                allPinCoordinates->insert(allPinCoordinates->end(),
                                          layerCoordinates->begin(),
                                          layerCoordinates->end());
            }
        }
    }

    return allPinCoordinates;
}

/* See header file for description */
void
CustomLayerManagerImpl::RefreshAllPinsOfLayer(uint32 layerID)
{
    NB_ASSERT_CCC_THREAD(m_pContext);
    
    // Check if layer ID is empty.
    if (!layerID)
    {
        return;
    }

    // Find the layer by layer ID.
    map<uint32, shared_ptr<CustomLayer> >::iterator layerIterator = m_layerMap.find(layerID);
    if (layerIterator == m_layerMap.end())
    {
        return;
    }
    shared_ptr<CustomLayer> refreshedLayer = layerIterator->second;
    if (!refreshedLayer)
    {
        return;
    }

    // Notify layer manager to update all tiles of the layer.
    if (m_pLayerManager)
    {
        UnifiedLayerManager* layerManager =
                static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
        if (layerManager)
        {
            layerManager->RefreshTilesOfLayer(refreshedLayer);
        }
    }
}

/* See header file for description */
void
CustomLayerManagerImpl::RefreshTilesOfLayer(const std::vector<TileKeyPtr>& tileKeys,
                                            uint32 layerID)
{
    NB_ASSERT_CCC_THREAD(m_pContext);
    
    // Check if layer ID is empty.
    if ((tileKeys.empty()) || (!layerID))
    {
        return;
    }

    // Find the layer by layer ID.
    map<uint32, shared_ptr<CustomLayer> >::iterator layerIterator = m_layerMap.find(layerID);
    if (layerIterator == m_layerMap.end())
    {
        return;
    }
    shared_ptr<CustomLayer> refreshedLayer = layerIterator->second;
    if (!refreshedLayer)
    {
        return;
    }

    // Notify layer manager to update a tile of the layer.
    if (m_pLayerManager)
    {
        UnifiedLayerManager* layerManager =
                static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
        if (layerManager)
        {
            layerManager->RefreshTiles(tileKeys, refreshedLayer);
        }
    }
}

/* See header file for description */
void
CustomLayerManagerImpl::NotifyPinsRemoved(shared_ptr<vector<PinPtr> > /*pins*/)
{
    NB_ASSERT_CCC_THREAD(m_pContext);
    
    // Notify the layer manager that the pins are removed.
    if (m_pLayerManager)
    {
        UnifiedLayerManager* layerManager = static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
        if (layerManager)
        {
            //TODO:NO need notify the pins remove, this notification is tell map view remover the accordingly bubble
            //layerManager->NotifyPinsRemoved(pins);
        }
    }
}

void
CustomLayerManagerImpl::SetVisible(uint32 layerID, bool isVisible)
{
    CustomLayerPtr customLayerPtr = GetCustomLayer(layerID);
    if(!customLayerPtr)
    {
        return;
    }

    if(isVisible ^ customLayerPtr->IsEnabled())
    {
        customLayerPtr->SetEnabled(isVisible);
    }
}

CustomLayerPtr
CustomLayerManagerImpl::GetCustomLayer(uint32 layerID)
{
    NB_ASSERT_CCC_THREAD(m_pContext);
    
    CustomLayerPtr layer;
    std::map<uint32, shared_ptr<CustomLayer> > :: iterator iter =
            m_layerMap.find(layerID);
    if (iter != m_layerMap.end())
    {
        layer = iter->second;
    }
    return layer;
}

/* See description in header file. */
void CustomLayerManagerImpl::Initialize(LayerProviderPtr layerProvider,
                                LayerManagerPtr layerManager,
                                shared_ptr < LayerIdGenerator > idGenerator)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    m_layerProvider         = layerProvider;
    m_pLayerManager         = layerManager;
    m_pLayerIdGenerator     = idGenerator;
}

/* See description in header file. */
void CustomLayerManagerImpl::RemoveAllPins()
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    map<uint32, shared_ptr<CustomLayer> >::iterator layerIterator = m_layerMap.begin();
    map<uint32, shared_ptr<CustomLayer> >::iterator layerEnd = m_layerMap.end();
    for (; layerIterator != layerEnd; ++layerIterator)
    {
        shared_ptr<CustomLayer> customLayer = layerIterator->second;
        if (customLayer)
        {
            customLayer->RemoveAllPins();
        }
    }
}


/*! @} */
