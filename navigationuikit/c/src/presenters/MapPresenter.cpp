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
 @file         MapPresenter.cpp
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
#include "nkui_macros.h"
#include "MapPresenter.h"
#include "palmath.h"
#include <cmath>
#include <vector>
#include <algorithm>

#include "NavApiNavUtils.h"
#include "map/BreadCrumb.h"

extern "C"
{
#include "paluitaskqueue.h"
#include "palclock.h"
}

using namespace std;
using namespace nkui;
using namespace nbnav;

const float NAVIGATING_ZOOM_LEVEL       = 18.8f;
const float NAVIGATING_TILT_ANGLE       = 37.293f;
const float MAP_HEADINGAGGLE            = 0;
const float ARRIVAL_ZOOMLEVEL           = 16;               // closer than zoom level 16
const float ARRIVAL_TILT_ANGLE          = 90;
const float PEDESTRIAN_NAVIGATING_ZOOM_LEVEL = 18.0f;
const float PEDESTRIAN_NAVIGATING_TILT_ANGLE = 90.0f;

const float ROUTE_POLYLINE_WIDTH             = 18.0f;
const float MANEUVER_ARROW_MANEUVER_WIDTH    = 12.0f;
const float MANEUVER_ARROW_ARROW_WIDTH       = MANEUVER_ARROW_MANEUVER_WIDTH * 2;
const float MANEUVER_ARROW_ARROW_LENGTH      = MANEUVER_ARROW_ARROW_WIDTH;

const double MAX_MANEUVER_ARROW_TAIL_LENGTH   = 90.0; // meters
const double MAX_MANEUVER_ARROW_HEADER_LENGTH = 46.0; // meters

const int ANIMATION_TRANSITION_DELAYED_TIME   = 1000; // milliseconds

#define LENGTH_THRESHOLD        0.1     // meters
#define TOLERANCE               0.001

#define ZOOM_TOP_MARGIN         0.35f
#define ZOOM_TOP_MARGIN_L       0.47f
#define ZOOM_BOTTOM_MARGIN      0.15f
#define ZOOM_BOTTOM_MARGIN_L    0.2f
#define ZOOM_LEFT_MARGIN        0.1f
#define ZOOM_RIGHT_MARGIN       0.1f
#define MAX_DEBUG_COUNT         255

#define COORDINATE_NAV2NKUI(X)       NKUICoordinate(X.latitude, X.longitude)

class FinalSegementOutputer
{
public:
    FinalSegementOutputer(Logger* logger)
        : m_logger(logger) {}

    FinalSegementOutputer(const FinalSegementOutputer& instance)
        : m_logger(instance.m_logger) {}

    virtual ~FinalSegementOutputer(){}
    void operator ()(const NKUIColorSegmentAttribute& colorSegment)
    {
        NKUI_DEBUG_LOG(m_logger, "MapPresenter - Segement: index: %d, color: %d\n",
                        colorSegment.m_endIndex, colorSegment.m_trafficColor);
    }
private:
    Logger* m_logger;
};

typedef map<double, NKUITrafficColor> TrafficSegmentMap;

class ColorSegmentOutputer
{
public:
    ColorSegmentOutputer(Logger* loger)
        : m_logger(loger)
    {
    }

    ColorSegmentOutputer(const ColorSegmentOutputer& instance)
        : m_logger(instance.m_logger),
          m_index(instance.m_index)
    {
    }

    virtual ~ColorSegmentOutputer()
    {}

    void operator() (const TrafficSegmentMap::value_type& p)
    {
        NKUI_DEBUG_LOG(m_logger, "MapPresenter: Segment: %d, distance: %lf, color: %d\n",
                       ++m_index, p.first, p.second);

    }
private:
    Logger* m_logger;
    int m_index;
};

class NKUICoordinateCollector
{
public:
    NKUICoordinateCollector(vector<NKUICoordinate>* points)
        : m_points(points) {}
    virtual ~NKUICoordinateCollector(){}
    void operator() (const Coordinates& coordinate)
    {
        if (m_points)
        {
            m_points->push_back(COORDINATE_NAV2NKUI(coordinate));
        }
    }
private:
    vector<NKUICoordinate>* m_points;
};

static void ConvertBoundingBox(const nbnav::BoundingBox& nbnavBoundingBox,
                               NKUICoordinateBounds& nkuiBoundingBox)
{
    nkuiBoundingBox.northEast.m_latitude  = MAX(nbnavBoundingBox.point1.latitude,
                                                nbnavBoundingBox.point2.latitude);
    nkuiBoundingBox.northEast.m_longitude = MAX(nbnavBoundingBox.point1.longitude,
                                                nbnavBoundingBox.point2.longitude);
    nkuiBoundingBox.southWest.m_latitude  = MIN(nbnavBoundingBox.point1.latitude,
                                                nbnavBoundingBox.point2.latitude);
    nkuiBoundingBox.southWest.m_longitude = MIN(nbnavBoundingBox.point1.longitude,
                                                nbnavBoundingBox.point2.longitude);
}

/*! get coordinates of polyline in the length.
 *
 * @param sourcePolyline - source coordinates of maneuver
 * @param resultPolyline - result
 * @param meterLength - length in meter, this is the max length from last coordinate
 *
 * @return pointer to vector of coordinates.
 */
