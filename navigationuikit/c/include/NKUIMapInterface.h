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
 @file         NKUIMapInterface.h
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
#ifndef __NKUI_MapInterface_h
#define __NKUI_MapInterface_h

#include <vector>
#include "NKUIMapDataType.h"
#include "NKUILocation.h"
#include "smartpointer.h"
#include "paltypes.h"
#include "NKUIMapDataType.h"
#include "NKUIData.h"

namespace nkui
{

/*!
 * coordinate parameters.
 */
class NKUICoordinate
{
public:
    NKUICoordinate()
        :m_latitude(0),
         m_longitude(0)
    {};
    NKUICoordinate(double latitude, double longitude)
        :m_latitude(latitude),
         m_longitude(longitude)
    {};

    double m_latitude;
    double m_longitude;
};


/*!
 * camera parameters
 */
class NKUICameraPosition
{
public:
    NKUICameraPosition()
        :m_zoomLevel(0.0),
         m_headingAngle(0.0),
         m_tiltAngle(0.0)
    {}
    NKUICameraPosition(NKUICoordinate coordinate,
                       float zoomLevel,
                       double headingAngle,
                       double tiltAngle)
        :m_coordinate(coordinate),
         m_zoomLevel(zoomLevel),
         m_headingAngle(headingAngle),
         m_tiltAngle(tiltAngle)
    {}

    NKUICameraPosition (const NKUICameraPosition& instance)
        :m_coordinate(instance.m_coordinate),
         m_zoomLevel(instance.m_zoomLevel),
         m_headingAngle(instance.m_headingAngle),
         m_tiltAngle(instance.m_tiltAngle)
    {}

    NKUICoordinate m_coordinate;
    float  m_zoomLevel;
    double m_headingAngle;
    double m_tiltAngle;
};

/*!
 * CoordinateBounds parameters
 */
typedef struct
{
    NKUICoordinate northEast;
    NKUICoordinate southWest;
} NKUICoordinateBounds;

/*! Animation parameters used to control map animations */
class NKUIAnimationParameters
{
public:
    NKUIAnimationParameters()
    {
        m_accelerationType = NAAT_None;
        m_durationTime     = 0;
        m_id               = 0;
    }

    NKUIAnimationParameters(NKUIAnimationAccelerationType type,
                            int duration,
                            int id = 0)
    {
        m_accelerationType = type;
        m_durationTime     = duration;
        m_id               = id;
    }

    void SetAnimationParameters(NKUIAnimationAccelerationType type,
                                int duration,
                                int id = 0)
    {
        m_accelerationType = type;
        m_durationTime     = duration;
        m_id               = id;
    }

    NKUIAnimationAccelerationType m_accelerationType;
    int m_durationTime;
    int m_id;  /*!< Identifier of this animation, can be used by MapInterface to notify
                    about the complete event about animation. */
};

/*! colorSegment attributes */
class NKUIColorSegmentAttribute
{
public:
    NKUIColorSegmentAttribute(uint32           endIndex,
                              NKUITrafficColor trafficColor)
        : m_endIndex(endIndex),
          m_trafficColor(trafficColor)
    {
    }

    virtual ~NKUIColorSegmentAttribute()
    {
    }

    uint32           m_endIndex;
    NKUITrafficColor m_trafficColor;
};

/*! polyline parameters */
typedef struct
{
    std::vector<NKUICoordinate>*              points;
    std::vector<NKUIColorSegmentAttribute>*   colorSegments;
    float width;
}NKUIPolylineParameter;

/*! ManeuverArrow Parameters */
typedef struct
{
    std::vector<NKUICoordinate> points;
    float                       maneuverwidth;
    float                      arrowWidth;
    float                      arrowLength;

}NKUIManeuverArrowParameter;

/*! pin parameters */
typedef struct
{
    NKUICoordinate coordinate;
    NKUIPinType    pinType;
}NKUIPinParameter;

/*! polyline class for C++ */
class NKUIPolyline
{
public:
    /*! destructor */
    virtual ~NKUIPolyline(){};

    /*! controls if polyline is visible or not
     *
     * @param visible ture means visible otherwise invisible
     * @return none
     */
    virtual void SetVisible(bool visible) = 0;

