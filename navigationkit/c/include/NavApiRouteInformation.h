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
    @file     NavApiRouteInformation.h
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

#ifndef __NAVAPIROUTEINFORMATION_H__
#define __NAVAPIROUTEINFORMATION_H__

#include "NavApiTypes.h"
#include "NavApiRouteSummaryInformation.h"

extern "C"
{
#include "navpublictypes.h"
}

namespace nbnav
{

/*!
 * enum of trafficColor;
 */
enum TrafficSegmentColor
{
    TSC_Transparent = 0,
    TSC_Red,
    TSC_Yellow,
    TSC_Green
};

/*! colorSegment attributes */
class ColorSegmentAttribute
{
public:
    ColorSegmentAttribute(uint32 endIndex, TrafficSegmentColor trafficColor)
        : m_endIndex(endIndex),
        m_trafficColor(trafficColor)
    {
    }

    virtual ~ColorSegmentAttribute()
    {
    }

    uint32  m_endIndex;
    TrafficSegmentColor    m_trafficColor;
};

/*! polyline parameters */
typedef struct
{
    std::vector<Coordinates>              points;
    std::vector<ColorSegmentAttribute>   colorSegments;
    float width;
}TrafficColorSegment;

/*! Non thread safe classes */

/*! Traffic event class.

    It contains either {@link TrafficCongestion} or {@link TrafficIncident}.
*/
class TrafficEvent
{
public:
    typedef enum
    {
        Incident = 0,
        Congestion
    } Type;

    /*! Traffic severity.
    */
    typedef enum{
        Unknown = 0, /*!< Unknown severity. */
        Moderate,    /*!< Moderate severity. */
        Severe       /*!< Severe severity. */
    } TrafficSeverity;

    /*! Returns Returns the delay in seconds due to this traffic event

        @return delay.
    */
    uint32 GetDelay() const;

    /*! Returns a description of the traffic event

        @return description.
    */
    std::string GetDescription() const;

    /*! Returns current distance to traffic event.

        @return current distance to traffic event.
    */
    double GetDistanceToTrafficItem() const;

     /*! Returns the expected end time for this traffic event

         @return time.
    */
    uint32 GetEndTime() const;

    /*! Returns the total length of the congestion

        @return total length.
    */
    double GetLength() const;

    /*! Returns index of maneuver traffic event is within.

        @return index of maneuver.
    */
    unsigned int GetManeuverNumber() const;

    /*! Returns the time at which the event was reported

        @return time.
    */
    unsigned int GetReportTime() const;

    /*! Returns name of the road where the traffic event.

        @return name of the road.
    */
    std::string GetRoad() const;

    /*! Returns the severity of the traffic event (Unknown/Moderate/Severe).

        @return severity.
    */
    TrafficSeverity GetSeverity() const;

    /*! Returns the traffic event start position measured from the start of the trip.

        @return start position.
    */
    double GetStartFromTrip() const;

    /*! Returns the type of traffic event (TYPE_CONGESTION or TYPE_INCIDENT).

        @return type of traffic event.
    */
    Type GetType() const;

    /*! Constructor. For internal use only. */
    TrafficEvent(NavigationImpl* impl, void* data, void* routeData);
    TrafficEvent();

    /*! Destructor. */
    ~TrafficEvent();

private:
    uint32          m_delay;
    std::string     m_description;
    double          m_distanceToTrafficItem;
    unsigned int    m_endTime;
    unsigned int    m_maneuverNumber;
    unsigned int    m_reportTime;
    std::string     m_roadName;
    TrafficSeverity m_severity;
    double          m_startFromTrip;
    double          m_length;
    Type            m_type;
};

/*! Route information. */
class RouteInformation
{
public:

    /*! Gets uncollapsed maneuver list

        @return {@link ManeuverList} object that is associated with this route.
    */
    ManeuverList GetManeuverList() const;
    
    /*! Gets maneuver list for display
     
     @return {@link ManeuverList} object that is associated with this route.
     */
    ManeuverList GetUpcomingManeuverList(double tripRemainingDistance);

    /*! Gets route ID

        @return route ID as a byte array.
    */
    const std::string& GetRouteID() const;

    /*! Gets total route distance in meters

        @return total route distance in meters.
    */
    double GetDistance() const;

    /*! Gets total route time in seconds

        @return total time of route in seconds.
    */
    uint32 GetTime() const;

    /*! Gets total route delay time in seconds

        @return total route delay time in seconds.
    */
    uint32 GetDelay() const;

    /*! Gets route error code

        @return route error code
    */
    NavigateRouteError GetRouteError() const;

    /*! Gets route origin place.

        @return route origin {@link Place} object.
    */
    Place GetOrigin() const;

