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
    @file     MapViewGestureHandler.h
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

#ifndef _MAPVIEW_GESTURE_HANDLER_H_
#define _MAPVIEW_GESTURE_HANDLER_H_

#include "MapViewUIInterface.h"
#include "PanGestureHandler.h"
#include "ScaleGestureHandler.h"
#include "RotateGestureHandler.h"
#include "TiltGestureHandler.h"
#include "DoubleTapGestureHandler.h"
#include "LongPressGestureHandler.h"


class MapViewGestureHandler
{
public:
    MapViewGestureHandler(MapViewUIInterface *mapView):m_pMapView(mapView),m_isActived(false) { nsl_assert(mapView); }
    ~MapViewGestureHandler() {}

public:
    // All public methods MUST be called in UI thread !
    void StartDetector();

    void StopDetector();

    bool OnTouchEvent(const GestureEvent &event);

private:
    MapViewUIInterface *m_pMapView;
    PanGestureDetector m_panDetector;
    ScaleGestureHandler m_scaleDetector;
    RotateGestureDetector m_rotateDetector;
    TiltGestureDetector m_tiltDetector;
    DoubleTapGestureDetector m_doubleTapDetector;
    LongPressGestureDetector m_longPressDetector;
    bool m_isActived;
};


#endif