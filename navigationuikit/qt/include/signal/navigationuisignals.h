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
    @file navigationuisignals.h
    @date 10/15/2014
    @addtogroup navigationuikit
*/
/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */
#ifndef LOCATIONTOOLKIT_NAVIGATIONUI_SIGNALS_H
#define LOCATIONTOOLKIT_NAVIGATIONUI_SIGNALS_H

#include <QObject>
#include <QtGlobal>
#include <QVector>

/**
* NavigationUISingals provides navigation events for users(applications) slots.
* Application(client) need to prepare slots to connect these signals if client want to
* receive these signals.
*/
namespace locationtoolkit
{

/**
 *  navigation public state
 */
typedef enum
{
    LPS_Invalid = 0,
    LPS_Starting_Navigation,        /** starting navigation state */
    LPS_Request_Navigation,         /** request navigation state*/
    LPS_Route_Selection,            /** route selection state */
    LPS_Navigation,                 /** navigating state */
    LPS_Detour,                     /** detour state*/
    LPS_Arrival,                    /** navigation arrival state*/
    LPS_Route_Overview              /** route overview state*/
}PublicState;

class MenuItem;
class NavigaitonUIKitWrapper;
class NavigationUISignals: public QObject
{
    Q_OBJECT
public:
    ~NavigationUISignals(){}
Q_SIGNALS:
    /**
     * Informs client that navigation is started.
     */
    void OnNavigationStart();

     /**
     * Informs client that Route Options are changed..
     */
    void OnRouteOptionsChanged();

     /**
     * Informs client that navigation ends.
     *
     * Navigation may end as a result of two reasons: destination arrived or navigation aborted.
     * If a delegate need to distinguish these two cases, delegate needs to implement onArrived
     * method.
     */
    void OnNavigationEnd();

     /**
     * Informs client that we have arrived our destination.
     */
    void OnArrived();

    /**
     * Informs client that navigation is cancelled before turn by turn navigation starts.
     */
    void OnNavigationCancel();

    /**
     * Informs client that Turn by Turn navigation is started.
     */
    void OnTurnByTurnNavigationStart();

    /**
     * Informs client that navigation state changed.
     *
     */
    void OnNavigationStateChanged(PublicState state);

    /**
     *  show Menu Items, this function is called when overflowmenu icon click
     *
     *  @param state current navigaiton state
     *  @param Items menu items LTKNU excepted to show, it is a array of MenuItem
     *  @param value Show or Hide Overflow menu
     *
     */
    void ShowOverflowMenu(PublicState state, QVector<MenuItem>& options, bool show);

private:
    explicit     NavigationUISignals(QObject* parent): QObject(parent){}
    friend class NavigaitonUIKitWrapper;
};
}  // namespace locationtoolkit
#endif
/*! @} */