    /*! Gets route destination place.

        @return rout destination {@link Place} object.
    */
    Place GetDestination() const;

    /*! Gets route bounding box.

        @return {@link BoundingBox} object that contains all of the maneuvers or null
        if there're no available maneuvers.
    */
    BoundingBox GetBoundingBox() const;

    /*! Returns the data corresponding to the given voice name. Can return null if
        the voice data is not present.

        @param name
        @return voice data
    */
    BinaryBuffer GetVoiceFile(std::string name) const;

    /*! Returns the coordinates on route polyline for the entire route

        @return Array of Coordinates
    */
    std::vector<Coordinates> GetPolyline() const;

    /*! Returns the initial guidance text of route.

        @return text of initial guidance
    */
    std::string GetInitialGuidanceText() const;

    /*! Returns the description of route.

       @return text of description
    */
    std::string GetRouteDescriptions() const;

    /*! Get route properties.

        Get route properties which this route contains.

        @return routeProperty array of this route.
    */
    std::vector<RouteProperty> GetRouteProperties() const;

    /**
     * Returns the traffic event const pointer with index.
     * @param index The index of traffic events.
     * @return traffic event const pointer.
     */
    const TrafficEvent* GetTrafficEvent(unsigned int index) const;

    /**
     * Returns the number of traffic events in the route.
     * @return number of traffic events
     */
    unsigned int GetNumberOfTrafficEvent() const;

    /**
     * Returns the traffic color of the route
     * @return char of traffic color
     */
    char GetTrafficColor() const;

    /*! Returns the startup guidance text of route.

        @return text of startup guidance
    */
    std::string GetStartupGuidanceText() const;

    /*! Returns the destination street side of route.

        @return DestinationStreetSide enum value
     */
    DestinationStreetSide GetDestinationStreetSide() const;

    /*! Returns the coordinate for the bubble location on current route.

        @return Coordinates
     */
    Coordinates GetLabelPoint() const;

    /*! Returns the first major road name on current route.

        @return text of first major road.
     */
    std::string GetFirstMajorRoad() const;

    /*! get route summary information list

        this function is valid when do a route summary request, other case it invalid.
        @return route summary vector
     */
    std::vector<RouteSummaryInformation>& GetRouteSummary();

    /*! get Traffic color segment. 
     */
    const TrafficColorSegment& GetTrafficColorSegment() const;
    
    /* Returns the maneuver by index
     
     @param index Index of maneuver in the list (0-based).
     @return Pointer to new {@link Maneuver} object. NULL if error happens.
     */
    const Maneuver* GetManeuver(unsigned int index) const;
    
    /*! Returns the number of uncollapsed maneuver number available in the route.
     
     @return number of maneuvers
     */
    unsigned int GetNumberOfManeuvers() const;

    /*! Costructor.

     For internal use only.

     @param impl
     @param data
     @param index
     */
    RouteInformation(NavigationImpl* impl, void* data, unsigned int index);
    
    void UpdateTraffic(bool newTraffic);

private:
    void CreateTrafficColorSegment();

private:
    ManeuverList               m_maneuverList;
    ManeuverList               m_uncollapsedManeuver;
    std::string                m_routeID;
    double                     m_distance;
    unsigned int               m_time;
    unsigned int               m_delay;
    NavigateRouteError         m_routeError;
    Place                      m_origin;
    Place                      m_destination;
    BoundingBox                m_boundingBox;
    std::vector<Coordinates>   m_routePolylines;
    void*                      m_resources;
    unsigned int               m_index;
    std::vector<RouteProperty> m_routeProperties;
    std::string                m_initialInstruction;
    std::string                m_startupInstruction;
    std::string                m_description;
    std::vector<TrafficEvent>  m_trafficEvents;
    std::vector<RouteSummaryInformation> m_routeSummarys;
    char                       m_trafficColor;
    DestinationStreetSide      m_streetSide;
    Coordinates                m_labelPoint;
    std::string                m_firstMajorRoad;
    bool                       m_isPlanRoute;
    TrafficColorSegment        m_trafficColorSegment;
    void*                      m_navigationImpl;
    uint32                     m_selectedRoute;
    void*       m_routeInfo;

    friend class NavigationImpl;
    friend class Detour;
    friend class RouteRequest;
};

/*! Traffic information. */
class TrafficInformation
{
public:
    /*! Returns the traffic event list that is ahead of the user

        @return vector of TrafficEvent.
    */
    std::vector<TrafficEvent> GetTrafficEvents() const;

    /*! Costructor.

        For internal use only.

        @param impl
    */
    TrafficInformation(NavigationImpl* impl, void* routeData);
    TrafficInformation();

private:
    std::vector<TrafficEvent> m_events;
};

}
#endif

/*! @} */
