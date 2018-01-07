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
 @file     mapcircle.cpp
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

#include "mapcircle.h"
#include "MapViewInterface.h"

using namespace nbmap;

MapCircle::MapCircle(MapController& controller, int id, double lat, double lon, float radius, int fillColor, int strokeColor, int zOrder, bool visible)
   :m_id(id),
    m_lat(lat),
    m_lon(lon),
    m_radius(radius),
    m_fillColor(fillColor),
    m_strokeColor(strokeColor),
    m_zOrder(zOrder),
    m_visible(visible),
    m_removed(false),
    m_mapController(controller)
{
}

MapCircle::~MapCircle()
{
}

int
MapCircle::GetId() const
{
    return m_id;
}

void
MapCircle::SetCenter(double lat, double lon)
{
    if(m_lat != lat || m_lon != lon)
    {
        m_lat = lat;
        m_lon = lon;
        if(!m_removed)
        {
            if(m_mapController.mMapView)
            {
                m_mapController.mMapView->SetCircleCenter(m_id, m_lat, m_lon);
            }
        }
    }
}

void
MapCircle::GetCenter(double &lat, double &lon)const
{
    lat = m_lat;
    lon = m_lon;
}

void
MapCircle::SetRadius(float radius)
{
    if(m_radius != radius)
    {
        m_radius = radius;
        if(!m_removed)
        {
            if(m_mapController.mMapView)
            {
                m_mapController.mMapView->SetCircleStyle(m_id, m_radius, m_fillColor, m_strokeColor);
            }
        }
    }
}

float
MapCircle::GetRadius()const
{
    return m_radius;
}

void
MapCircle::SetFillColor(int fillColor)
{
    if(m_fillColor != fillColor)
    {
        m_fillColor = fillColor;
        if(!m_removed)
        {
            if(m_mapController.mMapView)
            {
                m_mapController.mMapView->SetCircleStyle(m_id, m_radius, m_fillColor, m_strokeColor);
            }
        }
    }
}

int
MapCircle::GetFillColor()const
{
    return m_fillColor;
}

void
MapCircle::SetStrokeColor(int strokeColor)
{
    if(m_strokeColor != strokeColor)
    {
        m_strokeColor = strokeColor;
        if(!m_removed)
        {
            if(m_mapController.mMapView)
            {
                m_mapController.mMapView->SetCircleStyle(m_id, m_radius, m_fillColor, m_strokeColor);
            }
        }
    }
}

int
MapCircle::GetStrokeColor()const
{
    return m_strokeColor;
}

void
MapCircle::SetZOrder(int zOrder)
{
    if(m_zOrder != zOrder)
    {
        m_zOrder = zOrder;
        if(!m_removed)
        {
            //@todo: implement this.
        }
    }
}

int
MapCircle::GetZOrder()const
{
    return m_zOrder;
}

void
MapCircle::SetVisible(bool visible)
{
    if(m_visible != visible)
    {
        m_visible = visible;
        if(!m_removed)
        {
            m_mapController.mMapView->SetCircleVisible(m_id, m_visible);
        }
    }
}

bool
MapCircle::IsVisible()const
{
    return m_visible;
}

void
MapCircle::Remove()
{
    if(m_removed)
    {
        return;
    }

    if(m_mapController.mMapView)
    {
        m_mapController.mMapView->RemoveCircle(m_id);
    }
    m_removed = true;
}

bool
MapCircle::IsRemoved()const
{
    return m_removed;
}