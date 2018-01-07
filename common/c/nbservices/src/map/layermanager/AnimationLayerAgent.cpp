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
   @file        AnimationLayerAgent.cpp
   @defgroup    nbmap

   Description: Implementation of AnimationlayerAgent.

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

#include "AnimationLayerAgent.h"
#include "AnimationLayer.h"
#include "UnifiedLayerManager.h"
#include "LayerFunctors.h"
#include "FrameListLayer.h"

using namespace nbmap;

static const vector<uint32> EmptyFrameList;

/* See description in header file. */
AnimationLayerAgent::AnimationLayerAgent(shared_ptr <LayerManager>       layerManager,
                                         shared_ptr <Layer>              layer,
                                         shared_ptr <LocalLayerProvider> layerProvider)
        : LayerAgentImpl(layerManager, layer, layerProvider),
          m_needNotifyAdd(true)
{
    RegisterFrameListListener();
}

/* See description in header file. */
AnimationLayerAgent::~AnimationLayerAgent()
{
    UnregisterFrameListListener();
}

void
AnimationLayerAgent::FrameListUpdated(shared_ptr<map<uint32, shared_ptr<FrameListEntry> > > frameList)
{
    vector<uint32> timeStamps;
    if (!frameList->empty())
    {
        map<uint32, FrameListEntryPtr>::reverse_iterator iter = frameList->rbegin();
        map<uint32, FrameListEntryPtr>::reverse_iterator end  = frameList->rend();
        for (; iter != end; ++iter)
        {
            timeStamps.push_back(iter->first);
        }
    }

    NotifyFrameListUpdated(timeStamps);
}

/* See description in header file. */
void AnimationLayerAgent::FrameListFailed(NB_Error /*error*/)
{
    if (IsEnabled())
    {
        NotifyFrameListUpdated(EmptyFrameList);
    }
}

/* See description in header file. */
void AnimationLayerAgent::PostLayerToggled(bool enabled)
{
    UnifiedLayerManager* uLayerManager =
            static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    if (!uLayerManager)
    {
        return;
    }

    // Real animation layer is a child of m_pLayer (DWRFL), loop all children to tell
    // LayerManager about the adding/removing of DWR layers.
    vector<UnifiedLayerPtr> childLayers = GetChildrenLayers();
    vector<UnifiedLayerPtr>::const_iterator iter = childLayers.begin();
    vector<UnifiedLayerPtr>::const_iterator end  = childLayers.end();
    for (; iter != end; ++iter)
    {
        AnimationLayer* layer = static_cast<AnimationLayer*>(iter->get());
        shared_ptr <MapLegendInfo> mapLegend;
        if (layer)
        {
            mapLegend = layer->GetMapLegendInfo();
        }
        uLayerManager->NotifyShowMapLegend(enabled, mapLegend);
        if (enabled)
        {
            if (m_needNotifyAdd)
            {
                m_needNotifyAdd = false;
                uLayerManager->NotifyAnimationLayerAdded(*iter);
            }
        }
        else
        {
            uLayerManager->NotifyAnimationLayerRemoved(*iter);
            m_needNotifyAdd = true;
        }
    }

    LayerAgentImpl::PostLayerToggled(enabled);
}

/* See description in header file. */
void AnimationLayerAgent::RegisterFrameListListener()
{
    FrameListLayer* frameListLayer = static_cast<FrameListLayer*>(m_pLayer.get());
    if (frameListLayer)
    {
        frameListLayer->RegisterListener(this);
    }
}

/* See description in header file. */
void AnimationLayerAgent::UnregisterFrameListListener()
{
    FrameListLayer* frameListLayer = static_cast<FrameListLayer*>(m_pLayer.get());
    if (frameListLayer)
    {
        frameListLayer->UnregisterListener(this);
    }
}


/* See description in header file. */
void AnimationLayerAgent::ResetLayer(shared_ptr <Layer> layer)
{
    UnregisterFrameListListener();
    PrepareResetAnimationLayer();
    LayerAgentImpl::ResetLayer(layer);
    m_needNotifyAdd = true;
    RegisterFrameListListener();
}

/* Prepare reset animation layer. */
void AnimationLayerAgent::PrepareResetAnimationLayer()
{
    if (!m_pLayer->IsEnabled())
    {
        return;
    }

    UnifiedLayerManager* uLayerManager =
            static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    vector<UnifiedLayerPtr> childLayers = GetChildrenLayers();
    if (uLayerManager && !childLayers.empty())
    {
        vector<UnifiedLayerPtr>::iterator iter = childLayers.begin();
        vector<UnifiedLayerPtr>::iterator end  = childLayers.end();
        for (; iter != end; ++iter)
        {
            uLayerManager->NotifyAnimationLayerRemoved(*iter);
        }
    }
}

/* See description in header file. */
void AnimationLayerAgent::NotifyFrameListUpdated(const vector <uint32>& timeStamps)
{
    UnifiedLayerManager* uLayerManager =
            static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    vector<UnifiedLayerPtr> childLayers = GetChildrenLayers();
    if (uLayerManager && !childLayers.empty())
    {
        vector<UnifiedLayerPtr>::iterator iter = childLayers.begin();
        vector<UnifiedLayerPtr>::iterator end  = childLayers.end();
        for (; iter != end; ++iter)
        {
            if (uLayerManager)
            {
                if (m_needNotifyAdd)
                {
                    m_needNotifyAdd = false;
                    uLayerManager->NotifyAnimationLayerAdded(*iter);
                }
                uLayerManager->NotifyAnimationLayerUpdated(*iter, timeStamps);
            }
        }
    }
}




/*! @} */
