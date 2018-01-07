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

    @file       PinManagerImpl.cpp

    See header file for description.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "PinManagerImpl.h"
#include "LayerProvider.h"
#include "LocalLayerProvider.h"
#include "PinLayerImpl.h"
#include "UnifiedLayerManager.h"
#include "LayerIdGenerator.h"
#include <vector>

/*! @{ */

using namespace std;
using namespace nbmap;

// Public functions .............................................................................

static const string MATERIAL_CATEGORY_PIN("pin");

//@todo:
//     1. Initialize material Category, share it with PinLayers.
//     2. Tell MapView to load and activate Materials when first PinLayer is created.
//     3. Remove PinMaterial initialization in NBGMViewController.cpp.
//     4. Let all PinLayers share same instance of PinMaterialParser.

/* See declaration for description */
PinManagerImpl::PinManagerImpl(NB_Context* context,
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
PinManagerImpl::PinManagerImpl(NB_Context* context)
        : m_currentLayerIdDigital(0),
          m_pContext(context)
{
}

/* See declaration for description */
PinManagerImpl::~PinManagerImpl()
{
}

/* See description in PinManager.h */
shared_ptr<PinLayer>
PinManagerImpl::AddPinLayer(shared_ptr<string> layerID)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // If layer ID is not valid return an empty pointer.
    if ((!layerID) || (layerID->empty()) || !m_pLayerIdGenerator)
    {
        return shared_ptr<PinLayer>();
    }

    // Check if this layer ID exists.
    map<string, shared_ptr<PinLayerImpl> >::const_iterator layerIterator = m_layerMap.find(*layerID);
    if (layerIterator != m_layerMap.end())
    {
        return layerIterator->second;
    }

    //If you are here that means the layer doesnt exist.
    //Create a layer add it to map and return it.

    uint32 layerIdDigital = m_pLayerIdGenerator->GenerateId();

    //Create a new Shared pointer for the Layer
    shared_ptr<PinLayerImpl> pinLayerPtr(new PinLayerImpl(layerID,
                                                          m_materialConfigPath,
                                                          layerIdDigital,
                                                          this,
                                                          m_pContext));
    //Check if memory allocation fails.
    if(!pinLayerPtr)
    {
        //return a null pointer if memory allocation fails
        return shared_ptr<PinLayer>();
    }

    //Insert in map and retrn the LayerPtr
    m_layerMap.insert(std::make_pair(*layerID,pinLayerPtr));

    //Add the Layer to the Layer Provider
    LocalLayerProvider* pinLayerProvider = static_cast<LocalLayerProvider*>(m_layerProvider.get());
    if (pinLayerProvider)
    {
        bool needToUpdate = pinLayerProvider->AddLayer(pinLayerPtr);
        if (needToUpdate)
        {
            UnifiedLayerManager* layerManager =
                    static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
            if (layerManager)
            {
                vector<LayerPtr> layers;
                layers.push_back(pinLayerPtr);
                layerManager->NotifyLayersAdded(layers);
            }
        }
    }
    return pinLayerPtr;
}

/* See description in PinManager.h */
void
PinManagerImpl::RemovePinLayer(shared_ptr<PinLayer> pinLayer)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // Shared pointer to pin layer should not be NULL.
    if (!pinLayer)
    {
        return;
    }

    pinLayer->RemoveAllPins();

    // Layer ID should not be NULL.
    shared_ptr<string> layerID = pinLayer->GetLayerID();
    if (!layerID)
    {
        return;
    }

    // Find pin layer to remove.
    map<string, shared_ptr<PinLayerImpl> >::iterator layerIterator = m_layerMap.find(*layerID);
    if (layerIterator == m_layerMap.end())
    {
        return;
    }
    shared_ptr<PinLayerImpl> removedLayer = layerIterator->second;

    // Remove this pin layer from map of pin layers.
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
PinManagerImpl::SetMaterialConfigPath(shared_ptr<string> materialConfigPath)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    m_materialConfigPath = materialConfigPath;

    map<string, shared_ptr<PinLayerImpl> >::iterator layerIterator = m_layerMap.begin();
    map<string, shared_ptr<PinLayerImpl> >::iterator layerEnd = m_layerMap.end();
    for (; layerIterator != layerEnd; ++layerIterator)
    {
        shared_ptr<PinLayerImpl>& layer = layerIterator->second;
        if (layer)
        {
            layer->SetMaterialConfigPath(materialConfigPath);
        }
    }
}

