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
    @file nkuitypes.h
    @date 11/4/2014
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
#ifndef LOCATIONTOOLKIT_NAVIGATIONUI_TYPES_H
#define LOCATIONTOOLKIT_NAVIGATIONUI_TYPES_H

#include <QtGlobal>
#include "coordinate.h"
#include "colorsegment.h"
#include "data/place.h"

namespace locationtoolkit
{

/*!< BreadCrumb structures . */
typedef enum
{
    BCT_None = 0,
    BCT_Circle,
    BCT_Arrow
}BreadCrumbType;

/*!< Pin bubble direction type . */
typedef enum
{
    PBDT_None = 0,
    PBDT_LEFT_UP_OFF,
    PBDT_LEFT_UP_ON,
    PBDT_RIGHT_UP_OFF,
    PBDT_RIGHT_UP_ON
}PinBubbleDirectionType;

typedef struct
{
    bool                    visible;     /*!< To control if is visible or not . */
    Coordinates             center;      /*!< Position of bread crumb center. */
    qreal                   heading;     /*!< Heading angle of bread crumb if is arrow. */
    BreadCrumbType          type;        /*!<  type of the bread crumb.. */
}LTKNKUIBreadCrumb;

typedef struct
{
    Coordinates                 center;       /*!< Position of bread crumb center. */
    qreal                       heading;      /*!<  Heading angle of bread crumb if is arrow. */
    bool                        visible;
    BreadCrumbType              type;
}LTKNKUIBreadCrumbParameter;

/*!< Cameraposition structures . */
typedef enum
{
    AAT_None = 0,
    AAT_Linear,
    AAT_Deceleration
}AnimationAccelerationType;

typedef struct
{
    Coordinates     coordinate;         /*!<  Location on the Earth towards which the camera points. */
    qreal           zoomLevel;          /*!<Zoom level. Zoom uses an exponentional scale. */
    qreal           headingAngle;       /*!< Heading angle of the camera, in degrees clockwise from true north. */
    qreal           tiltAngle;          /*!< The angle, in degrees, of the camera angle from the nadir (directly facing the Earth). 0 is straight down, 90 is parallel to the ground. */
}LTKNKUICameraPosition;

typedef struct
{
    AnimationAccelerationType           accelerationType;
    qint32                              duration;
    qint32                              animationId;
}LTKNKUIAnimationParameters;

/*!< CoordinateBounds structures . */
typedef struct
{
    Coordinates    northEast;
    Coordinates    southWest;
}LTKNKUICoordinateBounds;

/*!< PIN structures . */
typedef enum
{
    PT_None = 0,       /*! none */
    PT_Origin,         /*! Origin */
    PT_Destination     /*! Destination */
}PinType;

typedef struct
{
    Coordinates     coordinate;     /*!<  Location on the Earth towards which the pin points. */
    bool            visible;        /*!<  controls if the pin is visible. */
    bool            selected;       /*!< controls if pin selected or not.. */
}LTKNKUIPin;

typedef struct
{
    Coordinates    coordinate;
    PinType        pinType;
}LTKNKUIPinParameter;

/*!< LTKNKUIPolyline structures . */
typedef enum
{
    TC_Transparent = 0,
    TC_Red,
    TC_Yellow,
    TC_Green
}NavTrafficColor;

typedef enum
{
    M_Remain = 0,
    M_TurnByTurn,
    M_TripOverView,
    M_RTS
}NavigationMode;

typedef struct
{
    qint32              endIndex;
    NavTrafficColor     trafficColor;
}LTKNKUIColorSegmentAttribute;

typedef struct
{
    QVector<Coordinates>             points;         /*!< array with NavCoordinate */
    QVector<LTKNKUIColorSegmentAttribute>            colorSegments;  /*!<  array with ColorSegmentAttribute */
    qreal                            width;          /*!< The width of the polyline. */
}LTKNKUIPolylineParameter;

typedef struct
{
    QVector<Coordinates>     points;            /*!<  array with NavCoordinate*/
    qint32                   maneuverwidth;     /*!<   The width of the maneuver polyline.*/
    qint32                   arrowWidth;        /*!< The width of the arrow. */
    qint32                   arrowLength;      /*!< The length of the arrow. */
}LTKNKUIManeuverArrowParameter;

class LTKNKUIPolyline
{
public:
    virtual ~LTKNKUIPolyline() {}
    virtual void SetVisible(bool visible) = 0;
    virtual void SetSelected(bool selected) = 0;
    virtual void SetZOrder(int zOrder) = 0;
    virtual void SetNavMode(NavigationMode navMode) = 0;
    virtual bool IsVisible() const = 0 ;
    virtual bool IsSeleced() const = 0 ;
    virtual int  GetZOrder() const = 0 ;
    virtual NavigationMode GetNavMode() const = 0;
    virtual void SetDayNightMode(bool isDayMode) = 0;
    virtual void UpdateTrafficPolyline(const LTKNKUIPolylineParameter* parameter) = 0;
    virtual const QString& GetID() = 0;
};

/*!< ShareInformation structures . */
typedef struct
{
    QString     currentRouteName;       /*!< current road name where drive on */
    qint32      tripRemainingTime;      /*!< trip remaining time in second */
    Place       destination;            /*!< the place which navigation to. */
    bool        inTrafficArea;          /*!< if drive on a traffic area, when a traffic icon is show on minimap this property is true. */
}ShareInformation;

typedef enum
{
    AM_Remain = 0,     /** remain current mode */
    AM_MapMode,        /** map mode */
    AM_NavMode         /** nav arrow mode */
}AvatarMode;

typedef enum
{
   AS_FINISHED = 0,       /** animation finished */
   AS_INTERRUPTED = 1,    /** animation interrupted */
   AS_UNKNOWN = 2,
}AnimationState;

}  // namespace locationtoolkit
#endif
/*! @} */
