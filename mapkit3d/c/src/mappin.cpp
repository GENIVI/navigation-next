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
 @file     mappin.cpp
 */
/*
 (C) Copyright 2013 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#include "mappin.h"

using namespace nbmap;

class SelectPinData
{
public:
    SelectPinData(MapController &map, MapPin *pin, bool selected):m_mapController(map),
        m_pin(pin),
        m_selected(selected){}
    ~SelectPinData() {}

 public:
    MapController &m_mapController;
    MapPin* m_pin;
    bool m_selected;
};

MapPin::MapPin(PAL_Instance& pal, MapController& controller):m_pal(pal),
    m_mapController(controller)
{
    m_selected = false;
}

MapPin::~MapPin()
{
}

void
MapPin::SetVisible(bool visible)
{
    //@todo:
}

void
MapPin::SelectPinFunc(PAL_Instance* pal, void* userData)
{
     SelectPinData *data = static_cast<SelectPinData*>(userData);
     if(!pal || !data)
     {
        return;
     }

     MapController &mapCotroller = data->m_mapController;
     MapPin *pin = data->m_pin;
     bool selected = data->m_selected;
     delete data;

     if(!pin || !pin->m_id || pin->m_id->empty())
     {
        return;
     }

     shared_ptr<string> pinId = shared_ptr<string>(new string(*(pin->m_id)));
     if(pinId)
     {
        if(mapCotroller.mMapView)
        {
            if(selected)
            {
                mapCotroller.mMapView->SelectPin(pinId);
            }
            else
            {
                mapCotroller.mMapView->UnselectPin(pinId);
            }
        }
     }
}

void
MapPin::SetSelected(bool selected)
{
    uint32 taskId = 0;
    PAL_EventTaskQueueAdd(&m_pal,
                          MapPin::SelectPinFunc,
                          new SelectPinData(m_mapController, this, selected),
                          &taskId);
    m_selected = selected;
}

bool
MapPin::GetSelected()
{
    return m_selected;
}
