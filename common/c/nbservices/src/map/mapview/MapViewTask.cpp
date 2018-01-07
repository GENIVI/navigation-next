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
   @file        MapViewTask.cpp
   @defgroup    nbmap

   Description: Implementation of kinds of MapViewTask.

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

#include "MapViewTask.h"
#include "LayerFunctors.h"
#include "MapLegendInfo.h"
#include "MapView.h"

using namespace nbmap;
using namespace std;

// Implementation of MapViewTask.
MapViewTask::MapViewTask(MapView* handler, TaskFunction function)
 : m_handler(handler),
   m_function(function)
{
    if (handler)
    {
        m_isValid = handler->IsValid();
    }
}

MapViewTask::~MapViewTask()
{
}

void MapViewTask::Execute(void)
{
    // Call the task function. Pass the task object to the function so that it can access
    // any public data in it.
    if (m_handler && m_isValid && *m_isValid)
    {
        (m_handler->*m_function)(this);
    }

    // Delete the task itself. We own the object ourselves.
    delete this;
}

MapViewTaskUnloadTiles::MapViewTaskUnloadTiles(MapView* handler, TaskFunction function)
 : MapViewTask(handler, function)
{
}

MapViewTaskLoadTiles::MapViewTaskLoadTiles(MapView* handler, TaskFunction function)
 : MapViewTask(handler, function)
{
}

MapViewTaskUpdateCommonMaterial::MapViewTaskUpdateCommonMaterial(MapView* handler, TaskFunction function)
 : MapViewTask(handler, function),
   isDay(false),
   isSatellite(false)
{
}

// Implementation of MapViewTaskUpdateFrame
MapViewTaskUpdateFrame::MapViewTaskUpdateFrame(MapView* handler, TaskFunction function)
 : MapViewTask(handler, function)
{
}
// Implementation of MapViewTaskShowBubbleById
MapViewTaskShowBubbleById::MapViewTaskShowBubbleById(MapView* handler,
                                                     TaskFunction function,
                                                     shared_ptr<string> pinId,
                                                     float x,
                                                     float y,
                                                     bool orientation)
 : MapViewTask(handler, function),
   m_pinId(pinId),
   m_x(x),
   m_y(y),
   m_orientation(orientation)
{
}

// Implementation of MapViewTaskHideBubbleById
MapViewTaskHideBubbleById::MapViewTaskHideBubbleById(MapView* handler,
                                                     TaskFunction function,
                                                     shared_ptr<string> pinId)
 : MapViewTask(handler, function),
   m_pinId(pinId)
{
}

// Implementation of MapViewTaskShowBubbleByPin
MapViewTaskShowBubbleByPin::MapViewTaskShowBubbleByPin(MapView* handler,
                                                       TaskFunction function,
                                                       PinPtr pin,
                                                       float x,
                                                       float y,
                                                       bool orientation)
 : MapViewTask(handler, function),
   m_pin(pin),
   m_x(x),
   m_y(y),
   m_orientation(orientation)
{
}

// Implementation of MapViewTaskHideBubbleByPin
MapViewTaskHideBubbleByPin::MapViewTaskHideBubbleByPin(MapView* handler,
                                                       TaskFunction function,
                                                       PinPtr pin)
 : MapViewTask(handler, function),
   m_pin(pin)
{
}

// Implementation of MapViewTaskRemovePinBubble
MapViewTaskRemovePinBubble::MapViewTaskRemovePinBubble(MapView* handler,
                                                       TaskFunction function,
                                                       shared_ptr<vector<PinPtr> > pins)
 : MapViewTask(handler, function),
   m_pins(pins)
{
}

// Implementation of MapViewTaskShowStaticPoiBubble
MapViewTaskShowStaticPoiBubble::MapViewTaskShowStaticPoiBubble(MapView* handler,
                                                       TaskFunction function,
                                                       float x,
                                                       float y,
                                                       bool orientation)
                                                       : MapViewTask(handler, function),
                                                       m_x(x),
                                                       m_y(y),
                                                       m_orientation(orientation)
{
}

// Implementation of MapViewTaskHideStaticPoiBubble
MapViewTaskHideStaticPoiBubble::MapViewTaskHideStaticPoiBubble(MapView* handler,
                                                       TaskFunction function)
                                                       : MapViewTask(handler, function)
{
}

// Implementation of MapViewTaskShowBubbleByPin
MapViewTaskShowBubble::MapViewTaskShowBubble(MapView* handler,
                                                       TaskFunction function,
                                                       shared_ptr<BubbleInterface> bubble,
                                                       float x,
                                                       float y,
                                                       bool orientation)
                                                       : MapViewTask(handler, function),
                                                       m_bubble(bubble),
                                                       m_x(x),
                                                       m_y(y),
                                                       m_orientation(orientation)
{
}

// Implementation of MapViewTaskHideBubbleByPin
MapViewTaskHideBubble::MapViewTaskHideBubble(MapView* handler,
                                                       TaskFunction function,
                                                       shared_ptr<BubbleInterface> bubble)
                                                       : MapViewTask(handler, function),
                                                       m_bubble(bubble)
{
}