void CalculateCoordinatesOfPolylineInMeterLength(const std::vector<nbnav::Coordinates>& sourcePolyline,
                                                 std::vector<NKUICoordinate>& resultPolyline,
                                                 double meterLength)
{
    if ((sourcePolyline.size() == 0) || meterLength <= 0)
    {
        return;
    }

    NKUICoordinate currentCoordinates;
    vector<nbnav::Coordinates>::const_iterator iter = sourcePolyline.begin();
    while (iter != sourcePolyline.end())
    {
        if(currentCoordinates.m_longitude != 0 && currentCoordinates.m_latitude != 0)
        {
            double distance = nbnav::NavUtils::CalculateDistance(currentCoordinates.m_latitude,
                                                                 currentCoordinates.m_longitude,
                                                                 iter->latitude,
                                                                 iter->longitude,
                                                                 NULL);
            if (meterLength >= distance)
            {
                NKUICoordinate coordinate(iter->latitude,iter->longitude);
                resultPolyline.push_back(coordinate);
                meterLength -= distance;
                if (meterLength == 0)
                {
                    break;
                }
            }
            else
            {
                double percent = meterLength / distance;
                percent = (percent >= 0.1) ? percent : 0.1;
                percent = (percent <= 0.9) ? percent : 0.9;
                double currentX = 0.0;
                double currentY = 0.0;
                double nextX = 0.0;
                double nextY = 0.0;
                nbnav::NavUtils::MercatorForward(currentCoordinates.m_latitude,
                                                 currentCoordinates.m_longitude,
                                                 &currentX, &currentY);
                nbnav::NavUtils::MercatorForward(iter->latitude, iter->longitude, &nextX, &nextY);
                double newX = currentX + (nextX - currentX) * percent;
                double newY = currentY + (nextY - currentY) * percent;
                double newLatitude = 0.0;
                double newLongitude = 0.0;
                nbnav::NavUtils::MercatorReverse(newX, newY ,&newLatitude, &newLongitude);
                NKUICoordinate coordinate(newLatitude, newLongitude);
                resultPolyline.push_back(coordinate);
                break;
            }
            currentCoordinates.m_latitude = iter->latitude;
            currentCoordinates.m_longitude = iter->longitude;
        }
        else
        {
            currentCoordinates.m_latitude = iter->latitude;
            currentCoordinates.m_longitude = iter->longitude;
            resultPolyline.push_back(currentCoordinates);
        }
        ++iter;
    }
}

/*! Create parameter of maneuver arrow
 *
 * @param maneuver - the maneuver which we will draw maneuver arrow.
 * @param isArrowHead - ManeuverArrow needs two maneuver, one for arrow body, one for arrowhead.
 *
 * @return pointer to NKUIManeuverArrowParameter created.
 */
shared_ptr<NKUIManeuverArrowParameter> CreateManeuverArrowParameter(const nbnav::Maneuver* maneuver,
                                                                    bool isArrowHead)
{
    shared_ptr<NKUIManeuverArrowParameter> pManeuverParameter(NKUI_NEW NKUIManeuverArrowParameter());
    if (!pManeuverParameter)
    {
        return pManeuverParameter;
    }

    if (!isArrowHead)
    {
        vector<nbnav::Coordinates> coordinates = maneuver->GetPolyline();
        std::reverse(coordinates.begin(), coordinates.end());
        CalculateCoordinatesOfPolylineInMeterLength(coordinates,
                                                    pManeuverParameter->points,
                                                    MAX_MANEUVER_ARROW_TAIL_LENGTH);
        std::reverse(pManeuverParameter->points.begin(), pManeuverParameter->points.end());
    }
    else
    {
        CalculateCoordinatesOfPolylineInMeterLength(maneuver->GetPolyline(),
                                                    pManeuverParameter->points,
                                                    MAX_MANEUVER_ARROW_HEADER_LENGTH);
    }

    pManeuverParameter->maneuverwidth  = MANEUVER_ARROW_MANEUVER_WIDTH;
    pManeuverParameter->arrowWidth     = MANEUVER_ARROW_ARROW_WIDTH;
    pManeuverParameter->arrowLength    = MANEUVER_ARROW_ARROW_LENGTH;

    return pManeuverParameter;
}

MapPresenter::MapPresenter()
    : m_currentRouteIndex(0),
      m_pOriginPin(NULL),
      m_pDestinationPin(NULL),
      m_pBreadCrumbs(NULL),
      m_heading(0),
      m_headingToRoute(0),
      m_isShowManeuverArrow(false),
      m_waitTransitionEnd(true),
      m_isShowCompass(false),
      m_animationId(0),
      m_animationCallbackCounter(0)
{
    m_lastCameraPosition.m_zoomLevel = NAVIGATING_ZOOM_LEVEL;
    m_lastCameraPosition.m_tiltAngle = NAVIGATING_TILT_ANGLE;
}

MapPresenter::~MapPresenter()
{
    ClearPolylines();
    RemoveManeuverArrow();
    ClearPins();
    m_gpsInterpolator.reset();
    if (m_pBreadCrumbs != NULL)
    {
        delete m_pBreadCrumbs;
    }
}

void MapPresenter::ZoomToFitMutipleRoute(bool retryIfFailed)
{
    NKUICoordinateBounds boundingBox;
    const nbnav::RouteInformation& routeInfo = m_pNavUIContex->GetChosenRoute();
    ConvertBoundingBox(routeInfo.GetBoundingBox(), boundingBox);
    const vector<nbnav::RouteInformation>& routes = m_pNavUIContex->GetAllRoutes();
    vector<nbnav::RouteInformation>::const_iterator iterRoute = routes.begin();
    while (iterRoute != routes.end())
    {
        const nbnav::RouteInformation& routeInfo = *iterRoute;
        Coordinates origin = routeInfo.GetOrigin().GetLocation().center;
        Coordinates destination = m_pNavUIContex->GetDestination().GetLocation().center;

        const nbnav::BoundingBox& routeBoundingBox = routeInfo.GetBoundingBox();
        boundingBox.northEast.m_latitude = MAX(boundingBox.northEast.m_latitude, routeBoundingBox.point1.latitude);
        boundingBox.northEast.m_latitude = MAX(boundingBox.northEast.m_latitude, routeBoundingBox.point2.latitude);
        boundingBox.northEast.m_latitude = MAX(boundingBox.northEast.m_latitude, origin.latitude);
        boundingBox.northEast.m_latitude = MAX(boundingBox.northEast.m_latitude, destination.latitude);
        boundingBox.northEast.m_longitude = MAX(boundingBox.northEast.m_longitude, routeBoundingBox.point1.longitude);
        boundingBox.northEast.m_longitude = MAX(boundingBox.northEast.m_longitude, routeBoundingBox.point2.longitude);
        boundingBox.northEast.m_longitude = MAX(boundingBox.northEast.m_longitude, origin.longitude);
        boundingBox.northEast.m_longitude = MAX(boundingBox.northEast.m_longitude, destination.longitude);

        boundingBox.southWest.m_latitude = MIN(boundingBox.southWest.m_latitude, routeBoundingBox.point1.latitude);
        boundingBox.southWest.m_latitude = MIN(boundingBox.southWest.m_latitude, routeBoundingBox.point2.latitude);
        boundingBox.southWest.m_latitude = MIN(boundingBox.southWest.m_latitude, origin.latitude);
        boundingBox.southWest.m_latitude = MIN(boundingBox.southWest.m_latitude, destination.latitude);
        boundingBox.southWest.m_longitude = MIN(boundingBox.southWest.m_longitude, routeBoundingBox.point1.longitude);
        boundingBox.southWest.m_longitude = MIN(boundingBox.southWest.m_longitude, routeBoundingBox.point2.longitude);
        boundingBox.southWest.m_longitude = MIN(boundingBox.southWest.m_longitude, origin.latitude);
        boundingBox.southWest.m_longitude = MIN(boundingBox.southWest.m_longitude, destination.latitude);
        iterRoute ++;
    }
    ZoomToFitBoundingBox(boundingBox, retryIfFailed);
}

