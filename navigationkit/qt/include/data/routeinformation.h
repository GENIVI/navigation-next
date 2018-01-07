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
    @file routeinformation.h
    @date 10/15/2014
    @addtogroup navigationkit
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
#ifndef LOCATIONTOOLKIT_ROUTE_INFORMATION_H
#define LOCATIONTOOLKIT_ROUTE_INFORMATION_H

#include <QtGlobal>
#include <QByteArray>
#include "ltkerror.h"
#include "data/maneuver.h"
#include "data/place.h"
#include "data/trafficevent.h"
#include "data/maneuverlist.h"
#include "data/boundingbox.h"
#include "navigationtypes.h"

namespace locationtoolkit
{
/**
 * Route information.
 */
class RouteInformation
{
public:
    /**
     * Route property.
     *
     * Used for getting the properties of route information.
     */
    enum RouteProperty
    {
        RI_Hov = 0,             /*!< The route contains at least one HOV segment. */
        RI_HighWay,             /*!< The route contains at least one RI_HighWay segment. */
        RI_TollRoad,            /*!< The route contains at least one toll road segment. */
        RI_Ferry,               /*!< The route includes taking a ferry. */
        RI_UnpavedRoad,         /*!< The route includes at least one unpaved road segment. */
        RI_Enter_Country,       /*!< The route contains at lest one enter country segment. */
        RI_Private_Road,        /*!< The route contains at lest one private road. */
        RI_Gated_Access_Road,   /*!< The route contains at lest one gated access road. */
        RI_Congestion,          /*!< The route includes at least one segment where there is a traffic congestion. */
        RI_Accident,            /*!< The route includes at least one segment where there is a traffic accident. */
        RI_Construction         /*!< The route includes at least one segment where there is construction.. */
    };

public:
    virtual ~RouteInformation() {}

    /**
     * Gets maneuver list
     *
     * @return {@link ManeuverList} object that is associated with this route.
     */
    virtual const ManeuverList* GetManeuverList() const = 0;

    /**
     * Gets route ID
     *
     * @return route ID.
     */
    virtual const QString& GetRouteID() const = 0;

    /**
     * Gets total route distance in meters
     *
     * @return total route distance in meters.
     */
    virtual qreal GetDistance() const = 0;

    /**
     * Gets total route time in seconds
     *
     * @return total time of route in seconds.
     */
    virtual quint32 GetTime() const = 0;

    /**
     * Gets total route delay time in seconds
     *
     * @return total route delay time in seconds.
     */
    virtual quint32 GetDelay() const = 0;

    /**
     * Gets route error code
     *
     * @return route error code
     */
    virtual LTKError GetRouteError() const =0;

    /**
     * Gets route origin place.
     *
     * @return route origin {@link Place} object.
     */
    virtual const Place& GetOrigin() const = 0;

    /**
     * Gets route destination place.
     *
     * @return rout destination {@link Place} object.
     */
    virtual const Place& GetDestination() const = 0;

    /**
     * Gets route bounding box.
     *
     * @return {@link BoundingBox} object that contains all of the maneuvers or null
     * if there're no available maneuvers.
     */
    virtual const BoundingBox& GetBoundingBox() const = 0;

    /**
     * Returns the coordinates on route polyline for the entire route
     *
     * @return Array of Coordinates
     */
    virtual const QVector<Coordinates*>& GetPolyline() const = 0;

    /**
     * Returns the initial guidance text of route.
     *
     * @return text of initial guidance
     */
    virtual const QString& GetInitialGuidanceText() const = 0;

    /**
     * Returns the description of route.
     *
     *  @return text of description
     */
    virtual const QString& GetRouteDescriptions() const = 0;

    /**
     * Get route properties.
     *
     * Get route properties which this route contains.
     *
     * @return routeProperty array of this route.
     */
    virtual const QVector<RouteProperty>& GetRouteProperties() const = 0;

    /**
     * Returns the traffic event const pointer with index.
     * @param index The index of traffic events.
     * @return traffic event const pointer.
     */
    virtual const TrafficEvent* GetTrafficEvent(quint32 index) const = 0;

    /**
     * Returns the number of traffic events in the route.
     *
     * @return number of traffic events
     */
    virtual quint32 GetNumberOfTrafficEvent() const = 0;

    /**
     * Returns the traffic color of the route
     * @return char of traffic color
     */
    virtual char GetTrafficColor() const = 0;

    /**
     * Returns the startup guidance text of route.
     *
     * @return text of startup guidance
     */
    virtual const QString& GetStartupGuidanceText() const = 0;

    /**
     * Returns the destination street side of route.
     *
     * @return DestinationStreetSide enum value
     */
    virtual DestinationStreetSide GetDestinationStreetSide() const = 0;

    /**
     * Returns the coordinate for the bubble location on current route.
     *
     * @return Coordinates
     */
    virtual const Coordinates& GetLabelPoint() const = 0;

    /**
     * Returns the first major road name on current route.
     *
     * @return text of first major road.
     */
    virtual const QString& GetFirstMajorRoad() const = 0;
};
} // namespace locationtoolkit
#endif
/*! @} */