    /*! controls if polyline is selected or not
     *
     * @param selected ture means selected otherwise unselected
     * @return none
     */
    virtual void SetSelected(bool selected) = 0;

    /*! set the zOrder of the polyline
     *
     * @param zOrder
     * @return none
     */
    virtual void SetZOrder(int zOrder) = 0;

    /*! set the navigation mode of the polyline
     *
     * @param navMode Item of the NKUINavigationMode
     * @return none
     */
    virtual void SetNavMode(NKUINavigationMode navMode) = 0;

    /*! if the polyline is visible
     *
     * @return true if visible, false if not.
     */
    virtual bool IsVisible() const = 0;

    /*! if the polyline is selected
     *
     * @return true if selected, false if not.
     */
    virtual bool IsSeleced() const = 0;

    /*! get the zOrder of the polyline
     *
     * @return zOrder.
     */
    virtual int  GetZOrder() const = 0;

    /*! get the navigation mode of the polyline
     *
     * @return navigation mode.
     */
    virtual NKUINavigationMode GetNavMode() const = 0;

    /*! Update the traffic polyline
     *
     * @param parameter  the color segment attributes
     * @return none.
     */
    virtual void UpdateTrafficPolyline(const NKUIPolylineParameter* parameter) = 0;

    /*! Set day/night mode of polyline.
     *
     *  This gives implementation of polyline a chance to change color based on day/night mode.
     *
     * @param isDayMode - Flag is Day mode or not.
     * @return None.
     */
    virtual void SetDayNightMode(bool isDayMode) = 0;
};

/*! pin class for C++ */
class NKUIPin
{
public:
    /*! destructor */
    virtual ~NKUIPin(){};

    /*! controls if pin is visible or not
     *
     * @param visible ture means visible otherwise invisible
     * @return none
     */
    virtual void SetVisible(bool visible) = 0;

    /*! controls if pin is selected or not
     *
     * @param selected ture means selected otherwise unselected
     * @return none
     */
    virtual void SetSelected(bool selected) = 0;

    /*! if the pin is visible
     *
     * @return true if visible, false if not.
     */
    virtual bool IsVisible() const = 0;

    /*! if the pin is selected
     *
     * @return true if selected, false if not.
     */
    virtual bool IsSeleced() const = 0;

    /*! get the coordinate of the pin
     *
     * @return coordinate.
     */
    virtual NKUICoordinate GetCoordinate() const = 0;
};

/*! Type of the Bread Crumb for PED. */
enum NKUIBreadCrumbType
{
    BCT_None = 0,
    BCT_Circle,
    BCT_Arrow,
};

/*! class NKUIBreadCrumb for c++ */
class NKUIBreadCrumb
{
public:
    /*! destructor */
    virtual ~NKUIBreadCrumb(){};

    /*! update coordinate
     *
     *  @param coordinate: new coordinate
     *  @return none
     */
    virtual void SetCoordinate(const NKUICoordinate& coordinate) = 0;

    /*! update heading
     *
     *  @param  heading: new heading
     *  @return none
     */
    virtual void SetHeading(double heading) = 0;

    /*! controls if is visible or not
     *
     * @param visible ture means visible otherwise invisible
     * @return none
     */
    virtual void SetVisible(bool visible) = 0;

    /*! if is visible or not
     *
     * @return true if visible, false if not.
     */
    virtual bool IsVisible() const = 0;

    /*! get the type of bread crumb
     *
     * @return type.
     */
    virtual NKUIBreadCrumbType GetBreadCrumbType() const = 0;
};

/*! class NKUIBreadCrumbParameter for c++ */
class NKUIBreadCrumbParameter
{
public:
    NKUIBreadCrumbParameter(const NKUICoordinate& coordinate,
                            double                heading,
                            bool                  visible,
                            NKUIBreadCrumbType    type)
        :m_center(coordinate),
         m_heading(heading),
         m_visible(visible),
         m_type(type)
    {
    };

    NKUICoordinate  m_center;
    double          m_heading;
    bool            m_visible;
    NKUIBreadCrumbType  m_type;
};

/* map Listener for mapview presenter */
class MapListener
{
public:
    virtual ~MapListener(){};