void MapPresenter::UpdateMapSetting(const MapSetting& mapSetting)
{
    if (mapSetting == m_currentMapSetting)
    {
        return;
    }

    m_currentMapSetting = mapSetting;

    NKUIMapInterface* pMapInterface = m_pNavUIContex->GetMapInterface();

    // Update preferred zoom level and tile angle..
    if (m_pNavUIContex->GetPublicState() == NPS_Navigation)
    {
        if (m_pNavUIContex->GetRouteOptions().GetTransportationMode() == nbnav::Pedestrian)
        {
            m_lastCameraPosition.m_zoomLevel =  PEDESTRIAN_NAVIGATING_ZOOM_LEVEL;
            m_lastCameraPosition.m_tiltAngle =  PEDESTRIAN_NAVIGATING_TILT_ANGLE;
        }
        else
        {
            m_lastCameraPosition.m_zoomLevel =  NAVIGATING_ZOOM_LEVEL;
            m_lastCameraPosition.m_tiltAngle =  NAVIGATING_TILT_ANGLE;
        }
    }

    pMapInterface->SetAvatarMode(mapSetting.avatarMode);
    if (mapSetting.trackingMode != NCTM_Free)
    {
        HandleEvent(EVT_MAP_CAMERA_LOCKED, NULL);
        ShowCompass(false);
        if (mapSetting.cameraMode != NCM_MapMode)
        {
            NKUILocation location = m_pNavUIContex->GetCurrentLocation();

            if (m_avatarLocation)
            {
                location.m_latitude   = m_avatarLocation->latitude;
                location.m_longitude  = m_avatarLocation->longitude;
            }

            if(m_pNavUIContex->IsStartupOffroute())
            {
                location.m_heading  = m_headingToRoute;
            }
            else
            {
                location.m_heading  = m_heading;
            }
            if (mapSetting.polylineType == NNM_TurnByTurn)
            {
                location.m_horizontalUncertainAlongAxis = 0;
            }
            SetAvatarLocationAndCamera(location,true);
        }
        else
        {
            NKUICoordinateBounds boundingBox;
            switch (mapSetting.zoomType)
            {
                case NZTAT_FitMutipleRoute:
                {
                    ZoomToFitMutipleRoute();
                    break;
                }
                case NZTAT_FitSingleRoute:
                {
                    const nbnav::RouteInformation& routeInfo = m_pNavUIContex->GetChosenRoute();
                    ConvertBoundingBox(routeInfo.GetBoundingBox(), boundingBox);
                    ZoomToFitBoundingBox(boundingBox);
                    break;
                }
                case NZTAT_FitPOI:
                case NZTAT_None:
                default:
                    break;
            }
        }
    }
    else
    {
        pMapInterface->LockCameraPosition(false);
        ShowCompass(true);
    }
    // Enable or disable Map's reference center based on CameraAngleMode
    pMapInterface->EnableReferenceCenter(mapSetting.cameraMode == NCM_NavMode);

    std::vector<NKUIPolyline*>::iterator iterPolyline = m_polylines.begin();
    while (iterPolyline != m_polylines.end())
    {
        NKUIPolyline* pPolyline = *iterPolyline;
        pPolyline->SetNavMode(mapSetting.polylineType);
        iterPolyline++;
    }


    m_isShowManeuverArrow = mapSetting.showManeuverArrow;
    RemoveManeuverArrow(false);
    if (m_isShowManeuverArrow)
    {
        if (m_pManeuverArrowParameter)
        {
            m_pManeuverArrow = m_pNavUIContex->GetMapInterface()->AddManeuverArrow(*m_pManeuverArrowParameter);
            m_pManeuverArrow->SetSelected(true);
        }
    }
}

void MapPresenter::OffRoutePositionUpdate(double headingToRoute)
{
    m_headingToRoute = headingToRoute;
}

void MapPresenter::OnCameraUpdate(const NKUICameraPosition& camera)
{
    HandleEvent(EVT_MAP_CAMERA_UPDATED, (void*)&camera);
    if (abs(camera.m_tiltAngle - m_lastCameraPosition.m_tiltAngle) > TOLERANCE ||
        abs(camera.m_zoomLevel - m_lastCameraPosition.m_zoomLevel) > TOLERANCE)
    {
        m_lastCameraPosition.m_tiltAngle = camera.m_tiltAngle;
        if (!m_waitTransitionEnd)
        {
            // Sometimes mapInterface may notify us "cameraUpdate" after animation finished,
            // as a result, MapPresenter will put map into unlock mode. Since MapInterface
            // can't ensure calling sequence of OnCameraUpdate() and OnAnimationFinish(), we
            // added this special checking, this works for current Mapkit3d, but we may need
            // to change this logic if switching to a thirdparty map.
            if (m_expectedCameraPostion)
            {
                if (abs(camera.m_zoomLevel - m_expectedCameraPostion->m_zoomLevel) > TOLERANCE)
                {
                    HandleEvent(EVT_MAP_ZOOM_UPDATED, (void*)&(camera.m_zoomLevel));
                    m_expectedCameraPostion.reset();
                }
            }
            else if (abs(camera.m_zoomLevel - m_lastCameraPosition.m_zoomLevel) > TOLERANCE)
            {
                HandleEvent(EVT_MAP_ZOOM_UPDATED, (void*)&(camera.m_zoomLevel));
            }

            m_lastCameraPosition.m_zoomLevel = camera.m_zoomLevel;
        }

        UpdateBreadCrumb();
    }
}

