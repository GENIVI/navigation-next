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
  @file         WidgetID.h
  @defgroup     nkui
*/
/*
  (C) Copyright 2014 by TeleCommunications Systems, Inc.

  The information contained herein is confidential, proprietary to
  TeleCommunication Systems, Inc., and considered a trade secret as defined
  in section 499C of the penal code of the State of California. Use of this
  information by anyone other than authorized employees of TeleCommunication
  Systems is granted only under a written non-disclosure agreement, expressly
  prescribing the scope and manner of such use.
  --------------------------------------------------------------------------*/

/*! @{ */

#ifndef _NKUI_WIDGETID_H_
#define _NKUI_WIDGETID_H_

namespace nkui
{
#define FOREACH_WIDGET(PROCESS)                 \
    PROCESS(WGT_INVALID)                        \
    PROCESS(WGT_MAP)                            \
    PROCESS(WGT_ANNOUNCER)                      \
    PROCESS(WGT_REQUEST_FOOTER)                 \
    PROCESS(WGT_STARTINGNAV_FOOTER)             \
    PROCESS(WGT_ROUTE_SELECT_FOOTER)            \
    PROCESS(WGT_ROUTE_SELECT_HEADER)            \
    PROCESS(WGT_ROUTE_SELECT_BUBBLE)            \
    PROCESS(WGT_ROUTE_SELECT_STATUS_BAR)        \
    PROCESS(WGT_ENDTRIP_BAR)                    \
    PROCESS(WGT_NAV_FOOTER)                     \
    PROCESS(WGT_NAV_HEADER)                     \
    PROCESS(WGT_OVERFLOW_MENU)                  \
    PROCESS(WGT_CURRENTROAD)                    \
    PROCESS(WGT_STACKED_MANEUVER)               \
    PROCESS(WGT_MINI_MAP_NAV)                   \
    PROCESS(WGT_MINI_MAP_LOCATEME)              \
    PROCESS(WGT_MINI_MAP_PED)                   \
    PROCESS(WGT_TRIP_OVERVIEW_HEADER)           \
    PROCESS(WGT_ROUTE_DETALLS_LIST)             \
    PROCESS(WGT_NAVIGATION_LIST)                \
    PROCESS(WGT_SAR)                            \
    PROCESS(WGT_LANE_GUIDANCE)                  \
    PROCESS(WGT_LANE_GUIDANCE_SAR)              \
    PROCESS(WGT_SPEED_LIMITS)                   \
    PROCESS(WGT_ARRIVAL_FOOTER)                 \
    PROCESS(WGT_ARRIVAL_HEADER)                 \
    PROCESS(WGT_MINI_MAP_RTS)                   \
    PROCESS(WGT_RETRY)                          \
    PROCESS(WGT_DETOUR_FOOTER)                  \
    PROCESS(WGT_DETOUR_HEADER)                  \
    PROCESS(WGT_NOGPS)                          \
    PROCESS(WGT_PEDESTRIAN_FOOTER)              \
    PROCESS(WGT_PEDESTRIAN_HEADER)              \
    PROCESS(WGT_PEDESTRIAN_LIST)                \
    PROCESS(WGT_RECALCULATE)                    \
    PROCESS(WGT_ENHANCE_NAV_NOTIFICATION)       \
    PROCESS(WGT_ENHANCE_HEADER)                 \
    PROCESS(WGT_STATUS_BAR)                     \
    PROCESS(WGT_BACKGROUND)                     \
    PROCESS(WGT_MAP_PREFETCH)                   \
    PROCESS(WGT_COMPASS)                        \
    PROCESS(WGT_STOP_NAVIGATION)                \
    PROCESS(WGT_PIN_BUBBLE)                     \
    PROCESS(WGT_MAX)                            \
    PROCESS(WGT_TRAFFIC_AHEAD)                  \
    PROCESS(WGT_OVERVIEW_FOOTER)                \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum WidgetID {
    FOREACH_WIDGET(GENERATE_ENUM)
};

static const char* WidgetName[] =  {
    FOREACH_WIDGET(GENERATE_STRING)
};

const char* GetWidgetName(WidgetID id);

WidgetID GetWidgetID(const char* name);

}

#endif

/*! @} */
