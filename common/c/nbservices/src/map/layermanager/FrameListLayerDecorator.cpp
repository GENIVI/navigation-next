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
   @file         FrameListLayerDecorator.cpp
   @defgroup     nbmap

   Description:  Implementation of FrameListLayerDecorator.
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

#include "FrameListLayerDecorator.h"
#include "AnimationLayer.h"

using namespace nbmap;

/* See description in header file. */
FrameListLayerDecorator::FrameListLayerDecorator(FrameListLayer& instance, uint32 layerId)
        : FrameListLayer(instance),
          m_instance(instance)
{
    m_tileLayerInfo->layerID = layerId;
}

/* See description in header file. */
FrameListLayerDecorator::~FrameListLayerDecorator()
{
    if (m_thisValid)
    {
        m_instance.UnregisterListener(this);
    }
}

/* See description in header file. */
void FrameListLayerDecorator::FrameListUpdated(FrameListMapPtr frameList)
{
    m_frameList = frameList;
    // Try to notify all children about this frameList. This will give them a chance to
    // start pending tile requests (if any).
    vector<UnifiedLayerPtr> childrenLayers = GetAllChildrenLayers();
    vector<UnifiedLayerPtr>::const_iterator iter = childrenLayers.begin();
    vector<UnifiedLayerPtr>::const_iterator end  = childrenLayers.end();
    for (; iter != end; ++iter)
    {
        AnimationLayer* layer = static_cast<AnimationLayer*>(iter->get());
        if (layer)
        {
            layer->FrameListUpdated(frameList);
        }
    }

    NotifyFrameListUpdated();
}

/* See description in header file. */
void FrameListLayerDecorator::FrameListFailed(NB_Error error)
{
    // Try to notify all children about this frameList. This will give them a chance to
    // start pending tile requests (if any).
    vector<UnifiedLayerPtr> childrenLayers = GetAllChildrenLayers();
    vector<UnifiedLayerPtr>::const_iterator iter = childrenLayers.begin();
    vector<UnifiedLayerPtr>::const_iterator end  = childrenLayers.end();
    for (; iter != end; ++iter)
    {
        AnimationLayer* layer = static_cast<AnimationLayer*>(iter->get());
        if (layer)
        {
            layer->FrameListFailed(error);
        }
    }

    // Notify the error to the listeners.
    NotifyFrameListError(error);
}

/* See description in header file. */
UnifiedLayerPtr FrameListLayerDecorator::Clone()
{
    return UnifiedLayerPtr();
}

/* See description in header file. */
void FrameListLayerDecorator::PostLayerToggled()
{
    if (IsEnabled())
    {
        m_instance.RegisterListener(this);

        FrameListMapPtr frameList;
        GetFrameList(frameList);
        if (frameList && !frameList->empty())
        {
            NotifyFrameListUpdated();
        }
    }
    else
    {
        if (m_thisValid)
        {
            m_instance.UnregisterListener(this);
        }
    }
    UnifiedLayer::PostLayerToggled();
}

/* See description in header file. */
NB_Error FrameListLayerDecorator::GetFrameList(FrameListMapPtr& frameList)
{
    return m_thisValid ? m_instance.GetFrameList(frameList) : NE_UNEXPECTED;
}

/*! @} */