void MapPresenter::OnUnlocked()
{
    if (m_waitTransitionEnd)
    {
        return;
    }

    HandleEvent(EVT_MAP_CAMERA_UNLOCKED, NULL);
    ShowCompass(true);
    //  update mapsetting.
    m_currentMapSetting = m_pNavUIContex->GetMapSetting();
}

void MapPresenter::OnPolylineClick(const std::vector<NKUIPolyline*>& polyline)
{
    int selectedPolylineIndex = -1;
    if (!m_polylines.empty())
    {
        uint32 index = (uint32)((m_currentRouteIndex + 1) % m_polylines.size());
        while (index != m_currentRouteIndex)
        {
            const NKUIPolyline* pPolyline = m_polylines[index];
            if (std::find(polyline.begin(), polyline.end(), pPolyline) != polyline.end())
            {
                selectedPolylineIndex = (int)index;
                break;
            }
            index = (index + 1) % m_polylines.size();
        }
    }

    if (selectedPolylineIndex >= 0)
    {
        HandleEvent(EVT_NEW_ROUTE_SELECTED, &selectedPolylineIndex);
    }

    if (m_pNKUIController)
    {
        NKUI_DEBUG_LOG(m_pNKUIController->GetLogger(),"Taping  to select a route polyline.\n");
    }

}

void MapPresenter::OnPinSelected(NKUIPin* pin)
{
    if (m_pOriginPin == pin)
    {
        PinBubbleParameter parameter;
        NKUICoordinate coordinate = m_pOriginPin->GetCoordinate();
        parameter.latitude = coordinate.m_latitude;
        parameter.longitude = coordinate.m_longitude;
        parameter.name = m_pStringHelper->GetLocalizedString("IDS_ORIGIN");
        HandleEvent(EVT_TAP_ON_PIN, &parameter);
    }
    else if (m_pDestinationPin == pin)
    {
        PinBubbleParameter parameter;
        NKUICoordinate coordinate = m_pDestinationPin->GetCoordinate();
        parameter.latitude = coordinate.m_latitude;
        parameter.longitude = coordinate.m_longitude;
        parameter.name = m_pNavUIContex->GetDestination().GetName();
        HandleEvent(EVT_TAP_ON_PIN, &parameter);
    }
}

void MapPresenter::OnTapAtCoordinate(const nkui::NKUICoordinate& coordinate)
{
    HandleEvent(EVT_TAP_ON_MAP, (void*)(&coordinate));
}

void MapPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    NKUIMapInterface* pMapInterface = m_pNavUIContex->GetMapInterface();
    switch (event)
    {
        case EVT_CURRENTLOCATION_UPDATED:
        {
            const NKUILocation& currentLocation = m_pNavUIContex->GetCurrentLocation();

            if (!m_avatarLocation)
            {
                m_avatarLocation.reset(NKUI_NEW Coordinates);
            }

            m_avatarLocation->latitude  = currentLocation.Latitude();
            m_avatarLocation->longitude = currentLocation.Longitude();

            nbnav::Coordinates coord = {0};
            coord.latitude = currentLocation.Latitude();
            coord.longitude = currentLocation.Longitude();
            coord.accuracy = currentLocation.HorizontalUncertaintyAlongAxis();
            InterpolatedGPSUpdated(coord, currentLocation.Heading());
            if (m_pNavUIContex->GetRouteOptions().GetTransportationMode() == nbnav::Pedestrian
                && m_pNavUIContex->GetPublicState() >= NPS_Navigation)
            {
                AddBreadCrumb(currentLocation);
            }
            break;
        }
        case EVT_ZOOM_FIT_ROUTE:
        {
            ZoomToFitMutipleRoute();
            break;
        }
        case EVT_SCREEN_SIZE_CHANGED:
        {
            NKUINavigationMode navMode = m_pNavUIContex->GetMapSetting().polylineType;
            if((navMode == NNM_TripOverView || navMode == NNM_RTS) && m_pNavUIContex->GetMapSetting().trackingMode != NCTM_Free)
            {
                ZoomToFitMutipleRoute(true);
            }
            if(m_isShowCompass)
            {
                ShowCompass(m_isShowCompass);
            }
            break;
        }
        case EVT_TAP_MINIMAP:
        {
            NKUIPublicState state = m_pNavUIContex->GetPublicState();
            if (state == NPS_Route_Overview || state == NPS_Route_Selection ||
                state == NPS_Detour)
            {
                ZoomToFitMutipleRoute();
                break;
            }
        }
        case EVT_MAPSETTING_UPDATED:
        {
            const MapSetting& mapSetting = m_pNavUIContex->GetMapSetting();
            UpdateMapSetting(mapSetting);
            break;
        }
        case EVT_START_BUTTON_PRESSED:
        {
            if (m_currentRouteIndex < m_polylines.size())
            {
                // clear polylines other than the selected one
                NKUIPolyline* pCurrentRoute = m_polylines[m_currentRouteIndex];
                m_polylines.erase(m_polylines.begin() + m_currentRouteIndex);
                pMapInterface->RemoveRoutes(m_polylines);
                m_polylines.clear();
                pCurrentRoute->SetNavMode(m_pNavUIContex->GetMapSetting().polylineType);
                m_polylines.push_back(pCurrentRoute);
                m_currentRouteIndex = 0;
            }
            break;
        }
        case EVT_NEW_ROUTE_SELECTED:
        {
            int* routeIndex = (int*)data.eventData;
            // unselect previous route
            if (m_currentRouteIndex < m_polylines.size())
            {
                NKUIPolyline* pCurrentRoute = m_polylines[m_currentRouteIndex];
                pCurrentRoute->SetSelected(false);
            }

            // select current route
            if (*routeIndex >= 0)
            {
                m_currentRouteIndex = *routeIndex;
                if (m_currentRouteIndex < m_polylines.size())
                {
                    NKUIPolyline* pCurrentRoute = m_polylines[m_currentRouteIndex];
                    pCurrentRoute->SetSelected(true);
                }
            }
            break;
        }
        case EVT_ROUTE_UPDATE:
        {
            if (m_pNavUIContex->GetRouteOptions().GetTransportationMode() == nbnav::Pedestrian)
            {
                m_gpsInterpolator->StopInterpolate();
            }
            else
            {
                if (m_pBreadCrumbs)
                {
                    m_pBreadCrumbs->Clear();
                }
            }

            // clear maneuver if selected route is new..
            if (m_currentRouteIndex != m_polylines.size() - 1)
            {
                RemoveManeuverArrow(true);
            }

            m_currentRouteIndex = 0;
            CreateRoutes(m_pNavUIContex->GetAllRoutes());

            if (!m_polylines.empty())
            {
                m_currentRouteIndex = m_pNavUIContex->GetChosenRouteIndex();
                NKUIPolyline* pPolyline = m_polylines[m_currentRouteIndex];
                pPolyline->SetSelected(true);
            }

            break;
        }
        case EVT_ARRIVAL:
        {
            NKUICameraPosition camera;
            camera.m_coordinate.m_latitude   = m_pDestinationPin->GetCoordinate().m_latitude;
            camera.m_coordinate.m_longitude  = m_pDestinationPin->GetCoordinate().m_longitude;
            camera.m_zoomLevel             = ARRIVAL_ZOOMLEVEL;
            camera.m_headingAngle          = m_heading;
            camera.m_tiltAngle             = ARRIVAL_TILT_ANGLE;

            m_lastCameraPosition.m_zoomLevel = camera.m_zoomLevel;
            pMapInterface->SetCamera(camera, PrepareAnimationParameter(
                                         &MapPresenter::AnimationCallback,
                                         &MapPresenter::AnimationDefaultFailover,
                                         __FUNCTION__));
            ShowCompass(true);
            break;
        }
        case EVT_RECALCING_END:
        {
            RemoveManeuverArrow();
            break;
        }
        case EVT_STATE_CHANGED:
        {
            if(m_isShowCompass)
            {
                ShowCompass(true);
            }
            break;
        }
        default:
            break;
    }
}