shared_ptr<string>
PinManagerImpl::GetMaterialConfigPath()
{
    return m_materialConfigPath;
}

/* See description in PinManager.h */
shared_ptr<vector<NB_LatitudeLongitude> >
PinManagerImpl::GetAllPinCoordinates()
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
    map<string, shared_ptr<PinLayerImpl> >::const_iterator layerIterator = m_layerMap.begin();
    map<string, shared_ptr<PinLayerImpl> >::const_iterator layerEnd = m_layerMap.end();
    for (; layerIterator != layerEnd; ++layerIterator)
    {
        shared_ptr<PinLayerImpl> pinLayer = layerIterator->second;
        if (pinLayer && pinLayer->IsEnabled())
        {
            shared_ptr<vector<NB_LatitudeLongitude> > layerCoordinates = pinLayer->GetAllPinCoordinates();
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
PinManagerImpl::RefreshAllPinsOfLayer(shared_ptr<string> layerID)
{
    NB_ASSERT_CCC_THREAD(m_pContext);
    
    // Check if layer ID is empty.
    if (!layerID)
    {
        return;
    }

    // Find the layer by layer ID.
    map<string, shared_ptr<PinLayerImpl> >::iterator layerIterator = m_layerMap.find(*layerID);
    if (layerIterator == m_layerMap.end())
    {
        return;
    }
    shared_ptr<PinLayerImpl> refreshedLayer = layerIterator->second;
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
PinManagerImpl::RefreshTilesOfLayer(const std::vector<TileKeyPtr>& tileKeys,
                                   shared_ptr<string> layerID)
{
    NB_ASSERT_CCC_THREAD(m_pContext);
    
    // Check if layer ID is empty.
    if ((tileKeys.empty()) || (!layerID))
    {
        return;
    }

    // Find the layer by layer ID.
    map<string, shared_ptr<PinLayerImpl> >::iterator layerIterator = m_layerMap.find(*layerID);
    if (layerIterator == m_layerMap.end())
    {
        return;
    }
    shared_ptr<PinLayerImpl> refreshedLayer = layerIterator->second;
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
PinManagerImpl::NotifyPinsRemoved(shared_ptr<vector<PinPtr> > pins)
{
    NB_ASSERT_CCC_THREAD(m_pContext);
    
    // Notify the layer manager that the pins are removed.
    if (m_pLayerManager)
    {
        UnifiedLayerManager* layerManager = static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
        if (layerManager)
        {
            layerManager->NotifyPinsRemoved(pins);
        }
    }
}

PinLayerPtr
PinManagerImpl::GetPinLayer(shared_ptr<std::string> layerID)
{
    NB_ASSERT_CCC_THREAD(m_pContext);
    
    return GetPinLayer(*layerID);
}

PinLayerPtr
PinManagerImpl::GetPinLayer(std::string& layerID)
{
    NB_ASSERT_CCC_THREAD(m_pContext);
    
    PinLayerPtr layer;
    if (!layerID.empty())
    {
        std::map<string, shared_ptr<PinLayerImpl> > :: iterator iter =
                m_layerMap.find(layerID);
        if (iter != m_layerMap.end())
        {
            layer = iter->second;
        }
    }
    return layer;
}

/* See description in header file. */
void PinManagerImpl::Initialize(LayerProviderPtr layerProvider,
                                LayerManagerPtr layerManager,
                                shared_ptr < LayerIdGenerator > idGenerator)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    m_layerProvider         = layerProvider;
    m_pLayerManager         = layerManager;
    m_pLayerIdGenerator     = idGenerator;
}

/* See description in header file. */
void PinManagerImpl::RemoveAllPins()
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    map<string, shared_ptr<PinLayerImpl> >::iterator layerIterator = m_layerMap.begin();
    map<string, shared_ptr<PinLayerImpl> >::iterator layerEnd = m_layerMap.end();
    for (; layerIterator != layerEnd; ++layerIterator)
    {
        shared_ptr<PinLayerImpl> pinLayer = layerIterator->second;
        if (pinLayer)
        {
            pinLayer->RemoveAllPins();
        }
    }
}


/*! @} */
