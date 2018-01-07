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
    @file mapinterfacewrapper.h
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
#ifndef LOCATIONTOOLKIT_NAVIGATIONUI_MAPINTERFACEWRAPPER_H
#define LOCATIONTOOLKIT_NAVIGATIONUI_MAPINTERFACEWRAPPER_H

#include <QObject>
#include "NKUIMapInterface.h"
#include "navigationuimapinterface.h"
#include "smartpointer.h"
#include "data/nkuitypes.h"

namespace locationtoolkit
{

class NKUIPolylineImpl : public nkui::NKUIPolyline
{
public:
    /*! constructor */
    NKUIPolylineImpl(LTKNKUIPolyline* polyline);
    /*! destructor */
    ~NKUIPolylineImpl();

    LTKNKUIPolyline* GetPolyline() const;

    /*! see description of NKIUPolyline */
    virtual void SetVisible(bool visible);
    virtual void SetSelected(bool selected);
    virtual void SetZOrder(int zOrder);
    virtual void SetNavMode(nkui::NKUINavigationMode navMode);
    virtual bool IsVisible() const;
    virtual bool IsSeleced() const;
    virtual int  GetZOrder() const;
    virtual nkui::NKUINavigationMode GetNavMode() const;
    virtual void SetDayNightMode(bool isDayMode);
    virtual void UpdateTrafficPolyline(const nkui::NKUIPolylineParameter* parameter);
private:
    LTKNKUIPolyline* mPolyline;
};

class NKUIPinImpl : public nkui::NKUIPin
{
public:
    /*! constructor */
    NKUIPinImpl(LTKNKUIPin* pin);
    /*! destructor */
    ~NKUIPinImpl();

    LTKNKUIPin* GetPin() const;

    /*! see description of NKIUPin */
    virtual void SetVisible(bool visible);
    virtual void SetSelected(bool selected);
    virtual bool IsVisible() const;
    virtual bool IsSeleced() const;
    virtual nkui::NKUICoordinate GetCoordinate() const;
private:
    LTKNKUIPin* mPin;
};

class NKUIBreadCrumbImpl : public nkui::NKUIBreadCrumb
{
public:
    /*! constructor */
    NKUIBreadCrumbImpl(LTKNKUIBreadCrumb* breadCrumb);
    /*! destructor */
    ~NKUIBreadCrumbImpl();

    LTKNKUIBreadCrumb* GetBreadCrumb() const;

    /*! see description of NKUIBreadCrumb */
    virtual void SetCoordinate(const nkui::NKUICoordinate& coordinate);
    virtual void SetHeading(double heading);
    virtual void SetVisible(bool visible);
    virtual bool IsVisible() const;
    virtual nkui::NKUIBreadCrumbType GetBreadCrumbType() const;
private:
    LTKNKUIBreadCrumb* mBreadCrumb;
};

class MapInterfaceWrapper: public QObject, public nkui::NKUIMapInterface
{
   Q_OBJECT
public:
    MapInterfaceWrapper();
    virtual ~MapInterfaceWrapper();
    void SetMapInterface(NavigationUIMapInterface* mapInterface);

    // refer to NKUIMapInterface for descriptions.
    virtual void SetNKMapListener(nkui::MapListener* listener);
    virtual void SetCamera(const nkui::NKUICameraPosition& camera, const nkui::NKUIAnimationParameters& animation);
    virtual void MoveCamera(const nkui::NKUICoordinateBounds& boundingbox, const nkui::NKUIAnimationParameters& animation);
    virtual void SetAvatarMode(nkui::NKUIAvatarMode avatarMode);
    virtual void SetAvatarLocation(const nkui::NKUILocation& location);
    virtual nkui::NKUIPolyline* AddRoute(const nkui::NKUIPolylineParameter& polylineParameter);
    virtual void RemoveRoutes(const std::vector<nkui::NKUIPolyline*>& routes);
    virtual shared_ptr<nkui::NKUIPolyline> AddManeuverArrow(const nkui::NKUIManeuverArrowParameter& maneuverArrowParameter);
    virtual void RemoveManeuverArrow(shared_ptr<nkui::NKUIPolyline> maneuverArrow);
    virtual nkui::NKUIPin* AddPin(const nkui::NKUIPinParameter* pinParameters);
    virtual void RemovePins();
    virtual nkui::NKUIBreadCrumb* AddBreadCrumb(const nkui::NKUIBreadCrumbParameter& breadCrumbParameters);
    virtual void RemoveAllBreadCrumb();
    virtual void LockCameraPosition(bool locked);
    virtual double MetersPerPixel(double expectedLatitude);
    virtual nkui::ScreenPoint ToScreenLocation(const nkui::NKUICoordinate& coordinates);
    virtual nkui::NKUICoordinate FromScreenLocation(const nkui::ScreenPoint& point);
    virtual void SetCompassStateAndPosition(bool isEnable, double screenX, double screenY);
    virtual void Prefetch(const std::vector<nkui::NKUICoordinate>& points, float zoomLevel);
    virtual void EnableReferenceCenter(bool enable);
    virtual void SetAvatarLocationAndCameraPosition(const nkui::NKUILocation& location,
                                                    const nkui::NKUICameraPosition& camera,
                                                    const nkui::NKUIAnimationParameters& animation);
    virtual shared_ptr<nkui::NKUICameraPosition> CameraFromBoundingBox (const nkui::NKUICoordinateBounds& boundingBox);
    virtual void SetSize(nkui::WidgetPosition position);

    //receive map changes from client/app
public slots:
    void OnCameraUpdate(const LTKNKUICameraPosition& camera);
    void OnUnlocked();
    void OnCameraAnimationDone(qint32 animationId, AnimationState state);
    void OnPolylineClicked(QVector<LTKNKUIPolyline*> polylines);
    void OnSelectPin(LTKNKUIPin* pin);
    void DidTapAtCoordinate(const Coordinates& coordinate);

private:
    NavigationUIMapInterface*                       mMapInterface;
    nkui::MapListener*                              mMapListener;
    QVector<NKUIPolylineImpl*>                      mRoutes;
    QVector<NKUIPinImpl*>                           mPins;
    QVector<NKUIBreadCrumbImpl*>                    mBreadCrumb;
    shared_ptr<NKUIPolylineImpl>                    mManeuverArrow;
};
}  // namespace locationtoolkit
#endif
/*! @} */