    enum AnimationStatusType
    {
        AST_FINISHED,         /*< Animation is finished. */
        AST_INTERRUPTED       /*< Animation is interrupted. */
    };

    /*!
     * Callback when the Camera update.
     *
     * @param camera The camera updated.
     * @return None.
     */
    virtual void OnCameraUpdate(const NKUICameraPosition& camera) = 0;

    /*!
     * Notify that the camera position of map is unlocked.
     *
     * @return None.
     */
    virtual void OnUnlocked() = 0;

    /*!
     * Notify that the click polyline of map.
     *
     * @param polylines the clicked polylines.
     * @return None.
     */
    virtual void OnPolylineClick(const std::vector<NKUIPolyline*>& polyline) = 0;

    /*!
     *  Notify that the pin is selected.
     *
     *  @param pin: The selected pin.
     #  @return None.
     */
    virtual void OnPinSelected(NKUIPin* pin) = 0;

    /*!
     *  Notify that the map has been tapped.
     *
     *  @param coordinate: the coordinate
     *  @return: None.
     */
    virtual void OnTapAtCoordinate(const nkui::NKUICoordinate& coordinate) = 0;

    /*!
     * Notify camera animation is complete.
     */
    virtual void
    OnCameraAnimationDone(int animationId,           /*!< Identifier of animation, */
                          AnimationStatusType status /*!< Status of this animation. */
                          ) = 0;
};

/* Map Interface for mapview presenter */
class NKUIMapInterface
{
public:
    virtual ~NKUIMapInterface(){};

    /*! NKUIMapInterface */
    /*!
     * Set the Listener object when NKMapImpl need to notify the NavKit UI.
     *
     * @parem listener the printor of NKMapListener.
     * @return None.
     */
    virtual void SetNKMapListener(MapListener*  listener) = 0;

    /*!
     * Set the camera settings.
     *
     * @param camera The camera setting is used to set.
     * @param animation contains set of animation parameters for creating effect of flying camera
     *   from current position to the new position
     * @return None
     */
    virtual void SetCamera(const NKUICameraPosition& camera,
                           const NKUIAnimationParameters& Animation) = 0;

    /*!
     * Set the avatar postion and camera postion at the same time.
     *
     * This interface is called whenever NavkitUI wants to set Avatar location and camera
     * Position at same time. Implementation of LNUMapInterface may use this API to do some
     * optimization internally.
     *
     * @param location Location of avatar to set.
     * @param camera The camera setting is used to set.
     * @param animation contains set of animation parameters for creating effect of flying camera
     *   from current position to the new position
     * @return None.
     */
    virtual void SetAvatarLocationAndCameraPosition(const NKUILocation& location,
                                                    const NKUICameraPosition& camera,
                                                    const NKUIAnimationParameters& Animation) = 0;
    /*!
     * Move the current camera according to a bounding box.
     *
     * This function sets the map center to the center of bounding box and
     * calculates the right zoom level (altitude) to display the whole bounding
     * box.
     *
     * @param boundingbox The bounding box is used to calculate the right zoom
     *                    level.
     * @param animation contains set of animation parameters for creating effect of flying camera
     *   from current position to the new position
     * @return None
     */
    virtual void MoveCamera(const NKUICoordinateBounds& boundingbox,
                            const NKUIAnimationParameters& navAnimation) = 0;

    /*!
     * Set the avatar mode.
     *
     * @param avatarMode The avatar mode is used to set.
     * @return None.
     */
    virtual void SetAvatarMode(NKUIAvatarMode avatarMode) = 0;

    /*!
     *  Set the avatar location.
     *
     *  This function is only used to set the location to display the avatar. It
     *  should not affect the camera setting (map center display).
     *
     * @param location Location of avatar to set.
     * @return None.
     */
    virtual void SetAvatarLocation(const NKUILocation& location) = 0;

    /*!
     * And a new route.
     *
     * @param NKUIPolylineParameter The parameter of the route.
     * @return The polyline object.
     */
    virtual NKUIPolyline* AddRoute(const NKUIPolylineParameter& polylineParameter) = 0;