// Implementation of MapViewTaskRemovePinBubble
MapViewTaskRemoveBubble::MapViewTaskRemoveBubble(MapView* handler,
                                                       TaskFunction function,
                                                       shared_ptr<vector<shared_ptr<BubbleInterface> > > bubbles)
                                                       : MapViewTask(handler, function),
                                                       m_bubbles(bubbles)
{
}

/* See description in header file. */
MapViewTaskShowMapLegend::MapViewTaskShowMapLegend(MapView* handler,
                                                   TaskFunction function,
                                                   shared_ptr <MapLegendInfo> mapLegend)
 : MapViewTask(handler, function),
   m_pMapLegend(mapLegend)
{
}

/* See description in header file. */
MapViewTaskDropPin::MapViewTaskDropPin(MapView* handler,
                                       TaskFunction function,
                                       double latitude,
                                       double longitude)
 : MapViewTask(handler, function),
   m_latitude(latitude),
   m_longitude(longitude)
{
}

/* See description in header file. */
MapViewTaskAddStaticPoiBubble::MapViewTaskAddStaticPoiBubble(MapView* handler,
                                       TaskFunction function,
                                       const string& id, 
                                       const string& name,
                                       double latitude,
                                       double longtitude)
                                       : MapViewTask(handler, function),
                                       m_id(id),
                                       m_name(name),
                                       m_latitude(latitude),
                                       m_longtitude(longtitude)
{
}

MapViewTaskAnimationLayer::MapViewTaskAnimationLayer(MapView* handler, TaskFunction function)
 : MapViewTask(handler, function)
{
}

MapViewTaskUIAnimationLayer::MapViewTaskUIAnimationLayer(MapView* handler, TaskFunction function,
                                                         bool createLabel, bool removeLabel,
                                                         uint32 timestamp, bool animationCanPlay)
 : MapViewTask(handler, function),
   m_createTimestampLabel(createLabel),
   m_removeTimestampLabel(removeLabel),
   m_timestamp(timestamp),
   m_animationCanPlay(animationCanPlay)
{
}

/* See description in header file. */
UITaskUpdateLocationBubble::UITaskUpdateLocationBubble(MapView* handler,
                                                             TaskFunction function,
                                                             float x,
                                                             float y,
                                                             bool orientation,
                                                             double latitude,
                                                             double longitude)
        : MapViewTask(handler, function),
          m_x(x),
          m_y(y),
          m_orientation(orientation),
          m_latitude(latitude),
          m_longitude(longitude)
{
}

/* See description in header file. */
MapViewTaskGetSpecifiedTiles::MapViewTaskGetSpecifiedTiles(MapView* handler,
                                                           TaskFunction function,
                                                           const vector <TileKeyPtr>& tileKeys,
                                                           const vector <LayerPtr>& layerList,
                                                           const vector <LayerPtr>& ignoreList)
        : MapViewTask(handler, function),
          m_tileKeys(tileKeys),
          m_layerList(layerList),
          m_ignoreList(ignoreList)
{
}


/* See description in header file. */
RenderTaskGetElementsAt::RenderTaskGetElementsAt(MapView* handler,
                                                 NBGMViewController* controller,
                                                 float x, float y,
                                                 set<string>& elements,
                                                 PAL_Event* event)
        : MapViewTask(handler, NULL),
          m_controller(controller),
          m_x(x),m_y(y),m_elements(elements), m_event(event)
{
}

/* See description in header file. */
void RenderTaskGetElementsAt::Execute()
{
    if (m_controller && m_isValid && *m_isValid)
    {
        m_controller->GetElementsAt(m_x, m_y, m_elements);
        if (m_event)
        {
            PAL_EventSet(m_event);
        }
    }

    delete this;
}

/* See description in header file. */
RenderTaskEnableCustomLayerCollisionDetection::RenderTaskEnableCustomLayerCollisionDetection(MapView* handler,
                                                 NBGMViewController* controller,
                                                 const std::string& layerId,
                                                 bool enable)
                                                 : MapViewTask(handler, NULL),
                                                 m_controller(controller),
                                                 m_layerId(layerId),m_enable(enable)
{
}

/* See description in header file. */
void RenderTaskEnableCustomLayerCollisionDetection::Execute()
{
    if (m_controller && m_isValid && *m_isValid)
    {
        m_controller->EnableCustomLayerCollisionDetection(m_layerId, m_enable);
    }

    delete this;
}


CombinedAtomicTask::CombinedAtomicTask(MapView* handler, TaskFunction function)
    :MapViewTask(handler, function)
{
}

CombinedAtomicTask::~CombinedAtomicTask()
{
}

void
CombinedAtomicTask::AddTask(MapViewTask* task)
{
    m_AtomicTasks.push_back(task);
}

void
CombinedAtomicTask::ExcuteAllTasks() const
{
    for(size_t i= 0; i < m_AtomicTasks.size(); ++i)
    {
        m_AtomicTasks[i]->Execute();
    }
}

void
CombinedAtomicTask::ClearAllTasks()
{
    for(size_t i= 0; i < m_AtomicTasks.size(); ++i)
    {
        delete m_AtomicTasks[i];
    }
}

/*! @} */
