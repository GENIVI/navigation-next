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
   @file        LayerAgentImpl.cpp
   @defgroup    nbmap

   Description: Implementation of LayerAgentImpl

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

#include "LayerAgentImpl.h"
#include "AnimationLayerAgent.h"
#include "UnifiedLayerManager.h"
#include "UnifiedLayer.h"
#include "StringUtility.h"
#include "LocalLayerProvider.h"
#include "nbmacros.h"

using namespace nbmap;

class LayerAgentParameters
{
public:
    LayerAgentParameters( LayerAgentImpl& agent, bool enabled)
            : m_agent(agent),
              m_enabled(enabled)
    {}
    virtual ~LayerAgentParameters(){}

    LayerAgentImpl& m_agent;
    bool m_enabled;
private:
    NB_DISABLE_COPY(LayerAgentParameters)
};

void CCC_SetLayerEnabled(PAL_Instance* /*pal*/, void* userData)
{
    LayerAgentParameters* param = static_cast<LayerAgentParameters*>(userData);
    if (param)
    {
        param->m_agent.CCC_SetEnabled(param->m_enabled);
        delete param;
    }
}

/* See description in header file. */
LayerAgentPtr LayerAgentImpl::CreateLayerAgent(shared_ptr <LayerManager> layerManager,
                                               shared_ptr <Layer> layer,
                                               shared_ptr <LocalLayerProvider> layerProvider)
{
    LayerAgentPtr agentLayer;
    UnifiedLayer* uLayer = static_cast<UnifiedLayer*>(layer.get());
    if (layerManager && uLayer)
    {
        if (uLayer->IsAnimationLayer())
        {
            agentLayer.reset(new AnimationLayerAgent(layerManager, layer, layerProvider));
        }
        else
        {
            agentLayer.reset(new LayerAgentImpl(layerManager, layer, layerProvider));
        }
    }
    return agentLayer;
}


/* See description in header file. */
LayerAgentImpl::LayerAgentImpl(shared_ptr <LayerManager> layerManager,
                               shared_ptr <Layer> layer,
                               shared_ptr <LocalLayerProvider> layerProvider)
        : m_pLayerManager(layerManager),
          m_pLayer(layer),
          m_pLayerProvider(layerProvider)
{
}

/* See description in header file. */
LayerAgentImpl::~LayerAgentImpl()
{
}

/* See description in header file. */
NB_Error LayerAgentImpl::SetEnabled(bool enabled)
{

    PAL_Instance* pal = NB_ContextGetPal(m_pLayerProvider->GetContext());
    if (PAL_IsPalThread(pal))
    {
        CCC_SetEnabled(enabled);
        return NE_OK;
    }

    // get should be thread safe, if we find issues here, we need to add locks...
    uint32 taskId = 0;
    if (PAL_EventTaskQueueAdd(NB_ContextGetPal(m_pLayerProvider->GetContext()),
                              CCC_SetLayerEnabled,
                              new LayerAgentParameters(*this, enabled),
                              &taskId) == PAL_Ok)
    {
        return NE_OK;
    }

    return NE_UNEXPECTED;
}

/* See description in header file. */
shared_ptr <string> LayerAgentImpl::GetName()
{
    shared_ptr<string> layerName;
    if (m_pLayer)
    {
        UnifiedLayer* uLayer = static_cast<UnifiedLayer*>(m_pLayer.get());
        if (uLayer)
        {
            layerName = uLayer->GetLayerName();
        }
    }
    return layerName;
}