    /*!
     * Remove the routes.
     *
     * @param routes The polyline objects are specified to remove.
     * @return None.
     */
    virtual void RemoveRoutes(const std::vector<NKUIPolyline*>& routes) = 0;

    /*!
     * And a maneuver arrow.
     *
     * @param maneuverArrowParameter The parameter of the maneuver arrow.
     * @return The polyline object.
     */
    virtual shared_ptr<NKUIPolyline> AddManeuverArrow(const NKUIManeuverArrowParameter& maneuverArrowParameter) = 0;

    /*!
     * Remove a maneuver arrow.
     *
     * @param maneuverArrow The polyline object is specified to remove.
     * @return None.
     */
    virtual void RemoveManeuverArrow(shared_ptr<NKUIPolyline> maneuverArrow) = 0;

    /*!
     * Add a pin.
     *
     * @param pinParameters The parameter of the pin.
     * @return Navpin object.
     */
    virtual NKUIPin* AddPin(const NKUIPinParameter* pinParameters) = 0;

    /*!
     * Remove all pins.
     *
     * @return None.
     */
    virtual void RemovePins() = 0;

    /*!
     * Add bread crumb for pedestrian.
     *
     * @param breadCrumbParameters - The parameter to create NKUIBreadCrumb.
     * @return NKUIBreadCrumb object pointer.
     */
    virtual NKUIBreadCrumb* AddBreadCrumb(const NKUIBreadCrumbParameter& breadCrumbParameters) = 0;

    /*!
     * Remove all bread crumb.
     *
     * @return None.
     */
    virtual void RemoveAllBreadCrumb() = 0;

    /*!
     * Set the map to the lock or unlock mode.
     *
     * @param locked The flag is used to lock or unlock the map.
     * @return None.
     */
    virtual void LockCameraPosition(bool locked) = 0;

    /*!
     * Retrieve the value of meters per pixel.
     *
     * @param expectedLatitude The expected latitude to retrieve the value of
     *                         meters per pixel.
     * @return The value of meters per pixel if success, otherwise a negative
     *         value (-1.0) is returned.
     */
    virtual double MetersPerPixel(double expectedLatitude) = 0;

    /*!
     * Change the coordinate to pixel point of the screen.
     *
     * @param coordinates The coordinate in real word.
     * @return The point shows on the screen.
     */
    virtual nkui::ScreenPoint ToScreenLocation(const NKUICoordinate& coordinates) = 0;

    /*!
     * Change the point of the screen to coordinate.
     *
     * @param The point shows on the screen.
     * @return coordinates The coordinate in real word.
     */
    virtual NKUICoordinate FromScreenLocation(const ScreenPoint& point) = 0;

    /*!
     * @name  setCompassStateAndPosition -  Enable or Disable the compass with set it's position.
     * @param isEnable- enable or disable compass.
     * @param screenX - position on screen x-coordinate (pixel).
     * @param screenY - position on screen y-coordinate (pixel).
     * @return None.
     */
    virtual void SetCompassStateAndPosition(bool isEnable, double screenX, double screenY) = 0;

    /*! Prefetching tiles.
     *
     * @param  points - The points of the expected prefetch.
     * @param  zoomLevel - Current zoomLevel.
     *
     * @return None.
     */
    virtual void Prefetch(const std::vector<NKUICoordinate>& points, float zoomLevel) = 0;

    /*!
     * EnableReferenceCenter - Enable or disable reference center.
     *
     * If reference center is enabled, map should use alternative point as center to zoom
     * in/out. In most cases, the alternative point should be avatar location.
     *
     * @param enable - Flag to indicate enable or no.
     * @return void
     */
    virtual void EnableReferenceCenter(bool enable) = 0;

    /*!
     * Calculate camera for specified bounding box.
     *
     * @param boundingBox: The bounding box is used to calculate camera
     * @return void
     */
    virtual shared_ptr<NKUICameraPosition>
    CameraFromBoundingBox (const NKUICoordinateBounds& boundingBox) = 0;

    /*!
     * set map view size.
     *
     * @param position: set the map size
     * @return void
     */
    virtual void SetSize(WidgetPosition position) = 0;
};
}

#endif
/*! @} */
