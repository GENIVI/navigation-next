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
 @file     maplayer.cpp
 */
/*
 (C) Copyright 2014 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#include "maplayer.h"
#include "CustomLayer.h"

using namespace nbmap;

MapLayer::MapLayer(PAL_Instance& pal, MapController& controller, bool visible):m_pal(pal),
    m_mapController(controller),
    m_layerId(0),
    m_visible(visible)
{
}

MapLayer::~MapLayer()
{
}

class LoadTileData
{
public:
    LoadTileData(MapController &map, MapTile *tile, MapLayer *layer):m_mapController(map),
        m_tile(tile),
        m_layer(layer){}
    ~LoadTileData() {}

 public:
    MapController &m_mapController;
    MapTile *m_tile;
    MapLayer *m_layer;
};

class LayerData
{
public:
    LayerData(MapController &map, MapLayer *layer, int x = 0, int y = 0, int zoom = 0):m_mapController(map),
        m_layer(layer), m_x(x), m_y(y), m_zoom(zoom){}
    ~LayerData() {}

 public:
    MapController &m_mapController;
    MapLayer *m_layer;
    int m_x;
    int m_y;
    int m_zoom;
};

class LayerVisibleData
{
public:
    LayerVisibleData(MapController &map, MapLayer *layer, bool visible):m_mapController(map),
        m_layer(layer), m_visible(visible){}
    ~LayerVisibleData() {}

 public:
    MapController &m_mapController;
    MapLayer *m_layer;
    bool m_visible;
};

bool MapLayer::IsVisible()const
{
    if(!m_mapController.mCustomLayerManager)
    {
        return false;
    }
    return m_visible;
}

void MapLayer::SetVisible(bool visible)
{
    if(!m_mapController.mCustomLayerManager)
    {
        return;
    }
    m_visible = visible;
    uint32 taskId = 0;
    PAL_EventTaskQueueAdd(&m_pal,
                          MapLayer::SetVisibleFunc,
                          new LayerVisibleData(m_mapController, this, visible),
                          &taskId);
}

void MapLayer::Invalidate(int x, int y, int zoom)
{
    if(!m_mapController.mCustomLayerManager)
    {
        return;
    }
    uint32 taskId = 0;
    PAL_EventTaskQueueAdd(&m_pal,
                          MapLayer::InvalidateFunc,
                          new LayerData(m_mapController, this, x, y, zoom),
                          &taskId);
}

void MapLayer::Invalidate(int zoom)
{
    if(!m_mapController.mCustomLayerManager)
    {
        return;
    }
    uint32 taskId = 0;
    PAL_EventTaskQueueAdd(&m_pal,
                          MapLayer::InvalidateZoomFunc,
                          new LayerData(m_mapController, this, 0, 0, zoom),
                          &taskId);
}

void MapLayer::Invalidate()
{
    if(!m_mapController.mCustomLayerManager)
    {
        return;
    }
    uint32 taskId = 0;
    PAL_EventTaskQueueAdd(&m_pal,
                          MapLayer::InvalidateAllFunc,
                          new LayerData(m_mapController, this),
                          &taskId);
}

uint32 MapLayer::GetLayerId() const
{
    return m_layerId;
}

void MapLayer::SetLayerId(uint32 id)
{
    m_layerId = id;
}

void
MapLayer::LoadTileFunc(PAL_Instance* pal, void* userData)
{
     LoadTileData *data = static_cast<LoadTileData*>(userData);
     if(!pal || !data)
     {
        return;
     }

     MapController &mapCotroller = data->m_mapController;
     MapTile *tile = data->m_tile;
     MapLayer *layer = data->m_layer;
     delete data;

     if(!tile || !(tile->GetTileData()))
     {
        return;
     }

     mapCotroller.mCustomLayerManager->LoadTile(layer->GetLayerId(), tile->GetTileData());
     delete tile;
}

void MapLayer::LoadTile(MapTile* tile)
{
    if(tile == NULL || !m_mapController.mCustomLayerManager)
    {
        return;
    }

    set<MapTile*>::iterator it = m_mapController.mTileSet.find(tile);
    if(it == m_mapController.mTileSet.end())
    {
        return;
    }
    m_mapController.mTileSet.erase(it);
    uint32 taskId = 0;
    PAL_EventTaskQueueAdd(&m_pal,
                          MapLayer::LoadTileFunc,
                          new LoadTileData(m_mapController, tile, this),
                          &taskId);
}

void
MapLayer::InvalidateFunc(PAL_Instance* pal, void* userData)
{
     LayerData *data = static_cast<LayerData*>(userData);
     if(!pal || !data)
     {
        return;
     }

     MapController &mapCotroller = data->m_mapController;
     MapLayer *layer = data->m_layer;
     int x = data->m_x;
     int y = data->m_y;
     int zoom = data->m_zoom;
     delete data;

     if(!layer || layer->GetLayerId() == 0)
     {
        return;
     }

     shared_ptr<CustomLayer> customlayer =  mapCotroller.mCustomLayerManager->GetCustomLayer(layer->GetLayerId());
     if(customlayer)
     {
         customlayer->Invalidate(x, y, zoom);
     }
}

void
MapLayer::InvalidateZoomFunc(PAL_Instance* pal, void* userData)
{
     LayerData *data = static_cast<LayerData*>(userData);
     if(!pal || !data)
     {
        return;
     }

     MapController &mapCotroller = data->m_mapController;
     MapLayer *layer = data->m_layer;
     int zoom = data->m_zoom;
     delete data;

     if(!layer || layer->GetLayerId() == 0)
     {
        return;
     }

     shared_ptr<CustomLayer> customlayer =  mapCotroller.mCustomLayerManager->GetCustomLayer(layer->GetLayerId());
     if(customlayer)
     {
         customlayer->Invalidate(zoom);
     }
}

void
MapLayer::InvalidateAllFunc(PAL_Instance* pal, void* userData)
{
     LayerData *data = static_cast<LayerData*>(userData);
     if(!pal || !data)
     {
        return;
     }

     MapController &mapCotroller = data->m_mapController;
     MapLayer *layer = data->m_layer;
     delete data;

     if(!layer || layer->GetLayerId() == 0)
     {
        return;
     }

     shared_ptr<CustomLayer> customlayer =  mapCotroller.mCustomLayerManager->GetCustomLayer(layer->GetLayerId());
     if(customlayer)
     {
         customlayer->Invalidate();
     }
}

void
MapLayer::SetVisibleFunc(PAL_Instance* pal, void* userData)
{
     LayerVisibleData *data = static_cast<LayerVisibleData*>(userData);
     if(!pal || !data)
     {
        return;
     }

     MapController &mapCotroller = data->m_mapController;
     MapLayer *layer = data->m_layer;
     bool visible = data->m_visible;
     delete data;

     if(!layer || layer->GetLayerId() == 0)
     {
        return;
     }

     mapCotroller.mCustomLayerManager->SetVisible(layer->GetLayerId(), visible);
}