/* See description in header file. */
void LayerAgentImpl::PostLayerToggled(bool enabled)
{
    if (!m_pLayerProvider)
    {
        return;
    }

    vector<LayerPtr> layers(1, m_pLayer);
    bool needToNotify =  enabled ? m_pLayerProvider->AddLayer(m_pLayer) :
                         m_pLayerProvider->RemoveLayer(m_pLayer);

    LayerOperator function;
    if (enabled)
    {
        function = &LocalLayerProvider::AddLayer;
    }
    else
    {
        function = &LocalLayerProvider::RemoveLayer;
    }

    bool refreshAllTiles = false;
    UnifiedLayer* uLayer = static_cast<UnifiedLayer*>(m_pLayer.get());
    if (uLayer)
    {

        // We may need to deactivate multiple other layers, for the sake of simplicity, just
        // refresh all tiles. If there are performance issue, optimize this.
        if (!uLayer->GetMutexLayers().empty())
        {
            refreshAllTiles = true;
        }

        const vector<UnifiedLayerPtr>& childrenLayers = uLayer->GetAllChildrenLayers();
        vector<UnifiedLayerPtr>::const_iterator iter = childrenLayers.begin();
        vector<UnifiedLayerPtr>::const_iterator end  = childrenLayers.end();
        for (; iter != end; ++iter)
        {
            UnifiedLayerPtr layer = *iter;
            if (layer)
            {
                if (((m_pLayerProvider.get())->*function)(layer))
                {
                    needToNotify = true;
                    layers.push_back(layer);
                }
            }
        }
    }

    if (needToNotify || refreshAllTiles)
    {
        UnifiedLayerManager* uLayerManager =
                static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
        if (uLayerManager)
        {
            if (enabled)
            {
                uLayerManager->NotifyLayersAdded(layers);
            }
            else
            {
                uLayerManager->NotifyLayersRemoved(layers);
            }

            if (refreshAllTiles)
            {
                uLayerManager->RefreshAllTiles();
            }
        }
    }
}

/* See description in header file. */
void LayerAgentImpl::ResetLayer(shared_ptr <Layer> layer)
{
    if (!m_pLayerProvider)
    {
        return;
    }

    UnifiedLayer* uLayer = static_cast<UnifiedLayer*>(m_pLayer.get());
    if (uLayer)
    {
        vector<LayerPtr> layers(1, m_pLayer);
        bool needToNotify =  m_pLayerProvider->RemoveLayer(m_pLayer);

        const vector<UnifiedLayerPtr>& childrenLayers = uLayer->GetAllChildrenLayers();
        vector<UnifiedLayerPtr>::const_iterator iter = childrenLayers.begin();
        vector<UnifiedLayerPtr>::const_iterator end  = childrenLayers.end();
        for (; iter != end; ++iter)
        {
            UnifiedLayerPtr layer = *iter;
            if (layer)
            {
                if (m_pLayerProvider->RemoveLayer(layer))
                {
                    needToNotify = true;
                    layers.push_back(layer);
                }
            }
        }

        if (needToNotify)
        {
            UnifiedLayerManager* uLayerManager =
                    static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
            if (uLayerManager)
            {
                uLayerManager->NotifyLayersRemoved(layers);
            }
        }

    }

    // Reset m_pLayer to new layer.
    m_pLayer = layer;
}

/* See description in header file. */
shared_ptr < Layer > LayerAgentImpl::GetLayerPtr()
{
    return m_pLayer;
}

/* See description in header file. */
bool LayerAgentImpl::IsEnabled()
{
    bool enabled = false;
    UnifiedLayer* uLayer = NULL;
    if (m_pLayer &&
        ((uLayer = static_cast<UnifiedLayer*>(m_pLayer.get()))) != NULL)
    {
        enabled = uLayer->IsEnabled();
    }
    return enabled;
}

/* See description in header file. */
vector <UnifiedLayerPtr> LayerAgentImpl::GetChildrenLayers()
{
    vector<UnifiedLayerPtr> childLayers;
    UnifiedLayer* uLayer = static_cast<UnifiedLayer*>(m_pLayer.get());
    if (uLayer)
    {
        childLayers = uLayer->GetAllChildrenLayers();
    }
    return childLayers;
}

/* See description in header file. */
void LayerAgentImpl::CCC_SetEnabled(bool enabled)
{
    UnifiedLayer* uLayer = static_cast<UnifiedLayer*>(m_pLayer.get());
    if (uLayer)
    {
        uLayer->SetEnabled(enabled);
        PostLayerToggled(enabled);
    }
}


/*! @} */
