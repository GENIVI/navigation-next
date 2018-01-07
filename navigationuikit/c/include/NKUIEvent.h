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
   @file        NKUIEvent.h
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

#ifndef _NKUIEVENT_H_
#define _NKUIEVENT_H_

namespace nkui
{

typedef enum
{
    EVT_INVALID = 0,
    EVT_STATE_CHANGED,
    EVT_BACK_PRESSED,
    EVT_START_BUTTON_PRESSED,
    EVT_START_NAV,
    EVT_RECALCING,
    EVT_RECALCING_END,
    EVT_GETTING_DETOUR,
    EVT_GETTING_ROUTE,
    EVT_ENDGETTING_DETOUR,
    EVT_DETOUR_SUCCESS,
    EVT_ENT_RTS,
    EVT_NEW_ROUTE_SELECTED,
    EVT_MENU_PRESSED,
    EVT_MENU_SELECTED,
    EVT_MENU_HIDE,
    EVT_ARRIVAL,
    EVT_END_TRIP,
    EVT_CANCEL_RTS,
    EVT_ROUTE_OPTIONS,
    EVT_TAP_MINIMAP,
    EVT_SAR_SHOW,
    EVT_SAR_HIDE,
    EVT_LIST_SCROLL_START,
    EVT_LIST_OPENED,
    EVT_LIST_CLOSED,
    EVT_TAP_RTS_MINIMAP,
    EVT_TAP_NAV_HEADER,
    EVT_NAV_RETRY,
    EVT_NAV_RETRY_TRIGGERED,
    EVT_CURRENTLOCATION_UPDATED,
    EVT_NAV_OFFROUTE,
    EVT_NAV_ONROUTE,
    EVT_ENT_STARTOFFROUTE,
    EVT_MAPSETTING_UPDATED,
    EVT_TAP_HEADER,
    EVT_MAP_CAMERA_UPDATED,
    EVT_MAP_CAMERA_UNLOCKED,
    EVT_MAP_CAMERA_LOCKED,
    EVT_MAP_ZOOM_UPDATED,
    EVT_ROUTE_UPDATE,
    EVT_ERROR_OCCURRED,
    EVT_ENTER_ENHANCED_STARTUP,
    EVT_EXIT_ENHANCED_STARTUP,
    EVT_ROUTE_REQUESTED,
    EVT_ROUTE_PROGRESS,
    EVT_ZOOM_FIT_ROUTE,
    EVT_ENTER_BACKGROUND,               //navuikit has sent to background.
    EVT_ENTER_FOREGROUND,               //navuikit will enter foreground.
    EVT_IDLE_THRESHOLD_REACHED,         //event will be sent if NavUIKit sent to background for 5 minutes.
    EVT_STILL_DRIVING,                  //user wants to contiune the navigation.
    EVT_ALERT_IDLE_THRESHOLD_REACHED,   //event will be sent if NavUIKit sent to background for other 5 minutes.
    EVT_TRIPOVERVIEW_SHOW,              //to show the tripOverView.
    EVT_ENTER_CALLING_STATE,            //enter calling state, include:dialing, incoming and connected
    EVT_LEAVE_CALLING_STATE,            //leave calling state,
    EVT_SCREEN_SIZE_CHANGED,
    EVT_SCREEN_SIZE_CHANGING,
    EVT_LANEGUIDANCE_SHOW,              //laneguidance shown.
    EVT_LANEGUIDANCE_HIDE,              //laneguidance hidden.
    EVT_END_TRIP_PRESSED,
    EVT_TAP_VOLUME_BUTTON,
    EVT_TRAFFIC_CLOSE,
    EVT_TRAFFIC_TRY_T0_SHOW_DETOUR,
    EVT_HIDE_CURRENTROAD,
    EVT_SHOW_CURRENTROAD,

    EVT_TAP_ON_PIN,                     // tapped on a pin
    EVT_TAP_ON_MAP,                     // tapped on the map

    EVT_ANNOUNCE,                       // Notify to play an announcement
    EVT_ANNOUNCE_MANEUVER               // notify to play with selected maneuver.
} NKEventID;

}
#endif /* _NKUIEVENT_H_ */

/*! @} */