void MapPresenter::TrafficChanged(const nbnav::TrafficInformation& trafficInfo)
{
    // @ntoe: temp code, should compare traffic events instead of this!
    if (m_pNavUIContex->GetPublicState() == NPS_Route_Selection)
    {
        return;
    }

    const nbnav::RouteInformation& routeInformation = m_pNavUIContex->GetChosenRoute();
    vector<nbnav::TrafficEvent> traffics = trafficInfo.GetTrafficEvents();
    NKUIPolylineParameter* pPolylineParameter = NULL;
    if (m_currentRouteIndex < m_polylineParameters.size())
    {
        pPolylineParameter = m_polylineParameters[m_currentRouteIndex];
        CreateRoute(*pPolylineParameter, routeInformation, traffics);
    }
    if (pPolylineParameter && !pPolylineParameter->points->empty()
        && m_currentRouteIndex < m_polylines.size())
    {
        NKUIPolyline* pPolyline = m_polylines[m_currentRouteIndex];
        pPolyline->UpdateTrafficPolyline(pPolylineParameter);
    }
}

void MapPresenter::ClearPolylines()
{
    NKUIMapInterface* pMapInterface = m_pNavUIContex->GetMapInterface();
    if (pMapInterface)
    {
        pMapInterface->RemoveRoutes(m_polylines);
    }
    m_polylines.clear();
    std::for_each(m_polylineParameters.begin(), m_polylineParameters.end(), DestroyPolylineParameter);
    m_polylineParameters.clear();
}

void MapPresenter::ClearPins()
{
    NKUIMapInterface* pMapInterface = m_pNavUIContex->GetMapInterface();
    if (pMapInterface)
    {
        pMapInterface->RemovePins();
    }
    m_pOriginPin = NULL;
    m_pDestinationPin = NULL;
}

void MapPresenter::OnActive()
{
    NKUIMapInterface* pMapInterface = m_pNavUIContex->GetMapInterface();
    pMapInterface->SetNKMapListener(this);
    m_pNavUIContex->GetNavigation()->AddSessionListener(this);
    m_pNavUIContex->GetNavigation()->AddTrafficListener(this);
    m_pNavUIContex->GetNavigation()->AddRoutePositionListener(this);
    m_pNKUIController->RegisterEvent(EVT_CURRENTLOCATION_UPDATED, this);
    m_pNKUIController->RegisterEvent(EVT_MAPSETTING_UPDATED, this);
    m_pNKUIController->RegisterEvent(EVT_NEW_ROUTE_SELECTED, this);
    m_pNKUIController->RegisterEvent(EVT_START_BUTTON_PRESSED, this);
    m_pNKUIController->RegisterEvent(EVT_TAP_MINIMAP, this);
    m_pNKUIController->RegisterEvent(EVT_ROUTE_UPDATE, this);
    m_pNKUIController->RegisterEvent(EVT_ARRIVAL, this);
    m_pNKUIController->RegisterEvent(EVT_ZOOM_FIT_ROUTE, this);
    m_pNKUIController->RegisterEvent(EVT_SCREEN_SIZE_CHANGED, this);
    m_pNKUIController->RegisterEvent(EVT_RECALCING_END, this);
    m_pNKUIController->RegisterEvent(EVT_STATE_CHANGED, this);

    m_gpsInterpolator.reset(NKUI_NEW GPSInterpolator(m_pNavUIContex->GetPalInstance()));
    m_gpsInterpolator->SetPositionListener(this);

    if(pMapInterface)
    {
        WidgetPosition position = m_pNKUIController->GetWidgetPosition(WGT_MAP);
        pMapInterface->SetSize(position);
    }
    m_currentRouteIndex = m_pNavUIContex->GetChosenRouteIndex();
    CreateRoutes(m_pNavUIContex->GetAllRoutes());
    if (m_currentRouteIndex < m_polylines.size())
    {
        NKUIPolyline* pPolyline = m_polylines[m_currentRouteIndex];
        pPolyline->SetSelected(true);
    }
}

void MapPresenter::OnDeactivate()
{
    NKUIMapInterface* pMapInterface = m_pNavUIContex->GetMapInterface();
    pMapInterface->SetNKMapListener(NULL);
    m_gpsInterpolator->StopInterpolate();
    m_gpsInterpolator->SetPositionListener(NULL);
    m_pNavUIContex->GetNavigation()->RemoveTrafficListener(this);
    m_pNavUIContex->GetNavigation()->RemoveSessionListener(this);
    m_pNavUIContex->GetNavigation()->RemoveRoutePositionListener(this);
    ClearPolylines();
    ClearPins();
    RemoveManeuverArrow();
    if (m_pBreadCrumbs != NULL)
    {
        m_pBreadCrumbs->Clear();
    }
}

