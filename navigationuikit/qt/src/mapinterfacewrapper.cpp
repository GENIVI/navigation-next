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
    @file mapinterfacewrapper.cpp
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

#include "mapinterfacewrapper.h"
#include "typetranslater.h"

namespace locationtoolkit
{
/*! implement class NKUIPolylineImpl */
NKUIPolylineImpl::NKUIPolylineImpl(LTKNKUIPolyline* polyline)
    :mPolyline(polyline)
{
}

NKUIPolylineImpl::~NKUIPolylineImpl()
{
    mPolyline = NULL;
}

LTKNKUIPolyline* NKUIPolylineImpl::GetPolyline() const
{
    return mPolyline;
}

void NKUIPolylineImpl::SetVisible(bool visible)
{
    if (mPolyline)
    {
        mPolyline->SetVisible(visible);
    }
}

void NKUIPolylineImpl::SetSelected(bool selected)
{
    if (mPolyline)
    {
        mPolyline->SetSelected(selected);
    }
}

void NKUIPolylineImpl::SetZOrder(int zOrder)
{
    if (mPolyline)
    {
        mPolyline->SetZOrder(zOrder);
    }
}

void NKUIPolylineImpl::SetNavMode(nkui::NKUINavigationMode navMode)
{
    if (mPolyline)
    {
        switch (navMode)
        {
            case nkui::NNM_TurnByTurn:
            {
                mPolyline->SetNavMode(M_TurnByTurn);
                break;
            }
            case nkui::NNM_TripOverView:
            {
                mPolyline->SetNavMode(M_TripOverView);
                break;
            }
            case nkui::NNM_RTS:
            {
                mPolyline->SetNavMode(M_RTS);
                break;
            }
            default:
            {
                mPolyline->SetNavMode(M_TurnByTurn);
                break;
            }
        }
    }
}

bool NKUIPolylineImpl::IsVisible() const
{
    if (mPolyline)
    {
        return mPolyline->IsVisible();
    }
    return false;
}

bool NKUIPolylineImpl::IsSeleced() const
{
    if (mPolyline)
    {
        return mPolyline->IsSeleced();
    }
    return false;
}

int NKUIPolylineImpl::GetZOrder() const
{
    if (mPolyline)
    {
        return mPolyline->GetZOrder();
    }
    return 0;
}

nkui::NKUINavigationMode NKUIPolylineImpl::GetNavMode() const
{
    if (mPolyline)
    {
        switch (mPolyline->GetNavMode())
        {
            case M_TurnByTurn:
                return nkui::NNM_TurnByTurn;
            case M_RTS:
                return nkui::NNM_RTS;
            case M_TripOverView:
                return nkui::NNM_TripOverView;
            default:
                break;
        }
    }
    return nkui::NNM_TurnByTurn;
}

void NKUIPolylineImpl::UpdateTrafficPolyline(const nkui::NKUIPolylineParameter* polylineParameter)
{
    NKUIPolylineImpl* nkuiPolyline = NULL;
    if( mPolyline )
    {
        LTKNKUIPolylineParameter ltkPolyLineParameter;
        TypeTranslater::Translate(*polylineParameter,ltkPolyLineParameter);
        mPolyline->UpdateTrafficPolyline(&ltkPolyLineParameter);
    }
}

void NKUIPolylineImpl::SetDayNightMode(bool isDayMode)
{
    if (mPolyline)
    {
        mPolyline->SetDayNightMode(isDayMode);
    }
}

/*! implement class NKUIPinImpl */
NKUIPinImpl::NKUIPinImpl(LTKNKUIPin* pin)
    :mPin(pin)
{
}

NKUIPinImpl::~NKUIPinImpl()
{
    mPin = NULL;
}

LTKNKUIPin* NKUIPinImpl::GetPin() const
{
   return mPin;
}

void NKUIPinImpl::SetVisible(bool visible)
{
    if (mPin)
    {
        mPin->visible = visible;
    }
}

void NKUIPinImpl::SetSelected(bool selected)
{
    if (mPin)
    {
        mPin->selected = selected;
    }
}

bool NKUIPinImpl::IsVisible() const
{
    if (mPin)
    {
        return mPin->visible;
    }
    return false;
}

bool NKUIPinImpl::IsSeleced() const
{
    if (mPin)
    {
        return mPin->selected;
    }
    return false;
}

nkui::NKUICoordinate NKUIPinImpl::GetCoordinate() const
{
    nkui::NKUICoordinate coordinate;
    if (mPin)
    {
       TypeTranslater::Translate(mPin->coordinate,coordinate);
    }
    return coordinate;
}

/*! implement of NKUIBreadCrumbImpl */
NKUIBreadCrumbImpl::NKUIBreadCrumbImpl(LTKNKUIBreadCrumb* breadCrumb)
    :mBreadCrumb(breadCrumb)
{
}

NKUIBreadCrumbImpl::~NKUIBreadCrumbImpl()
{
    mBreadCrumb = NULL;
}

LTKNKUIBreadCrumb* NKUIBreadCrumbImpl::GetBreadCrumb() const
{
    return mBreadCrumb;
}

void NKUIBreadCrumbImpl::SetCoordinate(const nkui::NKUICoordinate& coordinate)
{
    if (mBreadCrumb)
    {
        TypeTranslater::Translate(coordinate, mBreadCrumb->center);
    }
}

void NKUIBreadCrumbImpl::SetHeading(double heading)
{
    if (mBreadCrumb)
    {
        mBreadCrumb->heading = heading;
    }
}

void NKUIBreadCrumbImpl::SetVisible(bool visible)
{
    if (mBreadCrumb)
    {
        mBreadCrumb->visible = visible;
    }
}

bool NKUIBreadCrumbImpl::IsVisible() const
{
    if (mBreadCrumb)
    {
        return mBreadCrumb->visible;
    }
    return false;
}

nkui::NKUIBreadCrumbType NKUIBreadCrumbImpl::GetBreadCrumbType() const
{
    if (mBreadCrumb)
    {
        switch (mBreadCrumb->type)
        {
            case BCT_None:
                return nkui::BCT_None;
            case BCT_Circle:
                return nkui::BCT_Circle;
            case BCT_Arrow:
                return nkui::BCT_Arrow;
            default:
                break;
        }
    }
    return nkui::BCT_None;
}

/*! implement of MapInterfaceWrapper */
MapInterfaceWrapper::MapInterfaceWrapper():QObject(NULL)
{
    mMapInterface = NULL;
    mMapListener = NULL;
}

MapInterfaceWrapper::~MapInterfaceWrapper()
{

}

void MapInterfaceWrapper::SetMapInterface(NavigationUIMapInterface* mapInterface)
{
    if( !mapInterface )
    {
        return;
    }

    if( !mMapInterface )
    {
        mMapInterface = mapInterface;
        //1. connect signals in navigationuimapinterface
        QObject::connect(mMapInterface, SIGNAL(CameraUpdated(const LTKNKUICameraPosition&)),this, SLOT(OnCameraUpdate(const LTKNKUICameraPosition&)));
        QObject::connect(mMapInterface, SIGNAL(Unlocked()),this, SLOT(OnUnlocked()));
        QObject::connect(mMapInterface, SIGNAL(CameraAnimationDone(qint32, AnimationState)),this, SLOT(OnCameraAnimationDone(qint32, AnimationState)));
        QObject::connect(mMapInterface, SIGNAL(PolylineClicked(QVector<LTKNKUIPolyline*>)),this, SLOT(OnPolylineClicked(QVector<LTKNKUIPolyline*>)));
        QObject::connect(mMapInterface, SIGNAL(PinSelected(LTKNKUIPin*)),this, SLOT(OnSelectPin(LTKNKUIPin*)));
        QObject::connect(mMapInterface, SIGNAL(DidTapAtCoordinate(const Coordinates&)),this, SLOT(DidTapAtCoordinate(const Coordinates&)));
    }
}

void MapInterfaceWrapper::SetNKMapListener(nkui::MapListener* listener)
{
    if( !listener )
    {
        return;
    }

    if( !mMapListener )
    {
        mMapListener = listener;
    }
}

void MapInterfaceWrapper::SetCamera(const nkui::NKUICameraPosition& camera, const nkui::NKUIAnimationParameters& animation)
{
    if( mMapInterface )
    {
        LTKNKUICameraPosition ltkCameraPosition;
        TypeTranslater::Translate(camera,ltkCameraPosition);
        LTKNKUIAnimationParameters ltkAnimation;
        TypeTranslater::Translate(animation,ltkAnimation);
        mMapInterface->SetCamera(ltkCameraPosition,ltkAnimation);
    }
    return;
}

void MapInterfaceWrapper::MoveCamera(const nkui::NKUICoordinateBounds& boundingbox, const nkui::NKUIAnimationParameters& navAnimation)
{
    if( mMapInterface )
    {
        LTKNKUICoordinateBounds ltkCoordinateBounds;
        TypeTranslater::Translate(boundingbox,ltkCoordinateBounds);
        LTKNKUIAnimationParameters ltkAnimation;
        TypeTranslater::Translate(navAnimation,ltkAnimation);
        mMapInterface->MoveCamera(ltkCoordinateBounds,ltkAnimation);
    }
    return;
}

void MapInterfaceWrapper::SetAvatarMode(nkui::NKUIAvatarMode avatarMode)
{
    if( mMapInterface )
    {
        AvatarMode ltkAvatarMode = (AvatarMode)avatarMode;
        mMapInterface->SetAvatarMode(ltkAvatarMode);
    }
    return;
}

void MapInterfaceWrapper::SetAvatarLocation(const nkui::NKUILocation& location)
{
    if( mMapInterface )
    {
        Location ltkLocation;
        TypeTranslater::Translate(location,ltkLocation);
        mMapInterface->SetAvatarLocation(ltkLocation);
    }
    return;
}

nkui::NKUIPolyline* MapInterfaceWrapper::AddRoute(const nkui::NKUIPolylineParameter& polylineParameter)
{
    NKUIPolylineImpl* nkuiPolyline = NULL;
    if( mMapInterface )
    {
        LTKNKUIPolylineParameter ltkPolyLineParameter;
        TypeTranslater::Translate(polylineParameter,ltkPolyLineParameter);
        LTKNKUIPolyline* ltkPolyline = mMapInterface->AddRoute(ltkPolyLineParameter);
        nkuiPolyline = new NKUIPolylineImpl(ltkPolyline);
        mRoutes.push_back(nkuiPolyline);
    }
    return nkuiPolyline;
}

void MapInterfaceWrapper::RemoveRoutes(const std::vector<nkui::NKUIPolyline*>& routes)
{
    if( mMapInterface )
    {
        QVector<LTKNKUIPolyline*> ltkPolylines;
        std::vector<nkui::NKUIPolyline*>::const_iterator iter = routes.begin();
        QVector<NKUIPolylineImpl*>::iterator iterPolyline = mRoutes.begin();
        while ( iter != routes.end() )
        {
            NKUIPolylineImpl* nkuiPolyline = dynamic_cast<NKUIPolylineImpl*>(*iter);
            if (nkuiPolyline != NULL)
            {
                LTKNKUIPolyline* polyline = nkuiPolyline->GetPolyline();
                iterPolyline = std::find(mRoutes.begin(), mRoutes.end(), nkuiPolyline);
                if (iterPolyline != mRoutes.end())
                {
                    mRoutes.erase(iterPolyline);
                }
                ltkPolylines.push_back(polyline);
                delete nkuiPolyline;
            }
            iter++;
        }
        mMapInterface->RemoveRoutes(ltkPolylines);
    }
}

shared_ptr<nkui::NKUIPolyline> MapInterfaceWrapper::AddManeuverArrow(const nkui::NKUIManeuverArrowParameter& maneuverArrowParameter)
{
    if( mMapInterface )
    {
        LTKNKUIManeuverArrowParameter ltkManeuverArrowParameter;
        TypeTranslater::Translate(maneuverArrowParameter,ltkManeuverArrowParameter);
        LTKNKUIPolyline* ltkManeuverArrow = mMapInterface->AddManeuverArrow(ltkManeuverArrowParameter);
        mManeuverArrow.reset( new NKUIPolylineImpl(ltkManeuverArrow));
    }
    return mManeuverArrow;
}

void MapInterfaceWrapper::RemoveManeuverArrow(shared_ptr<nkui::NKUIPolyline> maneuverArrow)
{
    if( mMapInterface )
    {
        mManeuverArrow = dynamic_pointer_cast<NKUIPolylineImpl>(maneuverArrow);
        LTKNKUIPolyline* ltkManeuverArrow = mManeuverArrow->GetPolyline();
        mMapInterface->RemoveManeuverArrow(ltkManeuverArrow);
        mManeuverArrow.reset();
    }
    return;
}

nkui::NKUIPin* MapInterfaceWrapper::AddPin(const nkui::NKUIPinParameter* pinParameters)
{
    NKUIPinImpl* nkuiPin = NULL;
    if( mMapInterface )
    {
        LTKNKUIPinParameter ltkPinParameters;
        TypeTranslater::Translate(*pinParameters,ltkPinParameters);
        LTKNKUIPin* pin = mMapInterface->AddPin(ltkPinParameters);
        nkuiPin = new NKUIPinImpl(pin);
        mPins.push_back(nkuiPin);
    }
    return nkuiPin;
}

void MapInterfaceWrapper::RemovePins()
{
    if( mMapInterface )
    {
        mMapInterface->RemovePins();
        QVector<NKUIPinImpl*>::iterator iterPin = mPins.begin();
        while (iterPin != mPins.end())
        {
            delete *iterPin;
            iterPin++;
        }
        mPins.clear();
    }
    return;
}

nkui::NKUIBreadCrumb* MapInterfaceWrapper::AddBreadCrumb(const nkui::NKUIBreadCrumbParameter& breadCrumbParameters)
{
    NKUIBreadCrumbImpl* nkuiBreadCrumb = NULL;
    if( mMapInterface )
    {
        LTKNKUIBreadCrumbParameter ltkBreadCrumbParameter;
        TypeTranslater::Translate(breadCrumbParameters,ltkBreadCrumbParameter);
        LTKNKUIBreadCrumb* breadCrumb = mMapInterface->AddBreadCrumb(ltkBreadCrumbParameter);
        nkuiBreadCrumb = new NKUIBreadCrumbImpl(breadCrumb);
        mBreadCrumb.push_back(nkuiBreadCrumb);
    }
    return nkuiBreadCrumb;
}

void MapInterfaceWrapper::RemoveAllBreadCrumb()
{
    if( mMapInterface )
    {
        mMapInterface->RemoveAllBreadCrumb();
        QVector<NKUIBreadCrumbImpl*>::iterator iterBreadCrumb = mBreadCrumb.begin();
        while (iterBreadCrumb != mBreadCrumb.end())
        {
            delete *iterBreadCrumb;
            iterBreadCrumb++;
        }
        mBreadCrumb.clear();
    }
    return;
}

void MapInterfaceWrapper::LockCameraPosition(bool locked)
{
    if( mMapInterface )
    {
        mMapInterface->LockCameraPosition(locked);
    }
    return;
}

double MapInterfaceWrapper::MetersPerPixel(double expectedLatitude)
{
    double result = 0.0;
    if( mMapInterface )
    {
       result = mMapInterface->MetersPerPixel( (qreal)expectedLatitude );
    }
    return result;
}

nkui::ScreenPoint MapInterfaceWrapper::ToScreenLocation(const nkui::NKUICoordinate& coordinates)
{
    nkui::ScreenPoint screenPoint;
    if( mMapInterface )
    {
        Coordinates ltkCoordinates;
        TypeTranslater::Translate(coordinates,ltkCoordinates);
        QPoint qScreenLocation = mMapInterface->ToScreenLocation( ltkCoordinates );
        screenPoint.x = qScreenLocation.x();
        screenPoint.y = qScreenLocation.y();
    }
    return screenPoint;
}

nkui::NKUICoordinate MapInterfaceWrapper::FromScreenLocation(const nkui::ScreenPoint& point)
{
    nkui::NKUICoordinate nkuiCoordinate;
    if( mMapInterface )
    {
        QPoint qPoint;
        qPoint.setX( (int)point.x );
        qPoint.setY( (int)point.y );
        Coordinates ltkCoordinates = mMapInterface->FromScreenLocation( qPoint );
        TypeTranslater::Translate(ltkCoordinates,nkuiCoordinate);
    }
    return nkuiCoordinate;
}

void MapInterfaceWrapper::SetCompassStateAndPosition(bool isEnable, double screenX, double screenY)
{
    if( mMapInterface )
    {
        mMapInterface->SetCompassStateAndPosition( isEnable, (qreal)screenX, (qreal)screenY );
    }
    return;
}

void MapInterfaceWrapper::Prefetch(const std::vector<nkui::NKUICoordinate>& points, float zoomLevel)
{
    if( mMapInterface )
    {
        std::vector<nkui::NKUICoordinate>::const_iterator iterPoint = points.begin();
        QVector<Coordinates> ltkCoordinates;
        while ( iterPoint != points.end() )
        {
            nkui::NKUICoordinate nkuiCoordinate = *iterPoint;
            Coordinates coordinate;
            TypeTranslater::Translate(nkuiCoordinate,coordinate);
            ltkCoordinates.push_back(coordinate);
            iterPoint++;
        }
        mMapInterface->Prefetch( ltkCoordinates,(qreal)zoomLevel );
    }
    return;
}

void MapInterfaceWrapper::EnableReferenceCenter(bool enable)
{
    if( mMapInterface )
    {
        mMapInterface->EnableReferenCecenter(enable);
    }
    return;
}

void MapInterfaceWrapper::SetAvatarLocationAndCameraPosition(const nkui::NKUILocation& location,
                                                    const nkui::NKUICameraPosition& camera,
                                                    const nkui::NKUIAnimationParameters& animation)
{
    if( mMapInterface )
    {
        Location ltkLocation;
        TypeTranslater::Translate(location,ltkLocation);
        LTKNKUICameraPosition ltkCameraPosition;
        TypeTranslater::Translate(camera,ltkCameraPosition);
        LTKNKUIAnimationParameters ltkAnimation;
        TypeTranslater::Translate(animation,ltkAnimation);
        mMapInterface->SetAvatarLocationAndCameraPosition(ltkLocation,ltkCameraPosition,ltkAnimation);
    }
    return;
}

shared_ptr<nkui::NKUICameraPosition> MapInterfaceWrapper::CameraFromBoundingBox (const nkui::NKUICoordinateBounds& boundingBox)
{
    shared_ptr<nkui::NKUICameraPosition> nkuiCamera(new nkui::NKUICameraPosition());
    if( mMapInterface )
    {
        LTKNKUICoordinateBounds ltkCoordinateBounds;
        TypeTranslater::Translate(boundingBox,ltkCoordinateBounds);
        LTKNKUICameraPosition camera = mMapInterface->CameraFromBoundingBox(ltkCoordinateBounds);
        if (nkuiCamera)
        {
            nkuiCamera->m_coordinate.m_latitude  = camera.coordinate.latitude;
            nkuiCamera->m_coordinate.m_longitude = camera.coordinate.longitude;
            nkuiCamera->m_zoomLevel              = camera.zoomLevel;
            nkuiCamera->m_headingAngle           = camera.headingAngle;
            nkuiCamera->m_tiltAngle              = camera.tiltAngle;
        }
    }
    return nkuiCamera;
}

void MapInterfaceWrapper::SetSize(nkui::WidgetPosition position)
{
    if( mMapInterface )
    {
        QRect rect( position.left, position.top, position.width, position.height );
        mMapInterface->SetGeometry( rect );
    }
}


//slots:receive map changes signals
void MapInterfaceWrapper::OnCameraUpdate(const LTKNKUICameraPosition& camera)
{
    if( mMapListener )
    {
        nkui::NKUICameraPosition nkuiCamera;
        TypeTranslater::Translate(camera,nkuiCamera);
        mMapListener->OnCameraUpdate(nkuiCamera);
    }
}

void MapInterfaceWrapper::OnUnlocked()
{
    if( mMapListener )
    {
        mMapListener->OnUnlocked();
    }
}

void MapInterfaceWrapper::OnCameraAnimationDone(qint32 animationId, AnimationState state)
{
    if( mMapListener )
    {
        nkui::MapListener::AnimationStatusType nkuiStatus;
        switch( state )
        {
        case AS_FINISHED:
            nkuiStatus = nkui::MapListener::AST_FINISHED;
            break;
        case AS_INTERRUPTED:
            nkuiStatus = nkui::MapListener::AST_INTERRUPTED;
            break;
        default:
            break;
        }
        mMapListener->OnCameraAnimationDone( (int)animationId, nkuiStatus);
    }
}

void MapInterfaceWrapper::OnPolylineClicked(QVector<LTKNKUIPolyline*> polylines)
{
    if( mMapListener )
    {
        std::vector<nkui::NKUIPolyline*> nkuiPolylines;
        QVector<LTKNKUIPolyline*>::iterator polylinesIter = polylines.begin();
        while ( polylinesIter != polylines.end() )
        {
            QVector<NKUIPolylineImpl*>::iterator routesIter = mRoutes.begin();
            while ( routesIter != mRoutes.end() )
            {
                if ((*routesIter)->GetPolyline()->GetID() == (*polylinesIter)->GetID() )
                {
                    nkuiPolylines.push_back(*routesIter);
                }
                routesIter++;
            }

            polylinesIter++;
        }

        mMapListener->OnPolylineClick(nkuiPolylines);
    }
}

void MapInterfaceWrapper::OnSelectPin(LTKNKUIPin* pin)
{
    if( mMapListener )
    {
        NKUIPinImpl* pSelectedPin = NULL;
        QVector<NKUIPinImpl*>::iterator iter = mPins.begin();
        while (iter != mPins.end())
        {
            NKUIPinImpl* pNKUIPin = *iter;
            if (pNKUIPin->GetPin() == pin)
            {
                pSelectedPin = pNKUIPin;
                break;
            }
            iter++;
        }
        mMapListener->OnPinSelected(pSelectedPin);
    }
}

void MapInterfaceWrapper::DidTapAtCoordinate(const Coordinates& coordinate)
{
    if( mMapListener )
    {
        nkui::NKUICoordinate nkuiCoordinate;
        TypeTranslater::Translate(coordinate,nkuiCoordinate);
        mMapListener->OnTapAtCoordinate(nkuiCoordinate);
    }
}
}

