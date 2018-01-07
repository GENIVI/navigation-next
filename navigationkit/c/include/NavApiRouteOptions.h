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
    @file     NavApiRouteOptions.h
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __NAVAPIROUTEOPTIONS_H__
#define __NAVAPIROUTEOPTIONS_H__

#include <string>

namespace nbnav
{

/*! Non thread safe classes */

typedef enum
{
    Fastest = 0,  /*!< The route should be the fastest route. */
    Shortest,     /*!< The route should be the shortest route. */
    Easiest       /*!< The route should be the easiest route. */
} RouteType;

typedef enum
{
    AVD_HOV       = (1 << 0),  /*!< The route should avoid using HOV lanes. */
    AVD_Toll      = (1 << 1),  /*!< The route should avoid using toll roads. */
    AVD_Highway   = (1 << 2),  /*!< The route should avoid using highways. */
    AVD_Ferry     = (1 << 3),  /*!< The route should avoid using ferry. */
    AVD_Unpaved   = (1 << 4)   /*!< The route should avoid using unpaved roads. */
} RouteAvoid;

typedef enum
{
    Car = 0,     /*!< The route should be appropriate for cars and motorcycles. */
    Truck,       /*!< The route should be appropriate for trucks. */
    Bicycle,     /*!< The route should be appropriate for bicycles. */
    Pedestrian   /*!< The route should be appropriate for pedestrians. */
} TransportationMode;

/*! RouteOptions

    Specifies the various settings which control the route generated.
*/
class RouteOptions
{
public:
    /*! Creates a new instance of <code>RouteOptions</code>.

        @param routeType the type of route to get.
        @param transportationMode the type of vehicle to route for.
        @param avoid the features to avoid when routing. Must be either
               zero or a combination of {@link #HOV}, {@link #Toll}, {@link #Highway},
               {@link #Ferry} and {@link #Unpaved}.
        @param pronunciationStyle the pronunciation style.
    */
    RouteOptions(RouteType routeType, TransportationMode transportationMode,
                 unsigned int avoid, std::string pronunciationStyle = std::string());

    /*! Copy constructor. */
    RouteOptions(const RouteOptions& routeOptions);

    /*! Gets a set of features to avoid.

        @return either zero or a combination of {@link #HOV}, {@link #Toll},
        {@link #Highway} and {@link #Ferry}.
    */
    unsigned int GetAvoidFeatures() const;

    /*! Sets the features to avoid.

        @param avoid either zero or a combination of {@link #HOV}, {@link #Toll},
        {@link #Highway} and {@link #Ferry}.
        @return None
     */
    void SetAvoidFeatures(unsigned int avoid);

    /*! Gets the route type.

        @return route type.
    */
    RouteType GetRouteType() const;

    /*! Sets the type of route.

        @param routeType type of route.
        @return None
    */
    void SetRouteType(RouteType routeType);

    /*! Gets the vehicle type for the route.

        @return vehicle type.
    */
    TransportationMode GetTransportationMode() const;

    /*! Sets the vehicle type for the route.

        @param transportationMode vehicle type.
        @return None
    */
    void SetTransportationMode(TransportationMode transportationMode);

    /*! Gets the pronunciation style

        @return pronunciation style text.
    */
    std::string GetPronunciationStyle() const;

    /*! Sets the pronunciation style

        @param style pronunciation style
    */
    void SetPronunciationStyle(std::string style);

    /*! Assignment operator overload.

        @param right The assignment source object.
        @return RouteOptions object
    */
    RouteOptions& operator=(const RouteOptions& right);

private:
    unsigned int       m_avoid;
    RouteType          m_routeType;
    TransportationMode m_transportationMode;
    std::string        m_pronunciationStyle;
};

}

#endif

/*! @} */