void MapPresenter::OnSetWidget()
{

}

void MapPresenter::HandleEvent(NKEventID id, void* data)
{
    if (id == EVT_NEW_ROUTE_SELECTED)
    {
        int* routeIndex = (int*)data;
        m_pNavUIContex->SetChosenRouteIndex(*routeIndex);

        NKUIEventData eventData;
        eventData.source = this;
        eventData.eventData = data;
        NotifyEvent(id, eventData);
    }
    PresenterBase::HandleEvent(id, data);
}

void MapPresenter::CreateRoutes(const std::vector<nbnav::RouteInformation>& routes)
{
    ClearPolylines();
    ClearPins();

    NKUIMapInterface* pMapInterface = m_pNavUIContex->GetMapInterface();
    if (routes.empty() || routes[0].GetPolyline().empty())
    {
        return;
    }

    // Create route polyline
    std::vector<nbnav::RouteInformation>::const_iterator iter = routes.begin();
    while (iter != routes.end())
    {
        const nbnav::RouteInformation& route = *iter;
        std::vector<nbnav::TrafficEvent> trafficEvents;
        uint32 trafficEventCount = iter->GetNumberOfTrafficEvent();
        for (uint32 i = 0; i < trafficEventCount; ++i)
        {
            const nbnav::TrafficEvent* pTrafficEvent = iter->GetTrafficEvent(i);
            trafficEvents.push_back(*pTrafficEvent);
        }
        NKUIPolylineParameter* pPolylineParameter = CreatePolylineParameter();
        if (pPolylineParameter != NULL)
        {
            if (CreateRoute(*pPolylineParameter, route, trafficEvents))
            {
                m_polylineParameters.push_back(pPolylineParameter);
                NKUIPolyline* polyline = pMapInterface->AddRoute(*pPolylineParameter);
                polyline->SetNavMode(m_pNavUIContex->GetMapSetting().polylineType);
                polyline->SetSelected(false);
                m_polylines.push_back(polyline);
            }
        }
        iter++;
    }

    // Create Origin and Destination Pins.
    // Get first route
    const nbnav::RouteInformation& route = *routes.begin();
    NKUIPinParameter pinParameter;

    // use first point in polyline as origin
    const vector<nbnav::Coordinates>& polyline = route.GetPolyline();

    pinParameter.pinType = PT_Origin;
    nbnav::Coordinates originCoordinate = polyline[0];
    pinParameter.coordinate.m_latitude = originCoordinate.latitude;
    pinParameter.coordinate.m_longitude = originCoordinate.longitude;
    m_pOriginPin = pMapInterface->AddPin(&pinParameter);
    NKUI_DEBUG_LOG(m_pNKUIController->GetLogger(),
                   "Adding OriginPin at (%lf - %lf)",
                   originCoordinate.latitude, originCoordinate.longitude);

    pinParameter.pinType = PT_Destination;
    nbnav::MapLocation destination = m_pNavUIContex->GetDestination().GetLocation();
    pinParameter.coordinate.m_latitude = destination.center.latitude;
    pinParameter.coordinate.m_longitude = destination.center.longitude;
    m_pDestinationPin = pMapInterface->AddPin(&pinParameter);
    NKUI_DEBUG_LOG(m_pNKUIController->GetLogger(),
                   "Adding DestPin at (%lf - %lf)",
                   destination.center.latitude, destination.center.longitude);
}

bool MapPresenter::CreateRoute(NKUIPolylineParameter& polylineParameter,
                               const nbnav::RouteInformation& route,
                               const std::vector<nbnav::TrafficEvent>& traffics)
{
    // 1. Check if there is enough points....
    const vector<nbnav::Coordinates>& polyline = route.GetPolyline();
    if (polyline.size() < 2 || !polylineParameter.points || !polylineParameter.colorSegments)
    {
        return false;
    }

    polylineParameter.points->clear();
    polylineParameter.colorSegments->clear();
    polylineParameter.width = ROUTE_POLYLINE_WIDTH;
    polylineParameter.points->reserve(polyline.size());

    if (m_pNavUIContex->GetRouteOptions().GetTransportationMode() == nbnav::Pedestrian ||
        route.GetTrafficColor() == '\0' || route.GetTrafficColor() == 'U')
    {
        for_each(polyline.begin(), polyline.end(),
                 NKUICoordinateCollector(polylineParameter.points));
        polylineParameter.colorSegments->push_back(
            NKUIColorSegmentAttribute((uint32)polylineParameter.points->size()-1, TC_Transparent));
        return true;
    }

    // Prepare Logger for later use..
    nbnav::Logger* logger = m_pNKUIController ? m_pNKUIController->GetLogger() : NULL;
    const nbnav::TrafficColorSegment& segment = route.GetTrafficColorSegment();
    for(int i = 0; i< segment.points.size(); i++)
    {
        polylineParameter.points->push_back(COORDINATE_NAV2NKUI(segment.points[i]));
    }
    
    for(int i = 0; i< segment.colorSegments.size(); i++)
    {
        const ColorSegmentAttribute& attr = segment.colorSegments[i];
        polylineParameter.colorSegments->push_back(NKUIColorSegmentAttribute(attr.m_endIndex,
                                                                             (NKUITrafficColor)attr.m_trafficColor));
    }
    // Write to debug log.
    NKUI_DEBUG_LOG(logger, "Total segments: %lu, total points: %lu -- %lu\n",
                   polylineParameter.colorSegments->size(), polylineParameter.points->size(),
                   route.GetPolyline().size());
    if (polylineParameter.colorSegments->size() <= MAX_DEBUG_COUNT)
    {
        for_each(polylineParameter.colorSegments->begin(),
                 polylineParameter.colorSegments->end(), FinalSegementOutputer(logger));
    }
    return !polylineParameter.points->empty();
}

