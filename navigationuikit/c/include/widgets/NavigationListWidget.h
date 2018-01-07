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
    @file         NavigationListWidget.h
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
    --------------------------------------------------------------------------
*/

/*! @{ */

#ifndef _NKUI_NAVIGATION_LISTWIDGET_H_
#define _NKUI_NAVIGATION_LISTWIDGET_H_

#include "NavigationListBase.h"

namespace nkui
{
/** This listener is used to notify something happend in list widget.
**/
class NavigationListWidgetListener
{
public:
    virtual ~NavigationListWidgetListener() {};

    /** When maneuver item tapped.

        @param maneuverID: The ID of the tapped maneuver.
    **/
    virtual void OnManeuverItemTapped(int maneuverID) = 0;
};

/** This interface is called by presenter to show RTS list informations
    When ever data updated, the presenter need to call these interfaces to update widget
**/
class NavigationListWidget : public NavigationListBase
{
public:
    /** Pop the first maneuver

        @return: None
    **/
    virtual void PopManeuver() = 0;

    /** Update trip distance
        @param distance: trip distance
     **/
    virtual void UpdateTripDistance(std::string distance) {};
    /** Update arrival time
        @param time arrival time
     **/
    virtual void UpdateArrivalTime(std::string time) {};
    /* update traffic, have 3 color 'R'= red 'G' = green 'Y' = yellow */
    virtual void UpdateTraffic(char traffic)  {};

    /**
     * @brief Update the arriving tip message
     * @param destinationSide: The destination is on the left, right or center
     * @param destinationStreet: The street name
     */
    virtual void SetDestinationText(const std::string& destinationSide, const std::string& destinationStreet) {};

    /** Set the listener to widget

        @param listener: the listener instance pointer
    **/
    virtual void SetListener(NavigationListWidgetListener* listener) = 0;
};
}

#endif

/*! @} */
