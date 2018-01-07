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
@file         NKUIListener.h
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

#ifndef __NKUI_NKUIListener_h
#define __NKUI_NKUIListener_h

#include "NKUIPublicState.h"
#include "NKUIData.h"

namespace nkui
{

/*! Errors of NavkitUI. */
enum NKUI_Error
{
    NUE_None = 3000,
    NUE_TimeOut = 3001,
    NUE_BadDestination = 3002,
    NUE_BadOrigin = 3003,
    NUE_CannotRoute = 3004,
    NUE_EmptyRoute = 3005,
    NUE_NetError = 3006,
    NUE_UnknownError = 3007,
    NUE_NoMatch = 3008,
    NUE_ServerError = 3009,
    NUE_NoDetour = 3010,
    NUE_PedRouteTooLong = 3011,
    NUE_OriginCountryUnsuppoted = 3013,
    NUE_DestinationCountryUnsupported = 3014,
    NUE_Unauthorized = 4299,
    NUE_BicycleRouteTooLong = 23500,
    NUE_RouteTooLong = 23501,
    NUE_GPSTimeout = 23502,
    NUE_Cancelled = 23503,
    NUE_Internal  = 23504,
    NUE_ParseLayoutConfigFileError = 23805
};

typedef enum
{
    NKACT_INVALID = 0,
    NKACT_STOP_NAVIGATION,
    NKACT_CANCEL_NAVIGATION,
    NKACT_FINISH_NAVIGATION,
    NKACT_TRANSITION_FROM_LIST_TO_NAVIGATION,
    NKACT_TRANSITION_FROM_LIST_TO_ROUTESELECTION,
    NKACT_TRANSITION_FROM_TRIPOVERVIEW_TO_NAVIGAITON,
    NKACT_CANCEL_DETOUR,
    NKACT_RECALCULATION
} NavigationUIAction;

/*! NKUIController listener
 NKUIController use this listener notify app navigation state changed and when some event
 need confirm .
 */
class NKUIListener
{
public:
    virtual ~NKUIListener() {};

    /*! navigation start.*/
    virtual void OnNavigationStart() = 0;

    /*! RouteOptions change */
    virtual void OnRouteOptionsChanged() = 0; //@todo define routeoption sturct

    /*! Arrived destination. */
    virtual void OnArrived() = 0;

    /*! navigation end */
    virtual void OnNavigationEnd() = 0;

    /*! navigation canceled */
    virtual void OnNavigationCancel() = 0;

    /*! NKUIController use this callback to notify app some event action need confirm.
        if return true, controller will execute the action else will do nothing.
        eg. use press back an NKACT_STOP_NAVIGATION will send, if return true controller
        will stop the navigation.
     */
    virtual bool OnConfirmAction(NavigationUIAction action) = 0;

    /*! TurnbyTurn Navigation started. */
    virtual void OnTurnByTurnNavigationStart() = 0;

    /*! on error occured durning navigation session.*/
    virtual bool OnError(NKUI_Error errorCode) = 0;

    /*! Tell listener to show overflow menu.
     *
     *  @param state  current public state
     *  @param items contain menuitems nkui expected to show
     *  @param value Show or Hide Overflow menu
     *
     */
    virtual void ShowOverflowMenu(NKUIPublicState state, std::vector<MenuOption>& items, bool show) = 0;

    /*! Tell listener the navigation state is changed.
     *
     * @param state -  new state of navigation.
     * @return None.
     */
    virtual void OnNavigationStateChanged(NKUIPublicState state) = 0;
};
}

#endif


/*! @} */
