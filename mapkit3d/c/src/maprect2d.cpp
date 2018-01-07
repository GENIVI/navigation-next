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
 @file     maprect2d.cpp
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

#include "maprect2d.h"

using namespace nbmap;

MapRect2d::MapRect2d(MapController& controller, int id, double lat, double lon, float heading, float width, float height, bool visible)
   :m_id(id),
    m_lat(lat),
    m_lon(lon),
    m_heading(heading),
    m_width(width),
    m_height(height),
    m_visible(visible),
    m_removed(false),
    m_mapController(controller)
{
}

MapRect2d::~MapRect2d()
{
}

int
MapRect2d::GetId() const
{
    return m_id;
}

void
MapRect2d::Update(double lat, double lon, float heading)
{
    if(m_lat != lat || m_lon != lon || m_heading != heading)
    {
        m_lat = lat;
        m_lon = lon;
        m_heading = heading;
        if(!m_removed)
        {
            if(m_mapController.mMapView)
            {
                m_mapController.mMapView->UpdateRect2d(m_id, m_lat, m_lon, m_heading);
            }
        }
    }
}

void
MapRect2d::SetSize(float width, float height)
{
    if(m_width != width || m_height != height)
    {
        m_width = width;
        m_height = height;
        if(!m_removed)
        {
            if(m_mapController.mMapView)
            {
                m_mapController.mMapView->SetRect2dSize(m_id, m_width, m_height);
            }
        }
    }
}

void
MapRect2d::SetVisible(bool visible)
{
    if(m_visible != visible)
    {
        m_visible = visible;
        if(!m_removed)
        {
            m_mapController.mMapView->SetRect2dVisible(m_id, m_visible);
        }
    }
}

void
MapRect2d::Remove()
{
    if(m_removed)
    {
        return;
    }

    if(m_mapController.mMapView)
    {
        m_mapController.mMapView->RemoveRect2d(m_id);
    }
    m_removed = true;
}

void
MapRect2d::GetCenter(double &lat, double &lon)const
{
    lat = m_lat;
    lon = m_lon;
}

float
MapRect2d::GetHeading()const
{
    return m_heading;
}

void
MapRect2d::GetSize(float &width, float &height)const
{
    width = m_width;
    height = m_height;
}

bool
MapRect2d::IsVisible()const
{
    return m_visible;
}

bool
MapRect2d::IsRemoved()const
{
    return m_removed;
}