NKUIPolylineParameter* MapPresenter::CreatePolylineParameter()
{
    NKUIPolylineParameter* pParameter = NKUI_NEW NKUIPolylineParameter;
    if (!pParameter)
    {
        return NULL;
    }
    pParameter->points = new std::vector<NKUICoordinate>();
    if (!pParameter->points)
    {
        DestroyPolylineParameter(pParameter);
        return NULL;
    }
    pParameter->colorSegments = new std::vector<NKUIColorSegmentAttribute>();
    if (!pParameter->colorSegments)
    {
        DestroyPolylineParameter(pParameter);
        return NULL;
    }
    return pParameter;
}

void MapPresenter::DestroyPolylineParameter(NKUIPolylineParameter* pParameter)
{
    if (pParameter)
    {
        if (pParameter->points)
        {
            delete pParameter->points;
        }
        if (pParameter->colorSegments)
        {
            delete pParameter->colorSegments;
        }
        delete pParameter;
    }
}

/* See description in header file. */
void MapPresenter::SetAvatarLocationAndCamera(const NKUILocation& location, bool needsAnimation)
{
    if (!m_pNavUIContex)
    {
        return;
    }
    NKUIMapInterface* pMapInterface = m_pNavUIContex->GetMapInterface();
    if (!pMapInterface)
    {
        return;
    }
    const MapSetting& mapSetting = m_pNavUIContex->GetMapSetting();
    if (mapSetting.cameraMode == NCM_MapMode || mapSetting.trackingMode == NCTM_Free)
    {
        // update avatar location only.
        pMapInterface->SetAvatarLocation(location);
        return;
    }

    double headingAngle  = MAP_HEADINGAGGLE;
    //@todo: calculate camera based on avatar location/heading/tileangle.
    if(m_pNavUIContex->IsStartupOffroute() &&
       (mapSetting.cameraMode == NCM_NavMode || mapSetting.cameraMode == NCM_PedestrianMode))
    {
        headingAngle = m_headingToRoute;
    }
    else
    {
        if (mapSetting.trackingMode == NCTM_LocationAndHeading)
        {
            headingAngle = location.m_heading;
        }
    }

    NKUICoordinate nkuiCoordinate(location.m_latitude, location.m_longitude);
    NKUICameraPosition camera(nkuiCoordinate, m_lastCameraPosition.m_zoomLevel,
                              headingAngle, m_lastCameraPosition.m_tiltAngle);
    NKUIAnimationParameters animation;
    if (needsAnimation)
    {
        pMapInterface->LockCameraPosition(false);
        ++m_animationCallbackCounter;
        animation = PrepareAnimationParameter(
            &MapPresenter::AnimationCallbackLockCameraPosition,
            &MapPresenter::AnimationDefaultFailover,
            __FUNCTION__);
    }
    m_lastCameraPosition = camera;
    if (mapSetting.cameraMode == NCM_PedestrianMode)
    {
        pMapInterface->SetCamera(camera, animation);
    }
    else
    {
        pMapInterface->SetAvatarLocationAndCameraPosition(location,
                                                          camera,
                                                          animation);
    }
}

void MapPresenter::ZoomToFitBoundingBox(const nkui::NKUICoordinateBounds& boundingBox,
                                        bool retryIfFailed)
{
    m_waitTransitionEnd = true;

    NKUIMapInterface* pMapInterface = m_pNavUIContex->GetMapInterface();
    double maxX = 0.0;
    double minX = 0.0;
    double maxY = 0.0;
    double minY = 0.0;
    nbnav::NavUtils::MercatorForward(boundingBox.northEast.m_latitude,
                                     boundingBox.northEast.m_longitude,
                                     &minX, &maxY);
    nbnav::NavUtils::MercatorForward(boundingBox.southWest.m_latitude,
                                     boundingBox.southWest.m_longitude,
                                     &maxX, &minY);
    float topMargin = ZOOM_TOP_MARGIN;
    float bottomMargin = ZOOM_BOTTOM_MARGIN;
    if(m_pNKUIController->IsLandscape())
    {
        topMargin = ZOOM_TOP_MARGIN_L;
        bottomMargin = ZOOM_BOTTOM_MARGIN_L;
    }
    double diffX = maxX - minX;
    double diffY = maxY - minY;
    double totalX = diffX / (1.f - ZOOM_LEFT_MARGIN - ZOOM_RIGHT_MARGIN);
    double totalY = diffY / (1.f - topMargin - bottomMargin);
    maxX += totalX * ZOOM_RIGHT_MARGIN;
    minX -= totalX * ZOOM_LEFT_MARGIN;
    maxY += totalY * topMargin;
    minY -= totalY * bottomMargin;
    nkui::NKUICoordinateBounds fitBoundingBox;
    nbnav::NavUtils::MercatorReverse(minX, maxY,
                                     &fitBoundingBox.northEast.m_latitude,
                                     &fitBoundingBox.northEast.m_longitude);
    nbnav::NavUtils::MercatorReverse(maxX, minY,
                                     &fitBoundingBox.southWest.m_latitude,
                                     &fitBoundingBox.southWest.m_longitude);

    pMapInterface->LockCameraPosition(false);
    ++m_animationCallbackCounter;
    m_expectedCameraPostion = pMapInterface->CameraFromBoundingBox(fitBoundingBox);
    pMapInterface->MoveCamera(fitBoundingBox,
                              PrepareAnimationParameter(
                                  &MapPresenter::AnimationCallbackLockCameraPosition,
                                  retryIfFailed ? &MapPresenter::AnimationFailoverZoomToAll :
                                  &MapPresenter::AnimationDefaultFailover,
                                  __FUNCTION__));
}

void MapPresenter::UpdateManeuverList(const nbnav::ManeuverList& maneuvers)
{
    AddManeuverArrow(maneuvers);
}

