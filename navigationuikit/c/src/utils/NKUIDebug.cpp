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
   @file        NKUIDebug.cpp
   @defgroup    nkui
*/
/*
   (C) Copyright 2014 by TeleCommunication Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */

#include "NKUIDebug.h"
#include <stdlib.h>

//@todo: generate this string table automatically.
const char* WIDGET_DESCRIPTIONS[] = {
    "WGT_INVALID",
    "WGT_MAP",
    "WGT_ANNOUNCER",
    "WGT_REQUEST_FOOTER",
    "WGT_STARTINGNAV_FOOTER",
    "WGT_ROUTE_SELECT_FOOTER",
    "WGT_ROUTE_SELECT_HEADER",
    "WGT_ROUTE_SELECT_BUBBLE",
    "WGT_ROUTE_SELECT_STATUS_BAR",
    "WGT_ENDTRIP_BAR",
    "WGT_NAV_FOOTER",
    "WGT_NAV_HEADER",
    "WGT_OVERFLOW_MENU",
    "WGT_CURRENTROAD",
    "WGT_STACKED_MANEUVER",
    "WGT_MINI_MAP_NAV",
    "WGT_MINI_MAP_LOCATEME",
    "WGT_MINI_MAP_PED",
    "WGT_TRIP_OVERVIEW_HEADER",
    "WGT_ROUTE_DETALLS_LIST",
    "WGT_NAVIGATION_LIST",
    "WGT_SAR",
    "WGT_LANE_GUIDANCE",
    "WGT_LANE_GUIDANCE_SAR",
    "WGT_SPEED_LIMITS",
    "WGT_ARRIVAL_FOOTER",
    "WGT_ARRIVAL_HEADER",
    "WGT_MINI_MAP_RTS",
    "WGT_RETRY",
    "WGT_DETOUR_FOOTER",
    "WGT_DETOUR_HEADER",
    "WGT_NOGPS",
    "WGT_PEDESTRIAN_FOOTER",
    "WGT_PEDESTRIAN_HEADER",
    "WGT_PEDESTRIAN_LIST",
    "WGT_RECALCULATE",
    "WGT_ENHANCE_NAV_NOTIFICATION",
    "WGT_ENHANCE_HEADER",
    "WGT_STATUS_BAR",
    "WGT_BACKGROUND",
    "WGT_MAP_PREFETCH",
    "WGT_COMPASS",
    "WGT_STOP_NAVIGATION",
    "WGT_PIN_BUBBLE",
    NULL
};


const char* GetWidgetDescription(nkui::WidgetID id)
{
    return id < nkui::WGT_MAX ? WIDGET_DESCRIPTIONS[id] : "InvalidID";
}

/*! @} */
