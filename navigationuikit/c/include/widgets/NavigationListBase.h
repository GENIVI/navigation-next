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
   @file        NavigationListBase.h
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

#ifndef _NKUI_NAVIGATIONLISTBASE_H_
#define _NKUI_NAVIGATIONLISTBASE_H_

#include "Widget.h"
#include "NavApiNavigation.h"
#include <vector>

namespace nkui
{

struct ManeuverListData
{
    uint32          maneuverID;
    std::string     primary;
    std::string     secondary;
    std::string     exitText;
    std::string     distanceText;
    std::string     distanceUnit;
    std::string     destAddress;   //this filed only valid when isDestination is true
    uint16          nextTurnCharacter;
    bool            isDestination;

    bool IsDestination()
    {
        return isDestination;
    }
};

struct TrafficListData
{
    nbnav::TrafficEvent::Type               type;
    nbnav::TrafficEvent::TrafficSeverity    severity;
    uint32                                  maneuverID;
    uint32                                  delayTime;
    std::string                             delayText;
    std::string                             delayUnit;
    std::string                             delayText2;
    std::string                             delayUnit2;
    std::string                             description;
};


class NavigationListBase
{
public:
    virtual ~NavigationListBase() {}

    /** Update all the maneuvers on a route.
        This method will clear all traffic events.

        @param maneuvers: The maneuver informations
        @return: None
    **/
    virtual void
    UpdateManeuvers(const std::vector<ManeuverListData>& maneuvers) = 0;

    /** Update all the traffic informations on a route.

        @param trafficEvents: The traffic informations
        @return: None
    **/
    virtual void
    UpdateTrafficEvents(const std::vector<TrafficListData>& trafficEvents) = 0;

    /** Update the list title.
        Such as 'Starting on ...'.

        @param hint: A string like 'starting on'
        @param roadName: Road name.
        @return: None
    **/
    virtual void
    UpdateTitle(const std::string& hint, const std::string& roadName) = 0;

    /** Update the distance for top maneuver

        @param distance: new distance value
        @return: None
    **/
    virtual void
    UpdateDistance(const char* distance, const char* unit) = 0;
};

}

#endif

/*! @} */