void MapPresenter::AddManeuverArrow(const nbnav::ManeuverList& maneuvers)
{
    RemoveManeuverArrow();
    if (maneuvers.GetNumberOfManeuvers() > 1)
    {
        m_pManeuverArrowParameter = CreateManeuverArrowParameter(maneuvers.GetManeuver(0), false);
        if (m_pManeuverArrowParameter)
        {
            if ( m_pManeuverArrowParameter->points.size() == 0)
            {
                m_pManeuverArrowParameter.reset();
                return;
            }
        }
        else
        {
            return;
        }

        shared_ptr<NKUIManeuverArrowParameter> pNextManeuverArrowParameter = CreateManeuverArrowParameter(maneuvers.GetManeuver(1), true);
        if (pNextManeuverArrowParameter)
        {
            if (pNextManeuverArrowParameter->points.size() == 0)
            {
                m_pManeuverArrowParameter.reset();
                pNextManeuverArrowParameter.reset();
                return;
            }
        }
        else
        {
            m_pManeuverArrowParameter.reset();
            return;
        }

        //@todo:U turn should be processed later.
        m_pManeuverArrowParameter->points.insert(m_pManeuverArrowParameter->points.end(),
                                                  pNextManeuverArrowParameter->points.begin(),
                                                  pNextManeuverArrowParameter->points.end());

        if (m_isShowManeuverArrow)
        {
            m_pManeuverArrow = m_pNavUIContex->GetMapInterface()->AddManeuverArrow(*m_pManeuverArrowParameter);
            m_pManeuverArrow->SetSelected(true);
        }
        pNextManeuverArrowParameter.reset();
    }
}

void MapPresenter::RemoveManeuverArrow(bool isRemoveParameter)
{
    if (m_pManeuverArrow)
    {
        m_pNavUIContex->GetMapInterface()->RemoveManeuverArrow(m_pManeuverArrow);
        m_pManeuverArrow.reset();
    }
    if (isRemoveParameter)
    {
        m_pManeuverArrowParameter.reset();
    }
}

void MapPresenter::AddBreadCrumb(const NKUILocation& currentLocation)
{
    if (m_pBreadCrumbs == NULL)
    {
        m_pBreadCrumbs = NKUI_NEW BreadCrumb(m_pNavUIContex->GetMapInterface());
    }
    m_pBreadCrumbs->AddPoint(currentLocation);
}

void MapPresenter::UpdateBreadCrumb()
{
    if (m_pBreadCrumbs != NULL)
    {
        m_pBreadCrumbs->Update();
    }
}

void MapPresenter::InterpolatedGPSUpdated(const nbnav::Coordinates& coordinate,
                                          double heading)
{
    const MapSetting& mapSetting = m_pNavUIContex->GetMapSetting();
    //@todo: the code is temporary, we will try to find the reason later
    if ( nsl_isnan(coordinate.latitude)
         || nsl_isnan(coordinate.longitude)
         || nsl_fabs(heading) >= 360
         || nsl_isnan(heading)
         || nsl_fabs(coordinate.latitude) < 0.01
         || nsl_fabs(coordinate.longitude) < 0.01)
    {
        return;
    }

    if (!m_avatarLocation)
    {
        m_avatarLocation.reset(NKUI_NEW Coordinates);
    }

    m_avatarLocation->latitude  = coordinate.latitude;
    m_avatarLocation->longitude = coordinate.longitude;
    m_heading = heading;

    NKUILocation location   = m_pNavUIContex->GetCurrentLocation();
    location.m_latitude     = m_avatarLocation->latitude;
    location.m_longitude    = m_avatarLocation->longitude;
    location.m_heading      = m_heading;

    /*! disable halo in Turn by turn navigation */
    if (mapSetting.polylineType == NNM_TurnByTurn)
    {
        location.m_horizontalUncertainAlongAxis = 0;
    }
    if (m_waitTransitionEnd)
    {
        return;
    }
    NKUIMapInterface* pMapInterface = m_pNavUIContex->GetMapInterface();
    if (mapSetting.trackingMode != NCTM_Free)
    {
        SetAvatarLocationAndCamera(location,false);
    }
    else
    {
        pMapInterface->SetAvatarLocation(location);
    }
}

/* See description in header file. */
void MapPresenter::OnCameraAnimationDone(int animationId, AnimationStatusType status)
{
    fprintf(stderr, "Animation from: %s was %s.\n",
            m_animationDebugInfo[animationId].c_str(),
            status == AST_FINISHED ? "finished":"interrupted");
    m_animationDebugInfo.erase(animationId);
    AnimationCallbackMap::iterator iter = m_animationCallbacks.find(animationId);
    if (iter != m_animationCallbacks.end())
    {
        AnimationCallbackParameterPtr param = iter->second;
        param->m_status = status;
        (param->m_callback)(this, param);
        m_animationCallbacks.erase(iter);
    }
}

/* See description in header file. */
void MapPresenter::AnimationCallbackLockCameraPosition(AnimationCallbackParameterPtr param)
{
    if (--m_animationCallbackCounter || !param)
    {
        return;
    }
    m_waitTransitionEnd = false;
    NKUIMapInterface* pMapInterface = m_pNavUIContex->GetMapInterface();
    //@todo: Do we need to lock map if animation is interrupted?
    if (pMapInterface && param->m_status == MapListener::AST_FINISHED)
    {
        pMapInterface->LockCameraPosition(true);
    }
    else
    {
        param->m_failover(this);
    }
}

void MapPresenter::AnimationDefaultFailover()
{
    ShowCompass(true);
    HandleEvent(EVT_MAP_CAMERA_UNLOCKED, NULL);
}

void MapPresenter::AnimationFailoverZoomToAll()
{
    ZoomToFitMutipleRoute();
}

/* See description in header file. */
void MapPresenter::AnimationCallback(AnimationCallbackParameterPtr param)
{
    m_waitTransitionEnd = false;
}

/* See description in header file. */
inline NKUIAnimationParameters
MapPresenter::PrepareAnimationParameter(AnimationCallbackFunction function,
                                        AnimationFailoverFunction failover,
                                        const char* from)
{
    m_waitTransitionEnd = true;
    int id              = ++ m_animationId; // only be called in UI thread, don't need lock.
    AnimationCallbackParameterPtr param(
        NKUI_NEW AnimationCallbackParameter(function, failover));
    m_animationCallbacks.insert(make_pair(id, param));
    m_animationDebugInfo.insert(make_pair(id, string(from)));
    return NKUIAnimationParameters(NAAT_Linear,ANIMATION_TRANSITION_DELAYED_TIME, id);
}
void MapPresenter::ShowCompass(bool isShow)
{
    if (m_pNKUIController && m_pNavUIContex)
    {
        WidgetPosition position = m_pNKUIController->GetWidgetPosition(WGT_COMPASS);
        m_isShowCompass = isShow;
        m_pNavUIContex->GetMapInterface()->SetCompassStateAndPosition(m_isShowCompass,
                                                                      position.left,
                                                                      position.top);
    }
}
/*! @} */
