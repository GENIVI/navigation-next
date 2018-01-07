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

    @file     navigationuimapimpl.h
    @date     12/09/2014
    @defgroup navigationuikit sample
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef NAVIGATIONUIMAPIMPL_H
#define NAVIGATIONUIMAPIMPL_H

#include "navigationuimapinterface.h"
#include "navigationuimapcontroller.h"
#include "polyline.h"
#include <QMap>
#include <QVector>

using namespace locationtoolkit;

class LTKNKUIPolylineImpl;

class NavigationUIMapImpl : public NavigationUIMapInterface
{
    Q_OBJECT

public:
static const int AVATAR_REFERENCE_BOTTOM_GAP;
static const char* MANEUVER_COLOR;
static const float AVATAR_MAP_SCALE;

public:
    NavigationUIMapImpl(NavigationUIMapController* pNavigationUIMapController);

    void SetCamera(const LTKNKUICameraPosition& camera, const LTKNKUIAnimationParameters& animation);
    void MoveCamera(const LTKNKUICoordinateBounds& boundingbox, const LTKNKUIAnimationParameters& animation);
    void SetAvatarMode(AvatarMode avatarMode);
    void SetAvatarLocationAndCameraPosition(const Location& location, const LTKNKUICameraPosition& camera, const LTKNKUIAnimationParameters& animation);
    void SetAvatarLocation(const Location& location);
    void EnableReferenCecenter(bool enable);
    void LockCameraPosition(bool locked);

    LTKNKUIPolyline* AddRoute(const LTKNKUIPolylineParameter& polyLineParameter);
    void RemoveRoutes(QVector<LTKNKUIPolyline*>& polyline);
    LTKNKUIPolyline* AddManeuverArrow(const LTKNKUIManeuverArrowParameter& maneuverArrowParameter);
    void RemoveManeuverArrow(LTKNKUIPolyline* maneuverArrow);
    LTKNKUIPin* AddPin(const LTKNKUIPinParameter& pinParameters);
    void RemovePins();
    LTKNKUIBreadCrumb* AddBreadCrumb(const LTKNKUIBreadCrumbParameter& breadCrumbParameters);
    void RemoveAllBreadCrumb();
    qreal MetersPerPixel(qreal expectedLatitude);
    QPoint ToScreenLocation(const Coordinates& coordinate);
    Coordinates FromScreenLocation(const QPoint& point);
    void SetCompassStateAndPosition(bool isEnable, qreal screenX, qreal screenY);
    void Prefetch(QVector<Coordinates> points, qreal zoomLevel);
    LTKNKUICameraPosition CameraFromBoundingBox(const LTKNKUICoordinateBounds& boundingbox);
    virtual void SetGeometry( const QRect& rect );

public Q_SLOTS:
    void OnCameraUpdated(const locationtoolkit::CameraParameters& cameraParameters);
    void OnCameraAnimationDone(int cameraAnimationId, locationtoolkit::MapWidget::AnimationStatus animationStatus);
    void OnUnlocked();
    void OnPinClicked(const locationtoolkit::Pin& pin);
    void OnPolylineClicked(const QList<locationtoolkit::Polyline*> polylines);
    void OnMapClicked(const locationtoolkit::Coordinates& coordinate);

private:
    NavigationUIMapController* mNavigationUIMapController;
    QVector<LTKNKUIPolylineImpl*> mPolyLinesMap;
    QList<Polyline *> mArrowPolyLines;
    QMap<Pin*, LTKNKUIPin*> mPinMap;
};

#endif // NAVIGATIONUIMAP_H

/*! @} */
